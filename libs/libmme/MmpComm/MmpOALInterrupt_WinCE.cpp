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
#include"MmpOALInterrupt_WinCE.hpp"

#if (MMP_OS==MMP_OS_WINCE60 && MMP_DEVCONFIG==MMP_DEVCONFIG_DRIVER)

#if (MMP_DEVCONFIG==MMP_DEVCONFIG_DRIVER)
#include <nkintr.h>
#else

//When develop wince app,  intr code is not available.
//The follow functions and definition are all dummy..

#define SYSINTR_NOP -1
#define IOCTL_HAL_REQUEST_SYSINTR 100
#define IOCTL_HAL_RELEASE_SYSINTR 1-1

VOID InterruptDone(DWORD idInt) {}
BOOL KernelIoControl( 
  DWORD dwIoControlCode, 
  LPVOID lpInBuf, 
  DWORD nInBufSize, 
  LPVOID lpOutBuf, 
  DWORD nOutBufSize, 
  LPDWORD lpBytesReturned 
  ){ return FALSE; }

BOOL InterruptInitialize( 
  DWORD idInt, 
  HANDLE hEvent, 
  LPVOID pvData, 
  DWORD cbData 
  ){return FALSE;}

VOID InterruptDisable( 
  DWORD idInt 
  ){}

#endif

///////////////////////////////////////////////////////////////////////
// OAL ISR Prop Struct

struct SOALISRPROP
{
    bool bValid;
    MMPOALINT oalIrq;
    DWORD sysIrq;
    DWORD sysIrqID; 
    bool  enableIrq;
    bool threadRun;
    HANDLE hISREvent;
    HANDLE hISRThread;
    LPTHREAD_START_ROUTINE isrThreadFunc;
    void (*callbackFunc)(void*);
    void* callbackParm;
    DWORD procTick;
    CRITICAL_SECTION cs;
    WCHAR wszName[16];
};

//////////////////////////////////////////////////////////////////////
// static array 
#if (MMP_CPU==MMP_CPU_MV8770)
#include <mv8770Reg_Intr.h>

static  DWORD SYS_IRQ_TABLE[MMPOAL_INT_IRQ_MAX]=
{
     IRQ_USB,
     0, //BitBlit
     IRQ_EINT0
};
#elif (MMP_CPU==MMP_CPU_ZEUS1 || MMP_CPU==MMP_CPU_ZEUSV)

#if (MMP_DEVCONFIG==MMP_DEVCONFIG_DRIVER)
#include <Zeus1Reg_Intr.h>
#else
#define IRQ_USBOTG 0
#define IRQ_ICB 0
#define IRQ_DVC 0
#define IRQ_EINT0   0
#define IRQ_DMA0   0
#endif

static  DWORD SYS_IRQ_TABLE[MMPOAL_INT_IRQ_MAX]=
{
     IRQ_USBOTG,
     IRQ_ICB,
     IRQ_DVC,
     IRQ_EINT0,
     IRQ_DMA0,  //DMA_0
     IRQ_DMA0,  //DMA_1
     IRQ_DMA0,  //DMA_2
};

#else
#error "ERROR : Select CPU in MmpOALInterurpt_WinCE.cpp "
#endif

////////////////////////////////////////////////////////////////////
// ISR Property Array
static void MMPOAL_ISR_USB_THREAD(void* param);
static void MMPOAL_ISR_GPIO_THREAD(void* param);
static void MMPOAL_ISR_BITBLIT_THREAD(void* param);
static void MMPOAL_ISR_VIDEOCODEC_THREAD(void* param);

static void MMPOAL_ISR_DMA_THREAD(void* param);

static SOALISRPROP s_ISRObj[MMPOAL_INT_IRQ_MAX]=
{
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_USB, //MMPOALINT oalIrq;
        SYS_IRQ_TABLE[MMPOAL_INT_IRQ_USB], //unsigned sysIrq;
        SYSINTR_NOP,               //DWORD sysIrqID
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_USB_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        0, //ProcTick
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        TEXT("USB")
   },
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_BITBLIT, //MMPOALINT oalIrq;
        SYS_IRQ_TABLE[MMPOAL_INT_IRQ_BITBLIT], //unsigned sysIrq;
        SYSINTR_NOP,               //DWORD sysIrqID
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_BITBLIT_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        0,  //ProcTick
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        TEXT("BITBLIT")
   },
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_VIDEOCODEC, //MMPOALINT oalIrq;
        SYS_IRQ_TABLE[MMPOAL_INT_IRQ_VIDEOCODEC], //unsigned sysIrq;
        SYSINTR_NOP,               //DWORD sysIrqID
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_VIDEOCODEC_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        0,  //ProcTick
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        TEXT("VideoCodec")
   },
   //MMPOAL_INT_IRQ_GPIO
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_GPIO, //MMPOALINT oalIrq;
        SYS_IRQ_TABLE[MMPOAL_INT_IRQ_GPIO], //unsigned sysIrq;
        SYSINTR_NOP,               //DWORD sysIrqID
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_GPIO_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        0,  //ProcTick
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        TEXT("GPIO")
   },
   //MMPOAL_INT_IRQ_DMA_0
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_DMA_0, //MMPOALINT oalIrq;
        SYS_IRQ_TABLE[MMPOAL_INT_IRQ_DMA_0], //unsigned sysIrq;
        SYSINTR_NOP,               //DWORD sysIrqID
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_DMA_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        0,  //ProcTick
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        TEXT("DMA_0")
   },
    //MMPOAL_INT_IRQ_DMA_1
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_DMA_1, //MMPOALINT oalIrq;
        SYS_IRQ_TABLE[MMPOAL_INT_IRQ_DMA_1], //unsigned sysIrq;
        SYSINTR_NOP,               //DWORD sysIrqID
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_DMA_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        0,  //ProcTick
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        TEXT("DMA_1")
   },
   //MMPOAL_INT_IRQ_DMA_2
   {
        false,              //bool bValid;
        MMPOAL_INT_IRQ_DMA_2, //MMPOALINT oalIrq;
        SYS_IRQ_TABLE[MMPOAL_INT_IRQ_DMA_2], //unsigned sysIrq;
        SYSINTR_NOP,               //DWORD sysIrqID
        false,           // bool enableIrq;
        false,           // bool threadRun
        NULL,           // HANDLE hISREvent
        NULL,           // HANDLE hISRThread
        (LPTHREAD_START_ROUTINE)MMPOAL_ISR_DMA_THREAD,  //LPTHREAD_START_ROUTINE isrThreadFunc;
        0,               //void* callbackFunc;
        0,              //void* callbackParm;
        0,  //ProcTick
        {0,0,0,0,0},   //CRITICAL_SECTION cs;
        TEXT("DMA_2")
   }
};

///////////////////////////////////////////////////////////////////////
// ISR Thread
static void MMPOAL_ISR_USB_THREAD(void* param)
{
    SOALISRPROP* pISRObj=(SOALISRPROP*)param;
    DWORD startTick, endTick;
    
    MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_USB_THREAD] USB ISR Thread Start..\n\r")));

    pISRObj->procTick=0;
    while(1)
    {
        WaitForSingleObject(pISRObj->hISREvent, INFINITE);
        if(!pISRObj->threadRun)
            break;

        startTick=GetTickCount();
        EnterCriticalSection(&pISRObj->cs);
        if( pISRObj->enableIrq)
        {
    
            //MMPDEBUGMSG(ZONE_MMP, (TEXT("[MMPOAL_ISR_USB_THREAD] USB Interrupt Occure\n\r")));
            pISRObj->callbackFunc( pISRObj->callbackParm );
            ::InterruptDone(pISRObj->sysIrqID);
        }
        LeaveCriticalSection(&pISRObj->cs);
        endTick=GetTickCount();
        pISRObj->procTick+=endTick-startTick;
    }
    MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_USB_THREAD] USB ISR Thread Ended! \n\r")));
}

static void MMPOAL_ISR_BITBLIT_THREAD(void* param)
{
    SOALISRPROP* pISRObj=(SOALISRPROP*)param;
    DWORD startTick, endTick;
    
    //MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] USB ISR Thread Start..\n\r")));
    MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] BitBlit ISR Thread Start..\n\r")));

    pISRObj->procTick=0;
    while(1)
    {
        WaitForSingleObject(pISRObj->hISREvent, INFINITE);
        if(!pISRObj->threadRun)
        {
            break;
        }

        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] 1 \n\r")));

        startTick=GetTickCount();
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] 2 \n\r")));
        EnterCriticalSection(&pISRObj->cs);
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] 3 \n\r")));
        if( pISRObj->enableIrq)
        {
    
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] BitBlit Interrupt Occure\n\r")));
            pISRObj->callbackFunc( pISRObj->callbackParm );
            ::InterruptDone(pISRObj->sysIrqID);
        }
        LeaveCriticalSection(&pISRObj->cs);
        endTick=GetTickCount();
        pISRObj->procTick+=endTick-startTick;
    }
    //MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] USB ISR Thread Ended! \n\r")));
    MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] BitBlit ISR Thread Ended! \n\r")));
}

static void MMPOAL_ISR_VIDEOCODEC_THREAD(void* param)
{
    SOALISRPROP* pISRObj=(SOALISRPROP*)param;
    DWORD startTick, endTick;
    
    MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] USB ISR Thread Start..\n\r")));
    //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] VideoCodec ISR Thread Start..\n\r")));

    pISRObj->procTick=0;
    while(1)
    {
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] 0 \n\r")));
        WaitForSingleObject(pISRObj->hISREvent, INFINITE);
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] A 0x%x %d \n\r"), pISRObj, pISRObj->threadRun));
        if(!pISRObj->threadRun)
        {
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] B \n\r")));
            break;
        }

        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] 1 \n\r")));
        startTick=GetTickCount();
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] 2 \n\r")));
        EnterCriticalSection(&pISRObj->cs);
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] 3 \n\r")));
        if( pISRObj->enableIrq)
        {
           //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] Davinci Interrupt Occure\n\r")));
            pISRObj->callbackFunc( pISRObj->callbackParm );
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] 4 \n\r")));
            ::InterruptDone(pISRObj->sysIrqID);
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] 5 \n\r")));
        }
        LeaveCriticalSection(&pISRObj->cs);
        endTick=GetTickCount();
        pISRObj->procTick+=endTick-startTick;
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] 4 \n\r")));
    }
    MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_BITBLIT_THREAD] USB ISR Thread Ended! \n\r")));
    //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] VideoCodec ISR Thread Ended! \n\r")));
}

static void MMPOAL_ISR_GPIO_THREAD(void* param)
{
    SOALISRPROP* pISRObj=(SOALISRPROP*)param;
    DWORD startTick, endTick;
    DWORD isrCnt, totalTick;
    
    //MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_GPIO_THREAD] %s ISR Thread Start..\n\r"), pISRObj->wszName));
    MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_GPIO_THREAD (0x%08x) ] %s (%d) ISR Thread Start..\n\r"), 
            pISRObj, pISRObj->wszName, pISRObj->sysIrq-IRQ_EINT0));

    totalTick=0;
    isrCnt=1;
    pISRObj->procTick=0;
    while(1)
    {
        //MMPDEBUGMSG(1, (TEXT("ISR(0) \n\r")));
        WaitForSingleObject(pISRObj->hISREvent, INFINITE);
        if(!pISRObj->threadRun)
        {
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_GPIO_THREAD] B \n\r")));
            break;
        }
        //MMPDEBUGMSG(1, (TEXT("ISR(1) ")));

        startTick=GetTickCount();
        EnterCriticalSection(&pISRObj->cs);
        if( pISRObj->enableIrq)
        {
            //MMPDEBUGMSG(1, (TEXT("ISR(2) ")));
            //MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_USB_THREAD] %s Interrupt Occure\n\r"), pISRObj->wszName));
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_GPIO_THREAD] %s Interrupt Occure  GpioNum: %d  pISRObj(0x%08x) CallbackFunc:0x%08x\n\r"), 
            //    pISRObj->wszName, pISRObj->sysIrq-IRQ_EINT0, pISRObj, pISRObj->callbackFunc ));

            
            //MMPDEBUGMSG(1, (TEXT("ISR(3) ")));

            ::InterruptDone(pISRObj->sysIrqID);
             pISRObj->callbackFunc( pISRObj->callbackParm );

           
            //MMPDEBUGMSG(1, (TEXT("ISR(4) ")));
        }
        else
        {
            ::InterruptDone(pISRObj->sysIrqID);
        }
        LeaveCriticalSection(&pISRObj->cs);

        //MMPDEBUGMSG(1, (TEXT("ISR(5) ")));

        endTick=GetTickCount();

        totalTick+=endTick-startTick;
        pISRObj->procTick=totalTick/isrCnt;
        isrCnt++;
    }
    MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_GPIO_THREAD] %s ISR Thread Ended! \n\r"), pISRObj->wszName));
}


static void MMPOAL_ISR_DMA_THREAD(void* param)
{
    SOALISRPROP* pISRObj=(SOALISRPROP*)param;
    DWORD startTick, endTick;
    DWORD isrCnt, totalTick;
    
    //MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_DMA_THREAD] %s ISR Thread Start..\n\r"), pISRObj->wszName));
    MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_DMA_THREAD (0x%08x) ] %s (%d) ISR Thread Start..\n\r"), 
            pISRObj, pISRObj->wszName, pISRObj->sysIrq-IRQ_DMA0));

    totalTick=0;
    isrCnt=1;
    pISRObj->procTick=0;
    while(1)
    {
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_DMA_THREAD] A \n\r")));
        WaitForSingleObject(pISRObj->hISREvent, INFINITE);
        if(!pISRObj->threadRun)
        {
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_DMA_THREAD] B \n\r")));
            break;
        }
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_DMA_THREAD] C \n\r")));

        startTick=GetTickCount();
        EnterCriticalSection(&pISRObj->cs);
        if( pISRObj->enableIrq)
        {
            //MMPDEBUGMSG(1, (TEXT("G ")));
            //MMPDEBUGMSG(MMPZONE_ISR, (TEXT("[MMPOAL_ISR_DMA_0__THREAD] %s Interrupt Occure\n\r"), pISRObj->wszName));
            //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_DMA_THREAD] %s Interrupt Occure  GpioNum: %d  pISRObj(0x%08x) CallbackFunc:0x%08x\n\r"), 
            //    pISRObj->wszName, pISRObj->sysIrq-IRQ_DMA0, pISRObj, pISRObj->callbackFunc ));

            pISRObj->callbackFunc( pISRObj->callbackParm );

            ::InterruptDone(pISRObj->sysIrqID);
            //pISRObj->callbackFunc( pISRObj->callbackParm );
        }
        else
        {
            ::InterruptDone(pISRObj->sysIrqID);
        }
        LeaveCriticalSection(&pISRObj->cs);
        endTick=GetTickCount();

        totalTick+=endTick-startTick;
        pISRObj->procTick=totalTick/isrCnt;
        isrCnt++;
    }
    MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_DMA_THREAD] %s ISR Thread Ended! \n\r"), pISRObj->wszName));
}

/////////////////////////////////////////////////////////////////////
// class Member

CMmpOALInterrupt_WinCE::CMmpOALInterrupt_WinCE()
{

}

CMmpOALInterrupt_WinCE::~CMmpOALInterrupt_WinCE()
{

}

MMP_RESULT CMmpOALInterrupt_WinCE::Lock()
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_WinCE::UnLock()
{
    return MMP_SUCCESS;
}


MMPOALISR_HANDLE CMmpOALInterrupt_WinCE::Create(MMPOALINT oalIrq)
{
    SOALISRPROP* pISRObj=NULL;
    
    if(oalIrq<0 || oalIrq>=MMPOAL_INT_IRQ_MAX)
       return (MMPOALISR_HANDLE)NULL;

    pISRObj=&s_ISRObj[oalIrq];
    if(pISRObj->bValid==true)
        return (MMPOALISR_HANDLE)NULL;

    pISRObj->callbackFunc=NULL;
    pISRObj->callbackParm=NULL;
    pISRObj->sysIrqID=SYSINTR_NOP;
    pISRObj->hISRThread=NULL;
    pISRObj->hISREvent=NULL;
    pISRObj->threadRun=false;
    pISRObj->enableIrq=false;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &pISRObj->sysIrq, sizeof(UINT32), &pISRObj->sysIrqID, sizeof(DWORD), NULL))
    {
        //CApDebug::GetInstance()->Printf("[CApUtil_ISR::Open] FAIL : KernelIoControl IOCTL_HAL_REQUEST_SYSINTR sysIrq: 0x%x \r\n", sysIrq);
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d) ERROR : KernelIOCtrl(IOCTL_HAL_REQUEST_SYSINTR..) \n\r"), oalIrq));
        pISRObj->sysIrqID=SYSINTR_NOP;
        goto action_fail;
    }

    pISRObj->hISREvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if( !pISRObj->hISREvent )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateEvent \n\r"), oalIrq));
        goto action_fail;
	}

    if(!InterruptInitialize( pISRObj->sysIrqID, pISRObj->hISREvent, NULL, NULL) ) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : InterruptInitialize \n\r"), oalIrq));
        goto action_fail;
    }

    InitializeCriticalSection(&pISRObj->cs);
    
  
    pISRObj->threadRun=false;
    pISRObj->hISRThread=NULL;
    
    pISRObj->bValid=true;

    return (MMPOALISR_HANDLE)pISRObj;

action_fail:

    this->Destroy((MMPOALISR_HANDLE)pISRObj);

    return (MMPOALISR_HANDLE)NULL;
}


MMPOALISR_HANDLE CMmpOALInterrupt_WinCE::Create( MMPOALINT oalIrq, void (*callbackFunc)(void*), void* callbackParm, int sysIrq  )
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
    pISRObj->sysIrqID=SYSINTR_NOP;
    pISRObj->hISRThread=NULL;
    pISRObj->hISREvent=NULL;
    pISRObj->threadRun=false;
    pISRObj->enableIrq=false;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &pISRObj->sysIrq, sizeof(UINT32), &pISRObj->sysIrqID, sizeof(DWORD), NULL))
    {
        //CApDebug::GetInstance()->Printf("[CApUtil_ISR::Open] FAIL : KernelIoControl IOCTL_HAL_REQUEST_SYSINTR sysIrq: 0x%x \r\n", sysIrq);
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d) ERROR : KernelIOCtrl(IOCTL_HAL_REQUEST_SYSINTR..) \n\r"), oalIrq));
        pISRObj->sysIrqID=SYSINTR_NOP;
        goto action_fail;
    }

    pISRObj->hISREvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if( !pISRObj->hISREvent )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateEvent \n\r"), oalIrq));
        goto action_fail;
	}

    if(!InterruptInitialize( pISRObj->sysIrqID, pISRObj->hISREvent, NULL, NULL) ) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : InterruptInitialize \n\r"), oalIrq));
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
    CeSetThreadPriority( pISRObj->hISRThread, 100 );
    //CeSetThreadPriority( pISRObj->hISRThread, 210 );

    pISRObj->bValid=true;

    return (MMPOALISR_HANDLE)pISRObj;

action_fail:

    this->Destroy((MMPOALISR_HANDLE)pISRObj);

    return (MMPOALISR_HANDLE)NULL;
}


MMPOALISR_HANDLE CMmpOALInterrupt_WinCE::CreateGpio( int iGpioNum, void (*callbackFunc)(void*), void* callbackParm, int threadPriority, int sysIrq  )
{
    SOALISRPROP* pISRObj=NULL;
    MMPOALINT oalIrq;
        
    oalIrq=MMPOAL_INT_IRQ_GPIO;

    pISRObj=&s_ISRObj[oalIrq];
    if(pISRObj->bValid==true)
        return (MMPOALISR_HANDLE)NULL;

    if(callbackFunc==0)
        return (MMPOALISR_HANDLE)NULL;

    MMPDEBUGMSG( 1, (TEXT("ISR Obj 0x%08x 0x%08x \n\r"), s_ISRObj, &s_ISRObj[1] ));

    pISRObj->sysIrq=SYS_IRQ_TABLE[pISRObj->oalIrq]+iGpioNum;
    pISRObj->callbackFunc=callbackFunc;
    pISRObj->callbackParm=callbackParm;
    pISRObj->sysIrqID=SYSINTR_NOP;
    pISRObj->hISRThread=NULL;
    pISRObj->hISREvent=NULL;
    pISRObj->threadRun=false;
    pISRObj->enableIrq=false;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &pISRObj->sysIrq, sizeof(UINT32), &pISRObj->sysIrqID, sizeof(DWORD), NULL))
    {
        //CApDebug::GetInstance()->Printf("[CApUtil_ISR::Open] FAIL : KernelIoControl IOCTL_HAL_REQUEST_SYSINTR sysIrq: 0x%x \r\n", sysIrq);
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d) ERROR : KernelIOCtrl(IOCTL_HAL_REQUEST_SYSINTR..) \n\r"), oalIrq));
        pISRObj->sysIrqID=SYSINTR_NOP;
        goto action_fail;
    }

    pISRObj->hISREvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if( !pISRObj->hISREvent )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateEvent \n\r"), oalIrq));
        goto action_fail;
	}

    if(!InterruptInitialize( pISRObj->sysIrqID, pISRObj->hISREvent, NULL, NULL) ) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : InterruptInitialize \n\r"), oalIrq));
        goto action_fail;
    }

    InitializeCriticalSection(&pISRObj->cs);
    
    MMPDEBUGMSG( 1, (TEXT("ISR CreateThread 0x%08x 0x%08x \n\r"), pISRObj, pISRObj->callbackFunc ));
    pISRObj->threadRun=true;
    pISRObj->hISRThread=CreateThread(NULL, 0, pISRObj->isrThreadFunc, pISRObj, 0, NULL);
    if( !pISRObj->hISRThread )
    {
        pISRObj->hISRThread=false;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateThread \n\r"), oalIrq));
        goto action_fail;
    }
    CeSetThreadPriority( pISRObj->hISRThread, threadPriority );
    
    pISRObj->bValid=true;

    return (MMPOALISR_HANDLE)pISRObj;

action_fail:

    this->Destroy((MMPOALISR_HANDLE)pISRObj);

    return (MMPOALISR_HANDLE)NULL;
}


MMPOALISR_HANDLE CMmpOALInterrupt_WinCE::CreateDma( int iDmaCh, void (*callbackFunc)(void*), void* callbackParm, int sysIrq  )
{
    SOALISRPROP* pISRObj=NULL;
    MMPOALINT oalIrq;
    int i,j;
    
MMPDEBUGMSG(1, (TEXT("A0 ")));
    for(i=0, j=(int)MMPOAL_INT_IRQ_DMA_0; i<MMPOAL_INT_DMA_MAXCOUNT;i++, j++)
    {
        oalIrq=(MMPOALINT)j;
        pISRObj=&s_ISRObj[oalIrq];
        if(pISRObj->bValid==false)
        {
            break;
        }
    }
MMPDEBUGMSG(1, (TEXT("A1 ")));
    if(i==MMPOAL_INT_DMA_MAXCOUNT)
    {
        return (MMPOALISR_HANDLE)NULL;
    }

    if(callbackFunc==0)
        return (MMPOALISR_HANDLE)NULL;

    MMPDEBUGMSG( 1, (TEXT("ISR Obj 0x%08x 0x%08x \n\r"), s_ISRObj, &s_ISRObj[1] ));

    pISRObj->sysIrq+=iDmaCh;
    pISRObj->callbackFunc=callbackFunc;
    pISRObj->callbackParm=callbackParm;
    pISRObj->sysIrqID=SYSINTR_NOP;
    pISRObj->hISRThread=NULL;
    pISRObj->hISREvent=NULL;
    pISRObj->threadRun=false;
    pISRObj->enableIrq=false;

MMPDEBUGMSG(1, (TEXT("A3 ")));

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &pISRObj->sysIrq, sizeof(UINT32), &pISRObj->sysIrqID, sizeof(DWORD), NULL))
    {
        //CApDebug::GetInstance()->Printf("[CApUtil_ISR::Open] FAIL : KernelIoControl IOCTL_HAL_REQUEST_SYSINTR sysIrq: 0x%x \r\n", sysIrq);
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d) ERROR : KernelIOCtrl(IOCTL_HAL_REQUEST_SYSINTR..) \n\r"), oalIrq));
        pISRObj->sysIrqID=SYSINTR_NOP;
        goto action_fail;
    }

    pISRObj->hISREvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if( !pISRObj->hISREvent )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateEvent \n\r"), oalIrq));
        goto action_fail;
	}

    if(!InterruptInitialize( pISRObj->sysIrqID, pISRObj->hISREvent, NULL, NULL) ) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : InterruptInitialize \n\r"), oalIrq));
        goto action_fail;
    }

    InitializeCriticalSection(&pISRObj->cs);
    
    MMPDEBUGMSG( 1, (TEXT("ISR CreateThread 0x%08x 0x%08x \n\r"), pISRObj, pISRObj->callbackFunc ));
    pISRObj->threadRun=true;
    pISRObj->hISRThread=CreateThread(NULL, 0, pISRObj->isrThreadFunc, pISRObj, 0, NULL);
    if( !pISRObj->hISRThread )
    {
        pISRObj->hISRThread=false;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : CreateThread \n\r"), oalIrq));
        goto action_fail;
    }
    CeSetThreadPriority( pISRObj->hISRThread, 150 );
    //CeSetThreadPriority( pISRObj->hISRThread, 210 );

    pISRObj->bValid=true;

    return (MMPOALISR_HANDLE)pISRObj;

action_fail:

    this->Destroy((MMPOALISR_HANDLE)pISRObj);

    return (MMPOALISR_HANDLE)NULL;
}

MMP_RESULT CMmpOALInterrupt_WinCE::Destroy( MMPOALISR_HANDLE hdl )
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return MMP_FAILURE;
    
    pISRObj->threadRun=false;

    if(pISRObj->sysIrqID!=SYSINTR_NOP)
    {
        
    //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] Destroy B \n\r")));
        ::InterruptDisable(pISRObj->sysIrqID);
    }

    //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] Destroy C \n\r")));

    if(pISRObj->hISRThread && pISRObj->hISREvent)
    {
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] Destroy 1 0x%x \n\r"), pISRObj));
        pISRObj->threadRun=false;
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] Destroy 2 %d\n\r"), pISRObj->threadRun));
        ::SetEvent(pISRObj->hISREvent);
        //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] Destroy 3 \n\r")));
        ::WaitForSingleObject(pISRObj->hISRThread, INFINITE);
        ::CloseHandle(pISRObj->hISRThread);
        pISRObj->hISRThread=NULL;
    }

    //MMPDEBUGMSG(1, (TEXT("[MMPOAL_ISR_VIDEOCODEC_THREAD] Destroy D \n\r")));

    if(pISRObj->hISREvent)
    {
        ::CloseHandle(pISRObj->hISREvent);
        pISRObj->hISREvent=NULL;
    }

    if(pISRObj->cs.OwnerThread)
    {
        ::DeleteCriticalSection(&pISRObj->cs);
    }

    if(pISRObj->sysIrqID!=SYSINTR_NOP)
    {
        if(!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pISRObj->sysIrqID, sizeof(UINT32), NULL, NULL, NULL))
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpOALInterrupt_WInCE::Create] OalIRQ(%d)  ERROR : KernelIOCtrl(IOCTL_HAL_RELEASE_SYSINTR..) \n\r"), pISRObj->oalIrq));
        }
        pISRObj->sysIrqID=SYSINTR_NOP;
    }

    pISRObj->bValid=false;
    pISRObj->sysIrq=SYS_IRQ_TABLE[pISRObj->oalIrq];

    return MMP_SUCCESS;
}

void CMmpOALInterrupt_WinCE::SetISRThreadPriority( MMPOALISR_HANDLE hdl, int priority  )
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return;
    
    int cePriority;

    // 1~15 divice
    if(priority<=5)      cePriority=180;
    else if(priority<=7) cePriority=200;
    else if(priority<=10) cePriority=210;
    else if(priority<=11) cePriority=220;
    else if(priority<=12) cePriority=230;
    else if(priority<=13) cePriority=240;
    else cePriority=255;
   
    CeSetThreadPriority(pISRObj->hISRThread, cePriority);
}

MMP_RESULT CMmpOALInterrupt_WinCE::Enable(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return MMP_FAILURE;
    
    //if(!InterruptInitialize( pISRObj->sysIrqID, pISRObj->hISREvent, NULL, NULL) ) 
    //{
    //    return MMP_FAILURE;
    //}

    EnterCriticalSection(&pISRObj->cs);
    
    ::InterruptDone(s_ISRObj->sysIrqID);
    pISRObj->enableIrq=true;

    LeaveCriticalSection(&pISRObj->cs);
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALInterrupt_WinCE::Disable(MMPOALISR_HANDLE hdl)
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
 
bool CMmpOALInterrupt_WinCE::IsEnable(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return false;

    return pISRObj->enableIrq;
}

MMP_RESULT CMmpOALInterrupt_WinCE::Done(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return false;

    ::InterruptDone(pISRObj->sysIrqID);

    return MMP_SUCCESS;
}

int CMmpOALInterrupt_WinCE::GetSysIrq( MMPOALISR_HANDLE hdl )
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return false;

    return pISRObj->sysIrqID;
}

MMP_RESULT CMmpOALInterrupt_WinCE::ForceOccurIntrrupt( MMPOALISR_HANDLE hdl ) //강제로 인터럽트 발생시킨다.
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return MMP_FAILURE;

    ::SetEvent(pISRObj->hISREvent);
    return MMP_SUCCESS;
}

unsigned int  CMmpOALInterrupt_WinCE::GetISRProcTick(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
        return 0;

    return pISRObj->procTick;
}
    
void   CMmpOALInterrupt_WinCE::ResetISRProcTick(MMPOALISR_HANDLE hdl)
{
    SOALISRPROP* pISRObj;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(pISRObj)
    {
        pISRObj->procTick=0;
    }
}

MMP_RESULT  CMmpOALInterrupt_WinCE::Wait(MMPOALISR_HANDLE hdl, int timeOutMileSec)
{
    SOALISRPROP* pISRObj;
    DWORD timeOut, res;
    MMP_RESULT mmpResult=MMP_FAILURE;
    
    pISRObj=(SOALISRPROP*)hdl;
    if(!pISRObj)
    {
        return mmpResult;
    }

    if(timeOutMileSec==-1)
        timeOut=INFINITE;
    else
        timeOut=(DWORD)timeOutMileSec;

    res=WaitForSingleObject(pISRObj->hISREvent, timeOut);
    if(res==WAIT_OBJECT_0)
    {
        mmpResult=MMP_SUCCESS;
    }
    
    return mmpResult;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )



