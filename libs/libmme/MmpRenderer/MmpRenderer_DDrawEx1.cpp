/*
 *
 *  Copyright (C) 2010-2011 MtekVision Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "MmpRenderer_DDrawEx1.hpp"
#include "../MmpComm/MmpUtil.hpp"


/////////////////////////////////////////////////////////////
//CMmpRenderer_DDrawEx1 Member Functions

CMmpRenderer_DDrawEx1::CMmpRenderer_DDrawEx1(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_pDDrawDisplay(NULL)
,m_pMovieSurface(NULL)
,m_hMovieBitmap(NULL)
,m_pMovieBmpBuffer(NULL)
{

}

CMmpRenderer_DDrawEx1::~CMmpRenderer_DDrawEx1()
{

}

MMP_RESULT CMmpRenderer_DDrawEx1::Open()
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

    //mmpResult = m_pDDrawDisplay->CreateSurface(&m_pMovieSurface, m_pRendererProp->m_renderPixelFormat, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    mmpResult = m_pDDrawDisplay->CreateSurface(&m_pMovieSurface, MMP_PIXELFORMAT_YUV420_PLANAR, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG( MMPZONE_UNUSED,  (TEXT("[CMmpRenderer_DDrawEx1::Open] FAIL: Create Movie Surface( %d %d ) \n\r"), m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight ));
        m_pMovieSurface=NULL;
        return MMP_FAILURE;
    }
    m_pMovieSurface->Clear(RGB(255,0,0));

    mmpResult = m_pDDrawDisplay->CreateSurface(&m_pRGBSurface, MMP_PIXELFORMAT_RGB32, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG( MMPZONE_UNUSED,  (TEXT("[CMmpRenderer_DDrawEx1::Open] FAIL: Create Movie Surface( %d %d ) \n\r"), m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight ));
        m_pMovieSurface=NULL;
        return MMP_FAILURE;
    }
    

    char pbmi[ sizeof(BITMAPINFOHEADER)+sizeof(DWORD)*3 ];
    BITMAPINFO* ppbmi;
    DWORD* ColorTable;
    memset(pbmi, 0, sizeof(BITMAPINFOHEADER)+sizeof(DWORD)*3);

    ppbmi=(BITMAPINFO*)pbmi;
    ppbmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    ppbmi->bmiHeader.biWidth       = m_pRendererProp->m_iPicWidth;
    ppbmi->bmiHeader.biHeight      = m_pRendererProp->m_iPicHeight;
    ppbmi->bmiHeader.biPlanes      = 1;
    ppbmi->bmiHeader.biBitCount    = 16;
    ppbmi->bmiHeader.biCompression = BI_BITFIELDS; 
    ColorTable=(DWORD*)&pbmi[sizeof(BITMAPINFOHEADER)];
    ColorTable[0]=0xF800;
    ColorTable[1]=0x07E0;
    ColorTable[2]=0x001F;
    

    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDrawEx1::Open] TRY: CreateDIBSection\n\r")));
    m_hMovieBitmap=CreateDIBSection(::GetDC(NULL), ppbmi, DIB_RGB_COLORS,  (void**)&m_pMovieBmpBuffer, NULL, 0);
    if(!m_hMovieBitmap) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDrawEx1::Open] FAIL: CreateDIBSection\n\r")));
        m_hMovieBitmap=NULL;
        return MMP_FAILURE;
    }
    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDrawEx1::Open] SUCCESS: CreateDIBSection  m_hMovieBitmap:0x%08x \n\r"),
        m_hMovieBitmap ));

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_DDrawEx1::Close()
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

    if(m_hMovieBitmap)
    {
        ::DeleteObject(m_hMovieBitmap);
        m_hMovieBitmap=NULL;
        m_pMovieBmpBuffer=NULL;
    }

	
    mmpResult=CMmpRenderer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_DDrawEx1::Render_RGB565(CMmpMediaSampleDecodeResult* pDecResult)
{
    MMPDDSURFACEDESC ddsd;
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
    m_pMovieSurface->Lock(&ddsd);
    pSurf=(LPBYTE)ddsd.lpSurface;
    
    unsigned char* Y,*U,*V;
    int lumaSize=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
    Y=(unsigned char*)pDecResult->m_uiDecodedBufLogAddr;
    U=Y+lumaSize;
    V=U+lumaSize/4;
        
    memset(pSurf, 128, m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*3/2);
    //memset(pSurf, 128, m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*2);
    memcpy(pSurf, (void*)Y, m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight); //Copy Y

    //memcpy(pSurf, (void*)pDecResult->m_uiDecodedBufLogAddr, m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*3/2);
    m_pMovieSurface->UnLock();

    m_pRGBSurface->Blt(m_pMovieSurface);

    //Blit
    m_pDDrawDisplay->Blt(destX, destY, destPicWidth, destPicHeight, m_pRGBSurface, NULL );
    //m_pDDrawDisplay->Blt(m_pMovieSurface, NULL );

    //LCD Out
    m_pDDrawDisplay->Present(wx,wy,wcx,wcy);
    //m_pDDrawDisplay->Present();
    
    MMPDEBUGMSG(0, (TEXT("Dest(%d %d %d %d) Window(%d %d %d %d ) \n\r"), 
                    destX, destY, destPicWidth, destPicHeight,
                    wx,wy,wcx,wcy));

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_DDrawEx1::Render_YUV420PhyAddr(CMmpMediaSampleDecodeResult* pDecResult)
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
    
    memcpy(m_pMovieBmpBuffer, &pDecResult->m_uiDecodedBufPhyAddr, sizeof(unsigned int));
    m_pMovieSurface->DrawBitmap(m_hMovieBitmap, 0,0, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    
    //Blit
    m_pDDrawDisplay->Blt(destX, destY, destPicWidth, destPicHeight, m_pMovieSurface, NULL );
    //m_pDDrawDisplay->Blt(m_pMovieSurface, NULL );

    //LCD Out
    m_pDDrawDisplay->Present(wx,wy,wcx,wcy);
    //m_pDDrawDisplay->Present();
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_DDrawEx1::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    MMP_RESULT mmpResult=MMP_FAILURE;

    mmpResult=this->Render_RGB565(pDecResult);

    return mmpResult;
}
