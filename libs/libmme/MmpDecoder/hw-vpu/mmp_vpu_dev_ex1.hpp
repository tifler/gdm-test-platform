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

#ifndef MMP_VPU_DEV_EX1_HPP__
#define MMP_VPU_DEV_EX1_HPP__

#include "mmp_vpu_dev.hpp"
#include "vpu.h"
#include "mmp_buffer.hpp"
#include "mmp_msg_proc.hpp"
#include "mmp_oal_mutex.hpp"

class mmp_vpu_dev_ex1 : public mmp_vpu_dev {

friend class mmp_vpu_dev;

private:
    Uint32 m_coreIdx;
    
    
    class mmp_msg_res m_msg_res;
    class mmp_oal_mutex* m_p_mutex;
	
    
protected:
    
	mmp_vpu_dev_ex1(struct mmp_vpu_dev_create_config* p_create_config);
	virtual ~mmp_vpu_dev_ex1();

	virtual MMP_RESULT open();
	virtual MMP_RESULT close();

    static void service_stub(void* parm);
    void service();
    void service_msg_proc(class mmp_msg_packet* p_packet);

private:
    
    
    
    
    
    


public:

    virtual Uint32  VPU_IsBusy(Uint32 coreIdx);
	virtual Uint32  VPU_WaitInterrupt(Uint32 coreIdx, int timeout);
	virtual Uint32  VPU_IsInit(Uint32 coreIdx);
	virtual RetCode VPU_Init(Uint32 coreIdx);
	virtual RetCode VPU_InitWithBitcode(Uint32 coreIdx,	const Uint16 *bitcode,	Uint32 size);
	virtual void    VPU_DeInit(Uint32 coreIdx);
	virtual Uint32  VPU_GetOpenInstanceNum(Uint32 coreIdx);
	virtual RetCode VPU_GetVersionInfo(Uint32 coreIdx, Uint32 *versionInfo,	Uint32 *revision, Uint32 *productId);
	virtual void    VPU_ClearInterrupt(Uint32 coreIdx);
	virtual RetCode VPU_SWReset(Uint32 coreIdx,	int resetMode, void *pendingInst);
	virtual RetCode VPU_HWReset(Uint32 coreIdx);
	virtual RetCode VPU_SleepWake(Uint32 coreIdx, int iSleepWake);	
	virtual int     VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num);
	virtual int     VPU_GetFrameBufSize(int width, int height, int mapType,	int format,	DRAMConfig *pDramCfg);
	
    // function for decode
	virtual RetCode VPU_DecOpen(DecHandle *pHandle,	DecOpenParam *pop);
	virtual RetCode VPU_DecClose(DecHandle handle);
	virtual RetCode VPU_DecSetEscSeqInit(DecHandle handle, int escape);
	virtual RetCode VPU_DecGetInitialInfo(DecHandle handle,	DecInitialInfo *info);
	virtual RetCode VPU_DecIssueSeqInit(DecHandle handle);
	virtual RetCode VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo *info);
	virtual RetCode VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray,	int num, int stride, int height, int mapType);
	virtual RetCode VPU_DecGetFrameBuffer(DecHandle handle,	int frameIdx, FrameBuffer *frameBuf);
	virtual RetCode VPU_DecGetBitstreamBuffer(DecHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int *size);
	virtual RetCode VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size);
	virtual RetCode VPU_DecStartOneFrame(DecHandle handle, DecParam *param);
	virtual RetCode VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info);
	virtual RetCode VPU_DecFrameBufferFlush(DecHandle handle);
	virtual RetCode VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr,	int updateWrPtr);
	virtual RetCode VPU_DecClrDispFlag(DecHandle handle, int index);
	virtual RetCode VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * param);
	virtual RetCode VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer);
	
    // function for encode
	virtual RetCode VPU_EncOpen(EncHandle *pHandle,	EncOpenParam *pop);
	virtual RetCode VPU_EncClose(EncHandle handle);
	virtual RetCode VPU_EncGetInitialInfo(EncHandle handle,	EncInitialInfo * info);
	virtual RetCode VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride, int height, int mapType);
	virtual RetCode VPU_EncGetFrameBuffer(EncHandle handle,	int frameIdx, FrameBuffer *frameBuf);
	virtual RetCode VPU_EncGetBitstreamBuffer(EncHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int * size);
	virtual RetCode VPU_EncUpdateBitstreamBuffer(EncHandle handle, int size);
	virtual RetCode VPU_EncStartOneFrame(EncHandle handle, EncParam * param);
	virtual RetCode VPU_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info);
	virtual RetCode VPU_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * param);
	virtual RetCode VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer);

};


#endif
