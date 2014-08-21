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


#include "MmpPlayerAudio.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerAudio Member Functions

CMmpPlayerAudio::CMmpPlayerAudio(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)
,m_pDemuxer(NULL)
,m_pDecoderAudio(NULL)
,m_pRendererAudio(NULL)
,m_stream_buffer(NULL)
,m_stream_buffer_max_size(1024*1024)
,m_pcm_buffer(NULL)
,m_pcm_buffer_max_size(1024*1024)
{
    

}

CMmpPlayerAudio::~CMmpPlayerAudio()
{
    
    
}

MMP_RESULT CMmpPlayerAudio::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    CMmpRendererCreateProp RendererProp;
    CMmpRendererCreateProp* pRendererProp=&RendererProp; 

    CMmpPlayer::Open();

    /* create stream buffer */
    if(mmpResult == MMP_SUCCESS ) {
        
        m_stream_buffer = (MMP_U8*)malloc(m_stream_buffer_max_size);
        if(m_stream_buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* create pcm buffer */
    if(mmpResult == MMP_SUCCESS ) {
        
        m_pcm_buffer = (MMP_U8*)malloc(m_pcm_buffer_max_size);
        if(m_pcm_buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDemuxer = this->CreateDemuxer();
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }

    /* create audio decoder */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDecoderAudio = this->CreateDecoderAudio(m_pDemuxer);
        if(m_pDecoderAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {

            MMP_U32 stream_buf_size;
            CMmpMediaSample *pMediaSample = &m_MediaSample;
            CMmpMediaSampleDecodeResult* pDecResult = &m_DecResult;
    
            mmpResult = m_pDemuxer->GetAudioExtraData(m_stream_buffer, m_stream_buffer_max_size, &stream_buf_size);
            if(mmpResult == MMP_SUCCESS) {
                

                pMediaSample->pAu = m_stream_buffer;
                pMediaSample->uiAuSize = stream_buf_size;
                pMediaSample->uiSampleNumber = 0;
                pMediaSample->uiTimeStamp = 0;
                pMediaSample->uiFlag = MMP_MEDIASAMPMLE_FLAG_CONFIGDATA;

                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (MMP_U32)m_pcm_buffer;
                pDecResult->uiDecodedBufferMaxSize = m_pcm_buffer_max_size;
                        
                mmpResult = m_pDecoderAudio->DecodeAu(pMediaSample, pDecResult);
            }
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

    return mmpResult;
}


MMP_RESULT CMmpPlayerAudio::Close()
{
    CMmpPlayer::Close();

    CMmpDemuxer::DestroyObject(m_pDemuxer);  m_pDemuxer = NULL;
    CMmpDecoder::DestroyObject(m_pDecoderAudio);  m_pDecoderAudio = NULL;
    CMmpRenderer::DestroyObject(m_pRendererAudio);  m_pRendererAudio = NULL;
    
    if(m_stream_buffer!=NULL) { free(m_stream_buffer); m_stream_buffer = NULL; }
    if(m_pcm_buffer!=NULL) { free(m_pcm_buffer); m_pcm_buffer = NULL; }
    
     
    return MMP_SUCCESS;

}

#if 0

void CMmpPlayerAudio::Service() {
    
    this->Service_Audio_Only(m_pDemuxer, m_pDecoderAudio, m_pRendererAudio);
}

#else
void CMmpPlayerAudio::Service()
{
    MMP_U32 start_tick, before_tick, cur_tick;
    MMP_RESULT mmpResult;
    CMmpMediaSample *pMediaSample = &m_MediaSampleObj;
    CMmpMediaSampleDecodeResult* pDecResult = &m_DecResultObj;

    CMmpDemuxer* pDemuxer = m_pDemuxer;
    CMmpDecoderAudio* pDecoderAudio = m_pDecoderAudio;
    CMmpRenderer* pRendererAudio = NULL;//m_pRendererAudio;
    
    MMP_U32 stream_buf_size;
    MMP_U32 stream_buf_max_size = 1024*1024;
    MMP_U8* stream_buf, *pbuf;

    MMP_U8* pcm_buf;
    MMP_U32 pcm_buf_max_size = 1024*1024;

    MMP_U32 frame_count = 0, packet_count=0, packet_sub_index = 0;
    MMP_U32 render_size = 0, render_sub_size = 0;

    MMP_S64 packet_pts=0, last_render_pts=0;
    MMP_S64 timestamp_weight = 0;

    stream_buf = (MMP_U8*)malloc(stream_buf_max_size);
    if(stream_buf == NULL) {
        m_bServiceRun = MMP_FALSE;
    }

    pcm_buf = (MMP_U8*)malloc(pcm_buf_max_size);
    if(pcm_buf == NULL) {
        m_bServiceRun = MMP_FALSE;
    }
    
    start_tick = CMmpUtil::GetTickCount();
    before_tick = start_tick;
    while(m_bServiceRun == MMP_TRUE) {
    
        mmpResult = pDemuxer->GetNextAudioData(stream_buf, stream_buf_max_size, &stream_buf_size, &packet_pts);
        if(mmpResult == MMP_SUCCESS) {
        
            packet_sub_index = 0;
            render_sub_size = 0;
            pbuf = stream_buf;
            while(stream_buf_size > 0) {

                pMediaSample->pAu = pbuf;
                pMediaSample->uiAuSize = stream_buf_size;
                pMediaSample->uiSampleNumber = 0;
                pMediaSample->uiTimeStamp = packet_pts + render_sub_size * timestamp_weight;
                pMediaSample->uiFlag = 0;

                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (MMP_U32)pcm_buf;
                pDecResult->uiDecodedBufferMaxSize = pcm_buf_max_size;
                        
                pDecoderAudio->DecodeAu(pMediaSample, pDecResult);
                if(pDecResult->uiDecodedSize > 0) {


                    if(pRendererAudio == NULL) {

                        int ch, sample_rate, bits_per_sample;

                        ch = (int)(pDecoderAudio->GetWF_Out().nChannels); ch&=0xFFFF;
                        sample_rate = (int)pDecoderAudio->GetWF_Out().nSamplesPerSec;
                        bits_per_sample = (int)pDecoderAudio->GetWF_Out().wBitsPerSample;  bits_per_sample&=0xFFFF;
                        timestamp_weight = 1000000LL*8LL/(MMP_S64)(ch*bits_per_sample*sample_rate);
            
                        pRendererAudio = this->CreateRendererAudio(pDecoderAudio);
                    }

                    if(pRendererAudio != NULL) {

                        pRendererAudio->RenderPCM(pcm_buf, pDecResult->uiDecodedSize);
#if (MMPPLAYER_DUMP_PCM == 1)
                        this->DumpPCM_Write(pcm_buf, pDecResult->uiDecodedSize);
#endif
                    }

                    frame_count++;
                    render_size += pDecResult->uiDecodedSize;
                    render_sub_size += pDecResult->uiDecodedSize;
                    last_render_pts = pDecResult->uiTimeStamp;

#if 0  /* debug monitoring */
                    cur_tick = CMmpUtil::GetTickCount();
                    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerAudio::Service -- Dec Detail Info ] %d. pktcnt=(%d %d) frc=%d rnsz=(%d %d) pts=(%d %d)"),
                           (cur_tick-start_tick)/1000,
                           packet_count, packet_sub_index,
                           frame_count, 
                           render_size, render_sub_size, 
                           (unsigned int)(packet_pts/1000), (unsigned int)(pMediaSample->uiTimeStamp/1000)
                    ));

#endif


                    CMmpUtil::Sleep(1);
                }

                if(stream_buf_size >= pDecResult->uiAuUsedByte) {
                    stream_buf_size -= pDecResult->uiAuUsedByte;
                    pbuf += pDecResult->uiAuUsedByte;
                }
                else {
                    stream_buf_size = 0;
                }

                
                
                packet_sub_index++;
                CMmpUtil::Sleep(1);

            } /* end of while(stream_buf_size > 0) { */

            packet_count++;
        }

        CMmpUtil::Sleep(1);

        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick - before_tick) > 1000) {

            MMPDEBUGMSG(0, (TEXT("[CMmpPlayerAudio::Service] %d. frc=%d rnsz=%d pts=%d "),
                           (cur_tick-start_tick)/1000,
                           frame_count, render_size,
                           (unsigned int)(packet_pts/1000)
                ));

            if(this->m_create_config.callback != NULL) {
            
                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                void *data1 = NULL, *data2 = NULL;

                msg = MMP_PLAYER_CALLBACK_PLAYTIME;
                playtime_st.media_duration  = m_pDemuxer->GetDuration();
                playtime_st.media_pts = last_render_pts;

                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }

            before_tick = cur_tick;
            frame_count = 0;
            render_size = 0;
        }

    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    if(pRendererAudio != NULL) {
        CMmpRenderer::DestroyObject(pRendererAudio);  
        pRendererAudio = NULL;
    }

    if(stream_buf != NULL) {
        free(stream_buf);
    }

    if(pcm_buf != NULL) {
        free(pcm_buf);
    }

}
#endif
