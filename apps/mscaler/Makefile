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

TARGET			:= mscaler
OBJS			:= test.o

LOCAL_CFLAGS	:= -I$(BASEDIR)/libs/libmscaler
LOCAL_CFLAGS	+= -I$(BASEDIR)/libs/libini
LOCAL_CFLAGS	+= -I$(BASEDIR)/libs/libion
LOCAL_CFLAGS	+= -I$(BASEDIR)/include

LOCAL_LFLAGS	:= -L$(BASEDIR)/libs/libmscaler -lmscaler
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libini -lini
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion

COPY_SOURCE_DIR	:= tc images
COPY_TARGET_DIR	:= /usr/share

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
