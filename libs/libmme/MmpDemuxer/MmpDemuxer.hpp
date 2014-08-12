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

#ifndef _MMPDEMUXER_HPP__
#define _MMPDEMUXER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "MmpDemuxerBuffer.hpp"

#define MMP_DEMUXER_TYPE_FFMPEG       0x1000
#define MMP_DEMUXER_TYPE_MJPEG_STREAM 0x2001
#define MMP_DEMUXER_TYPE_AMMF         0x2002

struct MmpDemuxerCreateConfig
{
    MMP_U32 type;
    MMP_U8 filename[256];
};

class CMmpDemuxer : public CMmpDemuxerBuffer
{

public:
    static CMmpDemuxer* CreateObject(struct MmpDemuxerCreateConfig* pCreateConfig);
    static MMP_RESULT DestroyObject(CMmpDemuxer*);

protected:
    struct MmpDemuxerCreateConfig m_create_config;

protected:
    CMmpDemuxer(struct MmpDemuxerCreateConfig* pCreateConfig);
    virtual ~CMmpDemuxer();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT GetNextData(CMmpMediaSample* pMediaSample);
   
    virtual MMP_RESULT GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts);
    virtual MMP_RESULT GetNextAudioData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts);
    virtual MMP_RESULT GetNextMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts) = 0;
    
    virtual MMP_RESULT GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size);
    virtual MMP_RESULT GetAudioExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size);
    virtual MMP_RESULT GetMediaExtraData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size) = 0;


    virtual MMP_U32 GetVideoFormat() = 0;
    virtual MMP_U32 GetVideoPicWidth() = 0;
    virtual MMP_U32 GetVideoPicHeight() = 0;

    virtual MMP_U32 GetAudioFormat() = 0;
    virtual MMP_U32 GetAudioChannel() = 0;
    virtual MMP_U32 GetAudioSamplingRate() = 0;
    virtual MMP_U32 GetAudioBitsPerSample() = 0;
    
    virtual MMP_S64 GetDuration() { return 0LL; }

    virtual MMP_RESULT Seek(MMP_S64 pts) { return MMP_FAILURE; }
};


#endif //#ifndef _MMPDEMUXER_HPP__