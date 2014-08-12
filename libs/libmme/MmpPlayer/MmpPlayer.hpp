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

#ifndef _MMPPLAYER_HPP__
#define _MMPPLAYER_HPP__

#include "MmpDefine.h"
#include "MmpOAL.hpp"
#include "MmpPlayerDef.h"
#include "MmpPlayerService.hpp"
#include "MmpDemuxer.hpp"
#include "MmpDecoder.hpp"
#include "MmpDecoderVideo.hpp"
#include "MmpDecoderAudio.hpp"
#include "MmpRenderer.hpp"


#define MMP_PLAYER_NORMAL       0x100
#define MMP_PLAYER_VIDEO_ONLY   0x101
#define MMP_PLAYER_AUDIO_ONLY   0x102
#define MMP_PLAYER_AUDIO_VIDEO  0x103  
#define MMP_PLAYER_STAGEFRIGHT  0x104  
#define MMP_PLAYER_TONEPLAYER   0x105  


class CMmpPlayer : public CMmpPlayerService
{
public:
    static CMmpPlayer* CreateObject(MMP_U32 playerID, CMmpPlayerCreateProp* pPlayerProp);
    static MMP_RESULT DestroyObject(CMmpPlayer* pObj);
    
protected:
    CMmpPlayerCreateProp m_create_config;
    

protected:
    CMmpPlayer(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayer();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();
    
    CMmpDemuxer* CreateDemuxer(void);
    CMmpDecoderAudio* CreateDecoderAudio(CMmpDemuxer* pDemuxer);
    CMmpDecoderVideo* CreateDecoderVideo(CMmpDemuxer* pDemuxer, MMP_BOOL bFfmpegUse = MMP_FALSE);
    CMmpDecoder* CreateDecoder(MMP_U32 mediatype, CMmpDemuxer* pDemuxer);
    CMmpRenderer* CreateRendererAudio(CMmpDemuxer* pDemuxer, CMmpDecoderAudio* pDecoderAudio);
    CMmpRenderer* CreateRendererAudio(CMmpDecoderAudio* pDecoderAudio);
    CMmpRenderer* CreateRendererVideo(CMmpDemuxer* pDemuxer);

    MMP_RESULT DecodeMediaExtraData(MMP_U32 mediatype, 
                                    CMmpDemuxer* pDemuxer, CMmpDecoder* pDecoder);

    
};

#endif


