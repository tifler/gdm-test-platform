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

#include "mmp_oal_shm_linux.hpp"

#if (MMP_OS == MMP_OS_LINUX)

#include<sys/shm.h>

/*

How to check IPC Status

    hthwang@tako-desktop:~$ ipcs

    ------ Shared Memory Segments --------
    key        shmid      owner      perms      bytes      nattch     status
    0x000004d2 98307      hthwang    666        2052       2

    ------ Semaphore Arrays --------
    key        semid      owner      perms      nsems

    ------ Message Queues --------
    key        msqid      owner      perms      used-bytes   messages

How to rm  sharm memd

    hthwang@tako-desktop:~$ ipcrm -m 98307

*/


#if 0
/* One shmid data structure for each shared memory segment in the system. */
        struct shmid_ds {
                struct ipc_perm shm_perm;        /* operation perms */
                int     shm_segsz;               /* size of segment (bytes) */
                time_t  shm_atime;               /* last attach time */
                time_t  shm_dtime;               /* last detach time */
                time_t  shm_ctime;               /* last change time */
                unsigned short  shm_cpid;        /* pid of creator */
                unsigned short  shm_lpid;        /* pid of last operator */
                short   shm_nattch;              /* no. of current attaches */

                                                 /* the following are private */

                unsigned short   shm_npages;     /* size of segment (pages) */
                unsigned long   *shm_pages;      /* array of ptrs to frames -> SHMMAX */ 
                struct vm_area_struct *attaches; /* descriptors for attaches */
        };

    shm_perm �����޸𸮴� �������� ���μ����� ���ÿ� ���� �����ϹǷ�, ���ϰ� ���� �� ���ٱ����� �и��� �������� �Ѵ�.
    shm_segsz �Ҵ�� �޸��� byte ũ���̴�
    shm_atime �����ֱ��� ���μ����� ���׸�Ʈ�� attach�� �ð�
    shm_dtime �����ֱ��� ���μ����� ���׸�Ʈ�� detach�� �ð�
    shm_ctime ���������� �� ����ü�� ����� �ð�
    shm_cpid �� ����ü�� ������ ���μ����� pid
    shm_lpid ���������� �۵��� ������ ���μ����� pid
    shm_nattch ���� �������� ���μ����� ��

#endif


/**********************************************************
class members
**********************************************************/

mmp_oal_shm_linux::mmp_oal_shm_linux(struct mmp_oal_shm_create_config* p_create_config) : mmp_oal_shm(p_create_config)
,m_shm_id(-1)
,m_p_shm((void*)-1)
,m_is_create(MMP_FALSE)
{

}

mmp_oal_shm_linux::~mmp_oal_shm_linux() {

}


MMP_RESULT mmp_oal_shm_linux::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    
    m_is_create = MMP_FALSE;
    m_shm_id = shmget((key_t)this->m_create_config.key, this->m_create_config.size, 0666|IPC_EXCL);
    if(m_shm_id < 0) {

        m_shm_id = shmget((key_t)this->m_create_config.key, this->m_create_config.size, 0666|IPC_CREAT);
        if(m_shm_id >= 0) {
            /* shared memory was created by this process */
            m_is_create = MMP_TRUE;
        }
    }
    else {
        /* shared memory was created by other process */
    }

    if(m_shm_id >= 0) {
         
        m_p_shm = shmat(m_shm_id,(void*)0,0); /* attch shared memory */
        if(m_p_shm == (void*)-1) {
            mmpResult = MMP_FAILURE;
        }
    }

    return mmpResult;
}

MMP_RESULT mmp_oal_shm_linux::close(MMP_BOOL is_remove_from_system) {
	
    struct shmid_ds shm_buf;

    if(m_p_shm != (void*)-1) {
        shmdt(m_p_shm);  /* detach shared memory */
        m_p_shm = (void*)-1;
    }

    if(is_remove_from_system == MMP_TRUE) {
        if(m_shm_id >= 0) {
            shmctl(m_shm_id, IPC_STAT, &shm_buf);

            //printf("shm_buf.shm_nattch = %d\n",  (int)shm_buf.shm_nattch);

            if(shm_buf.shm_nattch == 0) {
                shmctl(m_shm_id, IPC_RMID,  NULL);
                m_shm_id = -1;
            }
        }
    }
    
	return MMP_SUCCESS;
}

MMP_S32 mmp_oal_shm_linux::get_attach_process_count() {
    
    MMP_S32 proc_count = 0;
    struct shmid_ds shm_buf;
    
    if(m_shm_id >= 0) {
        shmctl(m_shm_id, IPC_STAT, &shm_buf);
        proc_count = (MMP_S32)shm_buf.shm_nattch;
    }

    return proc_count;
}

#endif