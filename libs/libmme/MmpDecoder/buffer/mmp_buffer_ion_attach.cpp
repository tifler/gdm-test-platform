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

#include "mmp_buffer_ion_attach.hpp"
#include "ion_api.h"

#include <sys/mman.h>
#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "MmpUtil.hpp"
#include "vpu.h"

#if (MMP_OS==MMP_OS_WIN32)
#undef open
#endif

mmp_buffer_ion_attach::mmp_buffer_ion_attach(struct mmp_buffer_create_config *p_create_config) : mmp_buffer(p_create_config)

#if (MMP_OS == MMP_OS_WIN32)
,m_phyaddr_fd(-1)
#endif

{
    
}

mmp_buffer_ion_attach::~mmp_buffer_ion_attach() {

}

MMP_RESULT mmp_buffer_ion_attach::open() {

    int ion_fd;
    MMP_RESULT mmpResult = MMP_FAILURE;

    ion_fd = ion_open();
    if(ion_fd >= 0) {
        
        this->m_buf_addr.m_shared_fd = this->m_create_config.attach_shared_fd;
        this->m_buf_addr.m_phy_addr = this->m_create_config.attach_phy_addr;

        
        if(this->m_buf_addr.m_shared_fd >= 0) {
            
            this->m_buf_addr.m_vir_addr = (unsigned int)MMP_DRIVER_MMAP(NULL, this->m_buf_addr.m_size, (PROT_READ | PROT_WRITE), MAP_SHARED, this->m_buf_addr.m_shared_fd, 0);
            this->m_buf_addr.m_phy_addr = this->get_phy_addr_from_shared_fd(this->m_buf_addr.m_shared_fd);
        }
        else if(this->m_buf_addr.m_phy_addr != 0) {
            this->m_buf_addr.m_vir_addr = this->get_vir_addr_from_phy_addr(this->m_buf_addr.m_phy_addr, this->m_buf_addr.m_size);
            this->m_buf_addr.m_shared_fd = -1;
        }
        else {
            this->m_buf_addr.m_vir_addr = 0;
            this->m_buf_addr.m_phy_addr = 0;
            this->m_buf_addr.m_shared_fd = -1;
        }
        

#if (MMP_OS == MMP_OS_WIN32)
        this->m_buf_addr.m_phy_addr = this->m_buf_addr.m_vir_addr;
#endif

        if( (this->m_buf_addr.m_phy_addr != 0) && (this->m_buf_addr.m_vir_addr != 0) ) {
            
            this->m_buf_addr.m_phy_addr += this->m_create_config.attach_offset;
            this->m_buf_addr.m_vir_addr += this->m_create_config.attach_offset;

            mmpResult = MMP_SUCCESS;
        }

        ion_close(ion_fd);
    }

    return mmpResult;
}

MMP_RESULT mmp_buffer_ion_attach::close() {

    if(this->m_buf_addr.m_vir_addr != 0) {
        MMP_DRIVER_MUNMAP((void*)this->m_buf_addr.m_vir_addr, this->m_buf_addr.m_size);
        this->m_buf_addr.m_vir_addr = 0;
    }

    if(this->m_buf_addr.m_shared_fd >= 0) {
        
        this->m_buf_addr.m_shared_fd = -1;
        this->m_buf_addr.m_vir_addr = 0;
    }

#if (MMP_OS == MMP_OS_WIN32)        
    if(m_phyaddr_fd >= 0) {
        ion_close(m_phyaddr_fd);
        m_phyaddr_fd = -1;
    }
#endif

    return MMP_SUCCESS;
}


MMP_RESULT mmp_buffer_ion_attach::sync_buf() {
    
    MMP_S32 ion_fd;

    ion_fd = ion_open();
    if(ion_fd >= 0) {
        if(this->m_buf_addr.m_shared_fd >= 0) {
            ion_sync_fd(ion_fd, this->m_buf_addr.m_shared_fd);
        }
        ion_close(ion_fd);
    }
    return MMP_SUCCESS;
}
