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

#ifndef MMP_VPU_DEV_SHM_HPP__
#define MMP_VPU_DEV_SHM_HPP__

#include "mmp_jpu_dev.hpp"
#include "mmp_buffer.hpp"
#include "mmp_msg_proc.hpp"
#include "mmp_oal_mutex.hpp"
#include "mmp_oal_shm.hpp"

#define JDI_SHM_KEY 0xAAAA9829
#define JDI_INSTANCE_POOL_SIZE (JPU_MAX_INST_HANDLE_SIZE*(JPU_MAX_NUM_INSTANCE+1) + 1024*12)
//#define VDI_COMMON_MEMORY_INFO_SIZE 1024

struct jdi_shm {
    MMP_S32 app_count;
    MMP_U8 jdi_inst[JDI_INSTANCE_POOL_SIZE];
    class mmp_buffer_addr m_buf_addr_vdi_commmon;
};

class mmp_jpu_dev_shm : public mmp_jpu_dev {

friend class mmp_jpu_dev;

private:
    Uint32 m_coreIdx;
    class mmp_oal_mutex* m_p_mutex;
    class mmp_oal_mutex* m_p_mutex_external_cs;
	class mmp_oal_shm* m_p_shm_jdi;
    struct jdi_shm* m_p_shm_jdi_obj;
    
protected:
    
	mmp_jpu_dev_shm(struct mmp_jpu_dev_create_config* p_create_config);
	virtual ~mmp_jpu_dev_shm();

	virtual MMP_RESULT open();
	virtual MMP_RESULT close();
    
private:
    int m_jpu_fd;

    jpu_buffer_t m_vdb_register;
    class mmp_buffer* m_p_jpu_common_buffer;

    class mmp_buffer_addr m_code_buf;
    class mmp_buffer_addr m_parm_buf;
    class mmp_buffer_addr m_temp_buf;
    class mmp_buffer_addr m_jpu_reg_buf;

private:
    MMP_RESULT open_jdi_memory();
    MMP_RESULT close_jdi_memory();

public:

    virtual int			JPU_IsBusy();
	virtual Uint32		JPU_GetStatus();
	virtual void		JPU_ClrStatus(Uint32 val);
	virtual Uint32		JPU_IsInit(void);
	virtual Uint32		JPU_WaitInterrupt(int timeout);
	
	virtual JpgRet		JPU_Init();
	virtual void		JPU_DeInit();
	virtual int			JPU_GetOpenInstanceNum();
	virtual JpgRet     JPU_GetVersionInfo(Uint32 *versionInfo);

	// function for decode
	virtual JpgRet JPU_DecOpen(JpgDecHandle *, JpgDecOpenParam *);
	virtual JpgRet JPU_DecClose(JpgDecHandle);
	virtual JpgRet JPU_DecGetInitialInfo(JpgDecHandle handle,	JpgDecInitialInfo * info);
	virtual JpgRet JPU_DecSetRdPtr(JpgDecHandle handle, PhysicalAddress addr, int updateWrPtr);
	virtual JpgRet JPU_DecRegisterFrameBuffer(JpgDecHandle handle,	JPU_FrameBuffer * bufArray,	int num,	int stride);
	virtual JpgRet JPU_DecGetBitstreamBuffer(JpgDecHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr,	int * size );
	virtual JpgRet JPU_DecUpdateBitstreamBuffer(JpgDecHandle handle, int size); 
    virtual JpgRet JPU_HWReset();
	virtual JpgRet JPU_SWReset();
	virtual JpgRet JPU_DecStartOneFrame(JpgDecHandle handle, JpgDecParam *param ); 
	virtual JpgRet JPU_DecGetOutputInfo(JpgDecHandle handle, JpgDecOutputInfo * info);
	virtual JpgRet JPU_DecIssueStop(JpgDecHandle handle);
	virtual JpgRet JPU_DecCompleteStop(JpgDecHandle handle);
	virtual JpgRet JPU_DecGiveCommand(JpgDecHandle handle, JpgCommand cmd,	void * parameter);	
	
    // function for encode
	virtual JpgRet JPU_EncOpen(JpgEncHandle *, JpgEncOpenParam *);
	virtual JpgRet JPU_EncClose(JpgEncHandle);
	virtual JpgRet JPU_EncGetInitialInfo(JpgEncHandle, JpgEncInitialInfo *);
    virtual JpgRet JPU_EncGetBitstreamBuffer(JpgEncHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr, int * size);
	virtual JpgRet JPU_EncUpdateBitstreamBuffer(JpgEncHandle handle, int size);
	virtual JpgRet JPU_EncStartOneFrame(JpgEncHandle handle, JpgEncParam * param, int TmbEn);
	virtual JpgRet JPU_EncGetOutputInfo(JpgEncHandle handle, JpgEncOutputInfo * info);
	virtual JpgRet JPU_EncIssueStop(JpgDecHandle handle);
	virtual JpgRet JPU_EncCompleteStop(JpgDecHandle handle);
	virtual JpgRet JPU_EncGiveCommand(JpgEncHandle handle, JpgCommand cmd, void * parameter);
	virtual void JPU_EncSetHostParaAddr(PhysicalAddress baseAddr, PhysicalAddress paraAddr);
};


#endif
