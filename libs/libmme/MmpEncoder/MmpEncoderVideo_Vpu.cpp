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

#include "MmpEncoderVideo_Vpu.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo_Vpu Member Functions

CMmpEncoderVideo_Vpu::CMmpEncoderVideo_Vpu(struct MmpEncoderCreateConfig *pCreateConfig) : CMmpEncoderVideo(pCreateConfig, MMP_FALSE), CMmpEncoderVpu(pCreateConfig)
,m_temp_picture_buffer(NULL)
,m_nEncodedStreamCount(0)
,m_bEncodeDSI(MMP_FALSE)
{
    
}

CMmpEncoderVideo_Vpu::~CMmpEncoderVideo_Vpu()
{

}

MMP_RESULT CMmpEncoderVideo_Vpu::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpEncoderVpu::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Vpu::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    
    m_temp_picture_buffer = new MMP_U8[1920*1088*3/2];

    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoderVideo_Vpu::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderVpu::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::Close] CMmpEncoderVpu::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpEncoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::Close] CMmpEncoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Vpu::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    if(m_temp_picture_buffer != NULL) {
        delete [] m_temp_picture_buffer;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVideo_Vpu::EncodeAu(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

    MMP_RESULT mmpResult = MMP_SUCCESS; 
    MMP_U32 enc_start_tick, enc_end_tick;
    MMP_U8* pTemp;
    MMP_U8* pEncBuffer;
    MMP_U32 nEncBufSize, nEncBufMaxSize;
    MMP_U32 nEncFlag;

    enc_start_tick = CMmpUtil::GetTickCount();

    pEncResult->uiEncodedStreamSize[0] = 0;
    pEncResult->uiEncodedStreamSize[1] = 0;
    pEncResult->uiTimeStamp = pMediaSample->uiTimeStamp;
    pEncResult->uiFlag = 0;
    
    if(m_bEncodeDSI == MMP_FALSE) {
    
        mmpResult = CMmpEncoderVpu::EncodeDSI();
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
        m_bEncodeDSI = MMP_TRUE;

        this->EncodedFrameQueue_AddFrame((MMP_U8*)this->m_DSI, this->m_DSISize, MMP_ENCODED_FLAG_VIDEO_CONFIGDATA);

        pTemp = (MMP_U8*)this->m_DSI;
        MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::EncodeAu] ln=%d Sz=(%d) (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                this->m_DSISize,
               (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
               (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
             ));
   }


    mmpResult = CMmpEncoderVpu::EncodeAuEx1(pMediaSample, pEncResult);
    if(mmpResult == MMP_SUCCESS) {

        if(pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] > 0) {

            if(m_nEncodedStreamCount == 0) {

                pEncBuffer = (MMP_U8*)pEncResult->uiEncodedBufferLogAddr[MMP_ENCODED_BUF_STREAM];
                nEncBufSize = pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM];
                nEncBufMaxSize = pEncResult->uiEncodedBufferMaxSize[MMP_ENCODED_BUF_STREAM];

                this->EncodedFrameQueue_AddFrame(pEncBuffer, nEncBufSize, pEncResult->uiFlag);
                if(this->EncodedFrameQueue_IsEmpty() != MMP_TRUE) {

                    this->EncodedFrameQueue_GetFrame(pEncBuffer, nEncBufMaxSize, &nEncBufSize, &nEncFlag);

                    pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nEncBufSize;
                    pEncResult->uiFlag |= nEncFlag;

                    m_nEncodedStreamCount++;
                }
            }
        }
    }

    
    enc_end_tick = CMmpUtil::GetTickCount();
    pEncResult->uiEncodedDuration = enc_end_tick - enc_start_tick;

    CMmpEncoderVideo::EncodeMonitor(pMediaSample, pEncResult);

    return mmpResult;
}