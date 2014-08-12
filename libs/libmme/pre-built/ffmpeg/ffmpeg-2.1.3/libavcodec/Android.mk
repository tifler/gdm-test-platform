LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MME_TOP := $(LOCAL_PATH)
FFMPEG_TOP := $(TOP)/prebuilts/ffmpeg/ffmpeg-2.1.3

LOCAL_SRC_FILES := \
                ac3.c \
        ac3_parser.c \
        ac3dec.c \
        ac3dec_data.c \
        ac3dsp.c \
        ac3tab.c \
        acelp_filters.c \
        acelp_pitch_delay.c \
        acelp_vectors.c \
        audioconvert.c \
        avpacket.c \
        avpicture.c \
        bethsoftvideo.c \
        bitstream.c \
        cabac.c \
        cavs.c \
        cavsdsp.c \
        celp_filters.c \
        celp_math.c \
        codec_desc.c \
        cook.c \
        cook_parser.c \
        dct.c \
        dct32_fixed.c \
        dct32_float.c \
        dsputil.c \
        dv_profile.c \
        eac3_data.c \
        eac3dec.c \
        error_resilience.c \
        faandct.c \
        faanidct.c \
        fft.c \
        fmtconvert.c \
        huffman.c\
        frame_thread_encoder.c \
        golomb.c \
        hpeldsp.c \
        imgconvert.c \
        jfdctfst.c \
        jfdctint.c \
        jrevdct.c \
        kbdwin.c \
        lsp.c \
        mathtables.c \
        mdct.c \
        motionpixels_tablegen.c \
        mpeg4audio.c \
        mpegaudio.c \
        mpegaudiodata.c \
        mpegaudiodec.c \
        mpegaudiodecheader.c \
        mpegaudiodsp.c \
        mpegaudiodsp_data.c \
        mpegaudiodsp_fixed.c \
        mpegaudiodsp_float.c \
        options.c \
        parser.c \
        pthread.c \
        qcelpdec.c \
        ra144.c \
        ra144dec.c \
        ra288.c \
        ratecontrol.c \
        raw.c \
        rawdec.c \
        rdft.c \
        resample.c \
        resample2.c \
        simple_idct.c \
        sinewin.c \
        snow.c \
        snow_dwt.c \
        snowdec.c \
        snowenc.c \
        svq1.c \
        svq13.c \
        svq1dec.c \
        svq1enc.c \
        svq3.c \
        synth_filter.c \
        utils.c \
        videodsp.c \
        xiph.c \
        allcodecs.c \
        h261.c \
        h261_parser.c \
        h261data.c \
        h261dec.c \
        h261enc.c \
        h263.c \
        h263_parser.c \
        h263dec.c \
        intelh263dec.c \
        ituh263dec.c \
        mpeg4video.c mpeg4video_parser.c mpeg4videodec.c   mpegvideo.c    mpegvideo_motion.c  mpegvideo_parser.c \
        mpeg4videoenc.c mpegvideo_enc.c ituh263enc.c aandcttab.c motion_est.c \
        h264.c h264_cabac.c h264_cavlc.c h264_direct.c h264_loopfilter.c h264_parser.c h264_ps.c h264_refs.c h264_sei.c h264chroma.c h264dsp.c h264idct.c \
        h264pred.c h264qpel.c libx264.c \
        msmpeg4.c msmpeg4data.c msmpeg4dec.c \
        wma.c wma_common.c wmadec.c wmalosslessdec.c wmaprodec.c wmavoice.c \
        wmv2.c wmv2dec.c wmv2dsp.c \
        flvdec.c \
        intrax8.c \
        intrax8dsp.c \
        aac_ac3_parser.c \
        aacadtsdec.c \
        aactab.c \
        vc1.c vc1dec.c vc1data.c vc1_parser.c vc1dsp.c\
        aacdec.c aacps.c aacpsdsp.c aacsbr.c sbrdsp.c mpeg12.c mpeg12data.c mpeg12dec.c \
        pcm-bluray.c pcm-dvd.c pcm.c adpcm.c adpcm_data.c adx.c adx_parser.c adxdec.c g722.c g722dec.c g726.c\
        evrcdec.c \
        flac.c flac_parser.c flacdata.c flacdec.c flacdsp.c\
        vp3.c vp3dsp.c vp56.c vp56data.c vp56dsp.c vp56rac.c vp6.c vp6dsp.c vp8.c vp8dsp.c\
        sipr.c sipr16k.c \
        rv10.c rv30.c rv30dsp.c rv34.c rv34_parser.c rv34dsp.c rv40.c rv40dsp.c \
        vorbis.c vorbis_data.c vorbisdec.c vorbisdsp.c vorbis_parser.c dirac.c\
        gsm_parser.c gsmdec.c gsmdec_data.c msgsmdec.c \
        exif.c jpegls.c jpeglsdec.c mjpeg.c mjpegdec.c tiff.c tiff_common.c tiff_data.c \
        atrac.c atrac1.c atrac3.c\
        apedec.c\
        mss1.c mss2.c mss12.c mss2dsp.c mss3.c mss34dsp.c mss4.c
        
        
        
           
    
#LOCAL_CFLAGS += $(PV_CFLAGS_MINUS_VISIBILITY) -D__OMX_PLATFORM_ANDROID -D__STDINT_MACROS -Wformat
#LOCAL_CFLAGS := -DAAC_PLUS -DHQ_SBR -DPARAMETRICSTEREO -DOSCL_IMPORT_REF= -DOSCL_EXPORT_REF= -DOSCL_UNUSED_ARG=
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
LOCAL_MODULE := libavcodec_2.1.3_android
include $(BUILD_STATIC_LIBRARY)

