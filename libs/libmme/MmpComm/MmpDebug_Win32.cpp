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
#include "MmpDebug_Win32.hpp"

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE60_APP)


/////////////////////////////////////////////////////////////////
//
CMmpDebug_Win32::CMmpDebug_Win32(bool bDefaultLineFeed)  :
CMmpDebug(bDefaultLineFeed)
{
}

CMmpDebug_Win32::~CMmpDebug_Win32(void)
{
	this->Close();
}

MMP_RESULT CMmpDebug_Win32::Open()
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDebug_Win32::Close()
{
	return MMP_SUCCESS;
}

void CMmpDebug_Win32::PutChar( char ch )
{ 
    TCHAR wsz[32];//CString str;
    _stprintf(wsz, TEXT("%c"), ch );
    OutputDebugString(wsz);
}

void CMmpDebug_Win32::PutStr( MMPSTR str )
{
    //WCHAR ustr[2048];
    //MultiByteToWideChar( CP_ACP, NULL, str, (int)strlen(str)+1, ustr, 2048 );
    OutputDebugString(str);
}

void CMmpDebug_Win32::Printf( MMPSTR lpszFormat, ... )
{
    TCHAR rgchBuf[2048];

    va_list arglist;
    va_start(arglist, lpszFormat);
    //NKvDbgPrintfW(lpszFmt, arglist);

    _vstprintf( rgchBuf, lpszFormat, arglist );
    //sprintf( rgchBuf, lpszFormat, arglist );

    
    va_end(arglist);

    OutputDebugString(rgchBuf);
}


#endif //#if (MMP_OS==MMP_OS_WIN32)
