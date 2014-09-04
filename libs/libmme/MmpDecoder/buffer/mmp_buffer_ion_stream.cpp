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

#include "mmp_buffer_ion_stream.hpp"
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

#if (MMP_OS==MMP_OS_WIN32)
#undef open
#endif

extern "C"  int g_vpu_fd; 


mmp_buffer_ion_stream::mmp_buffer_ion_stream(struct mmp_buffer_create_object *p_create_object) : mmp_buffer(p_create_object)

{
    
}

mmp_buffer_ion_stream::~mmp_buffer_ion_stream() {

}

#define VPU_DEVICE_NAME "/dev/vpu"
MMP_RESULT mmp_buffer_ion_stream::open() {

    int vpu_fd;
    int ion_fd, ret;
    MMP_RESULT mmpResult = MMP_FAILURE;

    ion_fd = ion_open();
    if(ion_fd >= 0) {
        
        ret = ion_alloc_fd(ion_fd, this->m_buf_addr.m_size, 0, ION_HEAP_CARVEOUT_MASK,  0, (int*)&this->m_buf_addr.m_shared_fd);
        if(ret < 0) {
            /* error */
            this->m_buf_addr.m_shared_fd = -1;
        }
        else {
            this->m_buf_addr.m_vir_addr = (unsigned int)MMP_DRIVER_MMAP(NULL, this->m_buf_addr.m_size, (PROT_READ | PROT_WRITE), MAP_SHARED, this->m_buf_addr.m_shared_fd, 0);
            mmpResult = MMP_SUCCESS;

//#if (MMP_OS == MMP_OS_WIN32)
  //          this->m_buf_addr.m_phy_addr = this->m_buf_addr.m_vir_addr;
//#endif
            vpu_fd = g_vpu_fd;//MMP_DRIVER_OPEN(VPU_DEVICE_NAME, O_RDWR);
            if(vpu_fd >= 0) {

                unsigned int int_array[3];

                int_array[0] = this->m_buf_addr.m_shared_fd;

                ret = MMP_DRIVER_IOCTL(vpu_fd, 0x07779829, int_array);
                if(ret >= 0) {
                    this->m_buf_addr.m_phy_addr = int_array[1];      
                    MMPDEBUGMSG(1, (TEXT("[mmp_buffer_ion_stream::open] fd=%d vir_addr=0x%08x phy_addr=0x%08x sz=%d "), 
                         this->m_buf_addr.m_shared_fd,
                         this->m_buf_addr.m_vir_addr,   
                         this->m_buf_addr.m_phy_addr,
                         this->m_buf_addr.m_size
                         ));
                }
                else {
                    MMPDEBUGMSG(1, (TEXT("[mmp_buffer_ion_stream::open] FAIL: convert ion fd -> phy_addr")));
                    mmpResult = MMP_FAILURE;
                }

                //MMP_DRIVER_CLOSE(vpu_fd);
            }
            else {
                MMPDEBUGMSG(1, (TEXT("[mmp_buffer_ion_stream::open] FAIL: open vpu drv")));
                mmpResult = MMP_FAILURE;
            }

        }
        ion_close(ion_fd);
    }

    return mmpResult;
}

MMP_RESULT mmp_buffer_ion_stream::close() {

    if(this->m_buf_addr.m_shared_fd >= 0) {

        MMP_DRIVER_MUNMAP((void*)this->m_buf_addr.m_vir_addr, this->m_buf_addr.m_size);
        ion_close(this->m_buf_addr.m_shared_fd);

        MMPDEBUGMSG(1, (TEXT("[mmp_buffer_ion_stream::close] fd=%d vir_addr=0x%08x phy_addr=0x%08x sz=%d "), 
                         this->m_buf_addr.m_shared_fd,
                         this->m_buf_addr.m_vir_addr,   
                         this->m_buf_addr.m_phy_addr,
                         this->m_buf_addr.m_size
                         ));

        this->m_buf_addr.m_shared_fd = -1;
        this->m_buf_addr.m_vir_addr = 0;
    }


    return MMP_SUCCESS;
}

