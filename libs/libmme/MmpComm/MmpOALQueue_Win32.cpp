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
#include"MMPOALQueue_Win32.hpp"

#if (MMP_OS==MMP_OS_WIN32 )

struct SOALQUEUEITEM
{
    int key;
    int dataSize;
    unsigned char* pData;
};

struct SOALQUEUE
{
    HANDLE hEvent;
    int maxDataSize;
    TCircular_Queue<SOALQUEUEITEM>* pMsgQueue;
};


CMmpOALQueue_Win32::CMmpOALQueue_Win32() 
{

}

CMmpOALQueue_Win32::~CMmpOALQueue_Win32()
{

}

MMPOALQUEUE_HANDLE CMmpOALQueue_Win32::Create( MMPSTR  name, int maxDataCount, int maxDataSize )
{
    SOALQUEUE* pOALQueue=NULL;
    
    //Create OALQueue Object
    pOALQueue=new SOALQUEUE;
    if(!pOALQueue)
    {
        goto action_fail;
    }
    memset(pOALQueue, 0x00, sizeof(SOALQUEUE));
    pOALQueue->maxDataSize=maxDataSize;

    pOALQueue->pMsgQueue=new TCircular_Queue<SOALQUEUEITEM>(maxDataCount);
    if(!pOALQueue->pMsgQueue)
    {
        goto action_fail;
    }

    pOALQueue->hEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!pOALQueue->hEvent)
    {
        goto action_fail;
    }

    return (MMPOALQUEUE_HANDLE)pOALQueue;

action_fail:

    this->Destroy((MMPOALQUEUE_HANDLE)pOALQueue);
    
    return (MMPOALQUEUE_HANDLE)NULL;
}

MMP_RESULT CMmpOALQueue_Win32::Destroy( MMPOALQUEUE_HANDLE oalqueue_handle )
{
    SOALQUEUE* pOALQueue=(SOALQUEUE*)oalqueue_handle;

    if(pOALQueue)
    {
        if(pOALQueue->pMsgQueue)
        {
            SOALQUEUEITEM queueItem;
            while(!pOALQueue->pMsgQueue->IsEmpty())
            {
                pOALQueue->pMsgQueue->Delete(queueItem);
                delete [] queueItem.pData;
            }
            delete pOALQueue->pMsgQueue;
            pOALQueue->pMsgQueue=NULL;
        }

        if(pOALQueue->hEvent)
        {
            CloseHandle(pOALQueue->hEvent);
            pOALQueue->hEvent=NULL;
        }

        delete pOALQueue;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALQueue_Win32::Send( MMPOALQUEUE_HANDLE oalqueue_handle, void* const pData, int dataSize  )
{
    SOALQUEUE* pOALQueue=(SOALQUEUE*)oalqueue_handle;
    if(!pOALQueue)
        return MMP_FAILURE;

    if( pOALQueue->maxDataSize < dataSize )
    {
        return MMP_FAILURE;
    }

    if(pOALQueue->pMsgQueue->IsFull())
        return MMP_FAILURE;

    SOALQUEUEITEM queueItem;
    queueItem.dataSize=dataSize;
    queueItem.pData=new unsigned char[queueItem.dataSize];
    if(!queueItem.pData)
        return MMP_FAILURE;

    memcpy(queueItem.pData, pData, queueItem.dataSize);

    pOALQueue->pMsgQueue->Add(queueItem);

    SetEvent(pOALQueue->hEvent);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALQueue_Win32::Receive( MMPOALQUEUE_HANDLE oalqueue_handle, void* pData, int dataBufSize, int* readSize, int timeOut )
{
    DWORD dwWait;
    SOALQUEUEITEM queueItem;
    SOALQUEUE* pOALQueue=(SOALQUEUE*)oalqueue_handle;
    DWORD wait;
    if(!pOALQueue)
        return MMP_FAILURE;

    wait=(timeOut==-1)?INFINITE:timeOut;
    dwWait=WaitForSingleObject(pOALQueue->hEvent, wait);
    if(dwWait!=WAIT_OBJECT_0)
    {
        return MMP_FAILURE;
    }

    if(pOALQueue->pMsgQueue->IsEmpty() )
    {
        return MMP_FAILURE;
    }
    
    pOALQueue->pMsgQueue->Delete(queueItem);

    if(queueItem.dataSize>dataBufSize)
        return MMP_FAILURE;
    
    memcpy(pData, queueItem.pData, queueItem.dataSize );
    if(readSize) *readSize=queueItem.dataSize;
    delete [] queueItem.pData;
    
    return MMP_SUCCESS;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )