"""Publish checked artifacts with the pipeline token; never delete releases."""
import argparse
import json
import os
from pathlib import Path
import re
import time
import urllib.error
import urllib.parse
import urllib.request

from fetch_deps import digest

ROOT = Path(__file__).resolve().parents[1]
DIST = ROOT/'dist'


class APIError(RuntimeError):
    def __init__(self, code):
        self.code = code
        super().__init__('CNB API returned HTTP '+str(code))


class API:
    def __init__(self):
        self.token = os.environ['CNB_TOKEN']
        self.repo = os.environ['CNB_REPO_SLUG']
        self.root = 'https://api.cnb.cool/' + self.repo + '/-'

    def request(self, method, path, body=None):
        url = path if path.startswith('https://') else self.root+path
        if urllib.parse.urlparse(url).netloc != 'api.cnb.cool':
            raise RuntimeError('Refusing to send pipeline token to another host')
        data = json.dumps(body).encode() if body is not None else (b'' if method=='POST' else None)
        request = urllib.request.Request(url, data=data, method=method,
            headers={'Accept':'application/vnd.cnb.api+json','Authorization':'Bearer '+self.token,'Content-Type':'application/json','User-Agent':'ELF2-CNB-builder/1'})
        try:
            with urllib.request.urlopen(request, timeout=90) as response:
                data=response.read()
                return json.loads(data) if data else None
        except urllib.error.HTTPError as e:
            raise APIError(e.code) from None

    def assets(self, binding):
        if binding.startswith('/git/'):
            return self.request('GET',binding)
        return self.request('GET',binding).get('assets',[])

    def upload(self, binding, path):
        existing=[a for a in self.assets(binding) if a['name']==path.name]
        expected=digest(path)
        if existing:
            a=existing[0]
            if a.get('size_in_byte',a.get('size'))==path.stat().st_size and a.get('hash_value',a.get('digest'))==expected:
                print('Identical asset already exists:',path.name,flush=True)
                return
            raise RuntimeError('An existing asset has different content; use a new tag: '+path.name)
        grant=self.request('POST',binding+'/asset-upload-url',{'asset_name':path.name,'size':path.stat().st_size,'ttl':0})
        url=grant['upload_url']
        if urllib.parse.urlparse(url).scheme!='https':
            raise RuntimeError('Upload URL must use HTTPS')
        print('Uploading',path.name,path.stat().st_size,flush=True)
        with path.open('rb') as f:
            req=urllib.request.Request(url,data=f,method='PUT',headers={'Content-Length':str(path.stat().st_size),'Content-Type':'application/octet-stream'})
            with urllib.request.urlopen(req,timeout=900) as response:
                response.read()
        for attempt in range(3):
            try:
                self.request('POST',grant['verify_url'])
                break
            except APIError:
                if attempt==2:raise
                time.sleep(3*(attempt+1))
        asset=next(a for a in self.assets(binding) if a['name']==path.name)
        if asset.get('size_in_byte',asset.get('size'))!=path.stat().st_size or asset.get('hash_value',asset.get('digest'))!=expected:
            raise RuntimeError('Remote asset checksum mismatch: '+path.name)
        print('Remote SHA256 verified:',path.name,flush=True)


def publish(failure=False):
    if not DIST.is_dir():
        if failure:return
        raise RuntimeError('No build output')
    infos=list(DIST.glob('*-build-info.json'))
    if len(infos)!=1:
        raise RuntimeError('Expected one build-info file')
    info=json.loads(infos[0].read_text())
    api=API()
    commit=info['commit']
    if commit != os.environ.get('CNB_COMMIT',commit):
        raise RuntimeError('Artifact commit does not match pipeline commit')
    binding='/git/commit-assets/'+commit
    destination='commit'
    if failure:
        files=[infos[0],*DIST.glob('*-build.log.xz')]
    else:
        if info['status']!='verified':
            raise RuntimeError('Build did not pass verification')
        sums=list(DIST.glob('*-SHA256SUMS'))
        if len(sums)!=1:raise RuntimeError('Missing SHA256SUMS')
        files=[]
        for line in sums[0].read_text().splitlines():
            expected,name=line.split('  ',1)
            if Path(name).name!=name:raise RuntimeError('Unsafe artifact filename')
            p=DIST/name
            if digest(p)!=expected:raise RuntimeError('Local artifact checksum mismatch: '+name)
            files.append(p)
        files.append(sums[0])
        if os.environ.get('CNB_IS_TAG','').lower()=='true':
            tag=info['tag']
            try:
                releases=api.request('GET','/releases?page=1&page_size=100')
                release=next((r for r in releases if r['tag_name']==tag),None)
                if release:
                    if release.get('tag_commitish', release.get('target_commitish')) not in (commit,tag):
                        raise RuntimeError('Existing release points to a different source')
                else:
                    body=next(DIST.glob('*-RELEASE.md')).read_text()
                    release=api.request('POST','/releases',{'tag_name':tag,'target_commitish':commit,
                        'name':tag+' ELF2 system image','body':body,'prerelease':True,'make_latest':'false'})
                binding='/releases/'+str(release['id'])
                destination='release'
            except APIError as e:
                if e.code!=403:raise
                print('Release permission unavailable; publishing fixed commit attachments instead.',flush=True)
    for p in files:api.upload(binding,p)
    (DIST/'publication.json').write_text(json.dumps({'destination':destination,'binding':binding,'commit':commit,
        'files':[p.name for p in files]},indent=2)+'\n')
    print('PUBLICATION_COMPLETE',destination,commit,flush=True)


if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--failure',action='store_true')
    publish(p.parse_args().failure)
