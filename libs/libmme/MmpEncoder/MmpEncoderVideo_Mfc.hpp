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

#ifndef MMPENCODERVIDEO_MFC_HPP__
#define MMPENCODERVIDEO_MFC_HPP__

#include "MmpEncoderVideo.hpp"
#include "MmpEncoderMfc.hpp"
#include "mmp_oal_mutex.hpp"

#if ((MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) || (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC_ANDROID44) )

class CMmpEncoderVideo_Mfc : public CMmpEncoderVideo, CMmpEncoderMfc
{
friend class CMmpEncoder;

private:
    MMP_U32 m_nEncodedStreamCount;
    MMP_U32 m_nInputFrameCount;
   
    class mmp_oal_mutex* m_p_hwsync_mutex;
    
protected:
    CMmpEncoderVideo_Mfc(struct MmpEncoderCreateConfig *pCreateConfig);
    virtual ~CMmpEncoderVideo_Mfc();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_U8* GetClassName() { return (const MMP_U8*)"Mfc";}
public:
    virtual MMP_RESULT EncodeAu(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult);
};

#endif /* #if (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) */
#endif

