/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
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
#ifdef USE_ION
#include "ion.h"
#endif

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "MFC_DEC_APP"
#include <utils/Log.h>


#if (MMP_OS == MMP_OS_WIN32)

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libavresample/audio_convert.h"

struct mfc_dec {
    enum AVCodecID m_AVCodecID;
    AVCodec *m_pAVCodec;
    AVCodecContext *m_pAVCodecContext;
    AVFrame *m_pAVFrame_Decoded;
    MMP_U8* m_extra_data;
    MMP_U32 m_frame_tag;
    MMP_U32 m_out_frametag;

    MMP_U8* m_pAu;
    MMP_U8* m_pAu_Phy;
    MMP_U32 m_auSize;
    
    int m_nStreamBufferCount;
    MMP_U8* m_pStreamBuffer[10];

    MMP_U8 m_PicBuffer[1920*1080*2];
};


/*#define CRC_ENABLE
#define SLICE_MODE_ENABLE */
#define POLL_DEC_WAIT_TIMEOUT 25

#define USR_DATA_START_CODE (0x000001B2)
#define VOP_START_CODE      (0x000001B6)
#define MP4_START_CODE      (0x000001)

#define DEFAULT_NUMBER_OF_EXTRA_DPB 5
#define CLEAR(x)    memset (&(x), 0, sizeof(x))
#ifdef S3D_SUPPORT
#define OPERATE_BIT(x, mask, shift)    ((x & (mask << shift)) >> shift)
#define FRAME_PACK_SEI_INFO_NUM  4
#endif

enum {
    NV12MT_FMT = 0,
    NV12M_FMT,
    NV21M_FMT,
};

static char *mfc_dev_name = SAMSUNG_MFC_DEV_NAME;
static int mfc_dev_node = 6;

int read_header_data(void *openHandle);
int init_mfc_output_stream(void *openHandle);
int isBreak_loop(void *openHandle);

int v4l2_mfc_querycap(int fd)
{
    struct v4l2_capability cap;
    int ret;

    CLEAR(cap);

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret != 0) {
        ALOGE("[%s] VIDIOC_QUERYCAP failed", __func__);
        return ret;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        ALOGE("[%s] Device does not support capture", __func__);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
        ALOGE("[%s] Device does not support output", __func__);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        ALOGE("[%s] Device does not support streaming", __func__);
        return -1;
    }

    return 0;
}

int v4l2_mfc_s_fmt(int fd, enum v4l2_buf_type type,
                    int pixelformat, unsigned int sizeimage, int width, int height)
{
    int ret;
    struct v4l2_format fmt;

    CLEAR(fmt);

    fmt.type = type;

    if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        switch (pixelformat) {
        case H264_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
            break;
        case MPEG4_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_MPEG4;
            break;
        case H263_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H263;
            break;
        case XVID_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_XVID;
            break;
        case MPEG2_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_MPEG12;
            break;
        case FIMV1_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_FIMV1;
            fmt.fmt.pix_mp.width = width;
            fmt.fmt.pix_mp.height = height;
            break;
        case FIMV2_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_FIMV2;
            break;
        case FIMV3_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_FIMV3;
            break;
        case FIMV4_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_FIMV4;
            break;
        case VC1_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VC1;
            break;
        case VC1RCV_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VC1_RCV;
            break;
#if defined (MFC6x_VERSION)
        case VP8_DEC:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VP8;
            break;
#endif
        default:
            ALOGE("[%s] Does NOT support the codec type (%d)", __func__, pixelformat);
            return -1;
        }
        fmt.fmt.pix_mp.plane_fmt[0].sizeimage = sizeimage;
    } else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        switch (pixelformat) {
        case NV12MT_FMT:
#if defined (MFC6x_VERSION)
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12MT_16X16;
#else
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12MT;
#endif
            break;
        case NV12M_FMT:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M;
            break;
        case NV21M_FMT:
            fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV21M;
            break;
        default:
            ALOGE("[%s] Does NOT support the pixel format (%d)", __func__, pixelformat);
            return -1;
        }
    } else {
        ALOGE("[%s] Wrong buffer type", __func__);
        return -1;
    }

    ret = ioctl(fd, VIDIOC_S_FMT, &fmt);

    return ret;
}

int v4l2_mfc_reqbufs(int fd, enum v4l2_buf_type type, enum v4l2_memory memory, int *buf_cnt)
{
    struct v4l2_requestbuffers reqbuf;
    int ret;

    CLEAR(reqbuf);

    reqbuf.type = type;
    reqbuf.memory = memory;
    reqbuf.count = *buf_cnt;

    ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
    *buf_cnt = reqbuf.count;

    return ret;
}

int v4l2_mfc_querybuf(int fd, struct v4l2_buffer *buf, enum v4l2_buf_type type,
                        enum v4l2_memory memory, int index, struct v4l2_plane *planes)
{
    int length = -1, ret;

    if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        length = 1;
    else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        length = 2;

    CLEAR(*buf);
    buf->type = type;
    buf->memory = memory;
    buf->index = index;
    buf->m.planes = planes;
    buf->length = length;

    ret = ioctl(fd, VIDIOC_QUERYBUF, buf);

    return ret;
}

int v4l2_mfc_streamon(int fd, enum v4l2_buf_type type)
{
    int ret;

    ret = ioctl(fd, VIDIOC_STREAMON, &type);

    return ret;
}

int v4l2_mfc_streamoff(int fd, enum v4l2_buf_type type)
{
    int ret;

    ret = ioctl(fd, VIDIOC_STREAMOFF, &type);

    return ret;
}

int v4l2_mfc_s_ctrl(int fd, int id, int value)
{
    struct v4l2_control ctrl;
    int ret;

    CLEAR(ctrl);
    ctrl.id = id;
    ctrl.value = value;

    ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);

    return ret;
}

int v4l2_mfc_g_ctrl(int fd, int id, int *value)
{
    struct v4l2_control ctrl;
    int ret;

    CLEAR(ctrl);
    ctrl.id = id;

    ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
    *value = ctrl.value;

    return ret;
}

#ifdef S3D_SUPPORT
int v4l2_mfc_ext_g_ctrl(int fd, SSBSIP_MFC_DEC_CONF conf_type, void *value)
{
    struct v4l2_ext_control ext_ctrl[FRAME_PACK_SEI_INFO_NUM];
    struct v4l2_ext_controls ext_ctrls;
    struct mfc_frame_pack_sei_info *sei_info;
    int ret, i;

    ext_ctrls.ctrl_class = V4L2_CTRL_CLASS_CODEC;

    switch (conf_type) {
    case MFC_DEC_GETCONF_FRAME_PACKING:
        sei_info = (struct mfc_frame_pack_sei_info *)value;
        for (i=0; i<FRAME_PACK_SEI_INFO_NUM; i++)
            CLEAR(ext_ctrl[i]);

        ext_ctrls.count = FRAME_PACK_SEI_INFO_NUM;
        ext_ctrls.controls = ext_ctrl;
        ext_ctrl[0].id =  V4L2_CID_CODEC_FRAME_PACK_SEI_AVAIL;
        ext_ctrl[1].id =  V4L2_CID_CODEC_FRAME_PACK_ARRGMENT_ID;
        ext_ctrl[2].id =  V4L2_CID_CODEC_FRAME_PACK_SEI_INFO;
        ext_ctrl[3].id =  V4L2_CID_CODEC_FRAME_PACK_GRID_POS;

        ret = ioctl(fd, VIDIOC_G_EXT_CTRLS, &ext_ctrls);

        sei_info->sei_avail = ext_ctrl[0].value;
        sei_info->arrgment_id = ext_ctrl[1].value;
        sei_info->sei_info = ext_ctrl[2].value;
        sei_info->grid_pos = ext_ctrl[3].value;
        break;
    }

    return ret;
}
#endif

int v4l2_mfc_qbuf(int fd, struct v4l2_buffer *qbuf, enum v4l2_buf_type type,
        enum v4l2_memory memory, int index,
        struct v4l2_plane *planes, int frame_length)
{
    int ret, length = 0;

    if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        CLEAR(*qbuf);
        length = 1;
    } else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        length = 2;
    }

    qbuf->type = type;
    qbuf->memory = memory;
    qbuf->index = index;
    qbuf->m.planes = planes;
    qbuf->length = length;
    qbuf->m.planes[0].bytesused = frame_length;

    ret = ioctl(fd, VIDIOC_QBUF, qbuf);

    return ret;
}

int v4l2_mfc_dqbuf(int fd, struct v4l2_buffer *dqbuf, enum v4l2_buf_type type,
                    enum v4l2_memory memory)
{
    struct v4l2_plane planes[MFC_DEC_NUM_PLANES];
    int ret, length = 0;

    CLEAR(*dqbuf);
    if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        length = 1;
    else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        length = 2;

    dqbuf->type = type;
    dqbuf->memory = memory;
    dqbuf->m.planes = planes;
    dqbuf->length = length;

    ret = ioctl(fd, VIDIOC_DQBUF, dqbuf);

    return ret;
}

int v4l2_mfc_g_fmt(int fd, struct v4l2_format *fmt, enum v4l2_buf_type type)
{
    int ret;

    CLEAR(*fmt);
    fmt->type = type;
    ret = ioctl(fd, VIDIOC_G_FMT, fmt);

    return ret;
}

int v4l2_mfc_g_crop(int fd, struct v4l2_crop *crop, enum v4l2_buf_type type)
{
    int ret;

    CLEAR(*crop);
    crop->type = type;
    ret = ioctl(fd, VIDIOC_G_CROP, crop);

    return ret;
}

int v4l2_mfc_poll(int fd, int *revents, int timeout)
{
#ifdef WIN32
    return 0;
#else
    struct pollfd poll_events;
    int ret;

    poll_events.fd = fd;
    poll_events.events = POLLOUT | POLLERR;
    poll_events.revents = 0;

    ret = poll((struct pollfd*)&poll_events, 1, timeout);
    *revents = poll_events.revents;

    return ret;
#endif
}

static void getAByte(char *buff, int *code)
{
    int byte;

    *code = (*code << 8);
    byte = (int)*buff;
    byte &= 0xFF;
    *code |= byte;
}

static int isPBPacked(_MFCLIB *pCtx, int Frameleng)
{
    char *strmBuffer = NULL;
    int startCode = 0xFFFFFFFF;
    int leng_idx = 1;

    strmBuffer = (char*)pCtx->virStrmBuf;

    while (1) {
        while (startCode != USR_DATA_START_CODE) {
            if ((startCode == VOP_START_CODE) || (leng_idx == Frameleng)) {
                ALOGI("[%s] VOP START Found !!.....return",__func__);
                ALOGW("[%s] Non Packed PB",__func__);
                return 0;
            }
            getAByte(strmBuffer, &startCode);
            ALOGV(">> StartCode = 0x%08x <<\n", startCode);
            strmBuffer++;
            leng_idx++;
        }
        ALOGI("[%s] User Data Found !!",__func__);

        do {
            if (*strmBuffer == 'p') {
                ALOGW("[%s] Packed PB",__func__);
                return 1;
            }
            getAByte(strmBuffer, &startCode);
            strmBuffer++; leng_idx++;
        } while ((leng_idx <= Frameleng) && ((startCode >> 8) != MP4_START_CODE));

        if (leng_idx > Frameleng)
            break;
    }

    ALOGW("[%s] Non Packed PB",__func__);

    return 0;
}

static void getMFCName(char *devicename, int size)
{
    snprintf(devicename, size, "%s%d", SAMSUNG_MFC_DEV_NAME, mfc_dev_node);
}

void SsbSipMfcDecSetMFCNode(int devicenode)
{
    mfc_dev_node = devicenode;
}

void SsbSipMfcDecSetMFCName(char *devicename)
{
    mfc_dev_name = devicename;
}


void *SsbSipMfcDecOpen(void)
{
    struct mfc_dec* this = NULL;

    avcodec_register_all();

    this = (struct mfc_dec*)malloc(sizeof(struct mfc_dec));
    memset(this, 0x00, sizeof(struct mfc_dec));
    
    return (void*)this;
}

void *SsbSipMfcDecOpenExt(void *value)
{
    struct mfc_dec* this = NULL;

    this = (struct mfc_dec*)malloc(sizeof(struct mfc_dec));
    
    return (void*)this;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcDecClose(void *openHandle)
{
    int i;
    struct mfc_dec* this = (struct mfc_dec*)openHandle;

    if(this != NULL) {


        if(this->m_pAVCodecContext != NULL) {
           avcodec_close(this->m_pAVCodecContext);
            av_free(this->m_pAVCodecContext);
            this->m_pAVCodecContext = NULL;
        }
    
        if(this->m_pAVFrame_Decoded != NULL) {
            avcodec_free_frame(&this->m_pAVFrame_Decoded);
            this->m_pAVFrame_Decoded = NULL;
        }

        if(this->m_extra_data != NULL) {
            free(this->m_extra_data);
            this->m_extra_data = NULL;
        }

        for(i = 0; i < this->m_nStreamBufferCount; i++) {
            
            if(this->m_pStreamBuffer[i] != NULL) {
                free(this->m_pStreamBuffer[i]);
                this->m_pStreamBuffer[i] = NULL;
            }

        }//this->m_nStreamBufferCount++;


        free(this);
    }

    return MFC_RET_OK;

}

SSBSIP_MFC_ERROR_CODE SsbSipMfcDecInit(void *openHandle, SSBSIP_MFC_CODEC_TYPE codec_type, int Frameleng)
{
    struct mfc_dec* this = (struct mfc_dec*)openHandle;
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    struct mmp_ffmpeg_packet_header *p_ffmpeg_packet_header;
    MMP_U32 key, psz1, psz2, i;
    

    switch(codec_type) {
    
        /* Audio */
        //case MMP_WAVE_FORMAT_MPEGLAYER3: m_AVCodecID = AV_CODEC_ID_MP3; break;
        //case MMP_WAVE_FORMAT_WMA2: m_AVCodecID = AV_CODEC_ID_WMAV2; break;

        /* Video */
        case H263_DEC: this->m_AVCodecID=AV_CODEC_ID_H263; break;
        case H264_DEC: this->m_AVCodecID=AV_CODEC_ID_H264; break;
        case MPEG4_DEC: this->m_AVCodecID=AV_CODEC_ID_MPEG4; break;
        case MPEG2_DEC: this->m_AVCodecID=AV_CODEC_ID_MPEG2VIDEO; break;
        case VC1_DEC : this->m_AVCodecID=AV_CODEC_ID_VC1; break;
        case VC1RCV_DEC : this->m_AVCodecID=AV_CODEC_ID_WMV3; break;
        //case MMP_FOURCC_VIDEO_VC1: m_AVCodecID=AV_CODEC_ID_VC1; break;
        //case MMP_FOURCC_VIDEO_WMV1: m_AVCodecID=AV_CODEC_ID_WMV1; break;
        //case MMP_FOURCC_VIDEO_WMV2: m_AVCodecID=AV_CODEC_ID_WMV2; break;
        //case MMP_FOURCC_VIDEO_WMV3: m_AVCodecID=AV_CODEC_ID_WMV3; break;
        
        default:  this->m_AVCodecID = AV_CODEC_ID_NONE;
    }

    codec = avcodec_find_decoder(this->m_AVCodecID);
    if(codec == NULL) {
        return MFC_RET_FAIL;
    }

    cc= avcodec_alloc_context();


    p_ffmpeg_packet_header = (struct mmp_ffmpeg_packet_header *)this->m_pAu;
    key = p_ffmpeg_packet_header->key;
    psz1 = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size+p_ffmpeg_packet_header->extra_data_size; //Packet Size
    psz2 = p_ffmpeg_packet_header->packet_size;

    if((key == MMP_FFMPEG_PACKET_HEADER_KEY) && (psz1 == psz2) ) {
        
        switch(p_ffmpeg_packet_header->payload_type) {
        
            case MMP_FFMPEG_PACKET_TYPE_AVCodecContext:
                i = p_ffmpeg_packet_header->hdr_size;
                cc1 = (AVCodecContext *)&this->m_pAu[i];
                memcpy(cc, &this->m_pAu[i], p_ffmpeg_packet_header->payload_size);
                
                i = p_ffmpeg_packet_header->hdr_size + p_ffmpeg_packet_header->payload_size;

                this->m_extra_data = malloc(p_ffmpeg_packet_header->extra_data_size);
                memcpy(this->m_extra_data, &this->m_pAu[i], p_ffmpeg_packet_header->extra_data_size);
                cc->extradata = this->m_extra_data;
                cc->extradata_size = p_ffmpeg_packet_header->extra_data_size;
                break;
        
        }
    }
    else {
        this->m_extra_data = malloc(Frameleng);
        memcpy(this->m_extra_data, this->m_pAu, Frameleng);
        cc->extradata = this->m_extra_data;
        cc->extradata_size = Frameleng;
    }

    /* open it */
    if(avcodec_open(cc, codec) < 0) 
    {
       // MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMpDecoderA_MPlayer::Open] FAIL: could not open codec\n\r")));
        return MMP_FAILURE;
    }

    this->m_pAVCodec = codec;
    this->m_pAVCodecContext = cc;
    this->m_pAVFrame_Decoded = avcodec_alloc_frame();
   

    return MFC_RET_OK;
}

int read_header_data(void *openHandle)
{
    struct v4l2_format fmt;
    struct v4l2_crop crop;
    struct v4l2_pix_format_mplane pix_mp;
    int ctrl_value;
    int ret;

    _MFCLIB *pCTX;
    pCTX  = (_MFCLIB *) openHandle;

    ret = v4l2_mfc_g_fmt(pCTX->hMFC, &fmt, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    if (ret != 0) {
        ALOGE("[%s] VIDIOC_G_FMT failed",__func__);
        ret = MFC_RET_DEC_INIT_FAIL;
        goto error_case;
    }

    pix_mp = fmt.fmt.pix_mp;
    pCTX->decOutInfo.buf_width = pix_mp.plane_fmt[0].bytesperline;
    pCTX->decOutInfo.buf_height =
        pix_mp.plane_fmt[0].sizeimage / pix_mp.plane_fmt[0].bytesperline;

    pCTX->decOutInfo.img_width = pix_mp.width;
    pCTX->decOutInfo.img_height = pix_mp.height;

    ret = v4l2_mfc_g_crop(pCTX->hMFC, &crop, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    if (ret != 0) {
        ALOGE("[%s] VIDIOC_G_CROP failed",__func__);
        ret = MFC_RET_DEC_INIT_FAIL;
        goto error_case;
    }

    pCTX->decOutInfo.crop_left_offset = crop.c.left;
    pCTX->decOutInfo.crop_top_offset = crop.c.top;
    pCTX->decOutInfo.crop_right_offset =
        pix_mp.width - crop.c.width - crop.c.left;
    pCTX->decOutInfo.crop_bottom_offset =
        pix_mp.height - crop.c.height - crop.c.top;

    ret = v4l2_mfc_g_ctrl(pCTX->hMFC, V4L2_CID_CODEC_REQ_NUM_BUFS, &ctrl_value);
    if (ret != 0) {
        ALOGE("[%s] VIDIOC_G_CTRL failed",__func__);
        ret = MFC_RET_DEC_INIT_FAIL;
        goto error_case;
    }

    pCTX->v4l2_dec.mfc_num_dst_bufs = ctrl_value + pCTX->dec_numextradpb;

    ALOGV("[%s] Num of allocated buffers: %d\n",__func__, pCTX->v4l2_dec.mfc_num_dst_bufs);

    return 0;

error_case:
    return ret;
    }

/* Initialize output stream of MFC */
int init_mfc_output_stream(void *openHandle)
{
    struct v4l2_buffer buf;
    struct v4l2_plane planes[MFC_DEC_NUM_PLANES];
    int ret;
    int i, j;
    _MFCLIB *pCTX;
    pCTX  = (_MFCLIB *) openHandle;

    ret = v4l2_mfc_reqbufs(pCTX->hMFC, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                    V4L2_MEMORY_MMAP, (int *)&pCTX->v4l2_dec.mfc_num_dst_bufs);
    if (ret != 0) {
        ALOGE("[%s] VIDIOC_REQBUFS failed (destination buffers)",__func__);
        ret = MFC_RET_DEC_INIT_FAIL;
        goto error_case1;
    }

    for (i = 0;  i < pCTX->v4l2_dec.mfc_num_dst_bufs; ++i) {
        ret = v4l2_mfc_querybuf(pCTX->hMFC, &buf, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                        V4L2_MEMORY_MMAP, i, planes);
        if (ret != 0) {
            ALOGE("[%s] VIDIOC_QUERYBUF failed (destination buffers)",__func__);
            ret = MFC_RET_DEC_INIT_FAIL;
            goto error_case1;
        }

        pCTX->v4l2_dec.mfc_dst_bufs_len[0] = buf.m.planes[0].length;
        pCTX->v4l2_dec.mfc_dst_bufs_len[1] = buf.m.planes[1].length;

        pCTX->v4l2_dec.mfc_dst_phys[i][0] = buf.m.planes[0].cookie;
        pCTX->v4l2_dec.mfc_dst_phys[i][1] = buf.m.planes[1].cookie;

#ifdef USE_ION
        pCTX->dst_ion_fd[i][0] = (int)buf.m.planes[0].share;
        pCTX->dst_ion_fd[i][1] = (int)buf.m.planes[1].share;

        pCTX->v4l2_dec.mfc_dst_bufs[i][0] =
            ion_map(pCTX->dst_ion_fd[i][0],pCTX->v4l2_dec.mfc_dst_bufs_len[0],0);
        pCTX->v4l2_dec.mfc_dst_bufs[i][1] =
            ion_map(pCTX->dst_ion_fd[i][1],pCTX->v4l2_dec.mfc_dst_bufs_len[1],0);
        if (pCTX->v4l2_dec.mfc_dst_bufs[i][0] == MAP_FAILED ||
            pCTX->v4l2_dec.mfc_dst_bufs[i][0] == MAP_FAILED)
            goto error_case2;
#else
        pCTX->v4l2_dec.mfc_dst_bufs[i][0] = mmap(NULL, buf.m.planes[0].length,
             PROT_READ | PROT_WRITE, MAP_SHARED, pCTX->hMFC, buf.m.planes[0].m.mem_offset);
        if (pCTX->v4l2_dec.mfc_dst_bufs[i][0] == MAP_FAILED) {
            ALOGE("[%s] mmap failed (destination buffers (Y))",__func__);
            ret = MFC_RET_DEC_INIT_FAIL;
            goto error_case2;
        }

        pCTX->v4l2_dec.mfc_dst_bufs[i][1] = mmap(NULL, buf.m.planes[1].length,
        PROT_READ | PROT_WRITE, MAP_SHARED, pCTX->hMFC, buf.m.planes[1].m.mem_offset);
        if (pCTX->v4l2_dec.mfc_dst_bufs[i][1] == MAP_FAILED) {
            ALOGE("[%s] mmap failed (destination buffers (UV))",__func__);
            ret = MFC_RET_DEC_INIT_FAIL;
            goto error_case2;
        }
#endif

        ret = v4l2_mfc_qbuf(pCTX->hMFC, &buf, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                        V4L2_MEMORY_MMAP, i, planes, 0);
        if (ret != 0) {
            ALOGE("[%s] VIDIOC_QBUF failed, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE",__func__);
            ret = MFC_RET_DEC_INIT_FAIL;
            goto error_case2;
        }
    }
    pCTX->inter_buff_status |= MFC_USE_YUV_BUFF;

    return 0;

error_case2:
    for (j = 0; j < i; j++) {
        munmap(pCTX->v4l2_dec.mfc_dst_bufs[j][0], pCTX->v4l2_dec.mfc_dst_bufs_len[0]);
        munmap(pCTX->v4l2_dec.mfc_dst_bufs[j][1], pCTX->v4l2_dec.mfc_dst_bufs_len[1]);
    }
error_case1:
    return ret;
}

int resolution_change(void *openHandle)
{
    int i, ret;
    int req_count;
    _MFCLIB *pCTX;
    pCTX  = (_MFCLIB *) openHandle;

    ret = v4l2_mfc_streamoff(pCTX->hMFC, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    if (ret != 0)
        goto error_case;

    pCTX->inter_buff_status &= ~(MFC_USE_DST_STREAMON);

    for (i = 0; i < pCTX->v4l2_dec.mfc_num_dst_bufs; i++) {
        munmap(pCTX->v4l2_dec.mfc_dst_bufs[i][0], pCTX->v4l2_dec.mfc_dst_bufs_len[0]);
        munmap(pCTX->v4l2_dec.mfc_dst_bufs[i][1], pCTX->v4l2_dec.mfc_dst_bufs_len[1]);
    }
    pCTX->inter_buff_status &= ~(MFC_USE_YUV_BUFF);

    req_count = 0;
    ret = v4l2_mfc_reqbufs(pCTX->hMFC, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                    V4L2_MEMORY_MMAP, &req_count);
    if (ret != 0)
        goto error_case;

    read_header_data(pCTX);
    init_mfc_output_stream(pCTX);

    ret = v4l2_mfc_streamon(pCTX->hMFC, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    if (ret != 0)
        goto error_case;
    pCTX->inter_buff_status |= MFC_USE_DST_STREAMON;

    return 0;

error_case:
    return ret;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcDecExe(void *openHandle, int lengthBufFill)
{
    SSBSIP_MFC_ERROR_CODE mfcResult = MFC_RET_OK;
    struct mfc_dec *this = (struct mfc_dec *)openHandle;

    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;

    AVFrame *pAVFrame_Temp;

    pAVFrame_Temp = avcodec_alloc_frame();;

    av_init_packet (&avpkt);
    avpkt.data = this->m_pAu;
    avpkt.size = lengthBufFill;
    avpkt.pts = (int64_t)(this->m_frame_tag*1000);
    
    usebyte = avcodec_decode_video2(this->m_pAVCodecContext, pAVFrame_Temp, &frameFinished, &avpkt);
    if(usebyte > 0) {
        //pDecResult->uiAuUsedByte = usebyte;

    }
    else {
        mfcResult = MFC_RET_FAIL;
    }

    if(frameFinished == 0) {
    
        mfcResult = MFC_RET_FAIL;
    }

    if(mfcResult == MFC_RET_OK) {
        
        this->m_out_frametag = pAVFrame_Temp->pkt_pts/1000;

        avpicture_fill((AVPicture *)this->m_pAVFrame_Decoded, this->m_PicBuffer, PIX_FMT_YUV420P, this->m_pAVCodecContext->width, this->m_pAVCodecContext->height);
        av_picture_copy ((AVPicture *)this->m_pAVFrame_Decoded, (AVPicture*)pAVFrame_Temp, PIX_FMT_YUV420P, this->m_pAVCodecContext->width, this->m_pAVCodecContext->height);
        
    }

    av_free(pAVFrame_Temp);

    return mfcResult;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcDecExeNb(void *openHandle, int lengthBufFill)
{
    _MFCLIB *pCTX;
    int ret;

    struct v4l2_buffer buf;
    struct v4l2_plane planes[MFC_DEC_NUM_PLANES];

    if (openHandle == NULL) {
        ALOGE("[%s] openHandle is NULL",__func__);
        return MFC_RET_INVALID_PARAM;
    }

    if ((lengthBufFill < 0) || (lengthBufFill > MAX_DECODER_INPUT_BUFFER_SIZE)) {
        ALOGE("[%s] lengthBufFill is invalid. (lengthBufFill=%d)",__func__, lengthBufFill);
        return MFC_RET_INVALID_PARAM;
    }

    pCTX  = (_MFCLIB *) openHandle;

    if ((lengthBufFill > 0) && (SSBSIP_MFC_LAST_FRAME_PROCESSED != pCTX->lastframe)
                            && (pCTX->displayStatus != MFC_GETOUTBUF_DISPLAY_ONLY)) {
        /* Queue the stream frame */
        ret = v4l2_mfc_qbuf(pCTX->hMFC, &buf, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
                            V4L2_MEMORY_MMAP, pCTX->v4l2_dec.beingUsedIndex, planes, lengthBufFill);
        if (ret != 0) {
            ALOGE("[%s] VIDIOC_QBUF failed, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE",__func__);
            return MFC_RET_DEC_EXE_ERR;
        }
    } else if (pCTX->v4l2_dec.bBeingFinalized == 0) {
        /* Queue the stream frame */
        ret = v4l2_mfc_qbuf(pCTX->hMFC, &buf, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
                            V4L2_MEMORY_MMAP, pCTX->v4l2_dec.beingUsedIndex, planes, 0);
        if (ret != 0) {
            ALOGE("[%s] VIDIOC_QBUF failed, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE",__func__);
            return MFC_RET_DEC_EXE_ERR;
        }
    }

    if ((SSBSIP_MFC_LAST_FRAME_PROCESSED != pCTX->lastframe) && (lengthBufFill == 0))
        pCTX->lastframe = SSBSIP_MFC_LAST_FRAME_RECEIVED;

    return MFC_RET_OK;
}

int isBreak_loop(void *openHandle)
{
#ifdef WIN32

    return 0;
#else
    _MFCLIB *pCTX;
    pCTX  = (_MFCLIB *) openHandle;
    int ctrl_value;
    int ret = 0;

    if (pCTX->displayStatus == MFC_GETOUTBUF_DISPLAY_ONLY)
        return 1;

    ret = v4l2_mfc_g_ctrl(pCTX->hMFC, V4L2_CID_CODEC_CHECK_STATE, &ctrl_value);
    if (ret != 0) {
        ALOGE("[%s] VIDIOC_G_CTRL failed", __func__);
        return 0;
    }

    if (ctrl_value == MFCSTATE_DEC_RES_DETECT) {
        ALOGV("[%s] Resolution Change detect",__func__);
        return 1;
    } else if (ctrl_value == MFCSTATE_DEC_TERMINATING) {
        ALOGV("[%s] Decoding Finish!!!",__func__);
        return 1;
    }

    return 0;
#endif
}

SSBSIP_MFC_DEC_OUTBUF_STATUS SsbSipMfcDecWaitForOutBuf(void *openHandle, SSBSIP_MFC_DEC_OUTPUT_INFO *output_info)
{
    return MFC_GETOUTBUF_STATUS_NULL;
}

void  *SsbSipMfcDecGetInBuf(void *openHandle, void **phyInBuf, int inputBufferSize)
{
    struct mfc_dec* this = (struct mfc_dec*)openHandle;
    void* ptr;

    ptr = malloc(inputBufferSize);

    this->m_pStreamBuffer[this->m_nStreamBufferCount] = (MMP_U8*)ptr;
    this->m_nStreamBufferCount++;

    return ptr;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcDecSetInBuf(void *openHandle, void *phyInBuf, void *virInBuf, int size)
{
    struct mfc_dec* this = (struct mfc_dec*)openHandle;

    this->m_pAu_Phy = phyInBuf;
    this->m_pAu = virInBuf;
    this->m_auSize = size;

    return MFC_RET_OK;
}

SSBSIP_MFC_DEC_OUTBUF_STATUS SsbSipMfcDecGetOutBuf(void *openHandle, SSBSIP_MFC_DEC_OUTPUT_INFO *output_info)
{
    struct mfc_dec* this = (struct mfc_dec*)openHandle; //_MFCLIB *pCTX;
    
    output_info->YPhyAddr = (void*)this->m_pAVFrame_Decoded;//m_pCTX->decOutInfo.YPhyAddr;
    output_info->CPhyAddr = output_info->YPhyAddr;

    output_info->YVirAddr = output_info->YPhyAddr;
    output_info->CVirAddr = output_info->CPhyAddr;

    output_info->img_width = this->m_pAVCodecContext->width; //m_pCTX->decOutInfo.img_width;
    output_info->img_height= this->m_pAVCodecContext->height; //pCTX->decOutInfo.img_height;

    output_info->buf_width = this->m_pAVCodecContext->width;//pCTX->decOutInfo.buf_width;
    output_info->buf_height= this->m_pAVCodecContext->height;//pCTX->decOutInfo.buf_height;

    output_info->crop_right_offset =  0;//pCTX->decOutInfo.crop_right_offset;
    output_info->crop_left_offset =  0;//pCTX->decOutInfo.crop_left_offset;
    output_info->crop_bottom_offset = 0;//pCTX->decOutInfo.crop_bottom_offset;
    output_info->crop_top_offset = 0;//pCTX->decOutInfo.crop_top_offset;

    output_info->disp_pic_frame_type = 0;//pCTX->decOutInfo.disp_pic_frame_type;

    return MFC_GETOUTBUF_DISPLAY_ONLY;
 /*
    switch (pCTX->displayStatus) {
    case MFC_GETOUTBUF_DISPLAY_ONLY:
    case MFC_GETOUTBUF_DISPLAY_DECODING:
    case MFC_GETOUTBUF_DISPLAY_END:
    case MFC_GETOUTBUF_DECODING_ONLY:
    case MFC_GETOUTBUF_CHANGE_RESOL:
        break;
    default:
        return MFC_GETOUTBUF_DISPLAY_END;
    }

    return pCTX->displayStatus;
    */
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcDecSetConfig(void *openHandle, SSBSIP_MFC_DEC_CONF conf_type, void *value)
{
#ifdef WIN32

    struct mfc_dec* this = (struct mfc_dec*)openHandle;

    switch(conf_type) {
    
        case MFC_DEC_SETCONF_FRAME_TAG: /*be set before calling SsbSipMfcDecExe */
            this->m_frame_tag = *((MMP_U32*)value);
            break;

    }

    return 0;
#else

    int ret, i;

    _MFCLIB *pCTX;
    struct mfc_dec_fimv1_info *fimv1_res;

    struct v4l2_buffer buf;
    struct v4l2_plane planes[MFC_DEC_NUM_PLANES];

    int id, ctrl_value;

    if (openHandle == NULL) {
        ALOGE("[%s] openHandle is NULL",__func__);
        return MFC_RET_INVALID_PARAM;
    }

    if ((value == NULL) && (MFC_DEC_SETCONF_IS_LAST_FRAME !=conf_type)) {
        ALOGE("[%s] value is NULL",__func__);
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *) openHandle;

    /* First, process non-ioctl calling settings */
    switch (conf_type) {
    case MFC_DEC_SETCONF_EXTRA_BUFFER_NUM:
        pCTX->dec_numextradpb = *((unsigned int *) value);
        return MFC_RET_OK;

    case MFC_DEC_SETCONF_FIMV1_WIDTH_HEIGHT: /* be set before calling SsbSipMfcDecInit */
         fimv1_res = (struct mfc_dec_fimv1_info *)value;
         ALOGI("fimv1->width  = %d\n", fimv1_res->width);
         ALOGI("fimv1->height = %d\n", fimv1_res->height);
         pCTX->fimv1_res.width  = (int)(fimv1_res->width);
         pCTX->fimv1_res.height = (int)(fimv1_res->height);
         return MFC_RET_OK;

    case MFC_DEC_SETCONF_IS_LAST_FRAME:
        if (SSBSIP_MFC_LAST_FRAME_PROCESSED != pCTX->lastframe) {
            pCTX->lastframe = SSBSIP_MFC_LAST_FRAME_RECEIVED;
            return MFC_RET_OK;
        } else {
            return MFC_RET_FAIL;
        }

    case MFC_DEC_SETCONF_DPB_FLUSH:
        ret = v4l2_mfc_streamoff(pCTX->hMFC, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
        if (ret != 0) {
            ALOGE("[%s] VIDIOC_STREAMOFF failed (destination buffers)",__func__);
            return MFC_RET_DEC_SET_CONF_FAIL;
        }
        pCTX->inter_buff_status &= ~(MFC_USE_DST_STREAMON);

        for (i = 0;  i < pCTX->v4l2_dec.mfc_num_dst_bufs; ++i) {
            ret = v4l2_mfc_qbuf(pCTX->hMFC, &buf, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                        V4L2_MEMORY_MMAP, i, planes, 0);
            if (ret != 0) {
                ALOGE("[%s] VIDIOC_QBUF failed, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE",__func__);
                return MFC_RET_DEC_SET_CONF_FAIL;
            }
        }

        ret = v4l2_mfc_streamon(pCTX->hMFC, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
        if (ret != 0) {
            ALOGE("[%s] VIDIOC_STREAMON failed (destination buffers)",__func__);
            return MFC_RET_DEC_SET_CONF_FAIL;
        }
        pCTX->inter_buff_status |= MFC_USE_DST_STREAMON;
        return MFC_RET_OK;
    default:
        /* Others will be processed next */
        break;
    }

    /* Process ioctl calling settings */
    switch (conf_type) {
    case MFC_DEC_SETCONF_DISPLAY_DELAY: /* be set before calling SsbSipMfcDecInit */
        id = V4L2_CID_CODEC_DISPLAY_DELAY;
        ctrl_value = *((unsigned int *) value);
        break;

    case MFC_DEC_SETCONF_CRC_ENABLE:
        id = V4L2_CID_CODEC_CRC_ENABLE;
        ctrl_value = 1;
        break;

    case MFC_DEC_SETCONF_SLICE_ENABLE:
        id = V4L2_CID_CODEC_SLICE_INTERFACE;
        ctrl_value = 1;
        break;

    case MFC_DEC_SETCONF_FRAME_TAG: /*be set before calling SsbSipMfcDecExe */
        id = V4L2_CID_CODEC_FRAME_TAG;
        ctrl_value = *((unsigned int*)value);
        break;

    case MFC_DEC_SETCONF_POST_ENABLE:
        id = V4L2_CID_CODEC_LOOP_FILTER_MPEG4_ENABLE;
        ctrl_value = *((unsigned int*)value);
        break;
#ifdef S3D_SUPPORT
    case MFC_DEC_SETCONF_SEI_PARSE:
        id = V4L2_CID_CODEC_FRAME_PACK_SEI_PARSE;
        ctrl_value = 1;
        break;
#endif
    default:
        ALOGE("[%s] conf_type(%d) is NOT supported",__func__, conf_type);
        return MFC_RET_INVALID_PARAM;
    }

    ret = v4l2_mfc_s_ctrl(pCTX->hMFC, id, ctrl_value);
    if (ret != 0) {
        ALOGE("[%s] VIDIOC_S_CTRL failed (conf_type = %d)",__func__, conf_type);
        return MFC_RET_DEC_SET_CONF_FAIL;
    }

    return MFC_RET_OK;
#endif
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcDecGetConfig(void *openHandle, SSBSIP_MFC_DEC_CONF conf_type, void *value)
{
    struct mfc_dec* this = (struct mfc_dec*)openHandle;
    SSBSIP_MFC_IMG_RESOLUTION* p_img_resol;
    unsigned int* p;

    switch(conf_type) {
        case MFC_DEC_GETCONF_BUF_WIDTH_HEIGHT:

            p_img_resol = (SSBSIP_MFC_IMG_RESOLUTION*)value;
            p_img_resol->width = this->m_pAVCodecContext->width;
            p_img_resol->height = this->m_pAVCodecContext->height;
            p_img_resol->buf_width = this->m_pAVCodecContext->width;
            p_img_resol->buf_height = this->m_pAVCodecContext->height;
            break;

        case MFC_DEC_GETCONF_FRAME_TAG:
            p = (unsigned int*)value;
            (*p) = this->m_out_frametag;
            break;
    }

    return MFC_RET_OK;
}

#endif /*  #if (MMP_OS == MMP_OS_WIN32) */