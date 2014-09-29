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

int v4l2_jpeg_enc_open(void) {

    int fd;

    fd = v4l2_jpeg_open(V4L2_JPEG_ENC_NODE_NAME);
    
    return fd;
}

int v4l2_jpeg_enc_close(int fd) {

    struct v4l2_requestbuffers req;
    int ret = 0;

    if(fd >= 0) {

        v4l2_jpeg_stream_off(fd, V4L2_JPEG_BUF_TYPE_ENC_SRC);
        v4l2_jpeg_stream_off(fd, V4L2_JPEG_BUF_TYPE_ENC_DST);
            
        /* release inbuf queue */
        memset(&req, 0, sizeof(req));
        req.type = V4L2_JPEG_BUF_TYPE_ENC_SRC;
        req.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
        req.count = 0;
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req);

        /* release outbuf queue */
        memset(&req, 0, sizeof(req));
        req.type = V4L2_JPEG_BUF_TYPE_ENC_DST;
        req.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
        req.count = 0;
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req);

        V4L2_DRIVER_CLOSE(fd);
    }
    return 0;
}


#if 1

int v4l2_jpeg_enc_set_config(int fd, struct v4l2_ion_frame *p_ion_frame_src, int quality, struct v4l2_ion_buffer *p_ion_stream_dst) {

    struct v4l2_format fmt;
    struct v4l2_jpegcompression jpegcompression;
    int ret = 0, i;

    memset(&fmt, 0x00, sizeof(fmt));
    memset(&jpegcompression, 0x00, sizeof(jpegcompression));
    
    /* set jpeg quality */
    if(ret == 0) {
        jpegcompression.quality = quality;
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_JPEGCOMP, &jpegcompression);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set quality \n\r", __func__));
        }
    }

    /* set input format */
    if(ret == 0) {
        fmt.type = V4L2_JPEG_BUF_TYPE_ENC_SRC;
        fmt.fmt.pix_mp.width = p_ion_frame_src->width;
        fmt.fmt.pix_mp.height = p_ion_frame_src->height;
        fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
        fmt.fmt.pix_mp.num_planes = p_ion_frame_src->plane_count;
        fmt.fmt.pix_mp.pixelformat = p_ion_frame_src->pix_fourcc;
        for(i = 0; i < fmt.fmt.pix_mp.num_planes; i++) {
            fmt.fmt.pix_mp.plane_fmt[i].sizeimage = p_ion_frame_src->plane[i].buf_size;
            fmt.fmt.pix_mp.plane_fmt[i].bytesperline = p_ion_frame_src->plane[i].stride;
        }
        
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_FMT, &fmt);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set input fmt \n\r", __func__));
        }
    }
    
    /* set output format */
    if(ret == 0) {
        fmt.type = V4L2_JPEG_BUF_TYPE_ENC_DST;
        fmt.fmt.pix_mp.width = p_ion_frame_src->width;
        fmt.fmt.pix_mp.height = p_ion_frame_src->height;
        fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
        fmt.fmt.pix_mp.num_planes = 1;
        fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_JPEG;
        fmt.fmt.pix_mp.plane_fmt[0].sizeimage = p_ion_stream_dst->buf_size;
        fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;
        
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_FMT, &fmt);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set output fmt \n\r", __func__));
        }
    }
    //v4l2_jpeg_set_ctrl(fd, V4L2_CID_CACHEABLE, 1);

    return ret;
}

#else
int v4l2_jpeg_enc_set_config(int fd, struct v4l2_jpeg_enc_config* p_enc_config) {

    struct v4l2_format fmt;
    struct v4l2_jpegcompression jpegcompression;
    int ret = 0;
    
    /* set jpeg quality */
    if(ret == 0) {
        jpegcompression.quality = p_enc_config->quality;
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_JPEGCOMP, &jpegcompression);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set quality \n\r", __func__));
        }
    }

    /* set input format */
    if(ret == 0) {
        fmt.type = V4L2_JPEG_BUF_TYPE_ENC_SRC;
        fmt.fmt.pix_mp.width = p_enc_config->pic_width;
        fmt.fmt.pix_mp.height = p_enc_config->pic_height;
        fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
        fmt.fmt.pix_mp.num_planes = 1;
        fmt.fmt.pix_mp.pixelformat = p_enc_config->pixel_format_in;
        fmt.fmt.pix_mp.plane_fmt[0].sizeimage = (p_enc_config->pic_width*p_enc_config->pic_height)*4;
        
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_FMT, &fmt);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set input fmt \n\r", __func__));
        }
    }
    
    /* set output format */
    if(ret == 0) {
        fmt.type = V4L2_JPEG_BUF_TYPE_ENC_DST;
        fmt.fmt.pix_mp.width = p_enc_config->pic_width;
        fmt.fmt.pix_mp.height = p_enc_config->pic_height;
        fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
        fmt.fmt.pix_mp.num_planes = 1;
        fmt.fmt.pix_mp.pixelformat = p_enc_config->pixel_format_out;
        fmt.fmt.pix_mp.plane_fmt[0].sizeimage = (p_enc_config->pic_width*p_enc_config->pic_height)*4;
        
        ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_FMT, &fmt);
        if(ret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: set output fmt \n\r", __func__));
        }
    }
    //v4l2_jpeg_set_ctrl(fd, V4L2_CID_CACHEABLE, 1);

    return ret;
}
#endif

/*
int v4l2_jpeg_enc_alloc_inbuf(int fd, V4L2_IN struct v4l2_jpeg_enc_config* p_enc_config, struct v4l2_jpeg_enc_buf *buf)
{
    int plane_num = 1;

    switch(p_enc_config->pixel_format_in) {
        case V4L2_PIX_FMT_RGB32:
        default :
            plane_num = 1;
    }

    return v4l2_jpeg_enc_alloc_buf(fd, V4L2_JPEG_BUF_TYPE_ENC_SRC, V4L2_JPEG_ENC_MEMORY_TYPE, plane_num, buf);
}
*/
/*
int v4l2_jpeg_enc_alloc_outbuf(int fd, V4L2_OUT struct v4l2_jpeg_enc_buf *buf)
{
    int plane_num = 1;
    
    return v4l2_jpeg_enc_alloc_buf(fd, V4L2_JPEG_BUF_TYPE_ENC_DST, V4L2_JPEG_ENC_MEMORY_TYPE, plane_num, buf);
}
*/

int v4l2_jpeg_enc_reqbuf_src(int fd, struct v4l2_ion_frame *p_ion_frame_src) {

    struct v4l2_requestbuffers reqbufobj;
    int ret = 0;
    
    memset(&reqbufobj, 0x00, sizeof(reqbufobj));
        
    reqbufobj.type = V4L2_JPEG_BUF_TYPE_ENC_SRC;
    reqbufobj.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
    reqbufobj.count = p_ion_frame_src->plane_count;

    ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &reqbufobj);
    if (ret < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req) \n\r", __func__ ));
    }
   
    return ret;
}

int v4l2_jpeg_enc_reqbuf_dst(int fd, struct v4l2_ion_buffer *p_ion_stream_dst) {

    struct v4l2_requestbuffers reqbufobj;
    int ret = 0;
    
    memset(&reqbufobj, 0x00, sizeof(reqbufobj));
        
    reqbufobj.type = V4L2_JPEG_BUF_TYPE_ENC_DST;
    reqbufobj.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
    reqbufobj.count = 1;

    ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &reqbufobj);
    if (ret < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: V4L2_DRIVER_IOCTL(fd, VIDIOC_REQBUFS, &req) \n\r", __func__ ));
    }
   
    return ret;
}



#if 1

int v4l2_jpeg_enc_exe(int fd, struct v4l2_ion_frame *p_ion_frame_src, struct v4l2_ion_buffer *p_ion_stream_dst) {

    struct v4l2_buffer v4l2_buf_src, v4l2_buf_dst;
    struct v4l2_plane plane_src[V4L2_JPEG_MAX_PLANE];
    struct v4l2_plane plane_dst[V4L2_JPEG_MAX_PLANE];
    int i;
    int ret = 0;

    memset(&v4l2_buf_src, 0, sizeof(struct v4l2_buffer));
    memset(plane_src, 0, (int)V4L2_JPEG_MAX_PLANE * sizeof(struct v4l2_plane));
    v4l2_buf_src.index = 0;
    v4l2_buf_src.type = V4L2_JPEG_BUF_TYPE_ENC_SRC;
    v4l2_buf_src.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
    v4l2_buf_src.length = p_ion_frame_src->plane_count;
    v4l2_buf_src.m.planes = plane_src;
    //if(inbuf->memory_type == V4L2_MEMORY_DMABUF) {
        for (i = 0; i < p_ion_frame_src->plane_count; i++) {
            v4l2_buf_src.m.planes[i].m.fd = p_ion_frame_src->plane[i].shared_fd;
            v4l2_buf_src.m.planes[i].length = p_ion_frame_src->plane[i].buf_size;
        }
    //}

    memset(&v4l2_buf_dst, 0, sizeof(struct v4l2_buffer));
    memset(plane_dst, 0, (int)V4L2_JPEG_MAX_PLANE * sizeof(struct v4l2_plane));
    v4l2_buf_dst.index = 0;
    v4l2_buf_dst.type = V4L2_JPEG_BUF_TYPE_ENC_DST;
    v4l2_buf_dst.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
    v4l2_buf_dst.length = 1;
    v4l2_buf_dst.m.planes = plane_dst;
    //if(v4l2_buf_dst.memory == V4L2_MEMORY_DMABUF) {
        //for (i = 0; i < outbuf->num_planes; i++) {
            v4l2_buf_dst.m.planes[0].m.fd = p_ion_stream_dst->shared_fd;
            v4l2_buf_dst.m.planes[0].length = p_ion_stream_dst->buf_size;
        //}
    //}
    
    /* add input buffer to queue */
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
        ret = v4l2_jpeg_stream_on(fd, V4L2_JPEG_BUF_TYPE_ENC_SRC);
    }
    if(ret == 0) {
        ret = v4l2_jpeg_stream_on(fd, V4L2_JPEG_BUF_TYPE_ENC_DST);
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

#else
int v4l2_jpeg_enc_exe(int fd, V4L2_IN struct v4l2_jpeg_enc_buf *inbuf, V4L2_IN struct v4l2_jpeg_enc_buf *outbuf) {

    struct v4l2_buffer v4l2_buf_src, v4l2_buf_dst;
    struct v4l2_plane plane_src[V4L2_JPEG_MAX_PLANE];
    struct v4l2_plane plane_dst[V4L2_JPEG_MAX_PLANE];
    int i;
    int ret = 0;

    memset(&v4l2_buf_src, 0, sizeof(struct v4l2_buffer));
    memset(plane_src, 0, (int)V4L2_JPEG_MAX_PLANE * sizeof(struct v4l2_plane));
    v4l2_buf_src.index = 0;
    v4l2_buf_src.type = V4L2_JPEG_BUF_TYPE_ENC_SRC;
    v4l2_buf_src.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
    v4l2_buf_src.length = inbuf->num_planes;
    v4l2_buf_src.m.planes = plane_src;
    if(inbuf->memory_type == V4L2_MEMORY_DMABUF) {
        for (i = 0; i < inbuf->num_planes; i++) {
            v4l2_buf_src.m.planes[i].m.fd = (__s32)inbuf->start[i];
            v4l2_buf_src.m.planes[i].length = inbuf->length[i];
        }
    }

    memset(&v4l2_buf_dst, 0, sizeof(struct v4l2_buffer));
    memset(plane_dst, 0, (int)V4L2_JPEG_MAX_PLANE * sizeof(struct v4l2_plane));
    v4l2_buf_dst.index = 0;
    v4l2_buf_dst.type = V4L2_JPEG_BUF_TYPE_ENC_DST;
    v4l2_buf_dst.memory = V4L2_JPEG_ENC_MEMORY_TYPE;
    v4l2_buf_dst.length = outbuf->num_planes;
    v4l2_buf_dst.m.planes = plane_dst;
    if(v4l2_buf_dst.memory == V4L2_MEMORY_DMABUF) {
        for (i = 0; i < outbuf->num_planes; i++) {
            v4l2_buf_dst.m.planes[i].m.fd = (__s32)outbuf->start[i];
            v4l2_buf_dst.m.planes[i].length = outbuf->length[i];
        }
    }
    
    /* add input buffer to queue */
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
        ret = v4l2_jpeg_stream_on(fd, V4L2_JPEG_BUF_TYPE_ENC_SRC);
    }
    if(ret == 0) {
        ret = v4l2_jpeg_stream_on(fd, V4L2_JPEG_BUF_TYPE_ENC_DST);
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

#endif