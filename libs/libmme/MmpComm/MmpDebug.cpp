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

/*******************************************************************************
 * INCLUDE FILES                                                           
 ******************************************************************************/
#include "../MmpGlobal/MmpDefine.h"
#include "MmpDebug.hpp"
#include "MmpDebug_Win32.hpp"
#include "MmpDebug_WinCE.hpp"

CMmpDebug* CMmpDebug::m_pInstance=NULL;

MMP_RESULT CMmpDebug::CreateInstance(bool bDefaultLineFeed)
{
   MMP_RESULT mtvResult;

   if( CMmpDebug::m_pInstance )
   {
      return MMP_SUCCESS;
   }
   
#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60_APP)
    CMmpDebug::m_pInstance=new CMmpDebug_Win32(bDefaultLineFeed);
#elif (MMP_OS==MMP_OS_WINCE60 )
    CMmpDebug::m_pInstance=new CMmpDebug_WinCE(bDefaultLineFeed);
#elif (MMP_OS==MMP_OS_NUCLEUS )
    CMmpDebug::m_pInstance=new CMmpDebug_Nu(bDefaultLineFeed); 
#else
#error "ERROR : Mismatch Platform"
#endif
   mtvResult=CMmpDebug::m_pInstance->Open();
   if(mtvResult!=MMP_SUCCESS)
   {
       CMmpDebug::m_pInstance->Close();
       delete CMmpDebug::m_pInstance;
       CMmpDebug::m_pInstance=NULL;
       return mtvResult;
   }
   
   return MMP_SUCCESS;
}

MMP_RESULT CMmpDebug::DestroyInstance()
{
   if(CMmpDebug::m_pInstance==NULL)
   {
      return MMP_FAILURE;
   }

   CMmpDebug::m_pInstance->Close();
   delete CMmpDebug::m_pInstance;
   CMmpDebug::m_pInstance=NULL;

   return MMP_SUCCESS;
}

CMmpDebug* CMmpDebug::GetInstance()
{
   return CMmpDebug::m_pInstance;
}


/////////////////////////////////////////////////////////////////
//
CMmpDebug::CMmpDebug(bool bDefaultLineFeed)  :
m_bDefaultLineFeed(bDefaultLineFeed)
{
}

CMmpDebug::~CMmpDebug(void)
{
	
}
