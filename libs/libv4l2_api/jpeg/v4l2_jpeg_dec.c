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

#include "v4l2_jpeg_api.h"
#include <stdio.h>

int v4l2_jpeg_dec_open(void) {

    int fd;

    fd = v4l2_jpeg_open(V4L2_JPEG_DEC_NODE_NAME);
    
    return fd;
}

int v4l2_jpeg_dec_close(int fd) {

    struct v4l2_requestbuffers req;
    int ret = 0;

    if(fd >= 0) {

        v4l2_jpeg_stream_off(fd, V4L2_JPEG_BUF_TYPE_DEC_SRC);
        v4l2_jpeg_stream_off(fd, V4L2_JPEG_BUF_TYPE_DEC_DST);
            
        /* release inbuf queue */
        memset(&req, 0, sizeof(req));
        req.type = V4L2_JPEG_BUF_TYPE_DEC_SRC;
        req.memory = V4L2_JPEG_DEC_MEMORY_TYPE;
        req.count = 0;
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req);

        /* release outbuf queue */
        memset(&req, 0, sizeof(req));
        req.type = V4L2_JPEG_BUF_TYPE_DEC_DST;
        req.memory = V4L2_JPEG_DEC_MEMORY_TYPE;
        req.count = 0;
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req);

        V4L2_DRIVER_CLOSE(fd);
    }

    return ret;
}



int v4l2_jpeg_dec_set_config(int fd, struct v4l2_ion_buffer *p_ion_stream_src, int jpeg_size, struct v4l2_ion_frame *p_ion_frame_dst) {

    struct v4l2_format fmt;
    int i, ret = 0;

    /* set input format */
    if(ret == 0) {
        fmt.type = V4L2_JPEG_BUF_TYPE_DEC_SRC;
        fmt.fmt.pix_mp.width = p_ion_frame_dst->width;
        fmt.fmt.pix_mp.height = p_ion_frame_dst->height;
        fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
        fmt.fmt.pix_mp.num_planes = 1;
        fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_JPEG;
        fmt.fmt.pix_mp.plane_fmt[0].sizeimage = p_ion_stream_src->buf_size; /* stream buf size */
        fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;               /* jpeg real data size */
        
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_FMT, &fmt);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set input fmt \n\r", __func__));
        }
    }
    
    /* set output format */
    if(ret == 0) {
        fmt.type = V4L2_JPEG_BUF_TYPE_DEC_DST;
        fmt.fmt.pix_mp.width = p_ion_frame_dst->width;
        fmt.fmt.pix_mp.height = p_ion_frame_dst->height;
        fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
        fmt.fmt.pix_mp.num_planes = p_ion_frame_dst->plane_count;
        fmt.fmt.pix_mp.pixelformat = p_ion_frame_dst->pix_fourcc;
        for(i = 0; i < p_ion_frame_dst->plane_count; i++) {
            fmt.fmt.pix_mp.plane_fmt[i].sizeimage = p_ion_frame_dst->plane[i].buf_size;  /* each plane bu size */ 
            fmt.fmt.pix_mp.plane_fmt[i].bytesperline = (__u16)p_ion_frame_dst->plane[i].stride; /* each plane stride */ 
        }
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_FMT, &fmt);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set output fmt \n\r", __func__));
        }
    }
    
    return ret;
}

int v4l2_jpeg_dec_reqbuf_src(int fd, struct v4l2_ion_buffer *p_ion_stream_src) {

    struct v4l2_requestbuffers reqbufobj;
    int iret = 0;
    
    memset(&reqbufobj, 0, sizeof(reqbufobj));
        
    reqbufobj.count = 1;
    reqbufobj.type = V4L2_JPEG_BUF_TYPE_DEC_SRC;
    reqbufobj.memory = V4L2_JPEG_DEC_MEMORY_TYPE;
    iret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &reqbufobj);
    if(iret < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req) \n\r", __func__));
    }

    return iret;
}

int v4l2_jpeg_dec_reqbuf_dst(int fd, struct v4l2_ion_frame *p_ion_frame_dst) {

    struct v4l2_requestbuffers reqbufobj;
    int iret = 0;
    
    memset(&reqbufobj, 0, sizeof(reqbufobj));
        
    reqbufobj.count = p_ion_frame_dst->plane_count;
    reqbufobj.type = V4L2_JPEG_BUF_TYPE_DEC_DST;
    reqbufobj.memory = V4L2_JPEG_DEC_MEMORY_TYPE;
    iret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &reqbufobj);
    if(iret < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req) \n\r", __func__));
    }

    return iret;
}

int v4l2_jpeg_dec_free_buf(V4L2_IN struct v4l2_jpeg_dec_buf *buf) {

    int i;

    if(buf->memory_type == V4L2_MEMORY_MMAP) {
    
        for (i = 0; i < buf->num_planes; i++) {

            if( (buf->start[i] != NULL) && (buf->length[i] > 0) ) {
                V4L2_DRIVER_MUNMAP((char *)(buf->start[i]), buf->length[i]);

                buf->start[i] = NULL;
                buf->length[i] = 0;
            }
        }
    }

    return 0;
}

int v4l2_jpeg_dec_exe(int fd, struct v4l2_ion_buffer *p_ion_stream_src, struct v4l2_ion_frame *p_ion_frame_dst) {

    struct v4l2_buffer v4l2_buf_src, v4l2_buf_dst;
    struct v4l2_plane plane_src[V4L2_JPEG_MAX_PLANE];
    struct v4l2_plane plane_dst[V4L2_JPEG_MAX_PLANE];
    int i;
    int ret = 0;

    /* config v4l2_buf_src */
    memset(&v4l2_buf_src, 0, sizeof(struct v4l2_buffer));
    memset(plane_src, 0, (int)V4L2_JPEG_MAX_PLANE * sizeof(struct v4l2_plane));
    v4l2_buf_src.index = 0;
    v4l2_buf_src.type = V4L2_JPEG_BUF_TYPE_DEC_SRC;
    v4l2_buf_src.memory = V4L2_JPEG_DEC_MEMORY_TYPE;
    v4l2_buf_src.length = 1;
    v4l2_buf_src.m.planes = plane_src;
    //if(inbuf->memory_type == V4L2_MEMORY_DMABUF) {
    //for (i = 0; i < inbuf->num_planes; i++) {
        v4l2_buf_src.m.planes[0].m.fd = p_ion_stream_src->shared_fd;
        v4l2_buf_src.m.planes[0].length = p_ion_stream_src->buf_size;
    //}

    /* config v4l2_buf_dst */
    memset(&v4l2_buf_dst, 0, sizeof(struct v4l2_buffer));
    memset(plane_dst, 0, (int)V4L2_JPEG_MAX_PLANE * sizeof(struct v4l2_plane));
    v4l2_buf_dst.index = 0;
    v4l2_buf_dst.type = V4L2_JPEG_BUF_TYPE_DEC_DST;
    v4l2_buf_dst.memory = V4L2_JPEG_DEC_MEMORY_TYPE;
    v4l2_buf_dst.length = p_ion_frame_dst->plane_count;
    v4l2_buf_dst.m.planes = plane_dst;
    //if(inbuf->memory_type == V4L2_MEMORY_DMABUF) {
    for (i = 0; i < p_ion_frame_dst->plane_count; i++) {
        v4l2_buf_dst.m.planes[i].m.fd = p_ion_frame_dst->plane[i].shared_fd;
        v4l2_buf_dst.m.planes[i].length = p_ion_frame_dst->plane[i].buf_size;
        v4l2_buf_dst.m.planes[i].data_offset = p_ion_frame_dst->plane[i].mem_offset;

    }

    
    /* attach input buffer to queue */
    if(ret == 0) {
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_QBUF, &v4l2_buf_src);
        if (ret < 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: add inbuf to queue \n\r", __func__ ));
        }
    }

    /* add out buffer to queue */
    if(ret == 0) {
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_QBUF, &v4l2_buf_dst);
        if (ret < 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: add outbuf to queue \n\r", __func__ ));
        }
    }

    /* stream on */
    if(ret == 0) {
        ret = v4l2_jpeg_stream_on(fd, V4L2_JPEG_BUF_TYPE_DEC_SRC);
    }
    if(ret == 0) {
        ret = v4l2_jpeg_stream_on(fd, V4L2_JPEG_BUF_TYPE_DEC_DST);
    }


    /* delete inbuf from queue */
    if(ret == 0) {
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_DQBUF, &v4l2_buf_src);
        if (ret < 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: delete inbuf from queue \n\r", __func__ ));
        }
    }
    
    /* delete outbuf from queue */
    if(ret == 0) {
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_DQBUF, &v4l2_buf_dst);
        if (ret < 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: delete inbuf from queue \n\r", __func__ ));
        }
    }
    
    return ret;
}

