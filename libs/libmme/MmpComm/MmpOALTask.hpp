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

#ifndef	__MMPOALTASK_HPP
#define	__MMPOALTASK_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpObject.hpp"
#include "MmpOALDef.h"

class CMmpOALTask : public CMmpObject
{
private:
	
public:   
    CMmpOALTask();
    virtual ~CMmpOALTask();

    virtual MMPOALTASK_HANDLE  Create( void (*TaskHandler)( void* ),
                              void* param,
                              int stackSize,
                              int priority,
                              char* taskName,
                              int mode )=0;

    virtual MMP_RESULT Destroy(  MMPOALTASK_HANDLE mtvOALTask_Handle )=0;

    virtual MMP_RESULT Suspend(  MMPOALTASK_HANDLE mtvOALTask_Handle )=0;
    virtual MMP_RESULT Resume(  MMPOALTASK_HANDLE mtvOALTask_Handle )=0;


    virtual MMP_RESULT Sleep( int mileSec )=0;
    virtual unsigned int GetTickCount()=0;
    virtual MMP_RESULT uSleepWithLoop( int usec )=0; //Sleep with for loop  unit : micro second

    virtual unsigned int GetCurrentThreadPriority()=0;
};


#endif 

