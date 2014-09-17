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
MME_VPU_DEFINE := __VPU_PLATFORM_MME

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
    $(MME_TOP)/MmpComm/mmp_msg_proc.cpp \
    $(MME_TOP)/MmpComm/mmp_lock.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_cond.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_cond_linux.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_mutex.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_mutex_linux.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_mutex_linux_sema.cpp \
    $(MME_TOP)/MmpComm/mmp_simple_heap.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_task.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_task_linux.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_shm.cpp \
    $(MME_TOP)/MmpComm/mmp_oal_shm_linux.cpp \
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
  
MME_SRC_BUFFER = \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_ion.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_ion_attach.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_heap.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_heap_attach.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_ext.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_mgr.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_mgr_ex1.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_media.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_videoframe.cpp \
    $(MME_TOP)/MmpDecoder/buffer/mmp_buffer_videostream.cpp \

MME_SRC_CNM_VPU = \
    $(MME_TOP)/MmpDecoder/cnm-vpu/mmp_vpu_if.cpp \
    $(MME_TOP)/MmpDecoder/cnm-vpu/mmp_vpu_if_ana.cpp \
    $(MME_TOP)/MmpDecoder/cnm-vpu/mmp_vpu_if_cnm.cpp \
    $(MME_TOP)/MmpDecoder/cnm-vpu/mmp_vpu_dev.cpp \
    $(MME_TOP)/MmpDecoder/cnm-vpu/mmp_vpu_dev_shm.cpp \

MME_SRC_SINGLETON = \
   $(MME_TOP)/MmpDecoder/singleton/mmp_singleton_mgr.cpp\
   $(MME_TOP)/MmpDecoder/singleton/mmp_env_mgr.cpp\

MME_SRC_DECODER = \
  $(MME_TOP)/MmpDecoder/MmpDecoder.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderAudio.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderAudio_Ffmpeg.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVideo.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderFfmpeg.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVideo_Ffmpeg.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVpu.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVpuIF.cpp \
  $(MME_TOP)/MmpDecoder/MmpDecoderVideo_Vpu.cpp \
  
  
MME_SRC_ENCODER = \
  $(MME_TOP)/MmpEncoder/MmpEncoder.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderFfmpeg.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVideo.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVideo_Ffmpeg.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVpu.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVpuIF.cpp \
  $(MME_TOP)/MmpEncoder/MmpEncoderVideo_Vpu.cpp \
  
MME_SRC_VPULIB = \
  $(MME_VPULIB_TOP)/vpuapi/vpuapi.c \
  $(MME_VPULIB_TOP)/vpuapi/vpuapi_mme.c \
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
  $(MME_TOP)/MmpDemuxer/MmpDemuxer_ammf.cpp \
  
MME_SRC_MUXER = \
  $(MME_TOP)/MmpMuxer/MmpMuxer.cpp \
  $(MME_TOP)/MmpMuxer/MmpMuxer_ammf.cpp \
  $(MME_TOP)/MmpMuxer/MmpMuxer_Ffmpeg.cpp \
  $(MME_TOP)/MmpMuxer/MmpMuxer_rawstream.cpp \
  

MME_SRC_PLAYER = \
  $(MME_TOP)/MmpPlayer/MmpPlayerService.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayer.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerVideoEx1.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerAudio.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerAVEx3.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerStagefright.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerTONE.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerPCM.cpp \
  $(MME_TOP)/MmpPlayer/MmpPlayerYUV.cpp \

MME_SRC_RENDERER = \
  $(MME_TOP)/MmpRenderer/MmpRenderer.cpp \
  $(MME_TOP)/MmpRenderer/MmpRenderer_YUVWriter.cpp \
  $(MME_TOP)/MmpRenderer/MmpRenderer_Dummy.cpp \
  $(MME_TOP)/MmpRenderer/MmpRenderer_OdyClientEx1.cpp \
  
MME_SRC_API = \
  $(MME_TOP)/api/mme_c_api_util.cpp \
  $(MME_TOP)/api/mme_c_api_video_encoder.cpp \
  $(MME_TOP)/api/mme_c_api_muxer.cpp \

SRCS = $(MME_SRC_PLAYER) $(MME_SRC_MUXER) $(MME_SRC_DEMUXER) $(MME_SRC_DECODER) $(MME_SRC_ENCODER) $(MME_SRC_RENDERER) $(MME_SRC_UTILS) $(MME_SRC_OAL) $(MME_SRC_VPULIB) $(MME_SRC_BUFFER) $(MME_SRC_SINGLETON) $(MME_SRC_CNM_VPU) $(MME_SRC_API)
OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

LOCAL_CFLAGS := -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -D$(MME_PLATFORM) -D$(MME_VPU_DEFINE) \
            -I$(BASEDIR)/include\
            -I$(MME_TOP)/../libion \
            -I$(MME_TOP)/../libmsgio\
            -I$(MME_TOP)/../libhwc\
            -I$(FFMPEG_TOP) -I$(JPEGLIB_TOP)\
            -I$(MME_VPULIB_TOP)/vpuapi -I$(MME_VPULIB_TOP)/include -I$(MME_VPULIB_TOP)/vdi/linux/driver  -I$(MME_VPULIB_TOP)/vdi\
            -I$(MME_TOP)/MmpGlobal \
            -I$(MME_TOP)/MmpComm \
            -I$(MME_TOP)/MmpDemuxer -I$(MME_TOP)/MmpMuxer \
            -I$(MME_TOP)/MmpDecoder -I$(MME_TOP)/MmpEncoder \
            -I$(MME_TOP)/MmpDecoder/buffer\
            -I$(MME_TOP)/MmpDecoder/singleton\
            -I$(MME_TOP)/MmpDecoder/cnm-vpu\
            -I$(MME_TOP)/MmpRenderer \
            -I$(MME_TOP)/MmpPlayer \
            -I$(MME_TOP)/MmpBuffer \
            
LOCAL_CXXFLAGS := $(LOCAL_CFLAGS) -D__STDC_CONSTANT_MACROS
LOCAL_LFLAGS := -shared -lpthread -ldl -lrt -lm -lstdc++ -L$(MME_TOP)/pre-built/linux_arm_ody -lffmpeg_2.1.3_ody_vfp -lx264
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libhwc -lhwc -lpthread
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libsync -lsync
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libmsgio -lmsgio


#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------
