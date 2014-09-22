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

#include "MmpDecoderImage_Jpu.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderImage_Jpu Member Functions

CMmpDecoderImage_Jpu::CMmpDecoderImage_Jpu(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderImage(pCreateConfig, MMP_FALSE)
,m_p_jpu_if(NULL)
,m_p_buf_imageframe_yuv(NULL)
,m_p_buf_imageframe_rgb(NULL)
{
    
}

CMmpDecoderImage_Jpu::~CMmpDecoderImage_Jpu()
{

}

MMP_RESULT CMmpDecoderImage_Jpu::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    mmpResult=CMmpDecoderImage::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    m_p_jpu_if = mmp_jpu_if::create_object();
    if(m_p_jpu_if == NULL) {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL: mmp_jpu_if::create_object \n\r"), MMP_CLASS_NAME, MMP_CLASS_FUNC));
        mmpResult = MMP_FAILURE;
    }

    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s::%s] Success nForamt=(0x%08x %s) \n\r"), MMP_CLASS_NAME, MMP_CLASS_FUNC,
                  m_nFormat, m_szCodecName ));
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderImage_Jpu::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderImage::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderImage_Jpu::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }


    if(m_p_jpu_if != NULL) {
        mmp_jpu_if::destroy_object(m_p_jpu_if);
        m_p_jpu_if = NULL;
    }
        
    if(m_p_buf_imageframe_rgb != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_rgb);
        m_p_buf_imageframe_rgb = NULL;
    }

    if(m_p_buf_imageframe_yuv != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_yuv);
        m_p_buf_imageframe_yuv = NULL;
    }

    return MMP_SUCCESS;
}

#define NUM_FRAME_BUF 3
MMP_RESULT CMmpDecoderImage_Jpu::DecodeAu(class mmp_buffer_imagestream* p_buf_imagestream, class mmp_buffer_imageframe** pp_buf_imageframe) {

    MMP_RESULT mmpResult;

    JpgDecHandle handle		= {0};
	JpgDecOpenParam	decOP		= {0};
	JpgDecInitialInfo initialInfo = {0};
	JpgDecOutputInfo outputInfo	= {0};
	JpgDecParam decParam	= {0};
	JpgRet ret = JPG_RET_SUCCESS;	
	JPU_FrameBuffer frameBuf[NUM_FRAME_BUF];
	jpu_buffer_t vbStream    = {0};
	JPU_BufInfo bufInfo     = {0};
	//FRAME_BUF *pFrame[NUM_FRAME_BUF];
	//FRAME_BUF *pDispFrame = NULL;
	Uint32 bsSize=0, framebufSize=0, framebufWidth=0, framebufHeight=0, /*framebufStride = 0,*/ framebufFormat = JPU_FORMAT_420;
	int dispWidth = 0, dispHeight = 0;
	int i = 0, frameIdx = 0, ppIdx = 0, saveIdx =0, totalNumofErrMbs = 0, streameos = 0, dispImage = 0, decodefinish = 0;
	int key = 0,  suc = 1;
	Uint8 *pFileBuf = NULL;
	Uint8 *pYuv	 =	NULL;
	FILE *fchunkData =	NULL;
	FILE *fpYuv	 =	NULL;
	int kbhitRet = 0;
	int needFrameBufCount = 0, regFrameBufCount = 0;
	int rotEnable = 0;
	int int_reason = 0;
	int instIdx;
	int partPosIdx = 0;
    int partBufIdx = 0;
	int partMaxIdx = 0;
	int partialHeight = 0;

	JPU_DecConfigParam decConfig;

    if(pp_buf_imageframe != NULL) {
        *pp_buf_imageframe = NULL;
    }
	
    if(m_p_buf_imageframe_rgb != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_rgb);
        m_p_buf_imageframe_rgb = NULL;
    }

    if(m_p_buf_imageframe_yuv != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_yuv);
        m_p_buf_imageframe_yuv = NULL;
    }

	//memcpy(&decConfig, param, sizeof(DecConfigParam));
	//memset(&pFrame, 0x00, sizeof(FRAME_BUF *)*NUM_FRAME_BUF);
	//memset(&frameBuf, 0x00, sizeof(FrameBuffer)*NUM_FRAME_BUF);

    decOP.streamEndian = JPU_STREAM_ENDIAN;
	decOP.frameEndian = JPU_FRAME_ENDIAN;
	decOP.chromaInterleave = (JPU_CbCrInterLeave)0;//(CbCrInterLeave)decConfig.chromaInterleave;
    decOP.bitstreamBuffer = (PhysicalAddress)p_buf_imagestream->get_buf_phy_addr(); //vbStream.phys_addr;  
    decOP.bitstreamBufferSize = p_buf_imagestream->get_buf_size();  //vbStream.size; 
	decOP.pBitStream = (BYTE *)p_buf_imagestream->get_buf_vir_addr(); //vbStream.virt_addr; // set virtual address mapped of physical address
	decOP.thumbNailEn = 0;//decConfig.ThumbNailEn;	
	decOP.packedFormat = PACKED_FORMAT_444;//PACKED_FORMAT_NONE;//decConfig.packedFormat;
	decOP.roiEnable = 0;//decConfig.roiEnable;
	decOP.roiOffsetX = 0;//decConfig.roiOffsetX;
	decOP.roiOffsetY = 0;//decConfig.roiOffsetY;
    decOP.roiWidth = 0;//p_buf_imagestream->get_pic_width();//decConfig.roiWidth;
	decOP.roiHeight = 0;//decConfig.roiHeight;
	rotEnable = 0;

    ret = this->m_p_jpu_if->JPU_DecOpen(&handle, &decOP);
	if( ret != JPG_RET_SUCCESS )
	{
		//JLOG(ERR, "JPU_DecOpen failed Error code is 0x%x \n", ret );
		//goto ERR_DEC_INIT;
        MMPDEBUGMSG(1, (TEXT("FAIL: JPU_DecOpen ")));
        return MMP_FAILURE;
	}
	
    ret = ::WriteJpgBsBufHelper(handle, &bufInfo, 
                                               decOP.bitstreamBuffer, 
                                                decOP.bitstreamBuffer+decOP.bitstreamBufferSize, 0, 0, &streameos, decOP.streamEndian);
	if( ret != JPG_RET_SUCCESS )
	{
		//JLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
		//goto ERR_DEC_OPEN;
        MMPDEBUGMSG(1, (TEXT("FAIL: WriteJpgBsBufHelper ")));
        return MMP_FAILURE;
	}
	
    ret = ::JPU_DecGetInitialInfo(handle, &initialInfo);
	if( ret != JPG_RET_SUCCESS )
	{
		//JLOG(ERR, "JPU_DecGetInitialInfo failed Error code is 0x%x, inst=%d \n", ret, instIdx);
		//goto ERR_DEC_OPEN;
        MMPDEBUGMSG(1, (TEXT("FAIL: JPU_DecGetInitialInfo ")));
        return MMP_FAILURE;
	}

    if (initialInfo.sourceFormat == JPU_FORMAT_420 || initialInfo.sourceFormat == JPU_FORMAT_422)
		framebufWidth = ((initialInfo.picWidth+15)/16)*16;
	else
		framebufWidth = ((initialInfo.picWidth+7)/8)*8;

	if (initialInfo.sourceFormat == JPU_FORMAT_420 || initialInfo.sourceFormat == JPU_FORMAT_224)
		framebufHeight = ((initialInfo.picHeight+15)/16)*16;
	else
		framebufHeight = ((initialInfo.picHeight+7)/8)*8;

    //if(decConfig.roiEnable)
	//{
	//	framebufWidth  = initialInfo.roiFrameWidth ;
//		framebufHeight = initialInfo.roiFrameHeight;
//	}

    framebufFormat = initialInfo.sourceFormat;
    //framebufStride = framebufWidth;
    
    if(decOP.packedFormat >= PACKED_FORMAT_422_YUYV && decOP.packedFormat <= PACKED_FORMAT_422_VYUY)
	{
		framebufWidth = framebufWidth*2;
		//framebufStride = framebufStride*2;	
		framebufFormat = JPU_FORMAT_422;
		if (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) 
			framebufFormat = JPU_FORMAT_224;
		
	}
	else if (decOP.packedFormat == PACKED_FORMAT_444)
	{
		//framebufWidth = framebufWidth*3;
		//framebufStride = framebufStride*3;	
		framebufFormat = JPU_FORMAT_444;		
	}

    //framebufSize = ::JPU_Helper_GetFrameBufSize(framebufFormat, framebufStride, framebufHeight);
    //Allocate frame buffer 
	regFrameBufCount = initialInfo.minFrameBufferCount + 0;//EXTRA_FRAME_BUFFER_NUM;
	

    MMPDEBUGMSG(1, (TEXT("[%s::%s]  JPU_DecGetInitialInfo w=%d h=%d  framebufSize=%d "),  MMP_CLASS_NAME, MMP_CLASS_FUNC, framebufWidth, framebufHeight, framebufSize ));
	
    
    m_bih_in.biWidth = framebufWidth;
    m_bih_in.biHeight = framebufHeight;
    m_bih_out.biWidth = m_bih_in.biWidth;
    m_bih_out.biHeight = m_bih_in.biHeight;
    switch(initialInfo.sourceFormat) {
    
        case JPU_FORMAT_420:  
            m_bih_out.biCompression = MMP_FOURCC_IMAGE_I420; 
            break;

        case JPU_FORMAT_444:  
            if(decOP.packedFormat == PACKED_FORMAT_444) {
                m_bih_out.biCompression = MMP_FOURCC_IMAGE_YUV444Packed; 
            }
            else {
                m_bih_out.biCompression = MMP_FOURCC_IMAGE_YUV444P3; 
            }
            break;

        case JPU_FORMAT_422:
        case JPU_FORMAT_224:
        case JPU_FORMAT_400:
        default:
            MMPDEBUGMSG(1, (TEXT("[%s::%s]  FAIL: Not Support Frame Format jpu_format=%d "),  MMP_CLASS_NAME, MMP_CLASS_FUNC, framebufFormat));
            return MMP_FAILURE;
            break;
    }
    
        
    m_p_buf_imageframe_yuv = mmp_buffer_mgr::get_instance()->alloc_media_imageframe(m_bih_out.biWidth, m_bih_out.biHeight, (enum MMP_FOURCC)m_bih_out.biCompression);
    
    JPU_FrameBuffer user_frame[4];
    for(i = 0; i < 1; i++) {

        memset(&user_frame[i], 0x00, sizeof(JPU_FrameBuffer));
        user_frame[i].bufY = m_p_buf_imageframe_yuv->get_buf_phy_addr(MMP_MEDIASAMPLE_BUF_Y);
        user_frame[i].bufCb = m_p_buf_imageframe_yuv->get_buf_phy_addr(MMP_MEDIASAMPLE_BUF_CB);
        user_frame[i].bufCr = m_p_buf_imageframe_yuv->get_buf_phy_addr(MMP_MEDIASAMPLE_BUF_CR);
        user_frame[i].stride = m_p_buf_imageframe_yuv->get_stride();
        //user_frame[i].height = framebufHeight;
        //user_frame[i].ion_shared_fd = buf_addr.m_shared_fd;
        //user_frame[i].ion_base_phyaddr = buf_addr.m_phy_addr;
        //user_frame[i].myIndex = i;
    
    }

    this->m_p_jpu_if->JPU_DecRegisterFrameBuffer(handle, user_frame, regFrameBufCount, m_p_buf_imageframe_yuv->get_stride());
    
MMPDEBUGTRACE;
    ret = this->m_p_jpu_if->JPU_DecStartOneFrame(handle, &decParam);
    if (ret != JPG_RET_SUCCESS && ret != JPG_RET_EOS)
    {
	    mmpResult = MMP_FAILURE;	
    }

MMPDEBUGTRACE;
    while(1) {
        int_reason = this->m_p_jpu_if->JPU_WaitInterrupt(JPU_INTERRUPT_TIMEOUT_MS);

MMPDEBUGTRACE;
        if (int_reason & (1<<INT_JPU_DONE) || int_reason & (1<<INT_JPU_ERROR))	// Must catch PIC_DONE interrupt before catching EMPTY interrupt
		{
			// Do no clear INT_JPU_DONE and INT_JPU_ERROR interrupt. these will be cleared in JPU_DecGetOutputInfo.
MMPDEBUGTRACE;
			break;			
		}
    }
MMPDEBUGTRACE;
    ret = JPU_DecGetOutputInfo(handle, &outputInfo);

    MMPDEBUGMSG(1, (TEXT("[%s::%s]  outputInfo.decodingSuccess=%d"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, 
              outputInfo.decodingSuccess, 
              outputInfo.indexFrameDisplay
              ));
	

    //this->m_p_jpu_if->JPU_DecGiveCommand(handle, SET_JPG_SCALE_HOR,  &(decConfig.iHorScaleMode));
    //this->m_p_jpu_if->JPU_DecGiveCommand(handle, SET_JPG_SCALE_VER,  &(decConfig.iVerScaleMode));


    //// Register frame buffers requested by the decoder.
	//ret = JPU_DecRegisterFrameBuffer(handle, frameBuf, regFrameBufCount, framebufStride);
	//if( ret != JPG_RET_SUCCESS )
	//{
		//JLOG(ERR, "JPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
		//goto ERR_DEC_OPEN;
	//}
	ppIdx = 0;

    this->m_p_jpu_if->JPU_DecClose(handle);

    if(pp_buf_imageframe != NULL) {
        *pp_buf_imageframe = m_p_buf_imageframe_yuv;
    }

    return MMP_SUCCESS;
}




