

CROSS_COMPILE   :=/home1/hthwang/tools/arm-2013.05-linux/bin/arm-none-linux-gnueabi-
TOP = /home1/hthwang/develop/odysseus/test-platform
MME_TOP = $(TOP)/libs/libmme
FFMPEG_TOP = .

CC				:=$(CROSS_COMPILE)gcc
CXX				:=$(CROSS_COMPILE)g++
LD				:=$(CROSS_COMPILE)ld

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

