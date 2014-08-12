ifeq ($(TARGET_PRODUCT),cm_diamond)

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

MME_TOP := $(LOCAL_PATH)
FFMPEG_TOP := $(TOP)/prebuilts/ffmpeg/ffmpeg-2.1.3

LOCAL_SRC_FILES := \
                   cpu.c \
                   float_dsp_init_arm.c float_dsp_init_neon.c float_dsp_init_vfp.c float_dsp_neon.S float_dsp_vfp.S
                   
		    
LOCAL_CFLAGS += -D__OMX_PLATFORM_ANDROID             \
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
LOCAL_MODULE := libavutil_arm_2.1.3_android
include $(BUILD_STATIC_LIBRARY)

endif

