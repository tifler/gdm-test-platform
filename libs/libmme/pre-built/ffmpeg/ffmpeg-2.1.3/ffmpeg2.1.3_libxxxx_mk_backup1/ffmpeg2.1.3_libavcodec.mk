
FFMPEG_SRC_LIBAVCODEC_ARM=\
		           $(FFMPEG_LIBAVCODEC_TOP)/arm/dsputil_init_arm.c     \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/dsputil_init_armv5te.c     \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/dsputil_init_armv6.c     \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/dsputil_arm.S          \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/dsputil_armv6.S          \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/jrevdct_arm.S          \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/simple_idct_arm.S      \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/simple_idct_armv5te.S      \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/simple_idct_armv6.S      \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/videodsp_init_arm.c    \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/videodsp_init_armv5te.c    \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/videodsp_armv5te.S    \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/hpeldsp_init_arm.c     \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/hpeldsp_init_armv6.c     \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/hpeldsp_arm.S          \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/hpeldsp_armv6.S          \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/h264chroma_init_arm.c  \
				   $(FFMPEG_LIBAVCODEC_TOP)/arm/h264dsp_init_arm.c     \
				   $(FFMPEG_LIBAVCODEC_TOP)/arm/h264pred_init_arm.c    \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/h264qpel_init_arm.c    \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/h264dsp_armv6.S     \
				   $(FFMPEG_LIBAVCODEC_TOP)/arm/mpegvideo_arm.c \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/mpegvideo_armv5te.c \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/mpegvideo_armv5te_s.S \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/mpegaudiodsp_init_arm.c\
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/fft_init_arm.c \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/mpegaudiodsp_fixed_armv6.S  \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/fmtconvert_init_arm.c   \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/mpegaudiodsp_fixed_armv6.S  \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/aacpsdsp_init_arm.c $(FFMPEG_LIBAVCODEC_TOP)/arm/sbrdsp_init_arm.c  \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/flacdsp_arm.S $(FFMPEG_LIBAVCODEC_TOP)/arm/flacdsp_init_arm.c\
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/vp3dsp_init_arm.c  $(FFMPEG_LIBAVCODEC_TOP)/arm/vp6dsp_init_arm.c  \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/vp8_armv6.S $(FFMPEG_LIBAVCODEC_TOP)/arm/vp8dsp_armv6.S $(FFMPEG_LIBAVCODEC_TOP)/arm/vp8dsp_init_arm.c $(FFMPEG_LIBAVCODEC_TOP)/arm/vp8dsp_init_armv6.c \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/rv34dsp_init_arm.c $(FFMPEG_LIBAVCODEC_TOP)/arm/rv40dsp_init_arm.c \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/vorbisdsp_init_arm.c \
                   $(FFMPEG_LIBAVCODEC_TOP)/arm/dcadsp_init_arm.c \
                   
                   

FFMPEG_SRC_LIBAVCODEC=\
        $(FFMPEG_LIBAVCODEC_TOP)/acelp_filters.c \
        $(FFMPEG_LIBAVCODEC_TOP)/acelp_pitch_delay.c \
        $(FFMPEG_LIBAVCODEC_TOP)/acelp_vectors.c \
        $(FFMPEG_LIBAVCODEC_TOP)/audioconvert.c \
        $(FFMPEG_LIBAVCODEC_TOP)/avpacket.c \
        $(FFMPEG_LIBAVCODEC_TOP)/avpicture.c \
        $(FFMPEG_LIBAVCODEC_TOP)/bethsoftvideo.c \
        $(FFMPEG_LIBAVCODEC_TOP)/bitstream.c \
        $(FFMPEG_LIBAVCODEC_TOP)/cabac.c \
        $(FFMPEG_LIBAVCODEC_TOP)/cavs.c \
        $(FFMPEG_LIBAVCODEC_TOP)/cavsdsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/cavsdata.c \
        $(FFMPEG_LIBAVCODEC_TOP)/celp_filters.c \
        $(FFMPEG_LIBAVCODEC_TOP)/celp_math.c \
        $(FFMPEG_LIBAVCODEC_TOP)/codec_desc.c \
        $(FFMPEG_LIBAVCODEC_TOP)/cook.c \
        $(FFMPEG_LIBAVCODEC_TOP)/cook_parser.c \
        $(FFMPEG_LIBAVCODEC_TOP)/dct.c \
        $(FFMPEG_LIBAVCODEC_TOP)/dct32_fixed.c \
        $(FFMPEG_LIBAVCODEC_TOP)/dct32_float.c \
        $(FFMPEG_LIBAVCODEC_TOP)/dsputil.c \
        $(FFMPEG_LIBAVCODEC_TOP)/dv_profile.c \
        $(FFMPEG_LIBAVCODEC_TOP)/eac3_data.c \
        $(FFMPEG_LIBAVCODEC_TOP)/error_resilience.c \
        $(FFMPEG_LIBAVCODEC_TOP)/faandct.c \
        $(FFMPEG_LIBAVCODEC_TOP)/faanidct.c \
        $(FFMPEG_LIBAVCODEC_TOP)/fft.c \
        $(FFMPEG_LIBAVCODEC_TOP)/fmtconvert.c \
        $(FFMPEG_LIBAVCODEC_TOP)/huffman.c\
        $(FFMPEG_LIBAVCODEC_TOP)/frame_thread_encoder.c \
        $(FFMPEG_LIBAVCODEC_TOP)/golomb.c \
        $(FFMPEG_LIBAVCODEC_TOP)/hpeldsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/imgconvert.c \
        $(FFMPEG_LIBAVCODEC_TOP)/jfdctfst.c \
        $(FFMPEG_LIBAVCODEC_TOP)/jfdctint.c \
        $(FFMPEG_LIBAVCODEC_TOP)/jrevdct.c \
        $(FFMPEG_LIBAVCODEC_TOP)/kbdwin.c \
        $(FFMPEG_LIBAVCODEC_TOP)/lsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mathtables.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mdct.c \
        $(FFMPEG_LIBAVCODEC_TOP)/motionpixels_tablegen.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpeg4audio.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudio.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudiodata.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudiodec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudiodecheader.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudiodsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudiodsp_data.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudiodsp_fixed.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpegaudiodsp_float.c \
        $(FFMPEG_LIBAVCODEC_TOP)/options.c \
        $(FFMPEG_LIBAVCODEC_TOP)/parser.c \
        $(FFMPEG_LIBAVCODEC_TOP)/pthread.c \
        $(FFMPEG_LIBAVCODEC_TOP)/qcelpdec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/ra144.c \
        $(FFMPEG_LIBAVCODEC_TOP)/ra144dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/ra288.c \
        $(FFMPEG_LIBAVCODEC_TOP)/ratecontrol.c \
        $(FFMPEG_LIBAVCODEC_TOP)/raw.c \
        $(FFMPEG_LIBAVCODEC_TOP)/rawdec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/rdft.c \
        $(FFMPEG_LIBAVCODEC_TOP)/resample.c \
        $(FFMPEG_LIBAVCODEC_TOP)/resample2.c \
        $(FFMPEG_LIBAVCODEC_TOP)/simple_idct.c \
        $(FFMPEG_LIBAVCODEC_TOP)/sinewin.c \
        $(FFMPEG_LIBAVCODEC_TOP)/snow.c \
        $(FFMPEG_LIBAVCODEC_TOP)/snow_dwt.c \
        $(FFMPEG_LIBAVCODEC_TOP)/snowdec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/snowenc.c \
        $(FFMPEG_LIBAVCODEC_TOP)/svq1.c \
        $(FFMPEG_LIBAVCODEC_TOP)/svq13.c \
        $(FFMPEG_LIBAVCODEC_TOP)/svq1dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/svq1enc.c \
        $(FFMPEG_LIBAVCODEC_TOP)/svq3.c \
        $(FFMPEG_LIBAVCODEC_TOP)/synth_filter.c \
        $(FFMPEG_LIBAVCODEC_TOP)/utils.c \
        $(FFMPEG_LIBAVCODEC_TOP)/videodsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/xiph.c \
        $(FFMPEG_LIBAVCODEC_TOP)/allcodecs.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h261.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h261_parser.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h261data.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h261dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h261enc.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h263.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h263_parser.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h263dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/intelh263dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/ituh263dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpeg4video.c $(FFMPEG_LIBAVCODEC_TOP)/mpeg4video_parser.c $(FFMPEG_LIBAVCODEC_TOP)/mpeg4videodec.c   $(FFMPEG_LIBAVCODEC_TOP)/mpegvideo.c    $(FFMPEG_LIBAVCODEC_TOP)/mpegvideo_motion.c  $(FFMPEG_LIBAVCODEC_TOP)/mpegvideo_parser.c \
        $(FFMPEG_LIBAVCODEC_TOP)/mpeg4videoenc.c $(FFMPEG_LIBAVCODEC_TOP)/mpegvideo_enc.c $(FFMPEG_LIBAVCODEC_TOP)/ituh263enc.c $(FFMPEG_LIBAVCODEC_TOP)/aandcttab.c $(FFMPEG_LIBAVCODEC_TOP)/motion_est.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h264.c $(FFMPEG_LIBAVCODEC_TOP)/h264_cabac.c $(FFMPEG_LIBAVCODEC_TOP)/h264_cavlc.c $(FFMPEG_LIBAVCODEC_TOP)/h264_direct.c $(FFMPEG_LIBAVCODEC_TOP)/h264_loopfilter.c $(FFMPEG_LIBAVCODEC_TOP)/h264_parser.c $(FFMPEG_LIBAVCODEC_TOP)/h264_ps.c $(FFMPEG_LIBAVCODEC_TOP)/h264_refs.c $(FFMPEG_LIBAVCODEC_TOP)/h264_sei.c $(FFMPEG_LIBAVCODEC_TOP)/h264chroma.c $(FFMPEG_LIBAVCODEC_TOP)/h264dsp.c $(FFMPEG_LIBAVCODEC_TOP)/h264idct.c \
        $(FFMPEG_LIBAVCODEC_TOP)/h264pred.c $(FFMPEG_LIBAVCODEC_TOP)/h264qpel.c \
        $(FFMPEG_LIBAVCODEC_TOP)/msmpeg4.c $(FFMPEG_LIBAVCODEC_TOP)/msmpeg4data.c $(FFMPEG_LIBAVCODEC_TOP)/msmpeg4dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/wma.c $(FFMPEG_LIBAVCODEC_TOP)/wma_common.c $(FFMPEG_LIBAVCODEC_TOP)/wmadec.c $(FFMPEG_LIBAVCODEC_TOP)/wmalosslessdec.c $(FFMPEG_LIBAVCODEC_TOP)/wmaprodec.c $(FFMPEG_LIBAVCODEC_TOP)/wmavoice.c \
        $(FFMPEG_LIBAVCODEC_TOP)/wmv2.c $(FFMPEG_LIBAVCODEC_TOP)/wmv2dec.c $(FFMPEG_LIBAVCODEC_TOP)/wmv2dsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/flvdec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/intrax8.c \
        $(FFMPEG_LIBAVCODEC_TOP)/intrax8dsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/aac_ac3_parser.c \
        $(FFMPEG_LIBAVCODEC_TOP)/aacadtsdec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/aactab.c \
        $(FFMPEG_LIBAVCODEC_TOP)/vc1.c $(FFMPEG_LIBAVCODEC_TOP)/vc1dec.c $(FFMPEG_LIBAVCODEC_TOP)/vc1data.c $(FFMPEG_LIBAVCODEC_TOP)/vc1_parser.c $(FFMPEG_LIBAVCODEC_TOP)/vc1dsp.c\
        $(FFMPEG_LIBAVCODEC_TOP)/aacdec.c $(FFMPEG_LIBAVCODEC_TOP)/aacps.c $(FFMPEG_LIBAVCODEC_TOP)/aacpsdsp.c $(FFMPEG_LIBAVCODEC_TOP)/aacsbr.c $(FFMPEG_LIBAVCODEC_TOP)/sbrdsp.c $(FFMPEG_LIBAVCODEC_TOP)/mpeg12.c $(FFMPEG_LIBAVCODEC_TOP)/mpeg12data.c $(FFMPEG_LIBAVCODEC_TOP)/mpeg12dec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/pcm-bluray.c $(FFMPEG_LIBAVCODEC_TOP)/pcm-dvd.c $(FFMPEG_LIBAVCODEC_TOP)/pcm.c $(FFMPEG_LIBAVCODEC_TOP)/adpcm.c $(FFMPEG_LIBAVCODEC_TOP)/adpcm_data.c $(FFMPEG_LIBAVCODEC_TOP)/adx.c $(FFMPEG_LIBAVCODEC_TOP)/adx_parser.c $(FFMPEG_LIBAVCODEC_TOP)/adxdec.c $(FFMPEG_LIBAVCODEC_TOP)/g722.c $(FFMPEG_LIBAVCODEC_TOP)/g722dec.c $(FFMPEG_LIBAVCODEC_TOP)/g726.c\
        $(FFMPEG_LIBAVCODEC_TOP)/evrcdec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/flac.c $(FFMPEG_LIBAVCODEC_TOP)/flac_parser.c $(FFMPEG_LIBAVCODEC_TOP)/flacdata.c $(FFMPEG_LIBAVCODEC_TOP)/flacdec.c $(FFMPEG_LIBAVCODEC_TOP)/flacdsp.c\
        $(FFMPEG_LIBAVCODEC_TOP)/vp3.c $(FFMPEG_LIBAVCODEC_TOP)/vp3dsp.c $(FFMPEG_LIBAVCODEC_TOP)/vp56.c $(FFMPEG_LIBAVCODEC_TOP)/vp56data.c $(FFMPEG_LIBAVCODEC_TOP)/vp56dsp.c $(FFMPEG_LIBAVCODEC_TOP)/vp56rac.c $(FFMPEG_LIBAVCODEC_TOP)/vp6.c $(FFMPEG_LIBAVCODEC_TOP)/vp6dsp.c $(FFMPEG_LIBAVCODEC_TOP)/vp8.c $(FFMPEG_LIBAVCODEC_TOP)/vp8dsp.c\
        $(FFMPEG_LIBAVCODEC_TOP)/sipr.c $(FFMPEG_LIBAVCODEC_TOP)/sipr16k.c \
        $(FFMPEG_LIBAVCODEC_TOP)/rv10.c $(FFMPEG_LIBAVCODEC_TOP)/rv30.c $(FFMPEG_LIBAVCODEC_TOP)/rv30dsp.c $(FFMPEG_LIBAVCODEC_TOP)/rv34.c $(FFMPEG_LIBAVCODEC_TOP)/rv34_parser.c $(FFMPEG_LIBAVCODEC_TOP)/rv34dsp.c $(FFMPEG_LIBAVCODEC_TOP)/rv40.c $(FFMPEG_LIBAVCODEC_TOP)/rv40dsp.c \
        $(FFMPEG_LIBAVCODEC_TOP)/vorbis.c $(FFMPEG_LIBAVCODEC_TOP)/vorbis_data.c $(FFMPEG_LIBAVCODEC_TOP)/vorbisdec.c $(FFMPEG_LIBAVCODEC_TOP)/vorbisdsp.c $(FFMPEG_LIBAVCODEC_TOP)/vorbis_parser.c $(FFMPEG_LIBAVCODEC_TOP)/dirac.c\
        $(FFMPEG_LIBAVCODEC_TOP)/gsm_parser.c $(FFMPEG_LIBAVCODEC_TOP)/gsmdec.c $(FFMPEG_LIBAVCODEC_TOP)/gsmdec_data.c $(FFMPEG_LIBAVCODEC_TOP)/msgsmdec.c \
        $(FFMPEG_LIBAVCODEC_TOP)/exif.c $(FFMPEG_LIBAVCODEC_TOP)/jpegls.c $(FFMPEG_LIBAVCODEC_TOP)/jpeglsdec.c $(FFMPEG_LIBAVCODEC_TOP)/mjpeg.c $(FFMPEG_LIBAVCODEC_TOP)/mjpegdec.c $(FFMPEG_LIBAVCODEC_TOP)/tiff.c $(FFMPEG_LIBAVCODEC_TOP)/tiff_common.c $(FFMPEG_LIBAVCODEC_TOP)/tiff_data.c \
        $(FFMPEG_LIBAVCODEC_TOP)/atrac.c $(FFMPEG_LIBAVCODEC_TOP)/atrac1.c $(FFMPEG_LIBAVCODEC_TOP)/atrac3.c\
        $(FFMPEG_LIBAVCODEC_TOP)/apedec.c\
        $(FFMPEG_LIBAVCODEC_TOP)/mss1.c $(FFMPEG_LIBAVCODEC_TOP)/mss2.c $(FFMPEG_LIBAVCODEC_TOP)/mss12.c $(FFMPEG_LIBAVCODEC_TOP)/mss2dsp.c $(FFMPEG_LIBAVCODEC_TOP)/mss3.c $(FFMPEG_LIBAVCODEC_TOP)/mss34dsp.c $(FFMPEG_LIBAVCODEC_TOP)/mss4.c  \
        $(FFMPEG_LIBAVCODEC_TOP)/rangecoder.c  $(FFMPEG_LIBAVCODEC_TOP)/synth_filter.c\
        $(FFMPEG_LIBAVCODEC_TOP)/lzw.c  $(FFMPEG_LIBAVCODEC_TOP)/faxcompr.c\
        
        
        
