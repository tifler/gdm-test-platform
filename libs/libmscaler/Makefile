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

TARGET			:= libmscaler.so
OBJS			:= mscaler.o

LOCAL_CFLAGS	:= -fPIC
LOCAL_CFLAGS	+= -I$(BASEDIR)/include

LOCAL_LFLAGS	:= -shared

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
