//------------------------------------------------------------------------------
// File: config.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
// This file should be modified by some developers of C&M according to product version.
//------------------------------------------------------------------------------


#ifndef __JPU_CONFIG_H__
#define __JPU_CONFIG_H__

#ifdef __JPU_PLATFORM_MME
#include "MmpDefine.h"

#ifdef __cplusplus
extern "C" {
#endif

    int mme_util_get_jpu_fd(void);
    unsigned char* mme_util_get_jpu_instance_pool_buffer(void);
    unsigned int mme_util_get_pu_reg_vir_addr(void);
    void* mme_util_get_jpu_common_buffer(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifdef __VPU_PLATFORM_MME */


#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#	define PLATFORM_WIN32
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#	define PLATFORM_LINUX
#else
#	define PLATFORM_NON_OS
#endif

#if defined(_MSC_VER)
#	include <windows.h>
#	include <conio.h>
//#	define inline _inline
#	define VPU_DELAY_MS(X)		Sleep(X)
#	define VPU_DELAY_US(X)		Sleep(X)	// should change to delay function which can be delay a microsecond unut.
#	define kbhit _kbhit
#	define getch _getch
#elif defined(__GNUC__)
#ifdef	_KERNEL_
#	define VPU_DELAY_MS(X)		udelay(X*1000)
#	define VPU_DELAY_US(X)		udelay(X)
#else
#	define VPU_DELAY_MS(X)		usleep(X*1000)
#	define VPU_DELAY_US(X)		usleep(X)
#endif
#elif defined(__ARMCC__)
#else
#  error "Unknown compiler."
#endif

#define JPU_PROJECT_ROOT	"..\\..\\..\\"

#if defined(CNM_FPGA_PLATFORM)
#if defined(ANDROID) || defined(linux)
#else
#define SUPPORT_CONF_TEST
#endif
#endif




#define JPU_API_VERSION 166


//#define MJPEG_ERROR_CONCEAL



#endif	/* __CONFIG_H__ */

