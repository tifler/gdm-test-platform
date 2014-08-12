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


#include "MmpPlayerAVEx1.hpp"
#include "MmpUtil.hpp"

#if 0
#define MMP_PLAYER_MALLOC(x) m_simple_heap.alloc(x)
#define MMP_PLAYER_FREE(x) m_simple_heap.free(x)
#else
#define MMP_PLAYER_MALLOC(x) malloc(x)
#define MMP_PLAYER_FREE(x) free(x)

#endif

/////////////////////////////////////////////////////////////
//CMmpPlayerAVEx1 Member Functions

CMmpPlayerAVEx1::CMmpPlayerAVEx1(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)
,m_pDemuxer(NULL)
,m_pDecoderAudio(NULL)
,m_pDecoderVideo(NULL)
,m_pRendererAudio(NULL)
,m_pRendererVideo(NULL)

,m_service_hdl_video_dec(NULL)

,m_queue_video_stream(30*10)
,m_p_mutex_video_dec(NULL)
,m_p_cond_video_dec(NULL)

,m_queue_video_yuv(30*10)
,m_p_mutex_video_ren(NULL)
,m_p_cond_video_ren(NULL)


,m_service_hdl_audio_dec(NULL)

,m_queue_audio_stream(30*10)
,m_p_mutex_audio_dec(NULL)
,m_p_cond_audio_dec(NULL)

,m_queue_audio_pcm(30*10)
,m_p_mutex_audio_ren(NULL)
,m_p_cond_audio_ren(NULL)

,m_simple_heap(800)
{
    

}

CMmpPlayerAVEx1::~CMmpPlayerAVEx1()
{
    
    
}

MMP_RESULT CMmpPlayerAVEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    struct MmpDecoderCreateConfig decoder_create_config;

    CMmpRendererCreateProp RendererProp;
    CMmpRendererCreateProp* pRendererProp=&RendererProp; 


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
        m_pDecoderVideo = (CMmpDecoderVideo*)this->CreateDecoderVideo(m_pDemuxer, MMP_FALSE);
        if(m_pDecoderVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            this->DecodeMediaExtraData(MMP_MEDIATYPE_VIDEO, m_pDemuxer, m_pDecoderVideo);
        }
    }


    /* create audio render */
    if(mmpResult == MMP_SUCCESS ) {
        m_pRendererAudio = this->CreateRendererAudio(m_pDemuxer, m_pDecoderAudio);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }

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
        m_p_cond_video_ren = mmp_oal_cond::create_object();
        if(m_p_cond_video_ren == NULL) {
            mmpResult = MMP_FAILURE;
        }

        /* create video ren mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_video_ren = mmp_oal_mutex::create_object();
            if(m_p_mutex_video_ren == NULL) {
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
            m_p_cond_audio_ren = mmp_oal_cond::create_object();
            if(m_p_cond_audio_ren == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create audio ren mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_audio_ren = mmp_oal_mutex::create_object();
            if(m_p_mutex_audio_ren == NULL) {
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

    return mmpResult;
}


MMP_RESULT CMmpPlayerAVEx1::Close()
{
    CMmpMediaSample MediaSampleObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    

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

    if(m_p_cond_video_ren != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_video_ren);
        m_p_cond_video_ren = NULL;
    }
    
    if(m_p_mutex_video_ren != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_video_ren);
        m_p_mutex_video_ren = NULL;
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

    if(m_p_cond_audio_ren != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_audio_ren);
        m_p_cond_audio_ren = NULL;
    }
    
    if(m_p_mutex_audio_ren != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_audio_ren);
        m_p_mutex_audio_ren = NULL;
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

void CMmpPlayerAVEx1::Service_AudioDec() {

    MMP_RESULT mmpResult;
    CMmpMediaSample MediaSampleObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    MMP_U32 decoded_buf_max_size = 1024*1024;
    MMP_U8* stream_buf, *pbuf;
    MMP_S32 remain_buf_size;
 
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
            pbuf = stream_buf;
            while(remain_buf_size > 0) {

                pMediaSample->pAu = pbuf;
                pMediaSample->uiAuSize = remain_buf_size;
                pMediaSample->uiAuMaxSize = remain_buf_size;

                pDecResult->uiDecodedSize = 0;
                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (MMP_U32)MMP_PLAYER_MALLOC(decoded_buf_max_size);
                pDecResult->uiDecodedBufferMaxSize = decoded_buf_max_size;
                mmpResult = m_pDecoderAudio->DecodeAu(pMediaSample, pDecResult);
                if( (mmpResult==MMP_SUCCESS) && (pDecResult->uiDecodedSize>0) ) {
                    m_p_mutex_audio_ren->lock();
                    m_queue_audio_pcm.Add(*pDecResult);
                    m_p_mutex_audio_ren->unlock();
               // m_p_cond_video_ren->signal(); //signal
                }
                else {
                    MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
                }

                remain_buf_size -= pDecResult->uiAuUsedByte;
                pbuf += pDecResult->uiAuUsedByte;
            }

            MMP_PLAYER_FREE(stream_buf);
        }
    }

}


void CMmpPlayerAVEx1::Service_VideoDec() {

    MMP_RESULT mmpResult;
    CMmpMediaSample MediaSampleObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    MMP_U32 buffer_width, buffer_height;

    buffer_width = MMP_BYTE_ALIGN(m_pDemuxer->GetVideoPicWidth(), 16);
    buffer_height = MMP_BYTE_ALIGN(m_pDemuxer->GetVideoPicHeight(), 16);


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
        
            mmpResult = m_pDecoderVideo->DecodeAu(pMediaSample, pDecResult);
            if( (mmpResult==MMP_SUCCESS) && (pDecResult->uiDecodedSize>0) ) {

                m_p_mutex_video_ren->lock();
                m_queue_video_yuv.Add(*pDecResult);
                m_p_mutex_video_ren->unlock();
               // m_p_cond_video_ren->signal(); //signal
            }
            else {

                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                MMP_PLAYER_FREE((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);
            }
        

            MMP_PLAYER_FREE(pMediaSample->pAu);
        }
    }

}

#if 1
void CMmpPlayerAVEx1::Service() {

    MMP_BOOL bFreeAu;
    MMP_RESULT mmpResult;
    MMP_U32 stream_buf_max_size = 1024*1024;
    MMP_BOOL bRenderAudio;

    CMmpMediaSample MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObjAudio;
    CMmpMediaSampleDecodeResult DecResultObjVideo;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult* pDecResultAudio = &DecResultObjAudio;
    CMmpMediaSampleDecodeResult* pDecResultVideo = &DecResultObjVideo;
    
    MMP_U32 start_tick, cur_tick;
    MMP_U64 playtime;
    
    MMP_U32 video_render_count = 0 , audio_render_count = 0;

    start_tick = CMmpUtil::GetTickCount();

    while(m_bServiceRun == MMP_TRUE) {

        MMPDEBUGMSG(1, (TEXT("[AV Player] ln=%d. Qu(%d %d) "), 
                    __LINE__, 
                    m_queue_audio_pcm.GetSize(), m_queue_video_yuv.GetSize()
                    ));

        //if( ((m_queue_video_stream.GetSize() < 5) || (m_queue_audio_stream.GetSize() < 15) )) 
        //  && ((m_queue_video_yuv.GetSize() < 5) || (m_queue_audio_pcm.GetSize() < 15)) ) 
        //  && ((m_queue_video_yuv.GetSize() < 5) || (m_queue_audio_pcm.GetSize() < 15)) ) 
        if( (m_queue_video_yuv.GetSize() < 3)  || (m_queue_audio_pcm.GetSize() < 5) ) 

        //if( (m_queue_video_yuv.GetSize()==0) 
        //    || (m_queue_audio_pcm.GetSize()==0) 
        //    || ( (m_queue_video_yuv.GetSize() < 5) && (m_queue_audio_pcm.GetSize() < 15) ) 
        //    )
        {
            bFreeAu = MMP_TRUE;
            pMediaSample->pAu = (MMP_U8*)MMP_PLAYER_MALLOC(stream_buf_max_size);
            pMediaSample->uiAuMaxSize = stream_buf_max_size;
            pMediaSample->uiFlag = 0;

            if(pMediaSample->pAu != NULL) {
                mmpResult = m_pDemuxer->GetNextData(pMediaSample);
                if(mmpResult == MMP_SUCCESS) {
                
                    if(pMediaSample->uiMediaType == MMP_MEDIATYPE_VIDEO) {
#if 1
                        m_p_mutex_video_dec->lock();
                        m_queue_video_stream.Add(*pMediaSample);
                        m_p_mutex_video_dec->unlock();
                        m_p_cond_video_dec->signal(); //signal
                        bFreeAu = MMP_FALSE;

                        MMPDEBUGMSG(1, (TEXT("[AV Player] ln=%d. Video Input Qu(%d %d) "), 
                            __LINE__, 
                            m_queue_audio_pcm.GetSize(), m_queue_video_yuv.GetSize()
                        ));
#endif
                    }
                    else if(pMediaSample->uiMediaType == MMP_MEDIATYPE_AUDIO) {

#if 1
                        m_p_mutex_audio_dec->lock();
                        m_queue_audio_stream.Add(*pMediaSample);
                        m_p_mutex_audio_dec->unlock();
                        m_p_cond_audio_dec->signal(); //signal
                        bFreeAu = MMP_FALSE;

                        MMPDEBUGMSG(1, (TEXT("[AV Player] ln=%d. Audio Input Qu(%d %d) "), 
                            __LINE__, 
                            m_queue_audio_pcm.GetSize(), m_queue_video_yuv.GetSize()
                        ));
#endif
                    }
                }
            }
        
            if(bFreeAu == MMP_TRUE) {
                MMP_PLAYER_FREE(pMediaSample->pAu);
            }

        }

#if 0
        /* rendering video */
        pDecResultVideo->uiDecodedSize = 0;
        m_p_mutex_video_ren->lock();
        if(!m_queue_video_yuv.IsEmpty()) {
            m_queue_video_yuv.Delete(*pDecResultVideo);
        }
        m_p_mutex_video_ren->unlock();
        if(pDecResultVideo->uiDecodedSize > 0) {
            m_pRendererVideo->Render(pDecResultVideo);

            MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
            MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
            MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);
        }

        /* rendering audio */
        pDecResultAudio->uiDecodedSize = 0;
        m_p_mutex_audio_ren->lock();
        if(!m_queue_audio_pcm.IsEmpty()) {
            m_queue_audio_pcm.Delete(*pDecResultAudio);
        }
        m_p_mutex_audio_ren->unlock();
        if(pDecResultAudio->uiDecodedSize > 0) {
            m_pRendererAudio->Render(pDecResultAudio);
            MMP_PLAYER_FREE((void*)pDecResultAudio->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
            
        }
#else

        while(1) {
        
            pDecResultVideo->uiDecodedSize = 0;
            pDecResultAudio->uiDecodedSize = 0;

            m_p_mutex_video_ren->lock();
            if(!m_queue_video_yuv.IsEmpty()) {
                m_queue_video_yuv.GetFirstItem(*pDecResultVideo);
            }
            m_p_mutex_video_ren->unlock();

            m_p_mutex_audio_ren->lock();
            if(!m_queue_audio_pcm.IsEmpty()) {
                m_queue_audio_pcm.GetFirstItem(*pDecResultAudio);
            }
            m_p_mutex_audio_ren->unlock();


            if( (pDecResultVideo->uiDecodedSize > 0)  && (pDecResultAudio->uiDecodedSize > 0) ) {
            
                
                cur_tick = CMmpUtil::GetTickCount();
                playtime = (MMP_U64)(cur_tick - start_tick)*1000;
                
                if( (pDecResultVideo->uiTimeStamp + 1000*200) < (pDecResultAudio->uiTimeStamp) ) {

                    MMPDEBUGMSG(1, (TEXT("[AV Player] %d. TS(%d %d) Qu(%d %d)  Video  Cnt(%d, %d) "), 
                    cur_tick-start_tick, 
                    (unsigned int)(pDecResultAudio->uiTimeStamp/1000), (unsigned int)(pDecResultVideo->uiTimeStamp/1000) ,
                    m_queue_audio_pcm.GetSize(), m_queue_video_yuv.GetSize(),
                    audio_render_count, video_render_count
                    ));
                    
                    if(pDecResultVideo->uiTimeStamp < playtime ) { 

                        if( (playtime-pDecResultVideo->uiTimeStamp) < (500*1000) ) {
                            m_pRendererVideo->Render(pDecResultVideo);
                        }
                        else {
                            MMPDEBUGMSG(1, (TEXT("[AV Player] Video Skip diff : %d ms"), (unsigned int)(playtime-pDecResultVideo->uiTimeStamp)/1000 ));
                        }
                    
                        m_p_mutex_video_ren->lock();
                        m_queue_video_yuv.Delete(*pDecResultVideo);
                        m_p_mutex_video_ren->unlock();

                        MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                        MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                        MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);

                        video_render_count++;
                    }
                    else {
                        CMmpUtil::Sleep(1);
                    }

                }
                else {

                    MMPDEBUGMSG(1, (TEXT("[AV Player] %d. TS(%d %d) Qu(%d %d)  Audio  Cnt(%d %d)"), 
                    cur_tick-start_tick, 
                    (unsigned int)(pDecResultAudio->uiTimeStamp/1000), (unsigned int)(pDecResultVideo->uiTimeStamp/1000) ,
                    m_queue_audio_pcm.GetSize(), m_queue_video_yuv.GetSize(),
                    audio_render_count, video_render_count
                    ));

                    if(pDecResultAudio->uiTimeStamp < (playtime+200*1000) ) { 

                        m_pRendererAudio->Render(pDecResultAudio);

                        m_p_mutex_audio_ren->lock();
                        m_queue_audio_pcm.Delete(*pDecResultAudio);
                        m_p_mutex_audio_ren->unlock();

                        MMP_PLAYER_FREE((void*)pDecResultAudio->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
                     
                        audio_render_count++;
                    }
                    else {
                        CMmpUtil::Sleep(1);
                    }

                }
            
            }
            else if(pDecResultVideo->uiDecodedSize > 0) {
            
                cur_tick = CMmpUtil::GetTickCount();
                playtime = (MMP_U64)(cur_tick - start_tick)*1000;

                if(pDecResultVideo->uiTimeStamp < playtime ) { 

                    if( (playtime-pDecResultVideo->uiTimeStamp) < (500*1000) ) {
                        m_pRendererVideo->Render(pDecResultVideo);
                    }
                    else {
                        MMPDEBUGMSG(1, (TEXT("[AV Player] Video Skip diff : %d ms"), (unsigned int)(playtime-pDecResultVideo->uiTimeStamp)/1000 ));
                    }
                
                    m_p_mutex_video_ren->lock();
                    m_queue_video_yuv.Delete(*pDecResultVideo);
                    m_p_mutex_video_ren->unlock();

                    MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y]);
                    MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U]);
                    MMP_PLAYER_FREE((void*)pDecResultVideo->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V]);

                    video_render_count++;
                }
                else {
                    CMmpUtil::Sleep(1);
                }
                break;
            }
            else if(pDecResultAudio->uiDecodedSize > 0) {
            
                cur_tick = CMmpUtil::GetTickCount();
                playtime = (MMP_U64)(cur_tick - start_tick)*1000;
            
                if(pDecResultAudio->uiTimeStamp < (playtime+200*1000) ) { 

                    m_pRendererAudio->Render(pDecResultAudio);

                    m_p_mutex_audio_ren->lock();
                    m_queue_audio_pcm.Delete(*pDecResultAudio);
                    m_p_mutex_audio_ren->unlock();

                    MMP_PLAYER_FREE((void*)pDecResultAudio->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM]);
                 
                    audio_render_count++;
                }
                else {
                    CMmpUtil::Sleep(1);
                }
            
                break;
            }
            else {

                break;
            }
        
            CMmpUtil::Sleep(1);
        }


#endif

        
    }

}

#else
void CMmpPlayerAVEx1::Service()
{
    this->Service_AV_Simple(m_pDemuxer, m_pDecoderAudio, m_pDecoderVideo, m_pRendererAudio, m_pRendererVideo);

}
#endif