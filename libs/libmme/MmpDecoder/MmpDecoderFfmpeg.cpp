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

#include "MmpDecoderFfmpeg.hpp"
#include "../MmpComm/MmpUtil.hpp"


#if (MMP_OS == MMP_OS_WIN32) /* ffmpeg win32 mingw parameter..*/
//extern "C" int __chkstk_ms=0;
//extern "C" int __divdi3 = 0;
#endif

/////////////////////////////////////////////////////////////
//CMmpDecoderFfmpeg Member Functions

#define AV_CODEC_ID_FFMPEG    MKBETAG('F','F','M','P')

CMmpDecoderFfmpeg::CMmpDecoderFfmpeg(struct MmpDecoderCreateConfig *pCreateConfig) :
m_pAVCodec(NULL)
,m_pAVCodecContext(NULL)
,m_pAVFrame_Decoded(NULL)
,m_extra_data(NULL)

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
        case MMP_FOURCC_VIDEO_VC1: m_AVCodecID=AV_CODEC_ID_VC1; break;

        case MMP_FOURCC_VIDEO_WMV1: m_AVCodecID=AV_CODEC_ID_WMV1; break;
        case MMP_FOURCC_VIDEO_WMV2: m_AVCodecID=AV_CODEC_ID_WMV2; break;
        case MMP_FOURCC_VIDEO_WMV3: m_AVCodecID=AV_CODEC_ID_WMV3; break;
        case MMP_FOURCC_VIDEO_MSS1: m_AVCodecID=AV_CODEC_ID_MSS1; break;
        case MMP_FOURCC_VIDEO_MSS2: m_AVCodecID=AV_CODEC_ID_MSS2; break;
        
        case MMP_FOURCC_VIDEO_VP80: m_AVCodecID=AV_CODEC_ID_VP8; break;
        case MMP_FOURCC_VIDEO_VP60: m_AVCodecID=AV_CODEC_ID_VP6; break;
        case MMP_FOURCC_VIDEO_VP6F: m_AVCodecID=AV_CODEC_ID_VP6F; break;
        case MMP_FOURCC_VIDEO_VP6A: m_AVCodecID=AV_CODEC_ID_VP6A; break;

        case MMP_FOURCC_VIDEO_RV30: m_AVCodecID=AV_CODEC_ID_RV30; break;
        case MMP_FOURCC_VIDEO_RV40: m_AVCodecID=AV_CODEC_ID_RV40; break;
        
        case MMP_FOURCC_VIDEO_SVQ1: m_AVCodecID=AV_CODEC_ID_SVQ1; break;
        case MMP_FOURCC_VIDEO_SVQ3: m_AVCodecID=AV_CODEC_ID_SVQ3; break;
        
        case MMP_FOURCC_VIDEO_THEORA: m_AVCodecID=AV_CODEC_ID_THEORA; break;
        case MMP_FOURCC_VIDEO_MJPEG: m_AVCodecID=AV_CODEC_ID_MJPEG; break;
        case MMP_FOURCC_VIDEO_FLV1: m_AVCodecID=AV_CODEC_ID_FLV1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V1: m_AVCodecID=AV_CODEC_ID_MSMPEG4V1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V2: m_AVCodecID=AV_CODEC_ID_MSMPEG4V2; break;
        case MMP_FOURCC_VIDEO_MSMPEG4V3: m_AVCodecID=AV_CODEC_ID_MSMPEG4V3; break;
            
        
        case MMP_FOURCC_VIDEO_FFMPEG: m_AVCodecID = (AVCodecID)AV_CODEC_ID_FFMPEG; break;

        default:  
            m_AVCodecID = AV_CODEC_ID_NONE;
    }
}

CMmpDecoderFfmpeg::~CMmpDecoderFfmpeg()
{

}

MMP_RESULT CMmpDecoderFfmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    if(m_AVCodecID == AV_CODEC_ID_NONE) {
        mmpResult = MMP_FAILURE;
    }
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderFfmpeg::Close()
{
    if(this->m_pAVCodecContext != NULL) {
        avcodec_close(this->m_pAVCodecContext);
        av_free(this->m_pAVCodecContext);
        this->m_pAVCodecContext = NULL;
    }
    
    if(m_pAVFrame_Decoded != NULL) {
        avcodec_free_frame(&m_pAVFrame_Decoded);
        m_pAVFrame_Decoded = NULL;
    }

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderFfmpeg::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

#if 1
    return MMP_FAILURE;
#else
//    MMP_RESULT mmpResult;
    
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    struct mmp_ffmpeg_packet_header *p_ffmpeg_packet_header;
    MMP_U32 key=0, psz1, psz2, i;

    p_ffmpeg_packet_header = (struct mmp_ffmpeg_packet_header *)pStream;
    if(p_ffmpeg_packet_header != NULL) {
        key = p_ffmpeg_packet_header->key;
        psz1 = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size+p_ffmpeg_packet_header->extra_data_size; //Packet Size
        psz2 = p_ffmpeg_packet_header->packet_size;
    }

    if((m_AVCodecID == AV_CODEC_ID_FFMPEG) 
      || (m_AVCodecID == AV_CODEC_ID_VC1) )
    {
        if( (key == MMP_FFMPEG_PACKET_HEADER_KEY) 
            && (psz1 == psz2) 
            &&  (p_ffmpeg_packet_header->payload_type == MMP_FFMPEG_PACKET_TYPE_AVCodecContext)
            )
        {
            i = p_ffmpeg_packet_header->hdr_size;
            cc1 = (AVCodecContext *)&pStream[i];

            m_AVCodecID = cc1->codec_id;
        }
        else 
        {
            return MMP_FAILURE;
        }
    }
    
    codec = avcodec_find_decoder(m_AVCodecID);
    if(codec == NULL) {
        return MMP_FAILURE;
    }

    cc= avcodec_alloc_context();
    //cc = avcodec_alloc_context3(codec);
    
    if((key == MMP_FFMPEG_PACKET_HEADER_KEY) && (psz1 == psz2) ) {
        
        switch(p_ffmpeg_packet_header->payload_type) {
        
            case MMP_FFMPEG_PACKET_TYPE_AVCodecContext:

                i = p_ffmpeg_packet_header->hdr_size;
                cc1 = (AVCodecContext *)&pStream[i];

#if 1                
                memcpy(cc, &pStream[i], p_ffmpeg_packet_header->payload_size);
#else
                cc->sample_rate = cc1->sample_rate;
                cc->channels = cc1->channels;
                cc->codec_type = cc1->codec_type;
                cc->codec_id = cc1->codec_id;
#endif

                i = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size;

                if(p_ffmpeg_packet_header->extra_data_size > 0) {

                    m_extra_data = new MMP_U8[p_ffmpeg_packet_header->extra_data_size];
                    memcpy(m_extra_data, &pStream[i], p_ffmpeg_packet_header->extra_data_size);
                    cc->extradata = m_extra_data;
                    cc->extradata_size = p_ffmpeg_packet_header->extra_data_size;

                }
                else {
                    cc->extradata = NULL;
                    cc->extradata_size = 0;
                }
                break;
        
        }
    }
    else if(m_AVCodecID == AV_CODEC_ID_MSMPEG4V3) {
        memcpy(cc, &pStream[0], sizeof(AVCodecContext));
        cc->extradata = NULL;
        cc->extradata_size = 0;
    }
    else if( (m_AVCodecID == AV_CODEC_ID_RV30) 
        || (m_AVCodecID == AV_CODEC_ID_RV40) 
        || (m_AVCodecID == AV_CODEC_ID_WMV3) 
        )

    {

        memcpy(cc, &pStream[0], sizeof(AVCodecContext));
        cc->extradata = NULL;
        //cc->extradata_size = 0;

        //cc = (AVCodecContext *)pStream;
        //st = (AVStream *)&pStream[sizeof(AVCodecContext)];
        //avc = st->codec;
        cc->extradata = &pStream[sizeof(AVCodecContext)+sizeof(AVStream)];
    }
    else if( (m_AVCodecID == AV_CODEC_ID_VP8)) {

        memcpy(cc, &pStream[0], sizeof(AVCodecContext));
        cc->extradata = NULL;
        //cc->extradata_size = 0;

        //cc = (AVCodecContext *)pStream;
        //st = (AVStream *)&pStream[sizeof(AVCodecContext)];
        //avc = st->codec;
        cc->extradata = &pStream[sizeof(AVCodecContext)+sizeof(AVStream)];
    }
    else {
        if(m_extra_data != NULL) {
            delete [] m_extra_data;
            m_extra_data = NULL;
        }
        m_extra_data = new MMP_U8[nStreamSize];
        memcpy(m_extra_data, pStream, nStreamSize);
        cc->extradata = m_extra_data;
        cc->extradata_size = nStreamSize;
    }

    /* open it */
    if(avcodec_open(cc, codec) < 0) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderFfmpeg::DecodeDSI] FAIL: could not open codec\n\r")));
        return MMP_FAILURE;
    }

    m_pAVCodec = codec;
    m_pAVCodecContext = cc;
    m_pAVFrame_Decoded = avcodec_alloc_frame();
   
    return MMP_SUCCESS;
#endif
}

MMP_RESULT CMmpDecoderFfmpeg::DecodeDSI(class mmp_buffer_videostream* p_buf_videostream) {
        
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    
    codec = avcodec_find_decoder(m_AVCodecID);
    if(codec == NULL) {
        return MMP_FAILURE;
    }

    cc= avcodec_alloc_context3(codec);

    if(p_buf_videostream->get_ffmpeg_codec_context() != NULL) {
        switch(m_AVCodecID) {
        
            case AV_CODEC_ID_H264:
            case AV_CODEC_ID_MPEG4:
            case AV_CODEC_ID_MPEG2VIDEO:
                break;
            
            case AV_CODEC_ID_WMV3:
            default:
                memcpy(cc, p_buf_videostream->get_ffmpeg_codec_context(), sizeof(AVCodecContext));
                break;
        }
    }

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }
    m_extra_data = new MMP_U8[p_buf_videostream->get_stream_size()];
    memcpy(m_extra_data, (void*)p_buf_videostream->get_buf_vir_addr(), p_buf_videostream->get_stream_size());
    cc->extradata = m_extra_data;
    cc->extradata_size = p_buf_videostream->get_stream_size();
    
    
    /* open it */
    if(avcodec_open2(cc, codec, NULL) < 0) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderFfmpeg::DecodeDSI] FAIL: could not open codec\n\r")));
        return MMP_FAILURE;
    }

    m_pAVCodec = codec;
    m_pAVCodecContext = cc;
    m_pAVFrame_Decoded = avcodec_alloc_frame();
   
    return MMP_SUCCESS;
}

