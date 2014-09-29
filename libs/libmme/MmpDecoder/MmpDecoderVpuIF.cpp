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

#include "MmpDecoderVpuIF.hpp"
#include "MmpUtil.hpp"
#include "vpuhelper.h"
#include "MmpH264Tool.hpp"
#include "mmp_buffer_mgr.hpp"
#include "mmp_lock.hpp"

#define VPU_ENC_TIMEOUT       1000
#define VPU_DEC_TIMEOUT       10000
#define VPU_WAIT_TIME_OUT	100		//should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
//#define PARALLEL_VPU_WAIT_TIME_OUT 0 	//the value of timeout is 0 means we just check interrupt flag. do not wait any time to give a chance of an interrupt of the next core.


//#if PARALLEL_VPU_WAIT_TIME_OUT > 0 
//#undef VPU_DEC_TIMEOUT
//#define VPU_DEC_TIMEOUT       1000
//#endif


#define EXTRA_FRAME_BUFFER_NUM	1
#define STREAM_BUF_SIZE		 0x300000  // max bitstream size
#define STREAM_END_SIZE			0


/////////////////////////////////////////////////////////////
//CMmpDecoderVpuIF Member Functions

CMmpDecoderVpuIF::CMmpDecoderVpuIF(struct MmpDecoderCreateConfig *pCreateConfig) :

m_create_config(*pCreateConfig)

,m_p_vpu_if(NULL)
,m_vpu_instance_index(-1)

,m_codec_idx(0)
,m_version(0)
,m_revision(0)
,m_productId(0)

,m_DecHandle(NULL)
,m_regFrameBufCount(0)

,m_p_stream_buffer(NULL)

,m_last_int_reason(0)
,m_input_stream_count(0)
{
    int i;

    memset(&m_decOP, 0x00, sizeof(m_decOP));
    memset(&m_dec_init_info, 0x00, sizeof(m_dec_init_info));

    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        memset(&m_vbFrame[i], 0x00, sizeof(vpu_buffer_t));
    }
    
    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        m_p_buf_videoframe[i] = NULL;
    }
    
}

CMmpDecoderVpuIF::~CMmpDecoderVpuIF()
{
    
}

MMP_RESULT CMmpDecoderVpuIF::Open()
{
    RetCode vpu_ret;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    if(mmpResult == MMP_SUCCESS) {
        m_p_vpu_if = mmp_vpu_if::create_object();
        if(m_p_vpu_if == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL : mmp_vpu_if::create_object\n")));
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
        
        m_p_stream_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(STREAM_BUF_SIZE);
        if(m_p_stream_buffer == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL :  alloc stream buffer\n")));
            mmpResult = MMP_FAILURE;
        }
        else {

            class mmp_buffer_addr buf_addr;

            buf_addr = this->m_p_stream_buffer->get_buf_addr();
            m_vpu_stream_buffer.base = buf_addr.m_vir_addr;
            m_vpu_stream_buffer.phys_addr = buf_addr.m_phy_addr;
            m_vpu_stream_buffer.size = buf_addr.m_size;
            m_vpu_stream_buffer.virt_addr = buf_addr.m_vir_addr;

            m_p_vpu_if->vdi_register_dma_memory(m_codec_idx, &m_vpu_stream_buffer);
        }
    }

    /* Decoder Open */
    if(mmpResult == MMP_SUCCESS) {

        switch(this->m_create_config.nFormat) {
    
            /* Video */
            case MMP_FOURCC_VIDEO_H263: this->make_decOP_H263(); break;
            case MMP_FOURCC_VIDEO_H264: this->make_decOP_H264(); break;
            case MMP_FOURCC_VIDEO_MPEG4: this->make_decOP_MPEG4(); break;
            case MMP_FOURCC_VIDEO_MPEG2: this->make_decOP_MPEG2(); break;
            
            case MMP_FOURCC_VIDEO_WMV3:
            case MMP_FOURCC_VIDEO_VC1: this->make_decOP_VC1(); break;

            case MMP_FOURCC_VIDEO_MSMPEG4V2:
            case MMP_FOURCC_VIDEO_MSMPEG4V3: this->make_decOP_MSMpeg4V3(); break;
            
            case MMP_FOURCC_VIDEO_RV30: this->make_decOP_RV30(); break;
            case MMP_FOURCC_VIDEO_RV40: this->make_decOP_RV40(); break;

            case MMP_FOURCC_VIDEO_VP80: this->make_decOP_VP80(); break;

            case MMP_FOURCC_VIDEO_THEORA: this->make_decOP_Theora(); break;

            default:  
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL : Not Support Format(%c%c%c%c) %dx%d"), 
                            MMPGETFOURCC(this->m_create_config.nFormat, 0),MMPGETFOURCC(this->m_create_config.nFormat, 1),
                            MMPGETFOURCC(this->m_create_config.nFormat, 2),MMPGETFOURCC(this->m_create_config.nFormat, 3),
                            this->m_create_config.nPicWidth, this->m_create_config.nPicHeight
                          ));
                break;
        }

        if(mmpResult == MMP_SUCCESS) {

            vpu_ret = m_p_vpu_if->VPU_DecOpen(&m_DecHandle, &m_decOP);
	        if( vpu_ret != RETCODE_SUCCESS ) {
		        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL :  m_p_vpu_if->VPU_DecOpen (vpu_ret=%d) \n"), vpu_ret));
                mmpResult = MMP_FAILURE;
	        }
            else {

                m_vpu_instance_index = m_p_vpu_if->VPU_GetCodecInstanceIndex((void*)m_DecHandle);
               
                if (m_decOP.bitstreamMode == BS_MODE_PIC_END) {
                    m_p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, m_decOP.bitstreamBuffer, 1);	
                }

                SecAxiUse		secAxiUse = {0};

                secAxiUse.useBitEnable  = USE_BIT_INTERNAL_BUF;
	            secAxiUse.useIpEnable   = USE_IP_INTERNAL_BUF;
	            secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	            secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	            secAxiUse.useBtpEnable  = USE_BTP_INTERNAL_BUF;
	            secAxiUse.useOvlEnable  = USE_OVL_INTERNAL_BUF;

	            m_p_vpu_if->VPU_DecGiveCommand(m_DecHandle, SET_SEC_AXI, &secAxiUse);


                // MaverickCache configure
                MaverickCacheConfig decCacheConfig;

                //decConfig.frameCacheBypass   = 0;
                //    decConfig.frameCacheBurst    = 0;
                //    decConfig.frameCacheMerge    = 3;
                //    decConfig.frameCacheWayShape = 15;		
	            MaverickCache2Config(
		            &decCacheConfig, 
		            1, //decoder
		            m_decOP.cbcrInterleave, // cb cr interleave
		            0,//decConfig.frameCacheBypass,
		            0,//decConfig.frameCacheBurst,
		            3, //decConfig.frameCacheMerge,
                    m_mapType,
		            15 //decConfig.frameCacheWayShape
                    );
	            m_p_vpu_if->VPU_DecGiveCommand(m_DecHandle, SET_CACHE_CONFIG, &decCacheConfig);
            }
        }
	}

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVpuIF::Open] res=%d  CodecInstance=%d "), mmpResult, m_vpu_instance_index));
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderVpuIF::Close()
{
    MMP_S32 i;
    
    if(m_p_vpu_if)  m_p_vpu_if->enter_critical_section();

    if(m_DecHandle != NULL) {

       m_p_vpu_if->VPU_DecUpdateBitstreamBuffer(m_DecHandle, STREAM_END_SIZE);
	   m_p_vpu_if->VPU_DecClose(m_DecHandle);
       m_DecHandle = NULL;

       if(m_p_stream_buffer != NULL) {

           m_p_vpu_if->vdi_unregister_dma_memory(m_codec_idx, &m_vpu_stream_buffer);

            mmp_buffer_mgr::get_instance()->free_buffer(m_p_stream_buffer);
            m_p_stream_buffer = NULL;
       }
    }
  
    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        if(m_p_buf_videoframe[i] != NULL) {
            mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videoframe[i]);
            m_p_buf_videoframe[i] = NULL;
        }
    }

    if(m_p_vpu_if) m_p_vpu_if->leave_critical_section();

    if(m_p_vpu_if != NULL) {
        mmp_vpu_if::destroy_object(m_p_vpu_if);
        m_p_vpu_if = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::DecodeDSI(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_U8* p_stream;
    MMP_S32 stream_size;

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 size;
    RetCode vpu_ret;
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    p_stream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] ln=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                  __LINE__, stream_size,
                   p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                   p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                   p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                   p_stream[12], p_stream[13], p_stream[14], p_stream[15] 
          ));


    
    switch(this->m_create_config.nFormat) {

        case MMP_FOURCC_VIDEO_H264:
            mmpResult = this->make_seqheader_H264(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_VC1:
        case MMP_FOURCC_VIDEO_WMV3:
            mmpResult = this->make_seqheader_VC1(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_MSMPEG4V2:
        case MMP_FOURCC_VIDEO_MSMPEG4V3: 
            mmpResult = this->make_seqheader_DIV3(p_buf_videostream);
            break;
            
        case MMP_FOURCC_VIDEO_RV30:
        case MMP_FOURCC_VIDEO_RV40:
            mmpResult = this->make_seqheader_RV(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_VP80:
            mmpResult = this->make_seqheader_VP8(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_THEORA:
            mmpResult = this->make_seqheader_Theora(p_buf_videostream);
            break;

        default:
            mmpResult = this->make_seqheader_Common(p_buf_videostream);
    }
    
    /* Input DSI Stream */
    if(mmpResult == MMP_SUCCESS) {
        size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx,  m_DecHandle, 
                                                  &m_vpu_stream_buffer, 
                                                  (MMP_U8*)p_buf_videostream->get_dsi_buffer(), p_buf_videostream->get_dsi_size(), 
                                                  m_decOP.streamEndian);
	    if (size <0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: WriteBsBufFromBufHelper ")));
		    mmpResult = MMP_FAILURE;
	    }
    }
    else {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: make seqheader ")));
    }

    
    /* RUN Seq Init */
    if(mmpResult == MMP_SUCCESS) {

        if(m_decOP.bitstreamMode == BS_MODE_PIC_END)
	    {
		    vpu_ret = m_p_vpu_if->VPU_DecGetInitialInfo(m_DecHandle, &m_dec_init_info);
		    if(vpu_ret != RETCODE_SUCCESS) {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: m_p_vpu_if->VPU_DecGetInitialInfo ln=%d "), __LINE__));
                mmpResult = MMP_FAILURE;
		    }
		    m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
	    }
    }

    /* Framebuffer Register */
    if(mmpResult == MMP_SUCCESS) {
    
        FrameBuffer user_frame[MAX_FRAMEBUFFER_COUNT];//*pUserFrame = NULL;
        class mmp_buffer_addr buf_addr;
        int i;

        int framebufStride = MMP_BYTE_ALIGN(m_dec_init_info.picWidth, 16);
        int framebufHeight = MMP_BYTE_ALIGN(m_dec_init_info.picHeight, 16);

        m_regFrameBufCount = m_dec_init_info.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;

        if( (m_dec_init_info.minFrameBufferCount >= 1) 
            && (m_dec_init_info.picWidth > 16)
            && (m_dec_init_info.picHeight > 16) )
        {
        
            for(i = 0; i < m_regFrameBufCount; i++) {
                m_p_buf_videoframe[i] = mmp_buffer_mgr::get_instance()->alloc_media_videoframe(m_dec_init_info.picWidth, m_dec_init_info.picHeight, MMP_FOURCC_IMAGE_YUV420_P3);
                if(m_p_buf_videoframe[i] == NULL) {
                    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: alloc frame buf idx=%d  m_regFrameBufCount=%d w=%d h=%d "), i, m_regFrameBufCount, m_dec_init_info.picWidth, m_dec_init_info.picHeight));
                    break;
                }
                else {

                    
                    memset(&user_frame[i], 0x00, sizeof(FrameBuffer));
                    user_frame[i].bufY = m_p_buf_videoframe[i]->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_Y);
                    user_frame[i].bufCb = m_p_buf_videoframe[i]->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_U);
                    user_frame[i].bufCr = m_p_buf_videoframe[i]->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_V);
                    user_frame[i].mapType = m_mapType;
                    user_frame[i].stride = framebufStride;
                    user_frame[i].height = framebufHeight;
                    //user_frame[i].ion_shared_fd = buf_addr.m_shared_fd;
                    //user_frame[i].ion_base_phyaddr = buf_addr.m_phy_addr;
                    user_frame[i].myIndex = i;
                }
            }

            if(i == m_regFrameBufCount) {
                vpu_ret = m_p_vpu_if->VPU_DecRegisterFrameBuffer(m_DecHandle, user_frame, m_regFrameBufCount, framebufStride, framebufHeight, m_mapType);
                if(vpu_ret != RETCODE_SUCCESS)  {
                    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: m_p_vpu_if->VPU_DecRegisterFrameBuffer ")));
                    mmpResult = MMP_FAILURE;
                }
            }
            else {
            
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: alloc decoded buffer")));
                mmpResult = MMP_FAILURE;
            }

        } /* end of if(m_dec_init_info.minFrameBufferCount >= 1) */
        else {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: m_dec_init_info.minFrameBufferCount=%d w=%d h=%d "), 
                              m_dec_init_info.minFrameBufferCount,
                              m_dec_init_info.picWidth, m_dec_init_info.picHeight
                              ));
            mmpResult = MMP_FAILURE;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::DecodeAu_PinEnd(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    DecParam		decParam	= {0};
    MMP_S32 int_reason, size;
    MMP_U32 start_tick, t1, t2, t3 ,t4;
    MMP_U8* p_stream;
    MMP_S32 stream_size;
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    start_tick = CMmpUtil::GetTickCount();

    decParam.iframeSearchEnable = 0;
    decParam.skipframeMode = 0;
    decParam.DecStdParam.mp2PicFlush = 0;

    p_stream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    m_input_stream_count++;
    

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d cnt=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                  __LINE__, m_input_stream_count, stream_size,
                   p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                   p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                   p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                   p_stream[12], p_stream[13], p_stream[14], p_stream[15] 
    ));

    /* check header */
    switch(this->m_create_config.nFormat) {

        case MMP_FOURCC_VIDEO_H264: 
            mmpResult = this->make_frameheader_H264(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_VC1:
        case MMP_FOURCC_VIDEO_WMV3:
            mmpResult = this->make_frameheader_VC1(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_MSMPEG4V2:
        case MMP_FOURCC_VIDEO_MSMPEG4V3:
            mmpResult = this->make_frameheader_DIV3(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_RV30:
        case MMP_FOURCC_VIDEO_RV40:
            mmpResult = this->make_frameheader_RV(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_VP80:
            mmpResult = this->make_frameheader_VP8(p_buf_videostream);
            break;

        case MMP_FOURCC_VIDEO_THEORA:
            mmpResult = this->make_frameheader_Theora(p_buf_videostream);
            break;

        default:
            mmpResult = this->make_frameheader_Common(p_buf_videostream);
    }


    if(mmpResult == MMP_SUCCESS) {

        if(p_buf_videostream->get_header_size() > 0) {
            size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
                                                       &m_vpu_stream_buffer, 
                                                       (BYTE*)p_buf_videostream->get_header_buffer(), p_buf_videostream->get_header_size(), 
                                                       m_decOP.streamEndian);
            if (size <0)
	        {
		        mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: WriteBsBufFromBufHelper")));
	        }
        }

        if(mmpResult == MMP_SUCCESS) {
            size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
                                                       &m_vpu_stream_buffer, 
                                                       p_buf_videostream->get_stream_real_ptr(), p_buf_videostream->get_stream_real_size(), 
                                                       m_decOP.streamEndian);
            if (size <0)
	        {
		        mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: WriteBsBufFromBufHelper")));
	        }
        }
    }
    else {
        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: make frame header")));
    }

    t1 = CMmpUtil::GetTickCount();
    
    

    if(mmpResult == MMP_SUCCESS)  {

        if( (m_last_int_reason&(1<<INT_BIT_DEC_FIELD)) != 0) {
            m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
        }
        else {

	        // Start decoding a frame.
	        vpu_ret = m_p_vpu_if->VPU_DecStartOneFrame(m_DecHandle, &decParam);
	        if (vpu_ret != RETCODE_SUCCESS) 
	        {
		        mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d FAIL: m_p_vpu_if->VPU_DecStartOneFrame"), __LINE__));
	        }
        }
    }

    t2 = CMmpUtil::GetTickCount();

    int_reason = 0;
    while(mmpResult == MMP_SUCCESS) {
    
        int_reason = m_p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_DEC_TIMEOUT);
        if (int_reason == (Uint32)-1) // timeout
		{
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d  FAIL: m_p_vpu_if->VPU_WaitInterrupt   TimeOut "), __LINE__));
			VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_DecHandle);				
			mmpResult = MMP_FAILURE;
            int_reason = 0;
            break;
		}		
        
        if (int_reason & (1<<INT_BIT_DEC_FIELD)) {

            PhysicalAddress rdPtr, wrPtr;
            MMP_S32 room, chunkSize, picHeaderSize, seqHeaderSize;
            MMP_S32 remain_size;
            
            chunkSize = p_buf_videostream->get_stream_size();
            picHeaderSize = p_buf_videostream->get_header_size();
            seqHeaderSize = 0;

            m_p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);

            if(rdPtr <= wrPtr)  {
                remain_size = wrPtr - rdPtr;
            }
            else {
                remain_size = wrPtr-m_decOP.bitstreamBuffer;
                remain_size += m_decOP.bitstreamBufferSize - (rdPtr-m_decOP.bitstreamBuffer);
            }
            
            if(remain_size > 8) //(rdPtr-m_decOP.bitstreamBuffer) < (PhysicalAddress)(chunkSize+picHeaderSize+seqHeaderSize-8))	// there is full frame data in chunk data.
            {
				//m_p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, rdPtr, 0);		//set rdPtr to the position of next field data.
                MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] INT_BIT_DEC_FIELD 0x%08x Wait Next Dec (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
            }
            else {
                MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] INT_BIT_DEC_FIELD 0x%08x Wait Next Stream (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
                mmpResult = MMP_FAILURE;
                break;
            }
            
        }

        if (int_reason) {
    		m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
        }
    
        if (int_reason & (1<<INT_BIT_PIC_RUN))  {
            MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] INT_BIT_PIC_RUN 0x%08x "), int_reason ));
		    break;		
        }
    }
    m_last_int_reason = int_reason;

    t3 = CMmpUtil::GetTickCount();
    
    if(mmpResult == MMP_SUCCESS) {

        memset(&m_output_info, 0x00, sizeof(m_output_info));
        vpu_ret = m_p_vpu_if->VPU_DecGetOutputInfo(m_DecHandle, &m_output_info);
        
        t4 = CMmpUtil::GetTickCount();

        MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d dur=(%d %d %d %d) indexFrameDisplay(%d) interlace(%d) pict(%d %d) success(%d) topFieldFirst(%d)"), 
                                                            __LINE__, 
                                                            (t1-start_tick),(t2-start_tick),(t3-start_tick),(t4-start_tick), 
                                                            m_output_info.indexFrameDisplay,
                                                            m_output_info.interlacedFrame, 
                                                            m_output_info.picType, 
                                                            m_output_info.picTypeFirst, 
                                                            m_output_info.decodingSuccess,
                                                            m_output_info.topFieldFirst
                                                            ));
        if( (vpu_ret == RETCODE_SUCCESS) 
            && (m_output_info.picType<PIC_TYPE_MAX)
            ) 
        {
            if(m_output_info.indexFrameDisplay >= 0) {
            
                FrameBuffer frameBuf;
                m_p_vpu_if->VPU_DecGetFrameBuffer(m_DecHandle, m_output_info.indexFrameDisplay, &frameBuf);

                
                if(pp_buf_videoframe != NULL) {
                    *pp_buf_videoframe = m_p_buf_videoframe[m_output_info.indexFrameDisplay];
                }
                
                m_p_vpu_if->VPU_DecClrDispFlag(m_DecHandle, m_output_info.indexFrameDisplay);
            }
            else {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d m_output_info.indexFrameDisplay=%d "), __LINE__, m_output_info.indexFrameDisplay));
            }
            
            
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d FAIL: m_p_vpu_if->VPU_DecGetOutputInfo "), __LINE__));
        }
		
    }
    
    return mmpResult;
}

void CMmpDecoderVpuIF::make_decOP_Common() {
    
    
    m_mapType = 0; //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
        
    m_decOP.bitstreamFormat = (CodStd)STD_AVC_DEC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.avcExtension = 0;  // AVC extension 0(No) / 1(MVC) 
    m_decOP.coreIdx = m_codec_idx; 
#if 0
    m_decOP.bitstreamBuffer = m_vbStream.phys_addr;
	m_decOP.bitstreamBufferSize = m_vbStream.size;
#else

    m_decOP.bitstreamBuffer = this->m_p_stream_buffer->get_phy_addr();
	m_decOP.bitstreamBufferSize = this->m_p_stream_buffer->get_buf_size();
    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::make_decOP_Common] ln=%d 0x%08x %d"), __LINE__, m_decOP.bitstreamBuffer, m_decOP.bitstreamBufferSize));
#endif
	m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	m_decOP.tiled2LinearEnable = (m_mapType>>4)&0x1;
    m_decOP.bitstreamMode = BS_MODE_PIC_END; //Bitstream Mode(0: Interrupt mode, 1: Rollback mode, 2: PicEnd mode) 
    m_decOP.cbcrInterleave = CBCR_INTERLEAVE;
   	m_decOP.bwbEnable	  = VPU_ENABLE_BWB;
    m_decOP.frameEndian	= VPU_FRAME_ENDIAN;
	m_decOP.streamEndian   = VPU_STREAM_ENDIAN;
}

void CMmpDecoderVpuIF::make_decOP_H263() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_H263; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_H264() {
    
    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.avcExtension = 0;  // AVC extension 0(No) / 1(MVC) 
    
	
}

void CMmpDecoderVpuIF::make_decOP_MPEG4() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_MPEG4; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 1;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_MPEG2() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_MPEG2; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_VC1() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VC1; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_MSMpeg4V3() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_DIV3; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_RV30() {

    this->make_decOP_RV40();
}

void CMmpDecoderVpuIF::make_decOP_RV40() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_RV; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_VP80() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VP8; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_Theora() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_THO; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_Common(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult;
    MMP_U8 *p_stream, *p_dsi;
    MMP_S32 stream_size, dsi_size;

    p_stream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    mmpResult = p_buf_videostream->alloc_dsi_buffer(stream_size);
    if(mmpResult == MMP_SUCCESS) {

        p_dsi = (MMP_U8*)p_buf_videostream->get_dsi_buffer();
        dsi_size = stream_size;
        MMP_MEMCPY(p_dsi, p_stream, dsi_size);
        p_buf_videostream->set_dsi_size(dsi_size);
    }

    p_buf_videostream->set_stream_offset(0);

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_H264(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult;
    MMP_U8 *p_avc_dsi, *p_h264_dsi;
    MMP_S32 avc_dsi_size, h264_dsi_size;

    p_avc_dsi = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    avc_dsi_size = p_buf_videostream->get_stream_size();

    mmpResult = p_buf_videostream->alloc_dsi_buffer(128+avc_dsi_size);
    if(mmpResult == MMP_SUCCESS) {

        p_h264_dsi = (MMP_U8*)p_buf_videostream->get_dsi_buffer();
        mmpResult = CMmpH264Parser::ConvertDSI_AVC1_To_H264(p_avc_dsi, avc_dsi_size, p_h264_dsi, &h264_dsi_size);
        if(mmpResult == MMP_SUCCESS) {
            
        }
        else {
            MMP_MEMCPY(p_h264_dsi, p_avc_dsi, avc_dsi_size);
            h264_dsi_size = avc_dsi_size;
        }

        p_buf_videostream->set_dsi_size(h264_dsi_size);
        mmpResult = MMP_SUCCESS;
    }

    p_buf_videostream->set_stream_offset(0);

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_VC1(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 framerate, bitrate, w, h;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    mmpResult = p_buf_videostream->alloc_dsi_buffer(128+stream_size);
    if(mmpResult == MMP_SUCCESS) {
    
        framerate = p_buf_videostream->get_player_framerate();
        bitrate = p_buf_videostream->get_player_bitrate();
        w = p_buf_videostream->get_pic_width();
        h = p_buf_videostream->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_videostream->get_dsi_buffer();

        header_size = 0;
        MMP_PUT_LE32(pbHeader, ((0xC5 << 24)|0));   
        header_size += 4; //version
        MMP_PUT_LE32(pbHeader, stream_size);
        header_size += 4;
        if(stream_size > 0) {
            MMP_PUT_BUFFER(pbHeader, pstream, stream_size);
            header_size += stream_size;
        }
        MMP_PUT_LE32(pbHeader, h);
        header_size += 4;
        MMP_PUT_LE32(pbHeader, w);
        header_size += 4;
        MMP_PUT_LE32(pbHeader, 12);
        header_size += 4;
        MMP_PUT_LE32(pbHeader, 2 << 29 | 1 << 28 | 0x80 << 24 | 1 << 0);
        header_size += 4; // STRUCT_B_FRIST (LEVEL:3|CBR:1:RESERVE:4:HRD_BUFFER|24)
        MMP_PUT_LE32(pbHeader, bitrate);
        header_size += 4; // hrd_rate
        MMP_PUT_LE32(pbHeader, framerate);            
        header_size += 4; // frameRate

        p_buf_videostream->set_dsi_size(header_size);
        p_buf_videostream->set_stream_offset(0);
    }

    return mmpResult;
}


MMP_RESULT CMmpDecoderVpuIF::make_seqheader_DIV3(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    mmpResult = p_buf_videostream->alloc_dsi_buffer(128);
    if(mmpResult == MMP_SUCCESS) {
    
        w = p_buf_videostream->get_pic_width();
        h = p_buf_videostream->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_videostream->get_dsi_buffer();

        header_size = 0;
        MMP_PUT_LE32(pbHeader, MKTAG('C', 'N', 'M', 'V')); //signature 'CNMV'
        MMP_PUT_LE16(pbHeader, 0x00);                      //version
        MMP_PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        MMP_PUT_LE32(pbHeader, MKTAG('D', 'I', 'V', '3')); //codec FourCC
        MMP_PUT_LE16(pbHeader, w);                //width
        MMP_PUT_LE16(pbHeader, h);               //height
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.num*/);      //frame rate
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.den*/);      //time scale(?)
        MMP_PUT_LE32(pbHeader, 0 /*st->nb_index_entries*/);      //number of frames in file
        MMP_PUT_LE32(pbHeader, 0); //unused
        header_size += 32;

        p_buf_videostream->set_dsi_size(header_size);
        p_buf_videostream->set_stream_offset(0);
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_RV(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h, framerate;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    mmpResult = p_buf_videostream->alloc_dsi_buffer(128 + stream_size);
    if(mmpResult == MMP_SUCCESS) {
    
        framerate = p_buf_videostream->get_player_framerate();
        w = p_buf_videostream->get_pic_width();
        h = p_buf_videostream->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_videostream->get_dsi_buffer();

        header_size = 26 + stream_size;
        MMP_PUT_BE32(pbHeader, header_size); //Length
        MMP_PUT_LE32(pbHeader, MKTAG('V', 'I', 'D', 'O')); //MOFTag
        MMP_PUT_LE32(pbHeader, this->m_create_config.nFormat); //SubMOFTagl
        MMP_PUT_BE16(pbHeader, w);
        MMP_PUT_BE16(pbHeader, h);
        MMP_PUT_BE16(pbHeader, 0x0c); //BitCount;
        MMP_PUT_BE16(pbHeader, 0x00); //PadWidth;
        MMP_PUT_BE16(pbHeader, 0x00); //PadHeight;
        
	    MMP_PUT_BE32(pbHeader, framerate<<16);
        MMP_PUT_BUFFER(pbHeader, pstream, stream_size); //OpaqueDatata
        
        p_buf_videostream->set_dsi_size(header_size);
        p_buf_videostream->set_stream_offset(0);
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_VP8(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h, framerate;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    mmpResult = p_buf_videostream->alloc_dsi_buffer(128);
    if(mmpResult == MMP_SUCCESS) {
    
        framerate = p_buf_videostream->get_player_framerate();
        w = p_buf_videostream->get_pic_width();
        h = p_buf_videostream->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_videostream->get_dsi_buffer();

        header_size = 32;
        MMP_PUT_LE32(pbHeader, MKTAG('D', 'K', 'I', 'F')); //signature 'DKIF'
        MMP_PUT_LE16(pbHeader, 0x00);                      //version
        MMP_PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        MMP_PUT_LE32(pbHeader, MKTAG('V', 'P', '8', '0')); //codec FourCC
        MMP_PUT_LE16(pbHeader, w);                //width
        MMP_PUT_LE16(pbHeader, h);               //height
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.num*/);      //frame rate
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.den*/);      //time scale(?)
        MMP_PUT_LE32(pbHeader, 0 /*st->nb_index_entries*/);      //number of frames in file
        MMP_PUT_LE32(pbHeader, 0); //unused
        
        p_buf_videostream->set_dsi_size(header_size);
        p_buf_videostream->set_stream_offset(0);
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_Theora(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h, framerate;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;
    
    const int THO_SEQ_HEADER_LEN = 32;

    pstream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    mmpResult = p_buf_videostream->alloc_dsi_buffer(128);
    if(mmpResult == MMP_SUCCESS) {
    
        framerate = p_buf_videostream->get_player_framerate();
        w = p_buf_videostream->get_pic_width();
        h = p_buf_videostream->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_videostream->get_dsi_buffer();

        header_size = THO_SEQ_HEADER_LEN;

        // signature  : 4Byte
        MMP_PUT_BYTE(pbHeader, 'C');
        MMP_PUT_BYTE(pbHeader, 'N');
        MMP_PUT_BYTE(pbHeader, 'M');
        MMP_PUT_BYTE(pbHeader, 'V');

        // version  : 2Byte
        MMP_PUT_LE16(pbHeader, 0);

        // header length: 2Byte
        MMP_PUT_LE16(pbHeader, THO_SEQ_HEADER_LEN); 

        // FourCC : 4Byte
        MMP_PUT_BYTE(pbHeader, 'V');
        MMP_PUT_BYTE(pbHeader, 'P');
        MMP_PUT_BYTE(pbHeader, '3');
        MMP_PUT_BYTE(pbHeader, '0');

        // Size Info : 4Byte
        MMP_PUT_LE16(pbHeader, w);    // Picture Width
        MMP_PUT_LE16(pbHeader, h);   // Picture Height     

        // Etc : 16Byte
        MMP_PUT_LE32(pbHeader, 0);     // Frame Rate
        MMP_PUT_LE32(pbHeader, 0);     // Time Scale
        MMP_PUT_LE32(pbHeader, 0);     // Frame Number
        MMP_PUT_LE32(pbHeader, 0);     // Reserved
        
        p_buf_videostream->set_dsi_size(header_size);
        p_buf_videostream->set_stream_offset(0);
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_Common(class mmp_buffer_videostream* p_buf_videostream) {

    p_buf_videostream->set_header_size(0);
    p_buf_videostream->set_stream_offset(0);
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_H264(class mmp_buffer_videostream* p_buf_videostream) {

    //this->DecodeAu_StreamRemake_AVC1((MMP_U8*)p_buf_videostream->get_buf_vir_addr(), p_buf_videostream->get_stream_size()); 
    
    MMP_S32 remain_size, stream_size;
    MMP_U32* ps;
    MMP_U8* pau;

    pau = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    remain_size = p_buf_videostream->get_stream_size();

    while(remain_size > 0) {

        ps = (MMP_U32*)pau;
        stream_size = *ps;
        if(stream_size == 0x01000000) {
            break;
        }
        stream_size = MMP_SWAP_U32(stream_size);
        stream_size += 4;
        *ps = 0x01000000;
        pau+=stream_size;
        remain_size -= stream_size;
    }

    p_buf_videostream->set_header_size(0);
    p_buf_videostream->set_stream_offset(0);
        
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_VC1(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    MMP_S64 pts;
    MMP_U32 flag;

    mmpResult = p_buf_videostream->alloc_header_buffer(8);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_videostream->get_header_buffer();
        pts = p_buf_videostream->get_pts();
        stream_size = p_buf_videostream->get_stream_size();
        flag = p_buf_videostream->get_flag();

        if((flag&MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME) != 0) {
            MMP_PUT_LE32(p_header,  stream_size | 0x80000000 );
        }
        else {
            MMP_PUT_LE32(p_header, stream_size | 0x00000000 );
        }

        MMP_PUT_LE32(p_header, pts/1000); /* milli_sec */

        p_buf_videostream->set_header_size(8);
        p_buf_videostream->set_stream_offset(0);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_DIV3(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    
    mmpResult = p_buf_videostream->alloc_header_buffer(12);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_videostream->get_header_buffer();
        stream_size = p_buf_videostream->get_stream_size();

        MMP_PUT_LE32(p_header, stream_size);
        MMP_PUT_LE32(p_header, 0);
        MMP_PUT_LE32(p_header, 0);

        p_buf_videostream->set_header_size(12);
        p_buf_videostream->set_stream_offset(0);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_RV(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8 *p_header, *p_stream; 
    MMP_S32 stream_size, header_size;
    MMP_S32 cSlice, nSlice, offset, i;
    MMP_U32 val;

    p_stream = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    stream_size = p_buf_videostream->get_stream_size();

    cSlice = ((MMP_S32)p_stream[0])&0xFF;
    cSlice++;
    nSlice = stream_size - 1 - (cSlice * 8);
    header_size = 20 + (cSlice*8);
    
    mmpResult = p_buf_videostream->alloc_header_buffer(header_size);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_videostream->get_header_buffer();
        
        MMP_PUT_BE32(p_header, nSlice);
        MMP_PUT_LE32(p_header, 0);   /* time stamp (milesec) */
        MMP_PUT_BE16(p_header, 0);//m_input_stream_count);
        MMP_PUT_BE16(p_header, 0x02); //Flags
        MMP_PUT_BE32(p_header, 0x00); //LastPacket
        MMP_PUT_BE32(p_header, cSlice); //NumSegments

        offset = 1;
        for (i = 0; i < cSlice; i++)   {

            val = (p_stream[offset+3] << 24) | (p_stream[offset+2] << 16) | (p_stream[offset+1] << 8) | p_stream[offset];
            MMP_PUT_BE32(p_header, val); //isValid
            offset += 4;
            val = (p_stream[offset+3] << 24) | (p_stream[offset+2] << 16) | (p_stream[offset+1] << 8) | p_stream[offset];
            MMP_PUT_BE32(p_header, val); //Offset
            offset += 4;
        }

        p_buf_videostream->set_header_size(header_size);
        p_buf_videostream->set_stream_offset(1+(cSlice*8));
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_VP8(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    
    mmpResult = p_buf_videostream->alloc_header_buffer(12);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_videostream->get_header_buffer();
        stream_size = p_buf_videostream->get_stream_size();

        MMP_PUT_LE32(p_header, stream_size);
        MMP_PUT_LE32(p_header, 0);
        MMP_PUT_LE32(p_header, 0);

        p_buf_videostream->set_header_size(12);
        p_buf_videostream->set_stream_offset(0);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_Theora(class mmp_buffer_videostream* p_buf_videostream) {

#if 0
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    
    mmpResult = p_buf_videostream->alloc_header_buffer(12);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_videostream->get_header_buffer();
        stream_size = p_buf_videostream->get_stream_size();

        MMP_PUT_LE32(p_header, stream_size);
        MMP_PUT_LE32(p_header, 0);
        MMP_PUT_LE32(p_header, 0);

        p_buf_videostream->set_header_size(12);
        p_buf_videostream->set_stream_offset(0);
    }

    return MMP_SUCCESS;
#else
    return MMP_FAILURE;
#endif
}

