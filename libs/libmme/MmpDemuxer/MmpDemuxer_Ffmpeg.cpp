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

#include "MmpDemuxer_Ffmpeg.hpp"
#include "MmpUtil.hpp"



/////////////////////////////////////////////////////
// class

CMmpDemuxer_Ffmpeg::CMmpDemuxer_Ffmpeg(struct MmpDemuxerCreateConfig* pCreateConfig) : CMmpDemuxer(pCreateConfig)
,m_fp(NULL)
,m_IoBuffer(NULL)
,m_Url(NULL)
,m_pAVIOContext(NULL)
,m_pAvformatCtx(NULL)
,m_iFileSize(0)
,m_nFileBufferIndex(0)
,m_nFileBufferSize(0)
{
    int i;

    av_register_all();

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        m_nStreamIndex[i] = -1;
    }
}

CMmpDemuxer_Ffmpeg::~CMmpDemuxer_Ffmpeg()
{

}

MMP_RESULT CMmpDemuxer_Ffmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    int i;
    AVProbeData   pd;
    uint8_t *buffer = NULL;
    AVInputFormat *fmt = NULL;
    const int io_buffer_size = 32768;  /* FIXME */

	MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_Ffmpeg::Open] Init Demuxer ... Wait for about 5sec")));

    m_fp = fopen((const char*)this->m_create_config.filename, "rb");
    if(m_fp == NULL) {
        mmpResult = MMP_FAILURE;
    }
    else {
        fseek(m_fp, 0, SEEK_END);
        m_iFileSize = ftell(m_fp);
        fseek(m_fp, 0, SEEK_SET);
    
    }

    /* alloc first 1MB data */
    if(mmpResult == MMP_SUCCESS) {

        pd.buf_size = 1024 * 1024 * 1;
        buffer = (uint8_t *)malloc(pd.buf_size);
        if(buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        pd.filename = NULL;
        pd.buf = buffer;
    }

    /* read first 1MB data  and Guess format */
    if(mmpResult == MMP_SUCCESS) {
        
        int32_t size = fread(buffer, 1, pd.buf_size, m_fp);
        if(size <= 0)
        {
            mmpResult = MMP_FAILURE;
        }
        else {

            fseek(m_fp, 0, SEEK_SET);

            pd.buf_size = size;
            fmt = av_probe_input_format(&pd, 1);
            if(fmt == NULL)
            {
               mmpResult = MMP_FAILURE;     
            }
        }
    }

    /* Create I/O wrapper and URL */
    if(mmpResult == MMP_SUCCESS) {

        m_IoBuffer = (MMP_U8*)malloc(io_buffer_size);
        if(m_IoBuffer == NULL)  {
            mmpResult = MMP_FAILURE;     
        }
        else {
            memset(m_IoBuffer, 0x00, io_buffer_size);
        }
            	
        m_Url = (URLContext*)malloc( sizeof(URLContext) );
        if(m_Url == NULL) {
            mmpResult = MMP_FAILURE;     
        }
        else {
            memset(m_Url, 0x00, sizeof(URLContext));
            m_Url->priv_data = (void*)this;
        }
    }

    /* open io context */
    if(mmpResult == MMP_SUCCESS) {
    
        m_pAVIOContext = avio_alloc_context(m_IoBuffer, io_buffer_size, 0, m_Url, CMmpDemuxer_Ffmpeg::IORead_C_Stub, NULL, CMmpDemuxer_Ffmpeg::IOSeek_C_Stub);
        if(m_pAVIOContext == NULL) {
            mmpResult = MMP_FAILURE;     
        }
    }

    /* open format context */
    if(mmpResult == MMP_SUCCESS) {
        m_pAvformatCtx = avformat_alloc_context();
        m_pAvformatCtx->pb = m_pAVIOContext;
        if(avformat_open_input( &m_pAvformatCtx, "", fmt, NULL ) != 0)
        {
            mmpResult = MMP_FAILURE;  
        }
        else {

            //if (av_find_stream_info(m_pAvformatCtx) < 0) 
            if(avformat_find_stream_info(m_pAvformatCtx, NULL) < 0)     
            {
                mmpResult = MMP_FAILURE;  
            }
        }
    }

    if(mmpResult == MMP_SUCCESS) {

        for(i = 0; i < (int)(m_pAvformatCtx->nb_streams); i++ )
        {
            AVStream *s = m_pAvformatCtx->streams[i];
            AVCodecContext *cc = s->codec;

            //ALOGI ("AVCodecContext codecID = 0x%x", cc->codec_id);
            switch((MMP_U32)cc->codec_type ) {
            
                case AVMEDIA_TYPE_AUDIO:
                    m_nStreamIndex[MMP_MEDIATYPE_AUDIO] = i;
                    break;

                case AVMEDIA_TYPE_VIDEO:
                    m_nStreamIndex[MMP_MEDIATYPE_VIDEO] = i;
                    break;
            }
        }
    
    }

    if(buffer != NULL) {
        free(buffer);
    }
   
    return mmpResult;
}

MMP_RESULT CMmpDemuxer_Ffmpeg::Close()
{
 
    if(m_pAvformatCtx != NULL)
    {
        avformat_close_input(&m_pAvformatCtx);
        m_pAvformatCtx = NULL;
    }

    if(m_pAVIOContext != NULL) {
#if (MMP_OS == MMP_OS_WIN32)
        avio_close(m_pAVIOContext);
#endif   
        m_pAVIOContext = NULL;

        m_Url = NULL;
        m_IoBuffer = NULL;
    }

    if(m_Url != NULL) {
        free(m_Url);
        m_Url = NULL;
    }

    if(m_IoBuffer != NULL) {
        free(m_IoBuffer);
        m_IoBuffer = NULL;
    }

    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }

    return MMP_SUCCESS;
}

MMP_S64 CMmpDemuxer_Ffmpeg::GetDuration() {

    int i;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    int64_t dur, keydur = 0;
    
    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {

        stream_index = m_nStreamIndex[i];
        if(stream_index >= 0) {

            s = m_pAvformatCtx->streams[stream_index];
            cc = s->codec;

            if(s->duration == AV_NOPTS_VALUE) {
                dur = m_pAvformatCtx->duration;
                keydur = dur;
            }
            else {
                dur = s->duration;
                keydur = (dur * 1000000) * s->time_base.num / s->time_base.den;
            }
            
            break;
        }
    }

    return keydur;
}

MMP_U32 CMmpDemuxer_Ffmpeg::GetAudioFormat() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch(cc->codec_id) {
        
            case AV_CODEC_ID_MP3:
                format = MMP_WAVE_FORMAT_MPEGLAYER3;
                break;

            case AV_CODEC_ID_MP2:
                format = MMP_WAVE_FORMAT_MPEGLAYER2;
                break;
            
            case AV_CODEC_ID_AAC:
                format = MMP_WAVE_FORMAT_AAC;
                break;

            default:
                format = MMP_WAVE_FORMAT_FFMPEG;
        }
        

    }

    return format;
}

MMP_U32 CMmpDemuxer_Ffmpeg::GetAudioChannel() {
    
    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->channels;
        if(retvalue > 5) {
            retvalue = 2;
        }
    }

    return retvalue;
}
    
MMP_U32 CMmpDemuxer_Ffmpeg::GetAudioSamplingRate() {

    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->sample_rate;
    }

    return retvalue;
}

MMP_U32 CMmpDemuxer_Ffmpeg::GetAudioBitsPerSample() {

    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->bits_per_coded_sample;
        if(retvalue == 0) {
            retvalue = 16;    
        }
    }

    return retvalue;
}
    
MMP_U32 CMmpDemuxer_Ffmpeg::GetVideoFormat() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch(cc->codec_id) {
        
            case AV_CODEC_ID_H263:  format = MMP_FOURCC_VIDEO_H263; break;
            case AV_CODEC_ID_H264:  format = MMP_FOURCC_VIDEO_H264; break;
            case AV_CODEC_ID_MPEG4: format = MMP_FOURCC_VIDEO_MPEG4;  break;
            case AV_CODEC_ID_MPEG2VIDEO : format = MMP_FOURCC_VIDEO_MPEG2;  break;
            case AV_CODEC_ID_MJPEG : format = MMP_FOURCC_VIDEO_MJPEG;  break;

            /* Microsoft Codec */
            case AV_CODEC_ID_WMV1: format = MMP_FOURCC_VIDEO_WMV1; break;
            case AV_CODEC_ID_WMV2: format = MMP_FOURCC_VIDEO_WMV2; break;
            case AV_CODEC_ID_WMV3: format = MMP_FOURCC_VIDEO_WMV3; break;
            case AV_CODEC_ID_VC1:  format = MMP_FOURCC_VIDEO_VC1;  break;
            case AV_CODEC_ID_MSMPEG4V2: format = MMP_FOURCC_VIDEO_MSMPEG4V2; break;
            case AV_CODEC_ID_MSMPEG4V3: format = MMP_FOURCC_VIDEO_MSMPEG4V3; break;
            case AV_CODEC_ID_MSS1: format = MMP_FOURCC_VIDEO_MSS1;  break;
            case AV_CODEC_ID_MSS2: format = MMP_FOURCC_VIDEO_MSS2;  break;

            /* RV */
            case AV_CODEC_ID_RV30: format = MMP_FOURCC_VIDEO_RV30; break;
            case AV_CODEC_ID_RV40: format = MMP_FOURCC_VIDEO_RV40; break;

            /* VP 6/7/8 */
            case AV_CODEC_ID_VP8:  format = MMP_FOURCC_VIDEO_VP80; break;
            case AV_CODEC_ID_VP6:  format = MMP_FOURCC_VIDEO_VP60; break;
            case AV_CODEC_ID_VP6F: format = MMP_FOURCC_VIDEO_VP6F; break;
            case AV_CODEC_ID_VP6A: format = MMP_FOURCC_VIDEO_VP6A; break;

            /* Etc */
            case AV_CODEC_ID_SVQ3: format = MMP_FOURCC_VIDEO_SVQ3; break;
            case AV_CODEC_ID_THEORA: format = MMP_FOURCC_VIDEO_THEORA; break;
            case AV_CODEC_ID_FLV1: format = MMP_FOURCC_VIDEO_FLV1; break;
            case AV_CODEC_ID_INDEO2: format = MMP_FOURCC_VIDEO_INDEO2; break;
            case AV_CODEC_ID_INDEO3: format = MMP_FOURCC_VIDEO_INDEO3; break;
            case AV_CODEC_ID_INDEO4: format = MMP_FOURCC_VIDEO_INDEO4; break;
            case AV_CODEC_ID_INDEO5: format = MMP_FOURCC_VIDEO_INDEO5; break;
            case AV_CODEC_ID_TSCC:   format = MMP_FOURCC_VIDEO_TSCC; break;
            
            default:
                format = MMP_FOURCC_VIDEO_UNKNOWN;
        }
        

    }

    return format;
}

MMP_U32 CMmpDemuxer_Ffmpeg::GetVideoPicWidth() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 width = 0;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        width = cc->width;

    }

    return width;
}

MMP_U32 CMmpDemuxer_Ffmpeg::GetVideoPicHeight() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 height = 0;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        height = cc->height;
    }

    return height;
}

MMP_RESULT CMmpDemuxer_Ffmpeg::GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

#if 0
    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    struct mmp_ffmpeg_packet_header ffmpeg_packet_header;
    MMP_U8* pdata;
    
    if(buf_size) *buf_size = 0;
    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch((MMP_U32)cc->codec_id) {

            case AV_CODEC_ID_H264:
            case AV_CODEC_ID_MPEG4:
                if(cc->extradata_size > 0) {
                    memcpy(buffer, cc->extradata, cc->extradata_size);
                    if(buf_size) *buf_size = cc->extradata_size;
                    mmpResult = MMP_SUCCESS;
                }
                break;
            
            case AV_CODEC_ID_RV30:
            case AV_CODEC_ID_RV40:
            case AV_CODEC_ID_WMV3:
            case AV_CODEC_ID_VP8:
                pdata = buffer;
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, s, sizeof(AVStream)); pdata+=sizeof(AVStream);
                if(cc->extradata_size > 0) {
                    memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                }
                if(buf_size) *buf_size = (unsigned int)pdata-(unsigned int)buffer;
                break;
            
            case AV_CODEC_ID_MSMPEG4V3:
                pdata = buffer;
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, s, sizeof(AVStream)); pdata+=sizeof(AVStream);
                //memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                if(buf_size) *buf_size = (unsigned int)pdata-(unsigned int)buffer;
                break;

            default:
                ffmpeg_packet_header.key = MMP_FFMPEG_PACKET_HEADER_KEY;
                ffmpeg_packet_header.payload_type = MMP_FFMPEG_PACKET_TYPE_AVCodecContext;
                ffmpeg_packet_header.hdr_size = sizeof(struct mmp_ffmpeg_packet_header);
                ffmpeg_packet_header.payload_size = sizeof(AVCodecContext);
                ffmpeg_packet_header.extra_data_size = cc->extradata_size;
                ffmpeg_packet_header.packet_size = ffmpeg_packet_header.hdr_size+ffmpeg_packet_header.payload_size+ffmpeg_packet_header.extra_data_size;

                pdata = buffer;
                memcpy(pdata, &ffmpeg_packet_header, sizeof(struct mmp_ffmpeg_packet_header)); pdata+=sizeof(struct mmp_ffmpeg_packet_header);
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                
                mmpResult = MMP_SUCCESS;
                if(buf_size) *buf_size = ffmpeg_packet_header.packet_size;
        }

    }

    return mmpResult;

#else

    return MMP_FAILURE;
#endif
}

MMP_RESULT CMmpDemuxer_Ffmpeg::GetMediaExtraData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

#if 0
    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    struct mmp_ffmpeg_packet_header ffmpeg_packet_header;
    MMP_U8* pdata;
    
    if(buf_size) *buf_size = 0;
    stream_index = m_nStreamIndex[mediatype];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch((MMP_U32)cc->codec_id) {

            case AV_CODEC_ID_H264:
            case AV_CODEC_ID_MPEG4:
                if(cc->extradata_size > 0) {
                    memcpy(buffer, cc->extradata, cc->extradata_size);
                    if(buf_size) *buf_size = cc->extradata_size;
                    mmpResult = MMP_SUCCESS;
                }
                break;
            
            case AV_CODEC_ID_RV30:
            case AV_CODEC_ID_RV40:
            case AV_CODEC_ID_WMV3:
            case AV_CODEC_ID_VP8:
                pdata = buffer;
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, s, sizeof(AVStream)); pdata+=sizeof(AVStream);
                if(cc->extradata_size > 0) {
                    memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                }
                if(buf_size) *buf_size = (unsigned int)pdata-(unsigned int)buffer;
                mmpResult = MMP_SUCCESS;
                break;
            
            default:
                ffmpeg_packet_header.key = MMP_FFMPEG_PACKET_HEADER_KEY;
                ffmpeg_packet_header.payload_type = MMP_FFMPEG_PACKET_TYPE_AVCodecContext;
                ffmpeg_packet_header.hdr_size = sizeof(struct mmp_ffmpeg_packet_header);
                ffmpeg_packet_header.payload_size = sizeof(AVCodecContext);
                ffmpeg_packet_header.extra_data_size = cc->extradata_size;
                ffmpeg_packet_header.packet_size = ffmpeg_packet_header.hdr_size+ffmpeg_packet_header.payload_size+ffmpeg_packet_header.extra_data_size;

                pdata = buffer;
                memcpy(pdata, &ffmpeg_packet_header, sizeof(struct mmp_ffmpeg_packet_header)); pdata+=sizeof(struct mmp_ffmpeg_packet_header);
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                
                mmpResult = MMP_SUCCESS;
                if(buf_size) *buf_size = ffmpeg_packet_header.packet_size;
        }

    }

    return mmpResult;

#else
    return MMP_FAILURE;
#endif
}

MMP_RESULT CMmpDemuxer_Ffmpeg::GetVideoExtraData(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;

    MMP_U8* buffer;

    MMP_S32 frame_rate = 0;
    
    p_buf_videostream->set_stream_size(0);
    buffer = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        memcpy(buffer, cc->extradata, cc->extradata_size);
        p_buf_videostream->set_stream_size(cc->extradata_size);

        /* set cc */
        p_buf_videostream->set_ffmpeg_codec_context(cc, sizeof(AVCodecContext) );

        /* set frame rate */
        if(s->avg_frame_rate.den && s->avg_frame_rate.num)
            frame_rate = (MMP_S32)((double)s->avg_frame_rate.num/(double)s->avg_frame_rate.den);

        if(!frame_rate && s->r_frame_rate.den && s->r_frame_rate.num)
            frame_rate = (MMP_S32)((double)s->r_frame_rate.num/(double)s->r_frame_rate.den);
        p_buf_videostream->set_player_framerate(frame_rate);

        /* set width, height */
        p_buf_videostream->set_pic_width(s->codec->width);
        p_buf_videostream->set_pic_height(s->codec->height);

        /* set bit rate */
        p_buf_videostream->set_player_bitrate(s->codec->bit_rate);


        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

#if 0
MMP_RESULT CMmpDemuxer_Ffmpeg::GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    AVPacket  pkt;
    MMP_BOOL bRun = MMP_TRUE;

    if(buf_size) *buf_size = 0;

    while(bRun == MMP_TRUE) {

        if(av_read_frame(m_pAvformatCtx, &pkt ) != 0)
        {
            break;
        }

        if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
            memcpy(buffer, pkt.data, pkt.size);
            if(buf_size) *buf_size = pkt.size;
            if(packt_pts) *packt_pts = pkt.pts;
            mmpResult = MMP_SUCCESS;
            bRun = MMP_FALSE;
        }

        av_free_packet(&pkt);
    }

    return mmpResult;
}  
#endif

MMP_RESULT CMmpDemuxer_Ffmpeg::GetNextMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    AVPacket  pkt;
    MMP_BOOL bRun = MMP_TRUE;
    int64_t packet_timestamp;
    AVStream *s;
    char szMediaType[16];

    if(buf_size) *buf_size = 0;
    if(packt_pts) *packt_pts = 0;

    while(bRun == MMP_TRUE) {

        if(av_read_frame(m_pAvformatCtx, &pkt ) != 0)
        {
            break;
        }

        if(pkt.stream_index == m_nStreamIndex[mediatype]) {

            s = m_pAvformatCtx->streams[pkt.stream_index];

            if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
                strcpy(szMediaType, "Video");
            }
            else if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
                strcpy(szMediaType, "Audio");
            }
            else {
                strcpy(szMediaType, "Unknown");
            }

            memcpy(buffer, pkt.data, pkt.size);
            if(buf_size) *buf_size = pkt.size;
            if(packt_pts) {
             
                packet_timestamp = pkt.pts;
                if(packet_timestamp == AV_NOPTS_VALUE) {
                    packet_timestamp = pkt.dts;
                }
                (*packt_pts) = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
                //qpack.flags = 0;
                //if(pkt.flags & AV_PKT_FLAG_KEY) {
                //    qpack.flags |= MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME;
               // }

                MMPDEBUGMSG(0, (TEXT("[CMmpDemuxer_Ffmpeg::GetNextMediaData] MediaType=(%s)  pts=%d packsz=%d  pack(pts=%d dts=%d start=%d num=%d den=%d ) \n\r"),
                                      szMediaType,
                                      (unsigned int)((*packt_pts)/1000),
                                      pkt.size,
                                      //(unsigned int)(pkt.pts/1000), (unsigned int)(pkt.dts/1000),
                                      (unsigned int)(pkt.pts), (unsigned int)(pkt.dts),
                                      (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                      ));
            }

            mmpResult = MMP_SUCCESS;
            bRun = MMP_FALSE;

            
        }

        av_free_packet(&pkt);
    }

    return mmpResult;
}
    
int CMmpDemuxer_Ffmpeg::IORead_C_Stub(void *opaque, uint8_t *buf, int buf_size) {

    URLContext *p_url = (URLContext*)opaque;
    CMmpDemuxer_Ffmpeg *pObj = (CMmpDemuxer_Ffmpeg *)p_url->priv_data;
    return pObj->IORead_C(opaque, buf, buf_size);
}

int64_t CMmpDemuxer_Ffmpeg::IOSeek_C_Stub(void *opaque, int64_t offset, int whence) {

    URLContext *p_url = (URLContext*)opaque;
    CMmpDemuxer_Ffmpeg *pObj = (CMmpDemuxer_Ffmpeg *)p_url->priv_data;
    return pObj->IOSeek_C(opaque, offset, whence);
}

#if 0

int CMmpDemuxer_Ffmpeg::IORead_C(void *opaque, uint8_t *buf, int buf_size) {

    int i_ret ;
    int buffer_remain_byte;
    uint8_t * buf1;
    int buf_size1;
		
    if( buf_size < 0 ) return -1;
    if( buf_size == 0 ) return 0;
   

    buffer_remain_byte = m_nFileBufferSize - m_nFileBufferIndex;
    if(buffer_remain_byte == 0) {
       
        m_nFileBufferSize = fread(m_FileBuffer, 1, MMP_DEMUXER_FILE_BUFFER_MAX_SIZE, m_fp);
        m_nFileBufferIndex = 0;

        MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_Ffmpeg::IORead_C]  Burst Read 1 ")));
    }

    buffer_remain_byte = m_nFileBufferSize - m_nFileBufferIndex;

    if(buffer_remain_byte == 0) {
        i_ret = -1;
    }
    else if(buffer_remain_byte >= buf_size) {
        
        memcpy(buf, &m_FileBuffer[m_nFileBufferIndex], buf_size);
        m_nFileBufferIndex+=buf_size;

        i_ret = buf_size;
    }
    else {

        memcpy(buf, &m_FileBuffer[m_nFileBufferIndex], buffer_remain_byte);
        i_ret = buffer_remain_byte;
        
        buf1 = buf + buffer_remain_byte;
        buf_size1 = buf_size - buffer_remain_byte;
        while(buf_size1 > 0) {
            
            MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_Ffmpeg::IORead_C]  Burst Read 2 ")));
            m_nFileBufferSize = fread(m_FileBuffer, 1, MMP_DEMUXER_FILE_BUFFER_MAX_SIZE, m_fp);
            m_nFileBufferIndex = 0;

            buffer_remain_byte = m_nFileBufferSize - m_nFileBufferIndex;
            if(buffer_remain_byte == 0) {
                  break;
            }
            else if(buffer_remain_byte >= buf_size1) {
    
                memcpy(buf1, &m_FileBuffer[m_nFileBufferIndex], buf_size1);
                m_nFileBufferIndex+=buf_size1;

                i_ret += buf_size1;
                break;
            }
            else {
                //memcpy(buf1, &m_FileBuffer[m_nFileBufferIndex], buffer_remain_byte);
                //i_ret += buffer_remain_byte;
                //buf_size1 -= buffer_remain_byte;
                break;
            }

        } /* end of while */

    }
    

    return i_ret;
    //i_ret = fread(buf, 1, buf_size, m_fp);

    //return (i_ret >= 0) ? i_ret : -1;

}


#else
int CMmpDemuxer_Ffmpeg::IORead_C(void *opaque, uint8_t *buf, int buf_size) {

    int i_ret;
		
    if( buf_size < 0 ) return -1;

    i_ret = fread(buf, 1, buf_size, m_fp);

    return (i_ret >= 0) ? i_ret : -1;

}
#endif

int64_t CMmpDemuxer_Ffmpeg::IOSeek_C(void *opaque, int64_t offset, int whence) {


#if 0
    switch(whence)
    {
#ifdef AVSEEK_SIZE
        case AVSEEK_SIZE:
            return m_iFileSize;
#endif
        case SEEK_SET:
            //i_absolute = (int64_t)offset;
            fseek(m_fp, (long)offset, SEEK_SET);
            break;
        case SEEK_CUR:
            //i_absolute = pss->offset + (int64_t)offset;
            fseek(m_fp, (long)offset, SEEK_CUR);
            break;
        case SEEK_END:
            //i_absolute = i_size + (int64_t)offset;
            fseek(m_fp, (long)offset, SEEK_END);
            break;
        default:
            return -1;

    }

#else


    switch(whence)
    {
#ifdef AVSEEK_SIZE
        case AVSEEK_SIZE:
            return m_iFileSize;
#endif
        case SEEK_SET:
        case SEEK_CUR:
        case SEEK_END:
            fseek(m_fp, (long)offset, whence);
            m_nFileBufferIndex = 0;
            m_nFileBufferSize = 0;
            break;
            
        default:
            return -1;

    }

#endif

    return (int64_t)ftell(m_fp);
}


void CMmpDemuxer_Ffmpeg::queue_buffering(void) {

    AVPacket  pkt;
    struct queue_packet qpack;//, qpack_tmp;
    //int i;
    AVStream *s;
    int64_t packet_timestamp;
    MMP_S32 empty_stream_index;
    char szMediaType[32];
    
    while(1) {
        empty_stream_index = queue_get_empty_streamindex();
        if(empty_stream_index != -1) {
            /* Read a frame */
            if(av_read_frame(m_pAvformatCtx, &pkt ) )
            {
                //ALOGE ("av_read_frame error ... ");
                break;
            }
            else {
                
                s = m_pAvformatCtx->streams[pkt.stream_index];
                
                if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
                    qpack.mediatype = MMP_MEDIATYPE_VIDEO;
                    strcpy(szMediaType, "Video");
                }
                else if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
                    qpack.mediatype = MMP_MEDIATYPE_AUDIO;
                    strcpy(szMediaType, "Audio");
                }
                else {
                    qpack.mediatype =MMP_MEDIATYPE_UNKNOWN;
                    strcpy(szMediaType, "Unknown");
                }

                packet_timestamp = pkt.pts;
                if(packet_timestamp == AV_NOPTS_VALUE) {
                    packet_timestamp = pkt.dts;
                }
                qpack.pts = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
                qpack.flags = 0;
                if(pkt.flags & AV_PKT_FLAG_KEY) {
                    qpack.flags |= MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME;
                }
                qpack.size = pkt.size;
                qpack.data = pkt.data;
                this->queue_add(&qpack);
                av_free_packet(&pkt);

                MMPDEBUGMSG(0, (TEXT("[CMmpDemuxer_Ffmpeg::queue_buffering] MediaType=(%d %s)  pts=%d packsz=%d  pack(pts=%d dts=%d start=%d num=%d den=%d ) \n\r"),
                                      qpack.mediatype, szMediaType,
                                      (unsigned int)(qpack.pts/1000),
                                      qpack.size,
                                      (unsigned int)(pkt.pts/1000), (unsigned int)(pkt.dts/1000),
                                      (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                      ));
            }

        }/* end of if(i != ANAFFMPEG_MAX_STREAM_COUNT) */
        else {
            break;
        } 
    } /* end of while(run == true) { */
}

MMP_RESULT CMmpDemuxer_Ffmpeg::Seek(MMP_S64 pts) {

    this->queue_clear();
	
	MMPDEBUGMSG(0, (TEXT("[CMmpDemuxer_Ffmpeg::seek) = %lld \n\r"),pts ));

    if(av_seek_frame(m_pAvformatCtx, -1, pts, 0)  < 0)
    {
        MMPDEBUGMSG(0, (TEXT("[CMmpDemuxer_Ffmpeg::seek) av_seek_frame error \n\r")));      
		return MMP_FAILURE;
    }
    else {
    
        MMPDEBUGMSG(0, (TEXT("[CMmpDemuxer_Ffmpeg::seek) av_seek_frame success \n\r")));          
		return MMP_SUCCESS;
    }
    
}