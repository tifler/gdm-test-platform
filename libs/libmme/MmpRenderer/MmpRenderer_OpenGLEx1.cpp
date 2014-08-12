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

#include "MmpRenderer_OpenGLEx1.hpp"
#include "../MmpComm/MmpUtil.hpp"
#include "../MmpComm/colorspace/colorspace.h"

/////////////////////////////////////////////////////////////
//CMmpRenderer_OpenGLEx1 Member Functions

CMmpRenderer_OpenGLEx1::CMmpRenderer_OpenGLEx1(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_pMmpGL(NULL)
,m_iRenderCount(0)

#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
,m_fp_dump(NULL)
#endif

{
    colorspace_init();
    
    //set mmx
    yv12_to_bgra= yv12_to_bgra_mmx;
    yv12_to_bgr=yv12_to_bgr_mmx;
}

CMmpRenderer_OpenGLEx1::~CMmpRenderer_OpenGLEx1()
{
#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
    if(m_fp_dump!=NULL) fclose(m_fp_dump);
#endif

}

MMP_RESULT CMmpRenderer_OpenGLEx1::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    m_pMmpGL=new CMmpGL_MovieEx1((HWND)m_pRendererProp->m_hRenderWnd, 
                                ::GetDC((HWND)m_pRendererProp->m_hRenderWnd), 
                                m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight);
    if(m_pMmpGL==NULL)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CWndOpenGL::OnCreate] FAIL: CMmpGL::CreateObject \n\r")));
        return MMP_FAILURE;
    }

    if(m_pMmpGL->Open()!=MMP_SUCCESS)
    {
        m_pMmpGL->Close();
        delete m_pMmpGL;
        m_pMmpGL=NULL;
        return MMP_FAILURE;
    }

    RECT rect;
    ::GetClientRect((HWND)m_pRendererProp->m_hRenderWnd, &rect);
    //m_pMmpGL->Resize( (rect.right-rect.left)/2, (rect.bottom-rect.top)/2);
    m_pMmpGL->Resize( (rect.right-rect.left), (rect.bottom-rect.top));

    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_OpenGLEx1::Close()
{
    MMP_RESULT mmpResult;

    if(m_pMmpGL)
    {
        m_pMmpGL->Close();
        delete m_pMmpGL;
        m_pMmpGL=NULL;
    }

    mmpResult=CMmpRenderer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx1::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    unsigned char* Y,*U,*V;
    int picWidth, picHeight;
    int lumaSize, y_stride, uv_stride;
    unsigned char* pImageBuffer;

    picWidth=m_pMmpGL->GetPicWidth();
    picHeight=m_pMmpGL->GetPicHeight();
    pImageBuffer=m_pMmpGL->GetImageBuffer();
    lumaSize=picWidth*picHeight;
    
    Y=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y];
    U=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U];
    V=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V];

    y_stride = pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y];
    uv_stride = pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_U];
        
    (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				    picWidth*3, //int x_stride,
					Y, //uint8_t * y_src,
					V, //uint8_t * v_src,
					U, //uint8_t * u_src,
					y_stride,//int y_stride,
					uv_stride, //int uv_stride,
					picWidth, //int width,
					picHeight, //int height,
					1 //int vflip
                    );


    m_pMmpGL->Draw();

    m_iRenderCount++;
    this->Dump(Y, U, V, picWidth, picHeight);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx1::RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    int picWidth, picHeight;
    int lumaSize;
    unsigned char* pImageBuffer;

    picWidth=m_pMmpGL->GetPicWidth();
    picHeight=m_pMmpGL->GetPicHeight();
    pImageBuffer=m_pMmpGL->GetImageBuffer();
    lumaSize=picWidth*picHeight;
            
    (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				    picWidth*3, //int x_stride,
					Y, //uint8_t * y_src,
					V, //uint8_t * v_src,
					U, //uint8_t * u_src,
					picWidth, //buffer_width,//int y_stride,
					picWidth/2, //buffer_width/2, //int uv_stride,
					picWidth, //int width,
					picHeight, //int height,
					1 //int vflip
                    );


    m_pMmpGL->Draw();

    m_iRenderCount++;
    this->Dump(Y, U, V, buffer_width, buffer_height);

    CMmpRenderer::EncodeAndMux(Y, U, V, buffer_width, buffer_height);

    return MMP_SUCCESS;
}

void CMmpRenderer_OpenGLEx1::Dump(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {
    
#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
    if(m_fp_dump==NULL) {
        m_fp_dump = fopen("d:\\work\\dump.yuv", "wb");
    }

    if(m_fp_dump != NULL) {
    
        int lumaSize;
        int chromaSize;
        
        lumaSize = buffer_width*buffer_height;
        chromaSize = lumaSize/4;
    
        //if( (m_iRenderCount >= 30*2) && (m_iRenderCount < 30*3) ) {
        if( (m_iRenderCount == 30*2) || (m_iRenderCount == 30*10) ) {
            fwrite(Y, 1, lumaSize, m_fp_dump);
            fwrite(U, 1, chromaSize, m_fp_dump);
            fwrite(V, 1, chromaSize, m_fp_dump);
        }
    }

#endif
    

}