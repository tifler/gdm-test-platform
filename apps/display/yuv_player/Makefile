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

TARGET			:= yuv_player
OBJS			:= yuv_player.o

# LOCAL_CFLAGS	:= -I$(BASEDIR)/libs/libhwc
LOCAL_CFLAGS	+= -I$(BASEDIR)/libs/libion
LOCAL_CFLAGS	+= -I$(BASEDIR)/include

# LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libhwc -lhwc -lpthread
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion -lpthread
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libsync -lsync

COPY_TARGET_DIR	:= /usr/share

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
