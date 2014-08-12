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

#ifndef _MMPDECODERAUDIO_AAC_PV_HPP__
#define _MMPDECODERAUDIO_AAC_PV_HPP__

#include "MmpDecoderAudio.hpp"

#if (MMP_OS == MMP_OS_LINUX)
#define OSCL_IMPORT_REF
#endif
#include "pvmp4audiodecoder_api.h"


class CMmpDecoderAudio_AAC_PV : public CMmpDecoderAudio
{
friend class CMmpDecoder;

private:
    tPVMP4AudioDecoderExternal *mConfig;
	void* mDecoderBuf;
	MMP_BOOL m_bADTS;
	MMP_U32 mInputBufferCount;
	size_t mUpsamplingFactor;

    MMP_BOOL m_bConfigOK;

protected:
    CMmpDecoderAudio_AAC_PV(CMmpMediaInfo* pMediaInfo=NULL);
    CMmpDecoderAudio_AAC_PV(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderAudio_AAC_PV();

    virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT Close();


public:
    virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);

	virtual MMP_U32 GetAudioSampleRate();
	virtual MMP_U32 GetAudioChannelCount();

    virtual MMP_RESULT Flush();
};

#endif

