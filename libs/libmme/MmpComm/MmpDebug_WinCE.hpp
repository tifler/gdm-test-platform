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

#ifndef _HEADER_MMPDEBUG_WINCE_HPP
#define _HEADER_MMPDEBUG_WINCE_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpDebug.hpp"

#if (MMP_OS==MMP_OS_WINCE60 )

class CMmpDebug_WinCE : public CMmpDebug
{
friend class CMmpDebug;
private:
    HANDLE m_hDriver;
	HANDLE m_hDevFile;

private:
	CMmpDebug_WinCE(bool bDefaultLineFeed);
	virtual ~CMmpDebug_WinCE(void);

	virtual MMP_RESULT Open();
	virtual MMP_RESULT Close();

public :

    virtual unsigned char GetChar();
    virtual void PutChar( char ch );
    virtual void PutStr( MMPSTR str );
    virtual void Printf( MMPSTR lpszFormat, ... );
    //virtual void Printf( int level, MMPSTR lpszFormat, ... );
  
    virtual void PutStrAnsi( char* str );
};

#endif //#if (MMP_OS==MMP_OS_WIN32)

#endif //#ifndef _HEADER_MTDEBUG_HPP