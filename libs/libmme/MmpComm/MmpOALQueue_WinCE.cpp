/****************************************************************************
 * TokiPlayer Corp.
 * Copyright (C) 2006
 *
 * All rights reserved. This file contains information that is proprietary to
 * TokiPlayer Corp. and may not be distributed or copied without
 * written consent from Mtekvison Corp.
 *
 *   $Id: Class≈€«√∏¥.cpp,v 1.1.2.1 2007/06/07 23:55:13 hwanght Exp $
 *
 */


#include"../MmpGlobal/MmpDefine.h"
#include"MMPOALQueue_WinCE.hpp"

#if (MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

#include <pm.h>
#include <Msgqueue.h>

struct SOALQUEUE
{
    HANDLE hReadQueue;
    HANDLE hWriteQueue;
};

CMmpOALQueue_WinCE::CMmpOALQueue_WinCE()
{

}

CMmpOALQueue_WinCE::~CMmpOALQueue_WinCE()
{

}

MMPOALQUEUE_HANDLE CMmpOALQueue_WinCE::Create( MMPSTR  name, int maxDataCount, int maxDataSize )
{
    SOALQUEUE* pOALQueue=NULL;
    HANDLE hProcess=NULL;
    MSGQUEUEOPTIONS msgReadOptions = {0};  
    MSGQUEUEOPTIONS msgWriteOptions = {0};  
    
    //Create OALQueue Object
    pOALQueue=new SOALQUEUE;
    if(!pOALQueue)
    {
        goto action_fail;
    }
    memset(pOALQueue, 0x00, sizeof(SOALQUEUE));

    //Set Read Option
    msgReadOptions.dwSize = sizeof(MSGQUEUEOPTIONS);  
    msgReadOptions.dwFlags = 0;  
    msgReadOptions.dwMaxMessages = maxDataCount; 
    msgReadOptions.cbMaxMessage = maxDataSize;
    msgReadOptions.bReadAccess = TRUE; 
    
    //Set Write Option
    msgWriteOptions.dwSize = sizeof(MSGQUEUEOPTIONS);  
    msgWriteOptions.dwFlags = 0;  
    msgWriteOptions.dwMaxMessages = maxDataCount; 
    msgWriteOptions.cbMaxMessage = maxDataSize;
    msgWriteOptions.bReadAccess = FALSE; 
    
    //Create ReadQueue
    pOALQueue->hReadQueue = CreateMsgQueue(NULL, &msgReadOptions);  
    if(!pOALQueue->hReadQueue)
    {
        goto action_fail;
    }
    
    
    hProcess=OpenProcess(NULL, FALSE, GetCurrentProcessId() );
    if(!hProcess)
    {
        goto action_fail;
    }

    //Create ReadQueue
    pOALQueue->hWriteQueue = OpenMsgQueue(hProcess, pOALQueue->hReadQueue, &msgWriteOptions);  
    if(!pOALQueue->hWriteQueue)
    {
        goto action_fail;
    }

    CloseHandle(hProcess);
    
    return (MMPOALQUEUE_HANDLE)pOALQueue;

action_fail:

    if(pOALQueue)
    {
        if(pOALQueue->hWriteQueue)
        {
            CloseHandle(pOALQueue->hWriteQueue);
        }
    
        if(pOALQueue->hReadQueue)
        {
            CloseHandle(pOALQueue->hReadQueue);
        }

        delete pOALQueue;
        pOALQueue=NULL;
    }

    if(hProcess)
        CloseHandle(hProcess);

    return (MMPOALQUEUE_HANDLE)NULL;
}

MMP_RESULT CMmpOALQueue_WinCE::Destroy( MMPOALQUEUE_HANDLE oalqueue_handle )
{
    SOALQUEUE* pOALQueue=(SOALQUEUE*)oalqueue_handle;

    if(pOALQueue)
    {
        if(pOALQueue->hWriteQueue)
        {
            CloseHandle(pOALQueue->hWriteQueue);
        }
    
        if(pOALQueue->hReadQueue)
        {
            CloseHandle(pOALQueue->hReadQueue);
        }

        delete pOALQueue;
        pOALQueue=NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALQueue_WinCE::Send( MMPOALQUEUE_HANDLE oalqueue_handle, void* const pData, int msgSize  )
{
    BOOL flag;
    SOALQUEUE* pOALQueue=(SOALQUEUE*)oalqueue_handle;
    if(!pOALQueue)
        return MMP_FAILURE;

    flag=WriteMsgQueue(pOALQueue->hWriteQueue, pData, msgSize, INFINITE, 0);

    return flag?MMP_SUCCESS:MMP_FAILURE;
}

MMP_RESULT CMmpOALQueue_WinCE::Receive( MMPOALQUEUE_HANDLE oalqueue_handle, void* pData, int dataSize, int* readSize, int timeOut )
{
    BOOL flag;
    DWORD queueFlag;
    DWORD dwWait;
    DWORD wait;
    SOALQUEUE* pOALQueue=(SOALQUEUE*)oalqueue_handle;
    if(!pOALQueue)
        return MMP_FAILURE;

    wait=(timeOut==-1)?INFINITE:timeOut;
    dwWait=WaitForSingleObject(pOALQueue->hReadQueue, wait);
    if(dwWait!=WAIT_OBJECT_0)
    {
        return MMP_FAILURE;
    }

    flag=ReadMsgQueue(pOALQueue->hReadQueue, pData, dataSize, (LPDWORD)readSize, 0, &queueFlag );

    return flag?MMP_SUCCESS:MMP_FAILURE;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )

