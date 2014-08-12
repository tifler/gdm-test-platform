ifeq ($(TARGET_PRODUCT),cm_diamond)

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

MME_TOP := $(LOCAL_PATH)
FFMPEG_TOP := $(TOP)/prebuilts/ffmpeg/ffmpeg-2.1.3

LOCAL_SRC_FILES := \
                   dsputil_init_arm.c     \
                   dsputil_init_armv5te.c     \
                   dsputil_init_armv6.c     \
                   dsputil_arm.S          \
                   dsputil_armv6.S          \
                   dsputil_init_neon.c     \
                   dsputil_neon.S     \
                   int_neon.S     \
                   jrevdct_arm.S          \
                   simple_idct_arm.S      \
                   simple_idct_armv5te.S      \
                   simple_idct_armv6.S      \
                   simple_idct_neon.S      \
                   videodsp_init_arm.c    \
                   videodsp_init_armv5te.c    \
                   videodsp_armv5te.S    \
                   hpeldsp_init_arm.c     \
                   hpeldsp_init_armv6.c     \
                   hpeldsp_init_neon.c     \
                   hpeldsp_arm.S          \
                   hpeldsp_armv6.S          \
                   hpeldsp_neon.S         \
                   h264chroma_init_arm.c  \
				   h264dsp_init_arm.c     \
				   h264pred_init_arm.c    \
                   h264qpel_init_arm.c    \
                   h264qpel_neon.S         \
                   h264dsp_armv6.S     \
				   h264dsp_neon.S     \
				   h264cmc_neon.S  \
                   h264idct_neon.S \
                   h264pred_neon.S \
                   h264qpel_neon.S \
                   h264qpel_neon.S \
                   mpegvideo_arm.c \
                   mpegvideo_armv5te.c \
                   mpegvideo_armv5te_s.S \
                   mpegvideo_neon.S      \
                   mpegaudiodsp_init_arm.c\
                   fft_init_arm.c mpegaudiodsp_fixed_armv6.S mdct_vfp.S fft_vfp.S fft_neon.S\
                   fft_fixed_init_arm.c fft_fixed_neon.S\
                   rdft_neon.S\
                   fmtconvert_init_arm.c fmtconvert_neon.S fmtconvert_vfp.S fmtconvert_vfp_armv6.S \
                   mdct_fixed_neon.S mdct_neon.S \
                   mpegaudiodsp_fixed_armv6.S  \
                   ac3dsp_arm.S ac3dsp_armv6.S ac3dsp_init_arm.c ac3dsp_neon.S\
                   aacpsdsp_init_arm.c aacpsdsp_neon.S sbrdsp_init_arm.c sbrdsp_neon.S \
                   flacdsp_arm.S flacdsp_init_arm.c\
                   vp3dsp_init_arm.c vp3dsp_neon.S vp6dsp_init_arm.c vp6dsp_neon.S \
                   vp8_armv6.S vp8dsp_armv6.S vp8dsp_init_arm.c vp8dsp_init_armv6.c vp8dsp_init_neon.c vp8dsp_neon.S\
                   rv34dsp_init_arm.c rv34dsp_neon.S rv40dsp_init_arm.c rv40dsp_neon.S\
                   vorbisdsp_init_arm.c vorbisdsp_neon.S
                   
		    
LOCAL_CFLAGS += -D__OMX_PLATFORM_ANDROID            \
                -D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Dstrtod=avpriv_strtod -DPIC                                  \
                -D_DECLARE_C99_LDBL_MATH=1 -std=c99 -fomit-frame-pointer -fPIC -marm -g -Wdeclaration-after-statement        \
                -Wall -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wdisabled-optimization -Wpointer-arith -Wredundant-decls      \
                -Wno-unused-parameter -Wno-sign-compare \
                -Wno-pointer-sign -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes -O3 \
                -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -Werror=vla \

LOCAL_ASFLAGS += -D__OMX_PLATFORM_ANDROID             \
                 -D_DECLARE_C99_LDBL_MATH=1 -fPIC -g

LOCAL_C_INCLUDES:= \
      $(FFMPEG_TOP)

LOCAL_ARM_MODE := arm        
LOCAL_MODULE := libavcodec_arm_2.1.3_android
include $(BUILD_STATIC_LIBRARY)

endif

