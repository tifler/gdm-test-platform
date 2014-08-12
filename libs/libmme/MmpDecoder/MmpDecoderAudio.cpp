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

#include "MmpDecoderAudio.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderAudio Member Functions

CMmpDecoderAudio::CMmpDecoderAudio(CMmpMediaInfo* pMediaInfo) : CMmpDecoder(pMediaInfo)
,m_uiExpectedDecodedBufferSize(MMP_DEFAULT_AUDIO_DECODED_BUFSIZE)
{
    m_pwf=(MMPWAVEFORMATEX*)pMediaInfo->GetMediaInfo();

    memset(&m_wf_in, 0x00, sizeof(MMPWAVEFORMATEX));
    memset(&m_wf_out, 0x00, sizeof(MMPWAVEFORMATEX));
}

CMmpDecoderAudio::CMmpDecoderAudio(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoder(pCreateConfig->nFormat, pCreateConfig->nStreamType)
,m_uiExpectedDecodedBufferSize(MMP_DEFAULT_AUDIO_DECODED_BUFSIZE)
{
    memset(&m_wf_in, 0x00, sizeof(MMPWAVEFORMATEX));
    memset(&m_wf_out, 0x00, sizeof(MMPWAVEFORMATEX));
}

CMmpDecoderAudio::~CMmpDecoderAudio()
{

}

MMP_RESULT CMmpDecoderAudio::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio::Open(MMP_U8* pStream, MMP_U32 nStreamSize) {
	
	return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}



