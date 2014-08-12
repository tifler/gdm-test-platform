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


#ifndef	__MMPOALQUEUE_WIN32_HPP
#define	__MMPOALQUEUE_WIN32_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MMPOALQueue.hpp"

#if (MMP_OS==MMP_OS_WIN32 )

#include "../MmpGlobal/TemplateList.hpp"

class CMmpOALQueue_Win32 : public CMmpOALQueue
{
private:
    
	
public:   
   CMmpOALQueue_Win32();
   virtual ~CMmpOALQueue_Win32();

   virtual MMPOALQUEUE_HANDLE Create( MMPSTR name, int maxMsgCount, int msgSize );
   virtual MMP_RESULT Destroy( MMPOALQUEUE_HANDLE oalqueue_handle );
   virtual MMP_RESULT Send( MMPOALQUEUE_HANDLE oalqueue_handle, void* const pData, int dataSize);
   virtual MMP_RESULT Receive( MMPOALQUEUE_HANDLE oalqueue_handle, void* pData, int dataBufSize, int* readSize, int timeOut=-1 );

   virtual MMP_RESULT Wait(MMPOALISR_HANDLE hdl, int timeOutMileSec=-1 ) {return MMP_FAILURE;}
};

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )

#endif 

