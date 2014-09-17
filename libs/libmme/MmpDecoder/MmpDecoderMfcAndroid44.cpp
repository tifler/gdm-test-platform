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

#include "MmpDecoderMfcAndroid44.hpp"
#include "../MmpComm/MmpUtil.hpp"
#include "MmpH264Tool.hpp"

#if 1//(MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC_ANDROID44)

extern "C"  {
#include "color_space_convertor.h"
}



#define MAX_VIDEO_INPUTBUFFER_NUM    5
#define MAX_VIDEO_OUTPUTBUFFER_NUM   2

#define DEFAULT_FRAME_WIDTH          176
#define DEFAULT_FRAME_HEIGHT         144

#define DEFAULT_VIDEO_INPUT_BUFFER_SIZE    (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT) * 2
#define DEFAULT_VIDEO_OUTPUT_BUFFER_SIZE   (DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3) / 2

#define MFC_INPUT_BUFFER_NUM_MAX            2
#define DEFAULT_MFC_INPUT_BUFFER_SIZE    1024 * 1024 * MFC_INPUT_BUFFER_NUM_MAX    /*DEFAULT_VIDEO_INPUT_BUFFER_SIZE*/

#define INPUT_PORT_SUPPORTFORMAT_NUM_MAX    1
#define OUTPUT_PORT_SUPPORTFORMAT_NUM_MAX   4



#define MPEG4_DEC_NUM_OF_EXTRA_BUFFERS 7

/////////////////////////////////////////////////////////////
//CMmpDecoderMfcMember Functions

CMmpDecoderMfc::CMmpDecoderMfc(struct MmpDecoderCreateConfig *pCreateConfig) :
m_hMFCHandle(NULL)
,m_bDecodeDSI(MMP_FALSE)
,m_csc_set_format(MMP_FALSE)

,m_nDecFrameCount(0)
,m_nDecFrameCount_Before(0)
,m_mmp_task_hdl(NULL)
,m_sema_dec_start(NULL)
,m_sema_dec_end(NULL)

,m_extra_data(NULL)
,m_extra_data_size(0)

,m_p_hwsync_mutex((class mmp_oal_mutex*)pCreateConfig->hw_sync_mutex_hdl)
{
    int i;

    for(i = 0; i < MMPDECODER_MFC_STREAMBUF_COUNT; i++) {
        m_pStreamBuffer[i] = NULL;
        m_pStreamPhyBuffer[i] = NULL;
    }

    switch(pCreateConfig->nFormat) {
    
        /* Audio */
        //case MMP_WAVE_FORMAT_MPEGLAYER3: m_AVCodecID = AV_CODEC_ID_MP3; break;
        //case MMP_WAVE_FORMAT_WMA2: m_AVCodecID = AV_CODEC_ID_WMAV2; break;

        /* Video */
        case MMP_FOURCC_VIDEO_H263: m_MFCCodecType=H263_DEC; break;
        case MMP_FOURCC_VIDEO_H264: m_MFCCodecType=H264_DEC; break;
        case MMP_FOURCC_VIDEO_MPEG4: m_MFCCodecType=MPEG4_DEC; break;
        case MMP_FOURCC_VIDEO_MPEG2: m_MFCCodecType=MPEG2_DEC; break;
        //case MMP_FOURCC_VIDEO_VC1: m_MFCCodecType=VC1_DEC; break;
        case MMP_FOURCC_VIDEO_VC1: m_MFCCodecType=VC1RCV_DEC; break;
        //case MMP_FOURCC_VIDEO_WMV1: m_MFCCodecType=VC1_DEC; break;
        //case MMP_FOURCC_VIDEO_WMV2: m_MFCCodecType=VC1_DEC; break;
        //case MMP_FOURCC_VIDEO_WMV3: m_MFCCodecType=VC1_DEC; break;
        
        default:  m_MFCCodecType = UNKNOWN_TYPE;
    }

    m_MFCImgResol.width = 0;
    m_MFCImgResol.height = 0;
    m_MFCImgResol.buf_width = 0;
    m_MFCImgResol.buf_height = 0;
    
}

CMmpDecoderMfc::~CMmpDecoderMfc()
{
    if(m_extra_data!=NULL) {
        delete [] m_extra_data;
    }

}

MMP_RESULT CMmpDecoderMfc::Open()
{
    int i;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    

    if(mmpResult == MMP_SUCCESS) {
        /* MFC(Multi Format Codec) decoder and CMM(Codec Memory Management) driver open */
        m_hMFCHandle = SsbSipMfcDecOpen();
        if(m_hMFCHandle == NULL) {

            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderMfc::Open] FAIL: SsbSipMfcDecOpen \n\r")));
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        /* Allocate decoder's input buffer */
        /* Get first input buffer */
        m_nNextStreamBufferIndex = 0;
        m_nDecStreamBufferIndex = 0;
        for(i = 0; i < MMPDECODER_MFC_STREAMBUF_COUNT; i++) {
            m_pStreamBuffer[i] = SsbSipMfcDecGetInBuf(m_hMFCHandle, &m_pStreamPhyBuffer[i], DEFAULT_MFC_INPUT_BUFFER_SIZE/2);
            if(m_pStreamBuffer[i] == NULL) {
                mmpResult = MMP_ErrorInsufficientResources;
                break;
            }
        }

        if(i != MMPDECODER_MFC_STREAMBUF_COUNT) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderMfc::Open] FAIL: SsbSipMfcDecGetInBuf i=%d \n\r"), i));
            mmpResult = MMP_ErrorInsufficientResources;
        }
    }

    if(mmpResult == MMP_SUCCESS) {

        m_sema_dec_start = CMmpOAL::GetSemaphoreInstance()->Create(NULL, MMPOAL_SEMAPHORE_BINARY, 0); //Start on Block State
        m_sema_dec_end = CMmpOAL::GetSemaphoreInstance()->Create(NULL, MMPOAL_SEMAPHORE_BINARY, 1); //Start on NonBlock State

        if( (m_sema_dec_start != NULL) && (m_sema_dec_end != NULL) ) {

            m_bDecServiceRun = MMP_TRUE;
            m_mmp_task_hdl = CMmpOAL::GetTaskInstance()->Create(CMmpDecoderMfc::Service_DecodeNonBlockStub, this, 1024*8, 1, NULL, 1);
            if(m_mmp_task_hdl == NULL) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderMfc::Open] FAIL: Create Task \n\r")));
            }
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderMfc::Open] FAIL: Create Sema (start=0x%08x end=0x%08x) \n\r"), (unsigned int)m_sema_dec_start, (unsigned int)m_sema_dec_end ));
        }
    }

    return mmpResult;
}


MMP_RESULT CMmpDecoderMfc::Close()
{
    if(m_mmp_task_hdl != NULL) {

        /* Start Decoding.. */
        m_bDecServiceRun = MMP_FALSE;
        CMmpOAL::GetSemaphoreInstance()->Release(m_sema_dec_start);
    
        /* Task Exit */
        CMmpOAL::GetTaskInstance()->Destroy(m_mmp_task_hdl);
        m_mmp_task_hdl = NULL;
    }

    if(m_sema_dec_start != NULL) {
        CMmpOAL::GetSemaphoreInstance()->Destroy(m_sema_dec_start);
        m_sema_dec_start = NULL;
    }
    
    if(m_sema_dec_end != NULL) {
        CMmpOAL::GetSemaphoreInstance()->Destroy(m_sema_dec_end);
        m_sema_dec_end = NULL;
    }

    if(m_hMFCHandle != NULL) {
        SsbSipMfcDecClose(m_hMFCHandle);
        m_hMFCHandle = NULL;
    }

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderMfc::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize, MMP_U8* szCodecName) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 configValue;
    MMP_U32 oneFrameSize;
    SSBSIP_MFC_ERROR_CODE mfcResult = MFC_RET_OK;
    struct mmp_ffmpeg_packet_header *p_ffmpeg_packet_header;
    MMP_U32 key, psz1, psz2, i;
    AVCodecContext *cc1;
    unsigned char *pau;
    
    p_ffmpeg_packet_header = (struct mmp_ffmpeg_packet_header *)pStream;
    if(p_ffmpeg_packet_header != NULL) {
    key = p_ffmpeg_packet_header->key;
    psz1 = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size+p_ffmpeg_packet_header->extra_data_size; //Packet Size
    psz2 = p_ffmpeg_packet_header->packet_size;
    }
    else {
        key = 0;
    }


    if((key == MMP_FFMPEG_PACKET_HEADER_KEY) && (psz1 == psz2) ) {
        
        switch(p_ffmpeg_packet_header->payload_type) {
        
            case MMP_FFMPEG_PACKET_TYPE_AVCodecContext:
                i = p_ffmpeg_packet_header->hdr_size;
                cc1 = (AVCodecContext *)&pStream[i];
                
                i = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size;

                m_extra_data = new MMP_U8[p_ffmpeg_packet_header->extra_data_size];
                memcpy(m_extra_data, &pStream[i], p_ffmpeg_packet_header->extra_data_size);
                m_extra_data_size = p_ffmpeg_packet_header->extra_data_size;
                //cc->extradata = m_extra_data;
                //cc->extradata_size = p_ffmpeg_packet_header->extra_data_size;

                switch((MMP_U32)cc1->codec_id) {
                    case AV_CODEC_ID_WMV3:  m_MFCCodecType = VC1RCV_DEC; break;
                    case AV_CODEC_ID_VC1:  m_MFCCodecType = VC1_DEC; break;
                }
                
                if(m_MFCCodecType == VC1RCV_DEC){

                    m_extra_data_size = 4;
#if 0
                    m_extra_data[0]=0x4e;
                    m_extra_data[1]=0x79;
                    m_extra_data[2]=0x1a;
                    m_extra_data[3]=0x01;
#else

                    //HwnaChWon Success 4e 79 1a 01
                    //WMV..     FAIL    8f f8 0a 01 40 2f

                    /*
                    m_extra_data[0]=0x4f;
                    m_extra_data[1]=0xf9;
                    m_extra_data[2]=0x0a;
                    m_extra_data[3]=0x01;
                    */

                    //삼성 MFC는 이유는 잘 모르겠지만.. 이 Flag가 켜져야 한다.
                    //enable res_fasttx , ref ffmpeg vc1.c if (!v->res_fasttx)

                    m_extra_data[1]|=0x01; 

#endif


                    this->Make_WMV_Stream_MetaData(m_MFCCodecType, cc1->width, cc1->height, m_extra_data, m_extra_data_size, (MMP_U8*)m_pStreamBuffer[0], &oneFrameSize);

                    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderMfc::DecodeDSI] ln=%d oneFrameSize=%d wh(%d %d) ext %d  (%x %x %x %x %x %x )"), 
                                  __LINE__,  
                                  oneFrameSize,
                                  cc1->width, cc1->height,
                                  m_extra_data_size, m_extra_data[0], m_extra_data[1], m_extra_data[2], m_extra_data[3], m_extra_data[4], m_extra_data[5] , m_extra_data[6] 
                                  )); 

#if (MMP_OS == MMP_OS_WIN32)
                    memcpy(m_pStreamBuffer[0], pStream, nStreamSize);
                    oneFrameSize = nStreamSize; 
#endif

                }
                else {
                    memcpy(m_pStreamBuffer[0], pStream, nStreamSize);
                    oneFrameSize = nStreamSize; 
                }
                break;
        
            default:
                memcpy(m_pStreamBuffer[0], pStream, nStreamSize);
                oneFrameSize = nStreamSize; 
                break;
        }
    }
    else {

        memcpy(m_pStreamBuffer[0], pStream, nStreamSize);
        oneFrameSize = nStreamSize; 

#if 1//(MMP_OS != MMP_OS_WIN32)
        if(m_MFCCodecType == H264_DEC) {
            CMmpH264Parser::Remake_VideoDSI_AVC1((unsigned char*)m_pStreamBuffer[0], nStreamSize, (int*)&oneFrameSize,
                                                 NULL, NULL, NULL, NULL);
                                                 
        }
#endif
        
    }

    m_nStreamBufferSize[0] = oneFrameSize;
    

    /* Set mpeg4 deblocking filter enable */
    switch((MMP_U32)m_MFCCodecType) {
        
        case MPEG4_DEC:
            configValue = 1;
            SsbSipMfcDecSetConfig(m_hMFCHandle, MFC_DEC_SETCONF_POST_ENABLE, &configValue);

            pau = (unsigned char *)m_pStreamBuffer[0];
            if( (pau[0] == 0x00) && (pau[1] == 0x00) && (pau[2] == 0x01) && (pau[3] != 0xb6) ) {
                /* Nothing to do */
            }
            else {
                mmpResult = MMP_FAILURE;
            }
            break;

        case H264_DEC:
            configValue = 1;
            SsbSipMfcDecSetConfig(m_hMFCHandle, MFC_DEC_SETCONF_POST_ENABLE, &configValue);
            break;
    }

    if(0) {// (pVideoDec->bThumbnailMode == MMP_TRUE) {
        configValue = 0;    // the number that you want to delay
        SsbSipMfcDecSetConfig(m_hMFCHandle, MFC_DEC_SETCONF_DISPLAY_DELAY, &configValue);
    } else {
        configValue = MPEG4_DEC_NUM_OF_EXTRA_BUFFERS;
        SsbSipMfcDecSetConfig(m_hMFCHandle, MFC_DEC_SETCONF_EXTRA_BUFFER_NUM, &configValue);
    }

    switch((MMP_U32)m_MFCCodecType) {
        
        case MPEG4_DEC:  strcpy((char*)szCodecName, "Mpeg4"); break;
        case H264_DEC:   strcpy((char*)szCodecName, "H264"); break;
        case VC1RCV_DEC: strcpy((char*)szCodecName, "WMV3"); break;
        case VC1_DEC:    strcpy((char*)szCodecName, "VC1"); break;
     }

    pau = (unsigned char *)m_pStreamBuffer[0];
    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderMfc::DecodeDSI] ln=%d res=%d SsbSipMfcDecInit %s  sz=%d au=(%02x %02x %02x %02x %02x %02x %02x %02x )"), 
                                __LINE__, mmpResult, szCodecName,
                                m_nStreamBufferSize[0], 
                                pau[0], pau[1], pau[2], pau[3], 
                                pau[4], pau[5], pau[6], pau[7] 
                                )); 



    if(mmpResult == MMP_SUCCESS) {

        SsbSipMfcDecSetInBuf(m_hMFCHandle,
                             m_pStreamPhyBuffer[0],
                             m_pStreamBuffer[0],
                             m_nStreamBufferSize[0]);
        mfcResult = SsbSipMfcDecInit(m_hMFCHandle, m_MFCCodecType, oneFrameSize);
        if(mfcResult == MFC_RET_OK) {
        
            mfcResult =SsbSipMfcDecGetConfig(m_hMFCHandle, MFC_DEC_GETCONF_BUF_WIDTH_HEIGHT, &m_MFCImgResol);
            if(mfcResult == MFC_RET_OK) {

            }
            else {
                mmpResult = MMP_FAILURE;
            }
        }
        else {
            pau = (unsigned char *)m_pStreamBuffer[0];
            MMPDEBUGMSG(0, (TEXT("[CMmpDecoderMfc::DecodeDSI] ln=%d FAIL: SsbSipMfcDecInit %s  sz=%d au=(%02x %02x %02x %02x %02x %02x %02x %02x )"), 
                                __LINE__, szCodecName,
                                m_nStreamBufferSize[0], 
                                pau[0], pau[1], pau[2], pau[3], 
                                pau[4], pau[5], pau[6], pau[7] 
                                )); 

             mmpResult = MMP_FAILURE;
        }

    }

    if(mmpResult == MMP_SUCCESS) {
        m_bDecodeDSI = MMP_TRUE;
    }
        
    return mmpResult;
}

MMP_RESULT CMmpDecoderMfc::DecodeAu_Block(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) {

#if 1
    return MMP_FAILURE;

#else
    MMP_RESULT mmpResult = MMP_SUCCESS;
    SSBSIP_MFC_ERROR_CODE mfcResult = MFC_RET_OK;
    SSBSIP_MFC_DEC_OUTBUF_STATUS mfcDecStatus;
    SSBSIP_MFC_DEC_OUTPUT_INFO outputInfo;
    unsigned char  *pSrcBuf[3], *pYUVBuf[3];
    SSBSIP_MFC_IMG_RESOLUTION MFCImgResol;

    pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    pDecResult->uiDecodedDuration = 0;

    memcpy(m_pStreamBuffer, pMediaSample->pAu, pMediaSample->uiAuSize);
    mfcResult = SsbSipMfcDecExe(m_hMFCHandle, pMediaSample->uiAuSize);
    pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;

    if(mfcResult == MFC_RET_OK) {

        mfcDecStatus = SsbSipMfcDecGetOutBuf(m_hMFCHandle, &outputInfo);
        if( (mfcDecStatus == MFC_GETOUTBUF_DISPLAY_DECODING) 
            || (mfcDecStatus == MFC_GETOUTBUF_DISPLAY_ONLY) ) {

            mfcResult =SsbSipMfcDecGetConfig(m_hMFCHandle, MFC_DEC_GETCONF_BUF_WIDTH_HEIGHT, &MFCImgResol);
            if(mfcResult == MFC_RET_OK) {
                m_MFCImgResol = MFCImgResol;
            }

            unsigned int csc_src_color_format = HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED;//omx_2_hal_pixel_format((unsigned int)MMP_SEC_COLOR_FormatNV12Tiled);
            unsigned int csc_dst_color_format = HAL_PIXEL_FORMAT_YCbCr_420_P;//omx_2_hal_pixel_format((unsigned int)MMP_COLOR_FormatYUV420Planar);
            unsigned int cacheable = 1;
            
            if(this->m_csc_set_format == MMP_FALSE) {
            
                   this->m_csc_set_format = MMP_TRUE;
            }
            


            if( MMP_YV12_FRAME_SIZE(m_MFCImgResol.width, m_MFCImgResol.height) <= pDecResult->uiDecodedBufferMaxSize) {
            

                pDecResult->uiDecodedSize = MMP_YV12_FRAME_SIZE(m_MFCImgResol.width, m_MFCImgResol.height); //(m_MFCImgResol.width*m_MFCImgResol.height*3)/2;
                pDecResult->bImage = MMP_TRUE;
            }

        }
    }

    return mmpResult;
#endif
}

void CMmpDecoderMfc::Service_DecodeNonBlockStub(void* parm) {
    CMmpDecoderMfc* p_obj = (CMmpDecoderMfc*)parm;
    p_obj->Service_DecodeNonBlock();
}

void CMmpDecoderMfc::Service_DecodeNonBlock() {
    
    SSBSIP_MFC_ERROR_CODE mfcResult = MFC_RET_OK;

    while(m_bDecServiceRun) {
    
        CMmpOAL::GetSemaphoreInstance()->Obtain(m_sema_dec_start); //Wait Start Semaphore
        if(m_bDecServiceRun != MMP_TRUE) {
            break;
        }

        mfcResult = SsbSipMfcDecExe(m_hMFCHandle, m_nStreamBufferSize[m_nDecStreamBufferIndex] );
        if(mfcResult == MFC_RET_OK) {
            m_nDecFrameCount++;
        }
        
        CMmpOAL::GetSemaphoreInstance()->Release(m_sema_dec_end);
    }
}

#if (MMP_OS == MMP_OS_WIN32)
static int s_pic_width = 0;
static int s_pic_height = 0;
#endif

MMP_RESULT CMmpDecoderMfc::DecodeAu_NonBlock(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    SSBSIP_MFC_ERROR_CODE mfcResult = MFC_RET_OK;
    SSBSIP_MFC_DEC_OUTBUF_STATUS mfcDecStatus;
    SSBSIP_MFC_DEC_OUTPUT_INFO outputInfo;
    unsigned char  *pSrcBuf[3], *pYUVBuf[3];
    SSBSIP_MFC_IMG_RESOLUTION MFCImgResol;
    MMP_BOOL bRunColorConversion = MMP_FALSE;
    MMP_U32 timestamp, out_timestamp;

    pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;
    pDecResult->uiAuUsedByte = 0;
    pDecResult->uiAudioSampleRate = 0;
    pDecResult->uiAudioFrameCount = 0;
    pDecResult->uiDecodedDuration = 0;

    CMmpOAL::GetSemaphoreInstance()->Obtain(m_sema_dec_end); //Wait for finishing decoding

    if(m_nDecFrameCount > m_nDecFrameCount_Before) { 
        
        mfcDecStatus = SsbSipMfcDecGetOutBuf(m_hMFCHandle, &outputInfo);
        if( (mfcDecStatus == MFC_GETOUTBUF_DISPLAY_DECODING) || (mfcDecStatus == MFC_GETOUTBUF_DISPLAY_ONLY) ) {
           
                bRunColorConversion = MMP_TRUE;

                mfcResult =SsbSipMfcDecGetConfig(m_hMFCHandle, MFC_DEC_GETCONF_BUF_WIDTH_HEIGHT, &MFCImgResol);
                if(mfcResult == MFC_RET_OK) {
                    m_MFCImgResol = MFCImgResol;
                }
                SsbSipMfcDecGetConfig(m_hMFCHandle, MFC_DEC_GETCONF_FRAME_TAG, (void*)&out_timestamp);

     
                pSrcBuf[0] = (unsigned char*)outputInfo.YVirAddr;
                pSrcBuf[1] = (unsigned char*)outputInfo.CVirAddr;
                pSrcBuf[2] = NULL;

                pYUVBuf[0] = (unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y];
                pYUVBuf[1] = (unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U];
                pYUVBuf[2] = (unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V];

        }
    }

    /* Set Stream Buffer */
    m_nDecStreamBufferIndex = m_nNextStreamBufferIndex;
    m_nStreamBufferSize[m_nDecStreamBufferIndex] = pMediaSample->uiAuSize;
    memcpy(m_pStreamBuffer[m_nDecStreamBufferIndex], pMediaSample->pAu, m_nStreamBufferSize[m_nDecStreamBufferIndex]);

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderMfc::DecodeAu_NonBlock] sz=%d stream(%02x %02x %02x %02x %02x %02x %02x %02x )"),
                                pMediaSample->uiAuSize,
                                pMediaSample->pAu[0], pMediaSample->pAu[1], pMediaSample->pAu[2], pMediaSample->pAu[3], 
                                pMediaSample->pAu[4], pMediaSample->pAu[5], pMediaSample->pAu[6], pMediaSample->pAu[7] ));
                                
                      

#if 1//(MMP_OS != MMP_OS_WIN32)
    if(m_MFCCodecType == H264_DEC) {
        unsigned int* ps = (unsigned int*)m_pStreamBuffer[m_nDecStreamBufferIndex];
        if( *ps != 0x01000000) {
            *ps = 0x01000000;
        }
    }
#endif

    timestamp = (MMP_U32)(pMediaSample->uiTimeStamp/1000);
    SsbSipMfcDecSetConfig(m_hMFCHandle, MFC_DEC_SETCONF_FRAME_TAG, (void*)&timestamp);
    SsbSipMfcDecSetInBuf(m_hMFCHandle,
                         m_pStreamPhyBuffer[m_nDecStreamBufferIndex],
                         m_pStreamBuffer[m_nDecStreamBufferIndex],
                         m_nStreamBufferSize[m_nDecStreamBufferIndex]);
    m_nNextStreamBufferIndex = (m_nNextStreamBufferIndex+1)%MMPDECODER_MFC_STREAMBUF_COUNT;

    /* Start Decoding.. */
    CMmpOAL::GetSemaphoreInstance()->Release(m_sema_dec_start);
    pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;

    /* Start Color Conversion */
    if(bRunColorConversion == MMP_TRUE) {
    
        //unsigned int csc_src_color_format = HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED;//omx_2_hal_pixel_format((unsigned int)MMP_SEC_COLOR_FormatNV12Tiled);
        //unsigned int csc_dst_color_format = HAL_PIXEL_FORMAT_YCbCr_420_P;//omx_2_hal_pixel_format((unsigned int)MMP_COLOR_FormatYUV420Planar);
        unsigned int cacheable = 1;
        
        
        if( MMP_YV12_FRAME_SIZE(m_MFCImgResol.width, m_MFCImgResol.height) <= (int)pDecResult->uiDecodedBufferMaxSize) {
            
                int buffer_width, buffer_height;

                buffer_width = MMP_BYTE_ALIGN(m_MFCImgResol.width, 16);
                buffer_height = MMP_BYTE_ALIGN(m_MFCImgResol.height, 16);

#if (MMP_OS == MMP_OS_WIN32)
                s_pic_width = m_MFCImgResol.width;
                s_pic_height = m_MFCImgResol.height;
#endif

                csc_tiled_to_linear_y_neon(
                    (unsigned char *)pYUVBuf[0],
                    (unsigned char *)pSrcBuf[0], //outputInfo.YVirAddr,
                    buffer_width,
                    buffer_height);

                csc_tiled_to_linear_uv_deinterleave_neon(
                    (unsigned char *)pYUVBuf[1],
                    (unsigned char *)pYUVBuf[2],
                    (unsigned char *)pSrcBuf[1], //outputInfo.CVirAddr,
                    buffer_width,
                    buffer_height / 2);

                pDecResult->uiDecodedSize = MMP_YV12_FRAME_SIZE(m_MFCImgResol.width, m_MFCImgResol.height); //(m_MFCImgResol.width*m_MFCImgResol.height*3)/2;
                pDecResult->bImage = MMP_TRUE;
                pDecResult->uiTimeStamp = (MMP_S64)(out_timestamp)*1000LL;

            }

    }
  


    return mmpResult;
}

MMP_RESULT CMmpDecoderMfc::Make_WMV_Stream_MetaData(MMP_IN SSBSIP_MFC_CODEC_TYPE mfcCodeingType, 
                                        MMP_IN MMP_U32 width, MMP_IN MMP_U32 height, 
                                        MMP_IN MMP_U8* seq_header, MMP_IN MMP_U32 seq_hdr_size,
                                        MMP_OUT MMP_U8 *pMetaData, MMP_OUT MMP_U32 *pMetaDataSize ) {

    MMP_RESULT mmpResult = MMP_FAILURE;

    MMP_U8  *pCurrBuf;
    MMP_U32 currPos;

    /* Sequence Layer Data Structure */
    MMP_U8 const_C5[4] = {0x00, 0x00, 0x00, 0xc5};
    MMP_U8 const_04[4] = {0x04, 0x00, 0x00, 0x00};
    MMP_U8 const_0C[4] = {0x0C, 0x00, 0x00, 0x00};
    MMP_U8 struct_B_1[4] = {0xB3, 0x19, 0x00, 0x00};
    MMP_U8 struct_B_2[4] = {0x44, 0x62, 0x05, 0x00};
    MMP_U8 struct_B_3[4] = {0x0F, 0x00, 0x00, 0x00};
    MMP_U8 struct_C[8] = {0x30, 0x00, 0x00, 0x00};

    switch (mfcCodeingType) {

        case VC1RCV_DEC : //WMV_FORMAT_WMV3:
              
            if(seq_hdr_size >= 4) {

                pCurrBuf = pMetaData;
                currPos = 0;

                memcpy(struct_C, seq_header, 4);
                if(0)// (struct_C[0]&0x80) !=0) //Profile Complex
                {
                    struct_C[0]&=0x3F;
                    struct_C[0]|=0x40;
                }


                memcpy(pCurrBuf + currPos, const_C5, 4);
                currPos +=4;

                memcpy(pCurrBuf + currPos, const_04, 4);
                currPos +=4;

                memcpy(pCurrBuf + currPos, struct_C, 4);
                currPos +=4;

                /* struct_A : VERT_SIZE */
                pCurrBuf[currPos] =  height & 0xFF;
                pCurrBuf[currPos+1] = (height>>8) & 0xFF;
                pCurrBuf[currPos+2] = (height>>16) & 0xFF;
                pCurrBuf[currPos+3] = (height>>24) & 0xFF;
                currPos +=4;

                /* struct_A : HORIZ_SIZE */
                pCurrBuf[currPos] =  width & 0xFF;
                pCurrBuf[currPos+1] = (width>>8) & 0xFF;
                pCurrBuf[currPos+2] = (width>>16) & 0xFF;
                pCurrBuf[currPos+3] = (width>>24) & 0xFF;
                currPos +=4;

                memcpy(pCurrBuf + currPos,const_0C, 4);
                currPos +=4;

                memcpy(pCurrBuf + currPos, struct_B_1, 4);
                currPos +=4;

                memcpy(pCurrBuf + currPos, struct_B_2, 4);
                currPos +=4;

                memcpy(pCurrBuf + currPos, struct_B_3, 4);
                currPos +=4;

                if(pMetaDataSize) *pMetaDataSize = currPos;

                mmpResult = MMP_SUCCESS;
            }
            break;

        case VC1_DEC ://WMV_FORMAT_VC1:
    #if 0
            if (*pStreamSize >= BITMAPINFOHEADER_ASFBINDING_SIZE) {
                //memcpy(pCurrBuf, pInputStream + BITMAPINFOHEADER_ASFBINDING_SIZE, *pStreamSize - BITMAPINFOHEADER_ASFBINDING_SIZE);
                *pStreamSize -= BITMAPINFOHEADER_ASFBINDING_SIZE;
                return MMP_TRUE;
            } else {
                //SEC_OSAL_Log(SEC_LOG_ERROR, "%s: *pStreamSize is too small to contain metadata(%d)", __FUNCTION__, *pStreamSize);
                return MMP_FALSE;
            }
    #endif
            break;
        default:
            //SEC_OSAL_Log(SEC_LOG_WARNING, "%s: It is not necessary to make bitstream metadata for wmvFormat (%d)", __FUNCTION__, wmvFormat);
            break;
    }

    return mmpResult;
}

MMP_BOOL CMmpDecoderMfc::Make_WMV_Stream_MetaData(MMP_U8 *pInputStream, MMP_U32 *pStreamSize, SSBSIP_MFC_CODEC_TYPE mfcCodeingType)
{
    MMP_U32 width, height;
    MMP_U8  *pCurrBuf = pInputStream;
    MMP_U32 currPos = 0;


    /* Sequence Layer Data Structure */
    MMP_U8 const_C5[4] = {0x00, 0x00, 0x00, 0xc5};
    MMP_U8 const_04[4] = {0x04, 0x00, 0x00, 0x00};
    MMP_U8 const_0C[4] = {0x0C, 0x00, 0x00, 0x00};
    MMP_U8 struct_B_1[4] = {0xB3, 0x19, 0x00, 0x00};
    MMP_U8 struct_B_2[4] = {0x44, 0x62, 0x05, 0x00};
    MMP_U8 struct_B_3[4] = {0x0F, 0x00, 0x00, 0x00};
    MMP_U8 struct_C[4] = {0x30, 0x00, 0x00, 0x00};

    switch (mfcCodeingType) {

    case VC1RCV_DEC: //WMV_FORMAT_WMV3:
        if (*pStreamSize >= sizeof(MMPBITMAPINFOHEADER)) {
            MMPBITMAPINFOHEADER *pBitmapInfoHeader;
            pBitmapInfoHeader = (MMPBITMAPINFOHEADER *)pInputStream;

            width = pBitmapInfoHeader->biWidth;
            height = pBitmapInfoHeader->biHeight;
            if (*pStreamSize > sizeof(MMPBITMAPINFOHEADER))
                memcpy(struct_C, pInputStream+sizeof(MMPBITMAPINFOHEADER), 4);

        memcpy(pCurrBuf + currPos, const_C5, 4);
        currPos +=4;

        memcpy(pCurrBuf + currPos, const_04, 4);
        currPos +=4;

        memcpy(pCurrBuf + currPos, struct_C, 4);
        currPos +=4;

        /* struct_A : VERT_SIZE */
        pCurrBuf[currPos] =  height & 0xFF;
        pCurrBuf[currPos+1] = (height>>8) & 0xFF;
        pCurrBuf[currPos+2] = (height>>16) & 0xFF;
        pCurrBuf[currPos+3] = (height>>24) & 0xFF;
        currPos +=4;

        /* struct_A : HORIZ_SIZE */
        pCurrBuf[currPos] =  width & 0xFF;
        pCurrBuf[currPos+1] = (width>>8) & 0xFF;
        pCurrBuf[currPos+2] = (width>>16) & 0xFF;
        pCurrBuf[currPos+3] = (width>>24) & 0xFF;
        currPos +=4;

        memcpy(pCurrBuf + currPos,const_0C, 4);
        currPos +=4;

        memcpy(pCurrBuf + currPos, struct_B_1, 4);
        currPos +=4;

        memcpy(pCurrBuf + currPos, struct_B_2, 4);
        currPos +=4;

        memcpy(pCurrBuf + currPos, struct_B_3, 4);
        currPos +=4;

        *pStreamSize = currPos;
        return MMP_TRUE;
        } else {
            //SEC_OSAL_Log(SEC_LOG_ERROR, "%s: *pStreamSize is too small to contain metadata(%d)", __FUNCTION__, *pStreamSize);
            return MMP_FALSE;
        }
        break;
    
    case VC1_DEC ://WMV_FORMAT_VC1:
#if 0    
        if (*pStreamSize >= BITMAPINFOHEADER_ASFBINDING_SIZE) {
            //memcpy(pCurrBuf, pInputStream + BITMAPINFOHEADER_ASFBINDING_SIZE, *pStreamSize - BITMAPINFOHEADER_ASFBINDING_SIZE);
            *pStreamSize -= BITMAPINFOHEADER_ASFBINDING_SIZE;
            return MMP_TRUE;
        } else {
            //SEC_OSAL_Log(SEC_LOG_ERROR, "%s: *pStreamSize is too small to contain metadata(%d)", __FUNCTION__, *pStreamSize);
            return MMP_FALSE;
        }
#else
        return MMP_FALSE;
#endif
        break;
    default:
        //SEC_OSAL_Log(SEC_LOG_WARNING, "%s: It is not necessary to make bitstream metadata for wmvFormat (%d)", __FUNCTION__, wmvFormat);
        return MMP_FALSE;
        break;
    }
}


#ifdef WIN32

unsigned char *s_y_dst = NULL;
void csc_tiled_to_linear_y_neon(
    unsigned char *y_dst,
    unsigned char *y_src,
    unsigned int width,
    unsigned int height) {

#if 0
    AVPicture* pFrameSrc;
    AVPicture pic;
    AVPicture* pFrameDest;
    
    pFrameSrc = (AVPicture *)y_src;
    pFrameDest = &pic;

    memset(pFrameDest, 0x00, sizeof(AVPicture));
    pFrameDest->data[0] = (uint8_t*)y_dst;
    pFrameDest->data[1] = (uint8_t*)pFrameDest->data[0] + width*height;
    pFrameDest->data[2] = (uint8_t*)pFrameDest->data[1] + width*height/4;
    pFrameDest->linesize[0] = width;
    pFrameDest->linesize[1] = width>>1;
    pFrameDest->linesize[2] = width>>1;
    
    
    //avpicture_fill((AVPicture *)pFrameDest, output, PIX_FMT_YUV420P, csc_handle->dst_format.width, csc_handle->dst_format.height);
    av_picture_copy ((AVPicture *)pFrameDest, (AVPicture*)pFrameSrc, PIX_FMT_YUV420P, width, height);
    
    //av_free(pFrameDest);
#else
    s_y_dst = y_dst;
#endif
}

void csc_tiled_to_linear_uv_deinterleave_neon(
    unsigned char *u_dst,
    unsigned char *v_dst,
    unsigned char *uv_src,
    unsigned int width,
    unsigned int height) {

    AVPicture* pFrameSrc;
    AVPicture pic;
    AVPicture* pFrameDest;
    
    pFrameSrc = (AVPicture *)uv_src;
    pFrameDest = &pic;

    memset(pFrameDest, 0x00, sizeof(AVPicture));
    pFrameDest->data[0] = (uint8_t*)s_y_dst;
    pFrameDest->data[1] = (uint8_t*)u_dst;
    pFrameDest->data[2] = (uint8_t*)v_dst;
    pFrameDest->linesize[0] = width;
    pFrameDest->linesize[1] = width>>1;
    pFrameDest->linesize[2] = width>>1;
    
    
    //avpicture_fill((AVPicture *)pFrameDest, output, PIX_FMT_YUV420P, csc_handle->dst_format.width, csc_handle->dst_format.height);
    av_picture_copy ((AVPicture *)pFrameDest, (AVPicture*)pFrameSrc, PIX_FMT_YUV420P, s_pic_width, s_pic_height);
    
    //av_free(pFrameDest);
}


#endif

#endif /* #if (MMP_HWCODEC == MMP_HWCODEC_EXYNOS4_MFC) */