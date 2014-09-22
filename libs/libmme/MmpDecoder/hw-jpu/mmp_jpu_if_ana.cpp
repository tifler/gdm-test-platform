/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License") 
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

#include "mmp_singleton_mgr.hpp"
#include "mmp_jpu_if_ana.hpp"
#include "mmp_jpu_dev.hpp"
#include "MmpUtil.hpp"



/**********************************************************
class members
**********************************************************/


mmp_jpu_if_ana::mmp_jpu_if_ana(struct mmp_jpu_if_create_config* p_create_config) : mmp_jpu_if(p_create_config)
{

    
}

mmp_jpu_if_ana::~mmp_jpu_if_ana() {

}


MMP_RESULT mmp_jpu_if_ana::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult = mmp_singleton_mgr::get_result(mmp_singleton_mgr::ID_JPU_DEV);
        

	return mmpResult;
}

MMP_RESULT mmp_jpu_if_ana::close() {
	
	return MMP_SUCCESS;
}

int mmp_jpu_if_ana::JPU_IsBusy() {
    return mmp_jpu_dev::get_instance()->JPU_IsBusy();
}

Uint32 mmp_jpu_if_ana::JPU_GetStatus() {
    return mmp_jpu_dev::get_instance()->JPU_GetStatus();
}

void mmp_jpu_if_ana::JPU_ClrStatus(Uint32 val) {
    mmp_jpu_dev::get_instance()->JPU_ClrStatus(val);
}

Uint32 mmp_jpu_if_ana::JPU_IsInit(void) {
    return mmp_jpu_dev::get_instance()->JPU_IsInit();
}

Uint32 mmp_jpu_if_ana::JPU_WaitInterrupt(int timeout) {
    return mmp_jpu_dev::get_instance()->JPU_WaitInterrupt(timeout);
}

JpgRet mmp_jpu_if_ana::JPU_Init() {
    return JPG_RET_SUCCESS;//::JPU_Init();
}

void mmp_jpu_if_ana::JPU_DeInit() {
    //::JPU_DeInit();
}

int mmp_jpu_if_ana::JPU_GetOpenInstanceNum() {
    return mmp_jpu_dev::get_instance()->JPU_GetOpenInstanceNum();
}

JpgRet mmp_jpu_if_ana::JPU_GetVersionInfo(Uint32 *versionInfo) {
    return mmp_jpu_dev::get_instance()->JPU_GetVersionInfo(versionInfo);
}

// function for decode
JpgRet mmp_jpu_if_ana::JPU_DecOpen(JpgDecHandle *hdl, JpgDecOpenParam *op) {
    return mmp_jpu_dev::get_instance()->JPU_DecOpen(hdl, op);
}

JpgRet mmp_jpu_if_ana::JPU_DecClose(JpgDecHandle hdl) {
    return mmp_jpu_dev::get_instance()->JPU_DecClose(hdl);
}

JpgRet mmp_jpu_if_ana::JPU_DecGetInitialInfo(JpgDecHandle handle,	JpgDecInitialInfo * info) {
    return mmp_jpu_dev::get_instance()->JPU_DecGetInitialInfo(handle,	info);
}

JpgRet mmp_jpu_if_ana::JPU_DecSetRdPtr(JpgDecHandle handle, PhysicalAddress addr, int updateWrPtr) {
    return mmp_jpu_dev::get_instance()->JPU_DecSetRdPtr(handle, addr, updateWrPtr);
}

JpgRet mmp_jpu_if_ana::JPU_DecRegisterFrameBuffer(JpgDecHandle handle,	JPU_FrameBuffer * bufArray,	int num,	int stride) {
    return mmp_jpu_dev::get_instance()->JPU_DecRegisterFrameBuffer(handle,	bufArray, num,	stride);
}

JpgRet mmp_jpu_if_ana::JPU_DecGetBitstreamBuffer(JpgDecHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr,	int * size ) {
    return mmp_jpu_dev::get_instance()->JPU_DecGetBitstreamBuffer(handle, prdPrt,	pwrPtr,	size);
}

JpgRet mmp_jpu_if_ana::JPU_DecUpdateBitstreamBuffer(JpgDecHandle handle, int size) {
    return mmp_jpu_dev::get_instance()->JPU_DecUpdateBitstreamBuffer(handle, size);
}

JpgRet mmp_jpu_if_ana::JPU_HWReset() {
    return mmp_jpu_dev::get_instance()->JPU_HWReset();
}

JpgRet mmp_jpu_if_ana::JPU_SWReset() {
    return mmp_jpu_dev::get_instance()->JPU_SWReset();
}

JpgRet mmp_jpu_if_ana::JPU_DecStartOneFrame(JpgDecHandle handle, JpgDecParam *param ) {
    return mmp_jpu_dev::get_instance()->JPU_DecStartOneFrame(handle, param );
}

JpgRet mmp_jpu_if_ana::JPU_DecGetOutputInfo(JpgDecHandle handle, JpgDecOutputInfo * info) {
    return mmp_jpu_dev::get_instance()->JPU_DecGetOutputInfo(handle, info);
}

JpgRet mmp_jpu_if_ana::JPU_DecIssueStop(JpgDecHandle handle) {
    return mmp_jpu_dev::get_instance()->JPU_DecIssueStop(handle);
}

JpgRet mmp_jpu_if_ana::JPU_DecCompleteStop(JpgDecHandle handle) {
    return mmp_jpu_dev::get_instance()->JPU_DecCompleteStop(handle);
}

JpgRet mmp_jpu_if_ana::JPU_DecGiveCommand(JpgDecHandle handle, JpgCommand cmd,	void * parameter) {
    return mmp_jpu_dev::get_instance()->JPU_DecGiveCommand(handle, cmd, parameter);
}

// function for encode
JpgRet mmp_jpu_if_ana::JPU_EncOpen(JpgEncHandle *hdl, JpgEncOpenParam *op) {
    return mmp_jpu_dev::get_instance()->JPU_EncOpen(hdl, op);
}

JpgRet mmp_jpu_if_ana::JPU_EncClose(JpgEncHandle hdl) {
    return mmp_jpu_dev::get_instance()->JPU_EncClose(hdl);
}

JpgRet mmp_jpu_if_ana::JPU_EncGetInitialInfo(JpgEncHandle hdl, JpgEncInitialInfo *info) {
    return mmp_jpu_dev::get_instance()->JPU_EncGetInitialInfo(hdl, info);
}

JpgRet mmp_jpu_if_ana::JPU_EncGetBitstreamBuffer(JpgEncHandle handle, PhysicalAddress * prdPrt,	PhysicalAddress * pwrPtr, int * size) {
    return mmp_jpu_dev::get_instance()->JPU_EncGetBitstreamBuffer(handle,  prdPrt,	pwrPtr, size);
}

JpgRet mmp_jpu_if_ana::JPU_EncUpdateBitstreamBuffer(JpgEncHandle handle, int size) {
    return mmp_jpu_dev::get_instance()->JPU_EncUpdateBitstreamBuffer(handle, size);
}

JpgRet mmp_jpu_if_ana::JPU_EncStartOneFrame(JpgEncHandle handle, JpgEncParam * param, int TmbEn) {
    return mmp_jpu_dev::get_instance()->JPU_EncStartOneFrame(handle, param, TmbEn);
}

JpgRet mmp_jpu_if_ana::JPU_EncGetOutputInfo(JpgEncHandle handle, JpgEncOutputInfo * info) {
    return mmp_jpu_dev::get_instance()->JPU_EncGetOutputInfo(handle, info);
}

JpgRet mmp_jpu_if_ana::JPU_EncIssueStop(JpgDecHandle handle) {
    return mmp_jpu_dev::get_instance()->JPU_EncIssueStop(handle);
}

JpgRet mmp_jpu_if_ana::JPU_EncCompleteStop(JpgDecHandle handle) {
    return mmp_jpu_dev::get_instance()->JPU_EncCompleteStop(handle);
}

JpgRet mmp_jpu_if_ana::JPU_EncGiveCommand(JpgEncHandle handle, JpgCommand cmd, void * parameter) {
    return mmp_jpu_dev::get_instance()->JPU_EncGiveCommand(handle, cmd, parameter);
}

void mmp_jpu_if_ana::JPU_EncSetHostParaAddr(PhysicalAddress baseAddr, PhysicalAddress paraAddr) {
    mmp_jpu_dev::get_instance()->JPU_EncSetHostParaAddr(baseAddr, paraAddr);
}

