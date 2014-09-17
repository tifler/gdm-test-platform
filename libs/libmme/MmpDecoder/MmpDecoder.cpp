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
	
    if( (bForceFfmpeg == MMP_TRUE) ) {
        pObj=new CMmpDecoderVideo_Ffmpeg(pCreateConfig);
    }
    else { 

#if (MMP_HWCODEC == MMP_HWCODEC_VPU)
        //if(pCreateConfig->bThumbnailMode == MMP_TRUE) {
        //
        //}
        if(CMmpDecoderVideo_Vpu::CheckSupportCodec(pCreateConfig->nFormat) == MMP_TRUE) {
            pObj=new CMmpDecoderVideo_Vpu(pCreateConfig);
        }
        else {
        
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoder::CreateVideoObject] FAIL: Not support VPU codec  fourcc=%c%c%c%c resol=%dx%d"), 
                    MMPGETFOURCC(pCreateConfig->nFormat, 0),MMPGETFOURCC(pCreateConfig->nFormat, 1),
                    MMPGETFOURCC(pCreateConfig->nFormat, 2),MMPGETFOURCC(pCreateConfig->nFormat, 3),
                    pCreateConfig->nPicWidth, pCreateConfig->nPicHeight
                ));
        }
#else
#error "ERROR: Select HW Codec "    
#endif
    
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



