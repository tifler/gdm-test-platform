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
extern "C" int __chkstk_ms=0;
extern "C" int __divdi3 = 0;
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
        //case MMP_FOURCC_VIDEO_WMV1: m_AVCodecID=AV_CODEC_ID_WMV1; break;
        //case MMP_FOURCC_VIDEO_WMV2: m_AVCodecID=AV_CODEC_ID_WMV2; break;
        //case MMP_FOURCC_VIDEO_WMV3: m_AVCodecID=AV_CODEC_ID_WMV3; break;
        
        //case MMP_FOURCC_VIDEO_VP6: m_AVCodecID=AV_CODEC_ID_VP6; break;
        //case MMP_FOURCC_VIDEO_VP6F: m_AVCodecID=AV_CODEC_ID_VP6F; break;
        //case MMP_FOURCC_VIDEO_VP6A: m_AVCodecID=AV_CODEC_ID_VP6A; break;
        case MMP_FOURCC_VIDEO_RV30: m_AVCodecID=AV_CODEC_ID_RV30; break;
        case MMP_FOURCC_VIDEO_RV40: m_AVCodecID=AV_CODEC_ID_RV40; break;
        //case MMP_FOURCC_VIDEO_SVQ1: m_AVCodecID=AV_CODEC_ID_SVQ1; break;
        //case MMP_FOURCC_VIDEO_SVQ3: m_AVCodecID=AV_CODEC_ID_SVQ3; break;
        case MMP_FOURCC_VIDEO_THEORA: m_AVCodecID=AV_CODEC_ID_THEORA; break;
        case MMP_FOURCC_VIDEO_MJPEG: m_AVCodecID=AV_CODEC_ID_MJPEG; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V1: m_AVCodecID=AV_CODEC_ID_MSMPEG4V1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V2: m_AVCodecID=AV_CODEC_ID_MSMPEG4V2; break;
        case MMP_FOURCC_VIDEO_MSMPEG4V3: m_AVCodecID=AV_CODEC_ID_MSMPEG4V3; break;
        case MMP_FOURCC_VIDEO_VP80: m_AVCodecID=AV_CODEC_ID_VP8; break;
            
        
        case MMP_FOURCC_VIDEO_FFMPEG: m_AVCodecID = (AVCodecID)AV_CODEC_ID_FFMPEG; break;

        default:  m_AVCodecID = AV_CODEC_ID_NONE;
    }
}

CMmpDecoderFfmpeg::~CMmpDecoderFfmpeg()
{

}

MMP_RESULT CMmpDecoderFfmpeg::Open()
{
    
    return MMP_SUCCESS;
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

#if 1
MMP_RESULT CMmpDecoderFfmpeg::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult;
    
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
    else if( (m_AVCodecID == AV_CODEC_ID_RV30) || (m_AVCodecID == AV_CODEC_ID_RV40) ) {

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
}
#endif

#if 0
MMP_RESULT CMmpDecoderFfmpeg::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    int32_t frameFinished = 192000 * 2;
    int16_t *output;
    int32_t usebyte;
    AVPacket avpkt;
    AVCodecContext *cc;
    struct audio_decoded_packet decoded_packet;
    

	pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;

    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize, NULL);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }
    
    av_init_packet (&avpkt);
    avpkt.data = pMediaSample->pAu;
    avpkt.size = (int)pMediaSample->uiAuSize;
    output = (int16_t *)pDecResult->uiDecodedBufLogAddr;

    avcodec_get_frame_defaults(m_pAVFrame_Decoded);

    usebyte = avcodec_decode_audio4(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    if(usebyte > 0) {
        pDecResult->uiAuUsedByte = usebyte;
    }
    else {
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
        mmpResult = MMP_FAILURE;
    }
            
    if(frameFinished != 0) {
    
        /* if a frame has been decoded, output it */
        int data_size = av_samples_get_buffer_size(NULL, m_pAVCodecContext->channels, m_pAVFrame_Decoded->nb_samples, m_pAVCodecContext->sample_fmt, 1);
        
        int32_t bits_per_sample = av_get_bits_per_sample_fmt(m_pAVCodecContext->sample_fmt);
        int32_t numBytesPerSample = bits_per_sample * m_pAVCodecContext->channels / 8;
        int32_t sampleNum = data_size / numBytesPerSample;
        int32_t isample, destsamplenum, srcsamplenum;
        float *pcm_float;
        int16_t *pcm_16bit;
        //mNumSamplesOutput += sampleNum;
        //mLastTimeUs += sampleNum * 1000000 / pCodecCtx->sample_rate;
        
        pDecResult->uiAudioSampleRate = m_pAVCodecContext->sample_rate;
        pDecResult->uiAudioFrameCount = sampleNum;
        
        switch(bits_per_sample) {
        
            case 32 : 
                this->PostProcessing(m_pAVFrame_Decoded, m_pAVCodecContext);
                break;

            default:
                if(pDecResult->uiDecodedBufMaxSize < data_size) {
                    pDecResult->uiDecodedSize = pDecResult->uiDecodedBufMaxSize;
                }
                else {
                    pDecResult->uiDecodedSize = data_size;
                }
                memcpy(output, m_pAVFrame_Decoded->data[0], pDecResult->uiDecodedSize);
        }

        if( (m_pAVCodecContext->sample_rate!=m_wf_in.nSamplesPerSec) 
            || (m_pAVCodecContext->channels!=m_wf_in.nChannels) 
            
            ) {

            m_wf_in.wFormatTag = m_nFormat;
            m_wf_in.nChannels = m_pAVCodecContext->channels;
            m_wf_in.nSamplesPerSec = m_pAVCodecContext->sample_rate;
            m_wf_in.nAvgBytesPerSec = 0;
            m_wf_in.nBlockAlign = m_pAVCodecContext->block_align;
            m_wf_in.wBitsPerSample = 0;
            m_wf_in.cbSize = 0;

            m_wf_out.wFormatTag = MMP_WAVE_FORMAT_PCM;
            m_wf_out.nChannels = m_pAVCodecContext->channels;
            m_wf_out.nSamplesPerSec = m_pAVCodecContext->sample_rate;
            m_wf_out.nAvgBytesPerSec = 0;
            m_wf_out.nBlockAlign = m_pAVCodecContext->block_align;
            m_wf_out.wBitsPerSample = 0;
            m_wf_out.cbSize = 0;

        }
    }

    if(!m_queue_decoded.IsEmpty()) {
        
        struct audio_decoded_packet* p_decoded_packet;
        int buf_remain_size, buf_copy_size;
        MMP_U8* pOutBuffer = (MMP_U8*)pDecResult->uiDecodedBufLogAddr;

        m_queue_decoded.GetFirstItem(p_decoded_packet);
        buf_remain_size = p_decoded_packet->buf_size - p_decoded_packet->buf_index;
    
        if(buf_remain_size >= pDecResult->uiDecodedBufMaxSize) {

            buf_copy_size = pDecResult->uiDecodedBufMaxSize;
        }
        else {
            buf_copy_size = buf_remain_size;
        }
        
        memcpy(pOutBuffer, &p_decoded_packet->p_buffer[p_decoded_packet->buf_index], buf_copy_size);
        p_decoded_packet->buf_index += buf_copy_size;
        pDecResult->uiDecodedSize = buf_copy_size;

        if(p_decoded_packet->buf_index >= p_decoded_packet->buf_size) {
        
            m_queue_decoded.Delete(p_decoded_packet);
            delete p_decoded_packet->p_buffer;
            delete p_decoded_packet;
        }
    }

	return MMP_SUCCESS;
}
/*

int ff_audio_data_init(AudioData *a, uint8_t **src, int plane_size, int channels,
                       int nb_samples, enum AVSampleFormat sample_fmt,
                       int read_only, const char *name);


AudioConvert *ff_audio_convert_alloc(AVAudioResampleContext *avr,
                                     enum AVSampleFormat out_fmt,
                                     enum AVSampleFormat in_fmt,
                                     int channels, int sample_rate,
                                     int apply_map)
*/
void CMmpDecoderFfmpeg::PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext) {

    AudioConvert *ac;
    struct AVAudioResampleContext avr;
    AudioData out, in;
    struct audio_decoded_packet* p_decoded_packet;
    int iret;

    //uint8_t* in_buf = new uint8_t[1024*1024];
    //uint8_t* out_buf = new uint8_t[1024*1024];

    p_decoded_packet = new struct audio_decoded_packet;
    p_decoded_packet->buf_size = pAVFrame_Decoded->nb_samples * 2 * 2;
    p_decoded_packet->buf_index = 0;
    p_decoded_packet->p_buffer = new MMP_U8[p_decoded_packet->buf_size];

    memset(&avr, 0x00, sizeof(struct AVAudioResampleContext));
    avr.dither_method = AV_RESAMPLE_DITHER_NONE;
    avr.av_class = NULL;

    ff_audio_data_init(&in, &pAVFrame_Decoded->data[0], 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_FLTP, 1, NULL);
    ff_audio_data_init(&out, &p_decoded_packet->p_buffer, 0, 2, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_S16, 0, NULL);

    ac = ff_audio_convert_alloc(&avr, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP, 2, 44100, 0);
    if(ac != NULL) {
    
        iret = ff_audio_convert(ac, &out, &in);
        ff_audio_convert_free(&ac);

        if(iret == 0) {
            this->m_queue_decoded.Add(p_decoded_packet);
        }
        else {
            delete p_decoded_packet->p_buffer;
            delete p_decoded_packet;
        }
    }
}

#endif