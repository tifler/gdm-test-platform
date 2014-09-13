#------------------------------------------------------------------------------
#
# Odyssesus Test Platform Makefiles
#
# AUTHOR	: ydlee@anapass.com
# DATE		: 2014-07-21
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
ifndef	BASEDIR
BASEDIR	:= $(shell pwd)/../..
endif
#------------------------------------------------------------------------------

CROSS_COMPILE	:= HOST
DO_NOT_INSTALL	:= 1
TARGET			:= libmsgio.so
OBJS			:= gdm-msgio.o

LOCAL_CFLAGS	:= -fPIC
LOCAL_CFLAGS    += -I$(BASEDIR)/include
LOCAL_LFLAGS	:= -shared

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
