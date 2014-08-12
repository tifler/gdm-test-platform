ifeq ($(TARGET_PRODUCT),cm_diamond)

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

MME_TOP := $(LOCAL_PATH)
FFMPEG_TOP := $(TOP)/prebuilts/ffmpeg/ffmpeg-2.1.3

LOCAL_SRC_FILES := \
       adler32.c                                                        \
       aes.c                                                            \
       atomic.c                                                         \
       audio_fifo.c                                                     \
       avstring.c                                                       \
       base64.c                                                         \
       blowfish.c                                                       \
       bprint.c                                                         \
       buffer.c                                                         \
       channel_layout.c                                                 \
       cpu.c                                                            \
       crc.c                                                            \
       des.c                                                            \
       error.c                                                          \
       eval.c                                                           \
       fifo.c                                                           \
       file.c                                                           \
       file_open.c                                                      \
       float_dsp.c                                                      \
       frame.c                                                          \
       hash.c                                                           \
       hmac.c                                                           \
       imgutils.c                                                       \
       intfloat_readwrite.c                                             \
       intmath.c                                                        \
       lfg.c                                                            \
       lls1.c                                                           \
       lls2.c                                                           \
       log.c                                                            \
       log2_tab.c                                                       \
       mathematics.c                                                    \
       md5.c                                                            \
       mem.c                                                            \
       murmur3.c                                                       \
       dict.c                                                           \
       opt.c                                                            \
       parseutils.c                                                     \
       pixdesc.c                                                        \
       random_seed.c                                                    \
       rational.c                                                       \
       rc4.c                                                            \
       ripemd.c                                                         \
       samplefmt.c                                                      \
       sha.c                                                            \
       sha512.c                                                         \
       time.c                                                           \
       timecode.c                                                       \
       tree.c                                                           \
       utils.c                                                          \
       xga_font_data.c                                                  \
       xtea.c                                                           \
       lzo.c                                                            \
       des.c \
       ../compat/strtod.c \

    
#LOCAL_CFLAGS += $(PV_CFLAGS_MINUS_VISIBILITY) -D__OMX_PLATFORM_ANDROID -D__STDINT_MACROS -Wformat
#LOCAL_CFLAGS := -DAAC_PLUS -DHQ_SBR -DPARAMETRICSTEREO -DOSCL_IMPORT_REF= -DOSCL_EXPORT_REF= -DOSCL_UNUSED_ARG=
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
LOCAL_MODULE := libavutil_2.1.3_android
include $(BUILD_STATIC_LIBRARY)

endif

