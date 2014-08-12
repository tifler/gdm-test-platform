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

#ifndef _MMPOSDEFINE_WINCE_H__
#define _MMPOSDEFINE_WINCE_H__

#include <windows.h>

typedef int MMP_RESULT;
#define MMP_SUCCESS                                                          (0)
#define MMP_FAILURE                                                          (-1)

typedef int MMP_BOOL;
#define MMP_TRUE 1
#define MMP_FALSE 0



#define MMPDEBUG

#ifdef MMPDEBUG
#define MMPDEBUGMSG RETAILMSG
#else
#define MMPDEBUGMSG(a,b) //DEBUGMSG
#endif

typedef WCHAR* MMPSTR;
typedef WCHAR MMPCHAR;
#define __func__ "__FUNC__"

#define HKEY_DRIVER_ROOT HKEY_LOCAL_MACHINE
#define REGKEY_OPEN_DESIRED_VALUE   0

#define __MmpApiCall  //__stdcall __cdecl __fastcall

#define MMP_CPU MMP_CPU_ZENITH1
//#define MMP_CPU MMP_CPU_MV8770
#ifdef WINCE60_APP
#define MMP_DEVCONFIG MMP_DEVCONFIG_APP
#else
#define MMP_DEVCONFIG MMP_DEVCONFIG_DRIVER
#endif

#if (MMP_DEVCONFIG==MMP_DEVCONFIG_APP)
#define INREG32(reg) (*(volatile unsigned long * const)(reg))
#define SETREG32(reg, mask) (*(volatile unsigned long * const)(reg) |=  mask)
#define CLRREG32(reg, mask) (*(volatile unsigned long * const)(reg) &= ~mask)
#endif

#endif

