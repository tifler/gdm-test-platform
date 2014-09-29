#------------------------------------------------------------------------------
#
# Odyssesus Test Platform Makefiles
#
# AUTHOR	: hthwang@anapass.com
# DATE		: 2014-08-07
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
ifndef	BASEDIR
BASEDIR	:= $(shell pwd)/../..
endif
#------------------------------------------------------------------------------

TARGET			:= libv4l2_api.so

MME_PLATFORM := __PLATFORM_ODY_FPGA
MME_VPU_DEFINE := __VPU_PLATFORM_MME
MME_JPU_DEFINE := __JPU_PLATFORM_MME

V4L2_TOP = .

MME_SRC_JPEG = \
   $(V4L2_TOP)/jpeg/v4l2_jpeg.c \
   $(V4L2_TOP)/jpeg/v4l2_jpeg_dec.c \
   $(V4L2_TOP)/jpeg/v4l2_jpeg_enc.c \
   $(V4L2_TOP)/jpeg/v4l2_jpeg_api.c \
   $(V4L2_TOP)/ion/v4l2_ion_tool.c \
     
SRCS = $(MME_SRC_JPEG)
       
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

LOCAL_CFLAGS := -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -D$(MME_PLATFORM) -D$(MME_VPU_DEFINE) -D$(MME_JPU_DEFINE)\
            -I$(BASEDIR)/include\
            -I$(V4L2_TOP)/../libion\
            -I$(V4L2_TOP)/inc \
                        
LOCAL_CXXFLAGS := $(LOCAL_CFLAGS) -D__STDC_CONSTANT_MACROS
LOCAL_LFLAGS := -shared -lpthread -ldl -lrt -lm -lstdc++ 

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------

