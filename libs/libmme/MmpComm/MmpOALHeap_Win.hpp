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


#ifndef	__MMPOALHEAP_WIN_HPP
#define	__MMPOALHEAP_WIN_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpOALHeap.hpp"

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WINCE60_APP)


class CMmpOALHeap_Win : public CMmpOALHeap
{
private:
   
	
public:   
   CMmpOALHeap_Win();
   virtual ~CMmpOALHeap_Win();

   virtual void* Alloc(int allocSize);
   virtual MMP_RESULT Free(void* ptr); 
};

#endif //#if (MMP_OS==MMP_OS_MMP1)

#endif // __MEDIAPLAYER_HPP

