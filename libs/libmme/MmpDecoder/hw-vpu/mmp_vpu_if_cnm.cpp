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

#include "mmp_vpu_if_cnm.hpp"
#include "MmpUtil.hpp"


/**********************************************************
class members
**********************************************************/

mmp_vpu_if_cnm::mmp_vpu_if_cnm(struct mmp_vpu_if_create_config* p_create_config) : mmp_vpu_if(p_create_config)
{

}

mmp_vpu_if_cnm::~mmp_vpu_if_cnm() {

}


MMP_RESULT mmp_vpu_if_cnm::open() {

	MMP_RESULT ret = MMP_SUCCESS;
	
	return ret;
}

MMP_RESULT mmp_vpu_if_cnm::close() {
		
	return MMP_SUCCESS;
}

Uint32  mmp_vpu_if_cnm::VPU_IsBusy(Uint32 coreIdx) { return ::VPU_IsBusy(coreIdx); }
Uint32  mmp_vpu_if_cnm::VPU_WaitInterrupt(Uint32 coreIdx, int timeout) { return ::VPU_WaitInterrupt(coreIdx, timeout); }
Uint32  mmp_vpu_if_cnm::VPU_IsInit(Uint32 coreIdx) { return ::VPU_IsInit(coreIdx); }
RetCode  mmp_vpu_if_cnm::VPU_Init(Uint32 coreIdx) { return ::VPU_Init(coreIdx); }
RetCode  mmp_vpu_if_cnm::VPU_InitWithBitcode(Uint32 coreIdx,	const Uint16 *bitcode,	Uint32 size) { return ::VPU_InitWithBitcode(coreIdx,bitcode,size); }
void  mmp_vpu_if_cnm::VPU_DeInit(Uint32 coreIdx) { ::VPU_DeInit(coreIdx); }
Uint32  mmp_vpu_if_cnm::VPU_GetOpenInstanceNum(Uint32 coreIdx) { return ::VPU_GetOpenInstanceNum(coreIdx); }
RetCode  mmp_vpu_if_cnm::VPU_GetVersionInfo(Uint32 coreIdx, Uint32 *versionInfo,	Uint32 *revision, Uint32 *productId) { return ::VPU_GetVersionInfo(coreIdx, versionInfo, revision, productId); }
void  mmp_vpu_if_cnm::VPU_ClearInterrupt(Uint32 coreIdx) { ::VPU_ClearInterrupt(coreIdx); }
RetCode  mmp_vpu_if_cnm::VPU_SWReset(Uint32 coreIdx,	int resetMode, void *pendingInst) { return ::VPU_SWReset(coreIdx, resetMode, pendingInst); }
RetCode  mmp_vpu_if_cnm::VPU_HWReset(Uint32 coreIdx) { return ::VPU_HWReset(coreIdx); }
RetCode  mmp_vpu_if_cnm::VPU_SleepWake(Uint32 coreIdx, int iSleepWake) { return ::VPU_SleepWake(coreIdx, iSleepWake); }	
int  mmp_vpu_if_cnm::VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num) { return ::VPU_GetMvColBufSize(codStd, width, height, num); }
int  mmp_vpu_if_cnm::VPU_GetFrameBufSize(int width, int height, int mapType,	int format,	DRAMConfig *pDramCfg) { return ::VPU_GetFrameBufSize(width, height, mapType, format,pDramCfg); }

// function for decode
RetCode  mmp_vpu_if_cnm::VPU_DecOpen(DecHandle *pHandle,	DecOpenParam *pop) { return ::VPU_DecOpen(pHandle,pop); }
RetCode  mmp_vpu_if_cnm::VPU_DecClose(DecHandle handle) { return ::VPU_DecClose(handle); }
RetCode  mmp_vpu_if_cnm::VPU_DecSetEscSeqInit(DecHandle handle, int escape) { return ::VPU_DecSetEscSeqInit(handle, escape); }
RetCode  mmp_vpu_if_cnm::VPU_DecGetInitialInfo(DecHandle handle,	DecInitialInfo *info) { return ::VPU_DecGetInitialInfo(handle,	info); }
RetCode  mmp_vpu_if_cnm::VPU_DecIssueSeqInit(DecHandle handle) { return ::VPU_DecIssueSeqInit(handle); }
RetCode  mmp_vpu_if_cnm::VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo *info) { return ::VPU_DecCompleteSeqInit(handle, info); }
RetCode  mmp_vpu_if_cnm::VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray,	int num, int stride, int height, int mapType) { return ::VPU_DecRegisterFrameBuffer(handle, bufArray,  num, stride, height, mapType); }
RetCode  mmp_vpu_if_cnm::VPU_DecGetFrameBuffer(DecHandle handle,	int frameIdx, FrameBuffer *frameBuf) { return ::VPU_DecGetFrameBuffer(handle, frameIdx, frameBuf); }
RetCode  mmp_vpu_if_cnm::VPU_DecGetBitstreamBuffer(DecHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int *size) { return ::VPU_DecGetBitstreamBuffer(handle,prdPrt, pwrPtr, size); }
RetCode  mmp_vpu_if_cnm::VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size) { return ::VPU_DecUpdateBitstreamBuffer(handle, size); }
RetCode  mmp_vpu_if_cnm::VPU_DecStartOneFrame(DecHandle handle, DecParam *param) { return ::VPU_DecStartOneFrame(handle, param); }
RetCode  mmp_vpu_if_cnm::VPU_DecStartOneFrameAndWaitInterrupt(DecHandle handle, DecParam *param, int timeout) { 

    RetCode vpu_ret = RETCODE_SUCCESS;
    int int_reason;

    // Start decoding a frame.
    vpu_ret = ::VPU_DecStartOneFrame(handle, param);
	if (vpu_ret != RETCODE_SUCCESS) 
	{
        MMPDEBUGMSG(1, (TEXT("[mmp_vpu_if_cnm::VPU_DecStartOneFrameAndWaitInterrupt] ln=%d FAIL: ::VPU_DecStartOneFrame"), __LINE__));
	}

    while(vpu_ret == RETCODE_SUCCESS) {
    
        int_reason = ::VPU_WaitInterrupt(0, timeout);
        if (int_reason == (Uint32)-1) // timeout
		{
            MMPDEBUGMSG(1, (TEXT("[mmp_vpu_if_cnm::VPU_DecStartOneFrameAndWaitInterrupt] ln=%d  FAIL: ::VPU_WaitInterrupt   TimeOut "), __LINE__));
			//VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_DecHandle);				
			vpu_ret = RETCODE_FAILURE;
            break;
		}		

        //MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d int_reason=%d"), __LINE__, int_reason));
        if (int_reason)
            ::VPU_ClearInterrupt(0);

    
        if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;		
    }
    
    return vpu_ret;
}

RetCode  mmp_vpu_if_cnm::VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info) { return ::VPU_DecGetOutputInfo(handle, info); }
RetCode  mmp_vpu_if_cnm::VPU_DecFrameBufferFlush(DecHandle handle) { return ::VPU_DecFrameBufferFlush(handle); }
RetCode  mmp_vpu_if_cnm::VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr,	int updateWrPtr) { return ::VPU_DecSetRdPtr(handle, addr,updateWrPtr); }
RetCode  mmp_vpu_if_cnm::VPU_DecClrDispFlag(DecHandle handle, int index) { return ::VPU_DecClrDispFlag(handle, index); }
RetCode  mmp_vpu_if_cnm::VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * param) { return ::VPU_DecGiveCommand(handle, cmd, param); }
RetCode  mmp_vpu_if_cnm::VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { return ::VPU_DecAllocateFrameBuffer(handle, info,frameBuffer); }

// function for encode
RetCode  mmp_vpu_if_cnm::VPU_EncOpen(EncHandle *pHandle,	EncOpenParam *pop) { return ::VPU_EncOpen(pHandle,	pop); }
RetCode  mmp_vpu_if_cnm::VPU_EncClose(EncHandle handle) { return ::VPU_EncClose(handle); }
RetCode  mmp_vpu_if_cnm::VPU_EncGetInitialInfo(EncHandle handle,	EncInitialInfo * info) { return ::VPU_EncGetInitialInfo(handle,	info); }
RetCode  mmp_vpu_if_cnm::VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride, int height, int mapType) { return ::VPU_EncRegisterFrameBuffer(handle, bufArray, num, stride, height, mapType); }
RetCode  mmp_vpu_if_cnm::VPU_EncGetFrameBuffer(EncHandle handle,	int frameIdx, FrameBuffer *frameBuf) { return ::VPU_EncGetFrameBuffer(handle,frameIdx, frameBuf); }
RetCode  mmp_vpu_if_cnm::VPU_EncGetBitstreamBuffer(EncHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int * size) { return ::VPU_EncGetBitstreamBuffer(handle,	prdPrt, pwrPtr, size); }
RetCode  mmp_vpu_if_cnm::VPU_EncUpdateBitstreamBuffer(EncHandle handle, int size) { return ::VPU_EncUpdateBitstreamBuffer(handle, size); }
RetCode  mmp_vpu_if_cnm::VPU_EncStartOneFrame(EncHandle handle, EncParam * param) { return ::VPU_EncStartOneFrame(handle, param); }
RetCode  mmp_vpu_if_cnm::VPU_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info) { return ::VPU_EncGetOutputInfo(handle, info); }
RetCode  mmp_vpu_if_cnm::VPU_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * param) { return ::VPU_EncGiveCommand(handle, cmd, param); }
RetCode  mmp_vpu_if_cnm::VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { return ::VPU_EncAllocateFrameBuffer(handle, info,	frameBuffer); }

/* etc */
int mmp_vpu_if_cnm::vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { return ::vdi_allocate_dma_memory(core_idx, vb); }
void mmp_vpu_if_cnm::vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) {  ::vdi_free_dma_memory(core_idx, vb); } 

int mmp_vpu_if_cnm::vdi_register_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { return ::vdi_register_dma_memory(core_idx, vb); }
void mmp_vpu_if_cnm::vdi_unregister_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { ::vdi_unregister_dma_memory(core_idx, vb); }

int mmp_vpu_if_cnm::WriteBsBufFromBufHelper(Uint32 core_idx,  DecHandle handle,  vpu_buffer_t *pVbStream, BYTE *pChunk, int chunkSize, int endian) { return ::WriteBsBufFromBufHelper(core_idx,  handle,  pVbStream, pChunk, chunkSize, endian); }