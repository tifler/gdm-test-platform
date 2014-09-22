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

#ifndef MMP_JPU_DEV_HPP__
#define MMP_JPU_DEV_HPP__

#include "mmp_jpu_def.h"
#include "mmp_env_mgr.hpp"

class mmp_singleton_mgr;

struct mmp_jpu_dev_create_config {
	MMP_S32 dummy;
};

class mmp_jpu_dev {

friend class mmp_singleton_mgr;

private:
    static class mmp_jpu_dev* s_p_instance;

private:
    static MMP_RESULT create_instance();
	static MMP_RESULT create_instance(struct mmp_jpu_dev_create_config* p_create_config);
	static MMP_RESULT destroy_instance();

public:
    static class mmp_jpu_dev* get_instance();

protected:

    struct mmp_jpu_dev_create_config m_create_config;

protected:
    mmp_jpu_dev(struct mmp_jpu_dev_create_config* p_create_config);
    virtual ~mmp_jpu_dev();

    virtual MMP_RESULT open() = 0;
    virtual MMP_RESULT close() = 0;

protected:
    inline void export_jpu_fd(MMP_S32 fd) { mmp_env_mgr::get_instance()->set_uint(mmp_env_mgr::ENV_UINT_JPU_FD,fd); }
    inline void export_jpu_instance_pool_buffer(MMP_U8* ptr) { mmp_env_mgr::get_instance()->set_uint(mmp_env_mgr::ENV_UINT_JPU_INSTANCE_POOL_BUFFER, (MMP_U32)ptr); }
    inline void export_jpu_reg_vir_addr(MMP_U32 addr) { mmp_env_mgr::get_instance()->set_uint(mmp_env_mgr::ENV_UINT_JPU_REG_VIR_ADDR, (MMP_U32)addr); }
    inline void export_jpu_common_buffer(void* ptr) { mmp_env_mgr::get_instance()->set_uint(mmp_env_mgr::ENV_UINT_JPU_COMMON_BUFFER, (MMP_U32)ptr); }

public:
    //MMP_S32 get_phy_addr_from_shared_fd(MMP_S32 shared_fd);
    //MMP_U32 get_vir_addr_from_phy_addr(MMP_U32 phy_addr, MMP_S32 size);
    //MMP_S32 get_phy_addr_for_common_buffer();
    
public:
    virtual int			JPU_IsBusy() = 0;
	virtual Uint32		JPU_GetStatus() = 0;
	virtual void		JPU_ClrStatus(Uint32 val) = 0;
	virtual Uint32		JPU_IsInit(void) = 0;
	virtual Uint32		JPU_WaitInterrupt(int timeout) = 0;
	
	virtual JpgRet		JPU_Init() = 0;
	virtual void		JPU_DeInit() = 0;
	virtual int			JPU_GetOpenInstanceNum() = 0;
	virtual JpgRet     JPU_GetVersionInfo(Uint32 *versionInfo) = 0;

	// function for decode
	virtual JpgRet JPU_DecOpen(JpgDecHandle *, JpgDecOpenParam *) = 0;
	virtual JpgRet JPU_DecClose(JpgDecHandle) = 0;
	virtual JpgRet JPU_DecGetInitialInfo(JpgDecHandle handle,	JpgDecInitialInfo * info) = 0;
	virtual JpgRet JPU_DecSetRdPtr(JpgDecHandle handle, PhysicalAddress addr, int updateWrPtr) = 0;
	virtual JpgRet JPU_DecRegisterFrameBuffer(JpgDecHandle handle,	JPU_FrameBuffer * bufArray,	int num,	int stride) = 0;
	virtual JpgRet JPU_DecGetBitstreamBuffer(JpgDecHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr,	int * size ) = 0;
	virtual JpgRet JPU_DecUpdateBitstreamBuffer(JpgDecHandle handle, int size) = 0; 
    virtual JpgRet JPU_HWReset() = 0;
	virtual JpgRet JPU_SWReset() = 0;
	virtual JpgRet JPU_DecStartOneFrame(JpgDecHandle handle, JpgDecParam *param ) = 0; 
	virtual JpgRet JPU_DecGetOutputInfo(JpgDecHandle handle, JpgDecOutputInfo * info) = 0;
	virtual JpgRet JPU_DecIssueStop(JpgDecHandle handle) = 0;
	virtual JpgRet JPU_DecCompleteStop(JpgDecHandle handle) = 0;
	virtual JpgRet JPU_DecGiveCommand(JpgDecHandle handle, JpgCommand cmd,	void * parameter) = 0;	
	
    // function for encode
	virtual JpgRet JPU_EncOpen(JpgEncHandle *, JpgEncOpenParam *) = 0;
	virtual JpgRet JPU_EncClose(JpgEncHandle) = 0;
	virtual JpgRet JPU_EncGetInitialInfo(JpgEncHandle, JpgEncInitialInfo *) = 0;
    virtual JpgRet JPU_EncGetBitstreamBuffer(JpgEncHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr, int * size) = 0;
	virtual JpgRet JPU_EncUpdateBitstreamBuffer(JpgEncHandle handle, int size) = 0;
	virtual JpgRet JPU_EncStartOneFrame(JpgEncHandle handle, JpgEncParam * param, int TmbEn) = 0;
	virtual JpgRet JPU_EncGetOutputInfo(JpgEncHandle handle, JpgEncOutputInfo * info) = 0;
	virtual JpgRet JPU_EncIssueStop(JpgDecHandle handle) = 0;
	virtual JpgRet JPU_EncCompleteStop(JpgDecHandle handle) = 0;
	virtual JpgRet JPU_EncGiveCommand(JpgEncHandle handle, JpgCommand cmd, void * parameter) = 0;
	virtual void JPU_EncSetHostParaAddr(PhysicalAddress baseAddr, PhysicalAddress paraAddr) = 0;

};

#endif
