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

CALIB_OBJS		:= gisp/calibData/data.o gisp/calibData/calibDataTabSelection.o
CALIB_OBJS		+= gisp/calibData/ccParams.o gisp/calibData/ccParamsTabSelection.o
CALIB_OBJS		+= gisp/calibData/ctlParams.o

SENSOR_OBJS		:= gisp/OV2715/sensorOVT2715.o
SENSOR_OBJS		+= gisp/VIGEN/vigen.o

GISP_OBJS		:= gisp/gisp-isp.o gisp/gisp-dxo.o gisp/gisp-sif.o
GISP_OBJS		+= gisp/gisp-iodev.o gisp/gisp-sensor.o

MAIN_OBJS		:= main.o v4l2.o gdm-buffer.o display.o stream.o option.o
MAIN_OBJS		+= vid-enc.o jpeg-enc.o
MAIN_OBJS		+= yuv-writer.o v4l2.o media.o
MAIN_OBJS		+= file-io.o image-info.o

OBJS			:= $(CALIB_OBJS)
OBJS			+= $(SENSOR_OBJS)
OBJS			+= $(GISP_OBJS)
OBJS			+= $(MAIN_OBJS)

LOCAL_CFLAGS	:=-DDATA_SYMS_H_DIR=gisp/firmware/correctionData/include
LOCAL_CFLAGS	+=-DDATA_GLOBAL_HEADER=gisp/firmware/include/DxOISP.h
LOCAL_CFLAGS	+=-DDATA_ROOT_DIR=gisp/firmware/correctionData/include
LOCAL_CFLAGS	+=-DCHIP_VENDOR_PATH_ISP_INC_DIR=gisp/firmware/include
LOCAL_CFLAGS	+=-DPROC_NAMESPACE=generic
LOCAL_CFLAGS	+=-DDXO_PRINTF=DxOISP_Printf
LOCAL_CFLAGS	+=-DDATA_NAMESPACE=OV8820
LOCAL_CFLAGS	+=-DCC_DATA_INC_DIR=gisp/calibData/
LOCAL_CFLAGS	+=-IcalibData
LOCAL_CFLAGS	+=-Igisp/firmware/include
LOCAL_CFLAGS	+=-I$(BASEDIR)/libs/libion
LOCAL_CFLAGS	+=-I$(BASEDIR)/libs/libini
LOCAL_CFLAGS	+=-IOV2715
LOCAL_CFLAGS	+=-Igisp
LOCAL_CFLAGS	+=-g

LOCAL_CFLAGS    += -I$(BASEDIR)/libs/libhwc
LOCAL_CFLAGS    += -I$(BASEDIR)/libs/libmsgio
LOCAL_CFLAGS    += -I$(BASEDIR)/libs/libmme
LOCAL_CFLAGS	+= -I$(BASEDIR)/libs/libv4l2_api/inc/
LOCAL_CFLAGS    += -I$(BASEDIR)/include

#LOCAL_CFLAGS	+=-D__USE_IOCTL_IO

LOCAL_LFLAGS	:=gisp/firmware/object/arm-linux-androideabi-4.8/dbg/DxOISP_Firmware.o
LOCAL_LFLAGS	+=-lpthread
LOCAL_LFLAGS	+=-L$(BASEDIR)/libs/libion -lion
LOCAL_LFLAGS	+=-L$(BASEDIR)/libs/libini -lini

# for display
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libhwc -lhwc -lpthread
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libsync -lsync
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libmsgio -lmsgio
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libv4l2_api -lv4l2_api
LOCAL_LFLAGS    += -L$(BASEDIR)/libs/libmme -lmme

COPY_SOURCE_DIR	:= tc
COPY_TARGET_DIR	:= /usr/share

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
