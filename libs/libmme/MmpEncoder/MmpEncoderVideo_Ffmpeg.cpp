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

#include "MmpEncoderVideo_Ffmpeg.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo_Ffmpeg Member Functions

CMmpEncoderVideo_Ffmpeg::CMmpEncoderVideo_Ffmpeg(struct MmpEncoderCreateConfig *pCreateConfig) : CMmpEncoderVideo(pCreateConfig, MMP_FALSE), CMmpEncoderFfmpeg(pCreateConfig)
,m_temp_picture_buffer(NULL)
,m_nEncodedStreamCount(0)
{
    
}

CMmpEncoderVideo_Ffmpeg::~CMmpEncoderVideo_Ffmpeg()
{

}

MMP_RESULT CMmpEncoderVideo_Ffmpeg::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpEncoderFfmpeg::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    
    m_temp_picture_buffer = new MMP_U8[1920*1088*3/2];

    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoderVideo_Ffmpeg::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderFfmpeg::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Close] CMmpEncoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpEncoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Close] CMmpEncoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    if(m_temp_picture_buffer != NULL) {
        delete [] m_temp_picture_buffer;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVideo_Ffmpeg::EncodeAu(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

    MMP_RESULT mmpResult = MMP_SUCCESS; 
    AVPacket avpkt;
    int got_packet_ptr;
    int iret;
    MMP_U32 enc_start_tick, enc_end_tick;

    MMP_U8* pBuffer;
    MMP_U32 nBufSize, nBufMaxSize, nFlag;

    enc_start_tick = CMmpUtil::GetTickCount();

    pEncResult->uiEncodedStreamSize[0] = 0;
    pEncResult->uiEncodedStreamSize[1] = 0;
    pEncResult->uiTimeStamp = pMediaSample->uiTimeStamp;
    pEncResult->uiFlag = 0;
    
    if(m_pAVCodecContext == NULL) {
    
        mmpResult = CMmpEncoderFfmpeg::EncodeDSI();
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }

    av_init_packet (&avpkt);
    avpkt.data = NULL;
    avpkt.size = 0;
    avpkt.flags = 0;
    
    m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] = (uint8_t*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y];
    m_pAVFrame_Input->data[MMP_DECODED_BUF_U] = (uint8_t*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U];
    m_pAVFrame_Input->data[MMP_DECODED_BUF_V] = (uint8_t*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_V];

#if 0 /* test uv stride */
    memset((void*)m_pAVFrame_Input->data[MMP_DECODED_BUF_U], 128, m_bih_in.biWidth*m_bih_in.biHeight/4);
    memset((void*)m_pAVFrame_Input->data[MMP_DECODED_BUF_V], 128, m_bih_in.biWidth*m_bih_in.biHeight/4);
    int ssz;
    int chroma_width, chroma_height;
    chroma_width = m_bih_in.biWidth/2;
    chroma_height = m_bih_in.biHeight/2;
    ssz = chroma_width*chroma_height/2;
    memset((void*)m_pAVFrame_Input->data[MMP_DECODED_BUF_U], 0, ssz);
    memset((void*)m_pAVFrame_Input->data[MMP_DECODED_BUF_V], 0, ssz);
#endif
    
    if(m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] == NULL) {
    
        m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] = (uint8_t*)m_temp_picture_buffer;
        m_pAVFrame_Input->data[MMP_DECODED_BUF_U] = (uint8_t*)m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] + m_bih_in.biWidth*m_bih_in.biHeight;
        m_pAVFrame_Input->data[MMP_DECODED_BUF_V] = (uint8_t*)m_pAVFrame_Input->data[MMP_DECODED_BUF_U] + m_bih_in.biWidth*m_bih_in.biHeight/4;
    }
    m_pAVFrame_Input->linesize[MMP_DECODED_BUF_Y] = pMediaSample->uiBufferStride[MMP_DECODED_BUF_Y];
    m_pAVFrame_Input->linesize[MMP_DECODED_BUF_U] = pMediaSample->uiBufferStride[MMP_DECODED_BUF_U];
    m_pAVFrame_Input->linesize[MMP_DECODED_BUF_V] = pMediaSample->uiBufferStride[MMP_DECODED_BUF_V];
    m_pAVFrame_Input->format = m_pAVCodecContext->pix_fmt;
    m_pAVFrame_Input->width = m_pAVCodecContext->width;
    m_pAVFrame_Input->height = m_pAVCodecContext->height;
    m_pAVFrame_Input->pts = AV_NOPTS_VALUE;
    m_pAVFrame_Input->quality = 100;

    iret = avcodec_encode_video2(m_pAVCodecContext, &avpkt, m_pAVFrame_Input, &got_packet_ptr);
    if(iret == 0) /* Success */ {
        
        /* get the delayed frames */
        if(got_packet_ptr == 0) {

             iret = avcodec_encode_video2(m_pAVCodecContext, &avpkt, NULL, &got_packet_ptr);
             if (iret < 0) {
                got_packet_ptr = 0;    
             }
        }
         
        if(got_packet_ptr == 1) {
            
            pBuffer = (MMP_U8*)pEncResult->uiEncodedBufferLogAddr[MMP_ENCODED_BUF_STREAM];
            nBufMaxSize = pEncResult->uiEncodedBufferMaxSize[MMP_ENCODED_BUF_STREAM];

            if(m_nEncodedStreamCount == 0) {

                this->EncodedFrameQueue_AddFrameWithConfig_Mpeg4(avpkt.data, avpkt.size, (avpkt.flags&AV_PKT_FLAG_KEY)?MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME:0);

                if(this->EncodedFrameQueue_IsEmpty() != MMP_TRUE) {

                    this->EncodedFrameQueue_GetFrame(pBuffer, nBufMaxSize, &nBufSize, &nFlag);

                    pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nBufSize;
                    pEncResult->uiFlag |= nFlag;

                    m_nEncodedStreamCount++;
                }

            }
            else if(avpkt.size <= (int)nBufMaxSize) {

                memcpy((void*)pBuffer, avpkt.data, avpkt.size);
                pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = avpkt.size;

                if(avpkt.flags&AV_PKT_FLAG_KEY) {
                    pEncResult->uiFlag |= MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME;
                }

                m_nEncodedStreamCount++;
            }
        }
        
        mmpResult = MMP_SUCCESS;   
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    av_free_packet(&avpkt);

    enc_end_tick = CMmpUtil::GetTickCount();

    pEncResult->uiEncodedDuration = enc_end_tick - enc_start_tick;

    CMmpEncoderVideo::EncodeMonitor(pMediaSample, pEncResult);

    return mmpResult;
}

MMP_RESULT CMmpEncoderVideo_Ffmpeg::EncodeAu(class mmp_buffer_videoframe* p_buf_videoframe, class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_SUCCESS; 
    AVPacket avpkt;
    int got_packet_ptr;
    int iret;
    MMP_U32 i;
    MMP_U32 enc_start_tick, enc_end_tick;

    MMP_U8* pBuffer;
    MMP_U32 nBufSize, nBufMaxSize, nFlag;

    enc_start_tick = CMmpUtil::GetTickCount();

    p_buf_videostream->set_stream_size(0);
    p_buf_videostream->set_stream_offset(0);
    p_buf_videostream->set_flag(0);
    p_buf_videostream->set_pts(p_buf_videoframe->get_pts());
    p_buf_videostream->set_dsi_size(0);

    if(m_pAVCodecContext == NULL) {
        mmpResult = CMmpEncoderFfmpeg::EncodeDSI();
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }

    av_init_packet (&avpkt);
    avpkt.data = NULL;
    avpkt.size = 0;
    avpkt.flags = 0;
    
    for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
        m_pAVFrame_Input->data[i] = (uint8_t*)p_buf_videoframe->get_buf_vir_addr(i);
        m_pAVFrame_Input->linesize[i] = p_buf_videoframe->get_buf_stride(i);
    }
    m_pAVFrame_Input->format = m_pAVCodecContext->pix_fmt;
    m_pAVFrame_Input->width = m_pAVCodecContext->width;
    m_pAVFrame_Input->height = m_pAVCodecContext->height;
    m_pAVFrame_Input->pts = AV_NOPTS_VALUE;
    m_pAVFrame_Input->quality = 100;

    iret = avcodec_encode_video2(m_pAVCodecContext, &avpkt, m_pAVFrame_Input, &got_packet_ptr);
    if(iret == 0) /* Success */ {
        
        /* get the delayed frames */
        if(got_packet_ptr == 0) {

             iret = avcodec_encode_video2(m_pAVCodecContext, &avpkt, NULL, &got_packet_ptr);
             if (iret < 0) {
                got_packet_ptr = 0;    
             }
        }
         
        if(got_packet_ptr == 1) {
            
            pBuffer = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
            nBufMaxSize = p_buf_videostream->get_buf_size();

            if( (m_nEncodedStreamCount == 0)  && (m_CreateConfig.nFormat == MMP_FOURCC_VIDEO_MPEG4) ) {

                this->EncodedFrameQueue_AddFrameWithConfig_Mpeg4(avpkt.data, avpkt.size, (avpkt.flags&AV_PKT_FLAG_KEY)?MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME:0);

                if(this->EncodedFrameQueue_IsEmpty() != MMP_TRUE) {

                    this->EncodedFrameQueue_GetFrame(pBuffer, nBufMaxSize, &nBufSize, &nFlag);

                    /* get dsi */
                    p_buf_videostream->alloc_dsi_buffer(nBufSize);
                    p_buf_videostream->set_dsi_size(nBufSize);
                    memcpy(p_buf_videostream->get_dsi_buffer(), pBuffer, nBufSize);

                    /* get frame */
                    this->EncodedFrameQueue_GetFrame(pBuffer, nBufMaxSize, &nBufSize, &nFlag);
                    p_buf_videostream->set_stream_size(nBufSize);
                    p_buf_videostream->or_flag(nFlag);
                    
                    //pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nBufSize;
                    //pEncResult->uiFlag |= nFlag;

                    m_nEncodedStreamCount++;
                }

            }
            else if(avpkt.size <= (int)nBufMaxSize) {

                memcpy((void*)pBuffer, avpkt.data, avpkt.size);
                p_buf_videostream->set_stream_size(avpkt.size);
                //pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = avpkt.size;

                if(avpkt.flags&AV_PKT_FLAG_KEY) {
                    //pEncResult->uiFlag |= MMP_ENCODED_FLAG_VIDEO_KEYFRAME;
                    p_buf_videostream->or_flag(MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME);
                }

                m_nEncodedStreamCount++;
            }
        }
        
        mmpResult = MMP_SUCCESS;   
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    av_free_packet(&avpkt);

    enc_end_tick = CMmpUtil::GetTickCount();

    //pEncResult->uiEncodedDuration = enc_end_tick - enc_start_tick;
    p_buf_videostream->set_coding_dur(enc_end_tick - enc_start_tick);

    //CMmpEncoderVideo::EncodeMonitor(pMediaSample, pEncResult);
    CMmpEncoderVideo::EncodeMonitor(p_buf_videostream);

    return mmpResult;
}
