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

#include "MmpRenderer_DDrawEx2.hpp"
#include "../MmpComm/MmpUtil.hpp"
#include "../MmpComm/colorspace/colorspace.h"


/////////////////////////////////////////////////////////////
//CMmpRenderer_DDrawEx2 Member Functions

CMmpRenderer_DDrawEx2::CMmpRenderer_DDrawEx2(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_pDDrawDisplay(NULL)
,m_pMovieSurface(NULL)
{
    colorspace_init();
    
    //set mmx
    yv12_to_bgra    = yv12_to_bgra_mmx;
}

CMmpRenderer_DDrawEx2::~CMmpRenderer_DDrawEx2()
{

}

MMP_RESULT CMmpRenderer_DDrawEx2::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    m_pDDrawDisplay=CMmpDDrawDisplay::CreateWindowObject((HWND)m_pRendererProp->m_hRenderWnd, m_pRendererProp->m_iBoardWidth, m_pRendererProp->m_iBoardHeight );
    if(m_pDDrawDisplay==NULL)
    {
        return MMP_FAILURE;
    }

    mmpResult = m_pDDrawDisplay->CreateSurface(&m_pMovieSurface, MMP_PIXELFORMAT_RGB32, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG( MMPZONE_UNUSED,  (TEXT("[CMmpRenderer_DDrawEx2::Open] FAIL: Create Movie Surface( %d %d ) \n\r"), m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight ));
        m_pMovieSurface=NULL;
        return MMP_FAILURE;
    }
    
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_DDrawEx2::Close()
{
    MMP_RESULT mmpResult;

    if( m_pMovieSurface)
    {
        CMmpDDrawSurface::DestroyObject(m_pMovieSurface);
        m_pMovieSurface=NULL;
    }

    if( m_pDDrawDisplay )
    {
        CMmpDDrawDisplay::DestroyObject( m_pDDrawDisplay );
        m_pDDrawDisplay=NULL;
    }

	
    mmpResult=CMmpRenderer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_DDrawEx2::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
       //MMPDDSURFACEDESC ddsd;
    RECT rect;
    LONG wx, wy, wcx, wcy;
    LONG destX, destY, destPicWidth, destPicHeight;

    ::GetWindowRect((HWND)m_pRendererProp->m_hRenderWnd, &rect );

    wx = rect.left+m_pRendererProp->m_iScreenPosX;
    wy = rect.top+m_pRendererProp->m_iScreenPosY;;
    wcx = m_pRendererProp->m_iScreenWidth;
    wcy = m_pRendererProp->m_iScreenHeight;

    destPicWidth=wcx;
    destPicHeight=m_pRendererProp->m_iPicHeight*destPicWidth/m_pRendererProp->m_iPicWidth;
    if(destPicHeight>wcy)
    {
        destPicHeight=wcy;
        destPicWidth=m_pRendererProp->m_iPicWidth*destPicHeight/m_pRendererProp->m_iPicHeight;
    }
    destX=((wcx-destPicWidth)>>1)+wx;
    destY=((wcy-destPicHeight)>>1)+wy;

    //Copy Decoded Data to MovieSurface
    LPBYTE pSurf;
    MMPDDSURFACEDESC ddsd;

    unsigned char* Y,*U,*V;
    int lumaSize=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
    Y=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[0];
    U=Y+lumaSize;
    V=U+lumaSize/4;
    
    m_pMovieSurface->Lock(&ddsd);

    pSurf=(LPBYTE)ddsd.lpSurface;
    
    (*yv12_to_bgra)(pSurf, //uint8_t * x_ptr,
				    m_pRendererProp->m_iPicWidth*4, //int x_stride,
					Y, //uint8_t * y_src,
					U, //uint8_t * v_src,
					V, //uint8_t * u_src,
					m_pRendererProp->m_iPicWidth,//int y_stride,
					m_pRendererProp->m_iPicWidth/2, //int uv_stride,
					m_pRendererProp->m_iPicWidth, //int width,
					m_pRendererProp->m_iPicHeight, //int height,
					0 //int vflip
                    );

    //memcpy(pSurf, (void*)pDecResult->m_uiDecodedBufLogAddr, m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*3/2);

    m_pMovieSurface->UnLock();

    //Blit
    m_pDDrawDisplay->Blt(destX, destY, destPicWidth, destPicHeight, m_pMovieSurface, NULL );
    //m_pDDrawDisplay->Blt(m_pMovieSurface, NULL );

    //LCD Out
    m_pDDrawDisplay->Present(wx,wy,wcx,wcy);
    //m_pDDrawDisplay->Present();
    
    MMPDEBUGMSG(0, (TEXT("Dest(%d %d %d %d) Window(%d %d %d %d ) \n\r"), 
                    destX, destY, destPicWidth, destPicHeight,
                    wx,wy,wcx,wcy));

    return MMP_SUCCESS;
}
