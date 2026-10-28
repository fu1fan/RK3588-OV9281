"""Exercise the actual boot-handoff predicate with incomplete controller clocks."""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
DRIVER = ROOT / 'kernel/drivers/gpu/drm/bridge/synopsys/dw-hdmi-qp.c'


class HdmiHandoffTests(unittest.TestCase):
    def test_ddc_activity_alone_does_not_mean_video_registers_are_accessible(self):
        source = DRIVER.read_text()
        probe = source.split('__dw_hdmi_probe(struct platform_device', 1)[1]
        block = probe.split('hdmi->logo_plug_out = false;', 1)[1]
        predicate = block.split('if (', 1)[1].split(') {', 1)[0]
        mask = re.search(r'^#define HDMI_CTRL_CLK_EN\s+(\S+)', source, re.M)
        # The baseline has no clock mask; compile its original predicate too.
        define = '#define HDMI_CTRL_CLK_EN ' + mask[1] if mask else ''
        code = r'''
#include <assert.h>
#include <stdbool.h>
#define connector_status_connected 1
#define I2CM_INTERFACE_CONTROL0 0x90
#define CMU_STATUS 0xb0
MASK
static int hpd, ddc, clocks;
static int read_hpd(void *hdmi, void *data) { return hpd; }
struct ops { int (*read_hpd)(void *, void *); } ops = { read_hpd };
struct hdmi { struct { struct ops *ops; void *data; } phy; } instance = { { &ops, 0 } };
static unsigned int hdmi_readl(struct hdmi *hdmi, unsigned int reg)
{ return reg == CMU_STATUS ? clocks : ddc; }
static bool initialized(struct hdmi *hdmi) { return PREDICATE; }
int main(void) {
    for (hpd = 0; hpd <= 1; hpd++)
        for (ddc = 0; ddc <= 1; ddc++)
            for (clocks = 0; clocks < 32; clocks++)
                assert(initialized(&instance) ==
                       (hpd && ddc && (clocks & 0x15) == 0x15));
    return 0;
}
'''.replace('MASK', define).replace('PREDICATE', predicate)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / 'handoff.c').write_text(code)
            subprocess.run(['cc', '-Werror', str(path / 'handoff.c'),
                            '-o', str(path / 'handoff')], check=True)
            subprocess.run([str(path / 'handoff')], check=True)


if __name__ == '__main__':
    unittest.main()
