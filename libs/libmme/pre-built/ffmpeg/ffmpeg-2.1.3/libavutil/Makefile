

export TOP = /home1/hthwang/develop/vpu_verify/verification
include $(TOP)/rules.mk

FFMPEG_TOP = $(TOP)/multimeida/ffmpeg/ffmpeg-2.1.3
LIBNAME = libavutil_2.1.3_odyfpga.a

SRCS_LIBAVUTIL = \
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
     
SRCS_LIBAVUTIL_ARM = arm/cpu.c \
           arm/float_dsp_init_arm.c arm/float_dsp_init_neon.c arm/float_dsp_init_vfp.c \
           arm/float_dsp_neon.S arm/float_dsp_vfp.S
           

SRCS = $(SRCS_LIBAVUTIL) $(SRCS_LIBAVUTIL_ARM)
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

CFLAGS += -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -D__OMX_PLATFORM_ANDROID -I..
ASFLAGS += -I..

$(LIBNAME):   $(OBJS)
$(LIBNAME):
	$(AR) r $@ $^
	cp  $(LIBNAME) ../../lib
	
clean:
	rm -f $(OBJS) *.o *.a *~
	@for a in $(OBJS); do rm -f $$a ; done

