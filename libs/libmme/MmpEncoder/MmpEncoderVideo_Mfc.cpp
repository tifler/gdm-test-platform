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

#include "MmpEncoderVideo_Mfc.hpp"
#include "../MmpComm/MmpUtil.hpp"

#if ((MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) || (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC_ANDROID44) )

extern "C"  {
#include "color_space_convertor.h"
}

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo_Mfc Member Functions

CMmpEncoderVideo_Mfc::CMmpEncoderVideo_Mfc(struct MmpEncoderCreateConfig *pCreateConfig) : CMmpEncoderVideo(pCreateConfig, MMP_FALSE), CMmpEncoderMfc(pCreateConfig)
,m_nEncodedStreamCount(0)
,m_nInputFrameCount(0)
,m_p_hwsync_mutex(NULL)
{
    
}

CMmpEncoderVideo_Mfc::~CMmpEncoderVideo_Mfc()
{

}

MMP_RESULT CMmpEncoderVideo_Mfc::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpEncoderMfc::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Mfc::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    if(this->m_CreateConfig.hw_sync_mutex_hdl!=NULL) {
        m_p_hwsync_mutex = (class mmp_oal_mutex*)this->m_CreateConfig.hw_sync_mutex_hdl;
    }
    
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoderVideo_Mfc::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderMfc::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Mfc::Close] CMmpEncoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpEncoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Mfc::Close] CMmpEncoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Mfc::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));


    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVideo_Mfc::EncodeAu(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

    SSBSIP_MFC_ERROR_CODE mfc_ret;
    SSBSIP_MFC_ENC_OUTPUT_INFO mfc_outputInfo;
    SSBSIP_MFC_ENC_INPUT_INFO mfc_inputinfo;
    SSBSIP_MFC_ENC_INPUT_INFO *p_mfc_inputinfo = &mfc_inputinfo;
    
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U32 i;
    MMP_U32 enc_start_tick, enc_end_tick;

    MMP_U8* pTemp;
    MMP_U8* pEncBuffer;
    MMP_U32 nEncBufSize, nEncBufMaxSize, nEncFlag;
    MMP_U32 buf_width, buf_height;

    enc_start_tick = CMmpUtil::GetTickCount();

    pEncResult->uiEncodedStreamSize[0] = 0;
    pEncResult->uiEncodedStreamSize[1] = 0;
    pEncResult->uiTimeStamp = pMediaSample->uiTimeStamp;
    pEncResult->uiFlag = 0;

    if(m_p_hwsync_mutex!=NULL) m_p_hwsync_mutex->lock();
    
    /*
    if(m_pAVCodecContext == NULL) {
    
        mmpResult = CMmpEncoderMfc::EncodeDSI(pMediaSample, pEncResult);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }
    */

#ifndef WIN32
    if(m_nEncodedStreamCount == 0) {

        mfc_ret = SsbSipMfcEncGetOutBuf(m_hMFCHandle, &mfc_outputInfo);
        if(mfc_ret != MFC_RET_OK)
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] GetConfig Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ")));
            //return MMP_FAILURE;
        }
        else {
            this->EncodedFrameQueue_AddFrame((MMP_U8*)mfc_outputInfo.StrmVirAddr, mfc_outputInfo.headerSize, MMP_ENCODED_FLAG_VIDEO_CONFIGDATA);

            pTemp = (MMP_U8*)mfc_outputInfo.StrmVirAddr;
            MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] ln=%d Sz=(%d %d) (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                    mfc_outputInfo.headerSize, mfc_outputInfo.dataSize,
                   (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
                   (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
                 ));
        }
    }
#endif

    memcpy(p_mfc_inputinfo, &this->m_mfc_input_info[0], sizeof(SSBSIP_MFC_ENC_INPUT_INFO) );
    
    buf_width = MMP_BYTE_ALIGN(m_bih_in.biWidth, 16);
    buf_height = MMP_BYTE_ALIGN(m_bih_in.biHeight, 16);
    
    if( (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y] != 0) 
        && (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U] != 0) ) {
    
        p_mfc_inputinfo->YPhyAddr = (void*)pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y]; 
        p_mfc_inputinfo->CPhyAddr = (void*)pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U]; 
        p_mfc_inputinfo->YVirAddr = (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y]; 
        p_mfc_inputinfo->CVirAddr = (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U]; 

        p_mfc_inputinfo->YSize = buf_width*buf_height;
        p_mfc_inputinfo->CSize = p_mfc_inputinfo->YSize/2;
        
        p_mfc_inputinfo->y_cookie = 0;
        p_mfc_inputinfo->c_cookie = 0;

    }
    else if( (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y] == 0) 
        && (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U] == 0) ) {
    
            p_mfc_inputinfo->YSize = buf_width*buf_height;
            p_mfc_inputinfo->CSize = p_mfc_inputinfo->YSize/2;
            p_mfc_inputinfo->y_cookie = 0;
            p_mfc_inputinfo->c_cookie = 0;

            
            switch(pMediaSample->pixelformat) {
                
                case MMP_PIXELFORMAT_YUV420_PLANAR:

                    if(m_MFC_Encoding_Frame_Map == NV12_TILE) {//linear to tiled
                    
                        csc_linear_to_tiled_y((unsigned char*)p_mfc_inputinfo->YVirAddr, 
                                              (unsigned char*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y], 
                                              buf_width, buf_height);
                        csc_linear_to_tiled_uv((unsigned char*)p_mfc_inputinfo->CVirAddr,
                                               (unsigned char*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U],
                                               (unsigned char*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_V],
                                               buf_width,  /* uv width */
                                               buf_height/2  /* uv height */
                                               );
                    }
                    else if(m_MFC_Encoding_Frame_Map == NV12_LINEAR) {  //linear to semi planar

                        memcpy(p_mfc_inputinfo->YVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y], p_mfc_inputinfo->YSize);
                        int ii; 
                        unsigned char *p1, *p2, *p3;
                        p1 = (unsigned char*)p_mfc_inputinfo->CVirAddr;
                        p2 = (unsigned char*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U];
                        p3 = (unsigned char*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_V];
                        for(ii = 0; ii < buf_width*buf_height/2; ii+=2) {
                            p1[ii] = p2[ii/2];
                            p1[ii+1] = p3[ii/2];
                        }
                
                    }
                    break;

                case MMP_PIXELFORMAT_SAMSUNG_NV12:
                    memcpy(p_mfc_inputinfo->YVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y], p_mfc_inputinfo->YSize);
                    memcpy(p_mfc_inputinfo->CVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U], p_mfc_inputinfo->CSize);
                    break;

                default:
                    //memcpy(p_mfc_inputinfo->YVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y], p_mfc_inputinfo->YSize);
                    //memcpy(p_mfc_inputinfo->CVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U], p_mfc_inputinfo->CSize);
                    mmpResult = MMP_FAILURE;
                    break;
            
            }

    }
    else {
        mmpResult = MMP_FAILURE;
    }
            
MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] MediaSample(Phy:0x%08x 0x%08x Vir:0x%08x 0x%08x) MfcIn(Phy: 0x%08x 0x%08x Vir: 0x%08x 0x%08x Sz:%d %d ) "),
        pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y], pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U],
        pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y], pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U],
        p_mfc_inputinfo->YPhyAddr, p_mfc_inputinfo->CPhyAddr,
        p_mfc_inputinfo->YVirAddr, p_mfc_inputinfo->CVirAddr,
        p_mfc_inputinfo->YSize, p_mfc_inputinfo->CSize
        ));

#if 1
    if(mmpResult == MMP_SUCCESS) {

        mfc_ret = SsbSipMfcEncSetInBuf(m_hMFCHandle, p_mfc_inputinfo);
        if(mfc_ret == MFC_RET_OK) {

            mfc_ret = SsbSipMfcEncExe(m_hMFCHandle);
            if(mfc_ret == MFC_RET_OK) /* Success */ {
                
                mfc_ret = SsbSipMfcEncGetOutBuf(m_hMFCHandle, &mfc_outputInfo);
                if(mfc_ret == MFC_RET_OK) /* Success */ {
                
                    pEncBuffer = (MMP_U8*)pEncResult->uiEncodedBufferLogAddr[MMP_ENCODED_BUF_STREAM];
                    nEncBufMaxSize = pEncResult->uiEncodedBufferMaxSize[MMP_ENCODED_BUF_STREAM];
                    nEncBufSize = mfc_outputInfo.dataSize;

                    pTemp = (MMP_U8*)mfc_outputInfo.StrmVirAddr;
                    MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] ln=%d Sz=%d Type=%d (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                        mfc_outputInfo.dataSize,
                        mfc_outputInfo.frameType,
                       (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
                       (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
                     ));

                    if(m_nEncodedStreamCount == 0) {

    #ifdef WIN32
                        this->EncodedFrameQueue_AddFrameWithConfig_Mpeg4((MMP_U8*)mfc_outputInfo.StrmVirAddr, nEncBufSize, (mfc_outputInfo.frameType == MFC_FRAME_TYPE_I_FRAME)?MMP_ENCODED_FLAG_VIDEO_KEYFRAME:0);
    #else
                        this->EncodedFrameQueue_AddFrame((MMP_U8*)mfc_outputInfo.StrmVirAddr, nEncBufSize, (mfc_outputInfo.frameType == MFC_FRAME_TYPE_I_FRAME)?MMP_ENCODED_FLAG_VIDEO_KEYFRAME:0);
    #endif
                        if(this->EncodedFrameQueue_IsEmpty() != MMP_TRUE) {

                            this->EncodedFrameQueue_GetFrame(pEncBuffer, nEncBufMaxSize, &nEncBufSize, &nEncFlag);

                            pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nEncBufSize;
                            pEncResult->uiFlag |= nEncFlag;

                            m_nEncodedStreamCount++;
                        }
                    }
                    else {
                    
                        memcpy(pEncBuffer, (void*)mfc_outputInfo.StrmVirAddr, nEncBufSize);
                        pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nEncBufSize;

                        if (mfc_outputInfo.frameType == MFC_FRAME_TYPE_I_FRAME) {
                            pEncResult->uiFlag |= MMP_ENCODED_FLAG_VIDEO_KEYFRAME;
                        }

                        m_nEncodedStreamCount++;
                    }
                }
                
                mmpResult = MMP_SUCCESS; 
            }
        }
    }
#endif

    enc_end_tick = CMmpUtil::GetTickCount();

    pEncResult->uiEncodedDuration = enc_end_tick - enc_start_tick;

    CMmpEncoderVideo::EncodeMonitor(pMediaSample, pEncResult);

    m_nInputFrameCount++;

    if(m_p_hwsync_mutex!=NULL) m_p_hwsync_mutex->unlock();

    return mmpResult;
}

#if (MMP_OS == MMP_OS_WIN32) 

extern "C" void csc_linear_to_tiled_y(
    unsigned char *y_dst,
    unsigned char *y_src,
    unsigned int width,
    unsigned int height) {

}

extern "C" void csc_linear_to_tiled_uv(
    unsigned char *uv_dst,
    unsigned char *u_src,
    unsigned char *v_src,
    unsigned int uv_width, 
    unsigned int uv_height) {

    unsigned char* u_dst, *v_dst;
    
    int chroma_size = (uv_width/2)*uv_height;
    
    u_dst = uv_dst;
    v_dst = u_dst + chroma_size;

    memcpy((void*)u_dst, (void*)u_src, chroma_size);
    memcpy((void*)v_dst, (void*)v_src, chroma_size);
}

#endif

#endif /*  #if (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) */