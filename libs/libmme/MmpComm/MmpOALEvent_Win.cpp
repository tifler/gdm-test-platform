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
#include"MMPOALEvent_Win.hpp"


#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

struct SMmpOALEvent
{
    HANDLE hEvent;
    bool bCreate;
};

CMmpOALEvent_Win::CMmpOALEvent_Win()
{

}

CMmpOALEvent_Win::~CMmpOALEvent_Win()
{

}

MMPOALEVENT_HANDLE CMmpOALEvent_Win::Create(void* pObject )
{
    SMmpOALEvent* pEventObj;
    
    pEventObj=new SMmpOALEvent;
    if(!pEventObj)
    {
        return (MMPOALEVENT_HANDLE)NULL;
    }

    pEventObj->hEvent=(HANDLE)pObject;
    
    pEventObj->bCreate=false;

    return (MMPOALEVENT_HANDLE)pEventObj;
}

MMPOALEVENT_HANDLE CMmpOALEvent_Win::Create(bool bManualReset, bool bInitialState, MMPSTR pName )
{
    SMmpOALEvent* pEventObj;
    
    pEventObj=new SMmpOALEvent;
    if(!pEventObj)
    {
        return (MMPOALEVENT_HANDLE)NULL;
    }

    pEventObj->hEvent=CreateEvent(NULL, bManualReset?TRUE:FALSE, bInitialState?TRUE:FALSE, pName );
    if(!pEventObj->hEvent)
    {
        delete pEventObj;
        return (MMPOALEVENT_HANDLE)NULL;
    }
    
    pEventObj->bCreate=true;

    return (MMPOALEVENT_HANDLE)pEventObj;
}


MMP_RESULT CMmpOALEvent_Win::Destroy( MMPOALEVENT_HANDLE event_hdl )
{
    SMmpOALEvent* pEventObj=(SMmpOALEvent*)event_hdl;
    if(!pEventObj) return MMP_FAILURE;
    
    if(pEventObj->bCreate && pEventObj->hEvent)
    {
        CloseHandle(pEventObj->hEvent);
    }
    delete pEventObj;
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALEvent_Win::Set( MMPOALEVENT_HANDLE event_hdl )
{
    SMmpOALEvent* pEventObj=(SMmpOALEvent*)event_hdl;
    if(!pEventObj) return MMP_FAILURE;
    if(!pEventObj->hEvent) return MMP_FAILURE;

    SetEvent(pEventObj->hEvent);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALEvent_Win::Wait( MMPOALEVENT_HANDLE event_hdl, int timeOutMileSec )
{
    SMmpOALEvent* pEventObj=(SMmpOALEvent*)event_hdl;
    if(!pEventObj) return MMP_FAILURE;
    if(!pEventObj->hEvent) return MMP_FAILURE;

    DWORD res;
    res=WaitForSingleObject(pEventObj->hEvent, (timeOutMileSec==-1)?INFINITE:timeOutMileSec );
    
    return (res==WAIT_OBJECT_0)?MMP_SUCCESS:MMP_FAILURE;
}

void* CMmpOALEvent_Win::GetObjectHandle(MMPOALEVENT_HANDLE event_hdl)
{
    SMmpOALEvent* pEventObj=(SMmpOALEvent*)event_hdl;
    if(!pEventObj) return (void*)NULL;
    return (void*)pEventObj->hEvent;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

