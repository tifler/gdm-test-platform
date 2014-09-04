/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MMPOSDEFINE_LINUX_H__
#define _MMPOSDEFINE_LINUX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>

#define MMP_ALIGN_8 
#define MMP_DLL_EXPORT //extern _declspec(dllexport)

#if (MMP_OS_LINUX == MMP_OS_LINUX_ANDROID)
#include <utils/Log.h>
#endif

#if (MMP_OS_LINUX == MMP_OS_LINUX_ANDROID)
#define MMPDEBUGMSG(cond,printf_exp) do { if(cond) ALOGI printf_exp; }while(0);
#else
//#define MMPDEBUGMSG(cond,printf_exp) do { if(cond) printf printf_exp; }while(0);
#define MMPDEBUGMSG(cond,printf_exp) do { if(cond) CMmpUtil::Printf printf_exp; }while(0);
#endif

typedef char* MMPSTR;
typedef char MMPCHAR;
typedef char WCHAR;
typedef char TCHAR;
//#define __func__ "__FUNC__"

//#define HKEY_DRIVER_ROOT HKEY_CURRENT_USER
//#define REGKEY_OPEN_DESIRED_VALUE   KEY_ALL_ACCESS

#define __MmpApiCall  //__stdcall //__stdcall __cdecl __fastcall

//#define MMP_CPU MMP_CPU_ZENITH1
//#define MMP_CPU MMP_CPU_MV8770
//#define MMP_DEVCONFIG MMP_DEVCONFIG_DRIVER

#define TEXT(x) (char*)x

//#define INREG32(reg) (*(volatile unsigned long * const)(reg))
//#define SETREG32(reg, mask) (*(volatile unsigned long * const)(reg) |=  mask)
//#define CLRREG32(reg, mask) (*(volatile unsigned long * const)(reg) &= ~mask)


#ifdef __cplusplus
#define MMP_DRIVER_OPEN    ::open
#define MMP_DRIVER_CLOSE   ::close
#define MMP_DRIVER_WRITE   ::write
#define MMP_DRIVER_MMAP    ::mmap
#define MMP_DRIVER_MUNMAP  ::munmap
#define MMP_DRIVER_IOCTL   ::ioctl

#else
#define MMP_DRIVER_OPEN    open
#define MMP_DRIVER_CLOSE   close
#define MMP_DRIVER_WRITE   write
#define MMP_DRIVER_MMAP    mmap
#define MMP_DRIVER_MUNMAP  munmap
#define MMP_DRIVER_IOCTL   ioctl
#endif

#endif

