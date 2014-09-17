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


#include "MmpPlayer.hpp"
#include "MmpUtil.hpp"
#include "MmpPlayerVideoEx1.hpp"
#include "MmpPlayerAudio.hpp"
#include "MmpPlayerPCM.hpp"
#include "MmpPlayerAVEx1.hpp"
#include "MmpPlayerAVEx2.hpp"
#include "MmpPlayerAVEx3.hpp"
#include "MmpPlayerTONE.hpp"
#include "MmpPlayerYUV.hpp"

#if (MMP_OS == MMP_OS_LINUX_ANDROID)
#include "MmpPlayerStagefright.hpp"
#endif

//////////////////////////////////////////////////////////////
// CMmpPlayer CreateObject/DestroyObject

CMmpPlayer* CMmpPlayer::CreateObject(MMP_U32 playerID, CMmpPlayerCreateProp* pPlayerProp)
{
    CMmpPlayer* pObj = NULL;

    //pObj=new CMmpPlayerEx1(pPlayerProp);

    MMP_CHAR szext[16];

    CMmpUtil::SplitExtC(pPlayerProp->filename, szext);
    CMmpUtil::MakeLowerC(szext);

    if(strcmp(szext, "pcm") == 0) {
        pObj=new CMmpPlayerPCM(pPlayerProp);
    }
    else {

        switch(playerID)
        {
            case MMP_PLAYER_DEFAULT:
            case MMP_PLAYER_AUDIO_VIDEO:
                //pObj=new CMmpPlayerAVEx2(pPlayerProp);
                pObj=new CMmpPlayerAVEx3(pPlayerProp);
                break;

            case MMP_PLAYER_VIDEO_ONLY:
                pObj=new CMmpPlayerVideoEx1(pPlayerProp);
                break;

            case MMP_PLAYER_AUDIO_ONLY:
                pObj=new CMmpPlayerAudio(pPlayerProp);
                break;

            case MMP_PLAYER_TONEPLAYER:
                pObj=new CMmpPlayerTONE(pPlayerProp);
                break;

            case MMP_PLAYER_YUVPLAYER:
                pObj=new CMmpPlayerYUV(pPlayerProp);
                break;

    #if (MMP_OS == MMP_OS_LINUX_ANDROID)
            case MMP_PLAYER_STAGEFRIGHT:
                pObj=new android::CMmpPlayerStagefright(pPlayerProp);
                break;
    #endif
        }

    }

    if(pObj==NULL)
        return (CMmpPlayer*)NULL;

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpPlayer*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpPlayer::DestroyObject(CMmpPlayer* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////////////
//CMmpPlayer Member Functions

CMmpPlayer::CMmpPlayer(CMmpPlayerCreateProp* pPlayerProp) :

m_create_config(*pPlayerProp)

#if (MMPPLAYER_DUMP_PCM == 1)
,m_fp_dump_pcm(NULL)
#endif
{
    

}

CMmpPlayer::~CMmpPlayer()
{
    
}

MMP_RESULT CMmpPlayer::Open()
{

    CMmpPlayerService::Open();
    
#if (MMPPLAYER_DUMP_PCM == 1)
    m_fp_dump_pcm = fopen(MMPPLAYER_DUMP_PCM_FILENAME, "wb");
#endif

    return MMP_SUCCESS;
}


MMP_RESULT CMmpPlayer::Close()
{
    CMmpPlayerService::Close();

    this->PlayStop();
    
#if (MMPPLAYER_DUMP_PCM == 1)
    if(m_fp_dump_pcm != NULL) {
        fclose(m_fp_dump_pcm);
        m_fp_dump_pcm = NULL;
    }
#endif
    
    return MMP_SUCCESS;
}


CMmpDemuxer* CMmpPlayer::CreateDemuxer(void) {

    struct MmpDemuxerCreateConfig demuxer_create_config;
    CMmpDemuxer* pDemuxer = NULL;

    strcpy((char*)demuxer_create_config.filename, this->m_create_config.filename);
    pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);

    return pDemuxer;
}

CMmpDecoderAudio* CMmpPlayer::CreateDecoderAudio(CMmpDemuxer* pDemuxer) {
    
    MmpDecoderCreateConfig decoder_create_config;
    CMmpDecoderAudio* pDecoderAudio = NULL;

    memset(&decoder_create_config, 0x00, sizeof(decoder_create_config));

    decoder_create_config.nFormat = pDemuxer->GetAudioFormat();
    decoder_create_config.nStreamType = 0;
    decoder_create_config.pStream = NULL;
    decoder_create_config.nStreamSize = 0;
    pDecoderAudio = (CMmpDecoderAudio*)CMmpDecoder::CreateAudioObject(&decoder_create_config);
    
    return pDecoderAudio;
}

CMmpDecoderVideo* CMmpPlayer::CreateDecoderVideo(CMmpDemuxer* pDemuxer, MMP_BOOL bFfmpegUse) {
    
    MmpDecoderCreateConfig decoder_create_config;
    CMmpDecoderVideo* pDecoderVideo = NULL;

    memset(&decoder_create_config, 0x00, sizeof(decoder_create_config));

    decoder_create_config.nFormat = pDemuxer->GetVideoFormat();
    decoder_create_config.nPicWidth = pDemuxer->GetVideoPicWidth();
    decoder_create_config.nPicHeight = pDemuxer->GetVideoPicHeight();

    pDecoderVideo = (CMmpDecoderVideo*)CMmpDecoder::CreateVideoObject(&decoder_create_config, bFfmpegUse);

    return pDecoderVideo;
}

CMmpDecoder* CMmpPlayer::CreateDecoder(MMP_U32 mediatype, CMmpDemuxer* pDemuxer) {

    CMmpDecoder* pDecoder = NULL;

    switch(mediatype) {
        
        case MMP_MEDIATYPE_AUDIO:
            pDecoder = (CMmpDecoder*)this->CreateDecoderAudio(pDemuxer);
            break;

        case MMP_MEDIATYPE_VIDEO:
            pDecoder = (CMmpDecoder*)this->CreateDecoderVideo(pDemuxer);
            break;
    
    }
    
    return pDecoder;
}

CMmpRenderer* CMmpPlayer::CreateRendererAudio(CMmpDemuxer* pDemuxer, CMmpDecoderAudio* pDecoderAudio) {

    CMmpRenderer* pRenderer = NULL;
    MMPWAVEFORMATEX wf;


#if 0
	wf.wFormatTag = MMP_WAVE_FORMAT_PCM;
    wf.nChannels = pDecoderAudio->GetWF_Out().nChannels;
    wf.nSamplesPerSec = pDecoderAudio->GetWF_Out().nSamplesPerSec;
	wf.wBitsPerSample = pDecoderAudio->GetWF_Out().wBitsPerSample;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nChannels*(wf.wBitsPerSample/8);

#else

    wf.wFormatTag = MMP_WAVE_FORMAT_PCM;
    wf.nChannels = (short)pDemuxer->GetAudioChannel();
    wf.nSamplesPerSec = pDemuxer->GetAudioSamplingRate();
	wf.wBitsPerSample = (unsigned short)pDemuxer->GetAudioBitsPerSample();
	wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nChannels*(wf.wBitsPerSample/8);


#endif

	pRenderer = CMmpRenderer::CreateAudioObject(&wf);

    return pRenderer;
}

CMmpRenderer* CMmpPlayer::CreateRendererAudio(CMmpDecoderAudio* pDecoderAudio) {

    CMmpRenderer* pRenderer = NULL;
    MMPWAVEFORMATEX wf;


	wf.wFormatTag = MMP_WAVE_FORMAT_PCM;
    wf.nChannels = pDecoderAudio->GetWF_Out().nChannels;
    wf.nSamplesPerSec = pDecoderAudio->GetWF_Out().nSamplesPerSec;
	wf.wBitsPerSample = pDecoderAudio->GetWF_Out().wBitsPerSample;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nChannels*(wf.wBitsPerSample/8);

	pRenderer = CMmpRenderer::CreateAudioObject(&wf);

    return pRenderer;
}


CMmpRenderer* CMmpPlayer::CreateRendererVideo(CMmpDemuxer* pDemuxer) {

    CMmpRenderer* pRendererVideo = NULL;

    CMmpRendererCreateProp RendererProp;
    CMmpRendererCreateProp* pRendererProp=&RendererProp; 

    pRendererProp->m_hRenderWnd = this->m_create_config.video_config.m_hRenderWnd;
    pRendererProp->m_hRenderDC = this->m_create_config.video_config.m_hRenderDC;

    pRendererProp->m_iBoardWidth = 1920;
    pRendererProp->m_iBoardHeight = 1080;

#if (MMP_OS == MMP_OS_WIN32)
    HWND hWnd = (HWND)pRendererProp->m_hRenderWnd;
    RECT rect;

	::GetWindowRect(hWnd, &rect);

    pRendererProp->m_iScreenPosX = rect.left;
    pRendererProp->m_iScreenPosY = rect.top;
    pRendererProp->m_iScreenWidth = (rect.right-rect.left)/2;
    pRendererProp->m_iScreenHeight = (rect.bottom-rect.top)/2;
#endif

    pRendererProp->m_iPicWidth = pDemuxer->GetVideoPicWidth();
    pRendererProp->m_iPicHeight = pDemuxer->GetVideoPicHeight();

    pRendererProp->m_renderPixelFormat = MMP_PIXELFORMAT_YUV420_PLANAR;

    pRendererVideo = CMmpRenderer::CreateVideoObject(pRendererProp);
      
    return pRendererVideo;
}

MMP_RESULT CMmpPlayer::DecodeMediaExtraData(MMP_U32 mediatype, 
                                    CMmpDemuxer* pDemuxer, CMmpDecoder* pDecoder)  {

    CMmpMediaSample *pMediaSample = &m_MediaSampleObj;
    CMmpMediaSampleDecodeResult* pDecResult = &m_DecResultObj;
    MMP_U32 stream_buf_size;
    MMP_U8* stream_buf = NULL; 
    MMP_U32 stream_buf_maxsize = 1024*1024;

    MMP_RESULT mmpResult = MMP_FAILURE;

    stream_buf = (MMP_U8*)malloc(stream_buf_maxsize);

    if(stream_buf != NULL) {
        stream_buf_size = 0;
        pDemuxer->GetMediaExtraData(mediatype, stream_buf, stream_buf_maxsize, &stream_buf_size);
        if(stream_buf_size > 0) {
        
            pMediaSample->pAu = stream_buf;
            pMediaSample->uiAuSize = stream_buf_size;
            pMediaSample->uiSampleNumber = 0;
            pMediaSample->uiTimeStamp = 0;
            pMediaSample->uiFlag = MMP_MEDIASAMPMLE_FLAG_CONFIGDATA;

            memset((void*)pDecResult, 0x00, sizeof(CMmpMediaSampleDecodeResult));
                    
            mmpResult = pDecoder->DecodeAu(pMediaSample, pDecResult);
        }
    }

    if(stream_buf) free(stream_buf);

    return mmpResult;
}


#if (MMPPLAYER_DUMP_PCM == 1)
void CMmpPlayer::DumpPCM_Write(MMP_U8* pcmdata, MMP_S32 pcmbytesize) {

    if(m_fp_dump_pcm != NULL) {
        fwrite(pcmdata, 1, pcmbytesize, m_fp_dump_pcm);
    }
}
#endif
