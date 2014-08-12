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

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Ffmpeg Member Functions

CMmpDecoderVideo_Ffmpeg::CMmpDecoderVideo_Ffmpeg(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderVideo(pCreateConfig, MMP_FALSE), CMmpDecoderFfmpeg(pCreateConfig)

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
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    return MMP_SUCCESS;
}

#if 0
MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

#if 1
    MMP_RESULT mmpResult;
    
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    AVCodecID codecID;
    struct mmp_ffmpeg_packet_header *p_ffmpeg_packet_header;
    MMP_U32 key, psz1, psz2, i;
    

    switch(m_nFormat) {
    
        case MMP_FOURCC_VIDEO_MPEG4: codecID=AV_CODEC_ID_MPEG4; break;
        case MMP_FOURCC_VIDEO_VC1: codecID=AV_CODEC_ID_VC1; break;
        default: 
            return MMP_FAILURE;
    }

    codec = avcodec_find_decoder(codecID);
    cc= avcodec_alloc_context();
   
    p_ffmpeg_packet_header = (struct mmp_ffmpeg_packet_header *)pStream;
    key = p_ffmpeg_packet_header->key;
    psz1 = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size+p_ffmpeg_packet_header->extra_data_size; //Packet Size
    psz2 = p_ffmpeg_packet_header->packet_size;


    if((key == MMP_FFMPEG_PACKET_HEADER_KEY) && (psz1 == psz2) ) {
      
#if 0
        switch(p_ffmpeg_packet_header->payload_type) {
        
            case MMP_FFMPEG_PACKET_TYPE_AVCodecContext:
                i = p_ffmpeg_packet_header->hdr_size;
                cc1 = (AVCodecContext *)&pStream[i];
                memcpy(cc, &pStream[i], p_ffmpeg_packet_header->payload_size);
                
                i = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size;

                m_extra_data = new MMP_U8[p_ffmpeg_packet_header->extra_data_size];
                memcpy(m_extra_data, &pStream[i], p_ffmpeg_packet_header->extra_data_size);
                cc->extradata = m_extra_data;
                cc->extradata_size = p_ffmpeg_packet_header->extra_data_size;
                break;
        
        }
#endif
    }
    else {
        cc->extradata = pStream;
        cc->extradata_size = nStreamSize;
    }


    /* open it */
    if(avcodec_open(cc, codec) < 0) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeDSI] FAIL: avcodec_open \n\r")));
        return MMP_FAILURE;
    }

    m_pAVCodec = codec;
    m_pAVCodecContext = cc;

    
    if( (m_bih_out.biWidth != m_pAVCodecContext->width) ||  (m_bih_out.biHeight != m_pAVCodecContext->height) 
				|| (m_bih_in.biWidth != m_pAVCodecContext->width) ||  (m_bih_in.biHeight != m_pAVCodecContext->height) ) {
			
		if(bConfigChange!=NULL) *bConfigChange = MMP_TRUE;
    }

    m_bih_out.biWidth = m_pAVCodecContext->width;
    m_bih_out.biHeight = m_pAVCodecContext->height;
	m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	m_bih_in.biWidth = m_pAVCodecContext->width;
	m_bih_in.biHeight = m_pAVCodecContext->height;

    m_pAVFrame_Decoded = avcodec_alloc_frame();
    avpicture_fill((AVPicture *)m_pAVFrame_Decoded, NULL, PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
#endif

    return MMP_SUCCESS;
}

#else

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

#endif

MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
	MMP_RESULT mmpResult = MMP_SUCCESS;

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

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeAu] au(%02x %02x %02x %02x %02x %02x %02x %02x ) usedbyte=%d  ausz=%d pts=%d codec_width=%d codec_height=%d framefinished=%d "),
                       avpkt.data[0], avpkt.data[1], avpkt.data[2], avpkt.data[3],
                       avpkt.data[4], avpkt.data[5], avpkt.data[6], avpkt.data[7],
                       usebyte,
                       avpkt.size, (unsigned int)(avpkt.pts/1000LL),
                       m_pAVCodecContext->width, m_pAVCodecContext->height,
                       frameFinished
                       ));
        
    if(frameFinished != 0) {
    
        if( (m_bih_out.biWidth != m_pAVCodecContext->width) ||  (m_bih_out.biHeight != m_pAVCodecContext->height) 
				|| (m_bih_in.biWidth != m_pAVCodecContext->width) ||  (m_bih_in.biHeight != m_pAVCodecContext->height) ) {
		
            m_bih_out.biWidth = m_pAVCodecContext->width;
            m_bih_out.biHeight = m_pAVCodecContext->height;
            m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

            m_bih_in.biWidth = m_pAVCodecContext->width;
            m_bih_in.biHeight = m_pAVCodecContext->height;

        }

        if( (pDecResult->uiDecodedBufferMaxSize >= m_bih_out.biSizeImage) 
            && (pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y] >= m_pAVCodecContext->width) 
            && (pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_Y] >= m_pAVCodecContext->height) )
        {
            pDecResult->uiDecodedSize = (m_bih_out.biWidth*m_bih_out.biHeight*3)/2;
            pDecResult->bImage = MMP_TRUE;
            pDecResult->uiTimeStamp = m_pAVFrame_Decoded->pkt_pts;

            AVPicture pic;
            AVPicture *pFrameOut = &pic;
            memset(pFrameOut, 0x00, sizeof(AVPicture));
            
            pFrameOut->data[0] = (uint8_t*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y];//output;
            pFrameOut->data[1] = (uint8_t*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U];//pFrameOut->data[0];//+m_pAVCodecContext->width*m_pAVCodecContext->height;
            pFrameOut->data[2] = (uint8_t*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V];//pFrameOut->data[1];//+m_pAVCodecContext->width*m_pAVCodecContext->height/4;
            pFrameOut->linesize[0] = pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y];//em_pAVCodecContext->width;
            pFrameOut->linesize[1] = pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_U];//m_pAVCodecContext->width >> 1;
            pFrameOut->linesize[2] = pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_V];//m_pAVCodecContext->width >> 1;
            
            //avpicture_fill((AVPicture *)pFrameOut, (uint8_t*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y], PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);

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

