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

#include "MmpPlayerYUV.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerYUV Member Functions

CMmpPlayerYUV::CMmpPlayerYUV(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)

,m_fp(NULL)
,m_pRendererVideo(NULL)
,m_p_framebuf(NULL)

{
    

}

CMmpPlayerYUV::~CMmpPlayerYUV()
{
    
}

MMP_RESULT CMmpPlayerYUV::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    CMmpRendererCreateProp* pRendererProp=&m_RendererProp; 

    /* create YUVFile */
    if(mmpResult == MMP_SUCCESS) {
        m_fp = fopen(this->m_create_config.filename, "rb");
        if(m_fp == NULL) {
            mmpResult = MMP_FAILURE;
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


        pRendererProp->m_iPicWidth = m_create_config.option.yuv.width;
        pRendererProp->m_iPicHeight = m_create_config.option.yuv.height;

        m_pRendererVideo = CMmpRenderer::CreateVideoObject(pRendererProp);
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* create video frame buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_framebuf = mmp_buffer_mgr::get_instance()->alloc_media_videoframe(m_create_config.option.yuv.width, m_create_config.option.yuv.height, MMP_FOURCC_IMAGE_I420);
        if(m_p_framebuf == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* Base Class Open */
    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }

    return mmpResult;
}


MMP_RESULT CMmpPlayerYUV::Close()
{
    CMmpPlayer::Close();

    if(m_pRendererVideo != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererVideo);  
        m_pRendererVideo = NULL;
    }

    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }

    if(m_p_framebuf != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_framebuf);
        m_p_framebuf = NULL;
    }
    
    return MMP_SUCCESS;
}

void CMmpPlayerYUV::Service()
{
    MMP_U32 frame_count = 0;
    MMP_S32 pic_luma_size, pic_chroma_size;
    class mmp_buffer_addr buf_addr[MMP_MEDIASAMPLE_PLANE_COUNT];
    MMP_S32 buf_size[MMP_MEDIASAMPLE_PLANE_COUNT];
    MMP_S32 i, rdsz;
   
    pic_luma_size = m_create_config.option.yuv.width * m_create_config.option.yuv.height;
    pic_chroma_size = pic_luma_size>>2;

    for(i = 0; i < MMP_MEDIASAMPLE_PLANE_COUNT; i++) {
        buf_addr[i] = this->m_p_framebuf->get_buf_addr(i);
    }
    buf_size[MMP_MEDIASAMPLE_BUF_Y] = pic_luma_size;
    buf_size[MMP_MEDIASAMPLE_BUF_U] = pic_chroma_size;
    buf_size[MMP_MEDIASAMPLE_BUF_V] = pic_chroma_size;
    
    while(m_bServiceRun == MMP_TRUE) {
    
        for(i = 0; i < MMP_MEDIASAMPLE_PLANE_COUNT; i++) {
            rdsz = fread((void*)buf_addr[i].m_vir_addr, 1, buf_size[i], m_fp);
            if(rdsz != buf_size[i]) {
                m_bServiceRun = MMP_FALSE;
                break;
            }
        }

        if(m_bServiceRun != MMP_TRUE) break;

        m_pRendererVideo->Render(this->m_p_framebuf);
            

        frame_count++;
        
        CMmpUtil::Sleep(50);
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerYUV::Service] Task Ended!!")));

}

