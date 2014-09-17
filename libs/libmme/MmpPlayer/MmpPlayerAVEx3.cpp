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


#include "MmpPlayerAVEx3.hpp"
#include "MmpUtil.hpp"

#if 0
#define MMP_PLAYER_MALLOC(x) m_simple_heap.alloc(x)
#define MMP_PLAYER_FREE(x) m_simple_heap.free(x)
#else
#define MMP_PLAYER_MALLOC(x) malloc(x)
#define MMP_PLAYER_FREE(x) free(x)

#endif

/////////////////////////////////////////////////////////////
//CMmpPlayerAVEx3 Member Functions

CMmpPlayerAVEx3::CMmpPlayerAVEx3(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)
,m_pDemuxer(NULL)
,m_pDecoderAudio(NULL)
,m_pDecoderVideo(NULL)
,m_pRendererAudio(NULL)
,m_pRendererVideo(NULL)

,m_service_hdl_video_dec(NULL)

,m_queue_video_stream(10)
,m_p_mutex_video_dec(NULL)
,m_p_cond_video_dec(NULL)

,m_queue_video_yuv(15)
,m_p_mutex_video_render(NULL)
,m_p_cond_video_render(NULL)


,m_service_hdl_audio_dec(NULL)

,m_queue_audio_stream(10)
,m_p_mutex_audio_dec(NULL)
,m_p_cond_audio_dec(NULL)

,m_queue_audio_pcm(10)
,m_p_mutex_audio_render(NULL)
,m_p_cond_audio_render(NULL)

,m_bServiceRun_AudioRender(MMP_FALSE)
,m_service_hdl_audio_render(NULL)

,m_bServiceRun_VideoRender(MMP_FALSE)
,m_service_hdl_video_render(NULL)

,m_simple_heap(10)
,m_play_start_timestamp(0)
,m_seek_target_time(0)
{
    

}

CMmpPlayerAVEx3::~CMmpPlayerAVEx3()
{
    
    
}

MMP_RESULT CMmpPlayerAVEx3::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    CMmpRendererCreateProp RendererProp;
    CMmpRendererCreateProp* pRendererProp=&RendererProp; 


    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDemuxer = this->CreateDemuxer();
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerAVEx3::Open] FAIL: this->CreateDemuxer")));
        }
        else {
    
     
        }
    }

    /* create audio decoder */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDecoderAudio = (CMmpDecoderAudio*)this->CreateDecoderAudio(m_pDemuxer);
        if(m_pDecoderAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            this->DecodeMediaExtraData(MMP_MEDIATYPE_AUDIO, m_pDemuxer, m_pDecoderAudio);
        }
    }

    /* create vidoe decoder */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDecoderVideo = (CMmpDecoderVideo*)this->CreateDecoderVideo(m_pDemuxer, m_create_config.bForceSWCodec /* bFFmpeg Use*/);
        if(m_pDecoderVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            this->DecodeMediaExtraData(MMP_MEDIATYPE_VIDEO, m_pDemuxer, m_pDecoderVideo);
        }
    }

#if 0
    /* create audio render */
    if(mmpResult == MMP_SUCCESS ) {
        m_pRendererAudio = this->CreateRendererAudio(m_pDemuxer, m_pDecoderAudio);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }
#endif

    /* create video render */
    if(mmpResult == MMP_SUCCESS ) {
        m_pRendererVideo = this->CreateRendererVideo(m_pDemuxer);
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }

    /* create video decoding task */
    if(mmpResult == MMP_SUCCESS ) {

        /* create video dec cond */
        m_p_cond_video_dec = mmp_oal_cond::create_object();
        if(m_p_cond_video_dec == NULL) {
            mmpResult = MMP_FAILURE;
        }

        /* create video dec mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_video_dec = mmp_oal_mutex::create_object();
            if(m_p_mutex_video_dec == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create video rend cond */
        m_p_cond_video_render = mmp_oal_cond::create_object();
        if(m_p_cond_video_render == NULL) {
            mmpResult = MMP_FAILURE;
        }

        /* create video ren mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_video_render = mmp_oal_mutex::create_object();
            if(m_p_mutex_video_render == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create video dec task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_VideoDec=MMP_TRUE;
            m_service_hdl_video_dec = CMmpOAL::GetTaskInstance()->Create(CMmpPlayerService::ServiceStub_VideoDec, this, 1024*10, 255, NULL, 1);
            if(m_service_hdl_video_dec == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }

    
    /* create audio decoding task */
    if(mmpResult == MMP_SUCCESS ) {

        /* create audio dec cond */
        m_p_cond_audio_dec = mmp_oal_cond::create_object();
        if(m_p_cond_audio_dec == NULL) {
            mmpResult = MMP_FAILURE;
        }

        /* create audio dec mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_audio_dec = mmp_oal_mutex::create_object();
            if(m_p_mutex_audio_dec == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create audio rend cond */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_cond_audio_render = mmp_oal_cond::create_object();
            if(m_p_cond_audio_render == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create audio ren mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_audio_render = mmp_oal_mutex::create_object();
            if(m_p_mutex_audio_render == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create audio dec task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_AudioDec=MMP_TRUE;
            m_service_hdl_audio_dec = CMmpOAL::GetTaskInstance()->Create(CMmpPlayerService::ServiceStub_AudioDec, this, 1024*10, 255, NULL, 1);
            if(m_service_hdl_audio_dec == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }

    /* create audio render task */
    if(mmpResult == MMP_SUCCESS ) {

        /* create audio render task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_AudioRender=MMP_TRUE;
            m_service_hdl_audio_render = CMmpOAL::GetTaskInstance()->Create(CMmpPlayerService::ServiceStub_AudioRender, this, 1024*10, 255, NULL, 1);
            if(m_service_hdl_audio_render == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }

    /* create video render task */
    if(mmpResult == MMP_SUCCESS ) {

        /* create audio render task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_VideoRender=MMP_TRUE;
            m_service_hdl_video_render = CMmpOAL::GetTaskInstance()->Create(CMmpPlayerService::ServiceStub_VideoRender, this, 1024*10, 255, NULL, 1);
            if(m_service_hdl_video_render == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }


    m_play_start_timestamp = CMmpUtil::GetTickCountUS();

    return mmpResult;
}


MMP_RESULT CMmpPlayerAVEx3::Close()
{
    CMmpMediaSample MediaSampleObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    
    if(m_service_hdl_audio_render != NULL)
    {
        m_bServiceRun_AudioRender=MMP_FALSE;
        m_p_cond_audio_render->signal(); //signal
        CMmpOAL::GetTaskInstance()->Destroy(m_service_hdl_audio_render);
        m_service_hdl_audio_render=NULL;
    }

    if(m_service_hdl_video_render != NULL)
    {
        m_bServiceRun_VideoRender=MMP_FALSE;
        m_p_cond_video_render->signal(); //signal
        CMmpOAL::GetTaskInstance()->Destroy(m_service_hdl_video_render);
        m_service_hdl_video_render=NULL;
    }

    if(m_service_hdl_video_dec != NULL)
    {
        m_bServiceRun_VideoDec=MMP_FALSE;
        m_p_cond_video_dec->signal(); //signal
        CMmpOAL::GetTaskInstance()->Destroy(m_service_hdl_video_dec);
        m_service_hdl_video_dec=NULL;
    }

    if(m_service_hdl_audio_dec != NULL)
    {
        m_bServiceRun_AudioDec=MMP_FALSE;
        m_p_cond_audio_dec->signal(); //signal
        CMmpOAL::GetTaskInstance()->Destroy(m_service_hdl_audio_dec);
        m_service_hdl_audio_dec=NULL;
    }

    CMmpPlayer::Close();

    CMmpDemuxer::DestroyObject(m_pDemuxer);  m_pDemuxer = NULL;
    CMmpDecoder::DestroyObject(m_pDecoderAudio);  m_pDecoderAudio = NULL;
    CMmpDecoder::DestroyObject(m_pDecoderVideo);  m_pDecoderVideo = NULL;
    CMmpRenderer::DestroyObject(m_pRendererAudio);  m_pRendererAudio = NULL;
    CMmpRenderer::DestroyObject(m_pRendererVideo);  m_pRendererVideo = NULL;
    
    /* delete video dec prop */
    if(m_p_cond_video_dec != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_video_dec);
        m_p_cond_video_dec = NULL;
    }
    
    if(m_p_mutex_video_dec != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_video_dec);
        m_p_mutex_video_dec = NULL;
    }

    if(m_p_cond_video_render != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_video_render);
        m_p_cond_video_render = NULL;
    }
    
    if(m_p_mutex_video_render != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_video_render);
        m_p_mutex_video_render = NULL;
    }

    while(!m_queue_video_stream.IsEmpty()) {
        m_queue_video_stream.Delete(*pMediaSample);
        if(pMediaSample->pAu!=NULL) {
            MMP_PLAYER_FREE(pMediaSample->pAu);
        }
    }

    while(!m_queue_video_yuv.IsEmpty()) {
        m_queue_video_yuv.Delete(*pDecResult);
        MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
        MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
        MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);
    }

    /* delete audio dec prop */
    if(m_p_cond_audio_dec != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_audio_dec);
        m_p_cond_audio_dec = NULL;
    }
    
    if(m_p_mutex_audio_dec != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_audio_dec);
        m_p_mutex_audio_dec = NULL;
    }

    if(m_p_cond_audio_render != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_audio_render);
        m_p_cond_audio_render = NULL;
    }
    
    if(m_p_mutex_audio_render != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_audio_render);
        m_p_mutex_audio_render = NULL;
    }

    while(!m_queue_audio_stream.IsEmpty()) {
        m_queue_audio_stream.Delete(*pMediaSample);
        if(pMediaSample->pAu!=NULL) {
            MMP_PLAYER_FREE(pMediaSample->pAu);
        }
    }

    while(!m_queue_audio_pcm.IsEmpty()) {
        m_queue_audio_pcm.Delete(*pDecResult);
        MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
        
    }
    return MMP_SUCCESS;

}

MMP_RESULT CMmpPlayerAVEx3::Seek(MMP_S64 pts) {

    CMmpMediaSample MediaSampleObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    
    this->PlayStop();
    CMmpUtil::Sleep(200);

    /* flush video stream queue */
    m_p_mutex_video_dec->lock();
    while(!m_queue_video_stream.IsEmpty()) {
        m_queue_video_stream.Delete(*pMediaSample);
        if(pMediaSample->pAu!=NULL) {
            MMP_PLAYER_FREE(pMediaSample->pAu);
        }
    }
    m_p_mutex_video_dec->unlock();
    m_p_cond_video_dec->signal(); //signal

    /* flush video yuv queue */
    m_p_mutex_video_render->lock();
    while(!m_queue_video_yuv.IsEmpty()) {

        m_queue_video_yuv.Delete(*pDecResult);
        MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
        MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
        MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);
    }
    m_p_mutex_video_render->unlock();
    m_p_cond_video_render->signal(); //signal

    /* flush audio stream queue */
    m_p_mutex_audio_dec->lock();
    while(!m_queue_audio_stream.IsEmpty()) {
        m_queue_audio_stream.Delete(*pMediaSample);
        if(pMediaSample->pAu!=NULL) {
            MMP_PLAYER_FREE(pMediaSample->pAu);
        }
    }
    m_p_mutex_audio_dec->unlock();
    m_p_cond_audio_dec->signal(); //signal
    
    /* flush audio pcm queue */
    m_p_mutex_audio_render->lock();
    while(!m_queue_audio_pcm.IsEmpty()) {

       m_queue_audio_pcm.Delete(*pDecResult);
       MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
    }
    m_p_mutex_audio_render->unlock();
    m_p_cond_audio_render->signal(); //signal


    m_pDemuxer->Seek(pts);

    m_seek_target_time = pts;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerAVEx3::Seek] pts=%d "), (unsigned int)(pts/1000) ));
                      

    this->PlayStart();

    return MMP_SUCCESS;
}


void CMmpPlayerAVEx3::Service_AudioDec() {

    MMP_RESULT mmpResult;
    CMmpMediaSample MediaSampleObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj, DecResultObjTemp;;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    MMP_U32 decoded_buf_max_size = 1024*1024;
    
    MMP_U8* stream_buf, *pbuf;
    MMP_S32 remain_buf_size;
    MMP_S64 anchor_timestamp;
    MMP_U32 decoded_size;
    MMP_U32 dec_frame_count = 0;

    MMP_U32 ch, sample_rate, bits_per_sample;
    MMP_S64 timestamp_weight;

    MMP_U32 t1, t2;
 
    //p_omx_buffer_header_out->nTimeStamp = m_AnchorTimeUs + (m_NumSamplesOutput * 1000000ll) / m_pcm_parm.nSamplingRate;

    while(m_bServiceRun_AudioDec == MMP_TRUE) {

        m_p_mutex_audio_dec->lock();
        if( m_queue_audio_stream.IsEmpty() ) {
            m_p_cond_audio_dec->wait(m_p_mutex_audio_dec); //wait
        }
        
        pMediaSample->pAu = NULL;
        if((!m_queue_audio_stream.IsEmpty()) && (m_bServiceRun_AudioDec == MMP_TRUE) )
	    {
		    m_queue_audio_stream.Delete(*pMediaSample);		
	    }
        m_p_mutex_audio_dec->unlock();
    
        if(pMediaSample->pAu != NULL) {
        
            stream_buf = pMediaSample->pAu;
            remain_buf_size = pMediaSample->uiAuSize;
            anchor_timestamp = pMediaSample->uiTimeStamp;
            decoded_size = 0;
            ch = 2;//m_pDemuxer->GetAudioChannel();
            sample_rate = m_pDemuxer->GetAudioSamplingRate();
            bits_per_sample = 16;//m_pDemuxer->GetAudioBitsPerSample();
            timestamp_weight = 1000000LL*8LL/(MMP_S64)(ch*bits_per_sample*sample_rate);
            pbuf = stream_buf;
            while(remain_buf_size > 0) {

                pMediaSample->pAu = pbuf;
                pMediaSample->uiAuSize = remain_buf_size;
                pMediaSample->uiAuMaxSize = remain_buf_size;
                pMediaSample->uiTimeStamp = anchor_timestamp + (MMP_S64)(decoded_size)*timestamp_weight;
                pMediaSample->uiFlag = 0;

                memset(pDecResult, 0x00, sizeof(CMmpMediaSampleDecodeResult));
                pDecResult->uiDecodedSize = 0;
                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (MMP_U32)MMP_PLAYER_MALLOC(decoded_buf_max_size);
                pDecResult->uiDecodedBufferMaxSize = decoded_buf_max_size;
                mmpResult = m_pDecoderAudio->DecodeAu(pMediaSample, pDecResult);
                if( (mmpResult==MMP_SUCCESS) && (pDecResult->uiDecodedSize>0) ) {

                    t1 = CMmpUtil::GetTickCount();
                    t2 = t1;
                    while( (t2-t1) < 500) //Wait 100 ms
                    {
                        if(!m_queue_audio_pcm.IsFull()) {
                            break;
                        }
                        CMmpUtil::Sleep(10);
                        t2 = CMmpUtil::GetTickCount();
                    }

                    m_p_mutex_audio_render->lock();
                    if(m_queue_audio_pcm.IsFull()) {

                       MMPDEBUGMSG(1, (TEXT("[AV Player AudioDec] Render Task Busy Skip Audo")));
                       m_queue_audio_pcm.Delete(DecResultObjTemp);
                       MMP_PLAYER_FREE((void*)DecResultObjTemp.uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
                    }
                    m_queue_audio_pcm.Add(*pDecResult);
                    m_p_mutex_audio_render->unlock();

                    MMPDEBUGMSG(0, (TEXT("[AV Player AudioDec] %d. decsz=%d ts=(%d %d) decbuf=0x%08x pcm_q=%d "), 
                            dec_frame_count,
                            pDecResult->uiDecodedSize,
                             (unsigned int)(pMediaSample->uiTimeStamp/1000), (unsigned int)(pDecResult->uiTimeStamp/1000),
                             pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM],
                             m_queue_audio_pcm.GetSize()
                             ));

                    dec_frame_count++;
                    decoded_size +=  pDecResult->uiDecodedSize;
            
                    m_p_cond_audio_render->signal(); //signal
                }
                else {
                    MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
                }

                remain_buf_size -= pDecResult->uiAuUsedByte;
                pbuf += pDecResult->uiAuUsedByte;
            }

            MMP_PLAYER_FREE(stream_buf);
        }

        CMmpUtil::Sleep(10);
    }

}

void CMmpPlayerAVEx3::Service_AudioRender() {

    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    MMP_U64 cur_us, play_us;
    MMP_U64 audio_eval_timestamp;
    MMP_S64 audio_ren_dealy = 1000LL*500LL;
    MMP_U32 render_frame_count = 0;
    CMmpRenderer* pRendererAudio = NULL;//m_pRendererAudio;
    
    while(m_bServiceRun_AudioRender == MMP_TRUE) {
    
        m_p_mutex_audio_render->lock();
        if( m_queue_audio_pcm.IsEmpty() ) {
            m_p_cond_audio_render->wait(m_p_mutex_audio_render); //wait
        }
        
        pDecResult->uiDecodedSize = 0;
        if((!m_queue_audio_pcm.IsEmpty()) && (m_bServiceRun_AudioRender == MMP_TRUE) )
	    {
		    m_queue_audio_pcm.Delete(*pDecResult);		
	    }
        m_p_mutex_audio_render->unlock();
        
        if(pDecResult->uiTimeStamp > audio_ren_dealy) {
            audio_eval_timestamp = pDecResult->uiTimeStamp - audio_ren_dealy;
        }
        else {
            audio_eval_timestamp = 0;
        }

        if( (pDecResult->uiDecodedSize > 0) && (pRendererAudio == NULL) ) {
            pRendererAudio = this->CreateRendererAudio(m_pDecoderAudio);
        }

        while(pDecResult->uiDecodedSize > 0) {

            cur_us = CMmpUtil::GetTickCountUS();
            play_us = cur_us - m_play_start_timestamp + m_seek_target_time;

            if(audio_eval_timestamp < play_us ) { 
            
                MMPDEBUGMSG(0, (TEXT("[AV Player AudioRen] %d. decsz=%d ts=( %d %d, %d) decbuf=0x%08x "), 
                            render_frame_count,
                            pDecResult->uiDecodedSize,
                             (unsigned int)(pDecResult->uiTimeStamp/1000), (unsigned int)(audio_eval_timestamp/1000), (unsigned int)(play_us/1000),
                             pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]
                             ));
                             
                if( (play_us - audio_eval_timestamp) < 1000*2000 ) {
                    if(pRendererAudio!=NULL)  pRendererAudio->Render(pDecResult);
                    render_frame_count++;
                }
                else {
                    MMPDEBUGMSG(1, (TEXT("[AV Player AudioRen] frame is too late, skip")));
                }

                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);

                break;
            }
            else if( (play_us + 3*1000000LL) < (MMP_U64)pDecResult->uiTimeStamp) {
            
                MMPDEBUGMSG(1, (TEXT("[AV Player AudioRen] frame is too fast, skip  ts=%d/%d"),
                                        (unsigned int)(pDecResult->uiTimeStamp/1000), 
                                        (unsigned int)(play_us/1000)
                                         ));

                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);

                break;
            }
            else {
                CMmpUtil::Sleep(10);
            }

        }

        CMmpUtil::Sleep(10);

    }

    if(pRendererAudio != NULL) {
        CMmpRenderer::DestroyObject(pRendererAudio);  
        pRendererAudio = NULL;
    }

}

void CMmpPlayerAVEx3::Service_VideoDec() {

    MMP_RESULT mmpResult;
    CMmpMediaSample MediaSampleObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj, DecResultObjTemp;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    MMP_U32 buffer_width, buffer_height;

    buffer_width = MMP_BYTE_ALIGN(m_pDemuxer->GetVideoPicWidth(), 16);
    buffer_height = MMP_BYTE_ALIGN(m_pDemuxer->GetVideoPicHeight(), 16);

    MMP_U32 t1, t2;

    while(m_bServiceRun_VideoDec == MMP_TRUE) {

        m_p_mutex_video_dec->lock();
        if( m_queue_video_stream.IsEmpty() ) {
            m_p_cond_video_dec->wait(m_p_mutex_video_dec); //wait
        }
        
        pMediaSample->pAu = NULL;
        if((!m_queue_video_stream.IsEmpty()) && (m_bServiceRun_VideoDec == MMP_TRUE) )
	    {
		    m_queue_video_stream.Delete(*pMediaSample);		
	    }
        m_p_mutex_video_dec->unlock();
    
        if(pMediaSample->pAu != NULL) {
        
            pDecResult->uiDecodedSize = 0;
            pDecResult->uiDecodedBufferPhyAddr[MMP_DECODED_BUF_Y] = 0;
            pDecResult->uiDecodedBufferPhyAddr[MMP_DECODED_BUF_U] = 0;
            pDecResult->uiDecodedBufferPhyAddr[MMP_DECODED_BUF_V] = 0;
            pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y] = (MMP_U32)MMP_PLAYER_MALLOC(buffer_width*buffer_height);
            pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U] = (MMP_U32)MMP_PLAYER_MALLOC(buffer_width*buffer_height/4);
            pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V] = (MMP_U32)MMP_PLAYER_MALLOC(buffer_width*buffer_height/4);
            pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y] = buffer_width;
            pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_U] = buffer_width/2;
            pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_V] = buffer_width/2;
            pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_Y] = buffer_height;
            pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_U] = buffer_height/2;
            pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_V] = buffer_height/2;
            pDecResult->uiDecodedBufferMaxSize = buffer_width*buffer_height*3/2;
        
            MMPDEBUGMSG(0, (TEXT("[AV Player VideoDec] TS=%d sz=%d stream(%02x %02x %02x %02x %02x %02x %02x %02x) "),
                (unsigned int)(pMediaSample->uiTimeStamp/1000),
                pMediaSample->uiAuSize,
                pMediaSample->pAu[0], pMediaSample->pAu[1], pMediaSample->pAu[2], pMediaSample->pAu[3], 
                pMediaSample->pAu[4], pMediaSample->pAu[5], pMediaSample->pAu[6], pMediaSample->pAu[7] 
                ));

            mmpResult = m_pDecoderVideo->DecodeAu(pMediaSample, pDecResult);
            if( (mmpResult==MMP_SUCCESS) && (pDecResult->uiDecodedSize>0) ) {

                t1 = CMmpUtil::GetTickCount();
                t2 = t1;
                while( (t2-t1) < 1000) //Wait 1sec
                {
                    if(!m_queue_video_yuv.IsFull()) {
                        break;
                    }
                    CMmpUtil::Sleep(10);
                    t2 = CMmpUtil::GetTickCount();
                }

                m_p_mutex_video_render->lock();
                if(m_queue_video_yuv.IsFull()) {

                    MMPDEBUGMSG(1, (TEXT("[AV Player VideoDec] Render Task, Busy Skip Video")));

                    m_queue_video_yuv.Delete(DecResultObjTemp);
                    MMP_PLAYER_FREE((void*)DecResultObjTemp.uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                    MMP_PLAYER_FREE((void*)DecResultObjTemp.uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                    MMP_PLAYER_FREE((void*)DecResultObjTemp.uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);
                }
                m_queue_video_yuv.Add(*pDecResult);
                m_p_mutex_video_render->unlock();
                m_p_cond_video_render->signal(); //signal
            }
            else {

                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);
            }
        
            MMP_PLAYER_FREE(pMediaSample->pAu);
        }

        CMmpUtil::Sleep(10);

    }

}
    
void CMmpPlayerAVEx3::Service_VideoRender() {

#if 1
    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    MMP_S64 cur_us, play_us;
    MMP_S64 diff;
    
    while(m_bServiceRun_VideoRender == MMP_TRUE) {
    
        m_p_mutex_video_render->lock();
        if( m_queue_video_yuv.IsEmpty() ) {
            m_p_cond_video_render->wait(m_p_mutex_video_render); //wait
        }
        
        pDecResult->uiDecodedSize = 0;
        if((!m_queue_video_yuv.IsEmpty()) && (m_bServiceRun_VideoRender == MMP_TRUE) )
	    {
		    m_queue_video_yuv.Delete(*pDecResult);		
	    }
        m_p_mutex_video_render->unlock();
        

        while(pDecResult->uiDecodedSize > 0) {

            cur_us = CMmpUtil::GetTickCountUS();
            play_us = cur_us - m_play_start_timestamp + m_seek_target_time;

            diff =  (MMP_S64)play_us - (MMP_S64)pDecResult->uiTimeStamp;
            

            MMPDEBUGMSG(0, (TEXT("[AV Player VideoRen]  ts=%d / %d  (%d) "), 
                             (unsigned int)(pDecResult->uiTimeStamp/1000), 
                             (unsigned int)(play_us/1000),
                             (int)(diff/1000)
                             ));
                             
            if(pDecResult->uiTimeStamp < play_us ) { 
            
                if( (play_us-pDecResult->uiTimeStamp) < 1000*1000 ) {
                    if(pDecResult->uiResultType ==  MMP_MEDIASAMPLE_BUFFER_TYPE_VIDEO_FRAME) {
                        m_pRendererVideo->Render((class mmp_buffer_videoframe*)pDecResult->uiDecodedBufferPhyAddr[MMP_DECODED_BUF_VIDEO_FRAME]);
                    }
                    else {
                        m_pRendererVideo->Render(pDecResult);
                    }
                }
                else {
                    MMPDEBUGMSG(1, (TEXT("[AV Player VideoRen] frame is too late, skip  ts=%d/%d  (%d)"),
                                        (unsigned int)(pDecResult->uiTimeStamp/1000), 
                                        (unsigned int)(play_us/1000),
                                        (int)(diff/1000) ));
                }

                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);

                break;
            }
            else if( (play_us + 3*1000000LL) < pDecResult->uiTimeStamp) {
            
                MMPDEBUGMSG(1, (TEXT("[AV Player VideoRen] frame is too fast, skip  ts=%d/%d  (%d)"),
                                        (unsigned int)(pDecResult->uiTimeStamp/1000), 
                                        (unsigned int)(play_us/1000),
                                        (int)(diff/1000) ));

                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);

                break;
            }
            else {
                CMmpUtil::Sleep(10);
            }

        }

        CMmpUtil::Sleep(10);
        

    }
#endif

}

void CMmpPlayerAVEx3::Service() {

    MMP_BOOL bFreeAu;
    MMP_RESULT mmpResult;
    MMP_U32 stream_buf_max_size = 1024*1024;
    MMP_S64 cur_tick, before_tick;
    MMP_S64 packet_pts;

    MMP_U32 packet_count = 0;

    CMmpMediaSample MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObjAudio;
    CMmpMediaSampleDecodeResult DecResultObjVideo;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult* pDecResultAudio = &DecResultObjAudio;
    CMmpMediaSampleDecodeResult* pDecResultVideo = &DecResultObjVideo;
    
    m_play_start_timestamp = CMmpUtil::GetTickCountUS();
    before_tick = m_play_start_timestamp;
    while(m_bServiceRun == MMP_TRUE) {


        if( ((m_queue_video_stream.GetSize() < 3)  || (m_queue_audio_stream.GetSize() < 5))
            && (!m_queue_audio_stream.IsFull())
            && (!m_queue_video_stream.IsFull())
            ) 
        {
            bFreeAu = MMP_TRUE;
            pMediaSample->pAu = (MMP_U8*)MMP_PLAYER_MALLOC(stream_buf_max_size);
            pMediaSample->uiAuMaxSize = stream_buf_max_size;

            if(pMediaSample->pAu != NULL) {
                mmpResult = m_pDemuxer->GetNextData(pMediaSample);
                if(mmpResult == MMP_SUCCESS) {
            
                    MMPDEBUGMSG(0, (TEXT("[Source Task] pktcnt=%d %s pts=%02d:%02d:%02d "),
                                             packet_count, 
                                             (pMediaSample->uiMediaType == MMP_MEDIATYPE_VIDEO)?"Video":"Audio",
                                             CMmpUtil::Time_GetHour((unsigned int)(pMediaSample->uiTimeStamp/1000)),
                                             CMmpUtil::Time_GetMin((unsigned int)(pMediaSample->uiTimeStamp/1000)),
                                             CMmpUtil::Time_GetSec((unsigned int)(pMediaSample->uiTimeStamp/1000)) ));



                    if(packet_count == 0) {
                        m_play_start_timestamp = CMmpUtil::GetTickCountUS();
                    }
                    
                    packet_pts = pMediaSample->uiTimeStamp;
                    if(packet_pts < m_seek_target_time) {
                        m_seek_target_time = packet_pts;
                    }

                    if(pMediaSample->uiMediaType == MMP_MEDIATYPE_VIDEO) {
#if 1
                        m_p_mutex_video_dec->lock();
                        m_queue_video_stream.Add(*pMediaSample);
                        m_p_mutex_video_dec->unlock();
                        m_p_cond_video_dec->signal(); //signal
                        bFreeAu = MMP_FALSE;
#endif
                    }
                    else if(pMediaSample->uiMediaType == MMP_MEDIATYPE_AUDIO) {

#if 0
                        m_p_mutex_audio_dec->lock();
                        m_queue_audio_stream.Add(*pMediaSample);
                        m_p_mutex_audio_dec->unlock();
                        m_p_cond_audio_dec->signal(); //signal
                        bFreeAu = MMP_FALSE;
#endif
                    }

                    packet_count++;

                }
            }
        
            if(bFreeAu == MMP_TRUE) {
                MMP_PLAYER_FREE(pMediaSample->pAu);
            }

        } /* if( (m_queue_video_yuv.GetSize() < 3)  || (m_queue_audio_pcm.GetSize() < 5) )  */
        else {
        
            CMmpUtil::Sleep(30);
        }

        cur_tick = CMmpUtil::GetTickCountUS();
        if( (cur_tick - before_tick) > (1000LL*1000LL) ) {
        
            if(this->m_create_config.callback != NULL) {

                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                void *data1 = NULL, *data2 = NULL;

                msg = MMP_PLAYER_CALLBACK_PLAYTIME;
                playtime_st.media_duration  = m_pDemuxer->GetDuration();
                playtime_st.media_pts = packet_pts;

                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }

            before_tick = cur_tick;
        }
        
    } /* while(m_bServiceRun == MMP_TRUE) {  */

}

