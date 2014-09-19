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

#include "mmp_vpu_sample.hpp"


/**********************************************************
class members
**********************************************************/

mmp_vpu_sample::mmp_vpu_sample(struct mmp_vpu_create_config* p_create_config) : mmp_vpu(p_create_config)
{

}

mmp_vpu_sample::~mmp_vpu_sample() {

}


MMP_RESULT mmp_vpu_sample::open() {

	MMP_RESULT ret = MMP_SUCCESS;
	
	return ret;
}

MMP_RESULT mmp_vpu_sample::close() {
		
	return MMP_SUCCESS;
}

Uint32  mmp_vpu_sample::VPU_IsBusy(Uint32 coreIdx) { return RETCODE_SUCCESS; }
Uint32  mmp_vpu_sample::VPU_WaitInterrupt(Uint32 coreIdx, int timeout) { return RETCODE_SUCCESS; }
Uint32  mmp_vpu_sample::VPU_IsInit(Uint32 coreIdx) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_Init(Uint32 coreIdx) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_InitWithBitcode(Uint32 coreIdx,	const Uint16 *bitcode,	Uint32 size) { return RETCODE_SUCCESS; }
void  mmp_vpu_sample::VPU_DeInit(Uint32 coreIdx) {  }
Uint32  mmp_vpu_sample::VPU_GetOpenInstanceNum(Uint32 coreIdx) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_GetVersionInfo(Uint32 coreIdx, Uint32 *versionInfo,	Uint32 *revision, Uint32 *productId) { return RETCODE_SUCCESS; }
void  mmp_vpu_sample::VPU_ClearInterrupt(Uint32 coreIdx) {  }
RetCode  mmp_vpu_sample::VPU_SWReset(Uint32 coreIdx,	int resetMode, void *pendingInst) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_HWReset(Uint32 coreIdx) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_SleepWake(Uint32 coreIdx, int iSleepWake) { return RETCODE_SUCCESS; }	
int  mmp_vpu_sample::VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num) { return RETCODE_SUCCESS; }
int  mmp_vpu_sample::VPU_GetFrameBufSize(int width, int height, int mapType,	int format,	DRAMConfig *pDramCfg) { return RETCODE_SUCCESS; }

// function for decode
RetCode  mmp_vpu_sample::VPU_DecOpen(DecHandle *pHandle,	DecOpenParam *pop) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecClose(DecHandle handle) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecSetEscSeqInit(DecHandle handle, int escape) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecGetInitialInfo(DecHandle handle,	DecInitialInfo *info) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecIssueSeqInit(DecHandle handle) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo *info) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray,	int num, int stride, int height, int mapType) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecGetFrameBuffer(DecHandle handle,	int frameIdx, FrameBuffer *frameBuf) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecGetBitstreamBuffer(DecHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int *size) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecStartOneFrame(DecHandle handle, DecParam *param) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecFrameBufferFlush(DecHandle handle) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr,	int updateWrPtr) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecClrDispFlag(DecHandle handle, int index) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * param) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { return RETCODE_SUCCESS; }

// function for encode
RetCode  mmp_vpu_sample::VPU_EncOpen(EncHandle *pHandle,	EncOpenParam *pop) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncClose(EncHandle handle) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncGetInitialInfo(EncHandle handle,	EncInitialInfo * info) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride, int height, int mapType) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncGetFrameBuffer(EncHandle handle,	int frameIdx, FrameBuffer *frameBuf) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncGetBitstreamBuffer(EncHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int * size) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncUpdateBitstreamBuffer(EncHandle handle, int size) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncStartOneFrame(EncHandle handle, EncParam * param) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * param) { return RETCODE_SUCCESS; }
RetCode  mmp_vpu_sample::VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { return RETCODE_SUCCESS; }
