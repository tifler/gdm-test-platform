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

#include"MmpOALSemaphore_Linux.hpp"

#if (MMP_OS == MMP_OS_LINUX)

#include <pthread.h>
#include <semaphore.h>

struct sema_obj {
    sem_t hSema;
};

CMmpOALSemaphore_Linux::CMmpOALSemaphore_Linux() 
{

}

CMmpOALSemaphore_Linux::~CMmpOALSemaphore_Linux()
{

}

MMPOALSEMAPHORE_HANDLE CMmpOALSemaphore_Linux::Create( char* name, int mode, int count ) {

    struct sema_obj* p_obj;

    p_obj = (struct sema_obj*)malloc(sizeof(struct sema_obj));
    if(p_obj != NULL) {
        memset(p_obj, 0x00, sizeof(struct sema_obj));

        if (sem_init(&p_obj->hSema, 0, count) != 0) {
            free((void*)p_obj);
            p_obj = NULL;
        }
    }

    return (MMPOALSEMAPHORE_HANDLE)p_obj;
}

MMP_RESULT CMmpOALSemaphore_Linux::Destroy( MMPOALSEMAPHORE_HANDLE sema_handle ){

    struct sema_obj* p_obj = (struct sema_obj*)sema_handle;

    if(p_obj != NULL) {
        sem_destroy(&p_obj->hSema);
        free((void*)p_obj);
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpOALSemaphore_Linux::Obtain( MMPOALSEMAPHORE_HANDLE sema_handle ){

    struct sema_obj* p_obj = (struct sema_obj*)sema_handle;

    sem_wait(&p_obj->hSema);

    return MMP_SUCCESS;
}


MMP_RESULT CMmpOALSemaphore_Linux::Release( MMPOALSEMAPHORE_HANDLE sema_handle ){

    struct sema_obj* p_obj = (struct sema_obj*)sema_handle;

    sem_post(&p_obj->hSema);

    return MMP_SUCCESS;
}


#endif //#if (MMP_OS == MMP_OS_WIN32)