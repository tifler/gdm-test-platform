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

#include "MmpRenderer_Dummy.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpRenderer_Dummy Member Functions


CMmpRenderer_Dummy::CMmpRenderer_Dummy(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_VIDEO, pRendererProp)
{

}

CMmpRenderer_Dummy::~CMmpRenderer_Dummy()
{

}

MMP_RESULT CMmpRenderer_Dummy::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Open();

    m_luma_size = m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
	m_chroma_size = m_luma_size/4;

    return mmpResult;
}


MMP_RESULT CMmpRenderer_Dummy::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Close();

    return mmpResult;
}

MMP_RESULT CMmpRenderer_Dummy::Render(CMmpMediaSampleDecodeResult* pDecResult)
{

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_Dummy::Render(class mmp_buffer_videoframe* p_buf_videoframe) {

    CMmpRenderer::EncodeAndMux(p_buf_videoframe);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_Dummy::Render(class mmp_buffer_imageframe* p_buf_imageframe) {

    
    return MMP_SUCCESS;
}



