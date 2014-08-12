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
BASEDIR	:= $(shell pwd)/../..
endif
#------------------------------------------------------------------------------

TARGET			:= libmme.so

MME_PLATFORM := __PLATFORM_ODY_FPGA

MME_TOP = .
FFMPEG_TOP = $(MME_TOP)/pre-built/ffmpeg/ffmpeg-2.1.3
JPEGLIB_TOP = $(MME_TOP)/pre-built/jpeglib
MME_VPULIB_TOP=$(MME_TOP)/MmpDecoder/cnm-coda960-ref_sw_pkg-v3.3.0

MME_SRC_UTILS = \
   $(MME_TOP)/MmpComm/MmpUtil.cpp \
   $(MME_TOP)/MmpComm/MmpUtil_Jpeg.cpp \
   $(MME_TOP)/MmpGlobal/MmpMediaInfo.cpp \
  
MME_SRC_OAL = \
    $(MME_TOP)/MmpComm/linux_tool/sysinfo.c \
    $(MME_TOP)/MmpComm/linux_tool/version.c \
    $(MME_TOP)/MmpComm/mmp_oal_cond.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_cond_linux.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_mutex.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_mutex_linux.cpp \
    $(MME_TOP)/MmpComm/mmp_simple_heap.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_task.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_task_linux.cpp \
    $(MME_TOP)/MmpComm/MmpBitExtractor.cpp\
    $(MME_TOP)/MmpComm/MmpMpeg4Tool.cpp\
    $(MME_TOP)/MmpComm/MmpH264Tool.cpp\
    $(MME_TOP)/MmpComm/MmpObject.cpp \
    $(MME_TOP)/MmpComm/MmpOAL.cpp \
    $(MME_TOP)/MmpComm/MmpOALCriticalSection.cpp \
    $(MME_TOP)/MmpComm/MmpOALCriticalSection_Linux.cpp \
    $(MME_TOP)/MmpComm/MmpOALTask.cpp \
    $(MME_TOP)/MmpComm/MmpOALTask_Linux.cpp \
    $(MME_TOP)/MmpComm/MmpOALSemaphore.cpp \
    $(MME_TOP)/MmpComm/MmpOALSemaphore_Linux.cpp \
  

MME_SRC_DECODER = \
  $(MME_TOP)/MmpDecoder/MmpDecoder.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderAudio.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderAudio_Ffmpeg.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVideo.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderFfmpeg.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVideo_Ffmpeg.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVpu.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVideo_Vpu.cpp \
  
  
MME_SRC_ENCODER = \
  $(MME_TOP)/MmpEncoder/MmpEncoder.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderFfmpeg.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVideo.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVideo_Ffmpeg.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVpu.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVideo_Vpu.cpp \
  
MME_SRC_VPULIB = \
  $(MME_VPULIB_TOP)/vpuapi/vpuapi.c \
  $(MME_VPULIB_TOP)/vpuapi/vpuapifunc.c\
  $(MME_VPULIB_TOP)/vdi/mm.c\
  $(MME_VPULIB_TOP)/vdi/linux/vdi.c\
  $(MME_VPULIB_TOP)/vdi/linux/vdi_osal.c\
  $(MME_VPULIB_TOP)/src/vpuhelper.c\
  $(MME_VPULIB_TOP)/src/vpuio.c\

MME_SRC_DEMUXER = \
  $(MME_TOP)/MmpDemuxer/MmpDemuxerBuffer.cpp \
  $(MME_TOP)/MmpDemuxer/MmpDemuxer.cpp \
  $(MME_TOP)/MmpDemuxer/MmpDemuxer_Ffmpeg.cpp \
  $(MME_TOP)/MmpDemuxer/MmpDemuxer_FfmpegEx1.cpp \
  $(MME_TOP)/MmpDemuxer/MmpDemuxer_ammf.cpp \
  
MME_SRC_MUXER = \
  $(MME_TOP)/MmpMuxer/MmpMuxer.cpp \
  $(MME_TOP)/MmpMuxer/MmpMuxer_ammf.cpp \
  $(MME_TOP)/MmpMuxer/MmpMuxer_Ffmpeg.cpp \
  $(MME_TOP)/MmpMuxer/MmpMuxer_rawstream.cpp \
  

MME_SRC_PLAYER = \
  $(MME_TOP)/MmpPlayer/MmpPlayerService.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayer.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerVideo.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerAudio.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerAVEx1.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerAVEx2.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerStagefright.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerTONE.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerPCM.cpp \

MME_SRC_RENDERER = \
  $(MME_TOP)/MmpRenderer/MmpRenderer.cpp \
  $(MME_TOP)/MmpRenderer/MmpRenderer_FileWriter.cpp \
  $(MME_TOP)/MmpRenderer/MmpRenderer_Dummy.cpp \
  $(MME_TOP)/MmpRenderer/MmpRenderer_OdyFpgaDisplay.cpp \
  

SRCS = $(MME_SRC_PLAYER) $(MME_SRC_MUXER) $(MME_SRC_DEMUXER) $(MME_SRC_DECODER) $(MME_SRC_ENCODER) $(MME_SRC_RENDERER) $(MME_SRC_UTILS) $(MME_SRC_OAL) $(MME_SRC_VPULIB)
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

LOCAL_CFLAGS := -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -D$(MME_PLATFORM) \
            -I$(BASEDIR)/include\
            -I$(MME_TOP)/../libion \
            -I$(FFMPEG_TOP) -I$(JPEGLIB_TOP)\
            -I$(MME_VPULIB_TOP)/vpuapi -I$(MME_VPULIB_TOP)/include \
            -I$(MME_TOP)/MmpGlobal \
            -I$(MME_TOP)/MmpComm \
            -I$(MME_TOP)/MmpDemuxer -I$(MME_TOP)/MmpMuxer \
            -I$(MME_TOP)/MmpDecoder -I$(MME_TOP)/MmpEncoder \
            -I$(MME_TOP)/MmpRenderer \
            -I$(MME_TOP)/MmpPlayer \
            
LOCAL_CXXFLAGS := $(LOCAL_CFLAGS) -D__STDC_CONSTANT_MACROS
LOCAL_LFLAGS := -shared -lpthread -ldl -lrt -lm -lstdc++ -L$(MME_TOP)/pre-built/linux_arm_ody -lffmpeg_2.1.3_ody
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libhwc -lhwc -lpthread
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libsync -lsync


#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
