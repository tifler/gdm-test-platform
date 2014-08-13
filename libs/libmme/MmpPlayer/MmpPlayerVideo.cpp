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

#include "MmpPlayerVideo.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerVideo Member Functions

CMmpPlayerVideo::CMmpPlayerVideo(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)
,m_pDemuxer(NULL)
,m_pDecoderVideo(NULL)
,m_pRendererVideo(NULL)

,m_buffer_width(0)
,m_buffer_height(0)

,m_stream_buffer(NULL)
,m_stream_buffer_max_size(0)

,m_Y(NULL)
,m_U(NULL)
,m_V(NULL)
{
    

}

CMmpPlayerVideo::~CMmpPlayerVideo()
{
    
}

MMP_RESULT CMmpPlayerVideo::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    struct MmpDemuxerCreateConfig demuxer_create_config;
    struct MmpDecoderCreateConfig decoder_create_config;

    
    CMmpRendererCreateProp* pRendererProp=&m_RendererProp; 

    CMmpMediaSample *pMediaSample = &m_MediaSample;
    CMmpMediaSampleDecodeResult* pDecResult = &m_DecResult;
    MMP_U32 stream_buf_size;
    
    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        strcpy((char*)demuxer_create_config.filename, this->m_create_config.filename);
        m_pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
            m_stream_buffer_max_size = m_pDemuxer->GetVideoPicWidth()*m_pDemuxer->GetVideoPicHeight()*3/2;
            m_stream_buffer = (MMP_U8*)malloc(m_stream_buffer_max_size);
            if(m_stream_buffer == NULL) {
                mmpResult = MMP_FAILURE;    
            }
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
        }
        else {

            m_Y = (MMP_U8*)malloc(m_buffer_width * m_buffer_height);
            m_U = (MMP_U8*)malloc(m_buffer_width * m_buffer_height);
            m_V = (MMP_U8*)malloc(m_buffer_width * m_buffer_height);
            
            stream_buf_size = 0;
            m_pDemuxer->GetVideoExtraData(m_stream_buffer, m_stream_buffer_max_size, &stream_buf_size);
            if(stream_buf_size > 0) {
                pMediaSample->pAu = m_stream_buffer;
                pMediaSample->uiAuSize = stream_buf_size;
                pMediaSample->uiSampleNumber = 0;
                pMediaSample->uiTimeStamp = 0;
                pMediaSample->uiFlag = MMP_MEDIASAMPMLE_FLAG_CONFIGDATA;

                pDecResult->uiDecodedBufferLogAddr[0] = (MMP_U32)m_Y;
                pDecResult->uiDecodedBufferLogAddr[1] = (MMP_U32)m_U;
                pDecResult->uiDecodedBufferLogAddr[2] = (MMP_U32)m_V;
                pDecResult->uiDecodedBufferStride[0] = m_buffer_width;
                pDecResult->uiDecodedBufferStride[1] = m_buffer_width/2;
                pDecResult->uiDecodedBufferStride[2] = m_buffer_width/2;

                m_pDecoderVideo->DecodeAu(pMediaSample, pDecResult);
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
        }
    }
#endif

    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }

    return mmpResult;
}


MMP_RESULT CMmpPlayerVideo::Close()
{
    CMmpPlayer::Close();

    CMmpDemuxer::DestroyObject(m_pDemuxer);  m_pDemuxer = NULL;
    CMmpDecoder::DestroyObject(m_pDecoderVideo);  m_pDecoderVideo = NULL;
    CMmpRenderer::DestroyObject(m_pRendererVideo);  m_pRendererVideo = NULL;
    
    if(m_Y!=NULL) { free(m_Y); m_Y=NULL; }
    if(m_U!=NULL) { free(m_U); m_U=NULL; }
    if(m_V!=NULL) { free(m_V); m_V=NULL; }

    if(m_stream_buffer!=NULL) { free(m_stream_buffer); m_stream_buffer = NULL; }
    
    return MMP_SUCCESS;
}

void CMmpPlayerVideo::Service()
{
    MMP_RESULT mmpResult;
    CMmpMediaSample *pMediaSample = &m_MediaSample;
    CMmpMediaSampleDecodeResult* pDecResult = &m_DecResult;
    MMP_U32 stream_buf_size;
    MMP_U32 frame_count = 0;
    MMP_S64 packet_pts;
    FILE* dump_fp = NULL;

#if 1
#if (MMP_OS == MMP_OS_WIN32)
    dump_fp = fopen("d:\\work\\h264_BP_FullHD.h264", "wb");
#endif
#endif
    
    
    while(m_bServiceRun == MMP_TRUE) {
    
        mmpResult = m_pDemuxer->GetNextVideoData(m_stream_buffer, m_stream_buffer_max_size, &stream_buf_size, &packet_pts);
        if(mmpResult == MMP_SUCCESS) {

            if(dump_fp != NULL) {
                fwrite(m_stream_buffer, 1, stream_buf_size, dump_fp);
            }
        
            pMediaSample->pAu = m_stream_buffer;
            pMediaSample->uiAuSize = stream_buf_size;
            pMediaSample->uiSampleNumber = 0;
            pMediaSample->uiTimeStamp = 0;
            pMediaSample->uiFlag = 0;

            pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y] = (MMP_U32)m_Y;
            pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U] = (MMP_U32)m_U;
            pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V] = (MMP_U32)m_V;
            pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y] = m_buffer_width;
            pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_U] = m_buffer_width/2;
            pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_V] = m_buffer_width/2;
            pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_Y] = m_buffer_height;
            pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_U] = m_buffer_height/2;
            pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_V] = m_buffer_height/2;
            pDecResult->uiDecodedBufferMaxSize = m_buffer_width*m_buffer_height*3/2;
                    
            m_pDecoderVideo->DecodeAu(pMediaSample, pDecResult);
            if(pDecResult->uiDecodedSize > 0) {
                m_pRendererVideo->RenderYUV420Planar(m_Y, m_U, m_V,m_buffer_width, m_buffer_height);
            }
        }

        frame_count++;

        //if(frame_count > 120) {
        //    break;
        //}

        CMmpUtil::Sleep(1);
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    if(dump_fp != NULL) {
        fclose(dump_fp);
    }
    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideo::Service] Task Ended!!")));
}

