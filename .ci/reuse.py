"""Optional same-node second pass, so cache reuse can be measured explicitly."""
import json
import os
from pathlib import Path
import subprocess
import sys

from fetch_deps import digest

ROOT=Path(__file__).resolve().parents[1]


def probe():
    requested=os.environ.get('ELF2_VERIFY_REUSE')=='1' or '[verify-reuse]' in os.environ.get('CNB_TAG_MESSAGE','')
    if not requested:
        print('Reuse probe not requested; ordinary tags build once.',flush=True)
        return
    dist=ROOT/'dist'
    first=json.loads(next(dist.glob('*-build-info.json')).read_text())
    if first['status']!='verified':raise RuntimeError('First build is not verified')
    subprocess.run([sys.executable,str(ROOT/'.ci/build.py'),'--finish'],cwd=ROOT,check=True)
    subprocess.run([sys.executable,str(ROOT/'.ci/build.py')],cwd=ROOT,check=True)
    second_path=next(dist.glob('*-build-info.json'))
    second=json.loads(second_path.read_text())
    if not second['cache_hit'] or second['reused_object_files']==0 or first['cache_key']!=second['cache_key'] or first['commit']!=second['commit']:
        raise RuntimeError('Second build did not reuse compatible intermediate objects')
    report={'commit':second['commit'],'parallel_jobs':second['parallel_jobs'],
            'first_seconds':first['elapsed_seconds'],'second_seconds':second['elapsed_seconds'],
            'first_cache_hit':first['cache_hit'],'second_cache_hit':second['cache_hit'],
            'reused_objects':second['reused_object_files'],'total_objects':second['object_files'],
            'second_ccache_statistics':second['ccache_statistics'],'same_node_same_workspace':True}
    second['reuse_probe']=report
    second_path.write_text(json.dumps(second,indent=2)+'\n')
    prefix=second_path.name.removesuffix('-build-info.json')
    (dist/(prefix+'-reuse-validation.json')).write_text(json.dumps(report,indent=2)+'\n')
    sums=next(dist.glob('*-SHA256SUMS'))
    sums.write_text(''.join(digest(p)+'  '+p.name+'\n' for p in sorted(dist.iterdir()) if p.is_file() and p!=sums))
    print('REUSE_PROBE_VERIFIED',json.dumps(report),flush=True)


if __name__=='__main__':probe()
