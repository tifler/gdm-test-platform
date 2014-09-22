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

#include "mmp_vpu_dev_ex1.hpp"
#include "MmpUtil.hpp"

#include "mmp_buffer_mgr.hpp"
#include "mmp_lock.hpp"
#include "../coda960.h"

/**********************************************************
class members
**********************************************************/

mmp_vpu_dev_ex1::mmp_vpu_dev_ex1(struct mmp_vpu_dev_create_config* p_create_config) : mmp_vpu_dev(p_create_config)
,m_coreIdx(0)
,m_msg_res(256)
{
    this->m_vdb_register.virt_addr = (unsigned long)MAP_FAILED;
}

mmp_vpu_dev_ex1::~mmp_vpu_dev_ex1() {

}


MMP_RESULT mmp_vpu_dev_ex1::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U32 data;

    MMPDEBUGMSG(1, (TEXT("[mmp_vpu_dev_ex1::open]  ")));
    
    /* create mutex */
    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex = mmp_oal_mutex::create_object();
        if(m_p_mutex == NULL) {
            mmpResult = MMP_SUCCESS;
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        mmpResult = mmp_vpu_dev::open();
    }
        

#if 0
    /* msg proc */
	if(mmpResult == MMP_SUCCESS) {
        /* check msg queue */
        mmpResult = m_msg_res.open(NULL, NULL);
    }
#endif

    const Uint16 *codeWord = bit_code;
    int codeSize = sizeof(bit_code)/sizeof(bit_code[0]);
    MMP_U32 codeBase = m_code_buf.m_vir_addr;
    int coreIdx = m_coreIdx;
	
    //mmpResult = VPU_Init_internal();
    this->VPU_SWReset(m_coreIdx, SW_RESET_ON_BOOT, NULL);
	this->SetClockGate(m_coreIdx, 1);
	this->BitLoadFirmware(m_coreIdx, codeBase, codeWord, codeSize);

    this->VpuWriteReg(coreIdx, BIT_PARA_BUF_ADDR, m_parm_buf.m_phy_addr);
    this->VpuWriteReg(coreIdx, BIT_CODE_BUF_ADDR, m_code_buf.m_phy_addr);
    this->VpuWriteReg(coreIdx, BIT_TEMP_BUF_ADDR, m_temp_buf.m_phy_addr);
	this->VpuWriteReg(coreIdx, BIT_BIT_STREAM_CTRL, VPU_STREAM_ENDIAN);
	this->VpuWriteReg(coreIdx, BIT_FRAME_MEM_CTRL, CBCR_INTERLEAVE<<2|VPU_FRAME_ENDIAN);
	this->VpuWriteReg(coreIdx, BIT_BIT_STREAM_PARAM, 0);

	this->VpuWriteReg(coreIdx, BIT_AXI_SRAM_USE, 0);
	this->VpuWriteReg(coreIdx, BIT_INT_ENABLE, 0);
	this->VpuWriteReg(coreIdx, BIT_ROLLBACK_STATUS, 0);	
	data = (1<<INT_BIT_BIT_BUF_FULL);
	data |= (1<<INT_BIT_BIT_BUF_EMPTY);
	data |= (1<<INT_BIT_DEC_MB_ROWS);
	data |= (1<<INT_BIT_SEQ_INIT);
	data |= (1<<INT_BIT_DEC_FIELD);
	data |= (1<<INT_BIT_PIC_RUN);

	this->VpuWriteReg(coreIdx, BIT_INT_ENABLE, data);
	this->VpuWriteReg(coreIdx, BIT_INT_CLEAR, 0x1);

	this->VpuWriteReg(coreIdx, BIT_BUSY_FLAG, 0x1);
	this->VpuWriteReg(coreIdx, BIT_CODE_RESET, 1);
	this->VpuWriteReg(coreIdx, BIT_CODE_RUN, 1);

    if (this->vdi_wait_vpu_busy(m_coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) == -1) {

        //VLOG(ERR, "[InitializeVPU] ln=%d  FAIL:vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG). \n", __LINE__ );
		mmpResult = MMP_FAILURE; 
	}		

    return mmpResult;
    
}

MMP_RESULT mmp_vpu_dev_ex1::close() {

#if 0
    m_msg_res.close();
#endif
    
    mmp_vpu_dev::close();

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
        m_p_mutex = NULL;
    }

    MMPDEBUGMSG(1, (TEXT("[mmp_vpu_dev_ex1::close]  ")));

	return MMP_SUCCESS;
}

void mmp_vpu_dev_ex1::service_stub(void* parm) {
    class mmp_vpu_dev_ex1* p_obj = (class mmp_vpu_dev_ex1*)parm;
    p_obj->service();
}

void mmp_vpu_dev_ex1::service() {

    class mmp_msg_res *p_res;
    class mmp_msg_packet* p_packet;
    MMP_RESULT mmpResult;

    p_res = &m_msg_res;
    
    while(p_res->is_run() == MMP_TRUE) {
    
        p_packet = NULL;
        mmpResult = p_res->readmsg_from_queue(&p_packet);
        
        if(p_res->is_run() != MMP_TRUE) break;

        if(mmpResult == MMP_SUCCESS) {
            this->service_msg_proc(p_packet);
        }
    }
}

void mmp_vpu_dev_ex1::service_msg_proc(class mmp_msg_packet* p_packet) {

    switch(p_packet->m_msg) {
    
        case 0:
            break;

        default:
            break;
    }
}





Uint32  mmp_vpu_dev_ex1::VPU_IsBusy(Uint32 coreIdx) { 

    class mmp_lock autolock(m_p_mutex);

    Uint32 ret = 0;
	this->SetClockGate(coreIdx, 1);  
	ret = this->VpuReadReg(coreIdx, BIT_BUSY_FLAG);
	this->SetClockGate(coreIdx, 0);

    return ret != 0;
}

Uint32  mmp_vpu_dev_ex1::VPU_WaitInterrupt(Uint32 coreIdx, int timeout) { 
    class mmp_lock autolock(m_p_mutex);

    Uint32 reason = 0;
    int ret;

	this->SetClockGate(coreIdx, 1);

	ret = MMP_DRIVER_IOCTL(this->m_vpu_fd, VDI_IOCTL_WAIT_INTERRUPT, (void*)timeout); //vdi_wait_interrupt(coreIdx, timeout, BIT_INT_REASON);
    if(ret == 0) {

	    if (reason != (Uint32)-1)
		    reason = this->VpuReadReg(coreIdx, BIT_INT_REASON);

	    if (reason != (Uint32)-1) {
		    this->VpuWriteReg(coreIdx, BIT_INT_CLEAR, 1);		// clear HW signal				
	    }
    }

	this->SetClockGate(coreIdx, 0);

	return reason;
}

Uint32  mmp_vpu_dev_ex1::VPU_IsInit(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);

    Uint32 pc;

	this->SetClockGate(coreIdx, 1);
	pc = this->VpuReadReg(coreIdx, BIT_CUR_PC);
	this->SetClockGate(coreIdx, 0);

	return pc;
}

RetCode  mmp_vpu_dev_ex1::VPU_Init(Uint32 coreIdx) { 
    //class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_InitWithBitcode(Uint32 coreIdx,	const Uint16 *bitcode,	Uint32 size) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

void  mmp_vpu_dev_ex1::VPU_DeInit(Uint32 coreIdx) {  
    class mmp_lock autolock(m_p_mutex);

}

Uint32  mmp_vpu_dev_ex1::VPU_GetOpenInstanceNum(Uint32 coreIdx) { 
    //class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_GetVersionInfo(Uint32 coreIdx, Uint32 *versionInfo,	Uint32 *revision, Uint32 *productId) { 
    class mmp_lock autolock(m_p_mutex);

    Uint32 ver;
	Uint32 rev;
	Uint32 pid;

	this->EnterLock(coreIdx);

	if (versionInfo && revision) {

		if (this->VpuReadReg(coreIdx, BIT_CUR_PC) == 0) {
			return RETCODE_NOT_INITIALIZED;
		}

		if (this->GetPendingInst(coreIdx)) {
			LeaveLock(coreIdx);
			return RETCODE_FRAME_NOT_COMPLETE;
		}
			
		this->VpuWriteReg(coreIdx, RET_FW_VER_NUM , 0);

		this->BitIssueCommand(coreIdx, NULL, FIRMWARE_GET);
		if (this->vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) == -1) {
			this->LeaveLock(coreIdx);
			return RETCODE_VPU_RESPONSE_TIMEOUT;
		}	


		ver = this->VpuReadReg(coreIdx, RET_FW_VER_NUM);
		rev = this->VpuReadReg(coreIdx, RET_FW_CODE_REV);

		if (ver == 0) {
			this->LeaveLock(coreIdx);
			return RETCODE_FAILURE;
		}

		*versionInfo = ver;
		*revision = rev;
	}

	pid = this->VpuReadReg(coreIdx, DBG_CONFIG_REPORT_1);	
    if ((pid&0xff00) == 0x3200) pid = 0x3200;
	if (productId)
		*productId = pid;

	this->LeaveLock(coreIdx);
	return RETCODE_SUCCESS;
}

void  mmp_vpu_dev_ex1::VPU_ClearInterrupt(Uint32 coreIdx) {  
    class mmp_lock autolock(m_p_mutex);

}

RetCode  mmp_vpu_dev_ex1::VPU_SWReset(Uint32 coreIdx,	int resetMode, void *pendingInst) { 
    class mmp_lock autolock(m_p_mutex);

    Uint32 cmd;
	CodecInst *pCodecInst = (CodecInst *)pendingInst;

	this->SetClockGate(coreIdx, 1);

	if(resetMode == SW_RESET_SAFETY || resetMode == SW_RESET_ON_BOOT) {

		// Waiting for completion of BWB transaction first 
		if(this->vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, GDI_BWB_STATUS) == -1) {
			this->SetClockGate(coreIdx, 0);
			return RETCODE_VPU_RESPONSE_TIMEOUT;
		}

		// Waiting for completion of bus transaction
		// Step1 : No more request
		this->VpuWriteReg(coreIdx, GDI_BUS_CTRL, 0x11);	// no more request {3'b0,no_more_req_sec,3'b0,no_more_req}

		// Step2 : Waiting for completion of bus transaction
		if (this->vdi_wait_bus_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, GDI_BUS_STATUS) == -1) {
			//if (pCodecInst) {
			//	if (pCodecInst->loggingEnable) {
			//		vdi_log(pCodecInst->coreIdx, 0x10, 2);
			//	}
			//}

			this->VpuWriteReg(coreIdx, GDI_BUS_CTRL, 0x00);
			this->SetClockGate(coreIdx, 0);
			return RETCODE_VPU_RESPONSE_TIMEOUT;
		}
	}

	cmd = 0;
	// Software Reset Trigger
	if (resetMode != SW_RESET_ON_BOOT)
		cmd =  VPU_SW_RESET_BPU_CORE | VPU_SW_RESET_BPU_BUS;
	cmd |= VPU_SW_RESET_VCE_CORE | VPU_SW_RESET_VCE_BUS;
	if (resetMode == SW_RESET_ON_BOOT)
		cmd |= VPU_SW_RESET_GDI_CORE | VPU_SW_RESET_GDI_BUS;// If you reset GDI, tiled map should be reconfigured

	this->VpuWriteReg(coreIdx, BIT_SW_RESET, cmd);

	// wait until reset is done
	if(this->vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_SW_RESET_STATUS) == -1) {
		//if (pCodecInst) {
		//	if (pCodecInst->loggingEnable) {
		//		vdi_log(pCodecInst->coreIdx, 0x10, 2);
		//	}
		//}
		this->VpuWriteReg(coreIdx, BIT_SW_RESET, 0x00);
		this->VpuWriteReg(coreIdx, GDI_BUS_CTRL, 0x00);
		this->SetClockGate(coreIdx, 0);
		return RETCODE_VPU_RESPONSE_TIMEOUT;
	}

	this->VpuWriteReg(coreIdx, BIT_SW_RESET, 0);

	// Step3 : must clear GDI_BUS_CTRL after done SW_RESET
	this->VpuWriteReg(coreIdx, GDI_BUS_CTRL, 0x00);

	//if (pCodecInst) {
	//	if (pCodecInst->loggingEnable) {
	//		vdi_log(pCodecInst->coreIdx, 0x10, 0);
	//	}
	//}

	this->SetClockGate(coreIdx, 0);
	return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_HWReset(Uint32 coreIdx) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_SleepWake(Uint32 coreIdx, int iSleepWake) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

int  mmp_vpu_dev_ex1::VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

int  mmp_vpu_dev_ex1::VPU_GetFrameBufSize(int width, int height, int mapType,	int format,	DRAMConfig *pDramCfg) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}


// function for decode
RetCode  mmp_vpu_dev_ex1::VPU_DecOpen(DecHandle *pHandle,	DecOpenParam *pop) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	int val;
	RetCode ret;
#ifdef SUPPORT_MEM_PROTECT
	int i;
#endif

    ret = ::CheckDecOpenParam(pop);
	if (ret != RETCODE_SUCCESS) {
		return ret;
	}

	this->EnterLock(pop->coreIdx);


	if (this->VpuReadReg(pop->coreIdx, BIT_CUR_PC) == 0) {
		this->LeaveLock(pop->coreIdx);
		return RETCODE_NOT_INITIALIZED;
	}

#if 0
	ret = GetCodecInstance(pop->coreIdx, &pCodecInst);
	if (ret != RETCODE_SUCCESS) {
		*pHandle = 0;
		LeaveLock(pop->coreIdx);
		return ret;
	}
#else

    pCodecInst = new CodecInst;
    if(pCodecInst == NULL) {
        return RETCODE_FAILURE;
    }
    memset(pCodecInst , 0x00, sizeof(CodecInst));

    pCodecInst->instIndex = 0;
	pCodecInst->inUse = 1;
	pCodecInst->coreIdx = pop->coreIdx;
	
#endif

	*pHandle = pCodecInst;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	
	pDecInfo->openParam = *pop;

	pCodecInst->codecMode = 0;
	pCodecInst->codecModeAux = 0;
	if (pop->bitstreamFormat == STD_MPEG4) {
		pCodecInst->codecMode = MP4_DEC;
		pCodecInst->codecModeAux = MP4_AUX_MPEG4;
	}
	else if (pop->bitstreamFormat == STD_AVC) {
		pCodecInst->codecMode = AVC_DEC;
		pCodecInst->codecModeAux = pop->avcExtension;
	} 
	else if (pop->bitstreamFormat == STD_VC1) {
		pCodecInst->codecMode = VC1_DEC;
	}
	else if (pop->bitstreamFormat == STD_MPEG2) {
		pCodecInst->codecMode = MP2_DEC;
	}
	else if (pop->bitstreamFormat == STD_H263) {
		pCodecInst->codecMode = MP4_DEC;
	}
	else if (pop->bitstreamFormat == STD_DIV3) {
		pCodecInst->codecMode = DV3_DEC;
		pCodecInst->codecModeAux = MP4_AUX_DIVX3;
	}	
	else if (pop->bitstreamFormat == STD_RV) {
		pCodecInst->codecMode = RV_DEC;
	}
	else if (pop->bitstreamFormat == STD_AVS) {
		pCodecInst->codecMode = AVS_DEC;
	}
	else if (pop->bitstreamFormat == STD_THO) {
		pCodecInst->codecMode = VPX_DEC;
		pCodecInst->codecModeAux = VPX_AUX_THO;
	} 
	else if (pop->bitstreamFormat == STD_VP3) {
		pCodecInst->codecMode = VPX_DEC;
		pCodecInst->codecModeAux = VPX_AUX_THO;
	} 
	else if (pop->bitstreamFormat == STD_VP8) {
		pCodecInst->codecMode = VPX_DEC;
		pCodecInst->codecModeAux = VPX_AUX_VP8;
	}
	pDecInfo->wtlEnable = pop->wtlEnable;
	pDecInfo->streamWrPtr = pop->bitstreamBuffer;	
	pDecInfo->streamRdPtr = pop->bitstreamBuffer;

	pDecInfo->streamRdPtrRegAddr = BIT_RD_PTR;
	pDecInfo->streamWrPtrRegAddr = BIT_WR_PTR;
	pDecInfo->frameDisplayFlagRegAddr = BIT_FRM_DIS_FLG;

	pDecInfo->frameDisplayFlag = 0;
	pDecInfo->clearDisplayIndexes = 0;
	pDecInfo->setDisplayIndexes = 0;
		
	pDecInfo->frameDelay = -1;
	pDecInfo->streamBufStartAddr = pop->bitstreamBuffer;
	pDecInfo->streamBufSize = pop->bitstreamBufferSize;
	pDecInfo->streamBufEndAddr = pop->bitstreamBuffer + pop->bitstreamBufferSize;	

	pDecInfo->stride = 0;
	pDecInfo->vbFrame.size = 0;
	pDecInfo->vbPPU.size = 0;
	pDecInfo->vbMV.size = 0;
	pDecInfo->frameAllocExt = 0;
	pDecInfo->ppuAllocExt = 0;
	pDecInfo->reorderEnable = VPU_REORDER_ENABLE;
	pDecInfo->seqInitEscape = 0;
	pDecInfo->rotationEnable = 0;
	pDecInfo->mirrorEnable = 0;
	pDecInfo->mirrorDirection = MIRDIR_NONE;
	pDecInfo->rotationAngle = 0;
	pDecInfo->rotatorOutputValid = 0;
	pDecInfo->rotatorStride = 0;
	pDecInfo->deringEnable	= 0;
	pDecInfo->secAxiInfo.useBitEnable  = 0;
	pDecInfo->secAxiInfo.useIpEnable   = 0;
	pDecInfo->secAxiInfo.useDbkYEnable = 0;
	pDecInfo->secAxiInfo.useDbkCEnable = 0;
	pDecInfo->secAxiInfo.useOvlEnable  = 0;
	pDecInfo->secAxiInfo.useBtpEnable  = 0;
	pDecInfo->secAxiInfo.bufBitUse    = 0;
	pDecInfo->secAxiInfo.bufIpAcDcUse = 0;
	pDecInfo->secAxiInfo.bufDbkYUse   = 0;
	pDecInfo->secAxiInfo.bufDbkCUse   = 0;
	pDecInfo->secAxiInfo.bufOvlUse    = 0;
	pDecInfo->secAxiInfo.bufBtpUse    = 0;


	pDecInfo->initialInfoObtained = 0;
	pDecInfo->vc1BframeDisplayValid = 0;
	pDecInfo->userDataBufAddr = 0;
	pDecInfo->userDataEnable = 0;
	pDecInfo->userDataBufSize = 0;
	pDecInfo->vbUserData.size = 0;
	pDecInfo->dramCfg.rasBit = EM_RAS;
	pDecInfo->dramCfg.casBit = EM_CAS;
	pDecInfo->dramCfg.bankBit = EM_BANK;
	pDecInfo->dramCfg.busBit = EM_WIDTH;
	pDecInfo->avcErrorConcealMode = AVC_ERROR_CONCEAL_MODE_DEFAULT;

	pDecInfo->vbSlice.base = 0;
	pDecInfo->vbSlice.virt_addr = 0;
	pDecInfo->vbSlice.size = 0;
	pDecInfo->vbWork.size       = WORK_BUF_SIZE;
	if (pop->bitstreamFormat == STD_AVC) {
		pDecInfo->vbWork.size += PS_SAVE_SIZE;
	}
	
    if(this->vdi_allocate_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWork) < 0)
	{
		*pHandle = 0;
		this->LeaveLock(pCodecInst->coreIdx);
		return RETCODE_INSUFFICIENT_RESOURCE;
	}

	pDecInfo->tiled2LinearEnable = pop->tiled2LinearEnable;
	pDecInfo->cacheConfig.CacheMode = 0;

#ifdef SUPPORT_MEM_PROTECT
	for (i=0; i<WPROT_DEC_MAX; i++)
		pDecInfo->writeMemProtectCfg.decRegion[i].enable = 0;

	ConfigDecWPROTRegion(pCodecInst->coreIdx, pDecInfo); // set common & PS or Work buffer memory protection
#endif	
	this->VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr, pDecInfo->streamBufStartAddr);
	this->VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr, pDecInfo->streamWrPtr);


	val = this->VpuReadReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM);
	val &= ~(1 << 2);
	this->VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM, val);	// clear stream end flag at start


	pDecInfo->streamEndflag = val;	

	this->LeaveLock(pCodecInst->coreIdx);

	return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_DecClose(DecHandle handle) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	RetCode ret;


    ret = ::CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	this->EnterLock(pCodecInst->coreIdx);

	if (pDecInfo->initialInfoObtained) {

		pDecInfo->streamEndflag &= ~(3<<3);
		if (pDecInfo->openParam.bitstreamMode == BS_MODE_ROLLBACK)  //rollback mode
			pDecInfo->streamEndflag |= (1<<3);
		else if (pDecInfo->openParam.bitstreamMode == BS_MODE_PIC_END)
			pDecInfo->streamEndflag |= (2<<3);
		this->VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM, pDecInfo->streamEndflag); 

		this->BitIssueCommand(pCodecInst->coreIdx, pCodecInst, SEQ_END);
		if (this->vdi_wait_vpu_busy(pCodecInst->coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) == -1) {
			if (pCodecInst->loggingEnable)
				vdi_log(pCodecInst->coreIdx, SEQ_END, 2);
			this->LeaveLock(pCodecInst->coreIdx);
			return RETCODE_VPU_RESPONSE_TIMEOUT;		
		}
		if (pCodecInst->loggingEnable)
			vdi_log(pCodecInst->coreIdx, SEQ_END, 0);
	}

	if (pDecInfo->vbSlice.size)
		this->vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbSlice);
	if (pDecInfo->vbWork.size)
		this->vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWork);
	
	if (pDecInfo->vbFrame.size){
		if (pDecInfo->frameAllocExt == 0) {
			this->vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFrame);
		}		
	}	

	if (pDecInfo->vbMV.size) {
		this->vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbMV);
	}

	if (pDecInfo->vbPPU.size) {
		if (pDecInfo->ppuAllocExt == 0) {
			this->vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbPPU);
		}
	}
	if (pDecInfo->wtlEnable)	//coda980 only
	{
            if (pDecInfo->vbWTL.size)
                this->vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWTL);
        }

#if 0
	if (pDecInfo->vbUserData.size)
		this->vdi_dettach_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbUserData);
#endif

	this->LeaveLock(pCodecInst->coreIdx);

#if 0
	FreeCodecInstance(pCodecInst);
#else
    delete pCodecInst;
#endif

	return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_DecSetEscSeqInit(DecHandle handle, int escape) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_DecGetInitialInfo(DecHandle handle,	DecInitialInfo *info) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	Uint32 val, val2;
	RetCode ret;

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if (info == 0) {
		return RETCODE_INVALID_PARAM;
	}


	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	EnterLock(pCodecInst->coreIdx);

	if (GetPendingInst(pCodecInst->coreIdx)) {
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	if (DecBitstreamBufEmpty(pDecInfo)) {
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_WRONG_CALL_SEQUENCE;
	}


	VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_BB_START, pDecInfo->streamBufStartAddr);
	VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_BB_SIZE, pDecInfo->streamBufSize / 1024); // size in KBytes


	if(pDecInfo->userDataEnable) {
		val  = 0;
		val |= (pDecInfo->userDataReportMode << 10);
		val |= (pDecInfo->userDataEnable << 5);
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_USER_DATA_OPTION, val);
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_USER_DATA_BASE_ADDR, pDecInfo->userDataBufAddr);
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_USER_DATA_BUF_SIZE, pDecInfo->userDataBufSize);
	}
	else {
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_USER_DATA_OPTION, 0);
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_USER_DATA_BASE_ADDR, 0);
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_USER_DATA_BUF_SIZE, 0);
	}

	val  = 0;


	val |= (pDecInfo->reorderEnable<<1) & 0x2;
	val |= (pDecInfo->openParam.mp4DeblkEnable & 0x1);	
	val |= (pDecInfo->avcErrorConcealMode << 2);
	VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_OPTION, val);					

	switch(pCodecInst->codecMode) {
	case VC1_DEC:
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_VC1_STREAM_FMT, (0 << 3) & 0x08);
		break;
	case MP4_DEC:
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_MP4_ASP_CLASS, ((!VPU_GMC_PROCESS_METHOD)<<3)|pDecInfo->openParam.mp4Class);
		break;
	case AVC_DEC:
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_X264_MV_EN, VPU_AVC_X264_SUPPORT);
		break;	
	}
	if( pCodecInst->codecMode == AVC_DEC )
		VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_SEQ_SPP_CHUNK_SIZE, VPU_GBU_SIZE);

	VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr, pDecInfo->streamWrPtr);
	VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr, pDecInfo->streamRdPtr);


	pDecInfo->streamEndflag &= ~(3<<3);
	if (pDecInfo->openParam.bitstreamMode == BS_MODE_ROLLBACK)  //rollback mode
		pDecInfo->streamEndflag |= (1<<3);
	else if (pDecInfo->openParam.bitstreamMode == BS_MODE_PIC_END)
		pDecInfo->streamEndflag |= (2<<3);
	else {	// Interrupt Mode
		if (pDecInfo->seqInitEscape) {	
			pDecInfo->streamEndflag |= (2<<3);
		}
	}

	VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM, pDecInfo->streamEndflag);		

	val = pDecInfo->openParam.streamEndian;
	VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_CTRL, val);

	val = 0;	
	val |= (pDecInfo->wtlEnable<<17) |(pDecInfo->openParam.bwbEnable<<12);
	val |= ((pDecInfo->openParam.cbcrInterleave)<<2);
	val |= pDecInfo->openParam.frameEndian;
	VpuWriteReg(pCodecInst->coreIdx, BIT_FRAME_MEM_CTRL, val);
	
	VpuWriteReg(pCodecInst->coreIdx, pDecInfo->frameDisplayFlagRegAddr, 0);

	SetPendingInst(pCodecInst->coreIdx, pCodecInst);
	BitIssueCommand(pCodecInst->coreIdx, pCodecInst, SEQ_INIT);
	if (vdi_wait_vpu_busy(pCodecInst->coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) == -1) {
		if (pCodecInst->loggingEnable)
			vdi_log(pCodecInst->coreIdx, SEQ_INIT, 2);
		info->rdPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
		info->wrPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr);
		SetPendingInst(pCodecInst->coreIdx, 0);
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_VPU_RESPONSE_TIMEOUT;
	}

	if (pCodecInst->loggingEnable)
		vdi_log(pCodecInst->coreIdx, SEQ_INIT, 0);
	SetPendingInst(pCodecInst->coreIdx, 0);

	if (pDecInfo->openParam.bitstreamMode == BS_MODE_INTERRUPT &&
		pDecInfo->seqInitEscape) {
			pDecInfo->streamEndflag &= ~(3<<3);
			VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM, pDecInfo->streamEndflag);
			pDecInfo->seqInitEscape = 0;
	}
	
	pDecInfo->frameDisplayFlag = VpuReadReg(pCodecInst->coreIdx, pDecInfo->frameDisplayFlagRegAddr);
	pDecInfo->streamRdPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);	
	pDecInfo->streamEndflag = VpuReadReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM);

	info->rdPtr = pDecInfo->streamRdPtr;
	info->wrPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr);
    
    val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_SRC_SIZE);
    info->picWidth = ( (val >> 16) & 0xffff );
    info->picHeight = ( val & 0xffff );


	val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_SUCCESS);
	info->seqInitErrReason = 0;
#ifdef SUPPORT_MEM_PROTECT
	if (val & (1<<31)) {
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_MEMORY_ACCESS_VIOLATION;
	}
#endif

	if ( pDecInfo->openParam.bitstreamMode == BS_MODE_ROLLBACK) { 
		if (val & (1<<4)) {
			info->seqInitErrReason = (VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_SEQ_ERR_REASON) | (1<<20));
			SetPendingInst(pCodecInst->coreIdx, 0);
			LeaveLock(pCodecInst->coreIdx);
			return RETCODE_FAILURE;
		}
	}

	if (val == 0) {
		info->seqInitErrReason = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_SEQ_ERR_REASON);
        SetPendingInst(pCodecInst->coreIdx, 0);
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_FAILURE;
	}
	
	info->fRateNumerator    = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_FRATE_NR);
	info->fRateDenominator  = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_FRATE_DR);
	

	if (pCodecInst->codecMode == AVC_DEC && info->fRateDenominator > 0)
		info->fRateDenominator  *= 2;

	if (pCodecInst->codecMode  == MP4_DEC) 
	{
		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_INFO);
		info->mp4ShortVideoHeader = (val >> 2) & 1;
		info->mp4DataPartitionEnable = val & 1;
		info->mp4ReversibleVlcEnable =
			info->mp4DataPartitionEnable ?
			((val >> 1) & 1) : 0;
		info->h263AnnexJEnable = (val >> 3) & 1;
	}
	else if (pCodecInst->codecMode == VPX_DEC &&
		pCodecInst->codecModeAux == VPX_AUX_VP8)
	{
		// h_scale[31:30] v_scale[29:28] pic_width[27:14] pic_height[13:0]
		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_VP8_SCALE_INFO);
		info->vp8ScaleInfo.hScaleFactor = (val >> 30) & 0x03;
		info->vp8ScaleInfo.vScaleFactor = (val >> 28) & 0x03;
		info->vp8ScaleInfo.picWidth = (val >> 14) & 0x3FFF;
		info->vp8ScaleInfo.picHeight = (val >> 0) & 0x3FFF;
	}

	info->minFrameBufferCount = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_FRAME_NEED);
	info->frameBufDelay = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_FRAME_DELAY);

	if (pCodecInst->codecMode == AVC_DEC)
	{
		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_CROP_LEFT_RIGHT);	
		val2 = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_CROP_TOP_BOTTOM);
		if( val == 0 && val2 == 0 )
		{
			info->picCropRect.left = 0;
			info->picCropRect.right = info->picWidth;
			info->picCropRect.top = 0;
			info->picCropRect.bottom = info->picHeight;
		}
		else
		{
			info->picCropRect.left = ( (val>>16) & 0xFFFF );
			info->picCropRect.right = info->picWidth - ( ( val & 0xFFFF ) );
			info->picCropRect.top = ( (val2>>16) & 0xFFFF );
			info->picCropRect.bottom = info->picHeight - ( ( val2 & 0xFFFF ) );
		}

		val = (info->picWidth * info->picHeight * 3 / 2) / 1024;
		info->normalSliceSize = val / 4;
		info->worstSliceSize = val / 2;
	}
	else
	{
		info->picCropRect.left = 0;
		info->picCropRect.right = info->picWidth;
		info->picCropRect.top = 0;
		info->picCropRect.bottom = info->picHeight;
	}

       if (pCodecInst->codecMode == MP2_DEC)
       {
             val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_MP2_BAR_LEFT_RIGHT);
             val2 = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_MP2_BAR_TOP_BOTTOM);

             info->mp2BardataInfo.barLeft = ((val>>16) & 0xFFFF);
             info->mp2BardataInfo.barRight = (val&0xFFFF);
             info->mp2BardataInfo.barTop = ((val2>>16) & 0xFFFF);
             info->mp2BardataInfo.barBottom = (val2&0xFFFF);               
       }

	val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_HEADER_REPORT);
	info->profile =			(val >> 0) & 0xFF;
	info->level =			(val >> 8) & 0xFF;
	info->interlace =		(val >> 16) & 0x01;
	info->direct8x8Flag =		(val >> 17) & 0x01;
	info->vc1Psf =			(val >> 18) & 0x01;
	info->constraint_set_flag[0] = 	(val >> 19) & 0x01;
	info->constraint_set_flag[1] = 	(val >> 20) & 0x01;
	info->constraint_set_flag[2] = 	(val >> 21) & 0x01;
	info->constraint_set_flag[3] = 	(val >> 22) & 0x01;
	info->chromaFormat = ( val >> 23) & 0x03;
	info->avcIsExtSAR            =  (val >> 25) & 0x01;
	info->maxNumRefFrm     =  (val >> 27) & 0x0f;
	info->maxNumRefFrmFlag =  (val >> 31) & 0x01;

	val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_ASPECT);

	info->aspectRateInfo = val;

	val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_BIT_RATE);
	info->bitRate = val;

	if (pCodecInst->codecMode == MP2_DEC) {
		// seq_ext info
		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_EXT_INFO);
		info->mp2LowDelay = val & 1;
		info->mp2DispVerSize = (val>>1) & 0x3fff;
		info->mp2DispHorSize = (val>>15) & 0x3fff;
	}

	if (pCodecInst->codecMode == AVC_DEC) {
		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_VUI_INFO);
		info->avcVuiInfo.fixedFrameRateFlag    = val &1;
		info->avcVuiInfo.timingInfoPresent     = (val>>1) & 0x01;
		info->avcVuiInfo.chromaLocBotField     = (val>>2) & 0x07;
		info->avcVuiInfo.chromaLocTopField     = (val>>5) & 0x07;
		info->avcVuiInfo.chromaLocInfoPresent  = (val>>8) & 0x01;
		info->avcVuiInfo.colorPrimaries        = (val>>16) & 0xff;
		info->avcVuiInfo.colorDescPresent      = (val>>24) & 0x01;
		info->avcVuiInfo.isExtSAR              = (val>>25) & 0x01;
		info->avcVuiInfo.vidFullRange          = (val>>26) & 0x01;
		info->avcVuiInfo.vidFormat             = (val>>27) & 0x07;
		info->avcVuiInfo.vidSigTypePresent     = (val>>30) & 0x01;
		info->avcVuiInfo.vuiParamPresent       = (val>>31) & 0x01;

		val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_VUI_PIC_STRUCT);
		info->avcVuiInfo.vuiPicStructPresent = (val & 0x1);
		info->avcVuiInfo.vuiPicStruct = (val>>1);
	}

	if (pDecInfo->userDataEnable && pCodecInst->codecMode == MP2_DEC) {
		int userDataNum;
		int userDataSize;
		BYTE tempBuf[8] = {0,};		

		VpuReadMem(pCodecInst->coreIdx, pDecInfo->userDataBufAddr, tempBuf, 8, VPU_USER_DATA_ENDIAN); 

		val = ((tempBuf[0]<<24) & 0xFF000000) |
			((tempBuf[1]<<16) & 0x00FF0000) |
			((tempBuf[2]<< 8) & 0x0000FF00) |
			((tempBuf[3]<< 0) & 0x000000FF);

		userDataNum = (val >> 16) & 0xFFFF;
		userDataSize = (val >> 0) & 0xFFFF;
		if (userDataNum == 0)
			userDataSize = 0;

		info->userDataNum = userDataNum;
		info->userDataSize = userDataSize;

		val = ((tempBuf[4]<<24) & 0xFF000000) |
			((tempBuf[5]<<16) & 0x00FF0000) |
			((tempBuf[6]<< 8) & 0x0000FF00) |
			((tempBuf[7]<< 0) & 0x000000FF);

		if (userDataNum == 0)
			info->userDataBufFull = 0;
		else
			info->userDataBufFull = (val >> 16) & 0xFFFF;
	}	
	
	pDecInfo->initialInfo = *info;
	pDecInfo->initialInfoObtained = 1;

	LeaveLock(pCodecInst->coreIdx);
	return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_DecIssueSeqInit(DecHandle handle) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo *info) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray,	int num, int stride, int height, int mapType) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	PhysicalAddress paraBuffer;
	vpu_buffer_t vb;
	Uint32 val;
	int i;
	RetCode ret;
	BYTE frameAddr[MAX_GDI_IDX][3][4];
	BYTE colMvAddr[MAX_GDI_IDX][4];
	int framebufFormat;
#ifdef SUPPORT_MEM_PROTECT    
	PhysicalAddress start;
	PhysicalAddress end;
    PhysicalAddress frame_buffer_end;
#endif

	ret = CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	
	if (!pDecInfo->initialInfoObtained) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}
		
    if (!pDecInfo->thumbnailMode)
    {
        if (num < pDecInfo->initialInfo.minFrameBufferCount) {
            return RETCODE_INSUFFICIENT_FRAME_BUFFERS;
        }
    }

	if ( (stride < pDecInfo->initialInfo.picWidth) || (stride % 8 != 0) || (height<pDecInfo->initialInfo.picHeight) ) {
		return RETCODE_INVALID_STRIDE;
	}

	EnterLock(pCodecInst->coreIdx);
	if (GetPendingInst(pCodecInst->coreIdx)) {
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_FRAME_NOT_COMPLETE;
	}

	vdi_get_common_memory(pCodecInst->coreIdx, &vb);
	paraBuffer = vb.phys_addr + CODE_BUF_SIZE + TEMP_BUF_SIZE;

	pDecInfo->numFrameBuffers = num;
	
	pDecInfo->stride = stride;
	pDecInfo->frameBufferHeight = height;
	pDecInfo->mapType = mapType;
	val = SetTiledMapType(pCodecInst->coreIdx, &pDecInfo->mapCfg, &pDecInfo->dramCfg, pDecInfo->stride, mapType);
	if (val == 0) {
        LeaveLock(pCodecInst->coreIdx);
		return RETCODE_INVALID_PARAM;
	}
	
	//Allocate frame buffer
	framebufFormat = FORMAT_420;
	
	if (bufArray) {
        for(i=0; i<num; i++) {
			pDecInfo->frameBufPool[i] = bufArray[i];
        }
	}
	else {
		ret = AllocateFrameBufferArray(pCodecInst->coreIdx, &pDecInfo->frameBufPool[0], &pDecInfo->vbFrame, mapType, 
			pDecInfo->openParam.cbcrInterleave, framebufFormat, pDecInfo->numFrameBuffers, stride, height, 0, FB_TYPE_CODEC, 0, &pDecInfo->dramCfg);
		pDecInfo->mapCfg.tiledBaseAddr = pDecInfo->vbFrame.phys_addr;
	}
	if (ret != RETCODE_SUCCESS) {
		LeaveLock(pCodecInst->coreIdx);
		return ret;
	}
	
	
#ifdef SUPPORT_MEM_PROTECT
	pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].enable        = 1;
	pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].isSecondary  = 0;

	pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].startAddress = GetXY2AXIAddr(&pDecInfo->mapCfg, 0, 0, 0, stride, &pDecInfo->frameBufPool[0]);
	pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress   = GetXY2AXIAddr(&pDecInfo->mapCfg, 0, 0, 0, stride, &pDecInfo->frameBufPool[num-1]);

	pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress +=  VPU_GetFrameBufSize(stride, height, pDecInfo->mapCfg.mapType, framebufFormat, &pDecInfo->dramCfg);
#endif // #ifdef SUPPORT_MEM_PROTECT

	for (i=0; i<pDecInfo->numFrameBuffers; i++) {
		frameAddr[i][0][0] = (pDecInfo->frameBufPool[i].bufY  >> 24) & 0xFF;
		frameAddr[i][0][1] = (pDecInfo->frameBufPool[i].bufY  >> 16) & 0xFF;
		frameAddr[i][0][2] = (pDecInfo->frameBufPool[i].bufY  >>  8) & 0xFF;
		frameAddr[i][0][3] = (pDecInfo->frameBufPool[i].bufY  >>  0) & 0xFF;
		if (pDecInfo->openParam.cbcrOrder == CBCR_ORDER_NORMAL) {
			frameAddr[i][1][0] = (pDecInfo->frameBufPool[i].bufCb >> 24) & 0xFF;
			frameAddr[i][1][1] = (pDecInfo->frameBufPool[i].bufCb >> 16) & 0xFF;
			frameAddr[i][1][2] = (pDecInfo->frameBufPool[i].bufCb >>  8) & 0xFF;
			frameAddr[i][1][3] = (pDecInfo->frameBufPool[i].bufCb >>  0) & 0xFF;
			frameAddr[i][2][0] = (pDecInfo->frameBufPool[i].bufCr >> 24) & 0xFF;
			frameAddr[i][2][1] = (pDecInfo->frameBufPool[i].bufCr >> 16) & 0xFF;
			frameAddr[i][2][2] = (pDecInfo->frameBufPool[i].bufCr >>  8) & 0xFF;
			frameAddr[i][2][3] = (pDecInfo->frameBufPool[i].bufCr >>  0) & 0xFF;
		}
		else {
			frameAddr[i][2][0] = (pDecInfo->frameBufPool[i].bufCb >> 24) & 0xFF;
			frameAddr[i][2][1] = (pDecInfo->frameBufPool[i].bufCb >> 16) & 0xFF;
			frameAddr[i][2][2] = (pDecInfo->frameBufPool[i].bufCb >>  8) & 0xFF;
			frameAddr[i][2][3] = (pDecInfo->frameBufPool[i].bufCb >>  0) & 0xFF;
			frameAddr[i][1][0] = (pDecInfo->frameBufPool[i].bufCr >> 24) & 0xFF;
			frameAddr[i][1][1] = (pDecInfo->frameBufPool[i].bufCr >> 16) & 0xFF;
			frameAddr[i][1][2] = (pDecInfo->frameBufPool[i].bufCr >>  8) & 0xFF;
			frameAddr[i][1][3] = (pDecInfo->frameBufPool[i].bufCr >>  0) & 0xFF;
		}		
	}

	VpuWriteMem(pCodecInst->coreIdx, paraBuffer, (BYTE*)frameAddr, sizeof(frameAddr), VDI_BIG_ENDIAN); 

	// MV allocation and register
	if (pCodecInst->codecMode == AVC_DEC || pCodecInst->codecMode == VC1_DEC || pCodecInst->codecMode == MP4_DEC ||
		pCodecInst->codecMode == RV_DEC || pCodecInst->codecMode == AVS_DEC)
	{
		unsigned long   bufMvCol;
		int size_mvcolbuf;
				
		if (pCodecInst->codecMode == AVC_DEC || pCodecInst->codecMode == VC1_DEC || 
			pCodecInst->codecMode == MP4_DEC || pCodecInst->codecMode == RV_DEC || pCodecInst->codecMode == AVS_DEC) {

			size_mvcolbuf =  ((pDecInfo->initialInfo.picWidth+31)&~31)*((pDecInfo->initialInfo.picHeight+31)&~31);
			size_mvcolbuf = (size_mvcolbuf*3)/2;
			size_mvcolbuf = (size_mvcolbuf+4)/5;
			size_mvcolbuf = ((size_mvcolbuf+7)/8)*8;

			if (pDecInfo->vbMV.size == 0) {
				if (pCodecInst->codecMode == AVC_DEC) {
					pDecInfo->vbMV.size     = size_mvcolbuf*pDecInfo->numFrameBuffers;
					if (pDecInfo->thumbnailMode) {
						pDecInfo->vbMV.size = size_mvcolbuf;
					}
				}
				else {
					pDecInfo->vbMV.size = size_mvcolbuf;
				}
				if (vdi_allocate_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbMV)<0){
					LeaveLock(pCodecInst->coreIdx);
					return RETCODE_FAILURE;
				}
			}
		}

		bufMvCol = pDecInfo->vbMV.phys_addr;	

		if (pCodecInst->codecMode == AVC_DEC) {
			for (i=0; i<pDecInfo->numFrameBuffers; i++) {
				colMvAddr[i][0] = (bufMvCol >> 24) & 0xFF;
				colMvAddr[i][1] = (bufMvCol >> 16) & 0xFF;
				colMvAddr[i][2] = (bufMvCol >>  8) & 0xFF;
				colMvAddr[i][3] = (bufMvCol >>  0) & 0xFF;
				bufMvCol += size_mvcolbuf;
				if (pDecInfo->thumbnailMode) {
					bufMvCol = pDecInfo->vbMV.phys_addr;	
				}
			}
		}
		else {
			bufMvCol = pDecInfo->vbMV.phys_addr;

			colMvAddr[0][0] = (bufMvCol >> 24) & 0xFF;
			colMvAddr[0][1] = (bufMvCol >> 16) & 0xFF;
			colMvAddr[0][2] = (bufMvCol >>  8) & 0xFF;
			colMvAddr[0][3] = (bufMvCol >>  0) & 0xFF;
			bufMvCol += size_mvcolbuf;
		}
#ifdef SUPPORT_MEM_PROTECT
		start = pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].startAddress;
		end   = pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress;

		start = (start< pDecInfo->vbMV.phys_addr) ? start : pDecInfo->vbMV.phys_addr;
		end   = (end  > bufMvCol) ? end : bufMvCol;

		pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].startAddress = start;
		pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress   = end;
#endif
 		VpuWriteMem(pCodecInst->coreIdx, paraBuffer+384, (BYTE*)colMvAddr, sizeof(colMvAddr), VDI_BIG_ENDIAN);
	}


    if (pDecInfo->wtlEnable) {
        if (bufArray) {
            for(i=pDecInfo->numFrameBuffers; i<pDecInfo->numFrameBuffers*2; i++)
                pDecInfo->frameBufPool[i] = bufArray[i];
        }
        else
        {
            ret = AllocateFrameBufferArray(pCodecInst->coreIdx, &pDecInfo->frameBufPool[pDecInfo->numFrameBuffers], &pDecInfo->vbWTL, LINEAR_FRAME_MAP,
                pDecInfo->openParam.cbcrInterleave, framebufFormat, pDecInfo->numFrameBuffers, stride, height, 0, FB_TYPE_CODEC, 0, &pDecInfo->dramCfg);
        }
        if (ret != RETCODE_SUCCESS) {
            LeaveLock(pCodecInst->coreIdx);
            return ret;
        }
        for (i=pDecInfo->numFrameBuffers; i<pDecInfo->numFrameBuffers*2; i++) {
            frameAddr[i-pDecInfo->numFrameBuffers][0][0] = (pDecInfo->frameBufPool[i].bufY  >> 24) & 0xFF;
            frameAddr[i-pDecInfo->numFrameBuffers][0][1] = (pDecInfo->frameBufPool[i].bufY  >> 16) & 0xFF;
            frameAddr[i-pDecInfo->numFrameBuffers][0][2] = (pDecInfo->frameBufPool[i].bufY  >>  8) & 0xFF;
            frameAddr[i-pDecInfo->numFrameBuffers][0][3] = (pDecInfo->frameBufPool[i].bufY  >>  0) & 0xFF;
            if (pDecInfo->openParam.cbcrOrder == CBCR_ORDER_NORMAL) {
                frameAddr[i-pDecInfo->numFrameBuffers][1][0] = (pDecInfo->frameBufPool[i].bufCb >> 24) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][1][1] = (pDecInfo->frameBufPool[i].bufCb >> 16) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][1][2] = (pDecInfo->frameBufPool[i].bufCb >>  8) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][1][3] = (pDecInfo->frameBufPool[i].bufCb >>  0) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][2][0] = (pDecInfo->frameBufPool[i].bufCr >> 24) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][2][1] = (pDecInfo->frameBufPool[i].bufCr >> 16) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][2][2] = (pDecInfo->frameBufPool[i].bufCr >>  8) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][2][3] = (pDecInfo->frameBufPool[i].bufCr >>  0) & 0xFF;
            }
            else {
                frameAddr[i-pDecInfo->numFrameBuffers][2][0] = (pDecInfo->frameBufPool[i].bufCb >> 24) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][2][1] = (pDecInfo->frameBufPool[i].bufCb >> 16) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][2][2] = (pDecInfo->frameBufPool[i].bufCb >>  8) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][2][3] = (pDecInfo->frameBufPool[i].bufCb >>  0) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][1][0] = (pDecInfo->frameBufPool[i].bufCr >> 24) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][1][1] = (pDecInfo->frameBufPool[i].bufCr >> 16) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][1][2] = (pDecInfo->frameBufPool[i].bufCr >>  8) & 0xFF;
                frameAddr[i-pDecInfo->numFrameBuffers][1][3] = (pDecInfo->frameBufPool[i].bufCr >>  0) & 0xFF;
            }
        }

        VpuWriteMem(pCodecInst->coreIdx, paraBuffer+384+128+384, (BYTE*)frameAddr, sizeof(frameAddr), VDI_BIG_ENDIAN);			

#ifdef SUPPORT_MEM_PROTECT
        start = pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].startAddress;
        end   = pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress;
        frame_buffer_end = pDecInfo->vbWTL.phys_addr + pDecInfo->vbWTL.size;

        start = (start< pDecInfo->vbWTL.phys_addr) ? start :pDecInfo->vbWTL.phys_addr;
        end   = (end  > frame_buffer_end) ? end : frame_buffer_end;

        pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].startAddress = start;
        pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress   = end;
#endif
        num *= 2;
    }


	if (!ConfigSecAXI(pCodecInst->coreIdx, pDecInfo->openParam.bitstreamFormat, &pDecInfo->secAxiInfo, stride, height,
		pDecInfo->initialInfo.profile&0xff)) {
			LeaveLock(pCodecInst->coreIdx);
			return RETCODE_INSUFFICIENT_RESOURCE;
	}
	
#ifdef SUPPORT_MEM_PROTECT
	if (pDecInfo->secAxiInfo.bufSize) {
		pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_SEC_AXI].enable = 1;
		pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_SEC_AXI].isSecondary = 1;
		pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_SEC_AXI].startAddress = pDecInfo->secAxiInfo.bufBitUse;
		pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_SEC_AXI].endAddress = pDecInfo->secAxiInfo.bufBitUse + pDecInfo->secAxiInfo.bufSize;
	}
#endif


	// Tell the decoder how much frame buffers were allocated.
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_NUM, num);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_STRIDE, stride);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_BIT_ADDR, pDecInfo->secAxiInfo.bufBitUse);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_IPACDC_ADDR, pDecInfo->secAxiInfo.bufIpAcDcUse);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_DBKY_ADDR, pDecInfo->secAxiInfo.bufDbkYUse);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_DBKC_ADDR, pDecInfo->secAxiInfo.bufDbkCUse);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_OVL_ADDR, pDecInfo->secAxiInfo.bufOvlUse);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_BTP_ADDR, pDecInfo->secAxiInfo.bufBtpUse);
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_DELAY, pDecInfo->frameDelay);
	

	// Maverick Cache Configuration
	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_CACHE_CONFIG, pDecInfo->cacheConfig.CacheMode);	


	if (pCodecInst->codecMode == VPX_DEC) {
		vpu_buffer_t	*pvbSlice = &pDecInfo->vbSlice;
#ifdef SUPPORT_MEM_PROTECT		
		WriteMemProtectCfg *pCgf    = &pDecInfo->writeMemProtectCfg;
#endif	
		if (pvbSlice->size == 0) {
			//pvbSlice->size     = 17*4*(pDecInfo->initialInfo.picWidth*pDecInfo->initialInfo.picHeight/256);	//(MB information + split MVs)*4*MbNumbyte
			pvbSlice->size = VP8_MB_SAVE_SIZE;
			if (vdi_allocate_dma_memory(pCodecInst->coreIdx, pvbSlice) < 0) {
				LeaveLock(pCodecInst->coreIdx);
				return RETCODE_INSUFFICIENT_RESOURCE;
			}
		}
		
#ifdef SUPPORT_MEM_PROTECT
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].enable = 1;
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].isSecondary = 0;
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].startAddress = pvbSlice->phys_addr;
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].endAddress =  pvbSlice->phys_addr + pvbSlice->size;
#endif
		VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_MB_BUF_BASE, pvbSlice->phys_addr);
	}

	if (pCodecInst->codecMode == AVC_DEC && pDecInfo->initialInfo.profile == 66) {
		vpu_buffer_t	*pvbSlice = &pDecInfo->vbSlice;
#ifdef SUPPORT_MEM_PROTECT	
		WriteMemProtectCfg *pCgf    = &pDecInfo->writeMemProtectCfg;
#endif
		if (pvbSlice->size == 0) {
			//pvbSlice->size     = pDecInfo->initialInfo.picWidth*pDecInfo->initialInfo.picHeight*3/4;	// this buffer for ASO/FMO		
			pvbSlice->size = SLICE_SAVE_SIZE;
			if (vdi_allocate_dma_memory(pCodecInst->coreIdx, pvbSlice) < 0) {
				LeaveLock(pCodecInst->coreIdx);
				return RETCODE_INSUFFICIENT_RESOURCE;
			}
		}
		
#ifdef SUPPORT_MEM_PROTECT
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].enable = 1;
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].isSecondary = 0;
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].startAddress = pvbSlice->phys_addr;
		pCgf->decRegion[WPROT_DEC_PIC_SAVE].endAddress =  pvbSlice->phys_addr + pvbSlice->size;
#endif
		VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_SLICE_BB_START, pvbSlice->phys_addr);
		VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_SLICE_BB_SIZE, (pvbSlice->size/1024));
	}

	val = 0;    
	if (pDecInfo->mapType) {
		if (pDecInfo->mapType == TILED_FRAME_MB_RASTER_MAP || pDecInfo->mapType == TILED_FIELD_MB_RASTER_MAP)
			val |= (pDecInfo->tiled2LinearEnable<<11)|(0x03<<9)|(FORMAT_420<<6);	
		else
			val |= (pDecInfo->tiled2LinearEnable<<11)|(0x02<<9)|(FORMAT_420<<6);	
	}
	val |= (pDecInfo->wtlEnable<<17);
	val |= (pDecInfo->openParam.bwbEnable<<12);
	val |= ((pDecInfo->openParam.cbcrInterleave)<<2);
	val |= pDecInfo->openParam.frameEndian;
	VpuWriteReg(pCodecInst->coreIdx, BIT_FRAME_MEM_CTRL, val);

	VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_MAX_DEC_SIZE, 0);  // Must set to zero at API 2.1.5 version

	SetPendingInst(pCodecInst->coreIdx, pCodecInst);
	BitIssueCommand(pCodecInst->coreIdx, pCodecInst, SET_FRAME_BUF);
	if (vdi_wait_vpu_busy(pCodecInst->coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) == -1) {
		if (pCodecInst->loggingEnable)
			vdi_log(pCodecInst->coreIdx, SET_FRAME_BUF, 2);
		SetPendingInst(pCodecInst->coreIdx, 0);
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_VPU_RESPONSE_TIMEOUT;
	}
	if (pCodecInst->loggingEnable)
		vdi_log(pCodecInst->coreIdx, SET_FRAME_BUF, 0);

#ifdef SUPPORT_MEM_PROTECT
	if (VpuReadReg(pCodecInst->coreIdx, RET_SET_FRAME_SUCCESS) & (1<<31)) {
		SetPendingInst(pCodecInst->coreIdx, 0);
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_MEMORY_ACCESS_VIOLATION;
	}
#endif
	SetPendingInst(pCodecInst->coreIdx, 0);
	LeaveLock(pCodecInst->coreIdx);
	return RETCODE_SUCCESS;
}


RetCode  mmp_vpu_dev_ex1::VPU_DecGetFrameBuffer(DecHandle handle,	int frameIdx, FrameBuffer *frameBuf) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_DecGetBitstreamBuffer(DecHandle handle,	PhysicalAddress *prdPtr, PhysicalAddress *pwrPtr, int *size) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	PhysicalAddress rdPtr;
	PhysicalAddress wrPtr;
	int room;
	RetCode ret;


    ret = this->CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	if ( prdPtr == 0 || pwrPtr == 0 || size == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;

	this->SetClockGate(pCodecInst->coreIdx, 1);

	if(this->GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
		rdPtr = this->VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
	else
		rdPtr = pDecInfo->streamRdPtr;

	this->SetClockGate(pCodecInst->coreIdx, 0);

	wrPtr = pDecInfo->streamWrPtr;

	if (wrPtr < rdPtr) {
		room = rdPtr - wrPtr - VPU_GBU_SIZE*2 - 1;		
	}
	else {
		room = ( pDecInfo->streamBufEndAddr - wrPtr ) + ( rdPtr - pDecInfo->streamBufStartAddr ) - VPU_GBU_SIZE*2 - 1;			
	}


	*prdPtr = rdPtr;
	*pwrPtr = wrPtr;
	room = ( ( room >> 9 ) << 9 ); // multiple of 512
	*size = room;

	return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	PhysicalAddress wrPtr;
	PhysicalAddress rdPtr;
	RetCode ret;
	int		val = 0;
	int		room = 0;

    ret = this->CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;

	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	wrPtr = pDecInfo->streamWrPtr;

	SetClockGate(pCodecInst->coreIdx, 1);

	if (size == 0) 
	{

		val = VpuReadReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM);
		val |= 1 << 2;
		pDecInfo->streamEndflag = val;
		if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
			VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM, val);
		SetClockGate(pCodecInst->coreIdx, 0);
		return RETCODE_SUCCESS;
	}
	
	if (size == -1)
	{
		val = VpuReadReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM);
		val &= ~(1 << 2);
		pDecInfo->streamEndflag = val;
		if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
			VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM, val);

		SetClockGate(pCodecInst->coreIdx, 0);
		return RETCODE_SUCCESS;
	}


	if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
		rdPtr = VpuReadReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr);
	else
		rdPtr = pDecInfo->streamRdPtr;

	if (wrPtr < rdPtr) {
		if (rdPtr <= wrPtr + size) {
			SetClockGate(pCodecInst->coreIdx, 0);
			return RETCODE_INVALID_PARAM;
		}
	}

	wrPtr += size;

	if (wrPtr > pDecInfo->streamBufEndAddr) {
		room = wrPtr - pDecInfo->streamBufEndAddr;
		wrPtr = pDecInfo->streamBufStartAddr;
		wrPtr += room;
	}
	if (wrPtr == pDecInfo->streamBufEndAddr) {
		wrPtr = pDecInfo->streamBufStartAddr;
	}

	pDecInfo->streamWrPtr = wrPtr;
	pDecInfo->streamRdPtr = rdPtr;

	if (GetPendingInst(pCodecInst->coreIdx) == pCodecInst)
		VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamWrPtrRegAddr, wrPtr);

	SetClockGate(pCodecInst->coreIdx, 0);
	return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_DecStartOneFrame(DecHandle handle, DecParam *param) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_DecFrameBufferFlush(DecHandle handle) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}


RetCode  mmp_vpu_dev_ex1::VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr,	int updateWrPtr) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	RetCode ret;
	CodecInst * pPendingInst;
	
    ret = ::CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;
	
	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	
	pPendingInst = this->GetPendingInst(pCodecInst->coreIdx);
	if (pCodecInst == pPendingInst) {
		SetClockGate(pCodecInst->coreIdx, 1);
		VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr, addr);
		SetClockGate(pCodecInst->coreIdx, 0);
	}
	else {
		if (!pPendingInst) {
			EnterLock(pCodecInst->coreIdx);
			VpuWriteReg(pCodecInst->coreIdx, pDecInfo->streamRdPtrRegAddr, addr);
			LeaveLock(pCodecInst->coreIdx);
		}	
		else {
			// if pPendingInst is not my instance. must not write register in direct.
		}
	}
	
	pDecInfo->streamRdPtr = addr;
	if (updateWrPtr)
		pDecInfo->streamWrPtr = addr;
	
	return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_DecClrDispFlag(DecHandle handle, int index) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * param) { 
    class mmp_lock autolock(m_p_mutex);

    CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	RetCode ret;

    ret = ::CheckDecInstanceValidity(handle);
	if (ret != RETCODE_SUCCESS)
		return ret;


	pCodecInst = handle;
	pDecInfo = &pCodecInst->CodecInfo.decInfo;
	switch (cmd) 
	{
	case ENABLE_ROTATION :
		{
			if (pDecInfo->rotatorStride == 0) {
				return RETCODE_ROTATOR_STRIDE_NOT_SET;
			}
			pDecInfo->rotationEnable = 1;
			break;
		}

	case DISABLE_ROTATION :
		{
			pDecInfo->rotationEnable = 0;
			break;
		}

	case ENABLE_MIRRORING :
		{
			if (pDecInfo->rotatorStride == 0) {
				return RETCODE_ROTATOR_STRIDE_NOT_SET;
			}
			pDecInfo->mirrorEnable = 1;
			break;
		}
	case DISABLE_MIRRORING :
		{
			pDecInfo->mirrorEnable = 0;
			break;
		}
	case SET_MIRROR_DIRECTION :
		{

			MirrorDirection mirDir;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			mirDir = *(MirrorDirection *)param;
			if (!(MIRDIR_NONE <= (int)mirDir && MIRDIR_HOR_VER >= (int)mirDir)) {
				return RETCODE_INVALID_PARAM;
			}
			pDecInfo->mirrorDirection = mirDir;

			break;
		}
	case SET_ROTATION_ANGLE :
		{
			int angle;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			angle = *(int *)param;
			if (angle != 0 && angle != 90 &&
				angle != 180 && angle != 270) {
					return RETCODE_INVALID_PARAM;
			}
			if (pDecInfo->rotatorStride != 0) {				
				if (angle == 90 || angle ==270) {
					if (pDecInfo->initialInfo.picHeight > pDecInfo->rotatorStride) {
						return RETCODE_INVALID_PARAM;
					}
				} else {
					if (pDecInfo->initialInfo.picWidth > pDecInfo->rotatorStride) {
						return RETCODE_INVALID_PARAM;
					}
				}
			}

			pDecInfo->rotationAngle = angle;
			break;
		}
	case SET_ROTATOR_OUTPUT :
		{
#ifdef SUPPORT_MEM_PROTECT
			PhysicalAddress start, end, ppuAddr;
#endif
			FrameBuffer *frame;
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
#ifdef SUPPORT_MEM_PROTECT
			if (pDecInfo->rotatorStride == 0) {
				return RETCODE_ROTATOR_STRIDE_NOT_SET;
			}
#endif
			frame = (FrameBuffer *)param;

			pDecInfo->rotatorOutput = *frame;
			pDecInfo->rotatorOutputValid = 1;
#ifdef SUPPORT_MEM_PROTECT
			start   = pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].startAddress;
			end     = pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress;
			ppuAddr = GetXY2AXIAddr(&pDecInfo->mapCfg, 0, 0, 0, pDecInfo->rotatorStride, &pDecInfo->rotatorOutput);
			start = (start< ppuAddr) ? start : ppuAddr;
			ppuAddr +=  VPU_GetFrameBufSize(pDecInfo->rotatorStride, pDecInfo->rotatorOutput.height,pDecInfo->rotatorOutput.mapType, FORMAT_420, &pDecInfo->dramCfg);
			end   = (end  > ppuAddr) ? end : ppuAddr;

			pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].startAddress = start;
			pDecInfo->writeMemProtectCfg.decRegion[WPROT_DEC_FRAME].endAddress   = end;
#endif
			break;
		}		

	case SET_ROTATOR_STRIDE :
		{
			int stride;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;

			}
			stride = *(int *)param;
			if (stride % 8 != 0 || stride == 0) {
				return RETCODE_INVALID_STRIDE;
			}

			if (pDecInfo->rotationAngle == 90 || pDecInfo->rotationAngle == 270) {
				if (pDecInfo->initialInfo.picHeight > stride) {
					return RETCODE_INVALID_STRIDE;
				}
			} else {
				if (pDecInfo->initialInfo.picWidth > stride) {
					return RETCODE_INVALID_STRIDE;
				}                   
			}

			pDecInfo->rotatorStride = stride;
			break;
		}
	case DEC_SET_SPS_RBSP:
		{
			if (pCodecInst->codecMode != AVC_DEC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			
			return SetParaSet(handle, 0, (DecParamSet *)param);
			
		}

	case DEC_SET_PPS_RBSP:
		{
			if (pCodecInst->codecMode != AVC_DEC) {
				return RETCODE_INVALID_COMMAND;
			}
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}

			return SetParaSet(handle, 1, (DecParamSet *)param);			
		}
	case ENABLE_DERING :
		{
			if (pDecInfo->rotatorStride == 0) {
				return RETCODE_ROTATOR_STRIDE_NOT_SET;
			}
			pDecInfo->deringEnable = 1;
			break;
		}

	case DISABLE_DERING :
		{
			pDecInfo->deringEnable = 0;
			break;
		}
	case SET_SEC_AXI:
		{
			SecAxiUse secAxiUse;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			secAxiUse = *(SecAxiUse *)param;
			pDecInfo->secAxiInfo.useBitEnable = secAxiUse.useBitEnable;
			pDecInfo->secAxiInfo.useIpEnable = secAxiUse.useIpEnable;
			pDecInfo->secAxiInfo.useDbkYEnable = secAxiUse.useDbkYEnable;
			pDecInfo->secAxiInfo.useDbkCEnable = secAxiUse.useDbkCEnable;
			pDecInfo->secAxiInfo.useOvlEnable = secAxiUse.useOvlEnable;
			pDecInfo->secAxiInfo.useBtpEnable = secAxiUse.useBtpEnable;
			break;
		}
	case ENABLE_REP_USERDATA:
		{
#ifdef SUPPORT_MEM_PROTECT	
			WriteMemProtectCfg *pCgf    = &pDecInfo->writeMemProtectCfg;
#endif
			if (!pDecInfo->userDataBufAddr) {
				return RETCODE_USERDATA_BUF_NOT_SET;
			}
			if (pDecInfo->userDataBufSize == 0) {
				return RETCODE_USERDATA_BUF_NOT_SET;
			}
			pDecInfo->userDataEnable = 1;
#ifdef SUPPORT_MEM_PROTECT
			pCgf->decRegion[WPROT_DEC_REPORT].enable = 1;
			pCgf->decRegion[WPROT_DEC_REPORT].isSecondary = 0;
			pCgf->decRegion[WPROT_DEC_REPORT].startAddress = pDecInfo->userDataBufAddr;
			pCgf->decRegion[WPROT_DEC_REPORT].endAddress = pDecInfo->userDataBufAddr + pDecInfo->userDataBufSize;
#endif
			break;
		}
	case DISABLE_REP_USERDATA:
		{
			pDecInfo->userDataEnable = 0;
			break;
		}
	case SET_ADDR_REP_USERDATA:
		{
			PhysicalAddress userDataBufAddr;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			userDataBufAddr = *(PhysicalAddress *)param;
			if (userDataBufAddr % 8 != 0 || userDataBufAddr == 0) {
				return RETCODE_INVALID_PARAM;
			}

			pDecInfo->userDataBufAddr = userDataBufAddr;
			break;
		}
	case SET_VIRT_ADDR_REP_USERDATA:
		{
			unsigned long userDataVirtAddr;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}

			if (!pDecInfo->userDataBufAddr) {
				return RETCODE_USERDATA_BUF_NOT_SET;
			}
			if (pDecInfo->userDataBufSize == 0) {
				return RETCODE_USERDATA_BUF_NOT_SET;
			}

			userDataVirtAddr = *(unsigned long *)param;
			if (!userDataVirtAddr) {
				return RETCODE_INVALID_PARAM;
			}


			pDecInfo->vbUserData.phys_addr = pDecInfo->userDataBufAddr;
			pDecInfo->vbUserData.size = pDecInfo->userDataBufSize;
			pDecInfo->vbUserData.virt_addr = (unsigned long)userDataVirtAddr;
			if (vdi_attach_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbUserData) != 0) {
				return RETCODE_INSUFFICIENT_RESOURCE;
			}
			break;
		}
	case SET_SIZE_REP_USERDATA:
		{
			PhysicalAddress userDataBufSize;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			userDataBufSize = *(PhysicalAddress *)param;

			pDecInfo->userDataBufSize = userDataBufSize;
			break;
		}

	case SET_USERDATA_REPORT_MODE:
		{
			int userDataMode;

			userDataMode = *(int *)param;
			if (userDataMode != 1 && userDataMode != 0) {
				return RETCODE_INVALID_PARAM;
			}
			pDecInfo->userDataReportMode = userDataMode;
			break;
		}
	case SET_CACHE_CONFIG:
		{
			MaverickCacheConfig *mcCacheConfig;
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}			
			mcCacheConfig = (MaverickCacheConfig *)param;
			pDecInfo->cacheConfig.CacheMode = mcCacheConfig->CacheMode;			
			break;
		}
	case SET_LOW_DELAY_CONFIG:
		{
			LowDelayInfo *lowDelayInfo;
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}
			lowDelayInfo = (LowDelayInfo *)param;

			if (lowDelayInfo->lowDelayEn) {
				if (pCodecInst->codecMode != AVC_DEC ||
					pDecInfo->rotationEnable ||
					pDecInfo->mirrorEnable ||
					pDecInfo->tiled2LinearEnable ||
					pDecInfo->deringEnable) {
					return RETCODE_INVALID_PARAM;
				}
			}

			pDecInfo->lowDelayInfo.lowDelayEn = lowDelayInfo->lowDelayEn;
			pDecInfo->lowDelayInfo.numRows = lowDelayInfo->numRows;
			if (lowDelayInfo->lowDelayEn)
				pDecInfo->reorderEnable = 0;
			break;
		}

	case SET_DECODE_FLUSH:	// interrupt mode to pic_end 
		{
			
			Uint32 val;

			if (pDecInfo->openParam.bitstreamMode != BS_MODE_INTERRUPT){
				return RETCODE_INVALID_COMMAND;
			}
			
			val = VpuReadReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM);
			val &= ~(3<<3);
			val |= (2<<3);	// set to pic_end mode
			VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_PARAM, val); 			
			break;
		}

	case DEC_SET_FRAME_DELAY:
		{
			pDecInfo->frameDelay = *(int *)param;
			break;
		}
	case DEC_ENABLE_REORDER:
		{
			if (pDecInfo->initialInfoObtained) {
				return RETCODE_WRONG_CALL_SEQUENCE;
			}
			pDecInfo->reorderEnable = 1;
			break;
		}
	case DEC_DISABLE_REORDER:
		{
			if (pDecInfo->initialInfoObtained) {
				return RETCODE_WRONG_CALL_SEQUENCE;
			}
			pDecInfo->reorderEnable = 0;
			break;
		}
	case DEC_SET_AVC_ERROR_CONCEAL_MODE:
		{
			if(pCodecInst->codecMode != AVC_DEC) {
				return RETCODE_INVALID_COMMAND;
			}

			if (pDecInfo->initialInfoObtained) {
				return RETCODE_WRONG_CALL_SEQUENCE;
			}

			pDecInfo->avcErrorConcealMode = *(AVCErrorConcealMode *)param;
			break;
		}	
	case DEC_FREE_FRAME_BUFFER:
		{
			if (pDecInfo->vbSlice.size)
				vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbSlice);

			if (pDecInfo->vbFrame.size){
				if (pDecInfo->frameAllocExt == 0) {
					vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbFrame);
				}				
			}	

			if (pDecInfo->vbMV.size) {
				vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbMV);
			}

			if (pDecInfo->vbPPU.size) {
				if (pDecInfo->ppuAllocExt == 0) {
					vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbPPU);
				}				
			}
			if (pDecInfo->wtlEnable)	//coda980 only
			{
				if (pDecInfo->vbWTL.size)
					vdi_free_dma_memory(pCodecInst->coreIdx, &pDecInfo->vbWTL);
			}
			break;
		}

	case ENABLE_DEC_THUMBNAIL_MODE:
		{
			if (pDecInfo->initialInfoObtained) {
				return RETCODE_WRONG_CALL_SEQUENCE;
			}
			pDecInfo->thumbnailMode = 1;
			break;
		}

	case DEC_GET_FIELD_PIC_TYPE:
		{
			SetClockGate(pCodecInst->coreIdx, 1);
			*((int *)param)  = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_TYPE) & 0x1f;
			SetClockGate(pCodecInst->coreIdx, 0);
			break;
		}
	case DEC_GET_DISPLAY_OUTPUT_INFO:
		{
			DecOutputInfo *pDecOutInfo = (DecOutputInfo *)param;
			*pDecOutInfo = pDecInfo->decOutInfo[pDecOutInfo->indexFrameDisplay];
			break;
		}
	case GET_TILEDMAP_CONFIG:
		{
			TiledMapConfig *pMapCfg = (TiledMapConfig *)param;
			if (!pMapCfg) {
				return RETCODE_INVALID_PARAM;
			}
			if (!pDecInfo->stride) {
				return RETCODE_WRONG_CALL_SEQUENCE;

			}
			*pMapCfg = pDecInfo->mapCfg;
			break;
		}
	case SET_DRAM_CONFIG:
		{
			DRAMConfig *cfg = (DRAMConfig *)param;

			if (!cfg) {
				return RETCODE_INVALID_PARAM;
			}

			pDecInfo->dramCfg = *cfg;
			break;
		}
	case GET_DRAM_CONFIG:
		{
			DRAMConfig *cfg = (DRAMConfig *)param;

			if (!cfg) {
				return RETCODE_INVALID_PARAM;
			}

			*cfg = pDecInfo->dramCfg;

			break;
		}
	case GET_LOW_DELAY_OUTPUT:
		{
			DecOutputInfo *lowDelayOutput;
			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}

			if (!pDecInfo->lowDelayInfo.lowDelayEn || pCodecInst->codecMode != AVC_DEC) {
				return RETCODE_INVALID_COMMAND;
			}
			
			if (pCodecInst != GetPendingInst(pCodecInst->coreIdx)) {
				return RETCODE_WRONG_CALL_SEQUENCE;
			}

			lowDelayOutput = (DecOutputInfo *)param;

			GetLowDelayOutput(pCodecInst, lowDelayOutput);
						
			break;
		}
	case ENABLE_LOGGING:
		{
			pCodecInst->loggingEnable = 1;			
		}
		break;
	case DISABLE_LOGGING:
		{
			pCodecInst->loggingEnable = 0;
		}
		break;		
	case DEC_SET_DISPLAY_FLAG:
		{
			int index;

			if (param == 0) {
				return RETCODE_INVALID_PARAM;
			}

			index = *(int *) param;

			EnterDispFlagLock(pCodecInst->coreIdx);
			pDecInfo->setDisplayIndexes |= (1<<index);
			LeaveDispFlagLock(pCodecInst->coreIdx);
		}
		break;
		
	case DEC_GET_SEQ_INFO:
		{
			DecInitialInfo* seqInfo = (DecInitialInfo*)param;
			if (!seqInfo) {
				return RETCODE_INVALID_PARAM;
			}

			if (!pDecInfo->initialInfoObtained) {
				return RETCODE_WRONG_CALL_SEQUENCE;
			}

			*seqInfo = pDecInfo->initialInfo;
		}
		break;
	default:
		return RETCODE_INVALID_COMMAND;
	}
	
    return RETCODE_SUCCESS;
}

RetCode  mmp_vpu_dev_ex1::VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS;
}

// function for encode
RetCode  mmp_vpu_dev_ex1::VPU_EncOpen(EncHandle *pHandle,	EncOpenParam *pop) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncClose(EncHandle handle) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncGetInitialInfo(EncHandle handle,	EncInitialInfo * info) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride, int height, int mapType) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncGetFrameBuffer(EncHandle handle,	int frameIdx, FrameBuffer *frameBuf) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncGetBitstreamBuffer(EncHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int * size) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncUpdateBitstreamBuffer(EncHandle handle, int size) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncStartOneFrame(EncHandle handle, EncParam * param) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * param) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}

RetCode  mmp_vpu_dev_ex1::VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) { 
    class mmp_lock autolock(m_p_mutex);

    return RETCODE_SUCCESS; 
}


