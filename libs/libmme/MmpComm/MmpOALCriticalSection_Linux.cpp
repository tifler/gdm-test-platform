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


#include"../MmpGlobal/MmpDefine.h"
#include"MmpOALCriticalSection_Linux.hpp"


#if 1//(MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

#include <pthread.h>

struct SMmpOALCs
{
    pthread_mutexattr_t m_mutexattr;
	pthread_mutex_t m_mutex;
	
    bool bCreate;
};

CMmpOALCriticalSection_Linux::CMmpOALCriticalSection_Linux()
{

}

CMmpOALCriticalSection_Linux::~CMmpOALCriticalSection_Linux()
{

}

MMPOALCS_HANDLE CMmpOALCriticalSection_Linux::Create(void* pObject )
{
    return (MMPOALCS_HANDLE)NULL;
}

MMPOALCS_HANDLE CMmpOALCriticalSection_Linux::Create()
{
    SMmpOALCs* pCsObj;
    
    pCsObj=new SMmpOALCs;
    if(!pCsObj)
    {
        return (MMPOALCS_HANDLE)NULL;
    }

    pthread_mutexattr_init(&pCsObj->m_mutexattr);
	pthread_mutexattr_setpshared(&pCsObj->m_mutexattr, PTHREAD_PROCESS_SHARED);
		
	if(pthread_mutex_init(&pCsObj->m_mutex, &pCsObj->m_mutexattr) != 0) {
        
		//pCsObj->m_mutex.value = __PTHREAD_MUTEX_INIT_VALUE;
        //ret = OMX_ErrorUndefined;

        delete pCsObj;
        pCsObj = NULL;
    }

    return (MMPOALCS_HANDLE)pCsObj;
}


MMP_RESULT CMmpOALCriticalSection_Linux::Destroy( MMPOALCS_HANDLE cs_hdl )
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return MMP_FAILURE;
    
    pthread_mutex_destroy(&pCsObj->m_mutex);
    pthread_mutexattr_destroy(&pCsObj->m_mutexattr);
    delete pCsObj;
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALCriticalSection_Linux::Enter( MMPOALCS_HANDLE cs_hdl )
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return MMP_FAILURE;

    pthread_mutex_lock(&pCsObj->m_mutex);
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALCriticalSection_Linux::Leave( MMPOALCS_HANDLE cs_hdl )
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return MMP_FAILURE;
    
    pthread_mutex_unlock(&pCsObj->m_mutex);

    return MMP_SUCCESS;
}

void* CMmpOALCriticalSection_Linux::GetObjectHandle(MMPOALCS_HANDLE cs_hdl)
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return (void*)NULL;

    return (void*)&pCsObj->m_mutex;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

