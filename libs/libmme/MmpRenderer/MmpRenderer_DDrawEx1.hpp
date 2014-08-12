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

#ifndef _MMPRENDERER_DDRAWEX1_HPP__
#define _MMPRENDERER_DDRAWEX1_HPP__

#include "MmpRenderer.hpp"
#include "../MmpDDraw/MmpDDrawDisplay.hpp"

class CMmpRenderer_DDrawEx1 : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    CMmpDDrawDisplay* m_pDDrawDisplay;
    CMmpDDrawSurface* m_pMovieSurface;
    CMmpDDrawSurface* m_pRGBSurface;
    HBITMAP m_hMovieBitmap;
    unsigned char* m_pMovieBmpBuffer;

protected:
    CMmpRenderer_DDrawEx1(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_DDrawEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual MMP_RESULT Render_RGB565(CMmpMediaSampleDecodeResult* pDecResult);
    virtual MMP_RESULT Render_YUV420PhyAddr(CMmpMediaSampleDecodeResult* pDecResult);
public:
    virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);

};

#endif

