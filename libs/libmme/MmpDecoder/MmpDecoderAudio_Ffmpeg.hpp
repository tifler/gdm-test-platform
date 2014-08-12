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

#ifndef _MMPDECODERAUDIO_FFMPEG_HPP__
#define _MMPDECODERAUDIO_FFMPEG_HPP__

#include "MmpDecoderAudio.hpp"
#include "MmpDecoderFfmpeg.hpp"


class CMmpDecoderAudio_Ffmpeg : public CMmpDecoderAudio, CMmpDecoderFfmpeg
{
friend class CMmpDecoder;

private:
    MMP_U32 m_reconfig_count;

    struct AVAudioResampleContext m_AVAudioResampleContext;
    AudioConvert *m_pAudioConvert;

    TCircular_Queue<struct audio_decoded_packet*> m_queue_decoded;
   
    
protected:
    CMmpDecoderAudio_Ffmpeg(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderAudio_Ffmpeg();

    virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT Close();

    
    MMP_RESULT AudioConvert_Create(AVCodecContext *pAVCodecContext);
    //MMP_RESULT AudioConvert_Destroy();
    void AudioConvert_Processing(MMP_U64 uiTimeStamp, AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext);


public:
    virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
    
};

#endif

