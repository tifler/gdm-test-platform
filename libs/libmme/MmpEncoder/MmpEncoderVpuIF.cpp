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

#include "MmpEncoderVpuIF.hpp"
#include "MmpUtil.hpp"
#include "vpuhelper.h"
#include "MmpH264Tool.hpp"
#include "mmp_buffer_mgr.hpp"
#include "mmp_lock.hpp"

#define VPU_ENC_TIMEOUT       1000
#define VPU_DEC_TIMEOUT       1000
#define VPU_WAIT_TIME_OUT	 100		//should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function

#define STREAM_BUF_SIZE		 0x300000  // max bitstream size


/////////////////////////////////////////////////////////////
//CMmpEncoderVpuIF Member Functions

CMmpEncoderVpuIF::CMmpEncoderVpuIF(struct MmpEncoderCreateConfig *pCreateConfig) :

m_create_config(*pCreateConfig)

,m_p_vpu_if(NULL)

,m_codec_idx(0)
,m_version(0)
,m_revision(0)
,m_productId(0)

,m_EncHandle(NULL)
,m_regFrameBufCount(0)

,m_mapType(LINEAR_FRAME_MAP) //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
,m_srcFrameIdx(0)

,m_p_enc_buffer(NULL)
,m_p_src_frame_buffer(NULL)
{
    MMP_S32 i;

    memset(&m_encOP, 0x00, sizeof(m_encOP));
    memset(&m_enc_init_info, 0x00, sizeof(m_enc_init_info));

    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        m_p_frame_buffer[i] = NULL;
    }

}

CMmpEncoderVpuIF::~CMmpEncoderVpuIF()
{
    
}

MMP_RESULT CMmpEncoderVpuIF::Open()
{
    RetCode vpu_ret;
    MMP_RESULT mmpResult = MMP_SUCCESS;

    if(mmpResult == MMP_SUCCESS) {
        m_p_vpu_if = mmp_vpu_if::create_object();
        if(m_p_vpu_if == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL : mmp_vpu_if::create_object\n")));
            return mmpResult;
        }
    }
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    if(mmpResult == MMP_SUCCESS) {
    
        m_p_vpu_if->VPU_GetVersionInfo(m_codec_idx, &m_version, &m_revision, &m_productId);	

        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("VPU coreNum : [%d]\n"), m_codec_idx));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n"), 
                    (Uint32)(m_version>>16), (Uint32)((m_version>>(12))&0x0f), (Uint32)((m_version>>(8))&0x0f), (Uint32)((m_version)&0xff), m_revision));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("Hardware Version => %04x\n"), m_productId));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("API Version => %04x\n\n"), API_VERSION));
    
    }

    /* alloc dma buffer */
    if(mmpResult == MMP_SUCCESS) {
        
#if 1
        m_p_enc_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(STREAM_BUF_SIZE);
        if(m_p_enc_buffer == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL :  alloc stream buffer\n")));
            mmpResult = MMP_FAILURE;
        }
        else {

            class mmp_buffer_addr buf_addr;

            buf_addr = this->m_p_enc_buffer->get_buf_addr();
            m_vpu_enc_buffer.base = buf_addr.m_vir_addr;
            m_vpu_enc_buffer.phys_addr = buf_addr.m_phy_addr;
            m_vpu_enc_buffer.size = buf_addr.m_size;
            m_vpu_enc_buffer.virt_addr = buf_addr.m_vir_addr;

            m_p_vpu_if->vdi_register_dma_memory(m_codec_idx, &m_vpu_enc_buffer);
        }
#else
        m_vbStream.size	 = STREAM_BUF_SIZE;
        iret = vdi_allocate_dma_memory(m_codec_idx, &m_vbStream);
	    if(iret < 0)  {
            m_vbStream.base = 0;
		    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL :  vdi_allocate_dma_memory (iret=%d) \n"), iret));
            mmpResult = MMP_FAILURE;
	    }
#endif

    }

    /* Decoder Open */
    if(mmpResult == MMP_SUCCESS) {

        switch(this->m_create_config.nFormat) {
    
            /* Video */
            case MMP_FOURCC_VIDEO_H263: this->make_encOP_H263(); break;
            case MMP_FOURCC_VIDEO_H264: this->make_encOP_H264(); break;
            case MMP_FOURCC_VIDEO_MPEG4: this->make_encOP_MPEG4(); break;
            default:  
                mmpResult = MMP_FAILURE;
                break;
        }

        vpu_ret = m_p_vpu_if->VPU_EncOpen(&m_EncHandle, &m_encOP);
	    if( vpu_ret != RETCODE_SUCCESS ) {
		    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL :  m_p_vpu_if->VPU_EncOpen (vpu_ret=%d) \n"), vpu_ret));
            mmpResult = MMP_FAILURE;
	    }
        else {
           // ret = m_p_vpu_if->VPU_DecGiveCommand(m_EncHandle, GET_DRAM_CONFIG, &dramCfg);
	       // if( ret != RETCODE_SUCCESS ) {
		   //     VLOG(ERR, "m_p_vpu_if->VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
		   //     goto ERR_DEC_OPEN;
	       // }

            
            SecAxiUse		secAxiUse = {0};

            secAxiUse.useBitEnable = USE_BIT_INTERNAL_BUF;
	        secAxiUse.useIpEnable = USE_IP_INTERNAL_BUF;
	        secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	        secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	        secAxiUse.useBtpEnable = USE_BTP_INTERNAL_BUF;
	        secAxiUse.useOvlEnable = USE_OVL_INTERNAL_BUF;
	        m_p_vpu_if->VPU_EncGiveCommand(m_EncHandle, SET_SEC_AXI, &secAxiUse);
        }
	}


    return mmpResult;
}


MMP_RESULT CMmpEncoderVpuIF::Close()
{
    MMP_S32 i;

    if(m_p_vpu_if) m_p_vpu_if->enter_critical_section();

    if(m_EncHandle != NULL) {

        if(m_p_enc_buffer != NULL) {
           m_p_vpu_if->vdi_unregister_dma_memory(m_codec_idx, &m_vpu_enc_buffer);
        }

        if(m_p_src_frame_buffer != NULL) {
            m_p_vpu_if->vdi_unregister_dma_memory(m_codec_idx, &m_vpu_src_frame_buffer);
        }

	    m_p_vpu_if->VPU_EncClose(m_EncHandle);
        m_EncHandle = NULL;
    }

    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        if(m_p_frame_buffer[i] != NULL) {
            mmp_buffer_mgr::get_instance()->free_buffer(m_p_frame_buffer[i]);
            m_p_frame_buffer[i] = NULL;
        }
    }

    if(m_p_src_frame_buffer != NULL) {
        mmp_buffer_mgr::get_instance()->free_buffer(m_p_src_frame_buffer);
        m_p_src_frame_buffer = NULL;
    }

    if(m_p_enc_buffer != NULL) {
       mmp_buffer_mgr::get_instance()->free_buffer(m_p_enc_buffer);
       m_p_enc_buffer = NULL;
    }

    if(m_p_vpu_if)  m_p_vpu_if->leave_critical_section();

    if(m_p_vpu_if != NULL) {
        mmp_vpu_if::destroy_object(m_p_vpu_if);
        m_p_vpu_if = NULL;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVpuIF::EncodeDSI() {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    TiledMapConfig mapCfg;
//    FrameBufferAllocInfo fbAllocInfo;
    RetCode ret;
    MMP_S32 i;

    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    //srcFrameWidth = ((encOP.picWidth+15)&~15);
	//srcFrameStride = srcFrameWidth;
	//srcFrameFormat = FORMAT_420;
	//framebufFormat = FORMAT_420;
	//srcFrameHeight = ((encOP.picHeight+15)&~15);
    m_framebufWidth = MMP_BYTE_ALIGN(m_encOP.picWidth, 16); //(encConfig.rotAngle==90||encConfig.rotAngle ==270)?srcFrameHeight:srcFrameWidth;
	m_framebufHeight = MMP_BYTE_ALIGN(m_encOP.picHeight, 16);//(encConfig.rotAngle==90||encConfig.rotAngle ==270)?srcFrameWidth:srcFrameHeight;
	m_framebufStride = m_framebufWidth;

    /* Enc Seq Init */
    if(mmpResult == MMP_SUCCESS) {
        ret = m_p_vpu_if->VPU_EncGetInitialInfo(m_EncHandle, &m_enc_init_info);
	    if( ret != RETCODE_SUCCESS )
	    {
		    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: m_p_vpu_if->VPU_EncGetInitialInfo")));
		    mmpResult = MMP_FAILURE;
	    }
        else {
	        m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
    	    m_regFrameBufCount = m_enc_init_info.minFrameBufferCount;
        }
    }

    /* Register YUV FrameBuffer */
    if(mmpResult == MMP_SUCCESS) {
        // MaverickCache configure
        //encConfig.frameCacheBypass   = 0;
	    //encConfig.frameCacheBurst    = 0;
	    //encConfig.frameCacheMerge    = 3;
	    //encConfig.frameCacheWayShape = 15;			
	    MaverickCache2Config(
		    &m_encCacheConfig, 
		    0, //encoder
		    m_encOP.cbcrInterleave, // cb cr interleave
		    0,//m_encConfig.frameCacheBypass,
		    0,//m_encConfig.frameCacheBurst,
		    3,//m_encConfig.frameCacheMerge,
		    m_mapType,
		    15//encConfig.frameCacheWayShape
            );
	    m_p_vpu_if->VPU_EncGiveCommand(m_EncHandle, SET_CACHE_CONFIG, &m_encCacheConfig);

        FrameBuffer user_frame[MAX_FRAMEBUFFER_COUNT];//*pUserFrame = NULL;
        class mmp_buffer_addr buf_addr;
        
        for(i = 0; i < m_regFrameBufCount; i++) {
            m_p_frame_buffer[i] = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(m_framebufStride*m_framebufHeight*3/2);
            if(m_p_frame_buffer[i] == NULL) {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: alloc_dma_buffer (FRAME BUF) (%d/%d) sz=%d"), i, m_regFrameBufCount, m_framebufStride*m_framebufHeight*3/2 ));
                break;
            }
            else {

                buf_addr = m_p_frame_buffer[i]->get_buf_addr();

                memset(&user_frame[i], 0x00, sizeof(FrameBuffer));
                user_frame[i].bufY = buf_addr.m_phy_addr;
                user_frame[i].bufCb = buf_addr.m_phy_addr + m_framebufStride*m_framebufHeight;
                user_frame[i].bufCr = buf_addr.m_phy_addr + m_framebufStride*m_framebufHeight + m_framebufStride*m_framebufHeight/4;
                user_frame[i].mapType = m_mapType;
                user_frame[i].stride = m_framebufStride;
                user_frame[i].height = m_framebufHeight;
                user_frame[i].myIndex = i;
                
            }
        }

        if(i == m_regFrameBufCount) {
            ret = m_p_vpu_if->VPU_EncRegisterFrameBuffer(m_EncHandle, user_frame, m_regFrameBufCount, m_framebufStride, m_framebufHeight, m_mapType);
	        if( ret != RETCODE_SUCCESS )
	        {
		        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: m_p_vpu_if->VPU_EncRegisterFrameBuffer")));
		        mmpResult = MMP_FAILURE;
	        }
            else {
	            m_p_vpu_if->VPU_EncGiveCommand(m_EncHandle, GET_TILEDMAP_CONFIG, &mapCfg);
            }
        }
        else {
            mmpResult = MMP_FAILURE;
        }
    }

#if 1

    if(mmpResult == MMP_SUCCESS) {
        m_p_src_frame_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(m_framebufStride*m_framebufHeight*3/2);
        if(m_p_src_frame_buffer == NULL) {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: alloc_dma_buffer (SRC FRAME BUF)")));
            mmpResult = MMP_FAILURE;
        }
        else {

            class mmp_buffer_addr buf_addr;

            buf_addr = this->m_p_src_frame_buffer->get_buf_addr();
            m_vpu_src_frame_buffer.base = buf_addr.m_vir_addr;
            m_vpu_src_frame_buffer.phys_addr = buf_addr.m_phy_addr;
            m_vpu_src_frame_buffer.size = buf_addr.m_size;
            m_vpu_src_frame_buffer.virt_addr = buf_addr.m_vir_addr;

            m_p_vpu_if->vdi_register_dma_memory(m_codec_idx, &m_vpu_src_frame_buffer);
        }
    }

#else
    /* Allocate Frame Buffer */
    if(mmpResult == MMP_SUCCESS) {

        fbAllocInfo.format = FORMAT_420; //srcFrameFormat;
	    fbAllocInfo.cbcrInterleave = m_encOP.cbcrInterleave;
	    if(m_encOP.linear2TiledEnable)
		    fbAllocInfo.mapType = LINEAR_FRAME_MAP;
	    else
		    fbAllocInfo.mapType = m_mapType;

	    fbAllocInfo.stride = m_framebufStride;
	    fbAllocInfo.height = m_framebufHeight;
	    fbAllocInfo.num = ENC_SRC_BUF_NUM;
	    fbAllocInfo.endian = m_encOP.frameEndian;
	    fbAllocInfo.type = FB_TYPE_PPU;
	    ret = m_p_vpu_if->VPU_EncAllocateFrameBuffer(m_EncHandle, fbAllocInfo, m_fbSrc);
	    if( ret != RETCODE_SUCCESS )
	    {
		    //VLOG(ERR, "m_p_vpu_if->VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
		    mmpResult = MMP_FAILURE;
	    }
    }
#endif

    EncHeaderParam encHeaderParam = { 0 };
    MMP_U8* p_dsi = m_DSI;
    MMP_S32 dsi_size = 0;
    
    switch(m_encOP.bitstreamFormat) {
    
        case STD_MPEG4:

            encHeaderParam.buf = m_vpu_enc_buffer.phys_addr; //m_vbStream.phys_addr;
		    encHeaderParam.headerType = VOS_HEADER;
            encHeaderParam.size = m_vpu_enc_buffer.size;//m_vbStream.size;
            ret = m_p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
		    if (ret != RETCODE_SUCCESS)
		    {
			    //VLOG(ERR, "m_p_vpu_if->VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOS_HEADER failed Error code is 0x%x \n", ret);			
			    mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
		    }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
			}

            encHeaderParam.headerType = VOL_HEADER;
            encHeaderParam.size = m_vpu_enc_buffer.size; //m_vbStream.size;
		    ret = m_p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
		    if (ret != RETCODE_SUCCESS)  {
			    //VLOG(ERR, "m_p_vpu_if->VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOL_HEADER failed Error code is 0x%x \n", ret);			
			    mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
		    }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
            }
            break;

        case STD_AVC:
            encHeaderParam.headerType = SPS_RBSP;
            encHeaderParam.buf  = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;
            encHeaderParam.size = m_vpu_enc_buffer.size;//m_vbStream.size;
            ret = m_p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
            if (ret != RETCODE_SUCCESS){
                mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
            }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
            }
		
            encHeaderParam.headerType = PPS_RBSP;
            encHeaderParam.buf		= m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;
            encHeaderParam.pBuf	   = (BYTE *)m_vpu_enc_buffer.virt_addr;//m_vbStream.virt_addr;
            encHeaderParam.size	   = m_vpu_enc_buffer.size;//m_vbStream.size;
		    ret = m_p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
            if (ret != RETCODE_SUCCESS){
                mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
            }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
            }

            break;
    
    }

    if(mmpResult == MMP_SUCCESS) {
        m_DSISize = dsi_size;    
    }

    return mmpResult;
}

MMP_RESULT CMmpEncoderVpuIF::EncodeAuEx1(CMmpMediaSampleEncode* pMediaSampleEnc, CMmpMediaSampleEncodeResult* pEncResult) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    EncParam		encParam	= { 0 };
    FrameBuffer *pFB;
    int luma_size, chroma_size;
    int int_reason, timeout_count;
    RetCode ret;
    EncOutputInfo	outputInfo	= { 0 };
    MMP_U32 y_src, u_src, v_src;
    MMP_U8* pBuffer;
    MMP_U32 nBufSize, nBufMaxSize, nFlag;
    //class mmp_buffer* p_mmp_buf;

    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    luma_size = m_framebufStride*m_framebufHeight;
    chroma_size = luma_size/4;

    if(pMediaSampleEnc->uiSampleType == MMP_MEDIASAMPLE_BUFFER_TYPE_ION_FD) {
      //  p_mmp_buf = mmp_buffer_mgr::get_instance()->get_buffer(pMediaSampleEnc->uiBufferPhyAddr[MMP_YUV420_PLAINE_INDEX_Y]);
        //if
        return MMP_FAILURE;
    }
    else {
        y_src = (MMP_U32)m_p_src_frame_buffer->get_vir_addr();
        u_src = y_src + luma_size;
        v_src = u_src + chroma_size;

        m_FrameBuffer_src.bufY = (MMP_U32)m_p_src_frame_buffer->get_phy_addr();
        m_FrameBuffer_src.bufCb = m_FrameBuffer_src.bufY + luma_size;
        m_FrameBuffer_src.bufCr = m_FrameBuffer_src.bufCb + chroma_size;
        m_FrameBuffer_src.mapType = m_mapType;
        m_FrameBuffer_src.stride = m_framebufStride;
        m_FrameBuffer_src.height = m_framebufHeight;
        m_FrameBuffer_src.myIndex = MAX_FRAMEBUFFER_COUNT;
        m_FrameBuffer_src.sourceLBurstEn = 0;

        memcpy((void*)y_src, (void*)pMediaSampleEnc->uiBufferLogAddr[MMP_DECODED_BUF_Y], luma_size);
        memcpy((void*)u_src, (void*)pMediaSampleEnc->uiBufferLogAddr[MMP_DECODED_BUF_U], chroma_size);
        memcpy((void*)v_src, (void*)pMediaSampleEnc->uiBufferLogAddr[MMP_DECODED_BUF_V], chroma_size);

        pFB = &m_FrameBuffer_src;
    }

    
    encParam.forceIPicture = 0;
	encParam.skipPicture   = 0;
	encParam.quantParam	   = 10;//encConfig.picQpY;

    encParam.sourceFrame = pFB;
    encParam.picStreamBufferAddr = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;	// can set the newly allocated buffer.
    encParam.picStreamBufferSize = m_vpu_enc_buffer.size;//m_vbStream.size;

    ret = m_p_vpu_if->VPU_EncStartOneFrame(m_EncHandle, &encParam);
	if( ret != RETCODE_SUCCESS )
	{
        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeAuEx1] FAIL: m_p_vpu_if->VPU_EncStartOneFrame")));
		//VLOG(ERR, "m_p_vpu_if->VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
		//goto ERR_ENC_OPEN;
        mmpResult = MMP_FAILURE;
	}

    timeout_count = 0;
	while(mmpResult == MMP_SUCCESS) 
	{
		int_reason = m_p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_WAIT_TIME_OUT);
		if (int_reason == (int)-1)	{
			
            if( (timeout_count*VPU_WAIT_TIME_OUT) > VPU_ENC_TIMEOUT) {
				//VLOG(ERR, "Error : encoder timeout happened\n");
				m_p_vpu_if->VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_EncHandle);
                mmpResult = MMP_FAILURE;
				break;
			}
			int_reason = 0;
			timeout_count++;
		}
		
		if(1) //encOP.ringBufferEnable == 0 && encOP.lineBufIntEn) 
		{
			if (int_reason & (1<<INT_BIT_BIT_BUF_FULL))		{

                //vdi_read_memory(m_codec_idx, m_encOP.bitstreamBuffer, buf, size, endian);
				//    if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, encOP.streamEndian))
				//	goto ERR_ENC_OPEN;
                break;
			}					
		}


		if (int_reason)
		{
			m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
			if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;
		}
	}

    if(mmpResult == MMP_SUCCESS) {
    
        ret = m_p_vpu_if->VPU_EncGetOutputInfo(m_EncHandle, &outputInfo);
        if(ret != RETCODE_SUCCESS) {
            mmpResult = MMP_FAILURE;
        }
        else {
            pBuffer = (MMP_U8*)pEncResult->uiEncodedBufferLogAddr[MMP_ENCODED_BUF_STREAM];
            nBufMaxSize = pEncResult->uiEncodedBufferMaxSize[MMP_ENCODED_BUF_STREAM];
            nBufSize = outputInfo.bitstreamSize;
            if(outputInfo.picType == PIC_TYPE_I)  nFlag = MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME;
            else nFlag = 0;

            vdi_read_memory(m_codec_idx, outputInfo.bitstreamBuffer, pBuffer, outputInfo.bitstreamSize, m_encOP.streamEndian);

            pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nBufSize;
            pEncResult->uiFlag = nFlag;
        }

    }

    m_srcFrameIdx = (m_srcFrameIdx+1)%m_regFrameBufCount;

    return mmpResult;
}

MMP_RESULT CMmpEncoderVpuIF::EncodeAuEx2(CMmpMediaSampleEncode* pMediaSampleEnc, CMmpMediaSampleEncodeResult* pEncResult) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    EncParam		encParam	= { 0 };
    FrameBuffer *pFB;
    int luma_size, chroma_size;
    int int_reason, timeout_count;
    RetCode ret;
    EncOutputInfo	outputInfo	= { 0 };
    MMP_U8* pBuffer;
    MMP_U32 nBufSize, nBufMaxSize, nFlag;
    class mmp_buffer* p_mmp_buf;
    class mmp_buffer_addr mmp_buf_enc_addr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 i;

    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    luma_size = m_framebufStride*m_framebufHeight;
    chroma_size = luma_size/4;

    switch(pMediaSampleEnc->uiSampleType) {
        case MMP_MEDIASAMPLE_BUFFER_TYPE_ION_FD:

            for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                p_mmp_buf = mmp_buffer_mgr::get_instance()->get_buffer( pMediaSampleEnc->uiBufferPhyAddr[i] );
                if(p_mmp_buf == NULL) {
                    break;
                }
                mmp_buf_enc_addr[i] = p_mmp_buf->get_buf_addr();
            }
        
            if(i == MMP_IMAGE_MAX_PLANE_COUNT) {
                
                m_FrameBuffer_src.bufY = mmp_buf_enc_addr[MMP_YUV420_PLAINE_INDEX_Y].m_phy_addr + pMediaSampleEnc->uiBufferLogAddr[MMP_YUV420_PLAINE_INDEX_Y]; /* Y PhyAddr + Offset */
                m_FrameBuffer_src.bufCb = mmp_buf_enc_addr[MMP_YUV420_PLAINE_INDEX_U].m_phy_addr + pMediaSampleEnc->uiBufferLogAddr[MMP_YUV420_PLAINE_INDEX_U]; /* U PhyAddr + Offset */
                m_FrameBuffer_src.bufCr = mmp_buf_enc_addr[MMP_YUV420_PLAINE_INDEX_V].m_phy_addr + pMediaSampleEnc->uiBufferLogAddr[MMP_YUV420_PLAINE_INDEX_V]; /* V PhyAddr + Offset */
                m_FrameBuffer_src.mapType = m_mapType;
                m_FrameBuffer_src.stride = pMediaSampleEnc->uiBufferStride[MMP_YUV420_PLAINE_INDEX_Y];
                m_FrameBuffer_src.height = pMediaSampleEnc->uiBufferAlignHeight[MMP_YUV420_PLAINE_INDEX_Y];
                m_FrameBuffer_src.myIndex = MAX_FRAMEBUFFER_COUNT;
                m_FrameBuffer_src.sourceLBurstEn = 0;
                
                pFB = &m_FrameBuffer_src;
            }
            else {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeAuEx2] FAIL: get mmp_buffer from shared fd ( problem idx=%d fd=%d) "), i, pMediaSampleEnc->uiBufferPhyAddr[i] ));
                mmpResult = MMP_FAILURE;
            }

            break;
        
        default:
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeAuEx2] FAIL: Unsupport EncSample Type pMediaSampleEnc->uiSampleType=0x%x "), pMediaSampleEnc->uiSampleType));
            mmpResult = MMP_FAILURE;
            break;
    }

    
    /* Enc Start */
    if(mmpResult == MMP_SUCCESS) {

        encParam.forceIPicture = 0;
	    encParam.skipPicture   = 0;
	    encParam.quantParam	   = 10;//encConfig.picQpY;

        encParam.sourceFrame = pFB;
        encParam.picStreamBufferAddr = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;	// can set the newly allocated buffer.
        encParam.picStreamBufferSize = m_vpu_enc_buffer.size;//m_vbStream.size;

        ret = m_p_vpu_if->VPU_EncStartOneFrame(m_EncHandle, &encParam);
	    if( ret != RETCODE_SUCCESS )
	    {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeAuEx2] FAIL: m_p_vpu_if->VPU_EncStartOneFrame")));
		    //VLOG(ERR, "m_p_vpu_if->VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
		    //goto ERR_ENC_OPEN;
            mmpResult = MMP_FAILURE;
	    }
    }

    /* Wait Interrupt */
    timeout_count = 0;
	while(mmpResult == MMP_SUCCESS) 
	{
		int_reason = m_p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_WAIT_TIME_OUT);
		if (int_reason == (int)-1)	{
			
            if( (timeout_count*VPU_WAIT_TIME_OUT) > VPU_ENC_TIMEOUT) {
				//VLOG(ERR, "Error : encoder timeout happened\n");
				m_p_vpu_if->VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_EncHandle);
                mmpResult = MMP_FAILURE;
				break;
			}
			int_reason = 0;
			timeout_count++;
		}
		
		if(1) //encOP.ringBufferEnable == 0 && encOP.lineBufIntEn) 
		{
			if (int_reason & (1<<INT_BIT_BIT_BUF_FULL))		{

                //vdi_read_memory(m_codec_idx, m_encOP.bitstreamBuffer, buf, size, endian);
				//    if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, encOP.streamEndian))
				//	goto ERR_ENC_OPEN;
                break;
			}					
		}


		if (int_reason)
		{
			m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
			if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;
		}
	}

    /* Process Result */
    if(mmpResult == MMP_SUCCESS) {
    
        ret = m_p_vpu_if->VPU_EncGetOutputInfo(m_EncHandle, &outputInfo);
        if(ret != RETCODE_SUCCESS) {
            mmpResult = MMP_FAILURE;
        }
        else {
            pBuffer = (MMP_U8*)pEncResult->uiEncodedBufferLogAddr[MMP_ENCODED_BUF_STREAM];
            nBufMaxSize = pEncResult->uiEncodedBufferMaxSize[MMP_ENCODED_BUF_STREAM];
            nBufSize = outputInfo.bitstreamSize;
            if(outputInfo.picType == PIC_TYPE_I)  nFlag = MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME;
            else nFlag = 0;

            vdi_read_memory(m_codec_idx, outputInfo.bitstreamBuffer, pBuffer, outputInfo.bitstreamSize, m_encOP.streamEndian);

            pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nBufSize;
            pEncResult->uiFlag = nFlag;
        }

    }

    m_srcFrameIdx = (m_srcFrameIdx+1)%m_regFrameBufCount;

    return mmpResult;
}

MMP_RESULT CMmpEncoderVpuIF::EncodeAu(class mmp_buffer_videoframe* p_buf_videoframe, class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    EncParam		encParam	= { 0 };
    FrameBuffer *pFB;
    int luma_size, chroma_size;
    int int_reason, timeout_count;
    RetCode ret;
    EncOutputInfo	outputInfo	= { 0 };
    MMP_U8* pBuffer;
    MMP_U32 nBufSize, nBufMaxSize, nFlag;

    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    luma_size = m_framebufStride*m_framebufHeight;
    chroma_size = luma_size/4;
    
    m_FrameBuffer_src.bufY = p_buf_videoframe->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_Y);
    m_FrameBuffer_src.bufCb = p_buf_videoframe->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_U);
    m_FrameBuffer_src.bufCr = p_buf_videoframe->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_V);
    m_FrameBuffer_src.mapType = m_mapType;
    m_FrameBuffer_src.stride = p_buf_videoframe->get_stride_luma();
    m_FrameBuffer_src.height = p_buf_videoframe->get_pic_height();//p_buf_videoframe->get_buf_height(MMP_YUV420_PLAINE_INDEX_Y);
    m_FrameBuffer_src.myIndex = MAX_FRAMEBUFFER_COUNT;
    m_FrameBuffer_src.sourceLBurstEn = 0;
    
    pFB = &m_FrameBuffer_src;
    
    /* Enc Start */
    if(mmpResult == MMP_SUCCESS) {

        encParam.forceIPicture = 0;
	    encParam.skipPicture   = 0;
	    encParam.quantParam	   = 10;//encConfig.picQpY;

        encParam.sourceFrame = pFB;
        encParam.picStreamBufferAddr = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;	// can set the newly allocated buffer.
        encParam.picStreamBufferSize = m_vpu_enc_buffer.size;//m_vbStream.size;

        ret = m_p_vpu_if->VPU_EncStartOneFrame(m_EncHandle, &encParam);
	    if( ret != RETCODE_SUCCESS )
	    {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeAuEx2] FAIL: m_p_vpu_if->VPU_EncStartOneFrame")));
		    mmpResult = MMP_FAILURE;
	    }
    }

    /* Wait Interrupt */
    timeout_count = 0;
	while(mmpResult == MMP_SUCCESS) 
	{
		int_reason = m_p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_WAIT_TIME_OUT);
		if (int_reason == (int)-1)	{
			
            if( (timeout_count*VPU_WAIT_TIME_OUT) > VPU_ENC_TIMEOUT) {
				//VLOG(ERR, "Error : encoder timeout happened\n");
				m_p_vpu_if->VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_EncHandle);
                mmpResult = MMP_FAILURE;
				break;
			}
			int_reason = 0;
			timeout_count++;
		}
		
		if (int_reason & (1<<INT_BIT_BIT_BUF_FULL))	{
            break;
		}					

		if(int_reason != 0)	{
			m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
			if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;
		}
	}

    /* Process Result */
    if(mmpResult == MMP_SUCCESS) {
    
        ret = m_p_vpu_if->VPU_EncGetOutputInfo(m_EncHandle, &outputInfo);
        if(ret != RETCODE_SUCCESS) {
            mmpResult = MMP_FAILURE;
        }
        else {
            pBuffer = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
            nBufMaxSize = p_buf_videostream->get_buf_size();
            nBufSize = outputInfo.bitstreamSize;
            if(outputInfo.picType == PIC_TYPE_I)  nFlag = MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME;
            else nFlag = 0;

            memcpy(pBuffer, (void*)m_vpu_enc_buffer.virt_addr, nBufSize);

            p_buf_videostream->set_stream_size(nBufSize);
            p_buf_videostream->or_flag(nFlag);
        }
    }

    m_srcFrameIdx = (m_srcFrameIdx+1)%m_regFrameBufCount;

    return mmpResult;
}

void CMmpEncoderVpuIF::make_encOP_Common() {
    
    
    m_mapType = LINEAR_FRAME_MAP; //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
        
    m_encOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_encOP.linear2TiledEnable = (m_mapType>>4)&0x1;

    m_encOP.picWidth = this->m_create_config.nPicWidth;
    m_encOP.picHeight = this->m_create_config.nPicHeight;
    m_encOP.frameRateInfo = this->m_create_config.nFrameRate; //30;
    m_encOP.MESearchRange = 3;	
    m_encOP.bitRate = this->m_create_config.nBitRate/1024;
    m_encOP.initialDelay = 0;
    m_encOP.vbvBufferSize = 0;			// 0 = ignore
    m_encOP.meBlkMode = 0;		// for compare with C-model ( C-model = only 0 )
    m_encOP.frameSkipDisable = 1;			// for compare with C-model ( C-model = only 1 )
    m_encOP.gopSize = this->m_create_config.nIDRPeriod; //30;					// only first picture is I
    m_encOP.sliceMode.sliceMode = 1;		// 1 slice per picture
    m_encOP.sliceMode.sliceSizeMode = 1;
    m_encOP.sliceMode.sliceSize = 115;
    m_encOP.intraRefresh = 0;
    m_encOP.rcIntraQp = -1;				// disable == -1
    m_encOP.userQpMax		= 0;
    m_encOP.userGamma		= (Uint32)(0.75*32768);		//  (0*32768 < gamma < 1*32768)
    m_encOP.rcIntervalMode= 1;						// 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    m_encOP.mbInterval	= 0;
    //pEncConfig->picQpY = 23;
    //if (bitFormat == STD_MPEG4)
    //    m_encOP.MEUseZeroPmv = 1;			
    //else
    m_encOP.MEUseZeroPmv = 0;		
    m_encOP.intraCostWeight = 400;


    m_encOP.bitstreamBuffer = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;
    m_encOP.bitstreamBufferSize = m_vpu_enc_buffer.size;//m_vbStream.size;
	m_encOP.ringBufferEnable =  0;//encConfig.ringBufferEnable;
	m_encOP.cbcrInterleave = CBCR_INTERLEAVE;
	if (m_mapType == TILED_FRAME_MB_RASTER_MAP ||
		m_mapType == TILED_FIELD_MB_RASTER_MAP) {
		m_encOP.cbcrInterleave = 1;
	}
	m_encOP.frameEndian = VPU_FRAME_ENDIAN;
	m_encOP.streamEndian = VPU_STREAM_ENDIAN;
	m_encOP.bwbEnable = VPU_ENABLE_BWB;
	m_encOP.lineBufIntEn =  1; //encConfig.lineBufIntEn;
	m_encOP.coreIdx	  = m_codec_idx;
    
}

void CMmpEncoderVpuIF::make_encOP_H263() {

    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_H263; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    
    m_encOP.EncStdParam.h263Param.h263AnnexIEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexJEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexKEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexTEnable = 0;		
}

void CMmpEncoderVpuIF::make_encOP_H264() {
    
    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    
    m_encOP.EncStdParam.avcParam.constrainedIntraPredFlag = 0;
    m_encOP.EncStdParam.avcParam.disableDeblk = 1;
    m_encOP.EncStdParam.avcParam.deblkFilterOffsetAlpha = 6;
    m_encOP.EncStdParam.avcParam.deblkFilterOffsetBeta = 0;
    m_encOP.EncStdParam.avcParam.chromaQpOffset = 10;
    m_encOP.EncStdParam.avcParam.audEnable = 0;
    m_encOP.EncStdParam.avcParam.frameCroppingFlag = 0;
    m_encOP.EncStdParam.avcParam.frameCropLeft = 0;
    m_encOP.EncStdParam.avcParam.frameCropRight = 0;
    m_encOP.EncStdParam.avcParam.frameCropTop = 0;
    m_encOP.EncStdParam.avcParam.frameCropBottom = 0;
    m_encOP.EncStdParam.avcParam.level = 0;

	
}

void CMmpEncoderVpuIF::make_encOP_MPEG4() {

    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_MPEG4; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_encOP.MEUseZeroPmv = 1;			
    
    m_encOP.EncStdParam.mp4Param.mp4DataPartitionEnable = 0;
    m_encOP.EncStdParam.mp4Param.mp4ReversibleVlcEnable = 0;
    m_encOP.EncStdParam.mp4Param.mp4IntraDcVlcThr = 0;
    m_encOP.EncStdParam.mp4Param.mp4HecEnable	= 0;
    m_encOP.EncStdParam.mp4Param.mp4Verid = 2;		
}

void CMmpEncoderVpuIF::make_user_frame() {

#if 0
    FrameBufferAllocInfo fbAllocInfo;
    int framebufStride, framebufHeight;
    DRAMConfig dramCfg = {0};
    FrameBuffer  fbUser[MAX_REG_FRAME]={0,};

    framebufStride = MMP_BYTE_ALIGN(m_dec_init_info.picWidth, 16);
    framebufHeight = MMP_BYTE_ALIGN(m_dec_init_info.picHeight, 16);

    fbAllocInfo.format          = FORMAT_420;
	fbAllocInfo.cbcrInterleave  = m_encOP.cbcrInterleave;
	fbAllocInfo.mapType         = m_mapType;
	fbAllocInfo.stride          = framebufStride;
	fbAllocInfo.height          = framebufHeight;
	fbAllocInfo.num             = m_regFrameBufCount;
	
	fbAllocInfo.endian          = m_encOP.frameEndian;
	fbAllocInfo.type            = FB_TYPE_CODEC;

    m_framebufSize = m_p_vpu_if->VPU_GetFrameBufSize(framebufStride, framebufHeight, fbAllocInfo.mapType, framebufFormat, &dramCfg);

    for (i=0; i<fbAllocInfo.num; i++)
	{
		vbFrame[i].size = framebufSize;
		if (vdi_allocate_dma_memory(coreIdx, &vbFrame[i]) < 0)
		{
			VLOG(ERR, "fail to allocate frame buffer\n" );
			goto ERR_DEC_OPEN;
		}
		fbUser[i].bufY = vbFrame[i].phys_addr;
		fbUser[i].bufCb = -1;
		fbUser[i].bufCr = -1;
	}
    ret = m_p_vpu_if->VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, fbUser);
    pUserFrame = (FrameBuffer *)fbUser;				
#endif
}
