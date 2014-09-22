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

#include "MmpRenderer_YUVWriter.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/////////////////////////////////////////////////////////////
//CMmpRenderer_YUVWriter Member Functions

#if (MMP_OS == MMP_OS_WIN32)
#define FILE_PATH "d:\\work\\"
#else
#define FILE_PATH "/mnt/"
#endif

MMP_U32 CMmpRenderer_YUVWriter::m_render_file_id = 0;

CMmpRenderer_YUVWriter::CMmpRenderer_YUVWriter(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_VIDEO, pRendererProp)
,m_fp(NULL)
{

}

CMmpRenderer_YUVWriter::~CMmpRenderer_YUVWriter()
{

}

MMP_RESULT CMmpRenderer_YUVWriter::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_CHAR filename[64];

    mmpResult = CMmpRenderer::Open();

    if(mmpResult == MMP_SUCCESS) {

        sprintf(filename, "%sdump_%08x_%d_%dx%d.yuv", FILE_PATH, this, CMmpRenderer_YUVWriter::m_render_file_id, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight);
        m_fp = fopen(filename, "wb");
        if(m_fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }
    
    CMmpRenderer_YUVWriter::m_render_file_id++;

    return mmpResult;
}


MMP_RESULT CMmpRenderer_YUVWriter::Close()
{
    MMPBITMAPINFOHEADER bih;

    if(m_fp != NULL) {

        bih.biSize = sizeof(MMPBITMAPINFOHEADER);
        bih.biCompression = MMPMAKEFOURCC('Y','U','V',' ');
        bih.biWidth = m_pRendererProp->m_iPicWidth;
        bih.biHeight = m_pRendererProp->m_iPicHeight;

        fwrite((void*)&bih, 1, bih.biSize, m_fp);

        fclose(m_fp);
        m_fp = NULL;
    }

    CMmpRenderer::Close();

    return MMP_SUCCESS;
}

void CMmpRenderer_YUVWriter::SetFirstRenderer() {

    MMPDEBUGMSG(1, (TEXT("[%s::%s] "), MMP_CLASS_NAME, MMP_CLASS_FUNC ));
}

void CMmpRenderer_YUVWriter::SetRotate(enum MMP_ROTATE rotate) {
    MMPDEBUGMSG(1, (TEXT("[%s::%s] "), MMP_CLASS_NAME, MMP_CLASS_FUNC ));
}

MMP_RESULT CMmpRenderer_YUVWriter::Render(class mmp_buffer_imageframe* p_buf_imageframe) {

    return MMP_FAILURE;
}

MMP_RESULT CMmpRenderer_YUVWriter::Render(class mmp_buffer_videoframe* p_buf_videoframe) {

    MMP_S32 i, pic_width, pic_height, stride;
    class mmp_buffer_addr buf_addr;
    int pic_size[MMP_MEDIASAMPLE_PLANE_COUNT];

    pic_width = p_buf_videoframe->get_pic_width();
    pic_height = p_buf_videoframe->get_pic_height();
    stride = p_buf_videoframe->get_stride_luma();

    pic_size[0] = stride*pic_height;
    pic_size[1] = (stride*pic_height)/4;
    pic_size[2] = (stride*pic_height)/4;
    
    for(i = 0; i < MMP_MEDIASAMPLE_PLANE_COUNT; i++) {
        buf_addr = p_buf_videoframe->get_buf_addr(i);
        fwrite((void*)buf_addr.m_vir_addr, 1, pic_size[i], m_fp);
    }

    CMmpRenderer::EncodeAndMux(p_buf_videoframe);

    return MMP_SUCCESS;
}

