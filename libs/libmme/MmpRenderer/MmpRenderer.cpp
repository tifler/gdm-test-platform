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


#include "MmpRenderer.hpp"

#include "MmpUtil.hpp"

#if (MMP_OS == MMP_OS_WIN32)
#include "MmpRenderer_DDraw.hpp"
#include "MmpRenderer_DDrawEx1.hpp"
#include "MmpRenderer_DDrawEx2.hpp"
#include "MmpRenderer_OpenGL.hpp"
#include "MmpRenderer_OpenGLEx1.hpp"
#include "MmpRenderer_WaveOut.hpp"
//#include "MmpRenderer_WaveOutEx1.hpp"
#include "MmpRenderer_WaveOutEx2.hpp"

#elif (MMP_OS == MMP_OS_LINUX_ANDROID)

#include "MmpRenderer_AndroidSurfaceEx1.hpp"
#include "MmpRenderer_AndroidTinyAlsa.hpp"

#elif (MMP_OS == MMP_OS_LINUX_ODYSSEUS_FPGA)
//#include "MmpRenderer_OdyFpgaDisplay.hpp"
#else
#error "ERROR : Select OS"
#endif

#include "MmpRenderer_OdyClientEx1.hpp"
#include "MmpRenderer_YUVWriter.hpp"
#include "MmpRenderer_Dummy.hpp"

//////////////////////////////////////////////////////////////
// CMmpRenderer CreateObject/DestroyObject


CMmpRenderer* CMmpRenderer::CreateAudioObject(MMPWAVEFORMATEX* pwf)
{
    CMmpRendererCreateProp renderProp;
    CMmpRenderer* pObj=NULL;
    
    memcpy(&renderProp.m_wf, pwf, sizeof(MMPWAVEFORMATEX) );

#if (MMP_OS == MMP_OS_WIN32)
    pObj = new CMmpRenderer_WaveOutEx2(&renderProp);
#elif (MMP_OS == MMP_OS_LINUX_ANDROID)
    pObj = new CMmpRenderer_AndroidTinyAlsa(&renderProp);
#elif (MMP_OS == MMP_OS_LINUX_ODYSSEUS_FPGA)
    pObj = NULL;
#else
#error "ERROR : Select OS"
#endif

    if(pObj==NULL)
        return (CMmpRenderer*)NULL;

    if( pObj->Open( )!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpRenderer*)NULL;
    }

    return pObj;
}


CMmpRenderer* CMmpRenderer::CreateVideoObject(CMmpRendererCreateProp* pRendererProp) {
    
    CMmpRenderer* pObj;

    switch(pRendererProp->m_renderer_type) {

        case MMP_RENDERER_TYPE_DUMMY:
            pObj=new CMmpRenderer_Dummy(pRendererProp);
            break;

        case MMP_RENDERER_TYPE_YUVWRITER:
            pObj=new CMmpRenderer_YUVWriter(pRendererProp);
            break;

        case MMP_RENDERER_TYPE_NORMAL:
        default:
#if (MMP_OS == MMP_OS_WIN32)

            if(pRendererProp->m_hRenderWnd != NULL) {
                pObj=new CMmpRenderer_OpenGLEx1(pRendererProp);
            }
            else {
                pObj=new CMmpRenderer_YUVWriter(pRendererProp);
            }
            //pObj=new CMmpRenderer_OdyClientEx1(pRendererProp);
            //pObj=new CMmpRenderer_Dummy(pRendererProp);
            //pObj=new CMmpRenderer_FileWriter(pRendererProp);
#elif (MMP_OS_LINUX == MMP_OS_LINUX_ANDROID)
            pObj=new CMmpRenderer_AndroidSurfaceEx1(pRendererProp);
#elif (MMP_OS_LINUX == MMP_OS_LINUX_ARM)
            //pObj=new CMmpRenderer_OdyFpgaDisplay(pRendererProp);
            pObj=new CMmpRenderer_FileWriter(pRendererProp);
#elif (MMP_OS_LINUX == MMP_OS_LINUX_ODYSSEUS_FPGA)
            //pObj=new CMmpRenderer_OdyFpgaDisplay(pRendererProp);
			//pObj=new CMmpRenderer_Dummy(pRendererProp);
			//pObj=new CMmpRenderer_OdyClient(pRendererProp);
            pObj=new CMmpRenderer_OdyClientEx1(pRendererProp);
            //pObj=new CMmpRenderer_YUVWriter(pRendererProp);
#else
#error "ERROR : Select OS to create VideoRenderer"
#endif

            
    }

    if(pObj==NULL)
        return (CMmpRenderer*)NULL;

    if( pObj->Open( )!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpRenderer*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpRenderer::DestroyObject(CMmpRenderer* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

#define ENC_STREAM_MAX_SIZE (1024*1024*2)
/////////////////////////////////////////////////////////////
//CMmpRenderer Member Functions

CMmpRenderer* CMmpRenderer::s_pFirstRenderer[MMP_MEDIATYPE_MAX] = { NULL, NULL };

CMmpRenderer::CMmpRenderer(enum MMP_MEDIATYPE mt, CMmpRendererCreateProp* pRendererProp) :

m_MediaType(mt)
,m_pVideoEncoder(NULL)
,m_pMuxer(NULL)
,m_p_buf_videostream_enc(NULL)
{
	memcpy(&m_RendererProp, pRendererProp, sizeof(CMmpRendererCreateProp));
	m_pRendererProp = &m_RendererProp;

    CMmpRenderer::s_pFirstRenderer[m_MediaType] = this;
}

CMmpRenderer::~CMmpRenderer()
{

}

MMP_RESULT CMmpRenderer::Open()
{
    struct MmpEncoderCreateConfig *pEncoderCreateConfig;
    struct MmpMuxerCreateConfig muxer_create_config;

    if(m_pRendererProp->m_bVideoEncoder == MMP_TRUE) {

        /*Create Video Encoder */
        pEncoderCreateConfig = &m_pRendererProp->m_VideoEncoderCreateConfig;
        pEncoderCreateConfig->nPicWidth = this->m_pRendererProp->m_iPicWidth;
        pEncoderCreateConfig->nPicHeight = this->m_pRendererProp->m_iPicHeight;
        m_pVideoEncoder = (CMmpEncoderVideo*)CMmpEncoder::CreateVideoObject(pEncoderCreateConfig, m_pRendererProp->m_bVideoEncoderForceSWCodec);
                
        /*Create Muxer */
        if(m_pVideoEncoder != NULL) {

            memset(&muxer_create_config, 0x00, sizeof(muxer_create_config));
            strcpy((char*)muxer_create_config.filename, m_pRendererProp->m_VideoEncFileName);

            muxer_create_config.bMedia[MMP_MEDIATYPE_VIDEO] = MMP_TRUE;
            muxer_create_config.bih.biSize = sizeof(MMPBITMAPINFOHEADER);
            muxer_create_config.bih.biCompression = m_pVideoEncoder->GetFormat();
            muxer_create_config.bih.biWidth = m_pVideoEncoder->GetVideoPicWidth();
            muxer_create_config.bih.biHeight = m_pVideoEncoder->GetVideoPicHeight();
            muxer_create_config.video_bitrate = 1024*1024*4;
            muxer_create_config.video_fps = 24;
            muxer_create_config.video_idr_period = 10;
            
            m_pMuxer = CMmpMuxer::CreateObject(&muxer_create_config);
        }

        /* create stream buffer */
        m_p_buf_videostream_enc = mmp_buffer_mgr::get_instance()->alloc_media_videostream(ENC_STREAM_MAX_SIZE, mmp_buffer::HEAP);
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer::Close()
{
    if(m_pMuxer != NULL) {
        CMmpMuxer::DestroyObject(m_pMuxer);
        m_pMuxer = NULL;
    }

    if(m_pVideoEncoder != NULL) {
        CMmpEncoder::DestroyObject(m_pVideoEncoder);
        m_pVideoEncoder = NULL;
    }

    /* destoy stream buffer */
    if(m_p_buf_videostream_enc != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videostream_enc);
        m_p_buf_videostream_enc = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    MMP_RESULT mmpResult = MMP_FAILURE;

    switch(pDecResult->uiResultType) {
        case  MMP_MEDIASAMPLE_BUFFER_TYPE_ION_FD:
            mmpResult = this->Render_Ion(pDecResult);
            break;
    }
    
    return mmpResult;
}


MMP_RESULT CMmpRenderer::RenderPCM(MMP_U8* pcm_buffer, MMP_U32 pcm_byte_size) {
    CMmpMediaSampleDecodeResult dec;
	memset(&dec, 0x00, sizeof(CMmpMediaSampleDecodeResult));

    dec.uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (unsigned int)pcm_buffer;
    dec.uiDecodedSize = pcm_byte_size;

	return	this->Render(&dec);
}


MMP_RESULT CMmpRenderer::EncodeAndMux(class mmp_buffer_videoframe* p_buf_videoframe) {

    MMP_S64 pts;
    const MMP_S64 SECOND = 1000000L;
    const MMP_S64 FPS = 24;
    const MMP_S64 DurPerSecond =  SECOND/FPS;

    if( (m_pVideoEncoder != NULL) && (m_pMuxer != NULL) && (m_p_buf_videostream_enc!=NULL) ) {
        
        pts = m_p_buf_videostream_enc->get_pts();
        pts += DurPerSecond;
        p_buf_videoframe->set_pts(pts);
        
        m_pVideoEncoder->EncodeAu(p_buf_videoframe, m_p_buf_videostream_enc);
        m_pMuxer->AddMediaData(m_p_buf_videostream_enc);
    }

    return MMP_SUCCESS;
}

void* mmp_render_audio_create(int samplerate, int ch, int bitperpixel)
{
	CMmpRenderer* pRenderer;
	MMPWAVEFORMATEX wf;
	
	wf.wFormatTag = MMP_WAVE_FORMAT_PCM;
	wf.nChannels = ch;
	wf.nSamplesPerSec = samplerate;
	wf.wBitsPerSample = bitperpixel;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nChannels*(wf.wBitsPerSample/8);

	pRenderer = CMmpRenderer::CreateAudioObject(&wf);

	return (void*)pRenderer;
}

int mmp_render_audio_destroy(void* hdl)
{
	CMmpRenderer* pRenderer = (CMmpRenderer*)hdl;
	
	if(pRenderer)
	{
		CMmpRenderer::DestroyObject(pRenderer);
	}

	return 0;
}

int mmp_render_audio_write(void* hdl, char* data, int datasize)
{
	CMmpRenderer* pRenderer = (CMmpRenderer*)hdl;
	CMmpMediaSampleDecodeResult dec;
	memset(&dec, 0x00, sizeof(CMmpMediaSampleDecodeResult));

	if(pRenderer!=NULL) {
	
		dec.uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (unsigned int)data;
		dec.uiDecodedSize = datasize;

		pRenderer->Render(&dec);
	}
	return 0;
}

static CMmpRendererCreateProp s_last_renderprop;
static int s_jpeg_dump = 0;
static int s_jpeg_index = 0;
#define JPEG_FILE_PATH "d:\\work\\jpeg"

void mmp_render_video_init(void* hwnd, void* hdc, 
					  int boardwidth, int boardheight, 
					  int scrx, int scry, int scrwidht, int scrheight, 
					  int picwidht, int picheight,
                      int jpeg_dump
                      )
{
	CMmpRendererCreateProp renderprop;
	
    s_jpeg_dump = jpeg_dump;
    s_jpeg_index = 0;
	
	renderprop.m_hRenderWnd = hwnd;
	renderprop.m_hRenderDC = hdc;
	renderprop.m_iBoardWidth = boardwidth;
	renderprop.m_iBoardHeight = boardheight;
	renderprop.m_iScreenPosX = scrx;
	renderprop.m_iScreenPosY = scry;
	renderprop.m_iScreenWidth = scrwidht;
	renderprop.m_iScreenHeight = scrheight;
	renderprop.m_iPicWidth = picwidht;
	renderprop.m_iPicHeight = picheight;
	renderprop.m_renderPixelFormat = MMP_PIXELFORMAT_RGB24;
	//renderprop.m_renderPixelFormat = MMP_PIXELFORMAT_RGB32;
	
	s_last_renderprop = renderprop;
	
}

/*
void* mmp_render_video_create(void* hwnd, void* hdc, 
							  int boardwidth, int boardheight, 
							  int scrx, int scry, int scrwidht, int scrheight, 
							  int picwidht, int picheight)
{
	CMmpRendererCreateProp renderprop;
	CMmpRenderer* pRenderer;

	
	renderprop.m_hRenderWnd = hwnd;
	renderprop.m_hRenderDC = hdc;
	renderprop.m_iBoardWidth = boardwidth;
	renderprop.m_iBoardHeight = boardheight;
	renderprop.m_iScreenPosX = scrx;
	renderprop.m_iScreenPosY = scry;
	renderprop.m_iScreenWidth = scrwidht;
	renderprop.m_iScreenHeight = scrheight;
	renderprop.m_iPicWidth = picwidht;
	renderprop.m_iPicHeight = picheight;
	renderprop.m_renderPixelFormat = MMP_PIXELFORMAT_RGB24;
	//renderprop.m_renderPixelFormat = MMP_PIXELFORMAT_RGB32;
	
	pRenderer = CMmpRenderer::CreateObject(&renderprop);

    s_last_renderprop = renderprop;

	return (void*)pRenderer;
}
*/

void* mmp_render_video_create(int picwidht, int picheight, int rotate_degree)
{
	CMmpRendererCreateProp renderprop;
	CMmpRenderer* pRenderer;

    renderprop = s_last_renderprop;
	renderprop.m_iPicWidth = picwidht;
	renderprop.m_iPicHeight = picheight;
	
	pRenderer = CMmpRenderer::CreateVideoObject(&renderprop);

	return (void*)pRenderer;
}


int mmp_render_video_destroy(void* hdl)
{
	CMmpRenderer* pRenderer = (CMmpRenderer*)hdl;
	
	if(pRenderer)
	{
		CMmpRenderer::DestroyObject(pRenderer);
	}
	return 0;
}

int mmp_render_video_write(void* hdl, char* data, int datasize)
{
	CMmpRenderer* pRenderer = (CMmpRenderer*)hdl;
    CMmpMediaSampleDecodeResult dec;
	CMmpMediaSampleDecodeResult* pdec = &dec;//new CMmpMediaSampleDecodeResult;
	memset(pdec, 0x00, sizeof(CMmpMediaSampleDecodeResult));
	
    int image_width, image_height;
    if(pRenderer != NULL) {
        static int cnt = 0;
        //MMPDEBUGMSG(1, (TEXT("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww cnt=%d ts=%d \n"), cnt , cnt/30 ));
        cnt++;

        image_width = pRenderer->GetPicWidth();
        image_height = pRenderer->GetPicHeight();

	    pdec->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y] = (unsigned int)data;
        pdec->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U] = pdec->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y] + image_width*image_height;
        pdec->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V] = pdec->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U] + image_width*image_height/4;

        pdec->uiDecodedBufferStride[MMP_DECODED_BUF_Y] = image_width;
        pdec->uiDecodedBufferStride[MMP_DECODED_BUF_U] = image_width/4;
        pdec->uiDecodedBufferStride[MMP_DECODED_BUF_V] = image_width/4;


	    pRenderer->Render(pdec);

        if(s_jpeg_dump) {
            char jpegfilename[256];
            unsigned char* Y, *U, *V;
            
            Y = (unsigned char*)data;
            U = Y + (image_width*image_height);
            V = U + (image_width*image_height)/4;

            sprintf(jpegfilename, "%s\\mydump%02d.jpg", JPEG_FILE_PATH, s_jpeg_index);
            CMmpUtil::Jpeg_SW_YUV420Planar_Enc(Y, U, V, image_width, image_height, jpegfilename, 100);

            s_jpeg_index++;
        }
    }

    
    
	return 0;
}
