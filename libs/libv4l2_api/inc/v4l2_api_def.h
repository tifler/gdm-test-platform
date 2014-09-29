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

#ifndef V4L2_API_DEF_H__
#define V4L2_API_DEF_H__

#include <linux/videodev2.h>

/*****************************************************************************
      Pixel format          FOURCC                     depth  Description  
*****************************************************************************/

#define V4L2_PIX_FMT_YUV444_P1 v4l2_fourcc('4','4','4','1')  /* 24bit Y/U/V 444 Plane 1 */

/*****************************************************************************
      Structure & Define
*****************************************************************************/

#define V4L2_BYTE_ALIGN(x, align)   (((x) + (align) - 1) & ~((align) - 1))

#define V4L2_VIDEO_FRAME_STRIDE_ALIGN(stride)  V4L2_BYTE_ALIGN(stride, 16) 
#define V4L2_VIDEO_STREAM_SIZE_ALIGN(sz)       V4L2_BYTE_ALIGN(sz, 1024) 
#define V4L2_IMAGE_FRAME_STRIDE_ALIGN(stride)  V4L2_VIDEO_FRAME_STRIDE_ALIGN(stride)
#define V4L2_IMAGE_STREAM_SIZE_ALIGN(sz)       V4L2_VIDEO_STREAM_SIZE_ALIGN(sz)


#define V4L2_MAX_PLANE 3
#define V4L2_IN 
#define V4L2_OUT
#define V4L2_INOUT


struct v4l2_ion_buffer {
    int shared_fd;
    int buf_size;
    unsigned int vir_addr;
    int mem_offset;
    int stride;
};

struct v4l2_ion_frame {
    int width;
    int height;
    unsigned int pix_fourcc;
    int plane_count;
    struct v4l2_ion_buffer plane[V4L2_MAX_PLANE];
};


/*****************************************************************************
   Driver IO OAL
****************************************************************************/
#ifdef WIN32
#define V4L2_DRIVER_OPEN  kernel_driver_open
#define V4L2_DRIVER_CLOSE kernel_driver_close
#define V4L2_DRIVER_WRITE kernel_driver_write
#define V4L2_DRIVER_IOCTL kernel_driver_ioctl
#define V4L2_DRIVER_MMAP  kernel_driver_mmap
#define V4L2_DRIVER_MUNMAP  kernel_driver_munmap

#ifdef __cplusplus
#define V4L2_FUNC_EXTERN extern "C" 
#else
#define V4L2_FUNC_EXTERN extern
#endif
V4L2_FUNC_EXTERN int kernel_driver_open(const char* drvname, int flag);
V4L2_FUNC_EXTERN int kernel_driver_close(int fd);
V4L2_FUNC_EXTERN int kernel_driver_write(int fd, char* buf, int bufsize);
V4L2_FUNC_EXTERN int kernel_driver_ioctl(int d, unsigned long request, void* arg);
V4L2_FUNC_EXTERN void* kernel_driver_mmap(void *addr, size_t length, int prot, int flags, int fd, unsigned int offset);
V4L2_FUNC_EXTERN int kernel_driver_munmap(void* start, size_t length);

#else
#define V4L2_DRIVER_OPEN    open
#define V4L2_DRIVER_CLOSE   close
#define V4L2_DRIVER_WRITE   write
#define V4L2_DRIVER_MMAP    mmap
#define V4L2_DRIVER_MUNMAP  munmap
#define V4L2_DRIVER_IOCTL   ioctl

#endif

#endif /* end of #ifndef V4L2_API_DEF_H__ */
