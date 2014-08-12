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

#include "MmpEncoderFfmpeg.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpEncoderFfmpeg Member Functions

#define AV_CODEC_ID_FFMPEG    MKBETAG('F','F','M','P')

CMmpEncoderFfmpeg::CMmpEncoderFfmpeg(struct MmpEncoderCreateConfig *pCreateConfig) :
m_CreateConfig(*pCreateConfig)

,m_pAVCodec(NULL)
,m_pAVCodecContext(NULL)
,m_pAVFrame_Input(NULL)
,m_extra_data(NULL)

,m_nPicWidth(pCreateConfig->nPicWidth)
,m_nPicHeight(pCreateConfig->nPicHeight)
{
    avcodec_register_all();

    switch(pCreateConfig->nFormat) {
    
        /* Audio */
        case MMP_WAVE_FORMAT_MPEGLAYER3: m_AVCodecID = AV_CODEC_ID_MP3; break;
        case MMP_WAVE_FORMAT_MPEGLAYER2: m_AVCodecID = AV_CODEC_ID_MP2; break;
        case MMP_WAVE_FORMAT_MPEGLAYER1: m_AVCodecID = AV_CODEC_ID_MP1; break;
        case MMP_WAVE_FORMAT_WMA2: m_AVCodecID = AV_CODEC_ID_WMAV2; break;
        case MMP_WAVE_FORMAT_AC3: m_AVCodecID = AV_CODEC_ID_AC3; break;
        case MMP_WAVE_FORMAT_AAC: m_AVCodecID = AV_CODEC_ID_AAC; break;
        case MMP_WAVE_FORMAT_FLAC: m_AVCodecID = AV_CODEC_ID_FLAC; break;
        case MMP_WAVE_FORMAT_ADPCM_MS: m_AVCodecID = AV_CODEC_ID_ADPCM_MS; break;
        case MMP_WAVE_FORMAT_FFMPEG: m_AVCodecID = (AVCodecID)AV_CODEC_ID_FFMPEG; break;

        /* Video */
        case MMP_FOURCC_VIDEO_H263: m_AVCodecID=AV_CODEC_ID_H263; break;
        case MMP_FOURCC_VIDEO_H264: m_AVCodecID=AV_CODEC_ID_H264; break;
        case MMP_FOURCC_VIDEO_MPEG4: m_AVCodecID=AV_CODEC_ID_MPEG4; break;
        case MMP_FOURCC_VIDEO_MPEG2: m_AVCodecID=AV_CODEC_ID_MPEG2VIDEO; break;
        //case MMP_FOURCC_VIDEO_VC1: m_AVCodecID=AV_CODEC_ID_VC1; break;
        //case MMP_FOURCC_VIDEO_WMV1: m_AVCodecID=AV_CODEC_ID_WMV1; break;
        //case MMP_FOURCC_VIDEO_WMV2: m_AVCodecID=AV_CODEC_ID_WMV2; break;
        //case MMP_FOURCC_VIDEO_WMV3: m_AVCodecID=AV_CODEC_ID_WMV3; break;
        
        //case MMP_FOURCC_VIDEO_VP6: m_AVCodecID=AV_CODEC_ID_VP6; break;
        //case MMP_FOURCC_VIDEO_VP6F: m_AVCodecID=AV_CODEC_ID_VP6F; break;
        //case MMP_FOURCC_VIDEO_VP6A: m_AVCodecID=AV_CODEC_ID_VP6A; break;
        //case MMP_FOURCC_VIDEO_RV30: m_AVCodecID=AV_CODEC_ID_RV30; break;
        //case MMP_FOURCC_VIDEO_RV40: m_AVCodecID=AV_CODEC_ID_RV40; break;
        //case MMP_FOURCC_VIDEO_SVQ1: m_AVCodecID=AV_CODEC_ID_SVQ1; break;
        //case MMP_FOURCC_VIDEO_SVQ3: m_AVCodecID=AV_CODEC_ID_SVQ3; break;
        case MMP_FOURCC_VIDEO_THEORA: m_AVCodecID=AV_CODEC_ID_THEORA; break;
        case MMP_FOURCC_VIDEO_MJPEG: m_AVCodecID=AV_CODEC_ID_MJPEG; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V1: m_AVCodecID=AV_CODEC_ID_MSMPEG4V1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V2: m_AVCodecID=AV_CODEC_ID_MSMPEG4V2; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V3: m_AVCodecID=AV_CODEC_ID_MSMPEG4V3; break;
        
        case MMP_FOURCC_VIDEO_FFMPEG: m_AVCodecID = (AVCodecID)AV_CODEC_ID_FFMPEG; break;

        default:  m_AVCodecID = AV_CODEC_ID_NONE;
    }
}

CMmpEncoderFfmpeg::~CMmpEncoderFfmpeg()
{

}

MMP_RESULT CMmpEncoderFfmpeg::Open()
{
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoderFfmpeg::Close()
{
    if(this->m_pAVCodecContext != NULL) {
        avcodec_close(this->m_pAVCodecContext);
        av_free(this->m_pAVCodecContext);
        this->m_pAVCodecContext = NULL;
    }
    
    if(m_pAVFrame_Input != NULL) {
        avcodec_free_frame(&m_pAVFrame_Input);
        m_pAVFrame_Input = NULL;
    }

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderFfmpeg::EncodeDSI(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

    MMP_RESULT mmpResult;
    AVRational avr;
    
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    MMP_U32 key=0, psz1, psz2, i;


    codec = avcodec_find_encoder(m_AVCodecID);
    if(codec == NULL) {
        return MMP_FAILURE;
    }

    cc= avcodec_alloc_context();
    
    cc->bit_rate = m_CreateConfig.nBitRate;//400000;     /* put sample parameters */
    cc->width = m_nPicWidth;   /* resolution must be a multiple of two */
    cc->height = m_nPicHeight;

    /* frames per second */
    avr.num = 1;
    avr.den = m_CreateConfig.nIDRPeriod;//25;
    cc->time_base= avr; //(AVRational){1, 25}; 

    cc->gop_size = m_CreateConfig.nFrameRate; //10; /* emit one intra frame every ten frames */
    cc->max_b_frames=1;

    cc->pix_fmt = PIX_FMT_YUV420P;

    cc->extradata = NULL;//pStream;
    cc->extradata_size = NULL;//nStreamSize;
    
    /* open it */
    if(avcodec_open(cc, codec) < 0) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderFfmpeg::DecodeDSI] FAIL: could not open codec\n\r")));
        return MMP_FAILURE;
    }

    m_pAVCodec = codec;
    m_pAVCodecContext = cc;
    m_pAVFrame_Input = avcodec_alloc_frame();
   
    
    return MMP_SUCCESS;
}

