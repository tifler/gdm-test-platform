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

#include "MmpDecoderVideo_Mpeg4_PV.hpp"
#include "../MmpComm/MmpUtil.hpp"

#if (MMP_OS == MMP_OS_LINUX)
#define OSCL_IMPORT_REF
#endif

#include "mp4dec_api.h"

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Mpeg4_PV Member Functions

CMmpDecoderVideo_Mpeg4_PV::CMmpDecoderVideo_Mpeg4_PV(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderVideo(pCreateConfig, MMP_FALSE)
,mMode(MPEG4_MODE)
,mHandle(NULL)
,m_RefBuffer(NULL)
,m_bDecodeDSI(false)
{
     m_bih_in.biCompression = MMP_FOURCC_VIDEO_MPEG4;
}

CMmpDecoderVideo_Mpeg4_PV::~CMmpDecoderVideo_Mpeg4_PV()
{
    if(m_RefBuffer) delete [] m_RefBuffer;
}

MMP_RESULT CMmpDecoderVideo_Mpeg4_PV::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

	mHandle = new tagvideoDecControls;
	if(mHandle==NULL) {
		mmpResult = MMP_ErrorInsufficientResources;
	}
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_Mpeg4_PV::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Mpeg4_PV::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

	if(mHandle != NULL) {

		if(this->IsConfigDSI() == MMP_TRUE) {
			PVCleanUpVideoDecoder(mHandle);
		}

		delete mHandle;
		mHandle = NULL;
	}

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVideo_Mpeg4_PV::Flush() {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    if(mHandle != NULL) {
        PVResetVideoDecoder(mHandle);
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVideo_Mpeg4_PV::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize, MMP_BOOL* bConfigChange) {

	MP4DecodingMode actualMode;
	Bool success;
	MMP_RESULT mmpResult = MMP_SUCCESS;
	uint8_t *vol_data[1];
    int32_t vol_size = 0;
	int32_t disp_width, disp_height;
	int32_t buf_width, buf_height;

	vol_data[0] = pStream;
	vol_size = nStreamSize;

	if(bConfigChange!=NULL) *bConfigChange = MMP_FALSE;

	success = PVInitVideoDecoder(mHandle, vol_data, &vol_size, 1, m_bih_in.biWidth, m_bih_in.biHeight, (MP4DecodingMode)mMode);
	if(!success) {
		mmpResult = MMP_FAILURE;
	}

	if(mmpResult == MMP_SUCCESS) {
	
		actualMode = PVGetDecBitstreamMode(mHandle);
        if(mMode != actualMode) {
            mmpResult = MMP_FAILURE;
        }
		else {
		
			PVSetPostProcType((VideoDecControls *)mHandle, 0);

			PVGetVideoDimensions(mHandle, &disp_width, &disp_height);
			PVGetBufferDimensions(mHandle, &buf_width, &buf_height);

		    //CHECK_LE(disp_width, buf_width);
		    //CHECK_LE(disp_height, buf_height);

			//MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Mpeg4_PV::DecodeDSI] %d %d %d %d \n\r"), disp_width, disp_height, buf_width, buf_height ));

			if( (m_bih_out.biWidth != disp_width) ||  (m_bih_out.biHeight != disp_height) 
				|| (m_bih_in.biWidth != buf_width) ||  (m_bih_in.biHeight != buf_height) ) {
			
					if(bConfigChange!=NULL) *bConfigChange = MMP_TRUE;
			}

			m_bih_out.biWidth = disp_width;
			m_bih_out.biHeight = disp_height;
			m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

			m_bih_in.biWidth = buf_width;
			m_bih_in.biHeight = buf_height;

            m_RefBuffer = new char[m_bih_in.biWidth*m_bih_in.biHeight*3];
            PVSetReferenceYUV(mHandle, (uint8*)m_RefBuffer);
		}
	}

	return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_Mpeg4_PV::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
	MMP_RESULT mmpResult = MMP_SUCCESS;
	uint32_t timestamp = 0;
	uint useExtTimestamp = 0;
	int32 buffer_size;
    static int cnt = 0;

    pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    
    if(m_bDecodeDSI == false) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize, NULL);
        if(mmpResult != MMP_SUCCESS) {
            
        }
        else {
            m_bDecodeDSI = true;

        }
        
        return mmpResult;
    }


	buffer_size = (int32)pMediaSample->uiAuSize;

	// The PV decoder is lying to us, sometimes it'll claim to only have
    // consumed a subset of the buffer when it clearly consumed all of it.
    // ignore whatever it says...
    if (PVDecodeVideoFrame(
				mHandle, 
				&pMediaSample->pAu, 
				&timestamp, 
				(int32*)&buffer_size,
                &useExtTimestamp,
				(uint8*)pDecResult->uiDecodedBufferLogAddr[0]) != PV_TRUE) {
        //ALOGE("failed to decode video frame.");

        //notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
        //mSignalledError = true;
        mmpResult = MMP_FAILURE;
    }

	pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
	pDecResult->uiDecodedSize = (m_bih_out.biWidth*m_bih_out.biHeight*3)/2;

	MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Mpeg4_PV::DecodeAu] res=%d ts=%d cnt=%d %d \n\r"), mmpResult, timestamp, cnt, cnt/30 ));
    cnt++;

	return mmpResult;

}