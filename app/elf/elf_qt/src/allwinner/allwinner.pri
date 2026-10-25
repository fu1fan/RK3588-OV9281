#系统依赖库和头文件
LIBSDK_PATH =/home/elf/work/OKT507-linux-sdk/platform/framework/auto/sdk_lib
LIBSDK_LIB_PATH = $$LIBSDK_PATH/lib64/
INCLUDEPATH += 	$$LIBSDK_PATH \
                                        $$LIBSDK_PATH/eve_module/include \
                                        $$LIBSDK_PATH/eve_module \
                                $$LIBSDK_PATH/include \
                                $$LIBSDK_PATH/include/disp2 \
                                $$LIBSDK_PATH/media  \
                                        $$LIBSDK_PATH/include/camera \
                                        $$LIBSDK_PATH/sdk_misc \
                                $$LIBSDK_PATH/include/utils ./ ./src/ \
                                ../../../external-packages/gpu/mali400/fbdev/header_files/include/  \
                                        ../../../../tools/pack/chips/$$(LICHEE_CHIP)/hal/gpu/fbdev/include  \
                                Commons \
                                        $$LIBSDK_PATH/include/storage/ \
                                $$LIBSDK_PATH/storage/ \
                                $$LIBSDK_PATH/sound/ \
                                $$LIBSDK_PATH/misc/ \
                                $$LIBSDK_PATH/camera/allwinnertech/include \
                                $$LIBSDK_PATH/camera/allwinnertech/water_mark/ \
                                $$LIBSDK_PATH/camera/adas/ $$LIBSDK_PATH/storage/ \
                                $$LIBSDK_PATH/sound/ \
                                $$LIBSDK_PATH/memory/ \
                                        $$LIBSDK_PATH/audioenc/ \
                                $$LIBSDK_PATH/cedarx/include \
                                $$LIBSDK_PATH/cedarx/include/xplayer \
                                $$LIBSDK_PATH/cedarx/include/xplayer/include \
                                $$LIBSDK_PATH/cedarx/include/xmetadataretriever/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/common/plugin/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/common/iniparser/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/base/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/bd/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/hls/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/playlist/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/wav/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mpg/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/aiff/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/id3v2/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/sstr/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/flv/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/dash/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/ogg/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/base/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/env/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/awts/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mp3/	\
                                        $$LIBSDK_PATH/cedarx/include/libcore/parser/mms	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/flac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/AwSpecialStream/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mmshttp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/dsd/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/m3u9/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/ape/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/g729/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/pmp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/aac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mkv/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/asf/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/atrac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/AwRawStream/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/remux/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/amr/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/avi/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/ts/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/caf/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mov/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/base/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/aac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/mp4/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/ts/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/playback/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/playback/include	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/aes/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/mms/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/http/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/dtmb/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/rtsp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/rtmp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/udp/	\
                                $$LIBSDK_PATH/cedarx/include/libcedarc/include/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/live/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/alsa/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/alsa/sound/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/openssl/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/libxml/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/adecoder/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/aencoder/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/zlib/
#DEFINES += QT_ROTATE_APP

DEPENDPATH += $$LIBSDK_PATH $$LIBSDK_PATH/include \
                $$LIBSDK_PATH/include/disp2 \
                $$LIBSDK_PATH/media  \
                $$LIBSDK_PATH/camera \
                $$LIBSDK_PATH/memory \
                $$LIBSDK_PATH/include/utils . ./src/ \
                ../../../external-packages/gpu/mali400/fbdev/header_files/include/  \
                ../../../../tools/pack/chips/$$(LICHEE_CHIP)/hal/gpu/fbdev/include \
                $$LIBSDK_PATH/storage/ \
                $$LIBSDK_PATH/sound/  \
                $$LIBSDK_PATH/misc/ \
                                $$LIBSDK_PATH/cedarx/include \
                                $$LIBSDK_PATH/cedarx/include/xplayer \
                                $$LIBSDK_PATH/cedarx/include/xplayer/include \
                                $$LIBSDK_PATH/cedarx/include/xmetadataretriever/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/common/plugin/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/common/iniparser/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/base/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/bd/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/hls/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/playlist/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/wav/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mpg/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/aiff/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/id3v2/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/sstr/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/flv/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/dash/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/ogg/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/base/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/env/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/awts/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mp3/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mms	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/flac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/AwSpecialStream/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mmshttp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/dsd/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/m3u9/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/ape/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/g729/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/pmp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/aac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mkv/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/asf/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/atrac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/AwRawStream/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/remux/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/amr/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/avi/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/ts/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/caf/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/parser/mov/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/base/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/aac/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/mp4/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/muxer/ts/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/playback/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/playback/include	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/include/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/aes/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/mms/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/http/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/dtmb/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/rtsp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/rtmp/	\
                                $$LIBSDK_PATH/cedarx/include/libcore/stream/udp/	\
                                $$LIBSDK_PATH/cedarx/include/libcedarc/include/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/live/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/alsa/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/alsa/sound/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/openssl/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/libxml/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/adecoder/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/aencoder/	\
                                $$LIBSDK_PATH/cedarx/include/external/include/zlib/

#DEFINES += QT_ROTATE_APP
DEFINES += CDX_V27 HAVE_PTHREADS SUPPORT_NEW_DRIVER HAVE_SYS_UIO_H ANDROID_SMP=1 __ARM_HAVE_DMB __ARM_HAVE_LDREX_STREX __ARM_HAVE_LDREXD \
                                __ARM_HAVE_VFP __ARM_HAVE_NEON OPTION_CC_GNUEABIHF=1 OPTION_CC_GNUEABI=2 OPTION_CC_UCGNUEABI=3 \
           _GNU_SOURCE OPTION_CC_GNUEABIHF=1 OPTION_CC_GNUEABI=2 OPTION_CC_UCGNUEABI=3 OPTION_CC_LINUX_UCGNUEABI=4 OPTION_CC_LINUX_MUSLGNUEABI=5 OPTION_CC_LINUX_MUSLGNUEABI64=6 OPTION_OS_ANDROID=1 OPTION_OS_LINUX=2 OPTION_OS_VERSION_ANDROID_4_2=1 OPTION_OS_VERSION_ANDROID_4_4=2 OPTION_OS_VERSION_ANDROID_5_0=3 OPTION_OS_VERSION_ANDROID_6_0=4 OPTION_MEMORY_DRIVER_SUNXIMEM=1 OPTION_MEMORY_DRIVER_ION=2 OPTION_PRODUCT_PAD=1 OPTION_PRODUCT_TVBOX=2 OPTION_PRODUCT_OTT_CMCC=3 OPTION_PRODUCT_IPTV=4 OPTION_PRODUCT_DVB=5 OPTION_PRODUCT_LOUDSPEAKER=6 OPTION_CHIP_R8=1 OPTION_CHIP_R16=2 OPTION_CHIP_C500=3 OPTION_CHIP_R58=4 OPTION_CHIP_R18=5 OPTION_CHIP_T2=6 OPTION_CHIP_T3=7 OPTION_DRAM_INTERFACE_DDR1_16BITS=1 OPTION_DRAM_INTERFACE_DDR1_32BITS=2 OPTION_DRAM_INTERFACE_DDR2_16BITS=3 OPTION_DRAM_INTERFACE_DDR2_32BITS=4 OPTION_DRAM_INTERFACE_DDR3_16BITS=5 OPTION_DRAM_INTERFACE_DDR3_32BITS=6 OPTION_DRAM_INTERFACE_DDR3_64BITS=7 OPTION_CMCC_NO=0 OPTION_CMCC_YES=1 OPTION_DTV_NO=0 OPTION_DTV_YES=1 OPTION_ALI_YUNOS_NO=0 OPTION_ALI_YUNOS_YES=1 OPTION_IS_CAMERA_DECODER_NO=0 OPTION_IS_CAMERA_DECODER_YES=1 OPTION_VE_IPC_DISABLE=1 OPTION_VE_IPC_ENABLE=2 OPTION_NO_DEINTERLACE=0 OPTION_HW_DEINTERLACE=1 OPTION_SW_DEINTERLACE=2 LINUX_VERSION_3_4=1 LINUX_VERSION_3_10=2 DROP_DELAY_FRAME_NONE=0 DROP_DELAY_FRAME_720P=1 DROP_DELAY_FRAME_4K=2 ZEROCOPY_PIXEL_FORMAT_NONE=0 ZEROCOPY_PIXEL_FORMAT_YUV_MB32_420=1 ZEROCOPY_PIXEL_FORMAT_YV12=2 ZEROCOPY_PIXEL_FORMAT_NV21=3 GPU_Y16_C16_ALIGN=1 GPU_Y32_C16_ALIGN=2 GPU_Y16_C8_ALIGN=3 ZEROCOPY_HAL_PIXEL_FORMAT_AW_YUV_PLANNER420=1 ZEROCOPY_HAL_PIXEL_FORMAT_AW_YVU_PLANNER420=2 DEINTERLACE_FORMAT_MB32_12=1 DEINTERLACE_FORMAT_NV=2 DEINTERLACE_FORMAT_NV21=3 DEINTERLACE_FORMAT_NV12=4 OUTPUT_PIXEL_FORMAT_NV21=1 OUTPUT_PIXEL_FORMAT_YV12=2 OUTPUT_PIXEL_FORMAT_MB32=3 GPU_ALIGN_STRIDE_16=1 GPU_ALIGN_STRIDE_32=2 OPTION_HAVE_ZLIB=1 OPTION_NO_ZLIB=2 OPTION_HAVE_SSL=1 OPTION_NO_SSL=2 OPTION_HAVE_LIVE555=1 OPTION_NO_LIVE555=2 CONFIG_CC=4 CONFIG_OS=2 CONFIG_CHIP=7 CONFIG_ALI_YUNOS=0 CONFIG_OS_VERSION=2 CONFIG_TARGET_PRODUCT= CONFIG_PRODUCT=2 CONFIG_HAVE_ZLIB=1 CONFIG_HAVE_SSL=1 CONFIG_HAVE_LIVE555=2 CONFIG_MEMORY_DRIVER=2 CONFIG_DRAM_INTERFACE=6 CONFIG_VE_IPC=1 CONFIG_CMCC=0 CONFIG_DTV=0 CONFIG_IS_CAMERA_DECODER=0 CONFIG_DEINTERLACE=0 GPU_TYPE_MALI=1 CONFIG_ARM_VERSION= ENABLE_SUBTITLE_DISPLAY_IN_CEDARX=0 DTV=no LINUX_VERSION=2 USE_NEW_BDMV_STREAM=0 PLAYREADY_DRM_INVOKE=0 H265_4K_CHECK_SCALE_DOWN=1 NON_H265_4K_NOT_SCALE_DOWN=0 SUPPORT_H265=0 ANTUTU_NOT_SUPPORT=1 ENABLE_MEDIA_BOOST=1 DROP_DELAY_FRAME=2 ROTATE_PIC_HW=0 VE_PHY_OFFSET=0x40000000 ZEROCOPY_PIXEL_FORMAT=3 GPU_Y_C_ALIGN=1 SEND_3_BLACK_FRAME_TO_GPU=0 ZEROCOPY_DYNAMIC_CHECK=1 GRALLOC_PRIV=0 VIDEO_DIRECT_ACCESS_DE=0 KEY_PARAMETER_GET=0 DISPLAY_CMD_SETVIDEOSIZE_POSITION=0 DEINTERLACE_IOWR=0 DEINTERLACE_FORMAT=4 NEW_DISPLAY_DOUBLE_STREAM_NEED_NV21=1 OUTPUT_PIXEL_FORMAT=1 NOT_DROP_FRAME=1 SOUND_DEVICE_SET_RAW_FLAG=0 NATIVE_WIN_DISPLAY_CMD_GETDISPFPS=0 IMG_NV21_4K_ALIGN=0 DEINTERLACE_ADDR_64=0


#unix:!macx: LIBS += -L$$PWD/$$LIBSDK_PATH/ -L$$PWD/$$LIBSDK_LIB_PATH -lsdk_func \
unix:!macx: LIBS += -L$$LIBSDK_PATH/  -L$$LIBSDK_LIB_PATH -lsdk_compose  -lsdk_disp  -lsdk_dvr  -lsdk_g2d -lsdk_player \
                                -lsdk_log -lsdk_camera -lsdk_memory -lsdk_sound -lsdk_storage -lsdk_audenc -lsdk_cfg -lsdk_ctrl -lsdk_egl -lsdk_misc  \
                                        -L../../../../../tools/pack/chips/$$(LICHEE_CHIP)/hal/gpu/fbdev/lib/ -lGLESv2 \
                                        -L$$LIBSDK_PATH/cedarx/lib/ -Wl,--no-undefined   -lcdx_playback -lcdx_common -laencoder -lawrecorder -lxplayer   -lcdx_muxer \
             -lcdx_parser -lsubdecoder -lvideoengine -lvdecoder -lcdx_stream   -lvencoder -lVE   -lcdc_base -lMemAdapter -ladecoder -laencoder -lxmetadataretriever -lcdx_base \
             -lrt -lpthread -lz -lcrypto -lssl -lvenc_codec -lvenc_base -lcdx_ion -laudioplayer
