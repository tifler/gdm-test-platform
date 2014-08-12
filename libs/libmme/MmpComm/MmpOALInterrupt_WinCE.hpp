/****************************************************************************
 * TokiPlayer Corp.
 * Copyright (C) 2006
 *
 * All rights reserved. This file contains information that is proprietary to
 * TokiPlayer Corp. and may not be distributed or copied without
 * written consent from Mtekvison Corp.
 *
 *   $Id: Class템플릿.hpp,v 1.1.2.1 2007/06/07 23:55:13 hwanght Exp $
 *
 */


#ifndef	__MMPOALINTERRUPT_WINCE_HPP
#define	__MMPOALINTERRUPT_WINCE_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpOALInterrupt.hpp"

#if (MMP_OS==MMP_OS_WINCE60 )


class CMmpOALInterrupt_WinCE : public CMmpOALInterrupt
{
private:
   
	
public:   
   CMmpOALInterrupt_WinCE();
   virtual ~CMmpOALInterrupt_WinCE();

   virtual MMP_RESULT Lock();
   virtual MMP_RESULT UnLock(); 
   virtual MMP_RESULT Enable(MMPOALISR_HANDLE hdl);
   virtual MMP_RESULT Disable(MMPOALISR_HANDLE hdl);
   virtual bool IsEnable(MMPOALISR_HANDLE hdl);
   virtual MMP_RESULT Done(MMPOALISR_HANDLE hdl);
 

    virtual MMPOALISR_HANDLE Create(MMPOALINT oalIrq, void (*callbackFunc)(void*), void* callbackParm, int sysIrq=-1  );
    virtual MMPOALISR_HANDLE Create(MMPOALINT oalIrq);
    virtual MMPOALISR_HANDLE CreateGpio( int iGpioNum, void (*callbackFunc)(void*), void* callbackParm, int threadPriority, int sysIrq  );
    virtual MMPOALISR_HANDLE CreateDma( int iDmaCh, void (*callbackFunc)(void*), void* callbackParm, int sysIrq=-1  );
    virtual MMP_RESULT Destroy( MMPOALISR_HANDLE hdl  );
    virtual void SetISRThreadPriority( MMPOALISR_HANDLE hdl, int priority  );
    virtual int  GetSysIrq( MMPOALISR_HANDLE hdl  );
    virtual MMP_RESULT ForceOccurIntrrupt( MMPOALISR_HANDLE  ); //강제로 인터럽트 발생시킨다.

    virtual unsigned int  GetISRProcTick(MMPOALISR_HANDLE hdl);
    virtual void   ResetISRProcTick(MMPOALISR_HANDLE hdl);

    virtual MMP_RESULT Wait(MMPOALISR_HANDLE hdl, int timeOutMileSec=-1 );
};

#endif //#if (MMP_OS==MMP_OS_MMP1)

#endif // __MEDIAPLAYER_HPP

