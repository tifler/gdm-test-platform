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

#include "MmpDecoderVideo_Vpu.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Vpu Member Functions

MMP_BOOL CMmpDecoderVideo_Vpu::CheckSupportCodec(MMP_U32 format) {

    MMP_BOOL bflag;

    switch(format) {
        case MMP_FOURCC_VIDEO_H263:
        case MMP_FOURCC_VIDEO_H264:
        case MMP_FOURCC_VIDEO_MPEG4:
        case MMP_FOURCC_VIDEO_MPEG2:
        
        case MMP_FOURCC_VIDEO_VC1:
        case MMP_FOURCC_VIDEO_WMV3:
        
        case MMP_FOURCC_VIDEO_RV30: 
        case MMP_FOURCC_VIDEO_RV40: 
        
        case MMP_FOURCC_VIDEO_MSMPEG4V2:
        case MMP_FOURCC_VIDEO_MSMPEG4V3: /* Divx3 */
        
        case MMP_FOURCC_VIDEO_VP80:
		case MMP_FOURCC_VIDEO_THEORA:
		case MMP_FOURCC_VIDEO_FLV1:
        //case MMP_FOURCC_VIDEO_VP60:
        //case MMP_FOURCC_VIDEO_VP6F:
        
        /*
           VPU support THEORA, but It is difficult to understand NOW,,  
            case MMP_FOURCC_VIDEO_THEORA:
        */

        //case MMP_FOURCC_VIDEO_FLV1:
        
            bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
    }

    return bflag;
}

CMmpDecoderVideo_Vpu::CMmpDecoderVideo_Vpu(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderVideo(pCreateConfig, MMP_FALSE), CLASS_DECODER_VPU(pCreateConfig)
,m_bDecodeDSI(MMP_FALSE)
{
    
}

CMmpDecoderVideo_Vpu::~CMmpDecoderVideo_Vpu()
{

}

MMP_RESULT CMmpDecoderVideo_Vpu::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }
    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));


    mmpResult=CLASS_DECODER_VPU::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Vpu::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Vpu::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CLASS_DECODER_VPU::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Vpu::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Vpu::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Vpu::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Vpu::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

#if 1
    return MMP_FAILURE;
#else
    MMP_RESULT mmpResult;

    mmpResult = CLASS_DECODER_VPU::DecodeDSI(pStream, nStreamSize);
    if(mmpResult == MMP_SUCCESS) {
    
        m_bih_out.biWidth = m_dec_init_info.picWidth;
        m_bih_out.biHeight = m_dec_init_info.picHeight;
	    m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	    m_bih_in.biWidth = m_bih_out.biWidth;
	    m_bih_in.biHeight = m_bih_out.biHeight;
        
        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Vpu::DecodeDSI] Success nForamt=(0x%08x %s)  W=%d H=%d  FrmCnt=%d+1 \n\r"), 
                  m_nFormat, m_szCodecName,
                  m_bih_out.biWidth,
                  m_bih_out.biHeight  ,
                  m_dec_init_info.minFrameBufferCount
                  ));

    }

    return mmpResult;
#endif
}

MMP_RESULT CMmpDecoderVideo_Vpu::DecodeDSI(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult;

    mmpResult = CLASS_DECODER_VPU::DecodeDSI(p_buf_videostream);
    if(mmpResult == MMP_SUCCESS) {
    
        m_bih_out.biWidth = m_dec_init_info.picWidth;
        m_bih_out.biHeight = m_dec_init_info.picHeight;
	    m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);

	    m_bih_in.biWidth = m_bih_out.biWidth;
	    m_bih_in.biHeight = m_bih_out.biHeight;
        
        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Vpu::DecodeDSI] Success nForamt=(0x%08x %s)  W=%d H=%d  FrmCnt=%d+1 \n\r"), 
                  m_nFormat, m_szCodecName,
                  m_bih_out.biWidth,
                  m_bih_out.biHeight  ,
                  m_dec_init_info.minFrameBufferCount
                  ));

    }

    return mmpResult;

}

MMP_RESULT CMmpDecoderVideo_Vpu::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
#if 1

    return MMP_FAILURE;
#else
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 dec_start_tick, dec_end_tick;

    pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    pDecResult->uiDecodedDuration = 0;
    
    if(m_bDecodeDSI == MMP_FALSE) {
        mmpResult = this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
        m_bDecodeDSI = MMP_TRUE;
    }

    if((pMediaSample->uiFlag&MMP_MEDIASAMPMLE_FLAG_CONFIGDATA) != 0) {

        pDecResult->uiDecodedSize = 0;
        pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
        
        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();

     mmpResult = this->DecodeAu_PinEnd(pMediaSample, pDecResult);
    
    
    dec_end_tick = CMmpUtil::GetTickCount();
    pDecResult->uiDecodedDuration = dec_end_tick - dec_start_tick;
    CMmpDecoderVideo::DecodeMonitor(pMediaSample, pDecResult);

	return mmpResult; 
#endif
}

MMP_RESULT CMmpDecoderVideo_Vpu::DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 dec_start_tick, dec_end_tick;
    class mmp_buffer_videoframe* p_buf_videoframe = NULL;

     /* Init Parmeter */
    p_buf_videostream->set_used_byte(0);
	if(pp_buf_videoframe != NULL) {
        *pp_buf_videoframe = NULL;
    }
       
    if(m_bDecodeDSI == MMP_FALSE) {
        mmpResult = this->DecodeDSI(p_buf_videostream);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
        m_bDecodeDSI = MMP_TRUE;
    }

    if((p_buf_videostream->get_flag()&MMP_MEDIASAMPMLE_FLAG_CONFIGDATA) != 0) {

        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();

    //mmpResult = this->DecodeAu_PinEnd(pMediaSample, pDecResult);
    mmpResult = this->DecodeAu_PinEnd(p_buf_videostream, &p_buf_videoframe);
        
    dec_end_tick = CMmpUtil::GetTickCount();

    if(p_buf_videoframe != NULL) {
        p_buf_videoframe->set_coding_dur(dec_end_tick - dec_start_tick);
        CMmpDecoderVideo::DecodeMonitor(p_buf_videoframe);
    }

    if(pp_buf_videoframe != NULL) {
        *pp_buf_videoframe = p_buf_videoframe;
    }

	return mmpResult; 
}

MMP_RESULT CMmpDecoderVideo_Vpu::Play_Function_Tool(MMP_PLAY_FORMAT playformat, MMP_S64 curpos, MMP_S64 totalpos)
{
	MMP_RESULT mmpResult = MMP_SUCCESS; 

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVideo_Vpu] Play_Function_Tool!!!")));
	mmpResult = CLASS_DECODER_VPU::Play_Function_Tool(playformat,curpos,totalpos);
	
	return mmpResult;
}

