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

#include "MmpRenderer_DDraw.hpp"
#include "../MmpComm/MmpUtil.hpp"
#include "../MmpComm/colorspace/colorspace.h"


/////////////////////////////////////////////////////////////
//CMmpRenderer_DDraw Member Functions

CMmpRenderer_DDraw::CMmpRenderer_DDraw(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_pDDrawDisplay(NULL)
,m_pMovieSurface(NULL)
,m_hMovieBitmap(NULL)
,m_pMovieBmpBuffer(NULL)
{
    colorspace_init();

    //set mmx
    yv12_to_bgra= yv12_to_bgra_mmx;
	yv12_to_bgr = yv12_to_bgr_mmx;
}

CMmpRenderer_DDraw::~CMmpRenderer_DDraw()
{

}

MMP_RESULT CMmpRenderer_DDraw::Open()
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

    mmpResult = m_pDDrawDisplay->CreateSurface(&m_pMovieSurface, m_pRendererProp->m_renderPixelFormat, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    //mmpResult = m_pDDrawDisplay->CreateSurface(&m_pMovieSurface, MMP_PIXELFORMAT_YUV420_PLANAR, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG( MMPZONE_UNUSED,  (TEXT("[CMmpRenderer_DDraw::Open] FAIL: Create Movie Surface( %d %d ) \n\r"), m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight ));
        m_pMovieSurface=NULL;
        return MMP_FAILURE;
    }
    m_pMovieSurface->Clear(RGB(255,0,0));

    char pbmi[ sizeof(BITMAPINFOHEADER)+sizeof(DWORD)*4 + 256];
    BITMAPINFO* ppbmi;
    DWORD* ColorTable;
    memset(pbmi, 0, sizeof(BITMAPINFOHEADER)+sizeof(DWORD)*3);

    switch(m_pMovieSurface->GetPixelFormat())
    {
        case MMP_PIXELFORMAT_RGB565:
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] Make BMI (MMP_PIXELFORMAT_RGB565) \n\r")));

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
            break;

        case MMP_PIXELFORMAT_RGB32:
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] Make BMI (MMP_PIXELFORMAT_RGB32) \n\r")));

            ppbmi=(BITMAPINFO*)pbmi;
            ppbmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
            ppbmi->bmiHeader.biWidth       = m_pRendererProp->m_iPicWidth;
            ppbmi->bmiHeader.biHeight      = m_pRendererProp->m_iPicHeight;
            ppbmi->bmiHeader.biPlanes      = 1;
            ppbmi->bmiHeader.biBitCount    = 32;
            ppbmi->bmiHeader.biCompression = BI_BITFIELDS; 
            ColorTable=(DWORD*)&pbmi[sizeof(BITMAPINFOHEADER)];
            ColorTable[0]=0x000000FF;
            ColorTable[1]=0x0000FF00;
            ColorTable[2]=0x00FF0000;
			ColorTable[3]=0xFF000000;
            break;

		case MMP_PIXELFORMAT_RGB24:
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] Make BMI (MMP_PIXELFORMAT_RGB24) \n\r")));

            ppbmi=(BITMAPINFO*)pbmi;
            ppbmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
            ppbmi->bmiHeader.biWidth       = m_pRendererProp->m_iPicWidth;
            ppbmi->bmiHeader.biHeight      = m_pRendererProp->m_iPicHeight;
            ppbmi->bmiHeader.biPlanes      = 1;
            ppbmi->bmiHeader.biBitCount    = 24;
            ppbmi->bmiHeader.biCompression = BI_RGB; 
            ColorTable=(DWORD*)&pbmi[sizeof(BITMAPINFOHEADER)];
            ColorTable[0]=0x000000FF;
            ColorTable[1]=0x0000FF00;
            ColorTable[2]=0x00FF0000;
			ColorTable[3]=0xFF000000;
            break;

		case MMP_PIXELFORMAT_YUV420_PHYADDR:
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] Make BMI (MMP_PIXELFORMAT_YUV420_PHYADDR)\n\r")));

            ppbmi=(BITMAPINFO*)pbmi;
            ppbmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
            ppbmi->bmiHeader.biWidth       = m_pRendererProp->m_iPicWidth;
            ppbmi->bmiHeader.biHeight      = -m_pRendererProp->m_iPicHeight;
            ppbmi->bmiHeader.biPlanes      = 1;
            ppbmi->bmiHeader.biBitCount    = 16;
            ppbmi->bmiHeader.biCompression = BI_BITFIELDS; 
            ColorTable=(DWORD*)&pbmi[sizeof(BITMAPINFOHEADER)];
            ColorTable[0]=0xF800;
            ColorTable[1]=0x07E0;
            ColorTable[2]=0x001F;
            break;

        default:
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] Make BMI (MMP_PIXELFORMAT_YUV420_PHYADDR)\n\r")));
            return MMP_FAILURE;
    }//end of switch(m_pMovieSurface->GetPixelFormat())
    

    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] TRY: CreateDIBSection\n\r")));
    m_hMovieBitmap=CreateDIBSection(::GetDC(NULL), ppbmi, DIB_RGB_COLORS,  (void**)&m_pMovieBmpBuffer, NULL, 0);
    if(!m_hMovieBitmap) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] FAIL: CreateDIBSection\n\r")));
        m_hMovieBitmap=NULL;
        return MMP_FAILURE;
    }
    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_DDraw::Open] SUCCESS: CreateDIBSection  m_hMovieBitmap:0x%08x \n\r"),
        m_hMovieBitmap ));

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_DDraw::Close()
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

MMP_RESULT CMmpRenderer_DDraw::Render_RGB565(CMmpMediaSampleDecodeResult* pDecResult)
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
    
    
#if 1
    unsigned char* Y,*U,*V;
    int lumaSize=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
    Y=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[0];
    U=Y+lumaSize;
    V=U+lumaSize/4;

#if 1
    CMmpUtil::ColorConvertYUV420PlanarToRGB565(m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight, 
                                              Y, U, V, 
                                              (unsigned short*)m_pMovieBmpBuffer,
                                              false);
#else

	(*yv12_to_bgra)(m_pMovieBmpBuffer, //uint8_t * x_ptr,
				    m_pRendererProp->m_iPicWidth*4, //int x_stride,
					 Y, //uint8_t * y_src,
					 U, //uint8_t * v_src,
					 V, //uint8_t * u_src,
					 m_pRendererProp->m_iPicWidth,//int y_stride,
					 m_pRendererProp->m_iPicWidth/2, //int uv_stride,
					 m_pRendererProp->m_iPicWidth, //int width,
					 m_pRendererProp->m_iPicHeight, //int height,
					 1 //int vflip
                     );
#endif

    m_pMovieSurface->DrawBitmap(m_hMovieBitmap, 0,0, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight);

#else
    LPBYTE pSurf;

    unsigned char* Y,*U,*V;
    int lumaSize=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
    Y=(unsigned char*)pDecResult->m_uiDecodedBufLogAddr;
    U=Y+lumaSize;
    V=U+lumaSize/4;
    
    m_pMovieSurface->Lock(&ddsd);

    pSurf=(LPBYTE)ddsd.lpSurface;
    
    (*yv12_to_bgra)(pSurf, //uint8_t * x_ptr,
				    m_YSRFileHeader.width*4, //int x_stride,
					 Y, //uint8_t * y_src,
					 U, //uint8_t * v_src,
					 V, //uint8_t * u_src,
					 m_YSRFileHeader.width,//int y_stride,
					 m_YSRFileHeader.width/2, //int uv_stride,
					 m_YSRFileHeader.width, //int width,
					 m_YSRFileHeader.height, //int height,
					 1 //int vflip
                     );

    //memcpy(pSurf, (void*)pDecResult->m_uiDecodedBufLogAddr, m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight*3/2);

    m_pMovieSurface->UnLock();
#endif

    

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

MMP_RESULT CMmpRenderer_DDraw::Render_RGB32(CMmpMediaSampleDecodeResult* pDecResult)
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
    
    
    unsigned char* Y,*U,*V;
    int lumaSize=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
    Y=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[0];
    U=Y+lumaSize;
    V=U+lumaSize/4;

#if 1
    CMmpUtil::ColorConvertYUV420PlanarToRGB32(m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight, 
                                              Y, U, V, 
                                              (unsigned int*)m_pMovieBmpBuffer,
                                              false);
#else

	(*yv12_to_bgra)(m_pMovieBmpBuffer, //uint8_t * x_ptr,
				    m_pRendererProp->m_iPicWidth*4, //int x_stride,
					 Y, //uint8_t * y_src,
					 U, //uint8_t * v_src,
					 V, //uint8_t * u_src,
					 m_pRendererProp->m_iPicWidth,//int y_stride,
					 m_pRendererProp->m_iPicWidth/2, //int uv_stride,
					 m_pRendererProp->m_iPicWidth, //int width,
					 m_pRendererProp->m_iPicHeight, //int height,
					 1 //int vflip
                     );
#endif

    m_pMovieSurface->DrawBitmap(m_hMovieBitmap, 0,0, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight);


    

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


MMP_RESULT CMmpRenderer_DDraw::Render_RGB24(CMmpMediaSampleDecodeResult* pDecResult)
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
    
    
    unsigned char* Y,*U,*V;
    int lumaSize=m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
    Y=(unsigned char*)pDecResult->uiDecodedBufferLogAddr[0];
    U=Y+lumaSize;
    V=U+lumaSize/4;

#if 1
    CMmpUtil::ColorConvertYUV420PlanarToRGB32(m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight, 
                                              Y, U, V, 
                                              (unsigned int*)m_pMovieBmpBuffer,
                                              false);
#else

	(*yv12_to_bgr)(m_pMovieBmpBuffer, //uint8_t * x_ptr,
				    m_pRendererProp->m_iPicWidth*3, //int x_stride,
					 Y, //uint8_t * y_src,
					 U, //uint8_t * v_src,
					 V, //uint8_t * u_src,
					 m_pRendererProp->m_iPicWidth,//int y_stride,
					 m_pRendererProp->m_iPicWidth/2, //int uv_stride,
					 m_pRendererProp->m_iPicWidth, //int width,
					 m_pRendererProp->m_iPicHeight, //int height,
					 1 //int vflip
                     );
#endif

    m_pMovieSurface->DrawBitmap(m_hMovieBitmap, 0,0, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight);


    

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

MMP_RESULT CMmpRenderer_DDraw::Render_YUV420PhyAddr(CMmpMediaSampleDecodeResult* pDecResult)
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
    
    memcpy(m_pMovieBmpBuffer, &pDecResult->uiDecodedBufferPhyAddr[0], sizeof(unsigned int));
    m_pMovieSurface->DrawBitmap(m_hMovieBitmap, 0,0, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight );
    
    //Blit
    m_pDDrawDisplay->Blt(destX, destY, destPicWidth, destPicHeight, m_pMovieSurface, NULL );
    //m_pDDrawDisplay->Blt(m_pMovieSurface, NULL );

    //LCD Out
    m_pDDrawDisplay->Present(wx,wy,wcx,wcy);
    //m_pDDrawDisplay->Present();
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_DDraw::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    MMP_RESULT mmpResult=MMP_FAILURE;

    if(pDecResult->uiDecodedBufferLogAddr[0] != 0)
    {
    switch(m_pMovieSurface->GetPixelFormat())
    {
        case MMP_PIXELFORMAT_RGB565:
            mmpResult=this->Render_RGB565(pDecResult);
            break;

        case MMP_PIXELFORMAT_RGB32:
            mmpResult=this->Render_RGB32(pDecResult);
            break;

		case MMP_PIXELFORMAT_RGB24:
            mmpResult=this->Render_RGB24(pDecResult);
            break;

		case MMP_PIXELFORMAT_YUV420_PHYADDR:
            mmpResult=this->Render_YUV420PhyAddr(pDecResult);
            break;
     }
    }

    return mmpResult;
}
