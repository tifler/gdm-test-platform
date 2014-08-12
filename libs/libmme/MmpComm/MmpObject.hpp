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

#ifndef	MMPOBJECT_HPP__
#define	MMPOBJECT_HPP__

#include "../MmpGlobal/MmpDefine.h"

class CMmpObject
{
private:
         
public:   
   CMmpObject();
   ~CMmpObject();
   
#ifdef MMPOBJECT_OVERLOAD_NEW
   void* operator new(size_t num_bytes);
   void* operator new[](size_t num_bytes);
   
   void operator delete(void* p);
   void operator delete[](void* p);
#endif

   //Mmp_RESULT SendEventToHost( int msg );
  //Mmp_RESULT SendEventToHost( int msg, int par1 );
  //Mmp_RESULT SendEventToHost( const MmpEvent* pMmpServiceEvent );  
};

#endif // __MmpOBJECT_HPP
