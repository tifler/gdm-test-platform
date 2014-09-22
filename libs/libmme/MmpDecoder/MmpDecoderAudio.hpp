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

#ifndef _MMPDECODERAUDIO_HPP__
#define _MMPDECODERAUDIO_HPP__

#include "MmpDecoder.hpp"

#define MMP_DEFAULT_AUDIO_DECODED_BUFSIZE 8192
#define MMP_DEFAULT_AUDIO_OUT_CHANNEL    2
#define MMP_DEFAULT_AUDIO_OUT_SAMPLEBITS 16

class CMmpDecoderAudio : public CMmpDecoder
{
friend class CMmpDecoder;

protected:
    MMPWAVEFORMATEX* m_pwf;
    
    MMPWAVEFORMATEX m_wf_in;
    MMPWAVEFORMATEX m_wf_out;

    MMP_U32 m_uiExpectedDecodedBufferSize;

protected:
    CMmpDecoderAudio(CMmpMediaInfo* pMediaInfo);
    CMmpDecoderAudio(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderAudio();

    virtual MMP_RESULT Open();
	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT Close();

public:
//virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize, MMP_BOOL* bConfigChange) {return MMP_FAILURE;}
    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) = 0;
	
    inline const MMPWAVEFORMATEX& GetWF_In() { return m_wf_in; }
    inline const MMPWAVEFORMATEX& GetWF_Out() { return m_wf_out; }

    inline MMP_U32 GetExpectedDecodedBufferSize() { return m_uiExpectedDecodedBufferSize; }
};

#endif

