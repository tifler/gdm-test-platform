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


#include "MmpRenderer_OpenGL.hpp"
#include "../MmpComm/MmpUtil.hpp"
#include "../MmpComm/colorspace/colorspace.h"

/////////////////////////////////////////////////////////////
//CMmpRenderer_OpenGL Member Functions

CMmpRenderer_OpenGL::CMmpRenderer_OpenGL(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_pMmpGL(NULL)
,m_pImageBuffer(NULL)
{
    colorspace_init();
    
    //set mmx
    yv12_to_bgra= yv12_to_bgra_mmx;
    yv12_to_bgr=yv12_to_bgr_mmx;
}

CMmpRenderer_OpenGL::~CMmpRenderer_OpenGL()
{

}

CMmpGLIF* g_pMmpGL=NULL;
MMP_RESULT CMmpRenderer_OpenGL::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

#if 1
    m_pMmpGL=new CMmpGL_MovieEx1((HWND)m_pRendererProp->m_hRenderWnd, ::GetDC((HWND)m_pRendererProp->m_hRenderWnd), 100, 100);
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

    g_pMmpGL=m_pMmpGL;

    RECT rect;
    ::GetClientRect((HWND)m_pRendererProp->m_hRenderWnd, &rect);
    m_pMmpGL->Resize(rect.right-rect.left, rect.bottom-rect.top);


#else

    m_pMmpGL=CMmpGLIF::CreateObject((HWND)m_pRendererProp->m_hRenderWnd, (HDC)m_pRendererProp->m_hRenderDC);
    if(m_pMmpGL==NULL)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CWndOpenGL::OnCreate] FAIL: CMmpGL::CreateObject \n\r")));
        return -1;
    }

    g_pMmpGL=m_pMmpGL;

#endif

    m_pImageBuffer=new unsigned char[m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*4];
    if(!m_pImageBuffer)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CWndOpenGL::OnCreate] FAIL: Alloc ImageBuffer \n\r")));
        return MMP_FAILURE;
    }

    int i;
    unsigned int* pImage32=(unsigned int*)m_pImageBuffer;
    for(i=0;i<m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;i++)
        pImage32[i]=RGB(0,255,255);
    //m_pMmpGL->TEX_LoadRGB32(m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight, m_pImageBuffer, 1);
     
    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_OpenGL::Close()
{
    MMP_RESULT mmpResult;

    g_pMmpGL=NULL;

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

    if(m_pImageBuffer)
    {
        delete [] m_pImageBuffer;
        m_pImageBuffer=NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGL::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
#if 1
    unsigned char* Y,*U,*V;
    static bool bLoadText=false;
    
    int lumaSize=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
    Y=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[0];
    U=Y+lumaSize;
    V=U+lumaSize/4;
        
#if 1
    (*yv12_to_bgr)(m_pImageBuffer, //uint8_t * x_ptr,
				    m_pRendererProp->m_iPicWidth*3, //int x_stride,
					Y, //uint8_t * y_src,
					V, //uint8_t * v_src,
					U, //uint8_t * u_src,
					m_pRendererProp->m_iPicWidth,//int y_stride,
					m_pRendererProp->m_iPicWidth/2, //int uv_stride,
					m_pRendererProp->m_iPicWidth, //int width,
					m_pRendererProp->m_iPicHeight, //int height,
					1 //int vflip
                    );
#endif
    
    if(m_pMmpGL==NULL)
        return MMP_FAILURE;

#if 0
    int i;
    unsigned int* pImage32=(unsigned int*)m_pImageBuffer;
    static int iii=0;
    for(i=0;i<m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;i++)
        pImage32[i]=RGB(0,0,iii);

    iii++;
    iii%=255;
#endif

#if 0
    if(1)//bLoadText==false)
    {
        m_pMmpGL->TEX_LoadRGB32(m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight, m_pImageBuffer, 1);
        bLoadText=true;
    }
    else
    {
        m_pMmpGL->TEX_UpdateRGB32(m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight, m_pImageBuffer, 1);
    }
#endif

    m_pMmpGL->Draw();

   // m_pMmpGL->Key_Proc(VK_LEFT, 1, 0);

    //Sleep(10);
#endif
    return MMP_SUCCESS;//mmpResult;
}

MMP_RESULT CMmpRenderer_OpenGL::Render()
{
#if 0
    int i;
    unsigned int* pImage32=(unsigned int*)m_pImageBuffer;
    static int iii=0;
    //for(i=0;i<m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;i++)
    //    pImage32[i]=0x00000000;//RGB(0,0,iii);

    memset(m_pImageBuffer, 0x00, m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*4);

    for(i=0;//m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight/2;
        i<m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight/2;
        i++)
        pImage32[i]=0x000000ff;//RGB(0,0,iii);

    for(//i=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight/2
        ;
        i<m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*3/4;
        i++)
        pImage32[i]=0x0000ff00;//RGB(0,0,iii);

    for(//i=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight/2
        ;
        i<m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
        i++)
        pImage32[i]=0x00ff0000;//RGB(0,0,iii);

    iii++;
    iii%=255;
#endif
    //m_pMmpGL->TEX_LoadRGB32(m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight, m_pImageBuffer, 1);
    //m_pMmpGL->Draw();
    return MMP_SUCCESS;
}