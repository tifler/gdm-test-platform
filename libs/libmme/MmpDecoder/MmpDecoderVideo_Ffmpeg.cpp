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

#include "MmpDecoderVideo_Ffmpeg.hpp"
#include "../MmpComm/MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Ffmpeg Member Functions

CMmpDecoderVideo_Ffmpeg::CMmpDecoderVideo_Ffmpeg(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderVideo(pCreateConfig, MMP_FALSE), CMmpDecoderFfmpeg(pCreateConfig)
,m_p_buf_videoframe(NULL)
,m_decoded_picture_count(0)
{
    
}

CMmpDecoderVideo_Ffmpeg::~CMmpDecoderVideo_Ffmpeg()
{

}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::Open()
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

    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Ffmpeg::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderFfmpeg::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

    if(m_p_buf_videoframe != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videoframe);
        m_p_buf_videoframe = NULL;
    }
        
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoderFfmpeg::DecodeDSI(pStream, nStreamSize);
    if(mmpResult == MMP_SUCCESS) {
    
        m_bih_out.biWidth = m_pAVCodecContext->width;
        m_bih_out.biHeight = m_pAVCodecContext->height;
	    m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	    m_bih_in.biWidth = m_pAVCodecContext->width;
	    m_bih_in.biHeight = m_pAVCodecContext->height;


        if(m_pAVCodecContext->codec != NULL) {
            strcpy((char*)m_szCodecName, m_pAVCodecContext->codec->name);
        }

        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeDSI] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    }

    return mmpResult;

}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeDSI(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoderFfmpeg::DecodeDSI(p_buf_videostream);
    if(mmpResult == MMP_SUCCESS) {
    
        m_bih_out.biWidth = m_pAVCodecContext->width;
        m_bih_out.biHeight = m_pAVCodecContext->height;
	    m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	    m_bih_in.biWidth = m_pAVCodecContext->width;
	    m_bih_in.biHeight = m_pAVCodecContext->height;


        if(m_pAVCodecContext->codec != NULL) {
            strcpy((char*)m_szCodecName, m_pAVCodecContext->codec->name);
        }

        /* create video frame buffer */
        if( (m_bih_out.biWidth > 16)  && (m_bih_out.biHeight > 16) ) {

            if(m_p_buf_videoframe != NULL) {
                mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videoframe);
                m_p_buf_videoframe = NULL;
            }
            m_p_buf_videoframe = mmp_buffer_mgr::get_instance()->alloc_media_videoframe(m_bih_out.biWidth, m_bih_out.biHeight, MMP_FOURCC_IMAGE_YUV420_P3);
        }

        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeDSI] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    }

    return mmpResult;

}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
	MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i;
    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;
    
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

    if((pMediaSample->uiFlag&MMP_MEDIASAMPMLE_FLAG_CONFIGDATA) != 0) {

        pDecResult->uiDecodedSize = 0;
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
        
        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();
    
    av_init_packet (&avpkt);
    avpkt.data = pMediaSample->pAu;
    avpkt.size = (int)pMediaSample->uiAuSize;
    avpkt.pts = pMediaSample->uiTimeStamp;
    
    usebyte = avcodec_decode_video2(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    if(usebyte > 0) {
        pDecResult->uiAuUsedByte = usebyte;
    }
    else {
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
    }

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeAu] %d, au(%02x %02x %02x %02x %02x %02x %02x %02x ) usedbyte=%d  ausz=%d pts=%d codec_width=%d codec_height=%d framefinished=%d "),
                       m_decoded_picture_count,
                       avpkt.data[0], avpkt.data[1], avpkt.data[2], avpkt.data[3],
                       avpkt.data[4], avpkt.data[5], avpkt.data[6], avpkt.data[7],
                       usebyte,
                       avpkt.size, (unsigned int)(avpkt.pts/1000LL),
                       m_pAVCodecContext->width, m_pAVCodecContext->height,
                       frameFinished
                       ));
        
    if(frameFinished != 0) {
    
        m_decoded_picture_count++;

        if( (m_bih_out.biWidth != m_pAVCodecContext->width) ||  (m_bih_out.biHeight != m_pAVCodecContext->height) 
				|| (m_bih_in.biWidth != m_pAVCodecContext->width) ||  (m_bih_in.biHeight != m_pAVCodecContext->height) ) {
		
            m_bih_out.biWidth = m_pAVCodecContext->width;
            m_bih_out.biHeight = m_pAVCodecContext->height;
            m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

            m_bih_in.biWidth = m_pAVCodecContext->width;
            m_bih_in.biHeight = m_pAVCodecContext->height;

            /* create video frame buffer */
            if(m_p_buf_videoframe != NULL) {
                mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videoframe);
                m_p_buf_videoframe = NULL;
            }
            
            m_p_buf_videoframe = mmp_buffer_mgr::get_instance()->alloc_media_videoframe(m_bih_out.biWidth, m_bih_out.biHeight, MMP_FOURCC_IMAGE_YUV420_P3);
            if(m_p_buf_videoframe == NULL) {
                mmpResult = MMP_FAILURE;
            }

        }

        if( (pDecResult->uiDecodedBufferMaxSize >= m_bih_out.biSizeImage) 
            && (pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y] >= (MMP_U32)m_pAVCodecContext->width) 
            && (pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_Y] >= (MMP_U32)m_pAVCodecContext->height) 
            && (m_p_buf_videoframe != NULL)
            )
        {
            pDecResult->uiDecodedSize = (m_bih_out.biWidth*m_bih_out.biHeight*3)/2;
            pDecResult->bImage = MMP_TRUE;
            pDecResult->uiTimeStamp = m_pAVFrame_Decoded->pkt_pts;

            AVPicture pic;
            AVPicture *pFrameOut = &pic;
            memset(pFrameOut, 0x00, sizeof(AVPicture));
            
            for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                pFrameOut->data[i] = m_p_buf_videoframe->get_buf_vir_addr(i);
                pFrameOut->linesize[i] = pDecResult->uiDecodedBufferStride[i];
            }
            
            pDecResult->uiResultType = MMP_MEDIASAMPLE_BUFFER_TYPE_VIDEO_FRAME;
            pDecResult->uiDecodedBufferPhyAddr[MMP_DECODED_BUF_VIDEO_FRAME] = (MMP_U32)m_p_buf_videoframe;
            
            if(m_pAVFrame_Decoded->format == AV_PIX_FMT_YUV420P) {
                av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
            }
            else if(m_pAVFrame_Decoded->format == AV_PIX_FMT_PAL8) {
            
                int stride = m_pAVFrame_Decoded->linesize[0];
                unsigned char* rgb = (unsigned char*)m_pAVFrame_Decoded->data[0];
                unsigned int* pallete = (unsigned int*)m_pAVFrame_Decoded->data[1];

                CMmpUtil::ColorConvertRGB8Pallete32ToYUV420Planar(m_pAVCodecContext->width, m_pAVCodecContext->height, stride,
                                                     rgb, pallete, 
                                                     pFrameOut->data[0], pFrameOut->data[1], pFrameOut->data[2],
                                                     pFrameOut->linesize[0], pFrameOut->linesize[1]   
                                                    );
            }
            else if(m_pAVFrame_Decoded->format == AV_PIX_FMT_RGB24) {

                int stride = m_pAVFrame_Decoded->linesize[0];
                unsigned char* rgb24 = (unsigned char*)m_pAVFrame_Decoded->data[0];
                
                CMmpUtil::ColorConvertRGB24ToYUV420Planar(m_pAVCodecContext->width, m_pAVCodecContext->height, stride,
                                                     rgb24, 
                                                     pFrameOut->data[0], pFrameOut->data[1], pFrameOut->data[2],
                                                     pFrameOut->linesize[0], pFrameOut->linesize[1]   
                                                    );
            }
            else if(m_pAVFrame_Decoded->format == AV_PIX_FMT_YUVJ422P) { /* MJpeg */
                av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
            }
            else {
                pDecResult->uiDecodedSize = 0;
            }
            
        }
    }

    dec_end_tick = CMmpUtil::GetTickCount();
    pDecResult->uiDecodedDuration = dec_end_tick - dec_start_tick;

    CMmpDecoderVideo::DecodeMonitor(pMediaSample, pDecResult);

	return mmpResult; 

}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i;
    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;
    
    MMP_U32 dec_start_tick, dec_end_tick;
    class mmp_buffer_videoframe* p_buf_videoframe_output = NULL;

    /* Init Parmeter */
    p_buf_videostream->set_used_byte(0);
	if(pp_buf_videoframe != NULL) {
        *pp_buf_videoframe = NULL;
    }
    
    /* Decode DSI */
    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(p_buf_videostream);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }

    /* Decode DSI */
    if((p_buf_videostream->get_flag()&MMP_MEDIASAMPMLE_FLAG_CONFIGDATA) != 0) {

        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();
    
    av_init_packet (&avpkt);
    avpkt.data = (uint8_t*)p_buf_videostream->get_buf_vir_addr(); 
    avpkt.size = p_buf_videostream->get_stream_size();
    avpkt.pts = p_buf_videostream->get_pts();
    
    usebyte = avcodec_decode_video2(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    if(usebyte > 0) {
        p_buf_videostream->set_used_byte(usebyte);
    }
    else {
        p_buf_videostream->set_used_byte(p_buf_videostream->get_stream_size());
    }

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeAu] %d au(%02x %02x %02x %02x %02x %02x %02x %02x ) usedbyte=%d  ausz=%d pts=%d codec_width=%d codec_height=%d framefinished=%d "),
                       m_decoded_picture_count,
                       avpkt.data[0], avpkt.data[1], avpkt.data[2], avpkt.data[3],
                       avpkt.data[4], avpkt.data[5], avpkt.data[6], avpkt.data[7],
                       usebyte,
                       avpkt.size, (unsigned int)(avpkt.pts/1000LL),
                       m_pAVCodecContext->width, m_pAVCodecContext->height,
                       frameFinished
                       ));
        
    if(frameFinished != 0) {
    
        m_decoded_picture_count++;

        if( (m_bih_out.biWidth != m_pAVCodecContext->width) ||  (m_bih_out.biHeight != m_pAVCodecContext->height) 
				|| (m_bih_in.biWidth != m_pAVCodecContext->width) ||  (m_bih_in.biHeight != m_pAVCodecContext->height) ) {
		
            m_bih_out.biWidth = m_pAVCodecContext->width;
            m_bih_out.biHeight = m_pAVCodecContext->height;
            m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

            m_bih_in.biWidth = m_pAVCodecContext->width;
            m_bih_in.biHeight = m_pAVCodecContext->height;

            /* create video frame buffer */
            if(m_p_buf_videoframe != NULL) {
                mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videoframe);
                m_p_buf_videoframe = NULL;
            }
            m_p_buf_videoframe = mmp_buffer_mgr::get_instance()->alloc_media_videoframe(m_bih_out.biWidth, m_bih_out.biHeight, MMP_FOURCC_IMAGE_YUV420_P3);
        }

        if(m_p_buf_videoframe != NULL) 
        {
            m_p_buf_videoframe->set_pts(m_pAVFrame_Decoded->pkt_pts);

            AVPicture pic;
            AVPicture *pFrameOut = &pic;
            memset(pFrameOut, 0x00, sizeof(AVPicture));
            
            for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                pFrameOut->data[i] = m_p_buf_videoframe->get_buf_vir_addr(i);
                pFrameOut->linesize[i] = m_p_buf_videoframe->get_buf_stride(i);
            }

            switch(m_pAVFrame_Decoded->format) {
            
                case AV_PIX_FMT_YUV420P:
                case AV_PIX_FMT_YUVJ420P:

                    av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
                    p_buf_videoframe_output =m_p_buf_videoframe;
                    break;

                case AV_PIX_FMT_YUVJ422P:
                    av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
                    p_buf_videoframe_output =m_p_buf_videoframe;
                    break;
            
                case AV_PIX_FMT_PAL8:

                    {
                        int stride = m_pAVFrame_Decoded->linesize[0];
                        unsigned char* rgb = (unsigned char*)m_pAVFrame_Decoded->data[0];
                        unsigned int* pallete = (unsigned int*)m_pAVFrame_Decoded->data[1];

                        CMmpUtil::ColorConvertRGB8Pallete32ToYUV420Planar(m_pAVCodecContext->width, m_pAVCodecContext->height, stride,
                                                             rgb, pallete, 
                                                             pFrameOut->data[0], pFrameOut->data[1], pFrameOut->data[2],
                                                             pFrameOut->linesize[0], pFrameOut->linesize[1]   
                                                            );

                        p_buf_videoframe_output =m_p_buf_videoframe;
                    }
                    break;

                case AV_PIX_FMT_RGB24:

                    {
                        int stride = m_pAVFrame_Decoded->linesize[0];
                        unsigned char* rgb24 = (unsigned char*)m_pAVFrame_Decoded->data[0];
                    
                        CMmpUtil::ColorConvertRGB24ToYUV420Planar(m_pAVCodecContext->width, m_pAVCodecContext->height, stride,
                                                         rgb24, 
                                                         pFrameOut->data[0], pFrameOut->data[1], pFrameOut->data[2],
                                                         pFrameOut->linesize[0], pFrameOut->linesize[1]   
                                                        );

                        p_buf_videoframe_output =m_p_buf_videoframe;
                    }
                    break;
            }
        }
    }

    dec_end_tick = CMmpUtil::GetTickCount();

    if(p_buf_videoframe_output != NULL) {
        p_buf_videoframe_output->set_coding_dur(dec_end_tick - dec_start_tick);
            
        CMmpDecoderVideo::DecodeMonitor(p_buf_videoframe_output);
        
    }
    
    if(pp_buf_videoframe != NULL) {
       *pp_buf_videoframe = p_buf_videoframe_output;
    }
    
	return mmpResult; 
}