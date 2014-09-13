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
BASEDIR	:= $(shell pwd)/../../..
endif
#------------------------------------------------------------------------------

CROSS_COMPILE	:= HOST
TARGET			:= server
OBJS			:= server.o

LOCAL_CFLAGS	:= -I$(BASEDIR)/libs/libmsgio-x86
LOCAL_CFLAGS	+= -I$(BASEDIR)/include
LOCAL_LFLAGS	:= -L$(BASEDIR)/libs/libmsgio-x86 -lmsgio -lpthread

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
