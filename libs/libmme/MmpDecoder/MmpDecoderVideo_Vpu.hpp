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

#ifndef MMPDECODERVIDEO_VPU_HPP__
#define MMPDECODERVIDEO_VPU_HPP__

#include "MmpDecoderVideo.hpp"
#include "MmpDecoderVpu.hpp"
#include "MmpDecoderVpuIF.hpp"

#ifdef __VPU_PLATFORM_MME
#define CLASS_DECODER_VPU CMmpDecoderVpuIF
#else
#define CLASS_DECODER_VPU CMmpDecoderVpu
#endif

class CMmpDecoderVideo_Vpu : public CMmpDecoderVideo, CLASS_DECODER_VPU
{
friend class CMmpDecoder;

private:
    MMP_BOOL m_bDecodeDSI;
    
    
protected:
    CMmpDecoderVideo_Vpu(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderVideo_Vpu();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_CHAR* GetClassName() { return (const MMP_CHAR*)"VPU";}
public:
    static MMP_BOOL CheckSupportCodec(MMP_U32 format);

    virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT DecodeDSI(class mmp_buffer_videostream* p_buf_videostream);
    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe);
};

#endif

