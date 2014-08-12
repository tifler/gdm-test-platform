/****************************************************************************
 * TokiPlayer Corp.
 * Copyright (C) 2006
 *
 * All rights reserved. This file contains information that is proprietary to
 * TokiPlayer Corp. and may not be distributed or copied without
 * written consent from Mtekvison Corp.
 *
 *   $Id: ClassÅÛÇÃ¸´.cpp,v 1.1.2.1 2007/06/07 23:55:13 hwanght Exp $
 *
 */


#include "../MmpGlobal/MmpDefine.h"
#include"MmpOALInterrupt_Win.hpp"

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60|| MMP_OS==MMP_OS_WINCE60_APP)

DWORD CMmpOALInterrupt_Win::UsbInterruptThreadStub( IN PVOID context )
{
    MMP_ISR_OBJECT* pISRObj=(MMP_ISR_OBJECT*)context;
    HANDLE hEvent;
    
    hEvent=(HANDLE)pISRObj->event_hdl;

    while( pISRObj->isr_run )
    {
        WaitForSingleObject(hEvent, INFINITE);
        if(!pISRObj->isr_run) break;
        pISRObj->hisr_func(pISRObj->hisr_param);
    }

    //CMmpOALInterrupt_Win* pMmpOALInterrupt=(CMmpOALInterrupt_Win*)pISRObj->mmpoal_intr_hdl;
    return 0;//pMmpOALInterrupt->UsbInterruptThread(pISRObj);
}

CMmpOALInterrupt_Win::CMmpOALInterrupt_Win()
{

}

CMmpOALInterrupt_Win::~CMmpOALInterrupt_Win()
{

}

MMP_RESULT CMmpOALInterrupt_Win::Lock()
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_Win::UnLock()
{
    return MMP_SUCCESS;
}

MMPOALISR_HANDLE CMmpOALInterrupt_Win::Create( MMPOALINT oalIrq, void (*callbackFunc)(void*), void* callbackParm, int sysIrq  )
{
    MMP_ISR_OBJECT* pISRObj=NULL;
    
    pISRObj=new MMP_ISR_OBJECT;
    if(!pISRObj)
    {
        goto action_fail;
    }
    memset( pISRObj, 0x00, sizeof(MMP_ISR_OBJECT));
    pISRObj->mmpoal_intr_hdl=(void*)this;
    pISRObj->hisr_param=callbackParm;
    pISRObj->hisr_func=(void (*)(void*))callbackFunc;
    
    switch(oalIrq)
    {
        case MMPOAL_INT_IRQ_USB:
            pISRObj->isr_func=(void*)CMmpOALInterrupt_Win::UsbInterruptThreadStub;
            break;

        default:
            goto action_fail;
    }

    pISRObj->event_hdl=(void*)CreateEvent( NULL, FALSE, FALSE, NULL );
    if(!pISRObj->event_hdl)
    {
        goto action_fail;
    }

    pISRObj->isr_run=1;
    pISRObj->isr_hdl=CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)pISRObj->isr_func, pISRObj, 0, NULL );
    if( pISRObj->isr_hdl==NULL ) 
    {
        goto action_fail;
    }
    //CeSetThreadPriority( m_hUsbInterruptThread, g_IstThreadPriority );

    return (MMPOALISR_HANDLE)pISRObj;

action_fail:
    if(pISRObj->isr_hdl)
    {
        pISRObj->isr_run=0;
        
        WaitForSingleObject((HANDLE)pISRObj->isr_hdl, INFINITE);
        CloseHandle((HANDLE)pISRObj->isr_hdl);
        pISRObj->isr_hdl=NULL;
    }

    if(pISRObj->event_hdl)
    {
        CloseHandle((HANDLE)pISRObj->event_hdl);
        pISRObj->event_hdl=NULL;
    }

    if(pISRObj)
        delete pISRObj;
    
    return (MMPOALISR_HANDLE)NULL;
}

MMP_RESULT CMmpOALInterrupt_Win::Destroy( MMPOALISR_HANDLE hdl )
{
    MMP_ISR_OBJECT* pISRObj=(MMP_ISR_OBJECT*)hdl;
    
    if(pISRObj->isr_hdl)
    {
        pISRObj->isr_run=0;
        WaitForSingleObject((HANDLE)pISRObj->isr_hdl, INFINITE);
        CloseHandle((HANDLE)pISRObj->isr_hdl);
        pISRObj->isr_hdl=NULL;
    }

    if(pISRObj->event_hdl)
    {
        CloseHandle((HANDLE)pISRObj->event_hdl);
        pISRObj->event_hdl=NULL;
    }

    if(pISRObj)
        delete pISRObj;
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_Win::Enable(MMPOALISR_HANDLE hdl)
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_Win::Disable(MMPOALISR_HANDLE hdl)
{
    return MMP_SUCCESS;
}
 
bool CMmpOALInterrupt_Win::IsEnable(MMPOALISR_HANDLE hdl)
{
    return false;//prop->enable_irq?true:false;
}

MMP_RESULT CMmpOALInterrupt_Win::Done(MMPOALISR_HANDLE hdl)
{
    return MMP_SUCCESS;
}

DWORD CMmpOALInterrupt_Win::UsbInterruptThread(MMP_ISR_OBJECT* pISRObj)
{
    return 0;
}

int CMmpOALInterrupt_Win::GetSysIrq( MMPOALISR_HANDLE hdl  )
{
    return 0;    
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )


