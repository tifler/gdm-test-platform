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

#include "MmpPlayerVideoEx1.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerVideoEx1 Member Functions

CMmpPlayerVideoEx1::CMmpPlayerVideoEx1(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)
,m_pDemuxer(NULL)
,m_pDecoderVideo(NULL)
,m_pRendererVideo(NULL)

,m_buffer_width(0)
,m_buffer_height(0)

,m_last_packet_pts(0)
,m_fps(0)

,m_p_buf_videostream(NULL)

{
    

}

CMmpPlayerVideoEx1::~CMmpPlayerVideoEx1()
{
    
}

MMP_RESULT CMmpPlayerVideoEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct MmpDemuxerCreateConfig demuxer_create_config;
    struct MmpDecoderCreateConfig decoder_create_config;
   
    CMmpRendererCreateProp* pRendererProp=&m_RendererProp; 
    MMP_S32 stream_buf_max_size;
    
    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        strcpy((char*)demuxer_create_config.filename, this->m_create_config.filename);
        m_pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  CMmpDemuxer::CreateObject ")));
        }
        else {
            stream_buf_max_size = m_pDemuxer->GetVideoPicWidth()*m_pDemuxer->GetVideoPicHeight()*3/2;
        }
    }

    /* create stream buffer */
    if(mmpResult == MMP_SUCCESS ) {
        m_p_buf_videostream = mmp_buffer_mgr::get_instance()->alloc_media_videostream(stream_buf_max_size, mmp_buffer::HEAP);
        if(m_p_buf_videostream == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  mmp_buffer_mgr::get_instance()->alloc_media_videostream(%d) "), stream_buf_max_size));
            mmpResult = MMP_FAILURE;   
        }
    }

#if 1
    /* create video decoder */
    if(mmpResult == MMP_SUCCESS ) {

        memset(&decoder_create_config, 0x00, sizeof(decoder_create_config));
        decoder_create_config.nFormat = m_pDemuxer->GetVideoFormat();
        decoder_create_config.nPicWidth = m_pDemuxer->GetVideoPicWidth();
        decoder_create_config.nPicHeight = m_pDemuxer->GetVideoPicHeight();

        m_buffer_width = MMP_BYTE_ALIGN(decoder_create_config.nPicWidth, 16);
        m_buffer_height = MMP_BYTE_ALIGN(decoder_create_config.nPicHeight, 16);

        m_pDecoderVideo = (CMmpDecoderVideo*)CMmpDecoder::CreateVideoObject(&decoder_create_config, m_create_config.bForceSWCodec /*MMP_FALSE*/ /* MMP_TRUE: FFmpeg Force Use */);
        if(m_pDecoderVideo == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(0, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  CMmpDecoder::CreateVideoObject  %c%c%c%c %dx%d"), 
                    MMPGETFOURCC(decoder_create_config.nFormat, 0),MMPGETFOURCC(decoder_create_config.nFormat, 1),
                    MMPGETFOURCC(decoder_create_config.nFormat, 2),MMPGETFOURCC(decoder_create_config.nFormat, 3),

                    decoder_create_config.nPicWidth, decoder_create_config.nPicHeight
                ));
        }
        else {

            m_pDemuxer->GetVideoExtraData(m_p_buf_videostream);
            if(m_p_buf_videostream->get_stream_size() > 0) {

                m_p_buf_videostream->set_flag(MMP_MEDIASAMPMLE_FLAG_CONFIGDATA);
                m_pDecoderVideo->DecodeAu(m_p_buf_videostream, NULL);
            }
        }

    }
#endif

#if 1
    /* create video renderer */
    if(mmpResult == MMP_SUCCESS ) {
        
        pRendererProp->m_hRenderWnd = this->m_create_config.video_config.m_hRenderWnd;
        pRendererProp->m_hRenderDC = this->m_create_config.video_config.m_hRenderDC;
        pRendererProp->m_renderer_type = this->m_create_config.video_config.m_renderer_type;
        
        pRendererProp->m_bVideoEncoder = this->m_create_config.video_config.m_bVideoEncoder;
        strcpy(pRendererProp->m_VideoEncFileName, this->m_create_config.video_config.m_VideoEncFileName);
        pRendererProp->m_VideoEncoderCreateConfig = this->m_create_config.video_config.m_VideoEncoderCreateConfig;
        pRendererProp->m_bVideoEncoderForceSWCodec = this->m_create_config.video_config.m_bVideoEncoderForceSWCodec;

        pRendererProp->m_iBoardWidth = 1920;
        pRendererProp->m_iBoardHeight = 1080;

#if (MMP_OS == MMP_OS_WIN32)
        HWND hWnd = (HWND)pRendererProp->m_hRenderWnd;
        RECT rect;

	    ::GetWindowRect(hWnd, &rect);

        pRendererProp->m_iScreenPosX = rect.left;
        pRendererProp->m_iScreenPosY = rect.top;
        pRendererProp->m_iScreenWidth = (rect.right-rect.left)/2;
        pRendererProp->m_iScreenHeight = (rect.bottom-rect.top)/2;
#endif


        pRendererProp->m_iPicWidth = m_pDemuxer->GetVideoPicWidth();
        pRendererProp->m_iPicHeight = m_pDemuxer->GetVideoPicHeight();

        m_pRendererVideo = CMmpRenderer::CreateVideoObject(pRendererProp);
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  CMmpRenderer::CreateVideoObject ")));
        }
    }
#endif

    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }

    return mmpResult;
}


MMP_RESULT CMmpPlayerVideoEx1::Close()
{
    CMmpPlayer::Close();

    if(m_pDemuxer != NULL) {
        CMmpDemuxer::DestroyObject(m_pDemuxer);  
        m_pDemuxer = NULL;
    }


    if(m_pDecoderVideo != NULL) {
        CMmpDecoder::DestroyObject(m_pDecoderVideo);  
        m_pDecoderVideo = NULL;
    }


    if(m_pRendererVideo != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererVideo);  
        m_pRendererVideo = NULL;
    }
    
    /* destoy stream buffer */
    if(m_p_buf_videostream != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videostream);
        m_p_buf_videostream = NULL;
    }

    return MMP_SUCCESS;
}

MMP_S64 CMmpPlayerVideoEx1::GetDuration() {

    MMP_S64 dur = 0;
    if(m_pDemuxer != NULL) {
       dur = m_pDemuxer->GetDuration();
    }
    return dur;
}
    
MMP_S32 CMmpPlayerVideoEx1::GetPlayFPS() {
    return m_fps;
}

MMP_S64 CMmpPlayerVideoEx1::GetPlayPosition() {
    return m_last_packet_pts;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoDecoderFPS() {

    MMP_S32 fps = 0;
    
    if(m_pDecoderVideo != NULL) {
        fps = m_pDecoderVideo->GetAvgFPS();
    }

    return fps;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoDecoderDur() {

    MMP_S32 dur = 0;
    
    if(m_pDecoderVideo != NULL) {
        dur = m_pDecoderVideo->GetAvgDur();
    }

    return dur;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoDecoderTotalDecFrameCount() {

    MMP_S32 cnt = 0;
    
    if(m_pDecoderVideo != NULL) {
        cnt = m_pDecoderVideo->GetTotalDecFrameCount();
    }

    return cnt;
}

const MMP_CHAR* CMmpPlayerVideoEx1::GetVideoDecoderClassName() {

    return m_pDecoderVideo->GetClassName();
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoWidth() {
    MMP_S32 val = 0;
    if(m_pDemuxer != NULL) {
       val = m_pDemuxer->GetVideoPicWidth();
    }
    return val;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoHeight() {
    MMP_S32 val = 0;
    if(m_pDemuxer != NULL) {
       val = m_pDemuxer->GetVideoPicHeight();
    }
    return val;
}

MMP_U32 CMmpPlayerVideoEx1::GetVideoFormat(){
    MMP_U32 val = 0;
    if(m_pDemuxer != NULL) {
       val = m_pDemuxer->GetVideoFormat();
    }
    return val;
}

/* Video Renderer */
void CMmpPlayerVideoEx1::SetFirstVideoRenderer() {

    if(m_pRendererVideo != NULL) {
        m_pRendererVideo->SetFirstRenderer();
    }
}

MMP_BOOL CMmpPlayerVideoEx1::IsFirstVideoRenderer() {

    MMP_BOOL bFlag = MMP_FALSE;

    if(m_pRendererVideo != NULL) {
        bFlag = m_pRendererVideo->IsFirstRenderer();
    }

    return bFlag;
}

void CMmpPlayerVideoEx1::SetVideoRotate(enum MMP_ROTATE rotate) {

    if(m_pRendererVideo != NULL) {
        m_pRendererVideo->SetRotate(rotate);
    }

}

void CMmpPlayerVideoEx1::Service()
{
    MMP_U32 frame_count = 0;
    FILE* dump_fp = NULL;
    MMP_S32 fps = 0;
    MMP_U32 cur_tick, before_tick, t1, t2;
    class mmp_buffer_videoframe* p_buf_videoframe;
    
#if 0
#if (MMP_OS == MMP_OS_WIN32)
    dump_fp = fopen("d:\\work\\h264_BP_FullHD.h264", "wb");
#endif
#endif
    
    before_tick = CMmpUtil::GetTickCount();

    while(m_bServiceRun == MMP_TRUE) {

        t1 = CMmpUtil::GetTickCount();
                
        m_pDemuxer->GetNextVideoData(this->m_p_buf_videostream);
        if(this->m_p_buf_videostream->get_stream_size() > 0) {

            if(dump_fp != NULL) {
                fwrite((void*)m_p_buf_videostream->get_buf_vir_addr(), 1,  m_p_buf_videostream->get_stream_size(), dump_fp);
            }

            m_p_buf_videostream->set_flag(0);
            
            m_last_packet_pts = m_p_buf_videostream->get_pts();
            m_pDecoderVideo->DecodeAu(m_p_buf_videostream, &p_buf_videoframe);

            if(p_buf_videoframe != NULL) {
                m_pRendererVideo->Render(p_buf_videoframe);
            }

            fps++;
        }

        frame_count++;

        t2 = CMmpUtil::GetTickCount();
        
        cur_tick = t2;    
        if( (cur_tick - before_tick) > 1000 ) {
            m_fps = fps;
            fps = 0;

            if(this->m_create_config.callback != NULL) {

                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                void *data1 = NULL, *data2 = NULL;

                msg = CMmpPlayer::CALLBACK_PLAYTIME;
                playtime_st.media_duration  = m_pDemuxer->GetDuration();
                playtime_st.media_pts = this->m_p_buf_videostream->get_pts();

                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }

            before_tick = cur_tick;
        }
        
        if( (t2-t1) < 20 ) {
            CMmpUtil::Sleep(30);
        }
        else {
            CMmpUtil::Sleep(10);
        }

    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    if(dump_fp != NULL) {
        fclose(dump_fp);
    }
    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideoEx1::Service] Task Ended!!")));
}

