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

#include "v4l2_ion_tool.h"
#include "ion_api.h"
#include <fcntl.h>
#include <sys/mman.h>


int v4l2_ion_alloc_yuv_frame(int pic_width, int pic_height, unsigned int pix_fmt, struct v4l2_ion_frame* p_ion_frame) {

    int ion_fd, i;
    int iret;
    int stride;

    for(i = 0; i < V4L2_MAX_PLANE; i++) {
        p_ion_frame->plane[i].shared_fd = -1;
    }

    p_ion_frame->width = pic_width;
    p_ion_frame->height = pic_height;
    p_ion_frame->pix_fourcc = pix_fmt; 
    switch(pix_fmt) {
        case V4L2_PIX_FMT_YUV444_P1: 
            p_ion_frame->plane_count = 1;
            stride = V4L2_IMAGE_FRAME_STRIDE_ALIGN(pic_width*3);
            p_ion_frame->plane[0].stride = stride;
            p_ion_frame->plane[0].buf_size = stride * pic_height;
            break;

        case V4L2_PIX_FMT_NV16:  /* 16bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16 */
            p_ion_frame->plane_count = 2;
            
            /* Y buffer */
            stride = V4L2_IMAGE_FRAME_STRIDE_ALIGN(pic_width);
            p_ion_frame->plane[0].stride = stride;
            p_ion_frame->plane[0].buf_size = stride * pic_height; 
            
            /* CbCr buffer */
            p_ion_frame->plane[1].stride = p_ion_frame->plane[0].stride;
            p_ion_frame->plane[1].buf_size = p_ion_frame->plane[0].buf_size; 
            break;
        
        case V4L2_PIX_FMT_NV12:  /* 12bit Y/CbCr 4:2:0 Plane 2, V4L2_PIX_FMT_NV12 */
            p_ion_frame->plane_count = 2;

            /* Y buffer */
            stride = V4L2_IMAGE_FRAME_STRIDE_ALIGN(pic_width);
            p_ion_frame->plane[0].stride = stride;
            p_ion_frame->plane[0].buf_size = stride * pic_height; 
            
            /* CbCr buffer */
            stride = V4L2_IMAGE_FRAME_STRIDE_ALIGN(pic_width);
            p_ion_frame->plane[1].stride = stride;
            p_ion_frame->plane[1].buf_size = stride * (pic_height/2); 
            break;
        
        case V4L2_PIX_FMT_YUV420: 
            p_ion_frame->plane_count = 3;

            /* Y buffer */
            stride = V4L2_IMAGE_FRAME_STRIDE_ALIGN(pic_width);
            p_ion_frame->plane[0].stride = stride;
            p_ion_frame->plane[0].buf_size = stride * pic_height; 
            
            /* U buffer */
            stride = V4L2_IMAGE_FRAME_STRIDE_ALIGN(pic_width/2);
            p_ion_frame->plane[1].stride = stride;
            p_ion_frame->plane[1].buf_size = stride * (pic_height/2); 
            
            /* V buffer */
            p_ion_frame->plane[2].stride = p_ion_frame->plane[1].stride;
            p_ion_frame->plane[2].buf_size = p_ion_frame->plane[1].buf_size; 
            
            break;
    }
            
    ion_fd = ion_open();
    if(ion_fd >= 0) {
        for(i = 0; i < p_ion_frame->plane_count; i++) {
            iret = ion_alloc_fd(ion_fd, p_ion_frame->plane[i].buf_size, 0, ION_HEAP_CARVEOUT_MASK,  0, &p_ion_frame->plane[i].shared_fd);
            if(iret < 0) {
                //fprintf(stderr, "FAIL:  ion_alloc_fd i=%d bufsz=%d \n", i, p_ion_frame->plane[i].buf_size);
                break;
            }
            else {
                p_ion_frame->plane[i].vir_addr = (unsigned int)V4L2_DRIVER_MMAP(NULL, p_ion_frame->plane[i].buf_size, (PROT_READ | PROT_WRITE), MAP_SHARED, p_ion_frame->plane[i].shared_fd, 0);
                if(p_ion_frame->plane[i].vir_addr == 0) {
                    iret = -1;
                    //fprintf(stderr, "FAIL:  ion mmap i=%d bufsz=%d \n", i, p_ion_frame->plane[i].buf_size);
                    break;
                }
            }
        } /* end of for(i = 0; i < ion_plane_yuv.plane_count; i++) { */

        ion_close(ion_fd);
    } /* end of if(ion_fd >= 0) { */

    if(iret != 0) {
        for(i = 0; i < p_ion_frame->plane_count; i++) {
            if(p_ion_frame->plane[i].shared_fd >= 0) {
                V4L2_DRIVER_CLOSE(p_ion_frame->plane[i].shared_fd);
            }
        }

        for(i = 0; i < V4L2_MAX_PLANE; i++) {
            p_ion_frame->plane[i].shared_fd = -1;
        }
    }

    return iret;
}

void v4l2_ion_free_yuv_frame(struct v4l2_ion_frame* p_ion_frame) {
    
    int i;

    for(i = 0; i < p_ion_frame->plane_count; i++) {
    
        if(p_ion_frame->plane[i].shared_fd >= 0) {
            if(p_ion_frame->plane[i].vir_addr != 0) {
                V4L2_DRIVER_MUNMAP((void*)p_ion_frame->plane[i].vir_addr, p_ion_frame->plane[i].buf_size);
            }
            V4L2_DRIVER_CLOSE(p_ion_frame->plane[i].shared_fd);
        }
    }
}

int v4l2_ion_alloc_stream(int pic_width, int pic_height, struct v4l2_ion_buffer* p_ion_buf) {

    int ion_fd;
    int iret = 0;
    int buf_size, shared_fd = -1;
    unsigned int vir_addr;

    p_ion_buf->shared_fd = -1;
    buf_size = V4L2_IMAGE_STREAM_SIZE_ALIGN(pic_width*pic_height);
    
    /* alloc stream ion buf*/
    ion_fd = ion_open();
    if(ion_fd >= 0) {
        
        iret = ion_alloc_fd(ion_fd, buf_size, 0, ION_HEAP_CARVEOUT_MASK,  0, &shared_fd);
        if(iret < 0) {
            
        }
        else {
            vir_addr = (unsigned int)V4L2_DRIVER_MMAP(NULL, buf_size, PROT_READ, MAP_SHARED, shared_fd, 0);
            if(vir_addr == 0) {
                iret = -1;
            }
            else {
                p_ion_buf->shared_fd = shared_fd;
                p_ion_buf->vir_addr = vir_addr;
                p_ion_buf->buf_size = buf_size;
                p_ion_buf->mem_offset = 0;
            }
        }
        
        ion_close(ion_fd);
    } /* end of if(ion_fd >= 0) { */
    else {
        iret = -1;
    }

    if(iret != 0) {
        p_ion_buf->shared_fd = -1;
    }

    return iret;
}

void v4l2_ion_free_stream(struct v4l2_ion_buffer* p_ion_buf) {
    
    if(p_ion_buf->shared_fd >= 0) {
        if(p_ion_buf->vir_addr != 0) {
            V4L2_DRIVER_MUNMAP((void*)p_ion_buf->vir_addr, p_ion_buf->buf_size);
        }
        V4L2_DRIVER_CLOSE(p_ion_buf->shared_fd);
    }

}
