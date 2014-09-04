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


#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "mmp_vpu_dev.hpp"
#include "mmp_vpu_dev_ex2.hpp" 

#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_vpu_dev* mmp_vpu_dev::s_p_instance = NULL;

MMP_RESULT mmp_vpu_dev::create_instance() {
    
    struct mmp_vpu_dev_create_config vpu_create_config;
    memset(&vpu_create_config, 0x00, sizeof(struct mmp_vpu_dev_create_config));

    return mmp_vpu_dev::create_instance(&vpu_create_config);
}

MMP_RESULT mmp_vpu_dev::create_instance(struct mmp_vpu_dev_create_config* p_create_config) {

    MMP_RESULT mmpResult;
	class mmp_vpu_dev* p_obj = NULL;

	//p_obj = new class mmp_vpu_dev_ex1(p_create_config);
	p_obj = new class mmp_vpu_dev_ex2(p_create_config);
	if(p_obj!=NULL) {
        
		if(p_obj->open( ) != MMP_ErrorNone)
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    if(p_obj != NULL) {
        mmp_vpu_dev::s_p_instance = p_obj;
        mmpResult = MMP_SUCCESS;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}

MMP_RESULT mmp_vpu_dev::destroy_instance() {

    class mmp_vpu_dev* p_obj = mmp_vpu_dev::s_p_instance;

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;

        mmp_vpu_dev::s_p_instance = NULL;
    }

    return MMP_SUCCESS;
}

class mmp_vpu_dev* mmp_vpu_dev::get_instance() {

    return mmp_vpu_dev::s_p_instance;
}

extern "C" int vdi_allocate_dma_memory_anapass(unsigned long coreIdx, vpu_buffer_t *vb) {
    return mmp_vpu_dev::get_instance()->vdi_allocate_dma_memory(coreIdx, vb);
}
/**********************************************************
class members
**********************************************************/

#if (MMP_OS == MMP_OS_WIN32)
#define EXPORT_VAR extern "C"
#else
#define EXPORT_VAR extern
#endif

EXPORT_VAR int g_vpu_fd = -1; 
EXPORT_VAR unsigned int g_vpu_reg_vir_addr = 0;
EXPORT_VAR unsigned char* g_p_instance_pool_buffer=NULL;
EXPORT_VAR void* g_p_vpu_common_buffer = NULL;

static vpu_buffer_t s_vpu_common_buffer;

mmp_vpu_dev::mmp_vpu_dev(struct mmp_vpu_dev_create_config* p_create_config) :
m_create_config(*p_create_config)

,m_vpu_fd(-1)
,m_p_instance_pool_buffer(NULL)
{

}

mmp_vpu_dev::~mmp_vpu_dev() {

}

MMP_RESULT mmp_vpu_dev::open() {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 sz;

    /* open driver */
    if(mmpResult == MMP_SUCCESS) {

        m_vpu_fd = MMP_DRIVER_OPEN(VPU_DEVICE_NAME, O_RDWR);
        if(m_vpu_fd < 0) {
            mmpResult = MMP_FAILURE;
        }
        else {
            g_vpu_fd = m_vpu_fd;
        }
    }

    /* alloc instance pool */
    if(mmpResult == MMP_SUCCESS) {
        sz = MAX_INST_HANDLE_SIZE*(MAX_NUM_INSTANCE+1) + 1024*12;
        m_p_instance_pool_buffer = (MMP_U8*)malloc(sz);
        if(m_p_instance_pool_buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            memset(m_p_instance_pool_buffer, 0x00, sz);
            g_p_instance_pool_buffer = m_p_instance_pool_buffer;
        }
    }

    /* get register */
    if(mmpResult == MMP_SUCCESS) {
        this->m_vdb_register.size = VPU_BIT_REG_SIZE;
	    this->m_vdb_register.virt_addr = (unsigned long)MMP_DRIVER_MMAP(NULL, this->m_vdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, this->m_vpu_fd, 0);
        this->m_vdb_register.phys_addr = ANA_CODEC_SRAM_BASE;
	    if ( (void*)this->m_vdb_register.virt_addr == MAP_FAILED)
	    {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_vpu_dev_ex1::open] FAIL: map vpu registers")));
		    mmpResult = MMP_FAILURE;
	    }
        else {
            m_vpu_reg_buf.m_phy_addr = this->m_vdb_register.phys_addr;
            m_vpu_reg_buf.m_vir_addr = this->m_vdb_register.virt_addr;

            g_vpu_reg_vir_addr = this->m_vdb_register.virt_addr;
        }
    }

    /* create common buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_vpu_common_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(SIZE_COMMON);
        if(m_p_vpu_common_buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            m_code_buf = m_p_vpu_common_buffer->get_buf_addr();
	        m_parm_buf = m_code_buf + CODE_BUF_SIZE;
	        m_temp_buf = m_parm_buf + TEMP_BUF_SIZE;

            s_vpu_common_buffer.phys_addr = m_code_buf.m_phy_addr;
            s_vpu_common_buffer.virt_addr = m_code_buf.m_vir_addr;
            s_vpu_common_buffer.size = m_code_buf.m_size;
            s_vpu_common_buffer.ion_shared_fd = m_code_buf.m_shared_fd;
            s_vpu_common_buffer.base = s_vpu_common_buffer.virt_addr;
            g_p_vpu_common_buffer = (void*)&s_vpu_common_buffer;
	    }
    }

    return mmpResult;
}

MMP_RESULT mmp_vpu_dev::close() {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    if( (void*)this->m_vdb_register.virt_addr != MAP_FAILED) {
        MMP_DRIVER_MUNMAP((void*)this->m_vdb_register.virt_addr, this->m_vdb_register.size);
        this->m_vdb_register.virt_addr = (unsigned long)MAP_FAILED;
    }

    
    if(m_p_vpu_common_buffer != NULL) {
        mmp_buffer_mgr::get_instance()->free_buffer(m_p_vpu_common_buffer);
        m_p_vpu_common_buffer = NULL;
    }

    if(m_vpu_fd >= 0) {
       MMP_DRIVER_CLOSE(m_vpu_fd);
       m_vpu_fd = -1;
    }

    if(m_p_instance_pool_buffer != NULL) {
        free(m_p_instance_pool_buffer);
        m_p_instance_pool_buffer = NULL;
    }

    return mmpResult;
}

#if 0
RetCode mmp_vpu_dev::BitLoadFirmware(Uint32 coreIdx, PhysicalAddress codeBase, const Uint16 *codeWord, int codeSize)
{
	int i;
	Uint32 data;
    
	//const Uint16 *codeWord = bit_code;
    //int codeSize = sizeof(bit_code)/sizeof(bit_code[0]);
    BYTE code[8];
    //MMP_U32 codeBase;
	
    //codeBase = m_code_buf.m_vir_addr;
	
	for (i=0; i<codeSize; i+=4) {
		// 2byte little endian variable to 1byte big endian buffer
		code[0] = (BYTE)(codeWord[i+0]>>8);
		code[1] = (BYTE)codeWord[i+0];
		code[2] = (BYTE)(codeWord[i+1]>>8);
		code[3] = (BYTE)codeWord[i+1];
		code[4] = (BYTE)(codeWord[i+2]>>8);
		code[5] = (BYTE)codeWord[i+2];
		code[6] = (BYTE)(codeWord[i+3]>>8);
		code[7] = (BYTE)codeWord[i+3];
		this->VpuWriteMem(coreIdx, codeBase+i*2, (BYTE *)code, 8, VDI_BIG_ENDIAN);
	}

	this->VpuWriteReg(coreIdx, BIT_INT_ENABLE, 0);
	this->VpuWriteReg(coreIdx, BIT_CODE_RUN, 0);
	
	for (i=0; i<2048; ++i) {
		data = codeWord[i];
		this->VpuWriteReg(coreIdx, BIT_CODE_DOWN, (i << 16) | data);
	}
	
	this->vdi_set_bit_firmware_to_pm(coreIdx, codeWord); 

    return RETCODE_SUCCESS;
}


RetCode mmp_vpu_dev::SetClockGate(Uint32 coreIdx, Uint32 on) {
    RetCode ret;
    ret = (RetCode)MMP_DRIVER_IOCTL(this->m_vpu_fd, VDI_IOCTL_SET_CLOCK_GATE, &on);
    return ret;
}

int mmp_vpu_dev::vdi_get_common_memory(unsigned long core_idx, vpu_buffer_t *vb) {
    
    #if 0
    typedef struct vpu_buffer_t {
	unsigned int size;
	unsigned int  phys_addr;
#ifdef CNM_HISI_PLATFORM	
	unsigned long long base;
#else
	unsigned long base;
#endif
	unsigned long virt_addr;
    
    int ion_shared_fd; 
} vpu_buffer_t;
#endif

    int iret = -1;
    class mmp_buffer* p_mmp_buf;
    class mmp_buffer_addr buf_addr;

    p_mmp_buf = this->m_p_vpu_common_buffer;//mmp_buffer_mgr::get_instance()->alloc_dma_buffer(MMP_BUFFER_TYPE_DMA, vb->size);
    if(p_mmp_buf != NULL) {
        buf_addr = p_mmp_buf->get_buf_addr();
        vb->phys_addr = buf_addr.m_phy_addr;
        vb->virt_addr = buf_addr.m_vir_addr;
        vb->size = buf_addr.m_size;
        vb->ion_shared_fd = buf_addr.m_shared_fd;
        vb->base = vb->virt_addr;
        iret = 0;
    }   

    return iret;
}

int mmp_vpu_dev::vdi_wait_vpu_busy(unsigned long coreIdx, int timeout, unsigned int addr_bit_busy_flag)
{
	MMP_U32 regvalue;
    MMP_U32 start_tick, cur_tick;
    int iret = 0;
	
    start_tick = CMmpUtil::GetTickCount();
	while(1)
	{
        regvalue = this->VpuReadReg(coreIdx, addr_bit_busy_flag);
        if(regvalue == 0) {
			break;
        }

        cur_tick = CMmpUtil::GetTickCount();

        if( (cur_tick-start_tick) > (MMP_U32)timeout) {
            iret = -1;
            break;
        }
		
        CMmpUtil::Sleep(1);
	}

	return iret;
}

int mmp_vpu_dev::vdi_wait_bus_busy(unsigned long coreIdx, int timeout, unsigned int gdi_busy_flag)
{
	MMP_U32 start_tick, cur_tick;

    start_tick = CMmpUtil::GetTickCount();
	while(1)
	{
		if(this->vdi_read_register(coreIdx, gdi_busy_flag) == 0x77)
			break;

        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick-start_tick) > (MMP_U32)timeout) {
            return -1;
        }
		
        CMmpUtil::Sleep(1);
	}

	return 0;
}

int mmp_vpu_dev::vdi_set_bit_firmware_to_pm(unsigned long coreIdx, const unsigned short *code)
{
	int i;
	vpu_bit_firmware_info_t bit_firmware_info;

	bit_firmware_info.size = sizeof(vpu_bit_firmware_info_t);
	bit_firmware_info.core_idx = coreIdx;
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
	bit_firmware_info.reg_base_offset = (coreIdx*VPU_CORE_BASE_OFFSET);
#else
	bit_firmware_info.reg_base_offset = 0;
#endif
	for (i=0; i<512; i++)
		bit_firmware_info.bit_code[i] = code[i];

	if (MMP_DRIVER_WRITE(this->m_vpu_fd, (char*)&bit_firmware_info, bit_firmware_info.size) < 0)
	{
		VLOG(ERR, "[VDI] fail to vdi_set_bit_firmware core=%d\n", bit_firmware_info.core_idx);
		return -1;
	}

	return 0;
}

int mmp_vpu_dev::vdi_get_sram_memory(unsigned long coreIdx, vpu_buffer_t *vb)
{
    vpudrv_buffer_t vdb;
#if 0
	vdi_info_t *vdi = &s_vdi_info[coreIdx];
	

	if(!vb || !vdi || vdi->vpu_fd==-1 || vdi->vpu_fd == 0x00)
		return -1;
#endif


	osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

	if (VDI_SRAM_SIZE > 0)	// if we can know the sram address directly in vdi layer, we use it first for sdram address
	{
		vb->phys_addr = VDI_SRAM_BASE_ADDR+(coreIdx*VDI_SRAM_SIZE);		
		vb->size = VDI_SRAM_SIZE;

		return 0;
	}

	return 0;
}

int mmp_vpu_dev::VpuWriteMem(unsigned long coreIdx, unsigned int vir_addr, unsigned char *data, int len, int endian) {

    this->swap_endian(data, len, endian);
	memcpy((void *)vir_addr, data, len);

	return len;
}

int mmp_vpu_dev::swap_endian(unsigned char *data, int len, int endian)
{
	unsigned int *p;
	unsigned int v1, v2, v3;
	int i;
	const int sys_endian = VDI_SYSTEM_ENDIAN;
	int swap = 0;
	p = (unsigned int *)data;

	if(endian == sys_endian)
		swap = 0;
	else
		swap = 1;

	if (swap)
	{
		if (endian == VDI_LITTLE_ENDIAN ||
			endian == VDI_BIG_ENDIAN) {
				for (i=0; i<len/4; i+=2)
				{
					v1 = p[i];
					v2  = ( v1 >> 24) & 0xFF;
					v2 |= ((v1 >> 16) & 0xFF) <<  8;
					v2 |= ((v1 >>  8) & 0xFF) << 16;
					v2 |= ((v1 >>  0) & 0xFF) << 24;
					v3 =  v2;
					v1  = p[i+1];
					v2  = ( v1 >> 24) & 0xFF;
					v2 |= ((v1 >> 16) & 0xFF) <<  8;
					v2 |= ((v1 >>  8) & 0xFF) << 16;
					v2 |= ((v1 >>  0) & 0xFF) << 24;
					p[i]   =  v2;
					p[i+1] = v3;
				}
		}
		else
		{
			int swap4byte = 0;
			if (endian == VDI_32BIT_LITTLE_ENDIAN)
			{
				if (sys_endian == VDI_BIG_ENDIAN)
				{
					swap = 1;
					swap4byte = 0;
				}
				else if (sys_endian == VDI_32BIT_BIG_ENDIAN)
				{
					swap = 1;
					swap4byte = 1;
				}
				else if (sys_endian == VDI_LITTLE_ENDIAN)
				{
					swap = 0;
					swap4byte = 1;
				}
			}
			else	// VDI_32BIT_BIG_ENDIAN
			{
				if (sys_endian == VDI_LITTLE_ENDIAN)
				{
					swap = 1;
					swap4byte = 0;
				}
				else if (sys_endian == VDI_32BIT_LITTLE_ENDIAN)
				{
					swap = 1;
					swap4byte = 1;
				}
				else if (sys_endian == VDI_BIG_ENDIAN)
				{
					swap = 0;
					swap4byte = 1;
				}
			}
			if (swap) {
				for (i=0; i<len/4; i++) {
					v1 = p[i];
					v2  = ( v1 >> 24) & 0xFF;
					v2 |= ((v1 >> 16) & 0xFF) <<  8;
					v2 |= ((v1 >>  8) & 0xFF) << 16;
					v2 |= ((v1 >>  0) & 0xFF) << 24;
					p[i] = v2;
				}
			}
			if (swap4byte) {
				for (i=0; i<len/4; i+=2) {
					v1 = p[i];
					v2 = p[i+1];
					p[i]   = v2;
					p[i+1] = v1;
				}
			}
		}
	}

	return swap;
}

RetCode mmp_vpu_dev::EnterLock(Uint32 coreIdx) {

    return RETCODE_SUCCESS;
}
	
RetCode mmp_vpu_dev::LeaveLock(Uint32 coreIdx) {

    return RETCODE_SUCCESS;
}
    
CodecInst *mmp_vpu_dev::GetPendingInst(Uint32 coreIdx) {
    return NULL;
}

void mmp_vpu_dev::BitIssueCommand(Uint32 coreIdx, CodecInst *inst, int cmd)
{
	int instIdx = 0;
	int cdcMode = 0;
	int auxMode = 0;
		
	if (inst != NULL) // command is specific to instance
	{
		instIdx = inst->instIndex;
		cdcMode = inst->codecMode;
		auxMode = inst->codecModeAux;
	}


	if (inst) {
		if (inst->codecMode < AVC_ENC)	{
			this->VpuWriteReg(coreIdx, BIT_WORK_BUF_ADDR, inst->CodecInfo.decInfo.vbWork.phys_addr);		
#ifdef SUPPORT_MEM_PROTECT
			SetDecWriteProtectRegions(inst);
#endif
		}
		else {
			this->VpuWriteReg(coreIdx, BIT_WORK_BUF_ADDR, inst->CodecInfo.encInfo.vbWork.phys_addr);
#ifdef SUPPORT_MEM_PROTECT
			SetEncWriteProtectRegions(inst);
#endif
		}
	}	

	this->VpuWriteReg(coreIdx, BIT_BUSY_FLAG, 1);
	this->VpuWriteReg(coreIdx, BIT_RUN_INDEX, instIdx);
	this->VpuWriteReg(coreIdx, BIT_RUN_COD_STD, cdcMode);
	this->VpuWriteReg(coreIdx, BIT_RUN_AUX_STD, auxMode);
	if (inst && inst->loggingEnable)
		vdi_log(coreIdx, cmd, 1);

	this->VpuWriteReg(coreIdx, BIT_RUN_COMMAND, cmd);
}


/* etc */
int mmp_vpu_dev::vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) {
    
#if 0
    typedef struct vpu_buffer_t {
	unsigned int size;
	unsigned int  phys_addr;
#ifdef CNM_HISI_PLATFORM	
	unsigned long long base;
#else
	unsigned long base;
#endif
	unsigned long virt_addr;
    
    int ion_shared_fd; 
} vpu_buffer_t;
#endif

    int iret = -1;
    class mmp_buffer* p_mmp_buf;
    class mmp_buffer_addr buf_addr;

    p_mmp_buf = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(MMP_BUFFER_TYPE_DMA, vb->size);
    if(p_mmp_buf != NULL) {
        buf_addr = p_mmp_buf->get_buf_addr();
        vb->phys_addr = buf_addr.m_phy_addr;
        vb->virt_addr = buf_addr.m_vir_addr;
        vb->size = buf_addr.m_size;
        vb->ion_shared_fd = buf_addr.m_shared_fd;
        vb->base = vb->virt_addr;
        iret = 0;
    }   

    return iret;
}

void mmp_vpu_dev::vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) {

    class mmp_buffer_addr buf_addr;

    buf_addr.m_phy_addr = vb->phys_addr;
    buf_addr.m_vir_addr = vb->virt_addr;
    buf_addr.m_size = vb->size;
    buf_addr.m_shared_fd = vb->ion_shared_fd;
    
    mmp_buffer_mgr::get_instance()->free_buffer(buf_addr);
}

RetCode mmp_vpu_dev::SetParaSet(DecHandle handle, int paraSetType, DecParamSet * para)
{
	CodecInst * pCodecInst;
	PhysicalAddress paraBuffer;
	int i;
	Uint32 * src;
	
	
	pCodecInst = handle;
	src = para->paraSet;

	EnterLock(pCodecInst->coreIdx);

	paraBuffer = VpuReadReg(pCodecInst->coreIdx, BIT_PARA_BUF_ADDR);
	for (i = 0; i < para->size; i += 4) {
		VpuWriteReg(pCodecInst->coreIdx, paraBuffer + i, *src++);
	}
	VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_PARA_SET_TYPE, paraSetType); // 0: SPS, 1: PPS
	VpuWriteReg(pCodecInst->coreIdx, CMD_DEC_PARA_SET_SIZE, para->size);

	BitIssueCommand(pCodecInst->coreIdx, pCodecInst, DEC_PARA_SET);
	if (vdi_wait_vpu_busy(pCodecInst->coreIdx, VPU_BUSY_CHECK_TIMEOUT, BIT_BUSY_FLAG) == -1) {
		if (pCodecInst->loggingEnable)
			vdi_log(pCodecInst->coreIdx, DEC_PARA_SET, 2);
		LeaveLock(pCodecInst->coreIdx);
		return RETCODE_VPU_RESPONSE_TIMEOUT;
	}
	if (pCodecInst->loggingEnable)
		vdi_log(pCodecInst->coreIdx, DEC_PARA_SET, 0);
	
	LeaveLock(pCodecInst->coreIdx);
	return RETCODE_SUCCESS;
}

int mmp_vpu_dev::WriteBsBufFromBufHelper(Uint32 core_idx,  DecHandle handle,  vpu_buffer_t *pVbStream, BYTE *pChunk, int chunkSize, int endian) {

    RetCode ret = RETCODE_SUCCESS;
    int size = 0;
    int room = 0;
    PhysicalAddress paRdPtr, paWrPtr, targetAddr;
    
    
    if (chunkSize < 1)
        return 0;

    if (chunkSize > (int)pVbStream->size)
        return -1;	

    ret = this->VPU_DecGetBitstreamBuffer(handle, &paRdPtr, &paWrPtr, &size);
    if( ret != RETCODE_SUCCESS )
    {
        VLOG(ERR, "VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n", ret );
        return -1;			
    }

    if(size < chunkSize)
        return 0; // no room for feeding bitstream. it will take a change to fill stream

    targetAddr = paWrPtr;
    if ((targetAddr+chunkSize) >  (pVbStream->phys_addr+pVbStream->size))
    {
        room =  (pVbStream->phys_addr+pVbStream->size) - targetAddr;

        //write to physical address
        this->vdi_write_memory(core_idx, targetAddr, pChunk, room, endian);
        this->vdi_write_memory(core_idx, pVbStream->phys_addr, pChunk+room, chunkSize-room, endian);

    }
    else
    {
        //write to physical address
        this->vdi_write_memory(core_idx, targetAddr, pChunk, chunkSize, endian);
    }

    ret = this->VPU_DecUpdateBitstreamBuffer(handle, chunkSize);
    if( ret != RETCODE_SUCCESS )
    {
        VLOG(ERR, "VPU_DecUpdateBitstreamBuffer failed Error code is 0x%x \n", ret );
        return -1;					
    }

    return chunkSize;
}

RetCode mmp_vpu_dev::CheckInstanceValidity(CodecInst * pCodecInst)
{
#if 0
	int i;
	vpu_instance_pool_t *vip;

	vip = (vpu_instance_pool_t *)vdi_get_instance_pool(pCodecInst->coreIdx);
	if (!vip)
		return RETCODE_INSUFFICIENT_RESOURCE;

	for (i = 0; i < MAX_NUM_INSTANCE; i++) {
		if ((CodecInst *)vip->codecInstPool[i] == pCodecInst)
			return RETCODE_SUCCESS;
	}
	return RETCODE_INVALID_HANDLE;
#else

    return RETCODE_SUCCESS;
#endif
}

RetCode mmp_vpu_dev::CheckDecInstanceValidity(DecHandle handle)
{
	CodecInst * pCodecInst;
	RetCode ret;

	pCodecInst = handle;
	ret = this->CheckInstanceValidity(pCodecInst);
	if (ret != RETCODE_SUCCESS) {
		return RETCODE_INVALID_HANDLE;
	}
	if (!pCodecInst->inUse) {
		return RETCODE_INVALID_HANDLE;
	}
	if (pCodecInst->codecMode != AVC_DEC && 
		pCodecInst->codecMode != VC1_DEC &&
		pCodecInst->codecMode != MP2_DEC &&
		pCodecInst->codecMode != MP4_DEC &&
		pCodecInst->codecMode != DV3_DEC &&
		pCodecInst->codecMode != RV_DEC &&
		pCodecInst->codecMode != AVS_DEC &&
        pCodecInst->codecMode != VPX_DEC
        ) 
    {
		return RETCODE_INVALID_HANDLE;
	}

	return RETCODE_SUCCESS;
}

RetCode mmp_vpu_dev::AllocateFrameBufferArray(int coreIdx, FrameBuffer *frambufArray, vpu_buffer_t *pvbFrame, int mapType, int interleave, int framebufFormat, int num, int stride, int memHeight, int gdiIndex, int fbType, PhysicalAddress tiledBaseAddr, DRAMConfig *pDramCfg)
{
	int chr_hscale, chr_vscale;
	int size_dpb_lum, size_dpb_chr, size_dpb_all;
	int alloc_by_user;		
	int chr_size_y, chr_size_x;
	int i, width, height;
	PhysicalAddress addrY;
	int size_dpb_lum_4k, size_dpb_chr_4k, size_dpb_all_4k;
	
	alloc_by_user = (frambufArray[0].bufCb == (PhysicalAddress)-1 && frambufArray[0].bufCr == (PhysicalAddress)-1);

	height = memHeight;
	width = stride;
    
    switch (framebufFormat)
	{
	case FORMAT_420:
		height = (memHeight+1)/2*2;
		width = (stride+1)/2*2;
		break;
	case FORMAT_224:
		height = (memHeight+1)/2*2;
		break;
	case FORMAT_422:
		width = (stride+1)/2*2;
		break;
	case FORMAT_444:
		break;
	case FORMAT_400:
		height = (memHeight+1)/2*2;
		width = (stride+1)/2*2;	
		break;
	default:
		return RETCODE_INVALID_PARAM;
	}

	chr_hscale = framebufFormat == FORMAT_420 || framebufFormat == FORMAT_422 ? 2 : 1;
	chr_vscale = framebufFormat == FORMAT_420 || framebufFormat == FORMAT_224 ? 2 : 1;

	if (mapType == LINEAR_FRAME_MAP) { // AllocateFrameBuffer
		chr_size_y = (height/chr_hscale); 
		chr_size_x = (width/chr_vscale);

		size_dpb_lum   = width * height;
		size_dpb_chr   = chr_size_y * chr_size_x;
		size_dpb_all   = size_dpb_lum + (size_dpb_chr*2);

		pvbFrame->size     = size_dpb_all*num;
		if (alloc_by_user) {					
		}
		else {
			if (vdi_allocate_dma_memory(coreIdx, pvbFrame)<0)
				return RETCODE_INSUFFICIENT_RESOURCE;
		}

		addrY = pvbFrame->phys_addr;
		for (i=0; i<num; i++)
		{	
			if (alloc_by_user)
				addrY = frambufArray[i].bufY;

            frambufArray[i].ion_shared_fd = pvbFrame->ion_shared_fd;
            frambufArray[i].ion_base_phyaddr = pvbFrame->phys_addr;

			frambufArray[i].myIndex = i+gdiIndex;
			frambufArray[i].mapType = mapType;
			frambufArray[i].height = memHeight;
			frambufArray[i].bufY    = addrY;
			frambufArray[i].bufCb   = frambufArray[i].bufY + size_dpb_lum;
			if (!interleave)
				frambufArray[i].bufCr = frambufArray[i].bufY + size_dpb_lum + size_dpb_chr;
            frambufArray[i].stride         = width;		
            frambufArray[i].cbcrInterleave = interleave;
			frambufArray[i].sourceLBurstEn = 0;
			{				
				unsigned char *tt;
				tt = (unsigned char *)osal_malloc(size_dpb_all);
				osal_memset(tt,0,size_dpb_all);
				VpuWriteMem(coreIdx, addrY, tt, size_dpb_all, 0);
				osal_free(tt);
			}
			addrY += size_dpb_all;			
		}
		
	}
	else if (mapType == TILED_FRAME_MB_RASTER_MAP || mapType == TILED_FIELD_MB_RASTER_MAP) { //AllocateMBRasterTiled
		chr_size_x = width/chr_hscale;
		chr_size_y = height/chr_hscale;
		size_dpb_lum   = width * height;
		size_dpb_chr   = chr_size_y * chr_size_x;

		// aligned to 8192*2 (0x4000) for top/bot field
		// use upper 20bit address only
		size_dpb_lum_4k	=  ((size_dpb_lum + 16383)/16384)*16384;
		size_dpb_chr_4k	=  ((size_dpb_chr + 16383)/16384)*16384;
		size_dpb_all_4k =  size_dpb_lum_4k + 2*size_dpb_chr_4k;

		if (mapType == TILED_FIELD_MB_RASTER_MAP)
		{
			size_dpb_lum_4k = ((size_dpb_lum_4k+(0x8000-1))&~(0x8000-1));
			size_dpb_chr_4k = ((size_dpb_chr_4k+(0x8000-1))&~(0x8000-1));
			size_dpb_all_4k =  size_dpb_lum_4k + 2*size_dpb_chr_4k;
		}

		pvbFrame->size     = size_dpb_all_4k*num;
		if (alloc_by_user) {			
		}
		else 
		{
			if (vdi_allocate_dma_memory(coreIdx, pvbFrame)<0)
				return RETCODE_INSUFFICIENT_RESOURCE;
		}

		addrY = ((pvbFrame->phys_addr+(16384-1))&~(16384-1));

		for (i=0; i<num; i++)
		{
			int  lum_top_base;
			int  lum_bot_base;
			int  chr_top_base;
			int  chr_bot_base;
			
			//-------------------------------------
			// use tiled map
			//-------------------------------------
			if (alloc_by_user)
				addrY = ((frambufArray[i].bufY+(16384-1))&~(16384-1));

			lum_top_base = addrY;
			lum_bot_base = addrY + size_dpb_lum_4k/2;
			chr_top_base = addrY + size_dpb_lum_4k;
			chr_bot_base = addrY + size_dpb_lum_4k + size_dpb_chr_4k; // cbcr is interleaved

			lum_top_base = (lum_top_base>>12) & 0xfffff;
			lum_bot_base = (lum_bot_base>>12) & 0xfffff;
			chr_top_base = (chr_top_base>>12) & 0xfffff;
			chr_bot_base = (chr_bot_base>>12) & 0xfffff;
			
			frambufArray[i].myIndex = i+gdiIndex;
			frambufArray[i].mapType = mapType;
			frambufArray[i].height = memHeight;
			// {AddrY,AddrCb,AddrCr} = {lum_top_base[31:12],chr_top_base[31:12],lum_bot_base[31:12],chr_bot_base[31:12],16'b0}
			// AddrY  = {lum_top_base[31:12],chr_top_base[31:20]} : 20 + 12 bit
			// AddrCb = {chr_top_base[19:12],lum_bot_base[31:20],chr_bot_base[31:28]} : 8 + 20 + 4 bit
			// AddrCr = {chr_bot_base[27:12],16'b0} : 16 bit
			frambufArray[i].bufY  = ( lum_top_base           << 12) | (chr_top_base >> 8);
			frambufArray[i].bufCb = ((chr_top_base & 0xff  ) << 24) | (lum_bot_base << 4) | (chr_bot_base >> 16);
			frambufArray[i].bufCr = ((chr_bot_base & 0xffff) << 16) ;
            frambufArray[i].stride         = width;		
            frambufArray[i].cbcrInterleave = interleave;
			frambufArray[i].sourceLBurstEn = 0;
			addrY += size_dpb_all_4k;
		}

	}
	else {
		PhysicalAddress addrYRas;
		int ChrSizeYField;

		int  VerSizePerRas,HorSizePerRas,RasLowBitsForHor; 
		int  ChrFieldRasSize,ChrFrameRasSize,LumFieldRasSize,LumFrameRasSize;
		int  LumRasTop,LumRasBot,ChrRasTop,ChrRasBot;     

		if (mapType == TILED_FRAME_V_MAP) {
			if (pDramCfg->casBit == 9 && pDramCfg->bankBit == 2 && pDramCfg->rasBit == 13)	// CNN setting 
			{
				VerSizePerRas = 64;
				HorSizePerRas = 256;
				RasLowBitsForHor = 3;
			}
			else if(pDramCfg->casBit == 10 && pDramCfg->bankBit == 3 && pDramCfg->rasBit == 13)
			{
				VerSizePerRas = 64;
				HorSizePerRas = 512;
				RasLowBitsForHor = 2;
			}
			else
				return RETCODE_INVALID_PARAM;			
		} else if (mapType == TILED_FRAME_H_MAP) {
			if (pDramCfg->casBit == 9 && pDramCfg->bankBit == 2 && pDramCfg->rasBit == 13)	// CNN setting 
			{
				VerSizePerRas = 64;
				HorSizePerRas = 256;
				RasLowBitsForHor = 3;
			}
			else if(pDramCfg->casBit == 10 && pDramCfg->bankBit == 3 && pDramCfg->rasBit == 13)
			{
				VerSizePerRas = 64;
				HorSizePerRas = 512;
				RasLowBitsForHor = 2;
			}
			else
				return RETCODE_INVALID_PARAM;			
		} else if (mapType == TILED_FIELD_V_MAP) {
			if (pDramCfg->casBit == 9 && pDramCfg->bankBit == 2 && pDramCfg->rasBit == 13)	// CNN setting 
			{
				VerSizePerRas = 64;
				HorSizePerRas = 256;
				RasLowBitsForHor = 3;
			}
			else if(pDramCfg->casBit == 10 && pDramCfg->bankBit == 3 && pDramCfg->rasBit == 13)
			{
				VerSizePerRas = 64;
				HorSizePerRas = 512;
				RasLowBitsForHor = 2;
			}
			else
				return RETCODE_INVALID_PARAM;			
		} else {
			if (pDramCfg->casBit == 9 && pDramCfg->bankBit == 2 && pDramCfg->rasBit == 13)	// CNN setting 
			{
				VerSizePerRas = 64;
				HorSizePerRas = 256;
				RasLowBitsForHor = 3;
			}
			else if(pDramCfg->casBit == 10 && pDramCfg->bankBit == 3 && pDramCfg->rasBit == 13)
			{
				VerSizePerRas = 64;
				HorSizePerRas = 512;
				RasLowBitsForHor = 2;
			}
			else
				return RETCODE_INVALID_PARAM;
		} 

		UNREFERENCED_PARAMETER(HorSizePerRas);
		
		chr_size_y = height/chr_hscale;
		ChrSizeYField = ((chr_size_y+1)>>1);
		ChrFieldRasSize = ((ChrSizeYField + (VerSizePerRas-1))/VerSizePerRas) << RasLowBitsForHor;
		ChrFrameRasSize = ChrFieldRasSize *2;
		LumFieldRasSize = ChrFrameRasSize;
		LumFrameRasSize = LumFieldRasSize *2;

		size_dpb_all         = (LumFrameRasSize + ChrFrameRasSize) << (pDramCfg->bankBit+pDramCfg->casBit+pDramCfg->busBit);
		pvbFrame->size     = size_dpb_all*num;
		if (alloc_by_user)
		{
			pvbFrame->phys_addr = frambufArray[0].bufY;
			pvbFrame->phys_addr = GetTiledFrameBase(coreIdx, &frambufArray[0], num);
		}
		else 
		{
			if (vdi_allocate_dma_memory(coreIdx, pvbFrame)<0)
				return RETCODE_INSUFFICIENT_RESOURCE;
		}

		if (fbType == FB_TYPE_PPU) {
			addrY = pvbFrame->phys_addr - tiledBaseAddr;
		}
		else {
			SetTiledFrameBase(coreIdx, pvbFrame->phys_addr);			
			addrY = 0;
			tiledBaseAddr = pvbFrame->phys_addr;
		}
	
		for (i=0; i<num; i++)
		{
			if (alloc_by_user) {
				addrY = frambufArray[i].bufY - tiledBaseAddr;				
			}	

			// align base_addr to RAS boundary
			addrYRas  = (addrY + ((1<<(pDramCfg->bankBit+pDramCfg->casBit+pDramCfg->busBit))-1)) >> (pDramCfg->bankBit+pDramCfg->casBit+pDramCfg->busBit);
			// round up RAS lower 3(or 4) bits
			addrYRas  = ((addrYRas + ((1<<(RasLowBitsForHor))-1)) >> RasLowBitsForHor) << RasLowBitsForHor;

			LumRasTop = addrYRas;
			LumRasBot = LumRasTop  + LumFieldRasSize;
			ChrRasTop = LumRasTop  + LumFrameRasSize;
			ChrRasBot = ChrRasTop  + ChrFieldRasSize;
			
			frambufArray[i].myIndex = i+gdiIndex;
			frambufArray[i].mapType = mapType;
			frambufArray[i].height = memHeight;
			frambufArray[i].bufY  = (LumRasBot << 16) + LumRasTop;
			frambufArray[i].bufCb = (ChrRasBot << 16) + ChrRasTop;
            frambufArray[i].stride         = width;
            frambufArray[i].cbcrInterleave = interleave;
			frambufArray[i].sourceLBurstEn = 0;
			if (RasLowBitsForHor == 4)
				frambufArray[i].bufCr  = ((((ChrRasBot>>4)<<4) + 8) << 16) + (((ChrRasTop>>4)<<4) + 8);
			else if (RasLowBitsForHor == 3)
				frambufArray[i].bufCr  = ((((ChrRasBot>>3)<<3) + 4) << 16) + (((ChrRasTop>>3)<<3) + 4);
			else if (RasLowBitsForHor == 2)
				frambufArray[i].bufCr  = ((((ChrRasBot>>2)<<2) + 2) << 16) + (((ChrRasTop>>2)<<2) + 2);
			else if (RasLowBitsForHor == 1)
				frambufArray[i].bufCr  = ((((ChrRasBot>>1)<<1) + 1) << 16) + (((ChrRasTop>>1)<<1) + 1);
			else
				return RETCODE_INSUFFICIENT_RESOURCE; // Invalid RasLowBit value

			addrYRas = (addrYRas + LumFrameRasSize + ChrFrameRasSize);		
			addrY    = ((addrYRas) << (pDramCfg->bankBit+pDramCfg->casBit+pDramCfg->busBit));
		}
	}



	return RETCODE_SUCCESS;
}

int mmp_vpu_dev::ConfigSecAXI(Uint32 coreIdx, CodStd codStd, SecAxiInfo *sa, int width, int height, int profile)
{
	vpu_buffer_t vb;
	unsigned int offset;
	int MbNumX = ((width & 0xFFFF) + 15) / 16;
	int MbNumY = ((height & 0xFFFF) + 15) / 16;
	//int MbNumY = ((height & 0xFFFF) + 31) / 32; //field??


	if (vdi_get_sram_memory(coreIdx, &vb) < 0)
		return 0;

	if (!vb.size)
	{
		sa->bufSize = 0;
		sa->useBitEnable = 0;
		sa->useIpEnable = 0;
		sa->useDbkYEnable = 0;
		sa->useDbkCEnable = 0;
		sa->useOvlEnable = 0;
		sa->useBtpEnable = 0;
		return 0;
	}
	
	offset = 0;
	//BIT
	if (sa->useBitEnable)
	{
		sa->useBitEnable = 1;
		sa->bufBitUse = vb.phys_addr + offset;

		switch (codStd) 
		{
		case STD_AVC:
			offset = offset + MbNumX * 144; 
			break; // AVC
		case STD_RV:
			offset = offset + MbNumX * 128;
			break;
		case STD_VC1:
			offset = offset + MbNumX *  64;
			break;
		case STD_AVS:
			offset = offset + ((MbNumX + 3) & ~3) *  32; 
			break;
		case STD_MPEG2:
			offset = offset + MbNumX * 0; 
			break;
		case STD_VP8:
			offset = offset + MbNumX * 128; 
			break;
		default:
			offset = offset + MbNumX *  16; 
			break; // MPEG-4, Divx3
		}

		if (offset > vb.size)
		{
			sa->bufSize = 0;
			return 0;
		}	

	}

	//Intra Prediction, ACDC
	if (sa->useIpEnable)
	{
		sa->bufIpAcDcUse = vb.phys_addr + offset;
		sa->useIpEnable = 1;

		switch (codStd) 
		{
		case STD_AVC:
			offset = offset + MbNumX * 64; 
			break; // AVC
		case STD_RV:
			offset = offset + MbNumX * 64;
			break;
		case STD_VC1:
			offset = offset + MbNumX * 128;
			break;
		case STD_AVS:
			offset = offset + MbNumX * 64;
			break;
		case STD_MPEG2:
			offset = offset + MbNumX * 0; 
			break;
		case STD_VP8:
			offset = offset + MbNumX * 64; 
			break;
		default:
			offset = offset + MbNumX * 128; 
			break; // MPEG-4, Divx3
		}

		if (offset > vb.size)
		{
			sa->bufSize = 0;
			return 0;
		}
	}


	//Deblock Chroma
	if (sa->useDbkCEnable)
	{
		sa->bufDbkCUse = vb.phys_addr + offset;
		sa->useDbkCEnable = 1;
		switch (codStd) 
		{
		case STD_AVC:
			offset = (profile==66/*AVC BP decoder*/ || profile==0/* AVC encoder */)? offset + (MbNumX * 64) : offset + (MbNumX * 128);			
			break; // AVC
		case STD_RV:
			offset = offset + MbNumX * 128;
			break;
		case STD_VC1:
			offset = profile==2 ? offset + MbNumX * 256 : offset + MbNumX * 128;
			// sram size for Deblock Chroma should be aligned 256
			offset = (offset + 255) & (~255);
			break;
		case STD_AVS:
			offset = offset + MbNumX * 128;
			break;
		case STD_MPEG2:
			offset = offset + MbNumX * 64; 
			break;
		case STD_VP8:
			offset = offset + MbNumX * 128; 
			break;
		default:
			offset = offset + MbNumX * 64; 
			break;
		}

		if (offset > vb.size)
		{
			sa->bufSize = 0;
			return 0;
		}		
	}

	//Deblock Luma
	if (sa->useDbkYEnable)
	{
		sa->bufDbkYUse = vb.phys_addr + offset;
		sa->useDbkYEnable = 1;

		switch (codStd) 
		{
		case STD_AVC:
			offset = (profile==66/*AVC BP decoder*/ || profile==0/* AVC encoder */) ? offset+ (MbNumX * 64) : offset + (MbNumX * 128);
			break; // AVC
		case STD_RV:
			offset = offset + MbNumX * 128;
			break;
		case STD_VC1:
			offset = profile==2 ? offset + MbNumX * 256 : offset + MbNumX * 128;
//#ifdef CODA7L 
			// sram size for Deblock Luma should be aligned 256
			offset = (offset + 255) & (~255);
//#endif
			break;
		case STD_AVS:
			offset = offset + MbNumX * 128;
			break;
		case STD_MPEG2:
			offset = offset + MbNumX * 128; 
			break;
		case STD_VP8:
			offset = offset + MbNumX * 128; 
			break;
		default:
			offset = offset + MbNumX * 128; 
			break;
		}
		if (offset > vb.size)
		{
			sa->bufSize = 0;
			return 0;
		}		
	}

	// check the buffer address which is 256 byte is available.
	if (((offset + 255) & (~255)) > vb.size)
	{
		sa->bufSize = 0;
		return 0;
	}	


	//VC1 Bit-plane
	if (sa->useBtpEnable)
	{
		if (codStd != STD_VC1)
		{
			sa->useBtpEnable = 0;			
		}
		else
		{
			int oneBTP;
			offset = (offset + 255) & (~255);
			sa->bufBtpUse = vb.phys_addr + offset;
			sa->useBtpEnable = 1;

			oneBTP  = (((MbNumX+15)/16) * MbNumY + 1) * 2;
			oneBTP  = (oneBTP%256) ? ((oneBTP/256)+1)*256 : oneBTP;

			offset = offset + oneBTP * 3;

			if (offset > vb.size)
			{
				sa->bufSize = 0;
				return 0;
			}	
		}
	}
	//VC1 Overlap
	if (sa->useOvlEnable)
	{
		if (codStd != STD_VC1)
		{
			sa->useOvlEnable = 0;			
		}
		else
		{
			sa->bufOvlUse = vb.phys_addr + offset;
			sa->useOvlEnable = 1;

			offset = offset + MbNumX *  80;

			if (offset > vb.size)
			{
				sa->bufSize = 0;
				return 0;
			}		
		}		
	}


	sa->bufSize = offset;

	return 1;
}

#endif