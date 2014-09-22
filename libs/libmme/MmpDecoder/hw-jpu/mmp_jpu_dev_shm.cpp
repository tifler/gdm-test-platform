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

#include "mmp_jpu_dev_shm.hpp"
#include "MmpUtil.hpp"

#include "mmp_buffer_mgr.hpp"
#include "mmp_lock.hpp"
#include "../coda960.h"
#include "mmp_env_mgr.hpp"

static jpu_buffer_t s_jpu_common_buffer;


/**********************************************************
class members
**********************************************************/

mmp_jpu_dev_shm::mmp_jpu_dev_shm(struct mmp_jpu_dev_create_config* p_create_config) : mmp_jpu_dev(p_create_config)
,m_coreIdx(0)
,m_p_mutex(NULL)
,m_p_mutex_external_cs(NULL)
,m_p_shm_jdi(NULL)
,m_p_shm_jdi_obj(NULL)

,m_jpu_fd(-1)

{
    this->m_vdb_register.virt_addr = (unsigned long)MAP_FAILED;
}

mmp_jpu_dev_shm::~mmp_jpu_dev_shm() {

}


MMP_RESULT mmp_jpu_dev_shm::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    JpgRet jpu_ret;
    
    /* create mutex */
#if 0
    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex = mmp_oal_mutex::create_object(0x10);
        if(m_p_mutex == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }
#endif

    /* create external mutex */
    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex_external_cs = mmp_oal_mutex::create_object( mmp_env_mgr::get_instance()->get_uint(mmp_env_mgr::ENV_UINT_JPU_EXTERNAL_MUTEX_KEY));
        if(m_p_mutex_external_cs == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }
    
    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex_external_cs->lock();
        
        mmpResult = this->open_jdi_memory();
        if(mmpResult == MMP_SUCCESS) {
            
            if(m_p_shm_jdi_obj->app_count == 0) {
                jpu_ret = ::JPU_Init();
            }
            else {
                //jpu_ret = ::VPU_Init_Shm(m_coreIdx);
            }

            if( (jpu_ret == JPG_RET_SUCCESS) ) {
                /*Nothing to do */
                
            }
            else {
                mmpResult = MMP_FAILURE;
            }
        
            m_p_shm_jdi_obj->app_count++;
        }

        m_p_mutex_external_cs->unlock();
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[mmp_jpu_dev_shm::open] mmpResult=%d app_count=%d "), mmpResult, m_p_shm_jdi_obj->app_count));
    
    return mmpResult;
    
}

MMP_RESULT mmp_jpu_dev_shm::close() {

    MMP_S32 app_count = -1;

    if(m_p_mutex_external_cs != NULL) {
        m_p_mutex_external_cs->lock();

        m_p_shm_jdi_obj->app_count--;

        app_count = m_p_shm_jdi_obj->app_count;

        if(m_p_shm_jdi_obj->app_count == 0) {
            ::JPU_DeInit();
        }

        this->close_jdi_memory();
        m_p_mutex_external_cs->unlock();
    }

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
        m_p_mutex = NULL;
    }

    if(m_p_mutex_external_cs != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex_external_cs);
        m_p_mutex_external_cs = NULL;
    }

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[mmp_jpu_dev_shm::close] app_count=%d "), app_count));

	return MMP_SUCCESS;
}


MMP_RESULT mmp_jpu_dev_shm::open_jdi_memory() {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U8* p_instance_pool_buffer;
    
    /* open shm */
    if(mmpResult == MMP_SUCCESS) {
        m_p_shm_jdi = mmp_oal_shm::create_object(mmp_env_mgr::get_instance()->get_uint(mmp_env_mgr::ENV_UINT_JPU_SHM_KEY), sizeof(struct jdi_shm));
        if(m_p_shm_jdi == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_jpu_dev_shm::open_jdi_memory] FAIL: mmp_oal_shm::create_object")));
            mmpResult = MMP_FAILURE;
        }
        else {

            m_p_shm_jdi_obj = (struct jdi_shm*)m_p_shm_jdi->get_shm_ptr();

            if(m_p_shm_jdi->is_create() == MMP_TRUE) {
                MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[mmp_jpu_dev_shm::open_jdi_memory] shared memory is created.")));
                memset(m_p_shm_jdi_obj, 0x00, sizeof(struct jdi_shm));
            }
            else {
                MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[mmp_jpu_dev_shm::open_jdi_memory] attach shered memory")));
            }
        }
    }

    /* open driver */
    if(mmpResult == MMP_SUCCESS) {

        m_jpu_fd = MMP_DRIVER_OPEN(mmp_env_mgr::get_instance()->get_char(mmp_env_mgr::ENV_CHAR_JPU_DRV_NAME), O_RDWR);
        if(m_jpu_fd < 0) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[mmp_jpu_dev_shm::open_jdi_memory] FAIL: open jpu driver")));
        }
        else {
            this->export_jpu_fd(this->m_jpu_fd);
        }
    }


    /* alloc instance pool */
    if(mmpResult == MMP_SUCCESS) {
        p_instance_pool_buffer = (MMP_U8*)m_p_shm_jdi_obj->jdi_inst;
        if(p_instance_pool_buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            this->export_jpu_instance_pool_buffer(p_instance_pool_buffer);
        }
    }

    /* get register */
    if(mmpResult == MMP_SUCCESS) {
        
        this->m_vdb_register.size = mmp_env_mgr::get_instance()->get_uint(mmp_env_mgr::ENV_UINT_JPU_REG_SIZE);
	    this->m_vdb_register.virt_addr = (unsigned long)MMP_DRIVER_MMAP(NULL, this->m_vdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, this->m_jpu_fd, 0);
        this->m_vdb_register.phys_addr = mmp_env_mgr::get_instance()->get_uint(mmp_env_mgr::ENV_UINT_JPU_REG_PHY_ADDR);
	    if ( (void*)this->m_vdb_register.virt_addr == MAP_FAILED)  {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_jpu_dev_ex1::open_jdi_memory] FAIL: map jpu registers")));
		    mmpResult = MMP_FAILURE;
	    }
        else {
            m_jpu_reg_buf.m_phy_addr = this->m_vdb_register.phys_addr;
            m_jpu_reg_buf.m_vir_addr = this->m_vdb_register.virt_addr;

            this->export_jpu_reg_vir_addr(this->m_vdb_register.virt_addr);
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[mmp_jpu_dev_shm::open_jdi_memory] JPU_REG_ADDR ( phy=0x%08x vir=0x%08x )"), m_jpu_reg_buf.m_phy_addr, m_jpu_reg_buf.m_vir_addr ));
        }
        
    }


#if 0
    /* create common buffer */
    if(mmpResult == MMP_SUCCESS) {

        //if(m_p_shm_jdi_obj->app_count ==  0) {
        //    m_p_jpu_common_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(SIZE_COMMON);
        //}
        //else {
        //    m_p_jpu_common_buffer = mmp_buffer_mgr::get_instance()->attach_dma_buffer(m_p_shm_jdi_obj->m_buf_addr_vdi_commmon);
        //}

        m_p_jpu_common_buffer = NULL;
        m_p_shm_jdi_obj->m_buf_addr_vdi_commmon.m_phy_addr = 0;//this->get_phy_addr_for_common_buffer();
        m_p_shm_jdi_obj->m_buf_addr_vdi_commmon.m_shared_fd = -1;
        m_p_shm_jdi_obj->m_buf_addr_vdi_commmon.m_vir_addr = 0;  
        m_p_shm_jdi_obj->m_buf_addr_vdi_commmon.m_size = SIZE_COMMON;

        if(m_p_shm_jdi_obj->m_buf_addr_vdi_commmon.m_phy_addr != NULL) {
            m_p_jpu_common_buffer = mmp_buffer_mgr::get_instance()->attach_dma_buffer(m_p_shm_jdi_obj->m_buf_addr_vdi_commmon);
        }

        if(m_p_jpu_common_buffer == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_jpu_dev_ex1::open_jdi_memory] FAIL: alloc comm buffer (app_count=%d) "), m_p_shm_jdi_obj->app_count));
            mmpResult = MMP_FAILURE;
        }
        else {
            m_code_buf = m_p_jpu_common_buffer->get_buf_addr();
            m_parm_buf = m_code_buf + CODE_BUF_SIZE;
            m_temp_buf = m_parm_buf + TEMP_BUF_SIZE;

            s_jpu_common_buffer.phys_addr = m_code_buf.m_phy_addr;
            s_jpu_common_buffer.virt_addr = m_code_buf.m_vir_addr;
            s_jpu_common_buffer.size = m_code_buf.m_size;
            s_jpu_common_buffer.base = s_jpu_common_buffer.virt_addr;

            this->export_jpu_common_buffer((void*)&s_jpu_common_buffer);

            if(m_p_shm_jdi_obj->app_count ==  0) {
                m_p_shm_jdi_obj->m_buf_addr_vdi_commmon = m_p_jpu_common_buffer->get_buf_addr();

                m_p_shm_jdi_obj->m_buf_addr_vdi_commmon.m_shared_fd = -1; /* ION FD can transfer other process via only BIND, SOCKET */
                m_p_shm_jdi_obj->m_buf_addr_vdi_commmon.m_vir_addr = 0;  
            }

            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_jpu_dev_ex1::open_jdi_memory] common buf (phy=0x%08x vir=0x%08x sz=%d shard_fd=%d ) "), 
                                 s_jpu_common_buffer.phys_addr,
                                 s_jpu_common_buffer.virt_addr,
                                 s_jpu_common_buffer.size,
                                  m_code_buf.m_shared_fd  
                                 ));
        }
        
    }
#endif

    return mmpResult;
}

MMP_RESULT mmp_jpu_dev_shm::close_jdi_memory() {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    if( (void*)this->m_vdb_register.virt_addr != MAP_FAILED) {
        MMP_DRIVER_MUNMAP((void*)this->m_vdb_register.virt_addr, this->m_vdb_register.size);
        this->m_vdb_register.virt_addr = (unsigned long)MAP_FAILED;
    }

    if(m_p_jpu_common_buffer != NULL) {
        mmp_buffer_mgr::get_instance()->free_buffer(m_p_jpu_common_buffer);
        m_p_jpu_common_buffer = NULL;
    }
    
   /* close shm */
    if(m_p_shm_jdi != NULL) {
        mmp_oal_shm::destroy_object(m_p_shm_jdi, MMP_FALSE);
        m_p_shm_jdi = NULL;
    }

    if(m_jpu_fd >= 0) {
       MMP_DRIVER_CLOSE(m_jpu_fd);
       m_jpu_fd = -1;
    }

    return mmpResult;
}


int mmp_jpu_dev_shm::JPU_IsBusy() {
    return ::JPU_IsBusy();
}

Uint32 mmp_jpu_dev_shm::JPU_GetStatus() {
    return ::JPU_GetStatus();
}

void mmp_jpu_dev_shm::JPU_ClrStatus(Uint32 val) {
    ::JPU_ClrStatus(val);
}

Uint32 mmp_jpu_dev_shm::JPU_IsInit(void) {
    return ::JPU_IsInit();
}

Uint32 mmp_jpu_dev_shm::JPU_WaitInterrupt(int timeout) {
    return ::JPU_WaitInterrupt(timeout);
}

JpgRet mmp_jpu_dev_shm::JPU_Init() {
    return JPG_RET_SUCCESS;//::JPU_Init();
}

void mmp_jpu_dev_shm::JPU_DeInit() {
    //::JPU_DeInit();
}

int mmp_jpu_dev_shm::JPU_GetOpenInstanceNum() {
    return ::JPU_GetOpenInstanceNum();
}

JpgRet mmp_jpu_dev_shm::JPU_GetVersionInfo(Uint32 *versionInfo) {
    return ::JPU_GetVersionInfo(versionInfo);
}

// function for decode
JpgRet mmp_jpu_dev_shm::JPU_DecOpen(JpgDecHandle *hdl, JpgDecOpenParam *op) {
    return ::JPU_DecOpen(hdl, op);
}

JpgRet mmp_jpu_dev_shm::JPU_DecClose(JpgDecHandle hdl) {
    return ::JPU_DecClose(hdl);
}

JpgRet mmp_jpu_dev_shm::JPU_DecGetInitialInfo(JpgDecHandle handle,	JpgDecInitialInfo * info) {
    return ::JPU_DecGetInitialInfo(handle,	info);
}

JpgRet mmp_jpu_dev_shm::JPU_DecSetRdPtr(JpgDecHandle handle, PhysicalAddress addr, int updateWrPtr) {
    return ::JPU_DecSetRdPtr(handle, addr, updateWrPtr);
}

JpgRet mmp_jpu_dev_shm::JPU_DecRegisterFrameBuffer(JpgDecHandle handle,	JPU_FrameBuffer * bufArray,	int num,	int stride) {
    return ::JPU_DecRegisterFrameBuffer(handle,	bufArray, num,	stride);
}

JpgRet mmp_jpu_dev_shm::JPU_DecGetBitstreamBuffer(JpgDecHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr,	int * size ) {
    return ::JPU_DecGetBitstreamBuffer(handle, prdPrt,	pwrPtr,	size);
}

JpgRet mmp_jpu_dev_shm::JPU_DecUpdateBitstreamBuffer(JpgDecHandle handle, int size) {
    return ::JPU_DecUpdateBitstreamBuffer(handle, size);
}

JpgRet mmp_jpu_dev_shm::JPU_HWReset() {
    return ::JPU_HWReset();
}

JpgRet mmp_jpu_dev_shm::JPU_SWReset() {
    return ::JPU_SWReset();
}

JpgRet mmp_jpu_dev_shm::JPU_DecStartOneFrame(JpgDecHandle handle, JpgDecParam *param ) {
    return ::JPU_DecStartOneFrame(handle, param );
}

JpgRet mmp_jpu_dev_shm::JPU_DecGetOutputInfo(JpgDecHandle handle, JpgDecOutputInfo * info) {
    return ::JPU_DecGetOutputInfo(handle, info);
}

JpgRet mmp_jpu_dev_shm::JPU_DecIssueStop(JpgDecHandle handle) {
    return ::JPU_DecIssueStop(handle);
}

JpgRet mmp_jpu_dev_shm::JPU_DecCompleteStop(JpgDecHandle handle) {
    return ::JPU_DecCompleteStop(handle);
}

JpgRet mmp_jpu_dev_shm::JPU_DecGiveCommand(JpgDecHandle handle, JpgCommand cmd,	void * parameter) {
    return ::JPU_DecGiveCommand(handle, cmd, parameter);
}

// function for encode
JpgRet mmp_jpu_dev_shm::JPU_EncOpen(JpgEncHandle *hdl, JpgEncOpenParam *op) {
    return ::JPU_EncOpen(hdl, op);
}

JpgRet mmp_jpu_dev_shm::JPU_EncClose(JpgEncHandle hdl) {
    return ::JPU_EncClose(hdl);
}

JpgRet mmp_jpu_dev_shm::JPU_EncGetInitialInfo(JpgEncHandle hdl, JpgEncInitialInfo *info) {
    return ::JPU_EncGetInitialInfo(hdl, info);
}

JpgRet mmp_jpu_dev_shm::JPU_EncGetBitstreamBuffer(JpgEncHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr, int * size) {
    return ::JPU_EncGetBitstreamBuffer(handle,  prdPrt,	pwrPtr, size);
}

JpgRet mmp_jpu_dev_shm::JPU_EncUpdateBitstreamBuffer(JpgEncHandle handle, int size) {
    return ::JPU_EncUpdateBitstreamBuffer(handle, size);
}

JpgRet mmp_jpu_dev_shm::JPU_EncStartOneFrame(JpgEncHandle handle, JpgEncParam * param, int TmbEn) {
    return ::JPU_EncStartOneFrame(handle, param, TmbEn);
}

JpgRet mmp_jpu_dev_shm::JPU_EncGetOutputInfo(JpgEncHandle handle, JpgEncOutputInfo * info) {
    return ::JPU_EncGetOutputInfo(handle, info);
}

JpgRet mmp_jpu_dev_shm::JPU_EncIssueStop(JpgDecHandle handle) {
    return ::JPU_EncIssueStop(handle);
}

JpgRet mmp_jpu_dev_shm::JPU_EncCompleteStop(JpgDecHandle handle) {
    return ::JPU_EncCompleteStop(handle);
}

JpgRet mmp_jpu_dev_shm::JPU_EncGiveCommand(JpgEncHandle handle, JpgCommand cmd, void * parameter) {
    return ::JPU_EncGiveCommand(handle, cmd, parameter);
}

void mmp_jpu_dev_shm::JPU_EncSetHostParaAddr(PhysicalAddress baseAddr, PhysicalAddress paraAddr) {
    //::JPU_EncSetHostParaAddr(baseAddr, paraAddr);
}

