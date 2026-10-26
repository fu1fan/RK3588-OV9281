"""Tag-exact SDK build with isolated, compatible in-tree incremental state."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import lzma
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import time

from fetch_deps import digest, restore
from image import Ext4, install

SOURCE = Path(__file__).resolve().parents[1]
DIST = SOURCE / 'dist'
STATE = SOURCE / '.ci-state.json'
LOG = None


def command(args, cwd=None, env=None, capture=False):
    if capture:
        return subprocess.check_output(args, cwd=cwd, env=env, text=True).strip()
    print('+', ' '.join(map(str, args)), flush=True)
    p = subprocess.Popen(list(map(str, args)), cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for line in p.stdout:
        sys.stdout.buffer.write(line)
        sys.stdout.buffer.flush()
        if LOG:
            LOG.write(line)
            LOG.flush()
    p.stdout.close()
    if p.wait():
        raise subprocess.CalledProcessError(p.returncode, args)


def source_revision(path):
    return command(['git', 'rev-parse', 'HEAD'], cwd=path, capture=True)


def compatibility_key(source, manifest):
    h = hashlib.sha256()
    for p in [source/'.ci/Dockerfile', source/'.ci/build.py', manifest]:
        h.update(p.read_bytes())
    packages = Path('/etc/elf2-ci-packages.txt')
    h.update(packages.read_bytes() if packages.exists() else b'local-validation-host')
    listing = command(['git', 'ls-tree', '-r', 'HEAD', 'kernel', 'u-boot/configs', 'device/rockchip/.chips/elf2'], cwd=source, capture=True)
    for line in listing.splitlines():
        path = line.split('\t', 1)[-1]
        if '/configs/' in path or Path(path).name.startswith('Kconfig') or path in ('kernel/Makefile',):
            h.update(line.encode())
    return h.hexdigest()[:24]


def sync_source(source, work, revision):
    work.mkdir(parents=True, exist_ok=True)
    if not (work/'.git').is_dir():
        command(['git', 'init', '-q'], cwd=work)
    # Fetch from the platform's checkout, never copy its credential-bearing config.
    command(['git', 'fetch', '--no-tags', '--quiet', str(source), revision], cwd=work)
    command(['git', 'reset', '--hard', revision], cwd=work)
    if source_revision(work) != revision:
        raise RuntimeError('Cached source does not match the triggering commit')


def object_state(work):
    result = {}
    for tree in ('kernel', 'u-boot'):
        for p in (work/tree).rglob('*.o'):
            s = p.stat()
            result[str(p.relative_to(work))] = [s.st_size, s.st_mtime_ns]
    return result


def compiler_wrappers(work):
    root = work/'prebuilts/gcc/linux-x86/aarch64'
    gccs = list(root.glob('*/bin/aarch64-none-linux-gnu-gcc'))
    if len(gccs) != 1:
        raise RuntimeError('Expected exactly one pinned aarch64 cross compiler')
    gcc = gccs[0]
    prefix = gcc.name[:-3]
    destination = work/'.ci-toolchain'
    destination.mkdir(exist_ok=True)
    for src in gcc.parent.iterdir():
        if not src.name.startswith(prefix):
            continue
        target = destination/src.name
        if target.is_symlink() or target.exists():
            target.unlink()
        if src.name in (prefix+'gcc', prefix+'g++'):
            target.write_text('#!/bin/sh\nexec /usr/bin/ccache ' + shlex.quote(str(src)) + ' "$@"\n')
            target.chmod(0o755)
        else:
            target.symlink_to(src)
    return str(destination/prefix)


def verify_update(work, env):
    directory = work/'output/ci-final-check'
    if directory.exists():
        shutil.rmtree(directory)
    directory.mkdir()
    tools = work/'tools/linux/Linux_Pack_Firmware/rockdev'
    command([tools/'rkImageMaker','-unpack',work/'output/update/Image/update.img',directory], env=env)
    command([tools/'afptool','-unpack',directory/'firmware.img',directory/'parts'], env=env)
    checks = {'boot.img':work/'kernel/boot.img','recovery.img':work/'output/firmware/recovery.img',
              'rootfs.img':work/'output/buildroot/images/rootfs.ext4'}
    for name, expected in checks.items():
        matches = list((directory/'parts').rglob(name))
        if len(matches)!=1 or digest(matches[0])!=digest(expected):
            raise RuntimeError('Final update.img partition mismatch: '+name)
    if digest(directory/'boot.bin') != digest(work/'output/firmware/MiniLoaderAll.bin'):
        raise RuntimeError('Final update.img loader mismatch')
    shutil.rmtree(directory)


def finish():
    state = json.loads(STATE.read_text())
    work = Path(state['work'])
    assert (work/'.ci-building').exists()
    (work/'.ci-success.json').write_text(json.dumps(state, indent=2)+'\n')
    (work/'.ci-building').unlink()
    print('Successful cache committed:', state['key'], flush=True)


def build():
    global LOG
    start = time.monotonic()
    if DIST.exists():
        assert DIST.parent == SOURCE and DIST.name == 'dist'
        shutil.rmtree(DIST)
    DIST.mkdir()
    tag = os.environ.get('CNB_BRANCH', 'local-validation')
    is_tag = os.environ.get('CNB_IS_TAG', '').lower() == 'true'
    if is_tag and not re.fullmatch(r'system-v\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?', tag):
        raise RuntimeError('Expected system-vMAJOR.MINOR.PATCH[-suffix] tag')
    revision = source_revision(SOURCE)
    if os.environ.get('CNB_COMMIT', revision) != revision:
        raise RuntimeError('CNB commit differs from checkout')
    label = tag if is_tag else 'manual-' + os.environ.get('CNB_BUILD_ID', str(int(time.time())))
    prefix = 'elf2-' + re.sub('[^A-Za-z0-9.-]', '-', label)
    mode = os.environ.get('ELF2_BUILD_MODE', 'incremental')
    if mode not in ('clean', 'incremental'):
        raise RuntimeError('Unknown build mode')
    manifest = SOURCE/'docs/migration/dependencies.json'
    key = compatibility_key(SOURCE, manifest)
    cache = Path(os.environ.get('ELF2_WORK_CACHE', '/cache/elf2-work')).resolve()
    work = cache/key/'sdk'
    work.parent.mkdir(parents=True, exist_ok=True)
    if work.exists() and (mode == 'clean' or (work/'.ci-building').exists()):
        assert work.resolve().is_relative_to(cache) and work.name == 'sdk'
        shutil.rmtree(work)
    previous = json.loads((work/'.ci-success.json').read_text()) if (work/'.ci-success.json').exists() else None
    LOG = (DIST/(prefix+'-build.log')).open('wb')
    info = {'tag': tag, 'commit': revision, 'mode': mode, 'cache_key': key,
            'cache_hit': bool(previous), 'previous_success': previous.get('commit') if previous else None,
            'dependencies_sha256': digest(manifest), 'hardware_tested': False, 'status': 'building'}
    info_path = DIST/(prefix+'-build-info.json')
    STATE.write_text(json.dumps({'work': str(work), 'key': key, 'commit': revision, 'tag': tag}, indent=2))
    try:
        sync_source(SOURCE, work, revision)
        (work/'.ci-building').write_text(revision+'\n')
        if not previous:
            deps = Path(os.environ.get('ELF2_DEPENDENCY_ROOT', '/opt/elf2-deps'))
            if not (deps/'.manifest-sha256').exists() or (deps/'.manifest-sha256').read_text().strip() != digest(manifest):
                restore(manifest, deps, cache/'downloads')
            command(['rsync', '-a', str(deps)+'/', str(work)+'/'])
        # Never reuse a mutated prebuilt rootfs from a prior run.
        deps = Path(os.environ.get('ELF2_DEPENDENCY_ROOT', '/opt/elf2-deps'))
        shutil.copy2(deps/'prebuilts/elfboard/buildroot/rootfs.ext4', work/'prebuilts/elfboard/buildroot/rootfs.ext4')
        shutil.copy2(deps/'prebuilts/elfboard/recovery/rootfs.cpio.gz', work/'prebuilts/elfboard/recovery/rootfs.cpio.gz')
        free = shutil.disk_usage(work).free
        if free < 15 * 1024**3:
            raise RuntimeError('At least 15 GiB free scratch space is required after dependency restore')
        toolchain = compiler_wrappers(work)
        epoch = command(['git', 'show', '-s', '--format=%ct', revision], cwd=SOURCE, capture=True)
        # The vendor SDK saves its initial environment. Do not pass CI credentials.
        env = {'PATH': '/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin',
               'HOME': os.environ.get('HOME', '/root'), 'USER': 'elf2-ci', 'LANG': 'C.UTF-8', 'LC_ALL': 'C.UTF-8',
               'TZ': 'Asia/Shanghai', 'RK_CI_AARCH64_TOOLCHAIN': toolchain,
               'CCACHE_DIR': os.environ.get('ELF2_CCACHE_DIR', '/cache/elf2-ccache'),
               'CCACHE_BASEDIR': str(work), 'CCACHE_COMPILERCHECK': 'content', 'CCACHE_NOHASHDIR': '1',
               'KBUILD_BUILD_USER': 'elf2-ci', 'KBUILD_BUILD_HOST': 'cnb',
               'KBUILD_BUILD_TIMESTAMP': datetime.fromtimestamp(int(epoch), timezone.utc).strftime('%a %b %d %H:%M:%S UTC %Y'),
               'SOURCE_DATE_EPOCH': epoch}
        if mode == 'clean':
            env['CCACHE_DISABLE'] = '1'
        command(['ccache', '--max-size', '10G'], env=env)
        command(['ccache', '--zero-stats'], env=env)
        before = object_state(work)
        command(['./build.sh', 'elf2_sdkbuild_defconfig'], cwd=work, env=env)
        command(['./build.sh', 'all'], cwd=work, env=env)
        release = (work/'kernel/include/config/kernel.release').read_text().strip()
        stage = work/'output/ci-modules'
        if stage.exists():
            shutil.rmtree(stage)
        command(['make', '-C', str(work/'kernel'), 'ARCH=arm64', 'CROSS_COMPILE='+toolchain,
                 'modules_install', 'INSTALL_MOD_PATH='+str(stage), 'INSTALL_MOD_STRIP=1'], env=env)
        command(['depmod', '-b', str(stage), '-F', str(work/'kernel/System.map'), release], env=env)
        mods = stage/'lib/modules'/release
        module_files = list(mods.rglob('*.ko'))
        if not module_files:
            raise RuntimeError('No modules generated')
        for p in module_files:
            magic = command(['modinfo', '-F', 'vermagic', str(p)], capture=True)
            if magic.split()[0] != release:
                raise RuntimeError('Module kernel release mismatch: '+p.name)
        image = work/'output/buildroot/images/rootfs.ext4'
        build_identity = work/'output/ci-build-info.json'
        build_identity.write_text(json.dumps({'tag':tag,'commit':revision,'kernel_release':release}, indent=2)+'\n')
        overlay = [(build_identity, '/etc/elf2-build-info.json')]
        for name in ('S01cgroup', 'S80dnsmasq'):
            overlay.append((work/'buildroot/board/elfboard/elf2/fs-overlay/etc/init.d'/name, '/etc/init.d/'+name))
        fs = install(image, stage/'lib/modules', overlay, work/'output/ci-rootfs.debugfs')
        identity = json.loads(fs.run('cat /etc/elf2-build-info.json'))
        if identity['commit'] != revision:
            raise RuntimeError('Rootfs identity mismatch')
        # Compare each installed module byte-for-byte with this build's module.
        verify = work/'output/ci-module-check.ko'
        for p in module_files:
            target = '/lib/modules/' + p.relative_to(stage/'lib/modules').as_posix()
            command(['debugfs','-R','dump -p '+target+' '+str(verify),str(image)], capture=True)
            if digest(verify) != digest(p):
                raise RuntimeError('Rootfs module content mismatch: '+target)
        verify.unlink(missing_ok=True)
        command(['./build.sh', 'updateimg'], cwd=work, env=env)
        verify_update(work, env)
        after = object_state(work)
        info.update({'kernel_release': release, 'module_count':len(module_files),
                     'module_symvers_sha256':digest(work/'kernel/Module.symvers'),
                     'object_files':len(after), 'reused_object_files':sum(before.get(p)==s for p,s in after.items()),
                     'ccache_statistics': command(['ccache','--show-stats'],env=env,capture=True)})
        config = (work/'kernel/.config').read_text()
        for symbol in ('CONFIG_VIDEO_OV9281=y','CONFIG_TUN=y','CONFIG_EXFAT_FS=y'):
            if symbol not in config.splitlines():
                raise RuntimeError('Required kernel option missing: '+symbol)
        artifacts = {'update.img':work/'output/update/Image/update.img', 'boot.img':work/'kernel/boot.img',
                     'recovery.img':work/'output/firmware/recovery.img', 'kernel.config':work/'kernel/.config',
                     'Module.symvers':work/'kernel/Module.symvers', 'elf2-kernel.dtb':work/'kernel/arch/arm64/boot/dts/rockchip/elf2-kernel.dtb'}
        for name, p in artifacts.items():
            if not p.is_file() or p.stat().st_size == 0:
                raise RuntimeError('Missing artifact: '+name)
            shutil.copy2(p, DIST/(prefix+'-'+name))
        # Validate FIT structure/hash metadata using the platform's image parser.
        command(['dumpimage', '-l', str(DIST/(prefix+'-boot.img'))], env=env)
        info.update({'status':'verified','elapsed_seconds':round(time.monotonic()-start,2),
                     'artifacts':{p.name:{'bytes':p.stat().st_size,'sha256':digest(p)} for p in DIST.iterdir() if p.is_file() and p.name!=prefix+'-build.log'}})
        (DIST/(prefix+'-RELEASE.md')).write_text('ELF2 / RK3588 / OV9281 automated image build\n\nTag: '+tag+'\nCommit: '+revision+'\n\nKernel, U-Boot, recovery and update.img built; rootfs contains matching modules. No hardware acceptance has been performed.\n')
    except Exception as error:
        info.update({'status':'failed','error':str(error),'elapsed_seconds':round(time.monotonic()-start,2)})
        raise
    finally:
        info_path.write_text(json.dumps(info,indent=2)+'\n')
        LOG.close()
        LOG = None
        log = DIST/(prefix+'-build.log')
        with log.open('rb') as src, lzma.open(str(log)+'.xz','wb',preset=1) as out:
            shutil.copyfileobj(src,out)
        log.unlink()
    files = [p for p in DIST.iterdir() if p.is_file() and not p.name.endswith('SHA256SUMS')]
    (DIST/(prefix+'-SHA256SUMS')).write_text(''.join(digest(p)+'  '+p.name+'\n' for p in sorted(files)))
    print('IMAGE_VERIFIED',json.dumps({'tag':tag,'commit':revision,'cache_hit':info['cache_hit'],
          'reused_object_files':info['reused_object_files'],'modules':info['module_count']}),flush=True)


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--finish', action='store_true')
    a = p.parse_args()
    finish() if a.finish else build()
