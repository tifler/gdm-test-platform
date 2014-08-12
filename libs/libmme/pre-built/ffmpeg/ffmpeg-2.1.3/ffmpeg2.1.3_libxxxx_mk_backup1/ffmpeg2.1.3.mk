
FFMPEG_LIBAVUTIL_TOP = $(FFMPEG_TOP)/libavutil
FFMPEG_LIBAVCODEC_TOP = $(FFMPEG_TOP)/libavcodec
FFMPEG_LIBAVFORMAT_TOP = $(FFMPEG_TOP)/libavformat
FFMPEG_LIBAVRESAMPLE_TOP = $(FFMPEG_TOP)/libavresample

include $(FFMPEG_TOP)/ffmpeg2.1.3_libavutil.mk
include $(FFMPEG_TOP)/ffmpeg2.1.3_libavcodec.mk
include $(FFMPEG_TOP)/ffmpeg2.1.3_libavformat.mk
include $(FFMPEG_TOP)/ffmpeg2.1.3_libavresample.mk

FFMPEG_SRCS = $(FFMPEG_SRC_LIBAVUTIL_ARM) $(FFMPEG_SRC_LIBAVUTIL)
FFMPEG_SRCS += $(FFMPEG_SRC_LIBAVCODEC_ARM) $(FFMPEG_SRC_LIBAVCODEC)
FFMPEG_SRCS += $(FFMPEG_SRC_LIBAVFORMAT)
FFMPEG_SRCS += $(FFMPEG_SRC_LIBAVRESAMPLE)

CFLAGS_FFMPEG = -DFF_API_ALLOC_CONTEXT=1 -DFF_API_AVCODEC_OPEN=1
CFLAGS_FFMPEG += -I$(FFMPEG_TOP) 
CFLAGS_FFMPEG += -D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Dstrtod=avpriv_strtod -DPIC                                  \
                -D_DECLARE_C99_LDBL_MATH=1 -std=c99 -fomit-frame-pointer -fPIC -marm -g -Wdeclaration-after-statement        \
                -Wall -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wdisabled-optimization -Wpointer-arith -Wredundant-decls      \
                -Wno-unused-parameter -Wno-sign-compare \
                -Wno-pointer-sign -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes -O3 \
                -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -Werror=vla \

ASFLAGS_FFMPEG = -I$(FFMPEG_TOP) 

LDFLAGS_FFMPEG = 
