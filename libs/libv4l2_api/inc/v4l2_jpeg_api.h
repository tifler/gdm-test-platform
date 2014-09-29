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

#ifndef V4L2_JPEG_API_H__
#define V4L2_JPEG_API_H__

#include "v4l2_api_def.h"
#include "v4l2_ion_tool.h"

/*****************************************************************************
  Define
*****************************************************************************/
#define V4L2_JPEG_MAX_PLANE V4L2_MAX_PLANE

#define V4L2_JPEG_ENC_NODE_NAME "/dev/video11"
#define V4L2_JPEG_DEC_NODE_NAME "/dev/video12"


 /*
   **Note
      V4L2 concept of SRC/DST is follow.
            out_q_ctx => src
            cap_q_ctx => dst
      Therefore, type of Video-Capture-Frame is OUTPUT_MPLANE  because of SRC
                 type of Encoded-jpeg-stream is CAPTURE_MPLANE because of DST
 */
#define V4L2_JPEG_BUF_TYPE_ENC_SRC  V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
#define V4L2_JPEG_BUF_TYPE_ENC_DST  V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE

#define V4L2_JPEG_BUF_TYPE_DEC_SRC   V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
#define V4L2_JPEG_BUF_TYPE_DEC_DST  V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE

#define V4L2_JPEG_ENC_MEMORY_TYPE  V4L2_MEMORY_DMABUF
#define V4L2_JPEG_DEC_MEMORY_TYPE  V4L2_MEMORY_DMABUF

/*****************************************************************************
      V4L2-JPEG  Control ID 
*****************************************************************************/
#define V4L2_JEPG_CID_ENC_BASE (0xCCCC0000)
#define V4L2_JEPG_CID_ENC_GET_COMPRESSION_SIZE  (V4L2_JEPG_CID_ENC_BASE  + 0x00)

#define V4L2_JEPG_CID_DEC_BASE (0xDDDD0000)
#define V4L2_JEPG_CID_DEC_SET_COMPRESSION_SIZE  (V4L2_JEPG_CID_DEC_BASE  + 0x00)

/*****************************************************************************
Structure 
*****************************************************************************/

struct v4l2_jpeg_dec_config {
    int pic_width;
    int pic_height;
    int jpeg_image_size;
    unsigned int pixel_format_out;
};

struct v4l2_jpeg_dec_buf {
    int     num_planes;
    void    *start[V4L2_JPEG_MAX_PLANE];
    int     length[V4L2_JPEG_MAX_PLANE];
    enum    v4l2_memory    memory_type;
};


/*****************************************************************************
     V4L2 JPEG API 
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* v4l2_jpeg enc/dev API */
int v4l2_jpeg_encode_ion(struct v4l2_ion_frame *p_ion_frame_src, int quality /* rage 0~100 */, struct v4l2_ion_buffer *p_ion_stream_dst, int* jpegsize);
int v4l2_jpeg_decode_ion(struct v4l2_ion_buffer *p_ion_stream_src, int jpeg_size, struct v4l2_ion_frame *p_ion_frame_dst);

/* v4l2_jpeg enc  */
int v4l2_jpeg_enc_open(void);
int v4l2_jpeg_enc_close(int fd);
int v4l2_jpeg_enc_set_config(int fd, struct v4l2_ion_frame *p_ion_frame_src, int quality, struct v4l2_ion_buffer *p_ion_stream_dst);
int v4l2_jpeg_enc_reqbuf_src(int fd, struct v4l2_ion_frame *p_ion_frame_src);
int v4l2_jpeg_enc_reqbuf_dst(int fd, struct v4l2_ion_buffer *p_ion_stream_dst);
int v4l2_jpeg_enc_exe(int fd, struct v4l2_ion_frame *p_ion_frame_src, struct v4l2_ion_buffer *p_ion_stream_dst);

/* v4l2_jpeg dec */
int v4l2_jpeg_dec_open(void);
int v4l2_jpeg_dec_close(int fd);
int v4l2_jpeg_dec_set_config(int fd, struct v4l2_ion_buffer *p_ion_stream_src, int jpeg_size, struct v4l2_ion_frame *p_ion_frame_dst);
int v4l2_jpeg_dec_reqbuf_src(int fd, struct v4l2_ion_buffer *p_ion_stream_src);
int v4l2_jpeg_dec_reqbuf_dst(int fd, struct v4l2_ion_frame *p_ion_frame_dst);
int v4l2_jpeg_dec_exe(int fd, struct v4l2_ion_buffer *p_ion_stream_src, struct v4l2_ion_frame *p_ion_frame_dst);

/* v4l2_jpeg util */
int v4l2_jpeg_open(const char* nodename);
int v4l2_jpeg_querycap(int fd);
int v4l2_jpeg_get_ctrl(int fd, unsigned int cid);
int v4l2_jpeg_set_ctrl(int fd, unsigned int cid, int value);
int v4l2_jpeg_stream_on(int fd, enum v4l2_buf_type type);
int v4l2_jpeg_stream_off(int fd, enum v4l2_buf_type type);


#ifdef __cplusplus
}
#endif

#define JPUDEBUGMSG(cond,printf_exp) do { if(cond) printf printf_exp; }while(0);
#define JPUZONE_ERROR 1
#define JPUZONE_INFO  1

#endif
