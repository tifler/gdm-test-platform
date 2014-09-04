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

#ifndef MMPPLAYER_SERVICE_HPP__
#define MMPPLAYER_SERVICE_HPP__

#include "MmpDefine.h"
#include "MmpOAL.hpp"
#include "MmpPlayerDef.h"
#include "MmpDemuxer.hpp"
#include "MmpDecoder.hpp"
#include "MmpDecoderVideo.hpp"
#include "MmpDecoderAudio.hpp"
#include "MmpRenderer.hpp"

class CMmpPlayerService {

protected:

    MMP_BOOL m_bServiceRun;
    MMPOALTASK_HANDLE m_service_hdl;

    CMmpMediaSample m_MediaSampleObj;
    CMmpMediaSampleDecodeResult m_DecResultObj;
    
protected:
    CMmpPlayerService();
    virtual ~CMmpPlayerService();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static void ServiceStub(void* parm);
    virtual void Service()=0;

    static void ServiceStub_VideoDec(void* parm);
    virtual void Service_VideoDec() { };

    static void ServiceStub_AudioDec(void* parm);
    virtual void Service_AudioDec() { };

    static void ServiceStub_AudioRender(void* parm);
    virtual void Service_AudioRender() { };

    static void ServiceStub_VideoRender(void* parm);
    virtual void Service_VideoRender() { };

protected:

    void Service_Audio_Only(CMmpDemuxer* pDemuxer, CMmpDecoderAudio* pDecoderAudio, CMmpRenderer* pRendererAudio);

    void Service_AV_Simple(CMmpDemuxer* pDemuxer, 
                           CMmpDecoderAudio* pDecoderAudio, CMmpDecoderVideo* pDecoderVideo,
                           CMmpRenderer* pRendererAudio, CMmpRenderer* pRendererVideo);
    
public:
    virtual MMP_RESULT PlayStart();
    virtual MMP_RESULT PlayStop();

    virtual MMP_RESULT Seek(MMP_S64 pts) { return MMP_FAILURE; }
		
    /* Play Property*/
    virtual MMP_S64 GetDuration() { return 0LL;}
    virtual MMP_S64 GetDuration(MMP_S32 *hour, MMP_S32* min, MMP_S32* sec, MMP_S32* msec);
    virtual MMP_S64 GetPlayPosition() { return 0LL;}
    virtual MMP_S64 GetPlayPosition(MMP_S32 *hour, MMP_S32* min, MMP_S32* sec, MMP_S32* msec);

    virtual MMP_U32 GetDurationMS() { return 0LL;}
    virtual MMP_U32 GetPlayPositionMS() { return 0LL;}
    virtual MMP_S32 GetPlayFPS() { return 0; }

    virtual MMP_S32 GetVideoWidth() { return 0; }
    virtual MMP_S32 GetVideoHeight() { return 0; }
    virtual MMP_U32 GetVideoFormat() { return 0; }
    
    /* Decoder Prop */
    virtual MMP_S32 GetVideoDecoderFPS() { return 0; }
    virtual MMP_S32 GetVideoDecoderDur() { return 0; }
    virtual const MMP_CHAR* GetVideoDecoderClassName() { return "Unknown"; }
    
    /* Video Renderer */
    virtual void SetFirstVideoRenderer() { }
    virtual MMP_BOOL IsFirstVideoRenderer() { return MMP_FALSE; }
    
};

#endif

