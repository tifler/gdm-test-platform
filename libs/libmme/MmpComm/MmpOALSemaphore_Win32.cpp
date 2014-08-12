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

#include"MmpOALSemaphore_Win32.hpp"

#if (MMP_OS == MMP_OS_WIN32)

struct sema_obj {
    HANDLE hSema;
};

CMmpOALSemaphore_Win32::CMmpOALSemaphore_Win32() 
{

}

CMmpOALSemaphore_Win32::~CMmpOALSemaphore_Win32()
{

}

MMPOALSEMAPHORE_HANDLE CMmpOALSemaphore_Win32::Create( char* name, int mode, int count ) {

    struct sema_obj* p_obj;

    p_obj = (struct sema_obj*)malloc(sizeof(struct sema_obj));
    memset(p_obj, 0x00, sizeof(struct sema_obj));

    p_obj->hSema = CreateSemaphore(NULL, count, 2, NULL); 

    return (MMPOALSEMAPHORE_HANDLE)p_obj;
}

MMP_RESULT CMmpOALSemaphore_Win32::Destroy( MMPOALSEMAPHORE_HANDLE sema_handle ){

    struct sema_obj* p_obj = (struct sema_obj*)sema_handle;

    if(p_obj != NULL) {
        ::CloseHandle(p_obj->hSema);
        free((void*)p_obj);
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpOALSemaphore_Win32::Obtain( MMPOALSEMAPHORE_HANDLE sema_handle ){

    struct sema_obj* p_obj = (struct sema_obj*)sema_handle;

    ::WaitForSingleObject(p_obj->hSema, INFINITE);

    return MMP_SUCCESS;
}


MMP_RESULT CMmpOALSemaphore_Win32::Release( MMPOALSEMAPHORE_HANDLE sema_handle ){

    struct sema_obj* p_obj = (struct sema_obj*)sema_handle;

    ::ReleaseSemaphore(p_obj->hSema, 1, NULL);

    return MMP_SUCCESS;
}


#endif //#if (MMP_OS == MMP_OS_WIN32)