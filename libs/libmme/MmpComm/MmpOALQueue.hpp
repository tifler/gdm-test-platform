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


#ifndef	__MMPOALQUEUE_HPP
#define	__MMPOALQUEUE_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpObject.hpp"
#include "MmpOALDef.h"

class CMmpOALQueue : public CMmpObject
{
private:
	
public:   
   CMmpOALQueue();
   virtual ~CMmpOALQueue();

   virtual MMPOALQUEUE_HANDLE Create( MMPSTR name, int maxDataCount, int maxDataSize )=0;
   virtual MMP_RESULT Destroy( MMPOALQUEUE_HANDLE oalqueue_handle )=0;
   virtual MMP_RESULT Send( MMPOALQUEUE_HANDLE oalqueue_handle, void* const pData, int dataSize)=0;
   virtual MMP_RESULT Receive( MMPOALQUEUE_HANDLE oalqueue_handle, void* pData, int dataBufSize, int* readSize, int timeOut=-1 )=0;

};

#endif 

