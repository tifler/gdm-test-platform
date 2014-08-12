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

#include "MmpRenderer_AndroidSurfaceEx1.hpp"

#if (MMP_OS == MMP_OS_LINUX)


#include "../MmpComm/MmpUtil.hpp"
#include "../MmpComm/colorspace/colorspace.h"

/////////////////////////////////////////////////////////////
//CMmpRenderer_AndroidSurfaceEx1 Member Functions


CMmpRenderer_AndroidSurfaceEx1::CMmpRenderer_AndroidSurfaceEx1(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
{
    

}

CMmpRenderer_AndroidSurfaceEx1::~CMmpRenderer_AndroidSurfaceEx1()
{

}

MMP_RESULT CMmpRenderer_AndroidSurfaceEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult=CMmpRenderer::Open();
    

    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));
    
    SurfaceComposerClient::getDisplayInfo(display, &m_display_info);

    MMPDEBUGMSG(MMPZONE_INFO, (TEXT("[CMmpRenderer_AndroidSurfaceEx1::Open] DisplayInfo Resol(%d, %d) DPI(%3.1f %3.1f) density=%3.1f orientation=%d "), 
            m_display_info.w, m_display_info.h,
            m_display_info.xdpi, m_display_info.ydpi,
            m_display_info.density,
            m_display_info.orientation
            ));

    /* Create Composer Client */
    if(mmpResult == MMP_SUCCESS) {
    
        m_ComposerClient = new SurfaceComposerClient;
        if(m_ComposerClient->initCheck() != NO_ERROR) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidSurfaceEx1::Open] FAIL: mComposerClient = new SurfaceComposerClient")));
            mmpResult = MMP_SUCCESS;
        }
    }

    /* Create Surface */
    if(mmpResult == MMP_SUCCESS) {
        m_SurfaceControl = m_ComposerClient->createSurface(String8("CMmpRenderer_AndroidSurfaceEx1"), m_display_info.w, m_display_info.h, PIXEL_FORMAT_RGBA_8888, 0);
        //m_SurfaceControl = m_ComposerClient->createSurface(String8("CMmpRenderer_AndroidSurfaceEx1"), m_display_info.w, m_display_info.h, PIXEL_FORMAT_RGBA_8888, 0);
        if(m_SurfaceControl == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidSurfaceEx1::Open] FAIL: m_ComposerClient->createSurface")));
        }
        else { 
            if(m_SurfaceControl->isValid() != 1) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidSurfaceEx1::Open] ERROR : m_SurfaceControl->isValid() ")));
            }
        }
    }

    /* Set Surface Prop */
    if(mmpResult == MMP_SUCCESS) {
        SurfaceComposerClient::openGlobalTransaction();
        m_SurfaceControl->setLayer(0x7fffffff);
        //m_SurfaceControl->setLayer(0xffffffff);
        m_SurfaceControl->show();
        SurfaceComposerClient::closeGlobalTransaction();

        m_Surface = m_SurfaceControl->getSurface();
    }

    return mmpResult;
}


MMP_RESULT CMmpRenderer_AndroidSurfaceEx1::Close()
{
    MMP_RESULT mmpResult;

    if(m_ComposerClient != NULL) {
        m_ComposerClient->dispose();
    }

    mmpResult=CMmpRenderer::Close();
    

    return mmpResult;
}

MMP_RESULT CMmpRenderer_AndroidSurfaceEx1::Render(CMmpMediaSampleDecodeResult* pDecResult)
{

    return this->RenderYUV420Planar((MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y],
                                 (MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U],
                                 (MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V],
                                 m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight
                                 );
}

#if 1
MMP_RESULT CMmpRenderer_AndroidSurfaceEx1::RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    int luma_size, chroma_size;
    int pic_width, pic_height;
    sp<ANativeWindow> mNativeWindow(m_Surface);
    ANativeWindowBuffer *buf;
    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    void* dst[3] = { NULL, NULL, NULL };
    MMP_U8* dst_y, *dst_u, *dst_v;

    pic_width = m_RendererProp.m_iPicWidth;
    pic_height = m_RendererProp.m_iPicHeight;

    luma_size = pic_width*pic_height;
    chroma_size = luma_size/4;

    Rect bounds(pic_width, pic_height);

    native_window_set_scaling_mode(mNativeWindow.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_geometry(mNativeWindow.get(),  pic_width, pic_height, HAL_PIXEL_FORMAT_YV12);
    native_window_set_buffers_transform(mNativeWindow.get(), HAL_TRANSFORM_ROT_90);

    native_window_dequeue_buffer_and_wait(mNativeWindow.get(), &buf);
    mapper.lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN|GRALLOC_USAGE_YUV_ADDR, bounds, dst);

    //CMmpUtil::ColorConvertYUV420PlanarToRGB32(pic_width,pic_height, Y, U, V, (unsigned int*)dst, 1);

    MMPDEBUGMSG(0, (TEXT("[CMmpRenderer_AndroidSurfaceEx1::RenderYUV420Planar] 0x%08x 0x%08x 0x%08x"), (MMP_U32)dst[0],(MMP_U32)dst[1],(MMP_U32)dst[2] ));

#if 0
    dst_y = (MMP_U8*)dst[0];
    dst_u = dst_y + luma_size;
    dst_v = dst_u + chroma_size;

    
#else

    dst_y = (MMP_U8*)dst[0];
    dst_u = (MMP_U8*)dst[1];
    dst_v = (MMP_U8*)dst[2];
#endif

    memcpy(dst_y, Y, luma_size);
    memcpy(dst_u, V, chroma_size);
    memcpy(dst_v, U, chroma_size);

    mapper.unlock(buf->handle);
    mNativeWindow->queueBuffer(mNativeWindow.get(), buf,  -1);

    return MMP_SUCCESS;
}

#else
MMP_RESULT CMmpRenderer_AndroidSurfaceEx1::RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V) {

    int pic_width, pic_height;
    sp<ANativeWindow> mNativeWindow(m_Surface);
    ANativeWindowBuffer *buf;
    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    void* dst;

    pic_width = m_RendererProp.m_iPicWidth;
    pic_height = m_RendererProp.m_iPicHeight;

    Rect bounds(pic_width, pic_height);

    native_window_set_scaling_mode(mNativeWindow.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_geometry(mNativeWindow.get(),  pic_width, pic_height, PIXEL_FORMAT_RGBA_8888);
    native_window_set_buffers_transform(mNativeWindow.get(), HAL_TRANSFORM_ROT_90);

    native_window_dequeue_buffer_and_wait(mNativeWindow.get(), &buf);
    mapper.lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, bounds, &dst);

    CMmpUtil::ColorConvertYUV420PlanarToRGB32(pic_width,pic_height, Y, U, V, (unsigned int*)dst, 1);

    mapper.unlock(buf->handle);
    mNativeWindow->queueBuffer(mNativeWindow.get(), buf,  -1);

    return MMP_SUCCESS;
}
#endif

#endif