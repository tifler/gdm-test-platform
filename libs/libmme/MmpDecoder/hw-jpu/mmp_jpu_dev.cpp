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

#include "mmp_jpu_def.h"
#include "mmp_buffer.hpp"
#include "mmp_env_mgr.hpp"

#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "mmp_buffer_mgr.hpp"
#include "mmp_jpu_dev.hpp"
#include "mmp_jpu_dev_shm.hpp"  /* usigne shared jdi memory */
#include "MmpUtil.hpp"

#if (JPU_PLATFORM_V4L2_ENABLE != 1)

/**********************************************************
create/destroy object
**********************************************************/

class mmp_jpu_dev* mmp_jpu_dev::s_p_instance = NULL;

MMP_RESULT mmp_jpu_dev::create_instance() {
    
    struct mmp_jpu_dev_create_config jpu_create_config;
    memset(&jpu_create_config, 0x00, sizeof(struct mmp_jpu_dev_create_config));

    return mmp_jpu_dev::create_instance(&jpu_create_config);
}

MMP_RESULT mmp_jpu_dev::create_instance(struct mmp_jpu_dev_create_config* p_create_config) {

    MMP_RESULT mmpResult;
	class mmp_jpu_dev* p_obj = NULL;

	//p_obj = new class mmp_jpu_dev_ex1(p_create_config);
	//p_obj = new class mmp_jpu_dev_ex2(p_create_config);
	//p_obj = new class mmp_jpu_dev_ex3(p_create_config);
	p_obj = new class mmp_jpu_dev_shm(p_create_config);
	if(p_obj!=NULL) {
        
		if(p_obj->open( ) != MMP_ErrorNone)
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    if(p_obj != NULL) {
        mmp_jpu_dev::s_p_instance = p_obj;
        mmpResult = MMP_SUCCESS;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}

MMP_RESULT mmp_jpu_dev::destroy_instance() {

    class mmp_jpu_dev* p_obj = mmp_jpu_dev::s_p_instance;

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;

        mmp_jpu_dev::s_p_instance = NULL;
    }

    return MMP_SUCCESS;
}

class mmp_jpu_dev* mmp_jpu_dev::get_instance() {

    return mmp_jpu_dev::s_p_instance;
}

/**********************************************************
class members
**********************************************************/

mmp_jpu_dev::mmp_jpu_dev(struct mmp_jpu_dev_create_config* p_create_config) :
m_create_config(*p_create_config)
{

}

mmp_jpu_dev::~mmp_jpu_dev() {

}

#if 0
MMP_S32 mmp_jpu_dev::get_phy_addr_from_shared_fd(MMP_S32 shared_fd) {

    MMP_S32 ret;
    MMP_S32 jpu_fd;
    MMP_U32 int_array[3];
    MMP_U32 phy_addr = 0;

    jpu_fd = mmp_env_mgr::get_instance()->get_jpu_fd();
    if(jpu_fd >= 0) {

        int_array[0] = (unsigned int)shared_fd;

        ret = MMP_DRIVER_IOCTL(jpu_fd, VDI_IOCTL_GET_ION_PHY_ADDR_WITH_SHARED_FD, int_array);
        if(ret >= 0) {
            phy_addr = int_array[1];
        }
    }

    return phy_addr;
}

MMP_U32 mmp_jpu_dev::get_vir_addr_from_phy_addr(MMP_U32 phy_addr, MMP_S32 size) {

    MMP_S32 jpu_fd;
    MMP_U32 vir_addr = 0;

    jpu_fd = mmp_env_mgr::get_instance()->get_jpu_fd();
    if(jpu_fd >= 0) {
        vir_addr = (unsigned int)MMP_DRIVER_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, jpu_fd, phy_addr);
    }

    return vir_addr;
}

MMP_S32 mmp_jpu_dev::get_phy_addr_for_common_buffer() {

    MMP_S32 ret;
    MMP_S32 jpu_fd;
    MMP_U32 int_array[3];
    MMP_U32 phy_addr = 0;

    jpu_fd = mmp_env_mgr::get_instance()->get_jpu_fd();
    if(jpu_fd >= 0) {

        ret = MMP_DRIVER_IOCTL(jpu_fd, VDI_IOCTL_GET_ION_PHY_ADDR_FOR_COMMON_BUFFER, int_array);
        if(ret >= 0) {
            phy_addr = int_array[0];
        }
    }

    return phy_addr;
}

void mmp_jpu_dev::VPU_GetCodecInfo(int idx, struct mmp_video_hw_codec_instance_info *p_info) {
    ::VPU_GetCodecInfo(idx,  p_info);
}
#endif
#endif