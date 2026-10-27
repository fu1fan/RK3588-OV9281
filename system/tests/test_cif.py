"""Compile the actual driver functions with host-side V4L2/allocator stubs.

This tests enum inputs, all-device format fallback and allocation sizes; it
is not a replacement for a kernel build or DMA/streaming tests on hardware.
"""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]

def function(source, name):
    start = source.index('static int ' + name + '(')
    end = source.index('\n}\n', start) + 3
    return source[start:end]

STUBS = r'''
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <linux/media-bus-format.h>
typedef uint32_t u32;
typedef uint64_t u64;
#define U32_MAX UINT32_MAX
#define PAGE_SIZE 4096
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define V4L2_SUBDEV_FORMAT_ACTIVE 1
struct v4l2_subdev { int mode; u32 width, height, code; };
struct v4l2_subdev_pad_config { int unused; };
struct v4l2_subdev_frame_interval_enum {
 u32 index,pad,which,code,width,height,interval;
};
struct v4l2_subdev_format {
 u32 pad,which; struct { u32 width,height,code; } format;
};
static const struct { u32 bus_fmt,width,height,max_fps; } supported_modes[] = {
 {MEDIA_BUS_FMT_Y10_1X10,1280,800,120},
 {MEDIA_BUS_FMT_Y10_1X10,640,400,240}
};
struct rkcif_dummy_buffer { u32 size; bool is_need_vaddr,is_need_dbuf; void *vaddr; };
struct rkcif_hw { int dev_num; struct rkcif_device *cif_dev[2]; struct rkcif_dummy_buffer dummy_buf; };
struct rkcif_device { struct rkcif_hw *hw_dev; struct { struct v4l2_subdev *sd; } terminal_sensor; int v4l2_dev; };
struct rkcif_stream { struct rkcif_device *cifdev; };
static int allocations, allocation_error;
static int rkcif_alloc_buffer(struct rkcif_device *dev, struct rkcif_dummy_buffer *buf) {
 (void)dev; assert(buf->size); allocations++; return allocation_error;
}
#define v4l2_err(...) ((void)0)
#define v4l2_info(...) ((void)0)
'''
DISPATCH = r'''
static int fake_enum_frame_interval(struct v4l2_subdev *sd, void *cfg,
 struct v4l2_subdev_frame_interval_enum *fie) {
 if (sd->mode) return -EINVAL;
 return ov9281_enum_frame_interval(sd,cfg,fie);
}
static int fake_get_fmt(struct v4l2_subdev *sd, void *cfg, struct v4l2_subdev_format *fmt) {
 (void)cfg; assert(fmt->pad==0); assert(fmt->which==V4L2_SUBDEV_FORMAT_ACTIVE);
 if(sd->mode==2) return -EINVAL;
 fmt->format.width=sd->width; fmt->format.height=sd->height; fmt->format.code=sd->code;
 return 0;
}
#define v4l2_subdev_call(sd,group,op,cfg,arg) fake_##op(sd,cfg,arg)
'''
CASES = r'''
int main(void) {
 struct v4l2_subdev_frame_interval_enum fie={0};
 assert(ov9281_enum_frame_interval(NULL,NULL,&fie)==0);
 assert(fie.width==1280 && fie.height==800 && fie.code==MEDIA_BUS_FMT_Y10_1X10);
 fie.code=MEDIA_BUS_FMT_FIXED;
 assert(ov9281_enum_frame_interval(NULL,NULL,&fie)==-EINVAL);
 fie.code=MEDIA_BUS_FMT_Y10_1X10;
 assert(ov9281_enum_frame_interval(NULL,NULL,&fie)==0);
 fie.index=2;
 assert(ov9281_enum_frame_interval(NULL,NULL,&fie)==-EINVAL);
 struct v4l2_subdev sensors[2]={{0,1280,800,MEDIA_BUS_FMT_Y10_1X10},
                              {1,1920,1080,MEDIA_BUS_FMT_RGB888_1X24}};
 struct rkcif_hw hw={.dev_num=2};
 struct rkcif_device devices[2]={{.hw_dev=&hw,.terminal_sensor={&sensors[0]}},
                                {.hw_dev=&hw,.terminal_sensor={&sensors[1]}}};
 hw.cif_dev[0]=&devices[0]; hw.cif_dev[1]=&devices[1];
 struct rkcif_stream stream={.cifdev=&devices[0]};
 assert(rkcif_create_dummy_buf(&stream)==0);
 assert(hw.dummy_buf.size==1920*1080*3 && allocations==1);
 sensors[0].mode=2; sensors[1].mode=2; allocations=0;
 assert(rkcif_create_dummy_buf(&stream)==-EINVAL && allocations==0);
 sensors[1].mode=1; sensors[1].width=UINT32_MAX;
 assert(rkcif_create_dummy_buf(&stream)==-EOVERFLOW && allocations==0);
 sensors[1].width=1920; allocation_error=-ENOMEM;
 assert(rkcif_create_dummy_buf(&stream)==-ENOMEM && hw.dummy_buf.size==0);
 u32 size=0;
 assert(rkcif_dummy_size(1280,800,MEDIA_BUS_FMT_Y10_1X10,&size)==0 && size==2048000);
 assert(rkcif_dummy_size(1280,800,MEDIA_BUS_FMT_RGB888_1X24,&size)==0 && size==3072000);
 assert(rkcif_dummy_size(0,800,MEDIA_BUS_FMT_Y10_1X10,&size)==-EINVAL);
 return 0;
}
'''

class CifTests(unittest.TestCase):
    def test_real_driver_functions(self):
        sensor = (ROOT / 'kernel/drivers/media/i2c/ov9281.c').read_text()
        capture = (ROOT / 'kernel/drivers/media/platform/rockchip/cif/capture.c').read_text()
        source = (STUBS + function(sensor,'ov9281_enum_frame_interval') + DISPATCH
                  + function(capture,'rkcif_dummy_size')
                  + function(capture,'rkcif_create_dummy_buf') + CASES)
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'cif.c'
            path.write_text(source)
            binary = path.with_suffix('')
            subprocess.run(['cc','-std=gnu99','-Wall','-Wextra','-Werror',
                            '-Wno-unused-parameter','-Wno-sign-compare',
                            '-I'+str(ROOT/'kernel/include/uapi'),
                            str(path),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

if __name__ == '__main__':
    unittest.main()
