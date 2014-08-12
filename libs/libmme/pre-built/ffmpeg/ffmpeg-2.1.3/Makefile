

export TOP = /home1/hthwang/develop/vpu_verify/verification
MME_TOP = $(TOP)/ody_fpga/multimedia/mme
FFMPEG_TOP = /home1/hthwang/develop/vpu_verify/verification/ody_fpga/multimedia//ffmpeg/ffmpeg-2.1.3

include $(TOP)/rules.mk
include $(FFMPEG_TOP)/ffmpeg2.1.3.mk

LIBNAME = libffmpeg_2.1.3_odyfpga.a

SRCS = $(FFMPEG_SRCS)
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

CFLAGS += -O1 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -D__PLATFORM_ODY_FPGA -march=armv6
CFLAGS += $(CFLAGS_FFMPEG)
ASFLAGS += $(CFLAGS_FFMPEG) -march=armv6  

$(LIBNAME):   $(OBJS)
$(LIBNAME):
	$(AR) r $@ $^ 
	
clean:
	rm -f $(OBJS) *.o *.a *~
	@for a in $(OBJS); do rm -f $$a ; done

