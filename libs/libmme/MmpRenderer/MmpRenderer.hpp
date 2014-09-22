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

#ifndef _MMPRENDERER_HPP__
#define _MMPRENDERER_HPP__

#include "../MmpGlobal/MmpDefine.h"
#include "MmpPlayerDef.h"
#include "MmpMediaInfo.hpp"
#include "MmpEncoderVideo.hpp"
#include "MmpMuxer.hpp"
#include "mmp_buffer_videoframe.hpp"
#include "mmp_buffer_mgr.hpp"

class CMmpRenderer
{
public:
    
	static CMmpRenderer* CreateAudioObject(MMPWAVEFORMATEX* pwf);
    static CMmpRenderer* CreateVideoObject(CMmpRendererCreateProp* pRendererProp);
    
    static MMP_RESULT DestroyObject(CMmpRenderer* pObj);

protected:
    static CMmpRenderer* s_pFirstRenderer[MMP_MEDIATYPE_MAX];

protected:
    enum MMP_MEDIATYPE m_MediaType;

    CMmpRendererCreateProp* m_pRendererProp;
	CMmpRendererCreateProp m_RendererProp;

private:
    CMmpEncoderVideo* m_pVideoEncoder;
    CMmpMuxer* m_pMuxer;
    class mmp_buffer_videostream* m_p_buf_videostream_enc;
    
protected:
    CMmpRenderer(enum MMP_MEDIATYPE mt, CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

protected:
    virtual MMP_RESULT Render_Ion(CMmpMediaSampleDecodeResult* pDecResult) {return MMP_FAILURE;}
        
protected:
    MMP_RESULT EncodeAndMux(class mmp_buffer_videoframe* p_buf_videoframe);

public:
    virtual MMP_RESULT OnSize(int cx, int cy) { return MMP_FAILURE; }

    virtual void SetFirstRenderer() { CMmpRenderer::s_pFirstRenderer[m_MediaType] = this; }
    MMP_BOOL IsFirstRenderer() { return (CMmpRenderer::s_pFirstRenderer[m_MediaType] == this)?MMP_TRUE:MMP_FALSE; }
    virtual void SetRotate(enum MMP_ROTATE rotate) {};
    virtual enum MMP_ROTATE GetRotate() { return MMP_ROTATE_0; }


    int GetPicWidth() { return m_RendererProp.m_iPicWidth; }
    int GetPicHeight() { return m_RendererProp.m_iPicHeight; }
    
    virtual MMP_RESULT Render() {return MMP_FAILURE;}
    virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);
    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe) = 0;
    virtual MMP_RESULT Render(class mmp_buffer_imageframe* p_buf_imageframe) = 0;
    
    
    virtual MMP_RESULT RenderPCM(MMP_U8* pcm_buffer, MMP_U32 pcm_byte_size);


};


#ifdef __cplusplus
extern "C" {
#endif

void* mmp_render_audio_create(int samplerate, int ch, int bitperpixel);
int mmp_render_audio_destroy(void* hdl);
int mmp_render_audio_write(void* hdl, char* data, int datasize);

void mmp_render_video_init(void* hwnd, void* hdc, 
							  int boardwidth, int boardheight, 
							  int scrx, int scry, int scrwidht, int scrheight, 
							  int picwidht, int picheight,
                              int jpeg_dump);
void* mmp_render_video_create(int picwidht, int picheight, int rotationDegrees);
int mmp_render_video_destroy(void* hdl);
int mmp_render_video_write(void* hdl, char* data, int datasize);

#ifdef __cplusplus
}
#endif

#endif

