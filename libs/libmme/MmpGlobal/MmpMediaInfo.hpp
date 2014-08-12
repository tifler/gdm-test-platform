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

#ifndef _MMPMEDIAINFO_HPP__
#define _MMPMEDIAINFO_HPP__

#include "MmpPlayerDef.h"

class CMmpSource;

class CMmpMediaInfo
{
friend class CMmpSource;

private:
    MMP_MEDIATYPE m_mediaType;
    unsigned char m_Info[1024];
    int m_iInfoSize;
    unsigned char m_DSI[1024];
    int m_iDSISize;
    
public:
    CMmpMediaInfo(MMP_MEDIATYPE mt);
	CMmpMediaInfo();
    ~CMmpMediaInfo();

    MMP_RESULT SetMediaInfo(MMP_MEDIATYPE mt, void* info, int infoSize, unsigned char* dsi, int dsiSize );
public:

    MMP_RESULT SetAudioInfo(MMPWAVEFORMATEX* pwf, unsigned char* dsi, int dsiSize);
    MMP_RESULT SetVideoInfo(MMPBITMAPINFOHEADER* pbih, unsigned char* dsi, int dsiSize);
    
    MMP_MEDIATYPE GetMediaType() { return m_mediaType; }
    int GetMediaInfoSize() { return m_iInfoSize; }
    void* GetMediaInfo() { return (void*)m_Info; }
    int GetDSISize() { return m_iDSISize; }
    void* GetDSI() { return m_DSI; }

	CMmpMediaInfo& operator=(CMmpMediaInfo& info);
};


#endif

