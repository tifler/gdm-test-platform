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

#ifndef _MMPMUXER_HPP__
#define _MMPMUXER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"

#define MMP_MUXER_TYPE_FFMPEG 0x1000
#define MMP_MUXER_TYPE_ANAPASS_MULTIMEDIA_FORMAT 0x3001
#define MMP_MUXER_TYPE_RAWSTREAM                 0x3002

struct MmpMuxerCreateConfig
{
    MMP_U32 type;
    MMP_U8 filename[256];

    MMP_BOOL bMedia[MMP_MEDIATYPE_MAX];

    MMPWAVEFORMATEX wf;
    MMPBITMAPINFOHEADER bih;
};

class CMmpMuxer
{

public:
    static CMmpMuxer* CreateObject(struct MmpMuxerCreateConfig* pCreateConfig);
    static MMP_RESULT DestroyObject(CMmpMuxer*);

protected:
    struct MmpMuxerCreateConfig m_create_config;

protected:
    CMmpMuxer(struct MmpMuxerCreateConfig* pCreateConfig);
    virtual ~CMmpMuxer();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size) = 0;
    virtual MMP_RESULT AddVideoData(MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_U32 timestamp);
    
    virtual MMP_RESULT AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_U32 timestamp) = 0;
    
    
};


#endif //#ifndef _MMPDEMUXER_HPP__