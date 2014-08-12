
LOCAL_PATH := $(call my-dir)

#Building vpurun binary which will be placed in the /system/bin folder
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	main.c \
	vpurun.c \
	mixer.c \
	
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := vpurun


#LOCAL_CFLAGS +=  -DCNM_FPGA_PLATFORM


LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(TOP)/hardware/vpu/include \
                    $(TOP)/hardware/vpu/theoraparser/include	\
					$(TOP)/hardware/vpu/vpuapi		\
 
					

LOCAL_SHARED_LIBRARIES :=       \
        libvpu					\
        libtheoraparser         \
        libutils                \
		liblog					\

	
include $(BUILD_EXECUTABLE)

