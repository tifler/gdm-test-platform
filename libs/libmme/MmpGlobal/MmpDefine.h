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

#ifndef __MMPGLOBAL_H__
#define __MMPGLOBAL_H__

#include "MmpDefine_Type.h"

#define MMP_DEBUG_LEVEL 1  // 0: No Debug Msg  1: Output Debug Msg

/**********************************************************************************
  Platform/OS Definition
***********************************************************************************/

#define MMP_OS_WINCE60                 0x0FF
#define MMP_OS_WIN32                   0x100

#define MMP_OS_LINUX_START             0xAA00
#define MMP_OS_LINUX_X86               (MMP_OS_LINUX_START+1)
#define MMP_OS_LINUX_ARM               (MMP_OS_LINUX_START+2)
#define MMP_OS_LINUX_ANDROID           (MMP_OS_LINUX_START+3)
#define MMP_OS_LINUX_DIAMOND_ANDROID43 MMP_OS_LINUX_ANDROID
#define MMP_OS_LINUX_DIAMOND_ANDROID44 MMP_OS_LINUX_ANDROID
#define MMP_OS_LINUX_ODYSSEUS_FPGA     (MMP_OS_LINUX_START+10)
#define MMP_OS_LINUX_END               0xAB00

#ifdef __PLATFORM_DIAMOND_ANDROID43
#define MMP_OS_LINUX MMP_OS_LINUX_DIAMOND_ANDROID43
#elif defined(__PLATFORM_DIAMOND_ANDROID44)
#define MMP_OS_LINUX MMP_OS_LINUX_DIAMOND_ANDROID44
#elif defined(__PLATFORM_ODY_FPGA)
#define MMP_OS_LINUX MMP_OS_LINUX_ODYSSEUS_FPGA
#else
#define MMP_OS_LINUX    0
#endif

/**********************************************************************************
  Platform/OS Selection
***********************************************************************************/

#if ((MMP_OS_LINUX >= MMP_OS_LINUX_START) && (MMP_OS_LINUX <= MMP_OS_LINUX_END))
#define MMP_OS MMP_OS_LINUX

#else

#ifdef UNDER_CE
#define MMP_OS MMP_OS_WINCE60  

#elif defined ( WIN32 )
#define MMP_OS MMP_OS_WIN32

#else
#error "ERROR : Select OS in MmpDefine.h"
#endif

#endif

/**********************************************************************************
  Platform/OS Configuration
***********************************************************************************/

#if( MMP_OS==MMP_OS_WINCE60)  //WinCE 6.0 Kernel Level Config
#include "MmpOSDefine_WinCE.h"

#elif( MMP_OS==MMP_OS_WIN32) //Win32 App Level Config
#include "MmpOSDefine_Win32.h"

#elif( MMP_OS==MMP_OS_LINUX) //Nucleus App/Driver Config
#include "MmpOSDefine_Linux.h"

#else
#error "ERROR Select MMP_OS"
#endif//#ifdef MMPDEBUG

/**********************************************************************************
  HW Codec Video Selection
***********************************************************************************/

#define MMP_HWCODEC_VIDEO_NONE                  0x00
#define MMP_HWCODEC_VIDEO_SW                    0x10
#define MMP_HWCODEC_VIDEO_VPU                   0x20
#define MMP_HWCODEC_VIDEO_EXYNOS4_MFC           0x30  /* Exynos4 Android 4.3 */
#define MMP_HWCODEC_VIDEO_EXYNOS4_MFC_ANDROID44 0x31  /* Exynos4 Android 4.4 */

#if (MMP_OS == MMP_OS_LINUX_ODYSSEUS_FPGA)
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_VPU

#elif (MMP_OS == MMP_OS_LINUX_ANDROID)
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_EXYNOS4_MFC
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_NONE

#elif (MMP_OS == MMP_OS_WIN32)
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_VPU

#else
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_NONE

#endif

/**********************************************************************************
  HW Codec Image Selection  ( Imgae : OpenMax IL Term)
***********************************************************************************/

#define MMP_HWCODEC_IMAGE_NONE               0x00
#define MMP_HWCODEC_IMAGE_SW                 0x10
#define MMP_HWCODEC_IMAGE_JPU                0x20

#if (MMP_OS == MMP_OS_LINUX_ODYSSEUS_FPGA)
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_JPU

#elif (MMP_OS == MMP_OS_LINUX_ANDROID)
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_JPU

#elif (MMP_OS == MMP_OS_WIN32)
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_JPU
//#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_SW

#else
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_NONE

#endif

/**********************************************************************************
  Debug Option Define
***********************************************************************************/

#define MMPZONE_INIT               1
#define MMPZONE_REGISTERS          1
#define MMPZONE_ERROR              1
#define MMPZONE_VERBOSE            1
#define MMPZONE_WARNING            1
#define MMPZONE_UNUSED             1
#define MMPZONE_MONITOR            1
#define MMPZONE_INFO               1
#define MMPZONE_ISR                0


#define UnusedParameter(x)  x = x


#define IF_SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


enum MMP_PIXELFORMAT
{
    MMP_PIXELFORMAT_UNKNOWN=0,
    MMP_PIXELFORMAT_RGB565,
    MMP_PIXELFORMAT_RGB24,   //24Bit
    MMP_PIXELFORMAT_RGB32, //32Bit
    MMP_PIXELFORMAT_YUV420_PACKED, // samples are packed  together into macropixels which ar stored in a single array
    MMP_PIXELFORMAT_YUV420_PLANAR, // where each component is stored as a separate array
    MMP_PIXELFORMAT_YUV420_PHYADDR, // yuv420, physical address
    MMP_PIXELFORMAT_RGB565_PHYADDR,
    MMP_PIXELFORMAT_STREAM,
    MMP_PIXELFORMAT_SAMSUNG_NV12
 };

enum MMP_MEDIATYPE
{
    MMP_MEDIATYPE_UNKNOWN=-1,
    MMP_MEDIATYPE_AUDIO=0,
    MMP_MEDIATYPE_VIDEO,
    MMP_MEDIATYPE_MAX
};


#define MMP_MAX(a,b) ((a) > (b) ? (a) : (b))

#define MMP_SWAP_I16(v) (short)((((v)>>8)  & 0xff) | (((v)&0xff) << 8))
#define MMP_SWAP_I32(v) (int)((((v)>>24) & 0xff) | (((v)>>8) & 0xff00) | (((v)&0xff00) << 8) | (((v)&0xff) << 24))
#define MMP_SWAP_U16(v) (unsigned short)((((v)>>8)  & 0xff) | (((v)&0xff) << 8))
#define MMP_SWAP_U32(v) (unsigned int)((((v)>>24) & 0xff) | (((v)>>8) & 0xff00) | (((v)&0xff00) << 8) | (((v)&0xff) << 24))

#define MMP_PUT_BYTE(_p, _b) \
    *_p++ = (unsigned char)_b; 

#define MMP_PUT_BUFFER(_p, _buf, _len) \
    memcpy(_p, _buf, _len); \
    _p += _len;

#define MMP_PUT_LE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>16); \
    *_p++ = (unsigned char)((_var)>>24); 

#define MMP_PUT_BE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>24);  \
    *_p++ = (unsigned char)((_var)>>16);  \
    *_p++ = (unsigned char)((_var)>>8); \
    *_p++ = (unsigned char)((_var)>>0); 


#define MMP_PUT_LE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  


#define MMP_PUT_BE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>0);  



#define MMPMAKEFOURCC(ch0, ch1, ch2, ch3)                           \
                ((MMP_U32)(MMP_U8)(ch0) | ((MMP_U32)(MMP_U8)(ch1) << 8) |   \
                ((MMP_U32)(MMP_U8)(ch2) << 16) | ((MMP_U32)(MMP_U8)(ch3) << 24 ))
#define MMPGETFOURCC(fourcc,ch) (MMP_U8)(fourcc>>(8*ch))
#define MMPGETFOURCCARG(fourcc)  MMPGETFOURCC(fourcc,0),MMPGETFOURCC(fourcc,1),MMPGETFOURCC(fourcc,2),MMPGETFOURCC(fourcc,3)
#define MMP_BYTE_ALIGN(x, align)   (((x) + (align) - 1) & ~((align) - 1))

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60)
#define MMP_SLEEP(mileSec) Sleep(mileSec)
#elif (MMP_OS==MMP_OS_NUCLEUS)
#define MMP_SLEEP(mileSec) OAL_TaskSleep(mileSec*1000);
#elif (MMP_OS==MMP_OS_LINUX)
#define MMP_SLEEP(mileSec) usleep(mileSec*1000);
#else
#error "ERROR: Define MMP_SLEEP"
#endif

#include "MmpDefine_Audio.h"
#include "MmpDefine_Video.h"

#include "MmpDefine_Ffmpeg.h"
#include "MmpDefine_OMXComp.h"

#define MMP_FILENAME_MAX_LEN 256

/* In, Out */
#define MMP_IN
#define MMP_OUT
#define MMP_INOUT

/* malloc/free/memset/memcpy */
#define MMP_MALLOC(x) malloc(x)
#define MMP_FREE(x) free(x)
#define MMP_MEMSET(ptr, v, sz) memset(ptr, v, sz)
#define MMP_MEMCPY(dst, src, sz) memcpy(dst, src, sz)

#endif // #ifndef __MMPGLOBAL_H__

