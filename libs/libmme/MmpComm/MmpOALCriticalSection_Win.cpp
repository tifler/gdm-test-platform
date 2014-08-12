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
#include"MMPOALCriticalSection_Win.hpp"


#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

struct SMmpOALCs
{
    CRITICAL_SECTION* pCS;
    bool bCreate;
};

CMmpOALCriticalSection_Win::CMmpOALCriticalSection_Win()
{

}

CMmpOALCriticalSection_Win::~CMmpOALCriticalSection_Win()
{

}

MMPOALCS_HANDLE CMmpOALCriticalSection_Win::Create(void* pObject )
{
    SMmpOALCs* pCsObj;
    
    pCsObj=new SMmpOALCs;
    if(!pCsObj)
    {
        return (MMPOALCS_HANDLE)NULL;
    }

    pCsObj->pCS=(CRITICAL_SECTION*)pObject;
    
    pCsObj->bCreate=false;

    return (MMPOALCS_HANDLE)pCsObj;
}

MMPOALCS_HANDLE CMmpOALCriticalSection_Win::Create()
{
    SMmpOALCs* pCsObj;
    
    pCsObj=new SMmpOALCs;
    if(!pCsObj)
    {
        return (MMPOALCS_HANDLE)NULL;
    }

    pCsObj->pCS=new CRITICAL_SECTION;
    InitializeCriticalSection(pCsObj->pCS);
    
    pCsObj->bCreate=true;

    return (MMPOALCS_HANDLE)pCsObj;
}


MMP_RESULT CMmpOALCriticalSection_Win::Destroy( MMPOALCS_HANDLE cs_hdl )
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return MMP_FAILURE;
    
    if(pCsObj->bCreate && pCsObj->pCS)
    {
        DeleteCriticalSection(pCsObj->pCS);
        delete pCsObj->pCS;
    }
    delete pCsObj;
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALCriticalSection_Win::Enter( MMPOALCS_HANDLE cs_hdl )
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return MMP_FAILURE;
    if(!pCsObj->pCS) return MMP_FAILURE;

    EnterCriticalSection(pCsObj->pCS);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALCriticalSection_Win::Leave( MMPOALCS_HANDLE cs_hdl )
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return MMP_FAILURE;
    if(!pCsObj->pCS) return MMP_FAILURE;

    LeaveCriticalSection(pCsObj->pCS);

    return MMP_SUCCESS;
}

void* CMmpOALCriticalSection_Win::GetObjectHandle(MMPOALCS_HANDLE cs_hdl)
{
    SMmpOALCs* pCsObj=(SMmpOALCs*)cs_hdl;
    if(!pCsObj) return (void*)NULL;
    return (void*)pCsObj->pCS;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

