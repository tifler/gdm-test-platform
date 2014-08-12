ifeq ($(TARGET_PRODUCT),cm_diamond)

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

MME_TOP := $(LOCAL_PATH)
FFMPEG_TOP := $(TOP)/prebuilts/ffmpeg/ffmpeg-2.1.3

LOCAL_SRC_FILES := \
                asf.c \
        asfcrypt.c \
        asfdec.c \
        avidec.c avienc.c\
        avio.c \
        aviobuf.c \
        avlanguage.c \
        cutils.c \
        dv.c \
        file.c \
        file_open.c \
        format.c \
        id3v1.c \
        id3v2.c \
        img2.c \
        metadata.c \
        options.c \
        pcm.c \
        riff.c \
        riffdec.c \
        riffenc.c \
        rm.c \
        rmdec.c \
        spdif.c \
        spdifdec.c \
        utils.c \
        w64.c \
        wavdec.c \
        wavenc.c \
        wvdec.c \
        wvenc.c \
        allformats.c \
        mov.c mov_chan.c isom.c flvdec.c \
        matroska.c matroskadec.c \
        mp3dec.c rmsipr.c mpeg.c mpegts.c subtitles.c\
        oggdec.c oggparsecelt.c oggparsedirac.c oggparseflac.c oggparseogm.c oggparseopus.c oggparseskeleton.c oggparsespeex.c oggparsetheora.c oggparsevorbis.c\
        vorbiscomment.c\
        flac_picture.c \
        mpegvideodec.c rawdec.c \
        ape.c apetag.c \
        aacdec.c \
        mux.c
        


    
LOCAL_CFLAGS += -D__OMX_PLATFORM_ANDROID             \
                -D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Dstrtod=avpriv_strtod -DPIC                                  \
                -D_DECLARE_C99_LDBL_MATH=1 -std=c99 -fomit-frame-pointer -fPIC -marm -g -Wdeclaration-after-statement        \
                -Wall -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wdisabled-optimization -Wpointer-arith -Wredundant-decls      \
                -Wno-unused-parameter -Wno-sign-compare \
                -Wno-pointer-sign -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes -O3 \
                -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -Werror=vla \

LOCAL_C_INCLUDES:= \
      $(FFMPEG_TOP)

LOCAL_ARM_MODE := arm        
LOCAL_MODULE := libavformat_2.1.3_android
include $(BUILD_STATIC_LIBRARY)

endif

