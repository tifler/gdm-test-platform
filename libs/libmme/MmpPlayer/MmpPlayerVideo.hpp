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

#ifndef MMPPLAYERVIDEO_HPP__
#define MMPPLAYERVIDEO_HPP__

#include "MmpPlayer.hpp"
#include "mmp_buffer_mgr.hpp"

class CMmpPlayerVideo : public CMmpPlayer
{
friend class CMmpPlayer;

private:
    CMmpDemuxer* m_pDemuxer;
    CMmpDecoderVideo* m_pDecoderVideo;
    CMmpRenderer* m_pRendererVideo;

    MMP_U32 m_buffer_width;
    MMP_U32 m_buffer_height;
    
    MMP_U8* m_stream_buffer;
    MMP_U32 m_stream_buffer_max_size;

    MMP_U8* m_Y;
    MMP_U8* m_U;
    MMP_U8* m_V;

    CMmpMediaSample m_MediaSample;
    CMmpMediaSampleDecodeResult m_DecResult;
    CMmpRendererCreateProp m_RendererProp;
    
    MMP_S64 m_last_packet_pts;
    MMP_S32 m_fps;

protected:
    CMmpPlayerVideo(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerVideo();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void Service();

public:
    virtual MMP_S64 GetDuration();
    virtual MMP_S64 GetPlayPosition();
    virtual MMP_S32 GetPlayFPS();
    
    virtual MMP_S32 GetVideoWidth();
    virtual MMP_S32 GetVideoHeight();
    virtual MMP_U32 GetVideoFormat();

    virtual MMP_S32 GetVideoDecoderFPS();
    virtual MMP_S32 GetVideoDecoderDur();
    virtual MMP_S32 GetVideoDecoderTotalDecFrameCount();
    virtual const MMP_CHAR* GetVideoDecoderClassName();

    /* Video Renderer */
    virtual void SetFirstVideoRenderer();
    virtual MMP_BOOL IsFirstVideoRenderer();
    
};

#endif


