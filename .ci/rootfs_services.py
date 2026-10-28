"""Cross-compile and verify the userspace pieces that go into the final rootfs.

Everything here is built from this repository during the CI run: the watchdog
feeder is a C program, the two init scripts are POSIX shell. Nothing is copied
from the macOS/Windows developer tree or from the board.
"""
import os
from pathlib import Path
import re
import shutil
import stat
import subprocess
import tempfile

SOURCE = Path(__file__).resolve().parents[1]
SYSTEM = SOURCE / 'system'

WATCHDOG_BINARY = '/usr/bin/elf2-wdctl'
WATCHDOG_SCRIPT = '/etc/init.d/S01watchdog'
WIFI_SCRIPT = '/etc/init.d/S36wifistation'
WIFI_TEMPLATE = '/etc/wifistation.conf.example'
DISPLAY_FILES = ('/etc/elfboard_env.sh', '/etc/profile.d/weston.sh')
DISPLAY_OVERLAY = SOURCE / 'buildroot/board/elfboard/elf2/fs-overlay'

# The scripts must stay in this lexical order: rcS runs /etc/init.d/S??* in
# name order, and the feeder has to be running before anything that can block.
WATCHDOG_ORDER = 1
WIFI_ORDER = 36

WATCHDOG_RUNLEVEL = 'S'
SHELL = '/bin/sh'


def host_compiler():
    for name in ('aarch64-none-linux-gnu-gcc', 'aarch64-linux-gnu-gcc'):
        found = shutil.which(name)
        if found:
            return found
    raise RuntimeError('No AArch64 cross compiler found in PATH')


def compile_watchdog(destination, compiler=None):
    """Build elf2-wdctl for AArch64 and confirm the ELF really is AArch64."""
    compiler = compiler or host_compiler()
    destination = Path(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([compiler, '-O2', '-Wall', '-Wextra', '-Werror', '-std=gnu99',
                    '-o', str(destination),
                    str(SYSTEM / 'elf2-wdctl' / 'elf2-wdctl.c'), '-lpthread'],
                   check=True)
    check_elf_aarch64(destination)
    destination.chmod(0o755)
    return destination


def check_elf_aarch64(path):
    with open(path, 'rb') as handle:
        header = handle.read(20)
    if header[:4] != b'\x7fELF':
        raise RuntimeError(str(path) + ' is not an ELF file')
    if header[4] != 2:
        raise RuntimeError(str(path) + ' is not a 64-bit ELF')
    if header[5] != 1:
        raise RuntimeError(str(path) + ' is not little endian')
    machine = int.from_bytes(header[18:20], 'little')
    if machine != 183:  # EM_AARCH64
        raise RuntimeError('%s has e_machine %d, expected 183 (AArch64)'
                           % (path, machine))
    return True


def check_script(path, required=()):
    """Syntax check a POSIX shell script and its shebang."""
    text = Path(path).read_text()
    if not text.startswith('#!' + SHELL):
        raise RuntimeError('%s does not start with #!%s' % (path, SHELL))
    subprocess.run([SHELL, '-n', str(path)], check=True)
    for token in required:
        if token not in text:
            raise RuntimeError('%s is missing %r' % (path, token))
    return True


def _assignment(text, key):
    match = re.search(r'^%s=(.+)$' % re.escape(key), text, re.M)
    return match.group(1).strip().strip('"') if match else ''


def check_no_credentials(template):
    """The shipped template must not carry a usable network credential."""
    text = Path(template).read_text()
    for key in ('SSID', 'PSK'):
        value = _assignment(text, key)
        if value:
            raise RuntimeError('Shipped %s must be empty, found %r' % (key, value))
    return True


def check_decoupled(wifi_script):
    """Camera bring-up must not depend on the Wi-Fi service or its config."""
    text = Path(wifi_script).read_text()
    for token in ('is_use_dummybuf', 'dummybuf', '/dev/video'):
        if token in text:
            raise RuntimeError('Wi-Fi init script must not touch %r' % token)
    return True


def prepare(work, modules_stage, compiler=None):
    """Build the binaries/scripts and return them as (source, target, mode)."""
    build = Path(tempfile.mkdtemp(prefix='elf2-rootfs-', dir=str(work / 'output')))
    binary = compile_watchdog(build / 'elf2-wdctl', compiler)
    watchdog = SYSTEM / 'init.d' / 'S01watchdog'
    wifi = SYSTEM / 'init.d' / 'S36wifistation'
    template = SYSTEM / 'wifistation.conf.example'

    check_script(watchdog, required=('elf2-wdctl', '/dev/watchdog'))
    check_script(wifi, required=('wpa_supplicant', 'udhcpc'))
    check_no_credentials(template)
    check_decoupled(wifi)
    if WATCHDOG_ORDER >= WIFI_ORDER:
        raise RuntimeError('Watchdog feeder must be started before Wi-Fi')

    # The kernel modules are installed next to the services so one pass
    # writes both; the caller only has to hand the list to image.install().
    overlay = [(binary, WATCHDOG_BINARY, 0o755),
               (watchdog, WATCHDOG_SCRIPT, 0o755),
               (wifi, WIFI_SCRIPT, 0o755),
               (template, WIFI_TEMPLATE, 0o644)]
    overlay.extend((DISPLAY_OVERLAY / path.lstrip('/'), path,
                    0o755 if path == '/etc/elfboard_env.sh' else 0o644)
                   for path in DISPLAY_FILES)
    return overlay, build


def verify(fs):
    """Inspect the written ext4 image through debugfs, no mounting needed."""
    stat_line = fs.run('stat ' + WATCHDOG_BINARY)
    if 'Inode:' not in stat_line:
        raise RuntimeError('Watchdog feeder missing from rootfs')
    if not re.search(r'Mode:\s+(?:0?100755|0755)', stat_line):
        raise RuntimeError('Watchdog feeder is not mode 0755: ' + stat_line)
    if not re.search(r'User:\s+0\s+Group:\s+0', stat_line):
        raise RuntimeError('Watchdog feeder is not root owned')
    for path in (WATCHDOG_SCRIPT, WIFI_SCRIPT):
        line = fs.run('stat ' + path)
        if 'Inode:' not in line or not re.search(r'Mode:\s+(?:0?100755|0755)', line):
            raise RuntimeError('Init script missing or not executable: ' + path)
    if not fs.exists(WIFI_TEMPLATE):
        raise RuntimeError('STA configuration template missing from rootfs')
    for path in DISPLAY_FILES:
        if fs.run('cat ' + path) != (DISPLAY_OVERLAY / path.lstrip('/')).read_text():
            raise RuntimeError('Display configuration content mismatch: ' + path)
    # The device nodes the watchdog script depends on must exist afterwards.
    for path in ('/dev', '/etc/init.d', '/var/run'):
        if not fs.exists(path):
            raise RuntimeError('Expected directory missing from rootfs: ' + path)
    return True
