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

#ifndef MMP_VPU_DEF_H__
#define MMP_VPU_DEF_H__

#include "MmpDefine.h"

#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include "vpu.h"


#include "vpuapi.h"
#include "vpuapi_mme.h"
#include "vpurun.h"
#include "vpuapifunc.h"
#include "vpuhelper.h"

#define MMP_VPU_CNM     1 
#define MMP_VPU_ANAPASS 2 

#define MMP_VPU MMP_VPU_ANAPASS
#define VPU_DEVICE_NAME "/dev/vpu"

#	define SUPPORT_INTERRUPT
#	define VPU_BIT_REG_SIZE	(0x4000*MAX_NUM_VPU_CORE)
#		define VDI_SRAM_BASE_ADDR	ANA_CODEC_SRAM_BASE	// if we can know the sram address in SOC directly for vdi layer. it is possible to set in vdi layer without allocation from driver
#		define VDI_SRAM_SIZE		0x20000		// FHD MAX size, 0x17D00  4K MAX size 0x34600
#	define VDI_SYSTEM_ENDIAN VDI_LITTLE_ENDIAN
#define VDI_NUM_LOCK_HANDLES				4

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define VPU_CORE_BASE_OFFSET 0x4000
#endif


/* MMP VPU Message Definition */
#define MMP_MSG_VPU_BASE 0x1000

#define MMP_MSG_VPU_IsBusy                          (MMP_MSG_VPU_BASE+0x00)
#define MMP_MSG_VPU_WaitInterrupt                   (MMP_MSG_VPU_BASE+0x01)
#define MMP_MSG_VPU_IsInit                          (MMP_MSG_VPU_BASE+0x02)
#define MMP_MSG_VPU_Init                            (MMP_MSG_VPU_BASE+0x03)
#define MMP_MSG_VPU_InitWithBitcode                 (MMP_MSG_VPU_BASE+0x04)
#define MMP_MSG_VPU_DeInit                          (MMP_MSG_VPU_BASE+0x05)
#define MMP_MSG_VPU_GetOpenInstanceNum              (MMP_MSG_VPU_BASE+0x06)
#define MMP_MSG_VPU_GetVersionInfo                  (MMP_MSG_VPU_BASE+0x07)
#define MMP_MSG_VPU_ClearInterrupt                  (MMP_MSG_VPU_BASE+0x08)
#define MMP_MSG_VPU_SWReset                         (MMP_MSG_VPU_BASE+0x09)
#define MMP_MSG_VPU_HWReset                         (MMP_MSG_VPU_BASE+0x0A)
#define MMP_MSG_VPU_SleepWake                       (MMP_MSG_VPU_BASE+0x0B)
#define MMP_MSG_VPU_GetMvColBufSize                 (MMP_MSG_VPU_BASE+0x0C)
#define MMP_MSG_VPU_GetFrameBufSize                 (MMP_MSG_VPU_BASE+0x0D)

// function for decode
#define MMP_MSG_VPU_DecOpen                         (MMP_MSG_VPU_BASE+0x0E)
#define MMP_MSG_VPU_DecClose                        (MMP_MSG_VPU_BASE+0x0F)    
#define MMP_MSG_VPU_DecSetEscSeqInit                (MMP_MSG_VPU_BASE+0x10)
#define MMP_MSG_VPU_DecGetInitialInfo               (MMP_MSG_VPU_BASE+0x11)
#define MMP_MSG_VPU_DecIssueSeqInit                 (MMP_MSG_VPU_BASE+0x12)
#define MMP_MSG_VPU_DecCompleteSeqInit              (MMP_MSG_VPU_BASE+0x13)
#define MMP_MSG_VPU_DecRegisterFrameBuffer          (MMP_MSG_VPU_BASE+0x14)    
#define MMP_MSG_VPU_DecGetFrameBuffer               (MMP_MSG_VPU_BASE+0x15)
#define MMP_MSG_VPU_DecGetBitstreamBuffer           (MMP_MSG_VPU_BASE+0x16)    
#define MMP_MSG_VPU_DecUpdateBitstreamBuffer        (MMP_MSG_VPU_BASE+0x17)
#define MMP_MSG_VPU_DecStartOneFrame                (MMP_MSG_VPU_BASE+0x18)
#define MMP_MSG_VPU_DecGetOutputInfo                (MMP_MSG_VPU_BASE+0x19)
#define MMP_MSG_VPU_DecFrameBufferFlush             (MMP_MSG_VPU_BASE+0x1A)
#define MMP_MSG_VPU_DecSetRdPtr                     (MMP_MSG_VPU_BASE+0x1B)
#define MMP_MSG_VPU_DecClrDispFlag                  (MMP_MSG_VPU_BASE+0x1C)
#define MMP_MSG_VPU_DecGiveCommand                  (MMP_MSG_VPU_BASE+0x1D)
#define MMP_MSG_VPU_DecAllocateFrameBuffer          (MMP_MSG_VPU_BASE+0x1E)    

// function for encode
#define MMP_MSG_VPU_EncOpen                         (MMP_MSG_VPU_BASE+0x1F)
#define MMP_MSG_VPU_EncClose                        (MMP_MSG_VPU_BASE+0x20)
#define MMP_MSG_VPU_EncGetInitialInfo               (MMP_MSG_VPU_BASE+0x21)
#define MMP_MSG_VPU_EncRegisterFrameBuffer          (MMP_MSG_VPU_BASE+0x22)
#define MMP_MSG_VPU_EncGetFrameBuffer               (MMP_MSG_VPU_BASE+0x23)
#define MMP_MSG_VPU_EncGetBitstreamBuffer           (MMP_MSG_VPU_BASE+0x24)
#define MMP_MSG_VPU_EncUpdateBitstreamBuffer        (MMP_MSG_VPU_BASE+0x25)
#define MMP_MSG_VPU_EncStartOneFrame                (MMP_MSG_VPU_BASE+0x26)
#define MMP_MSG_VPU_EncGetOutputInfo                (MMP_MSG_VPU_BASE+0x27)
#define MMP_MSG_VPU_EncGiveCommand                  (MMP_MSG_VPU_BASE+0x28)
#define MMP_MSG_VPU_EncAllocateFrameBuffer          (MMP_MSG_VPU_BASE+0x29)

#endif
