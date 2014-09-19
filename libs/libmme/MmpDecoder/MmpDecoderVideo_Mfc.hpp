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

#ifndef _MMPDECODERVIDEO_MFC_HPP__
#define _MMPDECODERVIDEO_MFC_HPP__

#include "MmpDecoderVideo.hpp"
#include "MmpDecoderMfc.hpp"
#include "MmpDecoderMfcAndroid44.hpp"

#if ((MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC) || (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC_ANDROID44) )

class CMmpDecoderVideo_Mfc : public CMmpDecoderVideo, CMmpDecoderMfc
{
friend class CMmpDecoder;

private:

protected:
    CMmpDecoderVideo_Mfc(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderVideo_Mfc();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_U8* GetClassName() { return (const MMP_U8*)"Mfc";}
private:

public:
    virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
};

#endif /* #if ((MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC) || (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC_ANDROID44) ) */
#endif

