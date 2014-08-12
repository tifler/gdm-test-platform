

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(TARGET_PRODUCT),cm_diamond)
OMX_TOP := $(TOP)/device/anapass/multimedia/openmax_il
MME_TOP := $(TOP)/device/anapass/multimedia/mme
else
OMX_TOP := $(TOP)/device/anapass/diamond/multimedia/openmax_il
MME_TOP := $(TOP)/device/anapass/diamond/multimedia/mme
endif

OMX_INC := $(OMX_TOP)/inc
FFMPEG_TOP := $(TOP)/external/ffmpeg/ffmpeg-2.1.3
RIL_TOP := $(TOP)/hardware/ril

OMX_SRC_PATH := ../../openmax_il
MME_SRC_PATH := ../../mme

LOCAL_SRC_FILES:= \
  main_mmeplayer.cpp \
  mme_player.cpp \
  mme_shell_main.cpp\
  multimedia/mme_cmd_player.cpp\
  system/mme_cmd_system.cpp \
  system/mme_cmd_socket.cpp \
  ril/mme_cmd_ril.cpp \
  
LOCAL_SRC_FILES += \
  $(OMX_SRC_PATH)/oal/omx_oal_android.cpp \
  
LOCAL_SRC_FILES += \
  $(MME_SRC_PATH)/MmpGlobal/MmpMediaInfo.cpp \
  $(MME_SRC_PATH)/MmpGlobal/MmpDefine_Data.c \
  $(MME_SRC_PATH)/MmpComm/linux_tool/sysinfo.c \
  $(MME_SRC_PATH)/MmpComm/linux_tool/version.c \
  $(MME_SRC_PATH)/MmpComm/mmp_oal_cond.cpp \
  $(MME_SRC_PATH)/MmpComm/mmp_oal_cond_linux.cpp \
  $(MME_SRC_PATH)/MmpComm/mmp_oal_mutex.cpp \
  $(MME_SRC_PATH)/MmpComm/mmp_oal_mutex_linux.cpp \
  $(MME_SRC_PATH)/MmpComm/mmp_simple_heap.cpp \
  $(MME_SRC_PATH)/MmpComm/mmp_oal_task.cpp \
  $(MME_SRC_PATH)/MmpComm/mmp_oal_task_linux.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpUtil.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpUtil_Jpeg.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpBitExtractor.cpp\
  $(MME_SRC_PATH)/MmpComm/MmpMpeg4Tool.cpp\
  $(MME_SRC_PATH)/MmpComm/MmpH264Tool.cpp\
  $(MME_SRC_PATH)/MmpComm/MmpObject.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpOAL.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpOALCriticalSection.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpOALCriticalSection_Linux.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpOALTask.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpOALTask_Linux.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpOALSemaphore.cpp \
  $(MME_SRC_PATH)/MmpComm/MmpOALSemaphore_Linux.cpp \
  $(MME_SRC_PATH)/MmpPlayer/MmpPlayerService.cpp \
  $(MME_SRC_PATH)/MmpPlayer/MmpPlayer.cpp \
  $(MME_SRC_PATH)/MmpPlayer/MmpPlayerVideo.cpp \
  $(MME_SRC_PATH)/MmpPlayer/MmpPlayerAudio.cpp \
  $(MME_SRC_PATH)/MmpPlayer/MmpPlayerAVEx1.cpp \
  $(MME_SRC_PATH)/MmpPlayer/MmpPlayerAVEx2.cpp \
  $(MME_SRC_PATH)/MmpPlayer/MmpPlayerStagefright.cpp \
  $(MME_SRC_PATH)/MmpDemuxer/MmpDemuxerBuffer.cpp \
  $(MME_SRC_PATH)/MmpDemuxer/MmpDemuxer.cpp \
  $(MME_SRC_PATH)/MmpDemuxer/MmpDemuxer_Ffmpeg.cpp \
  $(MME_SRC_PATH)/MmpDemuxer/MmpDemuxer_ammf.cpp \
  $(MME_SRC_PATH)/MmpRenderer/MmpRenderer.cpp \
  $(MME_SRC_PATH)/MmpRenderer/MmpRenderer_AndroidSurfaceEx1.cpp\
  $(MME_SRC_PATH)/MmpRenderer/MmpRenderer_AndroidTinyAlsa.cpp\
  $(MME_SRC_PATH)/MmpEncoder/MmpEncoder.cpp \
  $(MME_SRC_PATH)/MmpEncoder/MmpEncoderFfmpeg.cpp \
  $(MME_SRC_PATH)/MmpEncoder/MmpEncoderMfc.cpp \
  $(MME_SRC_PATH)/MmpEncoder/MmpEncoderVideo.cpp \
  $(MME_SRC_PATH)/MmpEncoder/MmpEncoderVideo_Ffmpeg.cpp \
  $(MME_SRC_PATH)/MmpEncoder/MmpEncoderVideo_Mfc.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoder.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderAudio.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderAudio_Dummy.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderAudio_Ffmpeg.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderFfmpeg.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderMfc.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderMfcAndroid44.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderVideo.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderVideo_Dummy.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderVideo_Ffmpeg.cpp \
  $(MME_SRC_PATH)/MmpDecoder/MmpDecoderVideo_Mfc.cpp \
  $(MME_SRC_PATH)/MmpRIL/mmp_ril_ctrl.cpp \
  $(MME_SRC_PATH)/MmpRIL/mmp_ril_ctrl_comm.cpp \
  $(MME_SRC_PATH)/MmpRIL/mmp_ril_debug.cpp \
  $(MME_SRC_PATH)/MmpRIL/mmp_ril_cmd.cpp \
  $(MME_SRC_PATH)/MmpRIL/mmp_ril_unsol.cpp \
  $(MME_SRC_PATH)/MmpRIL/mmp_ril_state.cpp \
  $(MME_SRC_PATH)/MmpRIL/mmp_ril_msgproc.cpp \
  
    
  
LOCAL_C_INCLUDES:= \
      $(TOP)/frameworks/native/include/media/openmax \
      $(TOP)/frameworks/native/include/media/hardware\
      $(TOP)/frameworks/native/include/\
      $(TOP)/frameworks/av/include\
      $(TOP)/external/tinyalsa/include\
      $(TOP)/external/jpeg\
      $(TOP)/external/libyuv/files/include\
      $(OMX_TOP)/inc/anapass\
      $(OMX_TOP)/oal\
      $(OMX_TOP)/utils\
      $(OMX_TOP)/component/base\
      $(OMX_TOP)/component/video\
      $(OMX_TOP)/component/audio\
      $(MME_TOP)/MmpGlobal\
      $(MME_TOP)/MmpComm\
      $(MME_TOP)/MmpPlayer\
      $(MME_TOP)/MmpDemuxer\
      $(MME_TOP)/MmpMuxer\
      $(MME_TOP)/MmpDecoder\
      $(MME_TOP)/MmpEncoder\
      $(MME_TOP)/MmpRenderer\
      $(MME_TOP)/MmpDecoder/samsung\
      $(MME_TOP)/MmpRIL\
      $(MME_TOP)/libs/android/inc/aacdec\
      $(MME_TOP)/libs/android/inc/m4vh263dec\
      $(FFMPEG_TOP)\
      $(RIL_TOP)/include\
  
LOCAL_CFLAGS += -DHWCODEC_EXYNOS4_MFC_ANDROID44    
LOCAL_CFLAGS += -D__OMX_PLATFORM_ANDROID
LOCAL_CFLAGS += -DEXYNOS4_ENHANCEMENTS
LOCAL_CFLAGS += $(PV_CFLAGS_MINUS_VISIBILITY) -D__STDINT_MACROS -Wformat -Wswitch \
                -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -Werror=vla
 
  
LOCAL_SHARED_LIBRARIES :=    \
		libbinder            \
        libutils             \
        libcutils            \
        libui                \
		libgui \
	    libEGL \
		libGLESv1_CM \
		libGLESv2 \
		libdl                \
		libstlport \
		libsync \
	    libstagefright_foundation \
        libjpeg\
        libtinyalsa \
        libmedia libmediaplayerservice


LOCAL_STATIC_LIBRARIES := \
         libavformat_2.1.3_android\
         libavcodec_2.1.3_android libavcodec_arm_2.1.3_android\
         libavresample_2.1.3_android\
         libavutil_2.1.3_android libavutil_arm_2.1.3_android\         
         
LOCAL_LDFLAGS += -lstagefright_pv_aacdec -lstagefright_pv_m4vh263dec -lsecmfcapi -lseccscapi\
                 -L$(LOCAL_PATH)/$(MME_SRC_PATH)/libs/android \
         
LOCAL_MODULE:= mmeshell

include $(BUILD_EXECUTABLE)