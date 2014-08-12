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


#ifndef _HEADER_MMPDEBUG_HPP
#define _HEADER_MMPDEBUG_HPP

#include "../MmpGlobal/MmpDefine.h"


class CMmpDebug
{
private:
	static CMmpDebug* m_pInstance;
	
protected:	
    bool m_bDefaultLineFeed;

protected:
	CMmpDebug(bool bDefaultLineFeed);
	virtual ~CMmpDebug(void);

	virtual MMP_RESULT Open()=0;
	virtual MMP_RESULT Close()=0;

public:
   //Singleton Pattern 
   static MMP_RESULT __MmpApiCall CreateInstance(bool bDefaultLineFeed=false);
   static MMP_RESULT __MmpApiCall DestroyInstance();
   static CMmpDebug* __MmpApiCall GetInstance();

public :

   virtual unsigned char GetChar()=0;
   virtual void PutChar( char ch )=0;
   virtual void PutStr( MMPSTR str )=0;
   virtual void Printf( MMPSTR lpszFormat, ... )=0;
   virtual void PutStrAnsi( char* str ){}
};

#endif //#ifndef _HEADER_MTDEBUG_HPP

