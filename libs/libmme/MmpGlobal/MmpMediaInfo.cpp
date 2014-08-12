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

#include "MmpMediaInfo.hpp"
#include "../MmpComm/MmpOAL.hpp"

CMmpMediaInfo::CMmpMediaInfo(MMP_MEDIATYPE mt) :
m_mediaType(mt)
,m_iInfoSize(0)
,m_iDSISize(0)
{

}

CMmpMediaInfo::CMmpMediaInfo() :
m_mediaType(MMP_MEDIATYPE_UNKNOWN)
,m_iInfoSize(0)
,m_iDSISize(0)
{
	memset(this->m_Info, 0x00, 1024);
	memset(this->m_DSI, 0x00, 1024);
}

CMmpMediaInfo::~CMmpMediaInfo()
{

}

CMmpMediaInfo& CMmpMediaInfo::operator=(CMmpMediaInfo& info)
{

	this->m_mediaType = info.m_mediaType;
	memcpy(this->m_Info, info.m_Info, 1024);
	this->m_iInfoSize = info.m_iInfoSize;
	memcpy(this->m_DSI, info.m_DSI, 1024);
	this->m_iDSISize = info.m_iDSISize;

	return *this;
}

MMP_RESULT CMmpMediaInfo::SetMediaInfo(MMP_MEDIATYPE mt, void* info, int infoSize, unsigned char* dsi, int dsiSize )
{
    if(m_mediaType!=mt)
        return MMP_FAILURE;

    if(infoSize==0)
        return MMP_FAILURE;

    m_iInfoSize=infoSize;
    CMmpOAL_MemCopy(m_Info, info, infoSize);

    m_iDSISize=dsiSize;
    if(dsiSize>0)
    {
        CMmpOAL_MemCopy(m_DSI, dsi, dsiSize);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpMediaInfo::SetAudioInfo(MMPWAVEFORMATEX* pwf, unsigned char* dsi, int dsiSize)
{
    return this->SetMediaInfo(MMP_MEDIATYPE_AUDIO, (void*)pwf, sizeof(MMPWAVEFORMATEX), dsi, dsiSize);
}

MMP_RESULT CMmpMediaInfo::SetVideoInfo(MMPBITMAPINFOHEADER* pbih, unsigned char* dsi, int dsiSize)
{
    return this->SetMediaInfo(MMP_MEDIATYPE_VIDEO, (void*)pbih, sizeof(MMPBITMAPINFOHEADER), dsi, dsiSize);
}

