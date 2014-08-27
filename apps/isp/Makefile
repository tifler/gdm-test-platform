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

TARGET			:= isp
CALIB_OBJS		:= calibData/data.o calibData/calibDataTabSelection.o \
	calibData/ccParams.o calibData/ccParamsTabSelection.o calibData/ctlParams.o
SENSOR_OBJS		:= OV2715/DxOISP_SensorAPI.o OV2715/sensorOVT2715.o
#OBJS			:= main.o DxOISP_ahblAccess.o dxo.o isp-io.o ioctl.o v4l2.o \
#	$(CALIB_OBJS) $(SENSOR_OBJS)
OBJS			:= main.o v4l2.o gdm-buffer.o isp.o
OBJS			+= DxOISP_ahblAccess.o dxo.o isp-io.o event.o v4l2.o \
				   $(CALIB_OBJS) $(SENSOR_OBJS)
OBJS			+= display.o

LOCAL_CFLAGS	:=-DDATA_SYMS_H_DIR=firmware/correctionData/include
LOCAL_CFLAGS	+=-DDATA_GLOBAL_HEADER=firmware/include/DxOISP.h
LOCAL_CFLAGS	+=-DDATA_ROOT_DIR=firmware/correctionData/include
LOCAL_CFLAGS	+=-DCHIP_VENDOR_PATH_ISP_INC_DIR=firmware/include
LOCAL_CFLAGS	+=-DPROC_NAMESPACE=generic
LOCAL_CFLAGS	+=-DDXO_PRINTF=DxOISP_Printf
LOCAL_CFLAGS	+=-DDATA_NAMESPACE=OV8820
LOCAL_CFLAGS	+=-IcalibData
LOCAL_CFLAGS	+=-Ifirmware/include
LOCAL_CFLAGS	+=-I$(BASEDIR)/libs/libion
LOCAL_CFLAGS	+=-IOV2715

# for display
LOCAL_CFLAGS    += -I$(BASEDIR)/libs/libhwc
LOCAL_CFLAGS    += -I$(BASEDIR)/libs/libmsgio
LOCAL_CFLAGS    += -I$(BASEDIR)/include

#LOCAL_CFLAGS	+=-D__USE_IOCTL_IO

LOCAL_LFLAGS	:=firmware/object/arm-linux-androideabi-4.8/dbg/DxOISP_Firmware.o
LOCAL_LFLAGS	+=-lpthread
LOCAL_LFLAGS	+=-L$(BASEDIR)/libs/libion -lion -lpthread

# for display
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libhwc -lhwc -lpthread
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libsync -lsync
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libmsgio -lmsgio

COPY_TARGET_DIR	:= /usr/bin

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
