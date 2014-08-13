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

#include "MmpOAL.hpp"

#include "MmpOALTask_Win.hpp"
#include "MmpOALTask_Linux.hpp"
#include "MmpOALSemaphore_Win32.hpp"
#include "MmpOALSemaphore_Linux.hpp"
#include "MmpOALCriticalSection_Win.hpp"
#include "MmpOALCriticalSection_Linux.hpp"

#include "MmpUtil.hpp"

////////////////////////////////////////////////
//CMmpOAL Singleton Pattern 

CMmpOAL* CMmpOAL::m_pMmpOAL=NULL;
unsigned int CMmpOAL::m_nRefCount=0;

MMP_RESULT CMmpOAL::CreateInstance()
{
   MMP_RESULT mmpResult;

   if( CMmpOAL::m_pMmpOAL )
   {
       CMmpOAL::m_nRefCount++;
       MMPDEBUGMSG( MMPZONE_MONITOR, (TEXT("[CMmpOAL::CreateInstance] RefCount:%d pMmpOAL:0x%08x \n\r"), CMmpOAL::m_nRefCount, CMmpOAL::m_pMmpOAL ));
       return MMP_SUCCESS;
   }
   
   CMmpOAL::m_pMmpOAL=new CMmpOAL;
   mmpResult=CMmpOAL::m_pMmpOAL->Open();
   if(mmpResult!=MMP_SUCCESS)
   {
      delete CMmpOAL::m_pMmpOAL;
      CMmpOAL::m_pMmpOAL=NULL;
      MMPDEBUGMSG( MMPZONE_MONITOR, (TEXT("[CMmpOAL::CreateInstance] FAIL -  RefCount:%d \n\r"), CMmpOAL::m_nRefCount ));
      return mmpResult;
   }
   
   CMmpOAL::m_nRefCount++;
   //MMPDEBUGMSG( MMPZONE_MONITOR, (TEXT("[CMmpOAL::CreateInstance-CreateObj] RefCount:%d pMmpOAL:0x%08x \n\r"), CMmpOAL::m_nRefCount, CMmpOAL::m_pMmpOAL ));

   return MMP_SUCCESS;
}

MMP_RESULT CMmpOAL::DestroyInstance()
{
    if(CMmpOAL::m_nRefCount>0)
    {
       CMmpOAL::m_nRefCount--;
    }

    if(CMmpOAL::m_nRefCount==0 && CMmpOAL::m_pMmpOAL)
    {
       CMmpOAL::m_pMmpOAL->Close();
       delete CMmpOAL::m_pMmpOAL;
       CMmpOAL::m_pMmpOAL=NULL;
    }

    //MMPDEBUGMSG( MMPZONE_MONITOR, (TEXT("[CMmpOAL::DestroyInstance] RefCount:%d pMmpOAL:0x%08x \n\r"), CMmpOAL::m_nRefCount, CMmpOAL::m_pMmpOAL ));

    return MMP_SUCCESS;
}

#if 0
CMmpOAL* CMmpOAL::GetInstance()
{
   return CMmpOAL::m_pMmpOAL;
}

CMmpOALFileSystem* CMmpOAL::GetFileSystemInstance()
{
   if( CMmpOAL::m_pMmpOAL==NULL )
      return (CMmpOALFileSystem*)NULL;

   return CMmpOAL::m_pMmpOAL->m_pMmpOALFileSystem;
}
   
CMmpOALTask* CMmpOAL::GetTaskInstance()
{
   if( CMmpOAL::m_pMmpOAL==NULL )
      return (CMmpOALTask*)NULL;

   return CMmpOAL::m_pMmpOAL->m_pMmpOALTask;
}

CMmpOALSemaphore* CMmpOAL::GetSemaphoreInstance()
{
   if( CMmpOAL::m_pMmpOAL==NULL )
      return (CMmpOALSemaphore*)NULL;

   return CMmpOAL::m_pMmpOAL->m_pMmpOALSemaphore;
}

CMmpOALQueue* CMmpOAL::GetQueueInstance()
{
   if( CMmpOAL::m_pMmpOAL==NULL )
      return (CMmpOALQueue*)NULL;

   return CMmpOAL::m_pMmpOAL->m_pMmpOALQueue;
}

CMmpOALInterrupt* CMmpOAL::GetInterruptInstance()
{
   if( CMmpOAL::m_pMmpOAL==NULL )
      return (CMmpOALInterrupt*)NULL;

   return CMmpOAL::m_pMmpOAL->m_pMmpOALInterrupt;
}

CMmpOALHeap* CMmpOAL::GetHeapInstance()
{
   if( CMmpOAL::m_pMmpOAL==NULL )
      return (CMmpOALHeap*)NULL;

   return CMmpOAL::m_pMmpOAL->m_pMmpOALHeap;
}
#endif

///////////////////////////////////////////////////////////
//class CMmpOAL

CMmpOAL::CMmpOAL() :
m_pMmpOALTask(NULL),
m_pMmpOALSemaphore(NULL)
{

}

CMmpOAL::~CMmpOAL()
{
   this->Close();
}

MMP_RESULT CMmpOAL::Open()
{

//Task
#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)
   m_pMmpOALTask=new CMmpOALTask_Win;
#elif (MMP_OS==MMP_OS_LINUX)
   m_pMmpOALTask=new CMmpOALTask_Linux;
#else
#error "ERROR : Please Set MMP Platform"   
#endif
   if( m_pMmpOALTask==NULL )
      return MMP_FAILURE;


//Semaphore
#if (MMP_OS == MMP_OS_WIN32)
   m_pMmpOALSemaphore=new CMmpOALSemaphore_Win32;
#elif (MMP_OS == MMP_OS_LINUX)
   m_pMmpOALSemaphore=new CMmpOALSemaphore_Linux;
#else
#error "ERROR : Please Set MMP Platform"   
#endif
   if( m_pMmpOALTask==NULL )
      return MMP_FAILURE;

//Cs
#if (MMP_OS == MMP_OS_WIN32)
   m_pMmpOALCs=new CMmpOALCriticalSection_Win;
#elif (MMP_OS == MMP_OS_LINUX)
   m_pMmpOALCs=new CMmpOALCriticalSection_Linux;
#else
#error "ERROR : Please Set MMP Platform"   
#endif
   if( m_pMmpOALTask==NULL )
      return MMP_FAILURE;

   return MMP_SUCCESS;
}

MMP_RESULT CMmpOAL::Close()
{
    //Task
   if( m_pMmpOALTask )
   {
      delete m_pMmpOALTask;
      m_pMmpOALTask=NULL;
   }

   //Semaphore
   if( m_pMmpOALSemaphore )
   {
      delete m_pMmpOALSemaphore;
      m_pMmpOALSemaphore=NULL;
   }

   //Cs
   if( m_pMmpOALCs )
   {
      delete m_pMmpOALCs;
      m_pMmpOALCs=NULL;
   }
   
   return MMP_SUCCESS;
}


extern "C" MMP_RESULT CMmpOAL_CreateInstance() {
    return CMmpOAL::CreateInstance();
}

extern "C" MMP_RESULT CMmpOAL_DestroyInstance() {
    return CMmpOAL::DestroyInstance();
}