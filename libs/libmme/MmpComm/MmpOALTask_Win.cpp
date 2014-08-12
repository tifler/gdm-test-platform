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

#include "../MmpGlobal/MmpDefine.h"
#include"MmpOALTask_Win.hpp"

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)

CMmpOALTask_Win::CMmpOALTask_Win()
{

}

CMmpOALTask_Win::~CMmpOALTask_Win()
{

}

MMPOALTASK_HANDLE  CMmpOALTask_Win::Create( void (*TaskHandler)( void* ),
                              void* param,
                              int stackSize,
                              int priority,
                              char* taskName,
                              int mode )
{
    HANDLE hThread;

    hThread=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TaskHandler, param, 0, NULL);

#if (MMP_OS==MMP_OS_WINCE60)

/*
    int cePriority;

    // 1~15 divice
    if(priority<=5)      cePriority=180;
    else if(priority<=7) cePriority=200;
    else if(priority<=10) cePriority=210;
    else if(priority<=11) cePriority=220;
    else if(priority<=12) cePriority=230;
    else if(priority<=13) cePriority=240;
    else cePriority=255;
   
    CeSetThreadPriority(hThread, cePriority);
    */

    CeSetThreadPriority(hThread, priority);
#else
    priority-=(256-8);
    if(priority<0) priority=0;
    if(priority>7) priority=7;
    SetThreadPriority(hThread, priority);
#endif

    if(mode==0)
    {
        SuspendThread(hThread);
    }

   return (MMPOALTASK_HANDLE)hThread;
}


MMP_RESULT CMmpOALTask_Win::Destroy(  MMPOALTASK_HANDLE mtvOALTask_Handle )
{
    HANDLE hThread;

    hThread=(HANDLE)mtvOALTask_Handle;
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALTask_Win::Suspend(MMPOALTASK_HANDLE mtvOALTask_Handle)
{
    HANDLE hThread;

    hThread=(HANDLE)mtvOALTask_Handle;
    SuspendThread(hThread);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALTask_Win::Resume(MMPOALTASK_HANDLE mtvOALTask_Handle)
{
    HANDLE hThread;

    hThread=(HANDLE)mtvOALTask_Handle;
    ResumeThread(hThread);

    return MMP_SUCCESS;
}


unsigned int CMmpOALTask_Win::GetCurrentThreadPriority()
{
#if (MMP_OS==MMP_OS_WINCE60)
    return ::CeGetThreadPriority(::GetCurrentThread());
#else
    return ::GetThreadPriority(::GetCurrentThread());
#endif

}

MMP_RESULT CMmpOALTask_Win::Sleep( int mileSec )
{
    ::Sleep(mileSec);
   return MMP_SUCCESS;
}

MMP_RESULT CMmpOALTask_Win::uSleepWithLoop( int usec ) //Sleep with for loop unit : micro second
{
   volatile int i;
   
   for (i=0; i < 33*usec; i++) 
	{
		//__asm {nop};
		//''__asm {nop};
	}

   return MMP_SUCCESS;
}

#endif //#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 )


