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

#ifndef MMPPLAYERAVEX3_HPP__
#define MMPPLAYERAVEX3_HPP__

#include "MmpPlayer.hpp"
#include "TemplateList.hpp"
#include "mmp_oal_mutex.hpp"
#include "mmp_oal_cond.hpp"
#include "mmp_simple_heap.hpp"

class CMmpPlayerAVEx3 : public CMmpPlayer
{
friend class CMmpPlayer;

private:
    CMmpDemuxer* m_pDemuxer;
    CMmpDecoderAudio* m_pDecoderAudio;
    CMmpDecoderVideo* m_pDecoderVideo;
    CMmpRenderer* m_pRendererAudio;
    CMmpRenderer* m_pRendererVideo;
    CMmpMediaSampleDecodeResult m_DecResult;
    
    /* Video Dec */
    MMP_BOOL m_bServiceRun_VideoDec;
    MMPOALTASK_HANDLE m_service_hdl_video_dec;
    
    TCircular_Queue<CMmpMediaSample> m_queue_video_stream;
    class mmp_oal_mutex* m_p_mutex_video_dec;
    class mmp_oal_cond* m_p_cond_video_dec;

    TCircular_Queue<CMmpMediaSampleDecodeResult> m_queue_video_yuv;
    class mmp_oal_mutex* m_p_mutex_video_render;
    class mmp_oal_cond* m_p_cond_video_render;

    /* Audio Dec */
    MMP_BOOL m_bServiceRun_AudioDec;
    MMPOALTASK_HANDLE m_service_hdl_audio_dec;
    
    TCircular_Queue<CMmpMediaSample> m_queue_audio_stream;
    class mmp_oal_mutex* m_p_mutex_audio_dec;
    class mmp_oal_cond* m_p_cond_audio_dec;

    TCircular_Queue<CMmpMediaSampleDecodeResult> m_queue_audio_pcm;
    class mmp_oal_mutex* m_p_mutex_audio_render;
    class mmp_oal_cond* m_p_cond_audio_render;

    class mmp_simple_heap m_simple_heap;

    /* Audio Render */
    MMP_BOOL m_bServiceRun_AudioRender;
    MMPOALTASK_HANDLE m_service_hdl_audio_render;
    
    /* Video Render */
    MMP_BOOL m_bServiceRun_VideoRender;
    MMPOALTASK_HANDLE m_service_hdl_video_render;
    
    MMP_S64 m_play_start_timestamp;
    MMP_S64 m_seek_target_time;

protected:
    CMmpPlayerAVEx3(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerAVEx3();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void Service();
    virtual void Service_AudioDec();
    virtual void Service_VideoDec();
    virtual void Service_AudioRender();
    virtual void Service_VideoRender();

    virtual MMP_RESULT Seek(MMP_S64 pts);

};

#endif


