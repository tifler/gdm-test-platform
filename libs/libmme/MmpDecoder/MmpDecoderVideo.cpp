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

#include "MmpDecoderVideo.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo Member Functions

CMmpDecoderVideo::CMmpDecoderVideo(struct MmpDecoderCreateConfig *pCreateConfig, MMP_BOOL bNeedDecodedBufPhyAddr) : CMmpDecoder(pCreateConfig->nFormat, pCreateConfig->nStreamType) 
,m_bNeedDecodedBufPhyAddr(bNeedDecodedBufPhyAddr)
{

	/* In format */
	m_bih_in.biSize = sizeof(MMPBITMAPINFOHEADER);
    m_bih_in.biWidth = pCreateConfig->nPicWidth;
    m_bih_in.biHeight = pCreateConfig->nPicHeight;
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
	m_bih_out.biWidth = m_bih_in.biWidth;
	m_bih_out.biHeight = m_bih_in.biHeight;
	m_bih_out.biPlanes = 3;
	m_bih_out.biBitCount = 12;
	m_bih_out.biCompression = MMP_FOURCC_VIDEO_YV12;
	m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);
	m_bih_out.biXPelsPerMeter = 0;
	m_bih_out.biYPelsPerMeter = 0;
	m_bih_out.biClrUsed = 0;
	m_bih_out.biClrImportant = 0;

    strcpy((char*)m_szCodecName, "Unknown");
}


CMmpDecoderVideo::~CMmpDecoderVideo()
{

}

MMP_RESULT CMmpDecoderVideo::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

void CMmpDecoderVideo::SetVideoSize(MMP_U32 w, MMP_U32 h) {

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

void CMmpDecoderVideo::DecodeMonitor(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) {

    static MMP_U32 before_tick = 0, fps_sum=0, dur_sum=0;
    MMP_U32 start_tick = m_nClassStartTick, cur_tick;
    MMP_U32 dur_avg = 0;

    if(pDecResult->uiDecodedSize > 0) {
        fps_sum ++;
        dur_sum += pDecResult->uiDecodedDuration;
    }

    cur_tick = CMmpUtil::GetTickCount();
    if( (cur_tick - before_tick) > 1000 ) {
    
        if(fps_sum != 0) {
            dur_avg = dur_sum/fps_sum;
        }
        
        MMPDEBUGMSG(1, (TEXT("[VideoDec %s %s %dx%d] %d. fps=%d dur=%d "), 
                    this->GetClassName(),   m_szCodecName,  m_bih_out.biWidth, m_bih_out.biHeight,
                    (cur_tick-start_tick)/1000, fps_sum, dur_avg ));

        before_tick = cur_tick;
        fps_sum = 0;
        dur_sum = 0;
    }
    
}