/****************************************************************************
 * TokiPlayer Corp.
 * Copyright (C) 2006
 *
 * All rights reserved. This file contains information that is proprietary to
 * TokiPlayer Corp. and may not be distributed or copied without
 * written consent from Mtekvison Corp.
 *
 *   $Id: Class템플릿.cpp,v 1.1.2.1 2007/06/07 23:55:13 hwanght Exp $
 *
 */


#include "../MmpGlobal/MmpDefine.h"
#include"MmpOALInterrupt_Win32.hpp"

#if (MMP_OS==MMP_OS_WIN32 || (MMP_OS==MMP_OS_WINCE60 && MMP_DEVCONFIG!=MMP_DEVCONFIG_DRIVER))

#include "../MmpComm/MmpUtil.hpp"

///////////////////////////////////////////////////////////////////////
// OAL ISR Prop Struct

struct SOALISRPROP
{
    bool bValid;
    MMPOALINT oalIrq;
    bool  enableIrq;
    bool threadRun;
    HANDLE hISREvent;
    HANDLE hISRThread;
    LPTHREAD_START_ROUTINE isrThreadFunc;
    void (*callbackFunc)(void*);
    void* callbackParm;
    CRITICAL_SECTION cs;
    WCHAR wszName[16];
};

////////////////////////////////////////////////////////////////////
// ISR Property Array
static void MMPOAL_ISR_USB_THREAD(void* param);
static void MMPOAL_ISR_THREAD(void* param);

static SOALISRPROP s_ISRObj[MMPOAL_INT_IRQ_MAX]=
{
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_USB, //MMPOALINT oalIrq;
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_USB_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        L"USB"
   },
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_GPIO, //MMPOALINT oalIrq;
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        L"GPIO"
   }
};

///////////////////////////////////////////////////////////////////////
// ISR Thread
static void MMPOAL_ISR_THREAD(void* param)
{
    SOALISRPROP* pISRObj=(SOALISRPROP*)param;
    
    MMPDEBUGMSG(MMPZONE_OAL_INTR, (TEXT("[MMPOAL_ISR_THREAD] %s ISR Thread Start..\n\r"), pISRObj->wszName));
    while(1)
    {
        WaitForSingleObject(pISRObj->hISREvent, INFINITE);
        if(!pISRObj->threadRun)
            break;

        EnterCriticalSection(&pISRObj->cs);
        if( pISRObj->enableIrq)
        {
    
            MMPDEBUGMSG(MMPZONE_OAL_INTR, (TEXT("[MMPOAL_ISR_THREAD] %s Interrupt Occure\n\r"), pISRObj->wszName));
            s_ISRObj->callbackFunc( s_ISRObj->callbackParm );
            //::InterruptDone(s_ISRObj->sysIrqID);
        }
        LeaveCriticalSection(&pISRObj->cs);
    
    }
    MMPDEBUGMSG(MMPZONE_OAL_INTR, (TEXT("[MMPOAL_ISR_THREAD] %s ISR Thread Ended! \n\r"), pISRObj->wszName));
}

static void MMPOAL_ISR_USB_THREAD(void* param)
{
    SOALISRPROP* pISRObj=(SOALISRPROP*)param;
    
    MMPDEBUGMSG(MMPZONE_OAL_INTR, (TEXT("[MMPOAL_ISR_USB_THREAD] USB ISR Thread Start..\n\r")));
    while(1)
    {
        WaitForSingleObject(pISRObj->hISREvent, INFINITE);
        if(!pISRObj->threadRun)
            break;

        EnterCriticalSection(&pISRObj->cs);
        if( pISRObj->enableIrq)
        {
    
            MMPDEBUGMSG(MMPZONE_OAL_INTR, (TEXT("[MMPOAL_ISR_USB_THREAD] USB Interrupt Occure\n\r")));
            s_ISRObj->callbackFunc( s_ISRObj->callbackParm );
            //::InterruptDone(s_ISRObj->sysIrqID);
        }
        LeaveCriticalSection(&pISRObj->cs);
    
    }
    MMPDEBUGMSG(MMPZONE_OAL_INTR, (TEXT("[MMPOAL_ISR_USB_THREAD] USB ISR Thread Ended! \n\r")));
}



/////////////////////////////////////////////////////////////////////
// class Member

CMmpOALInterrupt_Win32::CMmpOALInterrupt_Win32()
{

}

CMmpOALInterrupt_Win32::~CMmpOALInterrupt_Win32()
{

}

MMP_RESULT CMmpOALInterrupt_Win32::Lock()
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_Win32::UnLock()
{
    return MMP_SUCCESS;
}

MMPOALISR_HANDLE CMmpOALInterrupt_Win32::Create( MMPOALINT oalIrq, void (*callbackFunc)(void*), void* callbackParm, int sysIrq  )
{
    SOALISRPROP* pISRObj=NULL;
    
    if(oalIrq<0 || oalIrq>=MMPOAL_INT_IRQ_MAX)
       return (MMPOALISR_HANDLE)NULL;
    
    pISRObj=&s_ISRObj[oalIrq];
    if(pISRObj->bValid==true)
        return (MMPOALISR_HANDLE)NULL;
        
    if(callbackFunc==0)
        return (MMPOALISR_HANDLE)NULL;
      
    pISRObj->callbackFunc=callbackFunc;
    pISRObj->callbackParm=callbackParm;
    pISRObj->hISRThread=NULL;
    pISRObj->hISREvent=NULL;
    pISRObj->threadRun=false;
    pISRObj->enableIrq=false;
    
    pISRObj->hISREvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if( !pISRObj->hISREvent )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateEvent \n\r"), oalIrq));
        goto action_fail;
	}

    InitializeCriticalSection(&pISRObj->cs);
  
    pISRObj->threadRun=true;
    pISRObj->hISRThread=CreateThread(NULL, 0, pISRObj->isrThreadFunc, pISRObj, 0, NULL);
    if( !pISRObj->hISRThread )
    {
        pISRObj->hISRThread=false;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateThread \n\r"), oalIrq));
        goto action_fail;
    }

    pISRObj->bValid=true;
    
    return (MMPOALISR_HANDLE)pISRObj;

action_fail:

    this->Destroy((MMPOALISR_HANDLE)pISRObj);

    return (MMPOALISR_HANDLE)NULL;
}

MMPOALISR_HANDLE CMmpOALInterrupt_Win32::CreateGpio( int iGpioNum, void (*callbackFunc)(void*), void* callbackParm, int threadPriority, int sysIrq    )
{
    SOALISRPROP* pISRObj=NULL;
    const MMPOALINT oalIrq=MMPOAL_INT_IRQ_GPIO;
    
    if(oalIrq<0 || oalIrq>=MMPOAL_INT_IRQ_MAX)
       return (MMPOALISR_HANDLE)NULL;
    
    pISRObj=&s_ISRObj[oalIrq];
    if(pISRObj->bValid==true)
        return (MMPOALISR_HANDLE)NULL;
        
    if(callbackFunc==0)
        return (MMPOALISR_HANDLE)NULL;
      
    pISRObj->callbackFunc=callbackFunc;
    pISRObj->callbackParm=callbackParm;
    pISRObj->hISRThread=NULL;
    pISRObj->hISREvent=NULL;
    pISRObj->threadRun=false;
    pISRObj->enableIrq=false;
    
    pISRObj->hISREvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if( !pISRObj->hISREvent )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateEvent \n\r"), oalIrq));
        goto action_fail;
	}

    InitializeCriticalSection(&pISRObj->cs);
  
    pISRObj->threadRun=true;
    pISRObj->hISRThread=CreateThread(NULL, 0, pISRObj->isrThreadFunc, pISRObj, 0, NULL);
    if( !pISRObj->hISRThread )
    {
        pISRObj->hISRThread=false;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateThread \n\r"), oalIrq));
        goto action_fail;
    }

    pISRObj->bValid=true;
    
    return (MMPOALISR_HANDLE)pISRObj;

action_fail:

    this->Destroy((MMPOALISR_HANDLE)pISRObj);

    return (MMPOALISR_HANDLE)NULL;
}

MMP_RESULT CMmpOALInterrupt_Win32::Destroy( MMPOALISR_HANDLE hdl )
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return MMP_FAILURE;
    

    if(pISRObj->hISRThread && pISRObj->hISREvent)
    {
        pISRObj->threadRun=false;
        ::SetEvent(pISRObj->hISREvent);
        ::WaitForSingleObject(pISRObj->hISRThread, INFINITE);
        ::CloseHandle(pISRObj->hISRThread);
        pISRObj->hISRThread=NULL;
    }

    if(pISRObj->hISREvent)
    {
        ::CloseHandle(pISRObj->hISREvent);
        pISRObj->hISREvent=NULL;
    }

    if(pISRObj->cs.OwningThread)
    {
        ::DeleteCriticalSection(&pISRObj->cs);
    }

    pISRObj->bValid=false;

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_Win32::Enable(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return MMP_FAILURE;
    
    EnterCriticalSection(&pISRObj->cs);
    
    pISRObj->enableIrq=true;

    LeaveCriticalSection(&pISRObj->cs);
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_Win32::Disable(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return MMP_FAILURE;
    
    EnterCriticalSection(&pISRObj->cs);
    
    //::InterruptDisable(pISRObj->sysIrqID);
    pISRObj->enableIrq=false;

    LeaveCriticalSection(&pISRObj->cs);
    

    return MMP_SUCCESS;

}
 
bool CMmpOALInterrupt_Win32::IsEnable(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return false;

    return pISRObj->enableIrq;
}

MMP_RESULT CMmpOALInterrupt_Win32::Done(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
	{
        return MMP_FAILURE;
	}

    
    return MMP_SUCCESS;
}

int CMmpOALInterrupt_Win32::GetSysIrq( MMPOALISR_HANDLE hdl )
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return 0;

    return pISRObj->oalIrq;
}

MMP_RESULT CMmpOALInterrupt_Win32::ForceOccurIntrrupt( MMPOALISR_HANDLE hdl ) //강제로 인터럽트 발생시킨다.
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return MMP_FAILURE;

    ::SetEvent(pISRObj->hISREvent);
    return MMP_SUCCESS;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )


