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

#ifndef _MMPDECODERFFMPEG_HPP__
#define _MMPDECODERFFMPEG_HPP__

#include "MmpDecoder.hpp"
#include "TemplateList.hpp"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libavresample/audio_convert.h"
}

struct audio_decoded_packet {
    MMP_U8* p_buffer;
    MMP_U32 buf_size;
    MMP_U32 buf_index;
    MMP_U64 timestamp;
};

class CMmpDecoderFfmpeg 
{
protected:
    AVCodecID m_AVCodecID;
    AVCodec *m_pAVCodec;
    AVCodecContext *m_pAVCodecContext;
    AVFrame *m_pAVFrame_Decoded;

    MMP_U8* m_extra_data;
    
protected:
    CMmpDecoderFfmpeg(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderFfmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    //void PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext);

public:
    virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize);
    //virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
    
};

#endif

