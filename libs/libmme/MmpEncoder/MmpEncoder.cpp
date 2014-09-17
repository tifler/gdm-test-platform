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

#include "MmpEncoder.hpp"

#include "MmpEncoderVideo_Ffmpeg.hpp"
#include "MmpEncoderVideo_Mfc.hpp"

#if (MMP_HWCODEC == MMP_HWCODEC_VPU)
#include "MmpEncoderVideo_Vpu.hpp"
#endif

#include "MmpUtil.hpp"

//////////////////////////////////////////////////////////////
// CMmpEncoder CreateObject/DestroyObject


CMmpEncoder* CMmpEncoder::CreateVideoObject(struct MmpEncoderCreateConfig *pCreateConfig, MMP_BOOL bForceFFMpeg) {

	CMmpEncoder* pObj=NULL;
	
    if(bForceFFMpeg == MMP_TRUE) {
        pObj=new CMmpEncoderVideo_Ffmpeg(pCreateConfig);
    }
    else {
	    switch(pCreateConfig->nFormat)
	    {
            case MMP_FOURCC_VIDEO_H263:
            case MMP_FOURCC_VIDEO_H264:
            case MMP_FOURCC_VIDEO_MPEG4:
    #if ((MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) || (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC_ANDROID44) )
                pObj=new CMmpEncoderVideo_Mfc(pCreateConfig);
    #elif (MMP_HWCODEC == MMP_HWCODEC_VPU)
                pObj=new CMmpEncoderVideo_Vpu(pCreateConfig);
    #else
                pObj=new CMmpEncoderVideo_Ffmpeg(pCreateConfig);
    #endif
                break;
	    }
    }
    
    
	if(pObj==NULL) {
        return (CMmpEncoder*)NULL;
	}

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpEncoder*)NULL;
    }

    return pObj;
}

CMmpEncoder* CMmpEncoder::CreateVideoObject(MMP_U32 mmp_video_fourcc,
                                      MMP_U32 pic_width, MMP_U32 pic_height,
                                      MMP_U32 framerate, MMP_U32 idr_period, MMP_U32 bitrate,
                                      MMP_BOOL bForceFFMpeg) {

    CMmpEncoder* pVideoEncoder = NULL;
    struct MmpEncoderCreateConfig MmpEncoderCreateConfig;
    struct MmpEncoderCreateConfig *pMmpEncoderCreateConfig = &MmpEncoderCreateConfig;
    
    memset(pMmpEncoderCreateConfig, 0x00,  sizeof(struct MmpEncoderCreateConfig) );

    /* basic info */
    pMmpEncoderCreateConfig->nFormat = mmp_video_fourcc;
    pMmpEncoderCreateConfig->nStreamType = 0;
    
    /* Inport info */
    pMmpEncoderCreateConfig->nPicWidth = pic_width;
    pMmpEncoderCreateConfig->nPicHeight = pic_height;
    pMmpEncoderCreateConfig->nFrameRate = framerate;

    /* Outport info */
    pMmpEncoderCreateConfig->nIDRPeriod = idr_period;
    pMmpEncoderCreateConfig->nBitRate = bitrate;//p_port_def->format.video.nBitrate;

    /* Set H264 Info */
    switch(mmp_video_fourcc) {
        case MMP_FOURCC_VIDEO_H264:
#if 1
                pMmpEncoderCreateConfig->enc_info.h264.nSliceHeaderSpacing = 0;
                pMmpEncoderCreateConfig->enc_info.h264.nPFrames = 20;
                pMmpEncoderCreateConfig->enc_info.h264.nBFrames = 0;
                pMmpEncoderCreateConfig->enc_info.h264.bUseHadamard = MMP_TRUE;
                pMmpEncoderCreateConfig->enc_info.h264.nRefFrames = 1;
	            pMmpEncoderCreateConfig->enc_info.h264.nRefIdx10ActiveMinus1 = 0;
	            pMmpEncoderCreateConfig->enc_info.h264.nRefIdx11ActiveMinus1 = 0;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableUEP = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableFMO = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableASO = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableRS = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.eProfile = MMP_VIDEO_AVCProfileMain;;
	            pMmpEncoderCreateConfig->enc_info.h264.eLevel = MMP_VIDEO_AVCLevel4;
                pMmpEncoderCreateConfig->enc_info.h264.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
	            pMmpEncoderCreateConfig->enc_info.h264.bFrameMBsOnly = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bMBAFF = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEntropyCodingCABAC = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bWeightedPPrediction = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.nWeightedBipredicitonMode = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bconstIpred  = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bDirect8x8Inference = MMP_FALSE;
	            pMmpEncoderCreateConfig->enc_info.h264.bDirectSpatialTemporal = MMP_FALSE;
	            pMmpEncoderCreateConfig->enc_info.h264.nCabacInitIdc = 0;
	            pMmpEncoderCreateConfig->enc_info.h264.eLoopFilterMode = MMP_VIDEO_AVCLoopFilterEnable;
#endif
               break;

        case MMP_FOURCC_VIDEO_H263: 
#if 1
                //pMmpEncoderCreateConfig->enc_info.h263.nSize = this->m_videotype.h263.nSize;
                //pMmpEncoderCreateConfig->enc_info.h263.nVersion= this->m_videotype.h263.nVersion;
                pMmpEncoderCreateConfig->enc_info.h263.nPortIndex= 1;
                pMmpEncoderCreateConfig->enc_info.h263.nPFrames= 20;
                pMmpEncoderCreateConfig->enc_info.h263.nBFrames= 0;
                pMmpEncoderCreateConfig->enc_info.h263.eProfile= MMP_VIDEO_H263ProfileHighCompression;
	            pMmpEncoderCreateConfig->enc_info.h263.eLevel= MMP_VIDEO_H263Level70;
                pMmpEncoderCreateConfig->enc_info.h263.bPLUSPTYPEAllowed= MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h263.nAllowedPictureTypes= (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
                pMmpEncoderCreateConfig->enc_info.h263.bForceRoundingTypeToZero= MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h263.nPictureHeaderRepetition= 0;
                pMmpEncoderCreateConfig->enc_info.h263.nGOBHeaderInterval= 0;
#endif
               break;

       case MMP_FOURCC_VIDEO_MPEG4: 
#if 1
                pMmpEncoderCreateConfig->enc_info.mpeg4.nPortIndex = 1;//this->m_videotype.mpeg4.nPortIndex;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nSliceHeaderSpacing = 0;//this->m_videotype.mpeg4.nSliceHeaderSpacing;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bSVH = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bGov = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nPFrames = 1;//this->m_videotype.mpeg4.nPFrames;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nBFrames = 0;//this->m_videotype.mpeg4.nBFrames;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nIDCVLCThreshold = 0;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bACPred = MMP_TRUE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nMaxPacketSize = 256;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nTimeIncRes = 1000;
                pMmpEncoderCreateConfig->enc_info.mpeg4.eProfile = MMP_VIDEO_MPEG4ProfileCore;
                pMmpEncoderCreateConfig->enc_info.mpeg4.eLevel = MMP_VIDEO_MPEG4Level2;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nHeaderExtension = 0;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bReversibleVLC = MMP_FALSE;
#endif
                break;
    }

    pVideoEncoder = CMmpEncoder::CreateVideoObject(pMmpEncoderCreateConfig, bForceFFMpeg);
    
    
    return pVideoEncoder;
}


CMmpEncoder* CMmpEncoder::CreateVideoObject(MMP_U32 mmp_video_fourcc,
                                          struct MmpEncoderProperty_AnapassVPU prop,
                                          MMP_BOOL bForceFFMpeg) {

    CMmpEncoder* pVideoEncoder = NULL;
    struct MmpEncoderCreateConfig MmpEncoderCreateConfig;
    struct MmpEncoderCreateConfig *pMmpEncoderCreateConfig = &MmpEncoderCreateConfig;
    
    memset(pMmpEncoderCreateConfig, 0x00,  sizeof(struct MmpEncoderCreateConfig) );

    /* basic info */
    pMmpEncoderCreateConfig->nFormat = mmp_video_fourcc;
    pMmpEncoderCreateConfig->nStreamType = 0;
    
    /* Inport info */
    pMmpEncoderCreateConfig->nPicWidth = prop.PICTURE_WIDTH;
    pMmpEncoderCreateConfig->nPicHeight = prop.PICTURE_HEIGHT;
    pMmpEncoderCreateConfig->nFrameRate = prop.FRAME_RATE;

    /* Outport info */
    pMmpEncoderCreateConfig->nIDRPeriod = prop.GOP_PIC_NUMBER;
    pMmpEncoderCreateConfig->nBitRate = prop.BIT_RATE_KBPS*1000;

    /* Set H264 Info */
    switch(mmp_video_fourcc) {
        case MMP_FOURCC_VIDEO_H264:
#if 1
                pMmpEncoderCreateConfig->enc_info.h264.nSliceHeaderSpacing = 0;
                pMmpEncoderCreateConfig->enc_info.h264.nPFrames = 20;
                pMmpEncoderCreateConfig->enc_info.h264.nBFrames = 0;
                pMmpEncoderCreateConfig->enc_info.h264.bUseHadamard = MMP_TRUE;
                pMmpEncoderCreateConfig->enc_info.h264.nRefFrames = 1;
	            pMmpEncoderCreateConfig->enc_info.h264.nRefIdx10ActiveMinus1 = 0;
	            pMmpEncoderCreateConfig->enc_info.h264.nRefIdx11ActiveMinus1 = 0;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableUEP = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableFMO = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableASO = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableRS = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.eProfile = MMP_VIDEO_AVCProfileMain;;
	            pMmpEncoderCreateConfig->enc_info.h264.eLevel = MMP_VIDEO_AVCLevel4;
                pMmpEncoderCreateConfig->enc_info.h264.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
	            pMmpEncoderCreateConfig->enc_info.h264.bFrameMBsOnly = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bMBAFF = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEntropyCodingCABAC = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bWeightedPPrediction = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.nWeightedBipredicitonMode = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bconstIpred  = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bDirect8x8Inference = MMP_FALSE;
	            pMmpEncoderCreateConfig->enc_info.h264.bDirectSpatialTemporal = MMP_FALSE;
	            pMmpEncoderCreateConfig->enc_info.h264.nCabacInitIdc = 0;
	            pMmpEncoderCreateConfig->enc_info.h264.eLoopFilterMode = MMP_VIDEO_AVCLoopFilterEnable;
#endif
               break;

        case MMP_FOURCC_VIDEO_H263: 
#if 1
                //pMmpEncoderCreateConfig->enc_info.h263.nSize = this->m_videotype.h263.nSize;
                //pMmpEncoderCreateConfig->enc_info.h263.nVersion= this->m_videotype.h263.nVersion;
                pMmpEncoderCreateConfig->enc_info.h263.nPortIndex= 1;
                pMmpEncoderCreateConfig->enc_info.h263.nPFrames= 20;
                pMmpEncoderCreateConfig->enc_info.h263.nBFrames= 0;
                pMmpEncoderCreateConfig->enc_info.h263.eProfile= MMP_VIDEO_H263ProfileHighCompression;
	            pMmpEncoderCreateConfig->enc_info.h263.eLevel= MMP_VIDEO_H263Level70;
                pMmpEncoderCreateConfig->enc_info.h263.bPLUSPTYPEAllowed= MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h263.nAllowedPictureTypes= (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
                pMmpEncoderCreateConfig->enc_info.h263.bForceRoundingTypeToZero= MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h263.nPictureHeaderRepetition= 0;
                pMmpEncoderCreateConfig->enc_info.h263.nGOBHeaderInterval= 0;
#endif
               break;

       case MMP_FOURCC_VIDEO_MPEG4: 
#if 1
                pMmpEncoderCreateConfig->enc_info.mpeg4.nPortIndex = 1;//this->m_videotype.mpeg4.nPortIndex;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nSliceHeaderSpacing = 0;//this->m_videotype.mpeg4.nSliceHeaderSpacing;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bSVH = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bGov = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nPFrames = 1;//this->m_videotype.mpeg4.nPFrames;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nBFrames = 0;//this->m_videotype.mpeg4.nBFrames;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nIDCVLCThreshold = 0;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bACPred = MMP_TRUE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nMaxPacketSize = 256;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nTimeIncRes = 1000;
                pMmpEncoderCreateConfig->enc_info.mpeg4.eProfile = MMP_VIDEO_MPEG4ProfileCore;
                pMmpEncoderCreateConfig->enc_info.mpeg4.eLevel = MMP_VIDEO_MPEG4Level2;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nHeaderExtension = 0;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bReversibleVLC = MMP_FALSE;
#endif
                break;
    }

    pVideoEncoder = CMmpEncoder::CreateVideoObject(pMmpEncoderCreateConfig, bForceFFMpeg);
    
    
    return pVideoEncoder;
}



MMP_RESULT CMmpEncoder::DestroyObject(CMmpEncoder* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoder::CreateVideoDefaultConfig(MMP_U32 mmp_video_fourcc,
                                          MMP_U32 pic_width, MMP_U32 pic_height,
                                          MMP_U32 framerate, MMP_U32 idr_period, MMP_U32 bitrate,
                                          MMP_OUT struct MmpEncoderCreateConfig *pMmpEncoderCreateConfig
                                          ) {
    MMP_RESULT mmpResult = MMP_SUCCESS;

    memset(pMmpEncoderCreateConfig, 0x00,  sizeof(struct MmpEncoderCreateConfig) );

    /* basic info */
    pMmpEncoderCreateConfig->nFormat = mmp_video_fourcc;
    pMmpEncoderCreateConfig->nStreamType = 0;
    
    /* Inport info */
    pMmpEncoderCreateConfig->nPicWidth = pic_width;
    pMmpEncoderCreateConfig->nPicHeight = pic_height;
    pMmpEncoderCreateConfig->nFrameRate = framerate;

    /* Outport info */
    pMmpEncoderCreateConfig->nIDRPeriod = idr_period;
    pMmpEncoderCreateConfig->nBitRate = bitrate;//p_port_def->format.video.nBitrate;

    /* Set H264 Info */
    switch(mmp_video_fourcc) {
        case MMP_FOURCC_VIDEO_H264:
#if 1
                pMmpEncoderCreateConfig->enc_info.h264.nSliceHeaderSpacing = 0;
                pMmpEncoderCreateConfig->enc_info.h264.nPFrames = 20;
                pMmpEncoderCreateConfig->enc_info.h264.nBFrames = 0;
                pMmpEncoderCreateConfig->enc_info.h264.bUseHadamard = MMP_TRUE;
                pMmpEncoderCreateConfig->enc_info.h264.nRefFrames = 1;
	            pMmpEncoderCreateConfig->enc_info.h264.nRefIdx10ActiveMinus1 = 0;
	            pMmpEncoderCreateConfig->enc_info.h264.nRefIdx11ActiveMinus1 = 0;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableUEP = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableFMO = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableASO = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEnableRS = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.eProfile = MMP_VIDEO_AVCProfileMain;;
	            pMmpEncoderCreateConfig->enc_info.h264.eLevel = MMP_VIDEO_AVCLevel4;
                pMmpEncoderCreateConfig->enc_info.h264.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
	            pMmpEncoderCreateConfig->enc_info.h264.bFrameMBsOnly = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bMBAFF = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bEntropyCodingCABAC = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bWeightedPPrediction = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.nWeightedBipredicitonMode = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bconstIpred  = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h264.bDirect8x8Inference = MMP_FALSE;
	            pMmpEncoderCreateConfig->enc_info.h264.bDirectSpatialTemporal = MMP_FALSE;
	            pMmpEncoderCreateConfig->enc_info.h264.nCabacInitIdc = 0;
	            pMmpEncoderCreateConfig->enc_info.h264.eLoopFilterMode = MMP_VIDEO_AVCLoopFilterEnable;
#endif
               break;

        case MMP_FOURCC_VIDEO_H263: 
#if 1
                //pMmpEncoderCreateConfig->enc_info.h263.nSize = this->m_videotype.h263.nSize;
                //pMmpEncoderCreateConfig->enc_info.h263.nVersion= this->m_videotype.h263.nVersion;
                pMmpEncoderCreateConfig->enc_info.h263.nPortIndex= 1;
                pMmpEncoderCreateConfig->enc_info.h263.nPFrames= 20;
                pMmpEncoderCreateConfig->enc_info.h263.nBFrames= 0;
                pMmpEncoderCreateConfig->enc_info.h263.eProfile= MMP_VIDEO_H263ProfileHighCompression;
	            pMmpEncoderCreateConfig->enc_info.h263.eLevel= MMP_VIDEO_H263Level70;
                pMmpEncoderCreateConfig->enc_info.h263.bPLUSPTYPEAllowed= MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h263.nAllowedPictureTypes= (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
                pMmpEncoderCreateConfig->enc_info.h263.bForceRoundingTypeToZero= MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.h263.nPictureHeaderRepetition= 0;
                pMmpEncoderCreateConfig->enc_info.h263.nGOBHeaderInterval= 0;
#endif
               break;

       case MMP_FOURCC_VIDEO_MPEG4: 
#if 1
                pMmpEncoderCreateConfig->enc_info.mpeg4.nPortIndex = 1;//this->m_videotype.mpeg4.nPortIndex;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nSliceHeaderSpacing = 0;//this->m_videotype.mpeg4.nSliceHeaderSpacing;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bSVH = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bGov = MMP_FALSE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nPFrames = 1;//this->m_videotype.mpeg4.nPFrames;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nBFrames = 0;//this->m_videotype.mpeg4.nBFrames;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nIDCVLCThreshold = 0;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bACPred = MMP_TRUE;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nMaxPacketSize = 256;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nTimeIncRes = 1000;
                pMmpEncoderCreateConfig->enc_info.mpeg4.eProfile = MMP_VIDEO_MPEG4ProfileCore;
                pMmpEncoderCreateConfig->enc_info.mpeg4.eLevel = MMP_VIDEO_MPEG4Level2;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);;
                pMmpEncoderCreateConfig->enc_info.mpeg4.nHeaderExtension = 0;
                pMmpEncoderCreateConfig->enc_info.mpeg4.bReversibleVLC = MMP_FALSE;
#endif
                break;

       default :
           mmpResult = MMP_FAILURE;
    }

    
    return mmpResult;
}

/////////////////////////////////////////////////////////////
//CMmpEncoder Member Functions

CMmpEncoder::CMmpEncoder(CMmpMediaInfo* pMediaInfo) :
m_nFormat(0)
,m_nStreamType(0)
,m_bConfigDSI(MMP_FALSE)
{
	if(pMediaInfo != NULL) {
		m_MediaInfo = *pMediaInfo;
	}
	else {
		
	}

	m_pMediaInfo=&m_MediaInfo;
    m_nClassStartTick = CMmpUtil::GetTickCount();
}

CMmpEncoder::CMmpEncoder(MMP_U32 nFormat, MMP_U32 nStreamType) :
m_nFormat(nFormat)
,m_nStreamType(nStreamType)
,m_bConfigDSI(MMP_FALSE)
{
	m_pMediaInfo=&m_MediaInfo;

    m_nClassStartTick = CMmpUtil::GetTickCount();
}

CMmpEncoder::~CMmpEncoder()
{
    
}

MMP_RESULT CMmpEncoder::Open()
{
    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoder::Close()
{
    return MMP_SUCCESS;
}



