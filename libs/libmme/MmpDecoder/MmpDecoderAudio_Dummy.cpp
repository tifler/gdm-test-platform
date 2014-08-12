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

#include "MmpDecoderAudio_Dummy.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderAudio_Dummy Member Functions

CMmpDecoderAudio_Dummy::CMmpDecoderAudio_Dummy(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderAudio(pCreateConfig), CMmpDecoderFfmpeg(pCreateConfig)
,m_pAudioConvert(NULL)
,m_queue_decoded(60)
,m_bCofigOK(MMP_FALSE)
{
    
}

CMmpDecoderAudio_Dummy::~CMmpDecoderAudio_Dummy()
{

}

MMP_RESULT CMmpDecoderAudio_Dummy::Open(MMP_U8* pStream, MMP_U32 nStreamSize)
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderAudio::Open();
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


MMP_RESULT CMmpDecoderAudio_Dummy::Close()
{
    MMP_RESULT mmpResult;
    struct audio_decoded_packet* p_decoded_packet;

    mmpResult=CMmpDecoderFfmpeg::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_Dummy::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderAudio::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_Dummy::Close] CMmpDecoderAudio::Close() \n\r")));
        return mmpResult;
    }

    
    while(!m_queue_decoded.IsEmpty()) {
        
        m_queue_decoded.Delete(p_decoded_packet);
        delete [] p_decoded_packet->p_buffer;
        delete p_decoded_packet;
    }

    if(m_pAudioConvert != NULL) {
        ff_audio_convert_free(&m_pAudioConvert);
        m_pAudioConvert = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio_Dummy::AudioConvert_Create(AVCodecContext *pAVCodecContext) {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    memset(&m_AVAudioResampleContext, 0x00, sizeof(struct AVAudioResampleContext));
    m_AVAudioResampleContext.dither_method = AV_RESAMPLE_DITHER_NONE;
    m_AVAudioResampleContext.av_class = NULL;

    if(m_pAudioConvert != NULL) {
        ff_audio_convert_free(&m_pAudioConvert);
        m_pAudioConvert = NULL;
    }

    if( (pAVCodecContext->sample_fmt >= 0) && (pAVCodecContext->channels > 0) && (pAVCodecContext->sample_rate!=0) ) {
    
        m_pAudioConvert = ff_audio_convert_alloc(&m_AVAudioResampleContext, 
                                                AV_SAMPLE_FMT_S16, 
                                                pAVCodecContext->sample_fmt, 
                                                pAVCodecContext->channels, 
                                                pAVCodecContext->sample_rate, 0);
    }

    if(m_pAudioConvert == NULL ) {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}
    
MMP_RESULT CMmpDecoderAudio_Dummy::AudioConvert_Destroy() {

    this->AudioConvert_Destroy();

    return MMP_SUCCESS;
}

#if 0
MMP_RESULT CMmpDecoderAudio_Dummy::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult;
    
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    AVCodecID codecID;
    struct mmp_ffmpeg_packet_header *p_ffmpeg_packet_header;
    MMP_U32 key, psz1, psz2, i;
    
    switch(m_nFormat) {
    
        case MMP_WAVE_FORMAT_MPEGLAYER3: codecID=AV_CODEC_ID_MP3; break;
        case MMP_WAVE_FORMAT_WMA2: codecID=AV_CODEC_ID_WMAV2; break;

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
    }
    else {
        cc->extradata = pStream;
        cc->extradata_size = nStreamSize;
    }

    /* open it */
    if(avcodec_open(cc, codec) < 0) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMpDecoderA_MPlayer::Open] FAIL: could not open codec\n\r")));
        return MMP_FAILURE;
    }

    m_pAVCodec = codec;
    m_pAVCodecContext = cc;

    
    m_wf_in.wFormatTag = m_nFormat;
    m_wf_in.nChannels = cc->channels;
    m_wf_in.nSamplesPerSec = cc->sample_rate;
    m_wf_in.nAvgBytesPerSec = 0;
    m_wf_in.nBlockAlign = cc->block_align;
    m_wf_in.wBitsPerSample = 0;
    m_wf_in.cbSize = 0;

    m_wf_out.wFormatTag = MMP_WAVE_FORMAT_PCM;
    m_wf_out.nChannels = cc->channels;
    m_wf_out.nSamplesPerSec = cc->sample_rate;
    m_wf_out.wBitsPerSample = av_get_bits_per_sample_fmt(m_pAVCodecContext->sample_fmt);
    m_wf_out.nBlockAlign = cc->block_align;
    m_wf_out.nAvgBytesPerSec = m_wf_out.nSamplesPerSec * m_wf_out.nChannels * m_wf_out.wBitsPerSample / 8;
    m_wf_out.cbSize = 0;

    m_pAVFrame_Decoded = avcodec_alloc_frame();
    
    return MMP_SUCCESS;
}
#else

MMP_RESULT CMmpDecoderAudio_Dummy::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoderFfmpeg::DecodeDSI(pStream, nStreamSize);
    if(mmpResult == MMP_SUCCESS) {
    
#if 0
        m_wf_in.wFormatTag = m_nFormat;
        m_wf_in.nChannels = m_pAVCodecContext->channels;
        m_wf_in.nSamplesPerSec = m_pAVCodecContext->sample_rate;
        m_wf_in.nAvgBytesPerSec = 0;
        m_wf_in.nBlockAlign = m_pAVCodecContext->block_align;
        m_wf_in.wBitsPerSample = 0;
        m_wf_in.cbSize = 0;

        m_wf_out.wFormatTag = MMP_WAVE_FORMAT_PCM;
        m_wf_out.nChannels = MMP_DEFAULT_AUDIO_OUT_CHANNEL;
        m_wf_out.wBitsPerSample = MMP_DEFAULT_AUDIO_OUT_SAMPLEBITS;//av_get_bits_per_sample_fmt(m_pAVCodecContext->sample_fmt);
        m_wf_out.nSamplesPerSec = m_pAVCodecContext->sample_rate;
        m_wf_out.nBlockAlign = m_pAVCodecContext->block_align;
        m_wf_out.nAvgBytesPerSec = m_wf_out.nSamplesPerSec * m_wf_out.nChannels * m_wf_out.wBitsPerSample / 8;
        m_wf_out.cbSize = 0;

        this->m_uiExpectedDecodedBufferSize = m_wf_out.nBlockAlign * m_wf_out.nChannels * m_wf_out.wBitsPerSample/8;
#endif
    
        this->AudioConvert_Create(m_pAVCodecContext);
     }

    return mmpResult;
}


#endif

MMP_RESULT CMmpDecoderAudio_Dummy::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;
    AVCodecContext *cc;
    struct audio_decoded_packet decoded_packet;
    
    MMP_U32 dec_start_tick, dec_end_tick;

	pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;

    dec_start_tick = CMmpUtil::GetTickCount();
    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }
    av_init_packet (&avpkt);
    avpkt.data = pMediaSample->pAu;
    avpkt.size = (int)pMediaSample->uiAuSize;
    
    avcodec_get_frame_defaults(m_pAVFrame_Decoded);

    if(m_bCofigOK == MMP_TRUE) {
        usebyte = pDecResult->uiAuUsedByte;
        frameFinished = 1;
    }
    else {
        usebyte = avcodec_decode_audio4(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    }
    if(usebyte > 0) {
        pDecResult->uiAuUsedByte = usebyte;
    }
    else {
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
        mmpResult = MMP_FAILURE;
    }
    if(frameFinished != 0) {
    
        /* if a frame has been decoded, output it */
        
        if( (m_pAVCodecContext->sample_rate!=m_wf_in.nSamplesPerSec) 
            || (m_pAVCodecContext->channels!=m_wf_in.nChannels) 
            //|| (m_pAVCodecContext->channels!=m_wf_in.nBlockAlign) 
            ) {

            if( (m_pAVCodecContext->sample_rate != 0)
                && (m_pAVCodecContext->channels != 0) )
            {
                m_bCofigOK = MMP_TRUE;

                m_wf_in.wFormatTag = m_nFormat;
                m_wf_in.nChannels = m_pAVCodecContext->channels;
                m_wf_in.nSamplesPerSec = m_pAVCodecContext->sample_rate;
                m_wf_in.nAvgBytesPerSec = 0;
                m_wf_in.nBlockAlign = m_pAVCodecContext->block_align;
                m_wf_in.wBitsPerSample = 0;
                m_wf_in.cbSize = 0;

                m_wf_out.wFormatTag = MMP_WAVE_FORMAT_PCM;
                if(m_pAVCodecContext->channels > 2) {
                    m_wf_out.nChannels = 2;
                }
                else {
                    m_wf_out.nChannels = m_pAVCodecContext->channels;
                }
                m_wf_out.wBitsPerSample = MMP_DEFAULT_AUDIO_OUT_SAMPLEBITS;//av_get_bits_per_sample_fmt(m_pAVCodecContext->sample_fmt);
                m_wf_out.nSamplesPerSec = m_pAVCodecContext->sample_rate;
                m_wf_out.nBlockAlign = m_pAVCodecContext->block_align;
                m_wf_out.nAvgBytesPerSec = m_wf_out.nSamplesPerSec * m_wf_out.nChannels * m_wf_out.wBitsPerSample / 8;
                m_wf_out.cbSize = 0;

                this->m_uiExpectedDecodedBufferSize = 4096;//m_pAVFrame_Decoded->nb_samples* m_wf_out.nChannels * m_wf_out.wBitsPerSample/8;

                this->AudioConvert_Create(m_pAVCodecContext);
            }
        }
#if 1
     
        if(pDecResult->uiDecodedBufferMaxSize >= this->m_uiExpectedDecodedBufferSize) {
            
            MMP_U8* pOutBuffer = (MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM];

            memset(pOutBuffer, 0x00, this->m_uiExpectedDecodedBufferSize);
                    
            pDecResult->uiDecodedSize = this->m_uiExpectedDecodedBufferSize;
            pDecResult->uiTimeStamp = 0;// p_decoded_packet->timestamp;
            pDecResult->uiAudioSampleRate = m_pAVCodecContext->sample_rate;
            pDecResult->uiAudioFrameCount = pDecResult->uiDecodedSize/4; //data_size / numBytesPerSample;

            //p_decoded_packet->buf_index += buf_copy_size;
        }
        
#else
        this->AudioConvert_Processing(pMediaSample->uiTimeStamp, m_pAVFrame_Decoded, m_pAVCodecContext);
#endif
    }

/*
#if 0
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
        pDecResult->uiTimeStamp = p_decoded_packet->timestamp;
        pDecResult->uiAudioSampleRate = m_pAVCodecContext->sample_rate;
        pDecResult->uiAudioFrameCount = pDecResult->uiDecodedSize/2; //data_size / numBytesPerSample;
        

        if(p_decoded_packet->buf_index >= p_decoded_packet->buf_size) {
        
            m_queue_decoded.Delete(p_decoded_packet);
            delete p_decoded_packet->p_buffer;
            delete p_decoded_packet;
        }
    }
#else

    if(!m_queue_decoded.IsEmpty()) {
        struct audio_decoded_packet* p_decoded_packet;
        int buf_remain_size, buf_copy_size;
        MMP_U8* pOutBuffer = (MMP_U8*)pDecResult->uiDecodedBufLogAddr;

        m_queue_decoded.Delete(p_decoded_packet);
        buf_remain_size = p_decoded_packet->buf_size - p_decoded_packet->buf_index;
        if(pDecResult->uiDecodedBufMaxSize >= buf_remain_size) {
            buf_copy_size = buf_remain_size;
            
            memcpy(pOutBuffer, p_decoded_packet->p_buffer, buf_copy_size);
                    
            pDecResult->uiDecodedSize = buf_copy_size;
            pDecResult->uiTimeStamp = p_decoded_packet->timestamp;
            pDecResult->uiAudioSampleRate = m_pAVCodecContext->sample_rate;
            pDecResult->uiAudioFrameCount = pDecResult->uiDecodedSize/4; //data_size / numBytesPerSample;

            p_decoded_packet->buf_index += buf_copy_size;
        }
        
        free(p_decoded_packet->p_buffer);
        delete p_decoded_packet;
    }

#endif
*/

    dec_end_tick = CMmpUtil::GetTickCount();
    pDecResult->uiDecodedDuration = dec_end_tick - dec_start_tick;

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

#if 1

void CMmpDecoderAudio_Dummy::AudioConvert_Processing(MMP_U32 uiTimeStamp, AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext) {

    AudioConvert *ac;
    AudioData out, in;
    struct audio_decoded_packet* p_decoded_packet;
    int iret;

    //uint8_t* in_buf = new uint8_t[1024*1024];
    //uint8_t* out_buf = new uint8_t[1024*1024];

    p_decoded_packet = new struct audio_decoded_packet;
    if(p_decoded_packet != NULL) {

        p_decoded_packet->buf_size = pAVFrame_Decoded->nb_samples * m_pAVCodecContext->channels * 2; /* ch * sample_byte */
        p_decoded_packet->buf_index = 0;
        p_decoded_packet->p_buffer = (MMP_U8*)malloc(p_decoded_packet->buf_size);

        if(p_decoded_packet->p_buffer != NULL ) {

            p_decoded_packet->timestamp = uiTimeStamp;

            ff_audio_data_init(&in, &pAVFrame_Decoded->data[0], 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, pAVCodecContext->sample_fmt, 1 /*read_only*/, NULL);
            //ff_audio_data_init(&out, &p_decoded_packet->p_buffer, 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_S16, 0, NULL);
            ff_audio_data_init(&out, &p_decoded_packet->p_buffer, 0, m_wf_out.nChannels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_S16, 0, NULL);
            
            
            if(m_pAudioConvert != NULL) {
            
                iret = ff_audio_convert(m_pAudioConvert, &out, &in);
                
                if(iret == 0) {
                    this->m_queue_decoded.Add(p_decoded_packet);
                }
                else {
                    free(p_decoded_packet->p_buffer);
                    delete p_decoded_packet;
                }
            }
            else {
                
                free(p_decoded_packet->p_buffer);
                delete p_decoded_packet;
            }

        }

    }
}

#else
void CMmpDecoderAudio_Dummy::PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext) {

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

    //ff_audio_data_init(&in, &pAVFrame_Decoded->data[0], 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_FLTP, 1, NULL);
    ff_audio_data_init(&in, &pAVFrame_Decoded->data[0], 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, pAVCodecContext->sample_fmt, 1 /*read_only*/, NULL);
    ff_audio_data_init(&out, &p_decoded_packet->p_buffer, 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_S16, 0, NULL);

    ac = ff_audio_convert_alloc(&avr, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP, m_pAVCodecContext->channels, m_pAVCodecContext->sample_rate, 0);
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