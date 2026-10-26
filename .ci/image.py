"""Edit an unmounted ext4 copy with debugfs, preserving root ownership/modes."""
import os
from pathlib import Path
import re
import stat
import subprocess


def quoted(value):
    value = str(value)
    if '\n' in value or '\r' in value:
        raise ValueError('Newline in filesystem path')
    return '"' + value.replace('\\', '\\\\').replace('"', '\\"') + '"'


class Ext4:
    def __init__(self, image):
        self.image = Path(image)

    def run(self, command):
        p = subprocess.run(['debugfs', '-R', command, str(self.image)], capture_output=True, text=True, check=True)
        return p.stdout

    def entries(self, path):
        result = []
        for line in self.run('ls -p ' + quoted(path)).splitlines():
            parts = line.split('/')
            if len(parts) >= 7 and parts[1].isdigit() and parts[5] not in ('.', '..'):
                result.append((parts[5], int(parts[2], 8)))
        return result

    def exists(self, path):
        return 'Inode:' in self.run('stat ' + quoted(path))

    def remove_commands(self, path):
        commands = []
        for name, mode in self.entries(path):
            child = path.rstrip('/') + '/' + name
            if stat.S_ISDIR(mode):
                commands.extend(self.remove_commands(child))
                commands.append('rmdir ' + quoted(child))
            else:
                commands.append('rm ' + quoted(child))
        return commands

    def apply(self, commands, script):
        Path(script).write_text('\n'.join(commands) + '\n')
        p = subprocess.run(['debugfs', '-w', '-f', str(script), str(self.image)],
                           capture_output=True, text=True, check=True)
        bad = re.compile(r'File not found|already exists|Could not|Usage:|not a directory|No free|while ', re.I)
        errors = [l for l in p.stderr.splitlines() if bad.search(l)]
        if errors:
            raise RuntimeError('debugfs failed: ' + '\n'.join(errors[:10]))


def install(image, modules, overlay_files, script):
    fs = Ext4(image)
    commands = []
    if fs.exists('/lib/modules'):
        commands.extend(fs.remove_commands('/lib/modules'))
    else:
        commands.append('mkdir /lib/modules')
    if commands:
        fs.apply(commands, str(script) + '.remove')
    commands = []
    known = {'/', '/lib', '/lib/modules'}

    def parent_dirs(path):
        parent = Path(path).parent
        for item in reversed([parent, *parent.parents]):
            name = item.as_posix()
            if name == '.' or name in known:
                continue
            if not fs.exists(name):
                commands.append('mkdir ' + quoted(name))
            known.add(name)

    files = []
    for p in sorted(Path(modules).rglob('*')):
        if p.is_file() and not p.is_symlink():
            files.append((p, '/lib/modules/' + p.relative_to(modules).as_posix(), None))
    for entry in overlay_files:
        # (source, target) keeps the source mode; (source, target, mode) forces one.
        src, target = entry[0], entry[1]
        files.append((src, target, entry[2] if len(entry) > 2 else None))
    for src, target, mode in files:
        parent_dirs(target)
        if fs.exists(target):
            commands.append('rm ' + quoted(target))
        commands.append('write ' + quoted(src) + ' ' + quoted(target))
        if mode is None:
            mode = Path(src).stat().st_mode & 0o777
        commands.append('set_inode_field ' + quoted(target) + ' mode 0' + format(stat.S_IFREG | mode, 'o'))
        commands.append('set_inode_field ' + quoted(target) + ' uid 0')
        commands.append('set_inode_field ' + quoted(target) + ' gid 0')
    fs.apply(commands, script)
    subprocess.run(['e2fsck', '-fn', str(image)], check=True)
    return fs
