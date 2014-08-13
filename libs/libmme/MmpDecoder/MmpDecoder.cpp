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


#include "MmpDecoder.hpp"
//#include "MmpDecoderVideo_Dvc.hpp"
//#include "MmpDecoderVideo_Mme.hpp"

//#include "MmpDecoderAudio_Mme.hpp"
#if (MMP_OS == MMP_OS_LINUX_ANDROID)
#include "MmpDecoderAudio_AAC_PV.hpp"
#include "MmpDecoderVideo_Mpeg4_PV.hpp"
#include "MmpDecoderVideo_Mfc.hpp"
#endif

#include "MmpDecoderAudio_Ffmpeg.hpp"
#include "MmpDecoderAudio_Dummy.hpp"
#include "MmpDecoderVideo_Ffmpeg.hpp"
#include "MmpDecoderVideo_Dummy.hpp"

#if (MMP_HWCODEC == MMP_HWCODEC_VPU)
#include "MmpDecoderVideo_Vpu.hpp"
#endif

#include "MmpUtil.hpp"

//////////////////////////////////////////////////////////////
// CMmpDecoder CreateObject/DestroyObject

CMmpDecoder* CMmpDecoder::CreateAudioObject(struct MmpDecoderCreateConfig *pCreateConfig /*MMP_U32 nFormat, MMP_U32 nStreamType, MMP_U8* pStream, MMP_U32 nStreamSize*/) {

    CMmpDecoder* pObj=NULL;
   
	//MMP_MEDIATYPE mt=pMediaInfo->GetMediaType();

	switch(pCreateConfig->nFormat)
	{
		case MMP_WAVE_FORMAT_AAC:
        case MMP_WAVE_FORMAT_AC3:
        case MMP_WAVE_FORMAT_MPEGLAYER3:
        case MMP_WAVE_FORMAT_MPEGLAYER2:
        case MMP_WAVE_FORMAT_MPEGLAYER1:
        case MMP_WAVE_FORMAT_WMA2:
		case MMP_WAVE_FORMAT_FLAC:
        case MMP_WAVE_FORMAT_ADPCM_MS:
		case MMP_WAVE_FORMAT_FFMPEG:
			pObj=new CMmpDecoderAudio_Ffmpeg(pCreateConfig);
			break;
	}
    
    
	if(pObj==NULL) {
        return (CMmpDecoder*)NULL;
	}

    if( pObj->Open(pCreateConfig->pStream, pCreateConfig->nStreamSize)!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpDecoder*)NULL;
    }

    return pObj;
}

CMmpDecoder* CMmpDecoder::CreateVideoObject(struct MmpDecoderCreateConfig *pCreateConfig, MMP_BOOL bForceFfmpeg) {

	CMmpDecoder* pObj=NULL;
	
    if(bForceFfmpeg == MMP_TRUE) {
        pObj=new CMmpDecoderVideo_Ffmpeg(pCreateConfig);
    }
    else { 

	    switch(pCreateConfig->nFormat)
	    {
            case MMP_FOURCC_VIDEO_H263:
            case MMP_FOURCC_VIDEO_H264:
            case MMP_FOURCC_VIDEO_MPEG4:
            case MMP_FOURCC_VIDEO_VC1:
            case MMP_FOURCC_VIDEO_WMV3:
            case MMP_FOURCC_VIDEO_MSMPEG4V3: /* Divx3 */
            case MMP_FOURCC_VIDEO_RV30: 
            case MMP_FOURCC_VIDEO_RV40: 
            case MMP_FOURCC_VIDEO_VP80:
                if(pCreateConfig->bThumbnailMode == MMP_TRUE) {
                    pObj=new CMmpDecoderVideo_Ffmpeg(pCreateConfig);
                }
                else {
    #if ((MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) || (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC_ANDROID44) )
                    pObj=new CMmpDecoderVideo_Mfc(pCreateConfig);
                    
    #elif (MMP_HWCODEC == MMP_HWCODEC_VPU)
                    pObj=new CMmpDecoderVideo_Vpu(pCreateConfig);
    #else
                    pObj=new CMmpDecoderVideo_Ffmpeg(pCreateConfig);
    #endif
                }
                break;
            
            case MMP_FOURCC_VIDEO_MJPEG:
            case MMP_FOURCC_VIDEO_THEORA:
            case MMP_FOURCC_VIDEO_MPEG2:
            case MMP_FOURCC_VIDEO_FFMPEG:
                //pObj=new CMmpDecoderVideo_Dummy(pCreateConfig);
                //pObj=new CMmpDecoderVideo_Mfc(pCreateConfig);
                pObj=new CMmpDecoderVideo_Ffmpeg(pCreateConfig);
		        break;

            //case MMP_FOURCC_VIDEO_VP6:
            //case MMP_FOURCC_VIDEO_VP6F:
            //case MMP_FOURCC_VIDEO_VP6A:
            //    pObj=new CMmpDecoderVideo_Ffmpeg(nFormat, nStreamType);
                //pObj=new CMmpDecoderVideo_Mfc(nFormat, nStreamType);
            //    break;

    		
            //case MMP_FOURCC_VIDEO_RV30:
            //case MMP_FOURCC_VIDEO_RV40:
              //  pObj=new CMmpDecoderVideo_Ffmpeg(nFormat, nStreamType);
                //pObj=new CMmpDecoderVideo_Dummy(nFormat, nStreamType);
                //break;
           
            //case MMP_FOURCC_VIDEO_SVQ1:
            //case MMP_FOURCC_VIDEO_SVQ3:
              //  pObj=new CMmpDecoderVideo_Ffmpeg(nFormat, nStreamType);
                //pObj=new CMmpDecoderVideo_Dummy(nFormat, nStreamType);
                //break;

            
            
            //case MMP_FOURCC_VIDEO_MSMPEG4V1:
            //case MMP_FOURCC_VIDEO_MSMPEG4V2:
            //case MMP_FOURCC_VIDEO_MSMPEG4V3:
              //  pObj=new CMmpDecoderVideo_Ffmpeg(nFormat, nStreamType);
                //pObj=new CMmpDecoderVideo_Dummy(nFormat, nStreamType);
                //break;
	    }
    
    }
    
	if(pObj==NULL) {
        return (CMmpDecoder*)NULL;
	}

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpDecoder*)NULL;
    }

    return pObj;
}



MMP_RESULT CMmpDecoder::DestroyObject(CMmpDecoder* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////////////
//CMmpDecoder Member Functions

CMmpDecoder::CMmpDecoder(CMmpMediaInfo* pMediaInfo) :
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

CMmpDecoder::CMmpDecoder(MMP_U32 nFormat, MMP_U32 nStreamType) :
m_nFormat(nFormat)
,m_nStreamType(nStreamType)
,m_bConfigDSI(MMP_FALSE)
{
	m_pMediaInfo=&m_MediaInfo;

    m_nClassStartTick = CMmpUtil::GetTickCount();
}

CMmpDecoder::~CMmpDecoder()
{
    
}

MMP_RESULT CMmpDecoder::Open()
{
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoder::Close()
{
    return MMP_SUCCESS;
}



