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


#ifndef	__MMPOALINTERRUPT_HPP
#define	__MMPOALINTERRUPT_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpObject.hpp"
#include "MmpOALDef.h"

#define MMPZONE_OAL_INTR 0

#define MMPOAL_INT_GPIO_MAXCOUNT 1
#define MMPOAL_INT_DMA_MAXCOUNT 3

enum MMPOALINT
{
   MMPOAL_INT_IRQ_USB=0,
   MMPOAL_INT_IRQ_BITBLIT,
   MMPOAL_INT_IRQ_VIDEOCODEC,
   MMPOAL_INT_IRQ_GPIO,
   MMPOAL_INT_IRQ_DMA_0,
   MMPOAL_INT_IRQ_DMA_1,
   MMPOAL_INT_IRQ_DMA_2,
   MMPOAL_INT_IRQ_MAX
};

class CMmpOALInterrupt : public CMmpObject
{
protected:
    bool m_bUsedIrq[MMPOAL_INT_IRQ_MAX];
	
public:   
   CMmpOALInterrupt();
   virtual ~CMmpOALInterrupt();

   virtual MMP_RESULT Lock()=0;
   virtual MMP_RESULT UnLock()=0; 
   virtual MMP_RESULT Enable(MMPOALISR_HANDLE hdl)=0;
   virtual MMP_RESULT Disable(MMPOALISR_HANDLE hdl)=0;
   virtual MMP_RESULT Done(MMPOALISR_HANDLE hdl)=0;
   virtual bool IsEnable(MMPOALISR_HANDLE hdl)=0;
 
   virtual MMPOALISR_HANDLE Create(MMPOALINT oalIrq, void (*callbackFunc)(void*), void* callbackParm, int sysIrq=-1  )=0;
   virtual MMPOALISR_HANDLE Create(MMPOALINT oalIrq)=0;
   virtual MMPOALISR_HANDLE CreateGpio( int iGpioNum, void (*callbackFunc)(void*), void* callbackParm, int threadPriority, int sysIrq  )=0;
   virtual MMPOALISR_HANDLE CreateDma( int iDmaCh, void (*callbackFunc)(void*), void* callbackParm, int sysIrq=-1  ) {return (MMPOALISR_HANDLE)0;}
   virtual MMP_RESULT Destroy( MMPOALISR_HANDLE hdl  )=0;
   virtual void SetISRThreadPriority(MMPOALISR_HANDLE hdl, int priority  ){ UnusedParameter(hdl); UnusedParameter(priority); }

   virtual int  GetSysIrq( MMPOALISR_HANDLE hdl  )=0;
   virtual MMP_RESULT ForceOccurIntrrupt( MMPOALISR_HANDLE  ) {return MMP_FAILURE;} //강제로 인터럽트 발생시킨다.

   virtual unsigned int  GetISRProcTick( MMPOALISR_HANDLE hdl  ){return 0;}
   virtual void   ResetISRProcTick( MMPOALISR_HANDLE hdl  ) {} 

   virtual MMP_RESULT Wait(MMPOALISR_HANDLE hdl, int timeOutMileSec=-1 )=0;
   
};


#endif 

