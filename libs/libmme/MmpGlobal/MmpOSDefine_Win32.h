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

#ifndef _MMPOSDEFINE_WIN32_H__
#define _MMPOSDEFINE_WIN32_H__

#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include <tchar.h>
#include <mmsystem.h>
#include <wchar.h>
#include <errno.h>

#define MMP_ALIGN_8 
#define MMP_DLL_EXPORT extern _declspec(dllexport)

#define MMPDEBUG
#ifdef MMPDEBUG
//#define MMPDEBUGMSG(cond,printf_exp) ((void)((cond)?(CMmpDebug::GetInstance()->Printf printf_exp ),1:0))
#define MMPDEBUGMSG(cond,printf_exp) ((void)((cond)?(CMmpUtil::Printf printf_exp ),1:0))
//#define MMPDEBUGMSG RETAILMSG
#else
#define MMPDEBUGMSG(a,b) //DEBUGMSG
#endif

typedef TCHAR* MMPSTR;
typedef TCHAR MMPCHAR;

#define HKEY_DRIVER_ROOT HKEY_CURRENT_USER
#define REGKEY_OPEN_DESIRED_VALUE   KEY_ALL_ACCESS

#define __MmpApiCall  __stdcall //__stdcall __cdecl __fastcall

#define MMP_CPU MMP_CPU_ZENITH1
//#define MMP_CPU MMP_CPU_MV8770
#define MMP_DEVCONFIG MMP_DEVCONFIG_DRIVER

#define INREG32(reg) (*(volatile unsigned long * const)(reg))
#define SETREG32(reg, mask) (*(volatile unsigned long * const)(reg) |=  mask)
#define CLRREG32(reg, mask) (*(volatile unsigned long * const)(reg) &= ~mask)

#define MMP_FILE_DEVIDER '\\'

/* linux driver io */
#define MMP_DRIVER_OPEN  kernel_driver_open
#define MMP_DRIVER_CLOSE kernel_driver_close
#define MMP_DRIVER_WRITE kernel_driver_write
#define MMP_DRIVER_IOCTL kernel_driver_ioctl
#define MMP_DRIVER_MMAP  kernel_driver_mmap
#define MMP_DRIVER_MUNMAP  kernel_driver_munmap

#ifdef __cplusplus
extern "C" {
#endif

int kernel_driver_open(char* drvname, int flag);
int kernel_driver_close(int fd);
int kernel_driver_write(int fd, char* buf, int bufsize);
int kernel_driver_ioctl(int d, unsigned long request, void* arg);

void* kernel_driver_mmap(void *addr, size_t length, int prot, int flags, int fd, unsigned int offset);
int kernel_driver_munmap(void* start, size_t length);

#ifdef __cplusplus
}
#endif

#endif

