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

#ifndef MMPDECODERVIDEO_HPP__
#define MMPDECODERVIDEO_HPP__

#include "MmpDecoder.hpp"

class CMmpDecoderVideo : public CMmpDecoder
{
friend class CMmpDecoder;

protected:
    MMP_U8 m_szCodecName[32];
    MMPBITMAPINFOHEADER* m_pbih;;
	MMPBITMAPINFOHEADER m_bih_in;
	MMPBITMAPINFOHEADER m_bih_out;

private:
    MMP_BOOL m_bNeedDecodedBufPhyAddr;
    MMP_S32 m_nDecodingAvgFPS;
    
    MMP_U32 m_nTotalDecDur;
    MMP_U32 m_nTotalDecFrameCount;

protected:
    CMmpDecoderVideo(struct MmpDecoderCreateConfig *pCreateConfig, MMP_BOOL bNeedDecodedBufPhyAddr);
    virtual ~CMmpDecoderVideo();

    virtual MMP_RESULT Open();
	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize) {return MMP_FAILURE;}
    virtual MMP_RESULT Close();

    void DecodeMonitor(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
    void DecodeMonitor(class mmp_buffer_videoframe* p_buf_videoframe);

    
public:
    virtual const MMP_CHAR* GetClassName() = 0;

	inline const MMPBITMAPINFOHEADER& GetBIH_Out() { return m_bih_out; }
	inline const MMPBITMAPINFOHEADER& GetBIH_In() { return m_bih_in; }
    inline MMP_BOOL IsNeedDecodedBufPhyAddr() { return m_bNeedDecodedBufPhyAddr; }

    void SetVideoSize(MMP_U32 w, MMP_U32 h);

    MMP_S32 GetAvgFPS();
    MMP_S32 GetAvgDur();
    MMP_S32 GetTotalDecFrameCount() { return m_nTotalDecFrameCount; }

    virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) = 0;
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe) = 0;
};

#endif

