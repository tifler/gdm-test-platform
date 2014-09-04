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

#include "MmpRenderer_Dummy.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpRenderer_Dummy Member Functions


CMmpRenderer_Dummy::CMmpRenderer_Dummy(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_VIDEO, pRendererProp)
{

}

CMmpRenderer_Dummy::~CMmpRenderer_Dummy()
{

}

MMP_RESULT CMmpRenderer_Dummy::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Open();

    m_luma_size = m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
	m_chroma_size = m_luma_size/4;

    return mmpResult;
}


MMP_RESULT CMmpRenderer_Dummy::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Close();

    return mmpResult;
}

MMP_RESULT CMmpRenderer_Dummy::Render(CMmpMediaSampleDecodeResult* pDecResult)
{

    return MMP_SUCCESS;
}

#include "vpuapi.h"

typedef void (*vdi_memcpy_func)(void* param, void* dest_vaddr, void* src_paddr, int size);

MMP_RESULT CMmpRenderer_Dummy::RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    //MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_Dummy::RenderYUV420Planar] +++ ")));

    MMP_RESULT mmpResult = MMP_SUCCESS;
    unsigned int key=0xAAAA9829;
    unsigned int key1, key2;

	key1 = *((unsigned int*)U);
    key2 = *((unsigned int*)V);

	if( (key1 == key) && (key2 == key) ) {
	    mmpResult = this->RenderYUV420Planar_Ion(Y, U, V, buffer_width, buffer_height);
    }
    else {
	    mmpResult = this->RenderYUV420Planar_Memory(Y, U, V, buffer_width, buffer_height);
    }

	return mmpResult;
}

MMP_RESULT CMmpRenderer_Dummy::RenderYUV420Planar_Memory(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    //MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_Dummy::RenderYUV420Planar_Memory] +++ ")));

    if( (m_pVideoEncoder != NULL) && (m_pMuxer != NULL) && (m_p_enc_stream!=NULL) ) {

		CMmpRenderer::EncodeAndMux(Y, U, V, buffer_width, buffer_height);

	}

    return MMP_SUCCESS;
}
    
#include "vpuapi.h"

typedef void (*vdi_memcpy_func)(void* param, void* dest_vaddr, void* src_paddr, int size);

MMP_RESULT CMmpRenderer_Dummy::RenderYUV420Planar_Ion(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    FrameBuffer* pVPU_FrameBuffer;
	//int iret;
	//unsigned int t1, t2;

    pVPU_FrameBuffer = (FrameBuffer*)Y;

    MMPDEBUGMSG(0, (TEXT("[CMmpRenderer_Dummy::RenderYUV420Planar] +++ ION  fd=%d buf(0x%08x 0x%08x 0x%08x , 0x%08x) Enc(0x%08x 0x%08x 0x%08x ) "), 
                          pVPU_FrameBuffer->ion_shared_fd, 
                          pVPU_FrameBuffer->bufY, pVPU_FrameBuffer->bufCb, pVPU_FrameBuffer->bufCr,
                          pVPU_FrameBuffer->ion_base_phyaddr,
                          
                          m_pVideoEncoder, m_pMuxer, m_p_enc_stream
                          ));
    
	
/*
    
	memset(&m_req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
	m_req_data.id = 0;
	m_req_data.num_plane = 1;
	m_req_data.data[0].memory_id = pVPU_FrameBuffer->ion_shared_fd;
	m_req_data.data[0].offset = pVPU_FrameBuffer->bufY - pVPU_FrameBuffer->ion_base_phyaddr;

	dss_overlay_queue(m_sock_fd, &m_req_data);
    if(m_gplayer.release_fd != -1) {
		//printf("wait frame done signal\n");

		t1 = CMmpUtil::GetTickCount();
		iret = sync_wait(m_gplayer.release_fd, 1000);
		t2 = CMmpUtil::GetTickCount();

		if( (t2-t1) < 100) {
			CMmpUtil::Sleep( 100 - (t2-t1) );
		}
	}
*/

	if( (m_pVideoEncoder != NULL) && (m_pMuxer != NULL) && (m_p_enc_stream!=NULL) ) {

		void* param;
		vdi_memcpy_func vdi_memcpy;
		unsigned int addr, value;
		unsigned char *dest_y, *dest_u, *dest_v;

		dest_y = new unsigned char[m_luma_size];
		dest_u = new unsigned char[m_chroma_size];
		dest_v = new unsigned char[m_chroma_size];

		addr = (unsigned int)V;

		addr+=sizeof(unsigned int);
		memcpy(&value, (void*)addr, sizeof(unsigned int));
		param = (void*)value;

		addr+=sizeof(unsigned int);
		memcpy(&value, (void*)addr, sizeof(unsigned int));
		vdi_memcpy = (vdi_memcpy_func)value;

		(*vdi_memcpy)(param, dest_y, (void*)pVPU_FrameBuffer->bufY, m_luma_size);
		(*vdi_memcpy)(param, dest_u, (void*)pVPU_FrameBuffer->bufCb, m_chroma_size);
		(*vdi_memcpy)(param, dest_v, (void*)pVPU_FrameBuffer->bufCr, m_chroma_size);

		CMmpRenderer::EncodeAndMux(dest_y, dest_u, dest_v, buffer_width, buffer_height);

        delete [] dest_y;
        delete [] dest_u;
        delete [] dest_v;
	}

	return MMP_SUCCESS;
}
    