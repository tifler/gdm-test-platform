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

#ifndef	__MMPOALSEMAPHORE_LINUX_HPP
#define	__MMPOALSEMAPHORE_LINUX_HPP

#include "MmpDefine.h"
#include "MmpOALSemaphore.hpp"

#if (MMP_OS == MMP_OS_LINUX)

class CMmpOALSemaphore_Linux : public CMmpOALSemaphore
{
private:
	
public:   
   CMmpOALSemaphore_Linux();
   virtual ~CMmpOALSemaphore_Linux();

   virtual MMPOALSEMAPHORE_HANDLE Create( char* name, int mode, int count );
   virtual MMP_RESULT Destroy( MMPOALSEMAPHORE_HANDLE mtvoalsema_handle );
   virtual MMP_RESULT Obtain( MMPOALSEMAPHORE_HANDLE mtvoalsema_handle );
   virtual MMP_RESULT Release( MMPOALSEMAPHORE_HANDLE mtvoalsema_handle );

};

#endif //#if (MMP_OS == MMP_OS_WIN32)
#endif 

