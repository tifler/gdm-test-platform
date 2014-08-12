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


#ifndef	__MMPOALTASK_WIN_HPP
#define	__MMPOALTASK_WIN_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpOALTask.hpp"

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)


class CMmpOALTask_Win : public CMmpOALTask
{
private:
   
	
public:   
    CMmpOALTask_Win();
    virtual ~CMmpOALTask_Win();

    virtual MMPOALTASK_HANDLE  Create( void (*TaskHandler)( void* ),
                              void* param,
                              int stackSize,
                              int priority,
                              char* taskName,
                              int mode );


    virtual MMP_RESULT Destroy(  MMPOALTASK_HANDLE mtvOALTask_Handle );
    virtual MMP_RESULT Suspend(  MMPOALTASK_HANDLE mtvOALTask_Handle );
    virtual MMP_RESULT Resume(  MMPOALTASK_HANDLE mtvOALTask_Handle );

    virtual MMP_RESULT Sleep( int mileSec );
    virtual unsigned int GetTickCount() { return GetTickCount(); }
    virtual MMP_RESULT uSleepWithLoop( int usec ); //Sleep with for loop unit : micro second

    virtual unsigned int GetCurrentThreadPriority();
};

#endif //#if (MMP_OS==MMP_OS_MMP1)

#endif // __MEDIAPLAYER_HPP

