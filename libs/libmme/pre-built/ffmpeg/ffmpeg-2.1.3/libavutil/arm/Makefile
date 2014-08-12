

export TOP = /home1/hthwang/develop/vpu_verify/verification
include $(TOP)/rules.mk

FFMPEG_TOP = $(TOP)/multimeida/ffmpeg/ffmpeg-2.1.3
LIBNAME = ibavutil_arm_2.1.3_odyfpga.a

SRCS_API = cpu.c \
           float_dsp_init_arm.c float_dsp_init_neon.c float_dsp_init_vfp.c \
           float_dsp_neon.S float_dsp_vfp.S
           

SRCS = $(SRCS_API)
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

CFLAGS += -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -I../..
ASFLAGS += -I../..

$(LIBNAME):   $(OBJS)
$(LIBNAME):
	$(AR) r $@ $^
	cp  $(LIBNAME) ../../../lib
	
clean:
	rm -f $(OBJS) *.o *.a *~
	@for a in $(OBJS); do rm -f $$a ; done

