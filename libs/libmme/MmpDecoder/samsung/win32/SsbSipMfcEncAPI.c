/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "MmpDefine.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <sys/poll.h>
#include "videodev2.h"

#include "mfc_interface.h"
#include "SsbSipMfcApi.h"

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "MFC_ENC_APP"
#include <utils/Log.h>

#define POLL_ENC_WAIT_TIMEOUT 25

#ifndef true
#define true  (1)
#endif

#ifndef false
#define false (0)
#endif

#define MAX_STREAM_SIZE (2*1024*1024)

static char *mfc_dev_name = SAMSUNG_MFC_DEV_NAME;
static int mfc_dev_node = 7;

#if defined (MFC5x_VERSION)
#define H263_CTRL_NUM   19
#define MPEG4_CTRL_NUM  27
#define H264_CTRL_NUM   50
#elif defined (MFC6x_VERSION)
#define H263_CTRL_NUM   20
#define MPEG4_CTRL_NUM  28
#define H264_CTRL_NUM   67
#endif


#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libavresample/audio_convert.h"

typedef unsigned int MMP_U32;

#define MAX_ENC_INPUT_BUF_COUNT 10
struct mfc_enc {
    enum AVCodecID m_AVCodecID;
    AVCodec *m_pAVCodec;
    AVCodecContext *m_pAVCodecContext;
    AVFrame *m_pAVFrame_Input;
    MMP_U8* m_extra_data;

    
    
    MMP_U8 m_PicBuffer[1920*1080*2];
    
    MMP_U8 m_EncStreamBuffer[1920*1080*2];
    MMP_U32 m_EncStreamSize;
    MMP_U32 m_EncStreamKeyFrame;

    SSBSIP_MFC_CODEC_TYPE m_codecType;
    
    SSBSIP_MFC_ENC_MPEG4_PARAM m_MfcMpeg4Param;
    SSBSIP_MFC_ENC_H264_PARAM m_MfcH264Param;
    SSBSIP_MFC_ENC_H263_PARAM m_MfcH263Param;
    
    SSBSIP_MFC_ENC_INPUT_INFO m_last_input_info;


    /* Inport info */
    MMP_U32 m_nPicWidth;
    MMP_U32 m_nPicHeight;
    MMP_U32 m_nFrameRate;
    
    /* Outport info */
    MMP_U32 m_nIDRPeriod;
    MMP_U32 m_nBitRate;
    
    MMP_U8 *m_pInputBuffer[MAX_ENC_INPUT_BUF_COUNT];
};


static void getMFCName(char *devicename, int size)
{
    snprintf(devicename, size, "%s%d", SAMSUNG_MFC_DEV_NAME, mfc_dev_node);
}

void SsbSipMfcEncSetMFCName(char *devicename)
{
    mfc_dev_name = devicename;
}

void *SsbSipMfcEncOpen(void)
{
    struct mfc_enc *p_mfc_enc;

    avcodec_register_all();
    
    p_mfc_enc = (struct mfc_enc *)malloc(sizeof(struct mfc_enc));
    if(p_mfc_enc != NULL) {
        memset(p_mfc_enc, 0x00, sizeof(struct mfc_enc));
    }

    return (void *)p_mfc_enc;
}

void *SsbSipMfcEncOpenExt(void *value)
{   
    return (void *)NULL;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncClose(void *openHandle)
{
    int i;
    struct mfc_enc *this = (struct mfc_enc *)openHandle;

    if(this != NULL) {

        for(i = 0; i < MAX_ENC_INPUT_BUF_COUNT; i++) {
            if(this->m_pInputBuffer[i] != NULL) {
                free(this->m_pInputBuffer[i]);
                this->m_pInputBuffer[i] = NULL;
            }
        }

    
        if(this->m_pAVCodecContext != NULL) {
            avcodec_close(this->m_pAVCodecContext);
            av_free(this->m_pAVCodecContext);
            this->m_pAVCodecContext = NULL;
        }

        if(this->m_pAVFrame_Input != NULL) {
            avcodec_free_frame(&this->m_pAVFrame_Input);
            this->m_pAVFrame_Input = NULL;
        }

    

        free((void*)this);
    }

    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncInit(void *openHandle, void *param)
{
    SSBSIP_MFC_ERROR_CODE mfc_ret = MFC_RET_FAIL;
    struct mfc_enc *this = (struct mfc_enc *)openHandle;

    AVRational avr;
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    
    memcpy(&this->m_codecType, param, sizeof(SSBSIP_MFC_CODEC_TYPE));

    switch(this->m_codecType) {
    
        case MPEG4_ENC:
            memcpy(&this->m_MfcMpeg4Param, param, sizeof(SSBSIP_MFC_ENC_MPEG4_PARAM));
            this->m_AVCodecID = AV_CODEC_ID_MPEG4;
            this->m_nPicWidth = this->m_MfcMpeg4Param.SourceWidth;
            this->m_nPicHeight = this->m_MfcMpeg4Param.SourceHeight;
            this->m_nIDRPeriod = this->m_MfcMpeg4Param.IDRPeriod;
            this->m_nBitRate = this->m_MfcMpeg4Param.Bitrate;
            this->m_nFrameRate = this->m_MfcMpeg4Param.TimeIncreamentRes;
            break;

        case H264_ENC:
            memcpy(&this->m_MfcH264Param, param, sizeof(SSBSIP_MFC_ENC_H264_PARAM));
            //this->m_AVCodecID = AV_CODEC_ID_H264;
            this->m_AVCodecID = AV_CODEC_ID_MPEG4;
            this->m_nPicWidth = this->m_MfcH264Param.SourceWidth;
            this->m_nPicHeight = this->m_MfcH264Param.SourceHeight;
            this->m_nIDRPeriod = this->m_MfcH264Param.IDRPeriod;
            this->m_nBitRate = this->m_MfcH264Param.Bitrate;
            this->m_nFrameRate = this->m_MfcH264Param.FrameRate;
            
            break;

        case H263_ENC:
            memcpy(&this->m_MfcH263Param, param, sizeof(SSBSIP_MFC_ENC_H264_PARAM));
            //this->m_AVCodecID = AV_CODEC_ID_H264;
            this->m_AVCodecID = AV_CODEC_ID_MPEG4;
            this->m_nPicWidth = this->m_MfcH263Param.SourceWidth;
            this->m_nPicHeight = this->m_MfcH263Param.SourceHeight;
            this->m_nIDRPeriod = this->m_MfcH263Param.IDRPeriod;
            this->m_nBitRate = this->m_MfcH263Param.Bitrate;
            this->m_nFrameRate = this->m_MfcH263Param.FrameRate;
            
            break;
    }

    codec = avcodec_find_encoder(this->m_AVCodecID);
    if(codec == NULL) {
        return mfc_ret;
    }

    cc= avcodec_alloc_context();
    
    cc->bit_rate = this->m_nBitRate;     /* put sample parameters */
    cc->width = this->m_nPicWidth;   /* resolution must be a multiple of two */
    cc->height = this->m_nPicHeight;

    /* frames per second */
    avr.num = 1;
    avr.den = this->m_nFrameRate;
    cc->time_base= avr; //(AVRational){1, 25}; 

    cc->gop_size = this->m_nIDRPeriod; /* emit one intra frame every ten frames */
    cc->max_b_frames=1;

    cc->pix_fmt = PIX_FMT_YUV420P;

    cc->extradata = NULL;//pStream;
    cc->extradata_size = NULL;//nStreamSize;
    
 
    /* open it */
    if(avcodec_open(cc, codec) < 0) 
    {
        //MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderFfmpeg::DecodeDSI] FAIL: could not open codec\n\r")));
        return mfc_ret;
    }

    this->m_pAVCodec = codec;
    this->m_pAVCodecContext = cc;
    this->m_pAVFrame_Input = avcodec_alloc_frame();

    mfc_ret = MFC_RET_OK;

    return mfc_ret;
}

#define MFC_BYTE_ALIGN(x, align)   (((x) + (align) - 1) & ~((align) - 1))
SSBSIP_MFC_ERROR_CODE SsbSipMfcEncGetInBuf(void *openHandle, SSBSIP_MFC_ENC_INPUT_INFO *input_info)
{
    struct mfc_enc *this = (struct mfc_enc *)openHandle;
    int i;
    int empty_index;
    SSBSIP_MFC_ERROR_CODE mfc_ret = MFC_RET_FAIL;
    MMP_U8* Y, *U;
    int buffer_width, buffer_height;

    empty_index = -1;
    for(i = 0; i < MAX_ENC_INPUT_BUF_COUNT; i++) {
        
        if(this->m_pInputBuffer[i] == NULL) {
            empty_index = i;
            break;
        }
    }
    
    if(empty_index != -1) {

        buffer_width = MFC_BYTE_ALIGN(this->m_nPicWidth, 16); 
        buffer_height = MFC_BYTE_ALIGN(this->m_nPicHeight, 16);
    
        this->m_pInputBuffer[empty_index] = (MMP_U8*)malloc(buffer_width*buffer_height*3/2);
        if(this->m_pInputBuffer[empty_index] != NULL) {

            Y = this->m_pInputBuffer[empty_index];
            U = Y + (this->m_nPicWidth*this->m_nPicHeight);
            
            input_info->YPhyAddr = Y;
            input_info->CPhyAddr = U;
            input_info->YVirAddr = input_info->YPhyAddr;
            input_info->CVirAddr = input_info->CPhyAddr;
            input_info->YSize = buffer_width * buffer_height;
            input_info->CSize = buffer_width * buffer_height /2 ;
            input_info->y_cookie = 0;
            input_info->c_cookie = 0;

            mfc_ret = MFC_RET_OK;
        }
        
    }


    return mfc_ret;
}


SSBSIP_MFC_ERROR_CODE SsbSipMfcEncSetInBuf(void *openHandle, SSBSIP_MFC_ENC_INPUT_INFO *input_info)
{
    struct mfc_enc *this = (struct mfc_enc *)openHandle;
   
    memcpy(&this->m_last_input_info, input_info, sizeof(SSBSIP_MFC_ENC_INPUT_INFO) );

    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncGetOutBuf(void *openHandle, SSBSIP_MFC_ENC_OUTPUT_INFO *output_info)
{
    struct mfc_enc *this = (struct mfc_enc *)openHandle;
    
    output_info->StrmVirAddr = this->m_EncStreamBuffer;
    if(this->m_EncStreamKeyFrame == 1) {
        output_info->frameType = MFC_FRAME_TYPE_I_FRAME;
    }
    else {
        output_info->frameType = MFC_FRAME_TYPE_P_FRAME;
    }
    output_info->dataSize = this->m_EncStreamSize;

    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncSetOutBuf(void *openHandle, void *phyOutbuf, void *virOutbuf, int outputBufferSize)
{
    if (openHandle == NULL) {
        ALOGE("[%s] openHandle is NULL\n",__func__);
        return MFC_RET_INVALID_PARAM;
    }

    return MFC_RET_ENC_SET_OUTBUF_FAIL;
}

#define MMP_DECODED_BUF_Y 0
#define MMP_DECODED_BUF_U 1
#define MMP_DECODED_BUF_V 2
SSBSIP_MFC_ERROR_CODE SsbSipMfcEncExe(void *openHandle)
{
    SSBSIP_MFC_ERROR_CODE mfc_ret = MFC_RET_FAIL;
    AVPacket avpkt;
    SSBSIP_MFC_ENC_INPUT_INFO *input_info;
    struct mfc_enc *this = (struct mfc_enc *)openHandle;
    int iret,got_packet_ptr;

    this->m_EncStreamSize = 0;
    this->m_EncStreamKeyFrame = 0;


    av_init_packet (&avpkt);
    avpkt.data = NULL;
    avpkt.size = 0;
    avpkt.flags = 0;

    input_info = &this->m_last_input_info;
    
    this->m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] = (uint8_t*)input_info->YVirAddr;
    this->m_pAVFrame_Input->data[MMP_DECODED_BUF_U] = (uint8_t*)input_info->CVirAddr;
    this->m_pAVFrame_Input->data[MMP_DECODED_BUF_V] = this->m_pAVFrame_Input->data[MMP_DECODED_BUF_U] + (this->m_pAVCodecContext->width*this->m_pAVCodecContext->height/4);
    if(this->m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] == NULL) {
    
        this->m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] = (uint8_t*)this->m_PicBuffer;
        this->m_pAVFrame_Input->data[MMP_DECODED_BUF_U] = (uint8_t*)this->m_pAVFrame_Input->data[MMP_DECODED_BUF_Y] + this->m_nPicWidth*this->m_nPicHeight;
        this->m_pAVFrame_Input->data[MMP_DECODED_BUF_V] = (uint8_t*)this->m_pAVFrame_Input->data[MMP_DECODED_BUF_U] + (this->m_nPicWidth*this->m_nPicHeight)/4;
    }
    this->m_pAVFrame_Input->linesize[MMP_DECODED_BUF_Y] = this->m_nPicWidth;
    this->m_pAVFrame_Input->linesize[MMP_DECODED_BUF_U] = this->m_nPicWidth/2;
    this->m_pAVFrame_Input->linesize[MMP_DECODED_BUF_V] = this->m_nPicWidth/2;
    this->m_pAVFrame_Input->format = this->m_pAVCodecContext->pix_fmt;
    this->m_pAVFrame_Input->width = this->m_pAVCodecContext->width;
    this->m_pAVFrame_Input->height = this->m_pAVCodecContext->height;
    this->m_pAVFrame_Input->pts = AV_NOPTS_VALUE;

    iret = avcodec_encode_video2(this->m_pAVCodecContext, &avpkt, this->m_pAVFrame_Input, &got_packet_ptr);
    if(iret == 0) /* Success */ {
 
        /* get the delayed frames */
        if(got_packet_ptr == 0) {
             iret = avcodec_encode_video2(this->m_pAVCodecContext, &avpkt, NULL, &got_packet_ptr);
             if (iret < 0) {
                got_packet_ptr = 0;    
             }
        }

        if(got_packet_ptr == 1) {
        
             memcpy((void*)this->m_EncStreamBuffer, avpkt.data, avpkt.size);
             this->m_EncStreamSize = avpkt.size;

             if(avpkt.flags&AV_PKT_FLAG_KEY) {
                   this->m_EncStreamKeyFrame = 1;
             }

             mfc_ret = MFC_RET_OK;
        }
        
    }

    av_free_packet(&avpkt);

    return mfc_ret;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncSetConfig(void *openHandle, SSBSIP_MFC_ENC_CONF conf_type, void *value)
{

    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncGetConfig(void *openHandle, SSBSIP_MFC_ENC_CONF conf_type, void *value)
{
    _MFCLIB *pCTX;

    pCTX = (_MFCLIB *) openHandle;

    if (openHandle == NULL) {
        ALOGE("[%s] openHandle is NULL\n",__func__);
        return MFC_RET_INVALID_PARAM;
    }

    if (value == NULL) {
        ALOGE("[%s] value is NULL\n",__func__);
        return MFC_RET_INVALID_PARAM;
    }

    switch (conf_type) {
    case MFC_ENC_GETCONF_FRAME_TAG:
        *((unsigned int *)value) = pCTX->outframetagtop;
        break;

    default:
        ALOGE("[%s] conf_type(%d) is NOT supported\n",__func__, conf_type);
        return MFC_RET_INVALID_PARAM;
    }

    return MFC_RET_OK;
}

