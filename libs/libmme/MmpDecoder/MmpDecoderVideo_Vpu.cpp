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

#include "MmpDecoderVideo_Vpu.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Vpu Member Functions

CMmpDecoderVideo_Vpu::CMmpDecoderVideo_Vpu(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderVideo(pCreateConfig, MMP_FALSE), CLASS_DECODER_VPU(pCreateConfig)
,m_bDecodeDSI(MMP_FALSE)
{
    
}

CMmpDecoderVideo_Vpu::~CMmpDecoderVideo_Vpu()
{

}

MMP_RESULT CMmpDecoderVideo_Vpu::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }
    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));


    mmpResult=CLASS_DECODER_VPU::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Vpu::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Vpu::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CLASS_DECODER_VPU::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Vpu::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Vpu::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Vpu::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Vpu::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult;

    mmpResult = CLASS_DECODER_VPU::DecodeDSI(pStream, nStreamSize);
    if(mmpResult == MMP_SUCCESS) {
    
        m_bih_out.biWidth = m_dec_init_info.picWidth;
        m_bih_out.biHeight = m_dec_init_info.picHeight;
	    m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	    m_bih_in.biWidth = m_bih_out.biWidth;
	    m_bih_in.biHeight = m_bih_out.biHeight;
        
        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Vpu::DecodeDSI] Success nForamt=(0x%08x %s)  W=%d H=%d  FrmCnt=%d+1 \n\r"), 
                  m_nFormat, m_szCodecName,
                  m_bih_out.biWidth,
                  m_bih_out.biHeight  ,
                  m_dec_init_info.minFrameBufferCount
                  ));

    }

    return mmpResult;

}

MMP_RESULT CMmpDecoderVideo_Vpu::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 dec_start_tick, dec_end_tick;

    pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    pDecResult->uiDecodedDuration = 0;
    
    if(m_bDecodeDSI == MMP_FALSE) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
        m_bDecodeDSI = MMP_TRUE;
    }

    if((pMediaSample->uiFlag&MMP_MEDIASAMPMLE_FLAG_CONFIGDATA) != 0) {

        pDecResult->uiDecodedSize = 0;
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
        
        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();

     mmpResult = this->DecodeAu_PinEnd(pMediaSample, pDecResult);
    
    
    dec_end_tick = CMmpUtil::GetTickCount();
    pDecResult->uiDecodedDuration = dec_end_tick - dec_start_tick;
    CMmpDecoderVideo::DecodeMonitor(pMediaSample, pDecResult);

	return mmpResult; 

}

