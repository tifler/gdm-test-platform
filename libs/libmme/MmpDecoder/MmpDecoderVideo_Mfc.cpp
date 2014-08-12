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

#include "MmpDecoderVideo_Mfc.hpp"
#include "../MmpComm/MmpUtil.hpp"

#if ((MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) || (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC_ANDROID44) )

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Mfc Member Functions

CMmpDecoderVideo_Mfc::CMmpDecoderVideo_Mfc(struct MmpDecoderCreateConfig *pCreateConfig) : 
//CMmpDecoderVideo(pCreateConfig, MMP_TRUE /* HW CSC */), CMmpDecoderMfc(pCreateConfig)
CMmpDecoderVideo(pCreateConfig, MMP_FALSE /* SW CSC */), CMmpDecoderMfc(pCreateConfig)
{

}

CMmpDecoderVideo_Mfc::~CMmpDecoderVideo_Mfc()
{

}

MMP_RESULT CMmpDecoderVideo_Mfc::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpDecoderMfc::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mfc::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Mfc::Close()
{
    MMP_RESULT mmpResult;

   mmpResult=CMmpDecoderMfc::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Mfc::Close] CMmpDecoderMfc::Close() \n\r")));
        return mmpResult;
    }
    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Mfc::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mfc::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVideo_Mfc::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoderMfc::DecodeDSI(pStream, nStreamSize, m_szCodecName);
    if(mmpResult == MMP_SUCCESS) {
    
        m_bih_out.biWidth = m_MFCImgResol.width;
        m_bih_out.biHeight = m_MFCImgResol.height;
	    m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	    m_bih_in.biWidth = m_MFCImgResol.width;
	    m_bih_in.biHeight = m_MFCImgResol.height;

        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mfc::DecodeDSI] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));


    }
    
    return mmpResult;

}


#if 1

MMP_RESULT CMmpDecoderVideo_Mfc::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
	MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U32 dec_start_tick, dec_end_tick;
    
    dec_start_tick = CMmpUtil::GetTickCount();

	pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    pDecResult->uiDecodedDuration = 0;

    if(m_p_hwsync_mutex!=NULL) m_p_hwsync_mutex->lock();

    if(m_bDecodeDSI != MMP_TRUE) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize);
        if(mmpResult != MMP_SUCCESS) {
            if(m_p_hwsync_mutex!=NULL) m_p_hwsync_mutex->unlock();
            return mmpResult;
        }
    }

    if((pMediaSample->uiFlag&MMP_MEDIASAMPMLE_FLAG_CONFIGDATA) != 0) {

        pDecResult->uiDecodedSize = 0;
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
        
        if(m_p_hwsync_mutex!=NULL) m_p_hwsync_mutex->unlock();
        return MMP_SUCCESS;
    }

    //mmpResult = CMmpDecoderMfc::DecodeAu_Block(pMediaSample, pDecResult);
    mmpResult = CMmpDecoderMfc::DecodeAu_NonBlock(pMediaSample, pDecResult);

    if( (m_bih_out.biWidth != m_MFCImgResol.width) ||  (m_bih_out.biHeight != m_MFCImgResol.height) 
				|| (m_bih_in.biWidth != m_MFCImgResol.width) ||  (m_bih_in.biHeight != m_MFCImgResol.height) ) {

        m_bih_out.biWidth = m_MFCImgResol.width;
        m_bih_out.biHeight = m_MFCImgResol.height;
        m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

        m_bih_in.biWidth = m_MFCImgResol.width;
        m_bih_in.biHeight = m_MFCImgResol.height;
    }

    dec_end_tick = CMmpUtil::GetTickCount();
    pDecResult->uiDecodedDuration = dec_end_tick - dec_start_tick;

    CMmpDecoderVideo::DecodeMonitor(pMediaSample, pDecResult);

    if(m_p_hwsync_mutex!=NULL) m_p_hwsync_mutex->unlock();

    return mmpResult;
}

#else
MMP_RESULT CMmpDecoderVideo_Mfc::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
	MMP_RESULT mmpResult = MMP_SUCCESS;
    SSBSIP_MFC_ERROR_CODE mfcResult = MFC_RET_OK;
    SSBSIP_MFC_DEC_OUTBUF_STATUS mfcDecStatus;
    SSBSIP_MFC_DEC_OUTPUT_INFO outputInfo;
    unsigned char  *pSrcBuf[3], *pYUVBuf[3];
    SSBSIP_MFC_IMG_RESOLUTION MFCImgResol;
    MMP_U32 dec_start_tick, dec_end_tick;
    MMP_U32 csc_start_tick, csc_end_tick;

	pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    pDecResult->uiDecodedDuration = 0;

    if(m_bDecodeDSI != MMP_TRUE) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }
/*
    //Check Reconfig
    if( (m_bih_out.biWidth != m_MFCImgResol.width) ||  (m_bih_out.biHeight != m_MFCImgResol.height) 
		|| (m_bih_in.biWidth != m_MFCImgResol.width) ||  (m_bih_in.biHeight != m_MFCImgResol.height) ) {
		
        m_bih_out.biWidth = m_MFCImgResol.width;
        m_bih_out.biHeight = m_MFCImgResol.height;
        m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

        m_bih_in.biWidth = m_MFCImgResol.width;
        m_bih_in.biHeight = m_MFCImgResol.height;

        return MMP_SUCCESS;
    }
*/

    dec_start_tick = CMmpUtil::GetTickCount();
    
    memcpy(m_pStreamBuffer, pMediaSample->pAu, pMediaSample->uiAuSize);
    mfcResult = SsbSipMfcDecExe(m_hMFCHandle, pMediaSample->uiAuSize);
    pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;

    dec_end_tick = CMmpUtil::GetTickCount();

    if(mfcResult == MFC_RET_OK) {

        mfcDecStatus = SsbSipMfcDecGetOutBuf(m_hMFCHandle, &outputInfo);
    
        if( (mfcDecStatus == MFC_GETOUTBUF_DISPLAY_DECODING) 
            || (mfcDecStatus == MFC_GETOUTBUF_DISPLAY_ONLY) ) {

            mfcResult =SsbSipMfcDecGetConfig(m_hMFCHandle, MFC_DEC_GETCONF_BUF_WIDTH_HEIGHT, &MFCImgResol);
            if(mfcResult == MFC_RET_OK) {
                m_MFCImgResol = MFCImgResol;
            }

            if( (m_bih_out.biWidth != m_MFCImgResol.width) ||  (m_bih_out.biHeight != m_MFCImgResol.height) 
				|| (m_bih_in.biWidth != m_MFCImgResol.width) ||  (m_bih_in.biHeight != m_MFCImgResol.height) ) {
		
                m_bih_out.biWidth = m_MFCImgResol.width;
                m_bih_out.biHeight = m_MFCImgResol.height;
                m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

                m_bih_in.biWidth = m_MFCImgResol.width;
                m_bih_in.biHeight = m_MFCImgResol.height;
            }

            //unsigned int csc_src_color_format = HAL_PIXEL_FORMAT_YCbCr_420_P;//HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED;//omx_2_hal_pixel_format((unsigned int)OMX_SEC_COLOR_FormatNV12Tiled);
            unsigned int csc_src_color_format = HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED;//omx_2_hal_pixel_format((unsigned int)OMX_SEC_COLOR_FormatNV12Tiled);
            unsigned int csc_dst_color_format = HAL_PIXEL_FORMAT_YCbCr_420_P;//omx_2_hal_pixel_format((unsigned int)OMX_COLOR_FormatYUV420Planar);
            unsigned int cacheable = 1;
                
            csc_start_tick = CMmpUtil::GetTickCount();

            if(this->m_csc_set_format == MMP_FALSE) {
                csc_set_src_format(
                    this->m_csc_handle,  /* handle */
                    m_bih_out.biWidth,//actualWidth,            /* width */
                    m_bih_out.biHeight, //actualHeight,           /* height */
                    0,                      /* crop_left */
                    0,                      /* crop_right */
                    m_bih_out.biWidth, //actualWidth,            /* crop_width */
                    m_bih_out.biHeight, //actualHeight,           /* crop_height */
                    csc_src_color_format,   /* color_format */
                    cacheable);             /* cacheable */

                csc_set_dst_format(
                    this->m_csc_handle,  /* handle */
                    m_bih_out.biWidth,//actualWidth,            /* width */
                    m_bih_out.biHeight,//actualHeight,           /* height */
                    0,                      /* crop_left */
                    0,                      /* crop_right */
                    m_bih_out.biWidth,            /* crop_width */
                    m_bih_out.biHeight,           /* crop_height */
                    csc_dst_color_format,   /* color_format */
                    cacheable);             /* cacheable */
            
                   this->m_csc_set_format = MMP_TRUE;
            }

            if(this->m_csc_method == CSC_METHOD_SW) {
                pSrcBuf[0] = (unsigned char*)outputInfo.YVirAddr;
                pSrcBuf[1] = (unsigned char*)outputInfo.CVirAddr;
                pSrcBuf[2] = NULL;
            
                pYUVBuf[0] = (unsigned char*)pDecResult->uiDecodedBufLogAddr;
                pYUVBuf[1] = pYUVBuf[0] + m_bih_out.biWidth*m_bih_out.biHeight;
                pYUVBuf[2] = pYUVBuf[1] + m_bih_out.biWidth*m_bih_out.biHeight/4;
            } 
            else {
            pSrcBuf[0] = (unsigned char*)outputInfo.YPhyAddr;
            pSrcBuf[1] = (unsigned char*)outputInfo.CPhyAddr;
            pSrcBuf[2] = NULL;

            pYUVBuf[0] = (unsigned char*)pDecResult->uiDecodedBufPhyAddr;
            pYUVBuf[1] = pYUVBuf[0] + m_bih_out.biWidth*m_bih_out.biHeight;
            pYUVBuf[2] = pYUVBuf[1] + m_bih_out.biWidth*m_bih_out.biHeight/4;
            }

//MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Mfc::DecodeAu] ln=%d resol(%d %d) status=%d "), __LINE__, m_bih_out.biWidth, m_bih_out.biHeight,  mfcDecStatus));
            if(m_bih_out.biSizeImage <= pDecResult->uiDecodedBufMaxSize) {
    #if 1
                csc_set_src_buffer(
                    this->m_csc_handle,  /* handle */
                    pSrcBuf[0],             /* y addr */
                    pSrcBuf[1],             /* u addr or uv addr */
                    pSrcBuf[2],             /* v addr or none */
                    0);                     /* ion fd */
                
                csc_set_dst_buffer(
                    this->m_csc_handle,  /* handle */
                    pYUVBuf[0],             /* y addr */
                    pYUVBuf[1],             /* u addr or uv addr */
                    pYUVBuf[2],             /* v addr or none */
                    0);                     /* ion fd */
                csc_convert(this->m_csc_handle);
    #endif

    #if 0
    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Mfc::DecodeAu] ln=%d resol(%d %d %d) status=%d  maxsz=%d Addr(0x%08x 0x%08x)"),
                      __LINE__, m_bih_out.biWidth, m_bih_out.biHeight,  m_bih_out.biSizeImage, mfcDecStatus, 
                       pDecResult->uiDecodedBufMaxSize,
                       pDecResult->uiDecodedBufLogAddr , pDecResult->uiDecodedBufPhyAddr
                      ));

                
                pYUVBuf[0] = (unsigned char*)pDecResult->uiDecodedBufLogAddr;
                pYUVBuf[1] = pYUVBuf[0] + m_bih_out.biWidth*m_bih_out.biHeight;
                pYUVBuf[2] = pYUVBuf[1] + m_bih_out.biWidth*m_bih_out.biHeight/4;

                memcpy(pYUVBuf[0], outputInfo.YVirAddr, m_bih_out.biWidth*m_bih_out.biHeight);
                //memset(pYUVBuf[0], 128, m_bih_out.biWidth*m_bih_out.biHeight);
                memset(pYUVBuf[1], 128, m_bih_out.biWidth*m_bih_out.biHeight/4);
                memset(pYUVBuf[2], 128, m_bih_out.biWidth*m_bih_out.biHeight/4);

                

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Mfc::DecodeAu] ln=%d resol(%d %d) status=%d "), __LINE__, m_bih_out.biWidth, m_bih_out.biHeight,  mfcDecStatus));
    #endif

                pDecResult->uiDecodedSize = m_bih_out.biSizeImage; //(m_bih_out.biWidth*m_bih_out.biHeight*3)/2;
                pDecResult->bImage = MMP_TRUE;

            csc_end_tick = CMmpUtil::GetTickCount();
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Mfc::DecodeAu] ln=%d csc_dur=%d "), __LINE__, csc_end_tick - csc_start_tick));
            }

        }
        
    }
    
    
    pDecResult->uiDecodedDuration = dec_end_tick - dec_start_tick;

//MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Mfc::DecodeAu] ln=%d resol(%d %d) status=%d "), __LINE__, m_bih_out.biWidth, m_bih_out.biHeight,  mfcDecStatus));

	return mmpResult; 

}
#endif
#endif /* #if (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) */