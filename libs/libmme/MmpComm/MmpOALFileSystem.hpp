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


#ifndef	__MMPOALFILESYSTEM_HPP
#define	__MMPOALFILESYSTEM_HPP

#include "../MmpGlobal/MmpDefine.h"

#include "MmpObject.hpp"
#include "MmpOALDef.h"

class CMmpOALFileSystem : public CMmpObject
{
private:
   
protected:
	
public:   
   CMmpOALFileSystem();
   virtual ~CMmpOALFileSystem();

   virtual void InitWithTask()=0;

   virtual MMP_RESULT I_Open( const char* filename, unsigned char mode, MMPOAL_FILE_HANDLE *fp)=0;
   virtual MMP_RESULT I_Close(MMPOAL_FILE_HANDLE fp)=0;
   virtual MMP_RESULT I_Read( MMPOAL_FILE_HANDLE fp, char* data, unsigned int datasize, unsigned int *pLengthRead)=0;
   virtual MMP_RESULT I_Write( MMPOAL_FILE_HANDLE fp, const char* data, unsigned int datasize, unsigned int *pLengthWritten)=0;   
   virtual MMP_RESULT I_Seek( MMPOAL_FILE_HANDLE fp, unsigned int offset, unsigned int mark, unsigned int *pPosition)=0;
   virtual MMP_RESULT I_Tell( MMPOAL_FILE_HANDLE fp, unsigned int* pPosition)=0;

};


#endif // __MEDIAPLAYER_HPP

