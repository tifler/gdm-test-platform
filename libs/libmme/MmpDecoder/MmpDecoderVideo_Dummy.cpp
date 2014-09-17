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

#include "MmpDecoderVideo_Dummy.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Dummy Member Functions

CMmpDecoderVideo_Dummy::CMmpDecoderVideo_Dummy(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderVideo(pCreateConfig, MMP_FALSE), CMmpDecoderFfmpeg(pCreateConfig)
,m_bConfigOK(MMP_FALSE)
{
    
}

CMmpDecoderVideo_Dummy::~CMmpDecoderVideo_Dummy()
{

}

MMP_RESULT CMmpDecoderVideo_Dummy::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpDecoderFfmpeg::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Dummy::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderFfmpeg::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Dummy::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Dummy::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVideo_Dummy::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoderFfmpeg::DecodeDSI(pStream, nStreamSize);
    if(mmpResult == MMP_SUCCESS) {
    
        m_bih_out.biWidth = m_pAVCodecContext->width;
        m_bih_out.biHeight = m_pAVCodecContext->height;
	    m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	    m_bih_in.biWidth = m_pAVCodecContext->width;
	    m_bih_in.biHeight = m_pAVCodecContext->height;

        if( (m_pAVCodecContext->width > 0)  && (m_pAVCodecContext->height > 0) ) {
            m_bConfigOK = MMP_TRUE;
        }
    }

    return mmpResult;

}

MMP_RESULT CMmpDecoderVideo_Dummy::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
	MMP_RESULT mmpResult = MMP_SUCCESS;

    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;
    //AVPicture *pFrameOut;

    MMP_U32 dec_start_tick, dec_end_tick;

	pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    pDecResult->uiDecodedDuration = 0;
    

    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }

    dec_start_tick = CMmpUtil::GetTickCount();
    
    av_init_packet (&avpkt);
    avpkt.data = pMediaSample->pAu;
    avpkt.size = (int)pMediaSample->uiAuSize;
    
/*
    m_pAVFrame->linesize[0] = m_pAVCodecContext->width;
    m_pAVFrame->linesize[1] = m_pAVCodecContext->width >> 1;
    m_pAVFrame->linesize[2] = m_pAVCodecContext->width >> 1;
    m_pAVFrame->data[0] = output;
    m_pAVFrame->data[1] = m_pAVFrame->data[0]+m_pAVCodecContext->width*m_pAVCodecContext->height;
    m_pAVFrame->data[2] = m_pAVFrame->data[1]+m_pAVCodecContext->width*m_pAVCodecContext->height/4;
  */

    if(m_bConfigOK == MMP_TRUE) {
        frameFinished = 1;
        usebyte = pDecResult->uiAuUsedByte;
    }
    else {
        usebyte = avcodec_decode_video2(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    }
    if(usebyte > 0) {
        pDecResult->uiAuUsedByte = usebyte;
    }
    else {
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
    }
        
    if(frameFinished != 0) {

        if( (m_pAVCodecContext->width > 0)  && (m_pAVCodecContext->height > 0) ) {
            m_bConfigOK = MMP_TRUE;
        }
        
        if( (m_bih_out.biWidth != m_pAVCodecContext->width) ||  (m_bih_out.biHeight != m_pAVCodecContext->height) 
				|| (m_bih_in.biWidth != m_pAVCodecContext->width) ||  (m_bih_in.biHeight != m_pAVCodecContext->height) ) {
		
            m_bih_out.biWidth = m_pAVCodecContext->width;
            m_bih_out.biHeight = m_pAVCodecContext->height;
            m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

            m_bih_in.biWidth = m_pAVCodecContext->width;
            m_bih_in.biHeight = m_pAVCodecContext->height;

        }

        if(pDecResult->uiDecodedBufferMaxSize >= m_bih_out.biSizeImage) {

            pDecResult->uiDecodedSize = (m_bih_out.biWidth*m_bih_out.biHeight*3)/2;
            pDecResult->bImage = MMP_TRUE;

#if 1
            static unsigned char dummy_color = 0;
            memset((void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y], dummy_color, pDecResult->uiDecodedBufferMaxSize);
            dummy_color++;
#else
            pFrameOut = (AVPicture *)avcodec_alloc_frame();;
            /*
            pFrameOut->linesize[0] = m_pAVCodecContext->width;
            pFrameOut->linesize[1] = m_pAVCodecContext->width >> 1;
            pFrameOut->linesize[2] = m_pAVCodecContext->width >> 1;
            pFrameOut->data[0] = output;
            pFrameOut->data[1] = pFrameOut->data[0];//+m_pAVCodecContext->width*m_pAVCodecContext->height;
            pFrameOut->data[2] = pFrameOut->data[1];//+m_pAVCodecContext->width*m_pAVCodecContext->height/4;
            */
            avpicture_fill((AVPicture *)pFrameOut, output, PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
            av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
            av_free(pFrameOut);
#endif
        }
    }

    dec_end_tick = CMmpUtil::GetTickCount();
    pDecResult->uiDecodedDuration = dec_end_tick - dec_start_tick;

	return mmpResult; 

}

