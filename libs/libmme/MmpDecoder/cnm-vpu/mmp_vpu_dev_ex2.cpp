/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License") { return RETCODE_SUCCESS; }
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

#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "mmp_vpu_dev_ex2.hpp"
#include "MmpUtil.hpp"

#include "mmp_buffer_mgr.hpp"
#include "mmp_lock.hpp"
#include "../coda960.h"

/**********************************************************
class members
**********************************************************/

mmp_vpu_dev_ex2::mmp_vpu_dev_ex2(struct mmp_vpu_dev_create_config* p_create_config) : mmp_vpu_dev(p_create_config)
,m_coreIdx(0)
,m_p_mutex(NULL)
,m_p_mutex_external_cs(NULL)
{
    this->m_vdb_register.virt_addr = (unsigned long)MAP_FAILED;
}

mmp_vpu_dev_ex2::~mmp_vpu_dev_ex2() {

}


MMP_RESULT mmp_vpu_dev_ex2::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    
    /* create mutex */
    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex = mmp_oal_mutex::create_object();
        if(m_p_mutex == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex_external_cs = mmp_oal_mutex::create_object();
        if(m_p_mutex_external_cs == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        mmpResult = mmp_vpu_dev::open();
    }

    if(mmpResult == MMP_SUCCESS) {
        vpu_ret = ::VPU_Init(m_coreIdx);
        if( (vpu_ret == RETCODE_SUCCESS) ||  (vpu_ret == RETCODE_CALLED_BEFORE) ) {
            /*Nothing to do */
        }
        else {
            mmpResult = MMP_FAILURE;
        }
    }

    return mmpResult;
    
}

MMP_RESULT mmp_vpu_dev_ex2::close() {

    ::VPU_DeInit(m_coreIdx);

    mmp_vpu_dev::close();

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
        m_p_mutex = NULL;
    }

    if(m_p_mutex_external_cs != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex_external_cs);
        m_p_mutex_external_cs = NULL;
    }

	return MMP_SUCCESS;
}

Uint32  mmp_vpu_dev_ex2::VPU_IsBusy(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_IsBusy(coreIdx); 
}

Uint32  mmp_vpu_dev_ex2::VPU_WaitInterrupt(Uint32 coreIdx, int timeout) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_WaitInterrupt(coreIdx, timeout); 
}
Uint32  mmp_vpu_dev_ex2::VPU_IsInit(Uint32 coreIdx) {
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_IsInit(coreIdx); 
}
RetCode  mmp_vpu_dev_ex2::VPU_Init(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_Init(coreIdx); 
}
RetCode  mmp_vpu_dev_ex2::VPU_InitWithBitcode(Uint32 coreIdx,	const Uint16 *bitcode,	Uint32 size) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_InitWithBitcode(coreIdx,bitcode,size); 
}
void  mmp_vpu_dev_ex2::VPU_DeInit(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);
    ::VPU_DeInit(coreIdx); 
}

Uint32  mmp_vpu_dev_ex2::VPU_GetOpenInstanceNum(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_GetOpenInstanceNum(coreIdx); 
}

RetCode  mmp_vpu_dev_ex2::VPU_GetVersionInfo(Uint32 coreIdx, Uint32 *versionInfo,	Uint32 *revision, Uint32 *productId) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_GetVersionInfo(coreIdx, versionInfo, revision, productId); 
}

void  mmp_vpu_dev_ex2::VPU_ClearInterrupt(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);
    ::VPU_ClearInterrupt(coreIdx); 
}

RetCode  mmp_vpu_dev_ex2::VPU_SWReset(Uint32 coreIdx,	int resetMode, void *pendingInst) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_SWReset(coreIdx, resetMode, pendingInst); 
}

RetCode  mmp_vpu_dev_ex2::VPU_HWReset(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_HWReset(coreIdx); 
}

RetCode  mmp_vpu_dev_ex2::VPU_SleepWake(Uint32 coreIdx, int iSleepWake) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_SleepWake(coreIdx, iSleepWake); 
}

int  mmp_vpu_dev_ex2::VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_GetMvColBufSize(codStd, width, height, num); 
}

int  mmp_vpu_dev_ex2::VPU_GetFrameBufSize(int width, int height, int mapType,	int format,	DRAMConfig *pDramCfg) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_GetFrameBufSize(width, height, mapType, format,pDramCfg); 
}

// function for decode
RetCode  mmp_vpu_dev_ex2::VPU_DecOpen(DecHandle *pHandle,	DecOpenParam *pop) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecOpen(pHandle,pop); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecClose(DecHandle handle) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecClose(handle); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecSetEscSeqInit(DecHandle handle, int escape) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecSetEscSeqInit(handle, escape); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecGetInitialInfo(DecHandle handle,	DecInitialInfo *info) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecGetInitialInfo(handle,	info); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecIssueSeqInit(DecHandle handle) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecIssueSeqInit(handle); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo *info) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecCompleteSeqInit(handle, info); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray,	int num, int stride, int height, int mapType) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecRegisterFrameBuffer(handle, bufArray,  num, stride, height, mapType); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecGetFrameBuffer(DecHandle handle,	int frameIdx, FrameBuffer *frameBuf) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecGetFrameBuffer(handle, frameIdx, frameBuf); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecGetBitstreamBuffer(DecHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int *size) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecGetBitstreamBuffer(handle,prdPrt, pwrPtr, size); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecUpdateBitstreamBuffer(handle, size); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecStartOneFrame(DecHandle handle, DecParam *param) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecStartOneFrame(handle, param); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecStartOneFrameAndWaitInterrupt(DecHandle handle, DecParam *param, int timeout) { 

    class mmp_lock autolock(m_p_mutex);

    RetCode vpu_ret = RETCODE_SUCCESS;
    int int_reason;

    MMPDEBUGMSG(1, (TEXT("[mmp_vpu_dev_ex2::VPU_DecStartOneFrameAndWaitInterrupt] ln=%d +++++++++  0x%08x"), __LINE__, handle));

    // Start decoding a frame.
    vpu_ret = ::VPU_DecStartOneFrame(handle, param);
	if (vpu_ret != RETCODE_SUCCESS) 
	{
        MMPDEBUGMSG(1, (TEXT("[mmp_vpu_dev_ex2::VPU_DecStartOneFrameAndWaitInterrupt] ln=%d FAIL: ::VPU_DecStartOneFrame (ret=%d)"), __LINE__, vpu_ret));
	}

    while(vpu_ret == RETCODE_SUCCESS) {
    
        int_reason = ::VPU_WaitInterrupt(m_coreIdx, timeout);
        if (int_reason == (Uint32)-1) // timeout
		{
            MMPDEBUGMSG(1, (TEXT("[mmp_vpu_dev_ex2::VPU_DecStartOneFrameAndWaitInterrupt] ln=%d  FAIL: ::VPU_WaitInterrupt   TimeOut "), __LINE__));
			//VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_DecHandle);				
			vpu_ret = RETCODE_FAILURE;
            break;
		}		

        //MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d int_reason=%d"), __LINE__, int_reason));
        if (int_reason)
            ::VPU_ClearInterrupt(m_coreIdx);

    
        if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;		
    }
    
    MMPDEBUGMSG(1, (TEXT("[mmp_vpu_dev_ex2::VPU_DecStartOneFrameAndWaitInterrupt] ln=%d -------  0x%08x"), __LINE__, handle));
    return vpu_ret;
}

RetCode  mmp_vpu_dev_ex2::VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecGetOutputInfo(handle, info); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecFrameBufferFlush(DecHandle handle) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecFrameBufferFlush(handle); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr,	int updateWrPtr) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecSetRdPtr(handle, addr,updateWrPtr); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecClrDispFlag(DecHandle handle, int index) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecClrDispFlag(handle, index); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * param) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecGiveCommand(handle, cmd, param); 
}

RetCode  mmp_vpu_dev_ex2::VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_DecAllocateFrameBuffer(handle, info,frameBuffer); 
}

// function for encode
RetCode  mmp_vpu_dev_ex2::VPU_EncOpen(EncHandle *pHandle,	EncOpenParam *pop) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncOpen(pHandle,	pop); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncClose(EncHandle handle) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncClose(handle); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncGetInitialInfo(EncHandle handle,	EncInitialInfo * info) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncGetInitialInfo(handle,	info); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride, int height, int mapType) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncRegisterFrameBuffer(handle, bufArray, num, stride, height, mapType); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncGetFrameBuffer(EncHandle handle,	int frameIdx, FrameBuffer *frameBuf) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncGetFrameBuffer(handle,frameIdx, frameBuf); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncGetBitstreamBuffer(EncHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int * size) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncGetBitstreamBuffer(handle,	prdPrt, pwrPtr, size); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncUpdateBitstreamBuffer(EncHandle handle, int size) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncUpdateBitstreamBuffer(handle, size); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncStartOneFrame(EncHandle handle, EncParam * param) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncStartOneFrame(handle, param); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncGetOutputInfo(handle, info); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * param) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncGiveCommand(handle, cmd, param); 
}

RetCode  mmp_vpu_dev_ex2::VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { 
    class mmp_lock autolock(m_p_mutex);
    return ::VPU_EncAllocateFrameBuffer(handle, info,	frameBuffer); 
}

/* etc */
int mmp_vpu_dev_ex2::vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { 
    class mmp_lock autolock(m_p_mutex);
    return ::vdi_allocate_dma_memory(core_idx, vb); 
}

void mmp_vpu_dev_ex2::vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { 
    class mmp_lock autolock(m_p_mutex);
    return ::vdi_free_dma_memory(core_idx, vb); 
}

int mmp_vpu_dev_ex2::vdi_register_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) {
    class mmp_lock autolock(m_p_mutex);
    return ::vdi_register_dma_memory(core_idx, vb); 
}

void mmp_vpu_dev_ex2::vdi_unregister_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) {
    class mmp_lock autolock(m_p_mutex);
    ::vdi_unregister_dma_memory(core_idx, vb); 
}

int mmp_vpu_dev_ex2::WriteBsBufFromBufHelper(Uint32 core_idx,  DecHandle handle,  vpu_buffer_t *pVbStream, BYTE *pChunk, int chunkSize, int endian) { 
    class mmp_lock autolock(m_p_mutex);
    return ::WriteBsBufFromBufHelper(core_idx,  handle,  pVbStream, pChunk, chunkSize, endian); 
}

void mmp_vpu_dev_ex2::enter_critical_section() {
    this->m_p_mutex_external_cs->lock();
}
    
void mmp_vpu_dev_ex2::leave_critical_section() {
    this->m_p_mutex_external_cs->unlock();
}
