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

#include "mmp_oal_mutex_linux_sema.hpp"

#if (MMP_OS == MMP_OS_LINUX)


#include <fcntl.h>

/**********************************************************
class members
**********************************************************/

mmp_oal_mutex_linux_sema::mmp_oal_mutex_linux_sema(MMP_U32 key) : mmp_oal_mutex(key) 
,m_p_sem(SEM_FAILED)
{
	
}

mmp_oal_mutex_linux_sema::~mmp_oal_mutex_linux_sema() {

}


MMP_RESULT mmp_oal_mutex_linux_sema::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    char mutex_name[32];
    int iret;

    if(m_key == 0) {
        iret = sem_init(&m_sem, 0, 1);
        if(iret < 0 ) {
            m_p_sem = SEM_FAILED;
            mmpResult = MMP_FAILURE;
        }
        else {
            m_p_sem = &m_sem;
        }
    }
    else {
        sprintf(mutex_name, "mme_sema-0x%08x", m_key);
        m_p_sem = sem_open(mutex_name, O_CREAT, 0777, 1);
        if(m_p_sem == SEM_FAILED) {
            mmpResult = MMP_FAILURE;
        }
    }

	return mmpResult;
}

MMP_RESULT mmp_oal_mutex_linux_sema::close() {
	
	MMP_RESULT ret = MMP_SUCCESS;

    if(m_p_sem != SEM_FAILED) {

        if(m_key == 0) {
            sem_destroy(m_p_sem);
        }
        else {
            sem_close(m_p_sem);
        }
        m_p_sem = SEM_FAILED;
    }

	return ret;
}

void mmp_oal_mutex_linux_sema::lock() {
	
	sem_wait(m_p_sem);
}
	
void mmp_oal_mutex_linux_sema::unlock() {

	sem_post(m_p_sem);
}

void* mmp_oal_mutex_linux_sema::get_handle() {

	return (void*)m_p_sem;
}

#endif
