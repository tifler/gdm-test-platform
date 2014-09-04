
FFMPEG_SRC_LIBAVRESAMPLE=\
        $(FFMPEG_LIBAVRESAMPLE_TOP)/audio_convert.c $(FFMPEG_LIBAVRESAMPLE_TOP)/audio_data.c $(FFMPEG_LIBAVRESAMPLE_TOP)/dither.c\
        $(FFMPEG_LIBAVRESAMPLE_TOP)/arm/audio_convert_init.c 
        
FFMPEG_SRC_LIBAVRESAMPLE_ARM_NEON=\
        $(FFMPEG_LIBAVRESAMPLE_TOP)/arm/audio_convert_neon.S\
        

