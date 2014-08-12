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
BASEDIR	:= $(shell pwd)/../../..
endif
#------------------------------------------------------------------------------

TARGET			:= mmeplayer
MME_TOP := $(BASEDIR)/libs/libmme
FFMPEG_TOP = $(MME_TOP)/pre-built/ffmpeg/ffmpeg-2.1.3
JPEGLIB_TOP = $(MME_TOP)/pre-built/jpeglib
MME_VPULIB_TOP=$(MME_TOP)/MmpDecoder/cnm-coda960-ref_sw_pkg-v3.3.0

SRCS_MAIN = \
       main_mmeplayer.cpp\
       mme_shell_main.cpp
       
SRCS_SYSTEM = system/mme_cmd_system.cpp

SRCS_PLAYER = multimedia/mme_cmd_player.cpp\
              multimedia/mme_cmd_vpu.cpp\
              multimedia/mme_cmd_encoder.cpp\
              
SRCS = $(SRCS_MAIN) $(SRCS_SYSTEM) $(SRCS_PLAYER) $(MMEPLAYER_SRCS)              
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

LOCAL_CFLAGS	:= -D__PLATFORM_ODY_FPGA
#LOCAL_CFLAGS	:= -I$(BASEDIR)/libs/libhwc
#LOCAL_CFLAGS	+= -I$(BASEDIR)/libs/libion
#LOCAL_CFLAGS	+= -I$(BASEDIR)/include
LOCAL_CFLAGS	+= \
            -I$(BASEDIR)/include\
            -I$(MME_TOP)/../libion \
            -I$(FFMPEG_TOP) -I$(JPEGLIB_TOP)\
            -I$(MME_VPULIB_TOP)/vpuapi -I$(MME_VPULIB_TOP)/include \
            -I$(MME_TOP)/MmpGlobal \
            -I$(MME_TOP)/MmpComm \
            -I$(MME_TOP)/MmpDemuxer -I$(MME_TOP)/MmpMuxer \
            -I$(MME_TOP)/MmpDecoder -I$(MME_TOP)/MmpEncoder \
            -I$(MME_TOP)/MmpRenderer \
            -I$(MME_TOP)/MmpPlayer \
                        
LOCAL_CXXFLAGS = $(LOCAL_CFLAGS) -D__STDC_CONSTANT_MACROS

LOCAL_LFLAGS := -lpthread -ldl -lrt -lm -lstdc++ 
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libhwc -lhwc
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libsync -lsync
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libmme -lmme


#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
