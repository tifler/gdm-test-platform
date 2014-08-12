/****************************************************************************
 * TokiPlayer Corp.
 * Copyright (C) 2006
 *
 * All rights reserved. This file contains information that is proprietary to
 * TokiPlayer Corp. and may not be distributed or copied without
 * written consent from Mtekvison Corp.
 *
 *   $Id: Class≈€«√∏¥.hpp,v 1.1.2.1 2007/06/07 23:55:13 hwanght Exp $
 *
 */


#ifndef	__MMPOALINTERRUPT_WIN_HPP
#define	__MMPOALINTERRUPT_WIN_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpOALInterrupt.hpp"

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

struct MMP_ISR_OBJECT
{
    MMPOALINT oalIrq;
    unsigned int sysIrq;
    void (*hisr_func)(void*);
    void* hisr_param;
    
    int isr_run;
    void* event_hdl;
    void* isr_hdl;
    void* isr_func;
    void* mmpoal_intr_hdl;
};


class CMmpOALInterrupt_Win : public CMmpOALInterrupt
{
private:
   static DWORD CALLBACK UsbInterruptThreadStub( IN PVOID context );   
   DWORD UsbInterruptThread(MMP_ISR_OBJECT* pISRObj);

public:   
   CMmpOALInterrupt_Win();
   virtual ~CMmpOALInterrupt_Win();

   virtual MMP_RESULT Lock();
   virtual MMP_RESULT UnLock(); 
   virtual MMP_RESULT Enable(MMPOALISR_HANDLE hdl);
   virtual MMP_RESULT Disable(MMPOALISR_HANDLE hdl);
   virtual bool IsEnable(MMPOALISR_HANDLE hdl);
   virtual MMP_RESULT Done(MMPOALISR_HANDLE hdl);
 

   virtual MMPOALISR_HANDLE Create( MMPOALINT oalIrq, void (*callbackFunc)(void*), void* callbackParm, int sysIrq=-1  );
   virtual MMP_RESULT Destroy( MMPOALISR_HANDLE hdl  );

   virtual int  GetSysIrq( MMPOALISR_HANDLE hdl  );
};

#endif //#if (MMP_OS==MMP_OS_MMP1)

#endif // __MEDIAPLAYER_HPP

