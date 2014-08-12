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

#ifndef MMPDECODERVIDEO_MPEG4_PV_HPP__
#define MMPDECODERVIDEO_MPEG4_PV_HPP__

#include "MmpDecoderVideo.hpp"

struct tagvideoDecControls;

class CMmpDecoderVideo_Mpeg4_PV : public CMmpDecoderVideo
{
friend class CMmpDecoder;

private:

	int mMode;
	tagvideoDecControls *mHandle;
    char* m_RefBuffer;
    bool m_bDecodeDSI;
	
protected:
    CMmpDecoderVideo_Mpeg4_PV(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderVideo_Mpeg4_PV();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_U8* GetClassName() { return (const MMP_U8*)"Mpeg4_PV";}

public:
	virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize, MMP_BOOL* bConfigChange);
    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);

    virtual MMP_RESULT Flush();
};

#endif

