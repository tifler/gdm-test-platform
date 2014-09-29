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

TARGET			:= mme_enc

SRCS_MAIN = main_mme_enc.c
              
SRCS = $(SRCS_MAIN) 
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

LOCAL_CFLAGS	:= \
            -I$(BASEDIR)/include\
            -I$(BASEDIR)/libs/libion \
            -I$(BASEDIR)/libs/libmme/api \
            
LOCAL_LFLAGS := -lpthread -ldl -lrt -lm -lstdc++ 
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libhwc -lhwc
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libsync -lsync
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libmsgio -lmsgio
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libv4l2_api -lv4l2_api
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libmme -lmme


#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
