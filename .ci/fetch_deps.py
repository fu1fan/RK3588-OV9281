"""Restore SHA256-pinned public SDK dependencies; never execute downloaded code."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import urllib.request


def digest(path):
    with open(path, 'rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest() if hasattr(hashlib, 'file_digest') else stream_hash(f)


def stream_hash(f):
    h = hashlib.sha256()
    for chunk in iter(lambda: f.read(1024 * 1024), b''):
        h.update(chunk)
    return h.hexdigest()


def restore(manifest_path, destination, downloads):
    manifest = json.loads(Path(manifest_path).read_text())
    destination = Path(destination)
    downloads = Path(downloads)
    destination.mkdir(parents=True, exist_ok=True)
    downloads.mkdir(parents=True, exist_ok=True)
    for item in manifest['archives']:
        name = item['file']
        if PurePosixPath(name).name != name:
            raise ValueError('Dependency filename must be a basename')
        marker = destination / ('.' + item['sha256'] + '.ready')
        if marker.exists():
            continue
        archive = downloads / name
        if not archive.exists() or digest(archive) != item['sha256']:
            partial = archive.with_suffix(archive.suffix + '.partial')
            print('Downloading', name, flush=True)
            request = urllib.request.Request(item['download_url'], headers={'User-Agent': 'ELF2-CNB-builder/1'})
            with urllib.request.urlopen(request, timeout=300) as response, partial.open('wb') as out:
                shutil.copyfileobj(response, out, 1024 * 1024)
            if partial.stat().st_size != item['bytes'] or digest(partial) != item['sha256']:
                raise RuntimeError('Dependency checksum mismatch: ' + name)
            partial.replace(archive)
        print('Restoring', name, flush=True)
        subprocess.run(['tar', '-xJf', str(archive), '-C', str(destination)], check=True)
        marker.write_text(item['sha256'] + '\n')
    (destination / '.manifest-sha256').write_text(digest(manifest_path) + '\n')


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--manifest', required=True)
    p.add_argument('--destination', required=True)
    p.add_argument('--downloads', required=True)
    a = p.parse_args()
    restore(a.manifest, a.destination, a.downloads)

