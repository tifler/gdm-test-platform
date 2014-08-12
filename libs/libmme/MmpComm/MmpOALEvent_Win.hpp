/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef	__MMPOALEVENT_WIN_HPP
#define	__MMPOALEVENT_WIN_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpObject.hpp"
#include "MmpOALDef.h"
#include "MmpOALEvent.hpp"

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

class CMmpOALEvent_Win : public CMmpOALEvent
{
private:
	
public:   
   CMmpOALEvent_Win();
   virtual ~CMmpOALEvent_Win();

   virtual MMPOALEVENT_HANDLE Create(void* pObject );
   virtual MMPOALEVENT_HANDLE Create(bool bManualReset=false, bool bInitialState=false, MMPSTR pName=NULL );
   virtual MMP_RESULT Destroy( MMPOALEVENT_HANDLE event_hdl );
   virtual MMP_RESULT Set( MMPOALEVENT_HANDLE event_hdl );
   virtual MMP_RESULT Wait( MMPOALEVENT_HANDLE event_hdl, int timeOutMileSec=-1 );
   virtual void* GetObjectHandle(MMPOALEVENT_HANDLE event_hdl);

};

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

#endif 

