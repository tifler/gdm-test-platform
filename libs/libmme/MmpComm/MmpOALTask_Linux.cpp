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
#include"MmpOALTask_Linux.hpp"

#if 1//(MMP_OS==MMP_OS_LINUX)

#include <pthread.h>

struct task_obj {
    pthread_t task_hdl;
};

CMmpOALTask_Linux::CMmpOALTask_Linux()
{

}

CMmpOALTask_Linux::~CMmpOALTask_Linux()
{

}

MMPOALTASK_HANDLE  CMmpOALTask_Linux::Create( void (*TaskHandler)( void* ),
                              void* param,
                              int stackSize,
                              int priority,
                              char* taskName,
                              int mode )
{

    int iret;
    struct task_obj* p_obj = NULL;

    p_obj = (struct task_obj*)malloc(sizeof(struct task_obj));
    if(p_obj != NULL) {
        memset(p_obj, 0x00, sizeof(struct task_obj) );

        iret = pthread_create(&p_obj->task_hdl, NULL, (void* (*)(void*))TaskHandler, param);
	    if(iret != 0) {

            free(p_obj);
            p_obj = NULL;
	    }
    }
	
    return (MMPOALTASK_HANDLE)p_obj;
}


MMP_RESULT CMmpOALTask_Linux::Destroy(  MMPOALTASK_HANDLE task_hdl )
{
    struct task_obj* p_obj = (struct task_obj*)task_hdl;
    void* result_t=NULL;

    if(p_obj != NULL) {
        pthread_join(p_obj->task_hdl, &result_t);
        free(p_obj);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALTask_Linux::Suspend(MMPOALTASK_HANDLE task_hdl)
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpOALTask_Linux::Resume(MMPOALTASK_HANDLE task_hdl)
{

    return MMP_SUCCESS;
}


unsigned int CMmpOALTask_Linux::GetCurrentThreadPriority()
{
    return 0;
}

MMP_RESULT CMmpOALTask_Linux::Sleep( int mileSec )
{
   return MMP_SUCCESS;
}

MMP_RESULT CMmpOALTask_Linux::uSleepWithLoop( int usec ) //Sleep with for loop unit : micro second
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


