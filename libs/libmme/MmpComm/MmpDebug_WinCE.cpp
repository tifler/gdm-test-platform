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
#include "MmpDebug_WinCE.hpp"

#if (MMP_OS==MMP_OS_WINCE60)


/////////////////////////////////////////////////////////////////
//
CMmpDebug_WinCE::CMmpDebug_WinCE(bool bDefaultLineFeed)  :
CMmpDebug(bDefaultLineFeed)
,m_hDriver(NULL)
,m_hDevFile(NULL)
{
}

CMmpDebug_WinCE::~CMmpDebug_WinCE(void)
{
	this->Close();
}

MMP_RESULT CMmpDebug_WinCE::Open()
{
    m_hDriver=RegisterDevice( L"MDV", 1, L"MtDebugDrv.dll", 10 );
	if(m_hDriver)
	{
	   OutputDebugString(_T("[CMmpDebug_WinCE::Open] SUCCESS : MtDebugDrv Dreiver Resister\n\r"));
	
       m_hDevFile = CreateFile(  _T("MDV1:"),
							 (GENERIC_READ | GENERIC_WRITE),
							 0,
							 NULL,
							 OPEN_EXISTING,
							 0,
							 NULL);
	    if( m_hDevFile )
	    {
	        OutputDebugString(_T("[CMmpDebug_WinCE::Open] SUCCESS : CreateFile\n\r"));
	    }
	    else
	    {
		    OutputDebugString(_T("[CMmpDebug_WinCE::Open] FAIL : CreateFile\n\r"));
	    }
    }
    else
    {
       OutputDebugString(_T("[CMmpDebug_WinCE::Open] FAIL : MtDebugDrv Dreiver Resister\n\r"));
    }
	
    OutputDebugString(_T("[CMmpDebug_WinCE::Open] SUCCESS \n\r"));
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDebug_WinCE::Close()
{
    if( m_hDevFile )
	{
		CloseHandle( m_hDevFile );
		m_hDevFile=NULL;
	}

	if( m_hDriver )
    {
       DeregisterDevice(m_hDriver);
	   m_hDriver=NULL;
    }

	return MMP_SUCCESS;
}

void CMmpDebug_WinCE::PutChar( char ch )
{ 
    if(m_hDevFile)
    {
        DWORD writtenSize;
        WriteFile( m_hDevFile, &ch, 1, &writtenSize, NULL );
    }
    else
    {
        WCHAR wsz[32];//CString str;
        swprintf(wsz, L"%c", ch );
        OutputDebugString(wsz);
    }
}

unsigned char CMmpDebug_WinCE::GetChar()
{
    DWORD ch;
    DWORD readSize;

    ReadFile( m_hDevFile, &ch,  1,  &readSize, NULL );

    if(readSize==0)
	   ch=0;
   
    return (unsigned char)ch;
}

void CMmpDebug_WinCE::PutStrAnsi( char* str )
{
    if(m_hDevFile)
    {
        DWORD writtenSize;
        int size;

        size=strlen((const char*)str);
        //if( str[size-1]=='\r') size--;
        WriteFile( m_hDevFile, str, size, &writtenSize, NULL );
    }
}

void CMmpDebug_WinCE::PutStr( MMPSTR str )
{
    //WCHAR ustr[2048];
    //MultiByteToWideChar( CP_ACP, NULL, str, (int)strlen(str)+1, ustr, 2048 );
    OutputDebugString(str);
}

void CMmpDebug_WinCE::Printf( MMPSTR lpszFormat, ... )
{
    WCHAR rgchBuf[2048];

    va_list arglist;
    va_start(arglist, lpszFormat);
    //NKvDbgPrintfW(lpszFmt, arglist);

    vswprintf( rgchBuf, lpszFormat, arglist );

    
    va_end(arglist);

    OutputDebugString(rgchBuf);
}


#endif //#if (MMP_OS==MMP_OS_WIN32)
