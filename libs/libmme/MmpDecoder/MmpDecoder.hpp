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

#ifndef _MMPDECODER_HPP__
#define _MMPDECODER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "MmpMediaInfo.hpp"
#include "mmp_buffer_mgr.hpp"

struct MmpDecoderCreateConfig {
    MMP_U32 nFormat;
    MMP_U32 nStreamType;
    MMP_BOOL bThumbnailMode;

    MMP_U32 nPicWidth;
    MMP_U32 nPicHeight;

    MMP_U8* pStream;
    MMP_U32 nStreamSize;

    /* mutex handle */
    void* hw_sync_mutex_hdl;
};

class CMmpDecoder
{
public:
    static CMmpDecoder* CreateAudioObject(struct MmpDecoderCreateConfig *pCreateConfig /*MMP_U32 nFormat, MMP_U32 nStreamType, MMP_U8* pStream, MMP_U32 nStreamSize*/);
    static CMmpDecoder* CreateVideoObject(struct MmpDecoderCreateConfig *pCreateConfig, MMP_BOOL bForceFfmpeg = MMP_FALSE);

    static MMP_RESULT DestroyObject(CMmpDecoder* pObj);

protected:
    CMmpMediaInfo* m_pMediaInfo;
	CMmpMediaInfo m_MediaInfo;

	MMP_U32 m_nFormat;
	MMP_U32 m_nStreamType;

	MMP_BOOL m_bConfigDSI;

    MMP_U32 m_nClassStartTick;

protected:
    CMmpDecoder(CMmpMediaInfo* pMediaInfo);
    CMmpDecoder(MMP_U32 nFormat, MMP_U32 nStreamType);
    virtual ~CMmpDecoder();

    virtual MMP_RESULT Open();
	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize) = 0;
    virtual MMP_RESULT Close();

public:
	virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize, MMP_BOOL* bConfigChange) {return MMP_FAILURE;}
    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) {return MMP_FAILURE;}
	
    virtual int GetPicWidth() { return 0; }
    virtual int GetPicHeight() { return 0; }

	virtual MMP_U32 GetAudioSampleRate() { return 0; }
	virtual MMP_U32 GetAudioChannelCount() { return 0; }

	MMP_BOOL IsConfigDSI() { return m_bConfigDSI; }
	virtual MMP_RESULT GetWF(MMPWAVEFORMATEX* pwf) {return MMP_FAILURE;}
	
    virtual MMP_RESULT Flush() {return MMP_FAILURE;}
};

#endif

