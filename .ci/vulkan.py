"""Build a pinned Vulkan runtime and SDK overlay with the SDK's AArch64 GCC."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile

from fetch_deps import digest
from rootfs_services import check_elf_aarch64

SOURCE = Path(__file__).resolve().parents[1]
VERSION = '1.3.239'
MALI_REVISION = 'ec78b74adc038f97cf173f4061fe3856efc0dc74'
MALI_BASE = 'https://raw.githubusercontent.com/tsukumijima/libmali-rockchip/' + MALI_REVISION
SOURCES = {
    'headers.tar.gz': ('https://codeload.github.com/KhronosGroup/Vulkan-Headers/tar.gz/refs/tags/v' + VERSION,
                       '86ef8969b96cf391dc86b9c4e5745b8ecaa12ebdaaefd3d8e38bc98e15f30653'),
    'loader.tar.gz': ('https://codeload.github.com/KhronosGroup/Vulkan-Loader/tar.gz/refs/tags/v' + VERSION,
                      'dcdbe3f5362035ecc5ffce2140393de377038c44e00dfb8bae43816eec57dfc6'),
    'tools.tar.gz': ('https://codeload.github.com/KhronosGroup/Vulkan-Tools/tar.gz/refs/tags/v' + VERSION,
                     '39d3798ef957d7bdb1bc2cf6288311eb84e5d11d00847612781b597e773409e7'),
    'libmali-vulkan.so': (MALI_BASE + '/lib/aarch64-linux-gnu/libmali-valhall-g610-g24p0-wayland-gbm.so',
                         'f84e9872f70ada59561d70e4fee7241fd46d5666aaf4aae3501a39bae51b4ca4'),
}


def run(*args):
    subprocess.run([str(a) for a in args], check=True)


def download(name, cache):
    url, expected = SOURCES[name]
    path = cache / name
    if not path.exists() or digest(path) != expected:
        partial = cache / (name + '.partial')
        run('curl', '-fL', '--retry', '3', '--connect-timeout', '30', '--max-time', '600',
            url, '-o', partial)
        if digest(partial) != expected:
            raise RuntimeError('Vulkan dependency checksum mismatch: ' + name)
        partial.replace(path)
    return path


def prepare(work, compiler):
    build = Path(tempfile.mkdtemp(prefix='elf2-vulkan-', dir=work / 'output'))
    cache = Path(os.environ.get('ELF2_VULKAN_DOWNLOADS', '/cache/elf2-vulkan-downloads'))
    cache.mkdir(parents=True, exist_ok=True)
    for name in ('headers', 'loader', 'tools'):
        run('tar', '-xzf', download(name + '.tar.gz', cache), '-C', build)
    prefix = build / 'headers'
    cross = build / 'cross.cmake'
    compiler = str(Path(compiler).resolve())
    cross.write_text('set(CMAKE_SYSTEM_NAME Linux)\nset(CMAKE_SYSTEM_PROCESSOR aarch64)\n'
                     'set(CMAKE_C_COMPILER "' + compiler + '")\n'
                     'set(CMAKE_CXX_COMPILER "' + compiler[:-3] + 'g++")\n')
    run('cmake', '-S', build / ('Vulkan-Headers-' + VERSION), '-B', build / 'headers-build',
        '-DCMAKE_INSTALL_PREFIX=' + str(prefix))
    run('cmake', '--install', build / 'headers-build')
    common = ['-DCMAKE_TOOLCHAIN_FILE=' + str(cross), '-DCMAKE_BUILD_TYPE=Release',
              '-DVulkanHeaders_DIR=' + str(prefix / 'share/cmake/VulkanHeaders'),
              '-DBUILD_WSI_XCB_SUPPORT=OFF', '-DBUILD_WSI_XLIB_SUPPORT=OFF']
    loader_build = build / 'loader-build'
    run('cmake', '-S', build / ('Vulkan-Loader-' + VERSION), '-B', loader_build, *common,
        '-DCMAKE_INSTALL_PREFIX=/usr', '-DCMAKE_INSTALL_LIBDIR=lib',
        '-DBUILD_WSI_WAYLAND_SUPPORT=ON', '-DBUILD_TESTS=OFF')
    run('cmake', '--build', loader_build, '-j8')
    tools_build = build / 'tools-build'
    run('cmake', '-S', build / ('Vulkan-Tools-' + VERSION), '-B', tools_build, *common,
        '-DVulkan_INCLUDE_DIR=' + str(prefix / 'include'),
        '-DVulkan_LIBRARY=' + str(loader_build / 'loader/libvulkan.so'),
        '-DBUILD_WSI_WAYLAND_SUPPORT=OFF', '-DBUILD_CUBE=OFF', '-DBUILD_ICD=OFF')
    run('cmake', '--build', tools_build, '-j8')
    stage = build / 'stage'

    def copy(src, target, executable=False):
        dest = stage / target.lstrip('/')
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dest)
        dest.chmod(0o755 if executable else 0o644)
        return dest

    # Keep the working EGL/GLES/GBM stack; Vulkan loads its own pinned ICD.
    copy(download('libmali-vulkan.so', cache), '/usr/lib/libmali-vulkan.so', True)
    for name in ('libvulkan.so', 'libvulkan.so.1'):
        copy(loader_build / 'loader/libvulkan.so', '/usr/lib/' + name, True)
    copy(tools_build / 'vulkaninfo/vulkaninfo', '/usr/bin/vulkaninfo', True)
    example = SOURCE / 'system/vulkan'
    binary = build / 'elf2-vulkan-test'
    run(compiler, '-O2', '-Wall', '-Wextra', '-Werror', example / 'elf2-vulkan-test.c',
        '-I' + str(prefix / 'include'), '-L' + str(loader_build / 'loader'), '-lvulkan', '-o', binary)
    copy(binary, '/usr/bin/elf2-vulkan-test', True)
    run('glslangValidator', '-V', example / 'compute.comp', '-o', build / 'compute.spv')
    run('spirv-val', build / 'compute.spv')
    for name in ('compute.comp', 'elf2-vulkan-test.c', 'MALI-LICENSE.txt'):
        copy(example / name, '/usr/share/elf2-vulkan/' + name)
    copy(build / 'compute.spv', '/usr/share/elf2-vulkan/compute.spv')
    for p in sorted((prefix / 'include').rglob('*')):
        if p.is_file():
            copy(p, '/usr/include/' + p.relative_to(prefix / 'include').as_posix())
    for component in ('Headers', 'Loader', 'Tools'):
        for p in (build / ('Vulkan-' + component + '-' + VERSION)).glob('LICENSE*'):
            copy(p, '/usr/share/elf2-vulkan/licenses/' + component + '-' + p.name)
    icd = stage / 'usr/share/vulkan/icd.d/mali.json'
    icd.parent.mkdir(parents=True, exist_ok=True)
    icd.write_text(json.dumps({'file_format_version': '1.0.0', 'ICD': {
        'library_path': '/usr/lib/libmali-vulkan.so', 'api_version': '1.3.276'}}, indent=2) + '\n')
    pc = stage / 'usr/lib/pkgconfig/vulkan.pc'
    pc.parent.mkdir(parents=True, exist_ok=True)
    pc.write_text('prefix=/usr\nlibdir=${prefix}/lib\nincludedir=${prefix}/include\n\n'
                  'Name: Vulkan-Loader\nDescription: Vulkan ICD loader\nVersion: ' + VERSION + '\n'
                  'Libs: -L${libdir} -lvulkan\nCflags: -I${includedir}\n')
    for target in ('usr/bin/elf2-vulkan-test', 'usr/bin/vulkaninfo',
                   'usr/lib/libmali-vulkan.so', 'usr/lib/libvulkan.so.1'):
        check_elf_aarch64(stage / target)
    files = sorted(p for p in stage.rglob('*') if p.is_file())
    manifest = stage / 'usr/share/elf2-vulkan/manifest.json'
    manifest.write_text(json.dumps({'headers_loader_tools': VERSION, 'mali_revision': MALI_REVISION,
        'files': {'/' + p.relative_to(stage).as_posix(): digest(p) for p in files}}, indent=2) + '\n')
    files.append(manifest)
    overlay = [(p, '/' + p.relative_to(stage).as_posix(), p.stat().st_mode & 0o777) for p in files]
    return overlay, build


def verify(fs, overlay):
    """Read every installed file back from the final ext4, including the ICD."""
    with tempfile.TemporaryDirectory(prefix='elf2-vulkan-verify-') as tmp:
        dumped = Path(tmp) / 'file'
        for src, target, _ in overlay:
            dumped.unlink(missing_ok=True)
            fs.run('dump ' + target + ' ' + str(dumped))
            if not dumped.exists() or digest(dumped) != digest(src):
                raise RuntimeError('Vulkan rootfs content mismatch: ' + target)


def bundle(overlay, destination):
    with tarfile.open(destination, 'w:gz') as archive:
        for src, target, mode in overlay:
            info = archive.gettarinfo(str(src), arcname=target.lstrip('/'))
            info.uid = info.gid = 0
            info.uname = info.gname = 'root'
            info.mode = mode
            with open(src, 'rb') as stream:
                archive.addfile(info, stream)
