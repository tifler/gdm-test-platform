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
#include "MmpEncoderVideo.hpp"
#include "MmpUtil.hpp"
#include "MmpMpeg4Tool.hpp"

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo Member Functions

CMmpEncoderVideo::CMmpEncoderVideo(struct MmpEncoderCreateConfig *pCreateConfig, MMP_BOOL bNeedPictureBufPhyAddr) : CMmpEncoder(pCreateConfig->nFormat, pCreateConfig->nStreamType) 
,m_bNeedPictureBufPhyAddr(bNeedPictureBufPhyAddr)
,m_queue_ecnframe(10)
{

	/* In format */
	m_bih_in.biSize = sizeof(MMPBITMAPINFOHEADER);
    m_bih_in.biWidth = pCreateConfig->nPicWidth;
    m_bih_in.biHeight = pCreateConfig->nPicHeight;
	m_bih_in.biPlanes = 1;
	m_bih_in.biBitCount = 24;
	m_bih_in.biCompression = MMP_FOURCC_VIDEO_YV12;
	m_bih_in.biSizeImage = 0;
	m_bih_in.biXPelsPerMeter = 0;
	m_bih_in.biYPelsPerMeter = 0;
	m_bih_in.biClrUsed = 0;
	m_bih_in.biClrImportant = 0;

	/* out format */
	m_bih_out.biSize = sizeof(MMPBITMAPINFOHEADER);
	m_bih_out.biWidth = m_bih_in.biWidth;
	m_bih_out.biHeight = m_bih_in.biHeight;
	m_bih_out.biPlanes = 3;
	m_bih_out.biBitCount = 12;
	m_bih_out.biCompression = pCreateConfig->nFormat;
	m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);
	m_bih_out.biXPelsPerMeter = 0;
	m_bih_out.biYPelsPerMeter = 0;
	m_bih_out.biClrUsed = 0;
	m_bih_out.biClrImportant = 0;

    strcpy((char*)m_szCodecName, "Unknown");
}


CMmpEncoderVideo::~CMmpEncoderVideo()
{

}

MMP_RESULT CMmpEncoderVideo::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpEncoder::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoderVideo::Close()
{
    MMP_RESULT mmpResult;
    struct mmp_enc_video_frame enc_frame;

    mmpResult=CMmpEncoder::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    while(!m_queue_ecnframe.IsEmpty()) {
        m_queue_ecnframe.Delete(enc_frame);
        if(enc_frame.pData != NULL) {
            delete [] enc_frame.pData;
        }
    }

    return MMP_SUCCESS;
}

void CMmpEncoderVideo::SetVideoSize(MMP_U32 w, MMP_U32 h) {

    /* In format */
	m_bih_in.biSize = sizeof(MMPBITMAPINFOHEADER);
	m_bih_in.biWidth = w;
	m_bih_in.biHeight = h;
	m_bih_in.biPlanes = 1;
	m_bih_in.biBitCount = 24;
	m_bih_in.biCompression = MMP_FOURCC_VIDEO_MPEG4;
	m_bih_in.biSizeImage = 0;
	m_bih_in.biXPelsPerMeter = 0;
	m_bih_in.biYPelsPerMeter = 0;
	m_bih_in.biClrUsed = 0;
	m_bih_in.biClrImportant = 0;

	/* out format */
	m_bih_out.biSize = sizeof(MMPBITMAPINFOHEADER);
	m_bih_out.biWidth = w;
	m_bih_out.biHeight = h;
	m_bih_out.biPlanes = 3;
	m_bih_out.biBitCount = 12;
	m_bih_out.biCompression = MMP_FOURCC_VIDEO_YV12;
	m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);
	m_bih_out.biXPelsPerMeter = 0;
	m_bih_out.biYPelsPerMeter = 0;
	m_bih_out.biClrUsed = 0;
	m_bih_out.biClrImportant = 0;

}

void CMmpEncoderVideo::EncodeMonitor(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

    static MMP_U32 before_tick = 0, fps_sum=0, dur_sum=0;
    MMP_U32 start_tick = m_nClassStartTick, cur_tick;
    MMP_U32 dur_avg = 0;

    if(pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] > 0) {
        fps_sum ++;
        dur_sum += pEncResult->uiEncodedDuration;
    }

    cur_tick = CMmpUtil::GetTickCount();
    if( (cur_tick - before_tick) > 1000 ) {
    
        if(fps_sum != 0) {
            dur_avg = dur_sum/fps_sum;
        }
        
        MMPDEBUGMSG(1, (TEXT("[VideoEnc %s %s %dx%d] %d. fps=%d dur=%d "), 
                    this->GetClassName(),   m_szCodecName,  m_bih_out.biWidth, m_bih_out.biHeight,
                    (cur_tick-start_tick)/1000, fps_sum, dur_avg ));

        before_tick = cur_tick;
        fps_sum = 0;
        dur_sum = 0;
    }
    
}

MMP_BOOL CMmpEncoderVideo::EncodedFrameQueue_IsEmpty() {
    
    return m_queue_ecnframe.IsEmpty()?MMP_TRUE:MMP_FALSE;
}
    
MMP_RESULT CMmpEncoderVideo::EncodedFrameQueue_GetFrame(MMP_U8* pBuffer, MMP_U32 nBufMaxSize, MMP_U32* nRetBufSize, MMP_U32* nRetFlag) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    struct mmp_enc_video_frame enc_frame;

    if(!m_queue_ecnframe.IsEmpty()) {
        
        m_queue_ecnframe.GetFirstItem(enc_frame);
        if(enc_frame.nDataSize <= nBufMaxSize) {
            
            m_queue_ecnframe.Delete(enc_frame);
            if(enc_frame.pData != NULL) {
            
                memcpy(pBuffer, enc_frame.pData, enc_frame.nDataSize);
                if(nRetBufSize!=NULL) *nRetBufSize = enc_frame.nDataSize;
                if(nRetFlag!=NULL) *nRetFlag = enc_frame.nFlag;
                mmpResult = MMP_SUCCESS;

                delete [] enc_frame.pData;
            }
        }

    }

    return mmpResult;
}

MMP_RESULT CMmpEncoderVideo::EncodedFrameQueue_AddFrame(MMP_U8* pBuffer, MMP_U32 nBufSize, MMP_U32 nFlag) {

    struct mmp_enc_video_frame enc_frame;

    if(m_queue_ecnframe.IsFull()) {
    
        m_queue_ecnframe.Delete(enc_frame);
        if(enc_frame.pData != NULL) {
            delete [] enc_frame.pData;
        }
    }

    enc_frame.pData = new MMP_U8[nBufSize];
    if(enc_frame.pData != NULL) {
        enc_frame.nFlag = nFlag;
        enc_frame.nDataSize = nBufSize;
        memcpy(enc_frame.pData, pBuffer, nBufSize);
        m_queue_ecnframe.Add(enc_frame);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVideo::EncodedFrameQueue_AddFrameWithConfig_Mpeg4(MMP_U8* pBuffer, MMP_U32 nBufSize, MMP_U32 nFlag) {

#define MAX_FRMAE_COUNT 10
    CMmpBitExtractor be;
    MMP_RESULT mmpResult;
    MMP_U32 nStartCodeIndex[MAX_FRMAE_COUNT];
    MMP_U32 nStartCodeCount = 0;
    MMP_U32 i, j, vop_index, framesize, maxcount;
    MMP_U8 code;
    unsigned long d;

    be.Start(pBuffer, nBufSize);
    maxcount = nBufSize;

    while(1) {
    
        maxcount -= be.GetCurByteIndex();

        mmpResult = CMmpMpeg4Parser::Decode_NextStartCodePrefix(&be, maxcount);
        if(mmpResult != MMP_SUCCESS) {
            break;
        }

        i = be.GetCurByteIndex();
        nStartCodeIndex[nStartCodeCount] = i-3;
        nStartCodeCount++;

        if(nStartCodeCount >= MAX_FRMAE_COUNT) {
            break;
        }

        be.Pop_BitCode(d, 8);
        d&=0xFF;

        if(d == MPEG4_VIDEO_VOP) {
            break;
        }
    }

    for(vop_index = 0; vop_index < nStartCodeCount; vop_index++) {
    
        j = nStartCodeIndex[vop_index];
        code = pBuffer[j+3];

        if(code == MPEG4_VIDEO_VOP) {
            break;
        }
    }

    //Add Config Data
    if(vop_index == nStartCodeCount) {
        framesize = nBufSize;
    }
    else {
        framesize = nStartCodeIndex[vop_index];  
    }
    if(framesize > 0) {
        this->EncodedFrameQueue_AddFrame(pBuffer, framesize, MMP_ENCODED_FLAG_VIDEO_CONFIGDATA);
    }
    
    //Add VOP Data
    if(vop_index < nStartCodeCount) {
        framesize = nBufSize-nStartCodeIndex[vop_index];  //suppose 00 00 01
        if(framesize > 0) {
            j = nStartCodeIndex[vop_index];
            this->EncodedFrameQueue_AddFrame(&pBuffer[j], framesize, nFlag);
        }
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVideo::Encode_YUV420Planar_Vir(MMP_U8* Y, MMP_U8* U, MMP_U8* V, 
                                                     MMP_U8* pEncStreamBuf, MMP_U32 nBufMaxSize, MMP_U32* nBufSize, MMP_U32* nFlag) {

    MMP_RESULT mmpResult;

    CMmpMediaSampleEncode EncMediaSample;
    CMmpMediaSampleEncode* pEncMediaSample=&EncMediaSample;
    CMmpMediaSampleEncodeResult EncResult;
    CMmpMediaSampleEncodeResult* pEncResult=&EncResult;

    if(nBufSize) *nBufSize = 0;
    if(nFlag) *nFlag = 0;

    pEncMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y] = (MMP_U32)Y;
    pEncMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U] = (MMP_U32)U;
    pEncMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_V] = (MMP_U32)V;

    pEncMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y] = 0;
    pEncMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U] = 0;
    pEncMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_V] = 0;
    
    pEncMediaSample->uiBufferStride[MMP_DECODED_BUF_Y] = m_bih_in.biWidth;
    pEncMediaSample->uiBufferStride[MMP_DECODED_BUF_U] = m_bih_in.biWidth>>1;
    pEncMediaSample->uiBufferStride[MMP_DECODED_BUF_V] = m_bih_in.biWidth>>1;

    pEncMediaSample->pixelformat = MMP_PIXELFORMAT_YUV420_PLANAR;;
    pEncMediaSample->uiBufferMaxSize = 0;
    pEncMediaSample->uiTimeStamp = 0;
    pEncMediaSample->uiFlag = 0;

    pEncResult->uiEncodedBufferLogAddr[0] = (MMP_U32)pEncStreamBuf;
    pEncResult->uiEncodedBufferMaxSize[0] = nBufMaxSize;
    pEncResult->uiEncodedStreamSize[0] = 0;
    pEncResult->uiFlag = 0;

    mmpResult = this->EncodeAu(pEncMediaSample, pEncResult);
    if(mmpResult == MMP_SUCCESS) {
    
        if(nBufSize) *nBufSize = pEncResult->uiEncodedStreamSize[0];
        if(nFlag) *nFlag = pEncResult->uiFlag;
    }

    return mmpResult;
}

