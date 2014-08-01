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
BASEDIR	:= $(shell pwd)/../../..
endif
#------------------------------------------------------------------------------

TARGET			:= pngdec
OBJS			:= pngdec.o

LOCAL_CFLAGS	:= -I$(BASEDIR)/libs/libhwc
LOCAL_CFLAGS	+= -I$(BASEDIR)/libs/libion
LOCAL_CFLAGS	+= -I$(BASEDIR)/libs/libmsgio
LOCAL_CFLAGS	+= -I$(BASEDIR)/include
LOCAL_CFLAGS	+= -I$(BASEDIR)/include/libpng16

LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libhwc -lhwc -lpthread
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libsync -lsync
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libmsgio -lmsgio
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/pre-built -lz -lpng

COPY_TARGET_DIR	:= /usr/bin

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------

