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

#ifndef _MMPRENDERER_OPENGLEX1_HPP__
#define _MMPRENDERER_OPENGLEX1_HPP__

#include "../MmpOpenGL/MmpGL_MovieEx1.hpp"
#include "MmpRenderer.hpp"

#define MMPRENDERER_OPENGLEX1_DUMP 0

class CMmpRenderer_OpenGLEx1 : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    CMmpGL_MovieEx1* m_pMmpGL;
    
    //HDC m_hDC;
    //HGLRC m_hRC;
    //CGLRGB m_rgbBack;
#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
    FILE* m_fp_dump;
#endif
    MMP_S32 m_iRenderCount;
    
protected:
    CMmpRenderer_OpenGLEx1(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_OpenGLEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    void Dump(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height);

public:
    virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);
    virtual MMP_RESULT RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height);
    virtual MMP_RESULT OnSize(int cx, int cy) 
    { 
        if(m_pMmpGL)
        {
            m_pMmpGL->Resize(cx,cy);
        }
        return MMP_SUCCESS;
    }
    
};

#endif
