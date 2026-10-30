"""Export a relocatable application SDK from a verified system build."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from fetch_deps import digest

ROOT = Path(__file__).resolve().parents[1]


def run(*args, **kwargs):
    return subprocess.run(list(map(str, args)), check=True, **kwargs)


def relativize(root):
    for path in root.rglob('*'):
        if path.is_symlink():
            target = os.readlink(path)
            if target.startswith('/'):
                path.unlink()
                path.symlink_to(os.path.relpath(root / target.lstrip('/'), path.parent))


def export():
    info = json.loads(next((ROOT / 'dist').glob('*-build-info.json')).read_text())
    if info['status'] != 'verified':
        raise RuntimeError('System build must be verified before SDK export')
    state = json.loads((ROOT / '.ci-state.json').read_text())
    if state['commit'] != info['commit']:
        raise RuntimeError('SDK state does not match verified artifacts')
    work = Path(state['work'])
    gccs = list((work / 'prebuilts/gcc/linux-x86/aarch64').glob('*/bin/aarch64-none-linux-gnu-gcc'))
    if len(gccs) != 1:
        raise RuntimeError('Expected one pinned toolchain')
    output = ROOT / 'sdk-image'
    if output.exists():
        shutil.rmtree(output)
    sdk = output / 'sdk'
    sdk.mkdir(parents=True)
    shutil.copytree(gccs[0].parent.parent, sdk / 'toolchain', symlinks=True)
    compiler = sdk / 'toolchain/bin/aarch64-none-linux-gnu-gcc'
    base = subprocess.check_output([str(compiler), '-print-sysroot'], text=True).strip()
    shutil.copytree(base, sdk / 'sysroot', symlinks=True)
    image = work / 'output/buildroot/images/rootfs.ext4'
    # Runtime libraries and target-provided development files come from this
    # exact final rootfs, including the Vulkan overlay installed by build.py.
    with tempfile.TemporaryDirectory(dir=output) as tmp:
        extracted = Path(tmp)
        result = run('debugfs', '-R', f'rdump / {extracted}', image, capture_output=True, text=True)
        errors = [line for line in result.stderr.splitlines() if not line.startswith('debugfs ')]
        if errors:
            raise RuntimeError('\n'.join(errors))
        identity = json.loads((extracted / 'etc/elf2-build-info.json').read_text())
        if identity['commit'] != info['commit']:
            raise RuntimeError('Final rootfs identity mismatch')
        for relative in ('lib', 'usr/lib', 'usr/include'):
            source = extracted / relative
            if source.is_dir():
                run('rsync', '-a', str(source) + '/', str(sdk / 'sysroot' / relative) + '/')
        shutil.copy2(extracted / 'etc/elf2-build-info.json', sdk / 'system.json')
    relativize(sdk / 'sysroot')
    metadata = {'schema': 1, 'system_commit': info['commit'], 'system_tag': info['tag'],
                'kernel_release': info['kernel_release'], 'rootfs_sha256': digest(image),
                'dependencies_sha256': info['dependencies_sha256'],
                'host': 'linux/amd64', 'target': 'aarch64-none-linux-gnu'}
    (sdk / 'manifest.json').write_text(json.dumps(metadata, indent=2) + '\n')
    # Validate at a different path so accidental references into the build tree fail.
    with tempfile.TemporaryDirectory() as tmp:
        relocated = Path(tmp) / 'sdk'
        shutil.copytree(sdk, relocated, symlinks=True)
        for language, text, libraries in [
            ('c', '#include <stdio.h>\nint main(void){ puts("sdk"); return 0; }\n', []),
            ('cpp', '#include <iostream>\nint main(){ std::cout << "sdk"; }\n', []),
            ('vulkan', '#include <vulkan/vulkan.h>\nint main(void){uint32_t n=0; return vkEnumerateInstanceExtensionProperties(0,&n,0);}\n', ['-lvulkan']),
        ]:
            source = Path(tmp) / ('test.cpp' if language == 'cpp' else 'test.c')
            source.write_text(text)
            cc = relocated / ('toolchain/bin/aarch64-none-linux-gnu-g++' if language == 'cpp' else 'toolchain/bin/aarch64-none-linux-gnu-gcc')
            run(cc, '--sysroot=' + str(relocated / 'sysroot'), source, '-o', Path(tmp) / 'test', *libraries)
    (output / 'Dockerfile').write_text('FROM scratch\nCOPY sdk/ /sdk/\nCMD ["/unused"]\n')
    print('SDK_EXPORT_VERIFIED', json.dumps(metadata), flush=True)


if __name__ == '__main__':
    export()
