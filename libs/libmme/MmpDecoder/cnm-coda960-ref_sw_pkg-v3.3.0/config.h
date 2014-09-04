﻿//------------------------------------------------------------------------------
// File: config.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
// This file should be modified by some developers of C&M according to product version.
//------------------------------------------------------------------------------


#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __VPU_PLATFORM_MME
#include "MmpDefine.h"
#include "mmp_buffer_mgr.h"

#ifdef __cplusplus
#if (MMP_OS == MMP_OS_WIN32)
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#else
#define EXTERN_C extern
#endif

EXTERN_C int g_vpu_fd; 
EXTERN_C unsigned char* g_p_instance_pool_buffer;
EXTERN_C unsigned int g_vpu_reg_vir_addr;
EXTERN_C void* g_p_vpu_common_buffer;

#endif /* #ifdef __VPU_PLATFORM_MME */

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#	define PLATFORM_WIN32
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#	define PLATFORM_LINUX
#else
#	define PLATFORM_NON_OS
#endif

#ifdef PLATFORM_WIN32
#	include <windows.h>
#endif

#if defined(_MSC_VER)
//#define inline _inline
#elif defined(__GNUC__)
#elif defined(__ARMCC__)
#else
#  error "Unknown compiler."
#endif




#define API_VERSION 330



#if defined(PLATFORM_NON_OS) || defined (ANDROID) || defined(MFHMFT_EXPORTS) || defined(OMXIL_COMPONENT) || defined(DXVA_UMOD_DRIVER) || defined(__MINGW32__)
//#define SUPPORT_FFMPEG_DEMUX
#else
#define SUPPORT_FFMPEG_DEMUX
#endif





//#define REPORT_PERFORMANCE


//#define SUPPORT_MEM_PROTECT

#define BIT_CODE_FILE_PATH "../coda960.h"
#define PROJECT_ROOT	"../../../../"













#endif	/* __CONFIG_H__ */
