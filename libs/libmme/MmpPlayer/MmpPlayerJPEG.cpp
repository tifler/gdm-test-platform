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

#include "MmpPlayerJPEG.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerJPEG Member Functions

CMmpPlayerJPEG::CMmpPlayerJPEG(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)

,m_p_buf_imagestream(NULL)
,m_p_buf_imageframe(NULL)

,m_pDecoderImage(NULL)
,m_pRendererVideo(NULL)

{
    

}

CMmpPlayerJPEG::~CMmpPlayerJPEG()
{
    
}

MMP_RESULT CMmpPlayerJPEG::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    CMmpRendererCreateProp* pRendererProp=&m_RendererProp; 
    struct MmpDecoderCreateConfig DecoderCreateConfig;

    /* load image file */
    if(mmpResult == MMP_SUCCESS) {

        m_p_buf_imagestream = mmp_buffer_mgr::get_instance()->alloc_media_imagestream(this->m_create_config.filename, mmp_buffer::ION);
        if(m_p_buf_imagestream == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* create image decoder */
    if(mmpResult == MMP_SUCCESS ) {
        
        memset(&DecoderCreateConfig, 0x00, sizeof(DecoderCreateConfig));
        m_pDecoderImage = (CMmpDecoderImage*)CMmpDecoder::CreateImageObject(&DecoderCreateConfig);
        if(m_pDecoderImage != NULL) {
            m_pDecoderImage->DecodeAu(m_p_buf_imagestream, &m_p_buf_imageframe);
            if(m_p_buf_imageframe == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }
    }

    /* create video renderer */
    if(mmpResult == MMP_SUCCESS ) {
        
        pRendererProp->m_hRenderWnd = this->m_create_config.video_config.m_hRenderWnd;
        pRendererProp->m_hRenderDC = this->m_create_config.video_config.m_hRenderDC;
        pRendererProp->m_renderer_type = this->m_create_config.video_config.m_renderer_type;
        
        pRendererProp->m_bVideoEncoder = this->m_create_config.video_config.m_bVideoEncoder;
        strcpy(pRendererProp->m_VideoEncFileName, this->m_create_config.video_config.m_VideoEncFileName);
        pRendererProp->m_VideoEncoderCreateConfig = this->m_create_config.video_config.m_VideoEncoderCreateConfig;

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


        pRendererProp->m_iPicWidth = m_pDecoderImage->GetPicWidth();
        pRendererProp->m_iPicHeight = m_pDecoderImage->GetPicHeight();

        m_pRendererVideo = CMmpRenderer::CreateVideoObject(pRendererProp);
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }
    
    /* Base Class Open */
    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }

    return mmpResult;
}


MMP_RESULT CMmpPlayerJPEG::Close()
{
    CMmpPlayer::Close();

    if(m_pDecoderImage != NULL) {
        CMmpDecoder::DestroyObject(m_pDecoderImage);
        m_pDecoderImage = NULL;
    }

    if(m_pRendererVideo != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererVideo);  
        m_pRendererVideo = NULL;
    }

    if(m_p_buf_imagestream != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imagestream);
        m_p_buf_imagestream = NULL;
    }

    m_p_buf_imageframe = NULL;
    
    return MMP_SUCCESS;
}

void CMmpPlayerJPEG::Service()
{
    while(m_bServiceRun == MMP_TRUE) {
        
        if(m_p_buf_imageframe != NULL) {
            m_pRendererVideo->Render(m_p_buf_imageframe);
        }
        
        CMmpUtil::Sleep(100);
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerJPEG::Service] Task Ended!!")));
}

