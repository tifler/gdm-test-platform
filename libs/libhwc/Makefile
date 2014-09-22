#------------------------------------------------------------------------------
#
# Odyssesus Test Platform Makefiles
#
# AUTHOR	: ydlee@anapass.com
# DATE		: 2014-07-14
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
ifndef	BASEDIR
BASEDIR	:= $(shell pwd)/../..
endif
#------------------------------------------------------------------------------

TARGET			:= libhwc.so
OBJS			:= gdm_dss_wrapper.o writeback.o hwc_vsync.o
# hwc_utils.o
LOCAL_CFLAGS	:= -I$(BASEDIR)/include -fPIC
LOCAL_LFLAGS	:= -shared

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
