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
#include <fcntl.h>
#include <sys/mman.h>
#include "ion_api.h"


int v4l2_jpeg_encode_ion(struct v4l2_ion_frame *p_ion_frame_src, int quality /* rage 0~100 */, struct v4l2_ion_buffer *p_ion_stream_dst, int* jpegsize) {

    int v4l2_fd = -1, iret = 0, ion_fd = -1, i;
        
    /* open v4l2-jpu */
    if(iret == 0) {
        v4l2_fd = v4l2_jpeg_enc_open();
        if(v4l2_fd < 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_enc_open \n\r", __func__));
            iret = -1;
        }
        else {
            v4l2_jpeg_querycap(v4l2_fd);
        }
    }

    /* set config */
    if(iret == 0) {
        iret = v4l2_jpeg_enc_set_config(v4l2_fd, p_ion_frame_src, quality, p_ion_stream_dst);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_enc_set_config \n\r", __func__));
        }
    }

    /* reqbuf src */
    if(iret == 0) {
        iret = v4l2_jpeg_enc_reqbuf_src(v4l2_fd, p_ion_frame_src);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_enc_reqbuf_src \n\r", __func__));
        }
    }

    /* reqbuf dst */
    if(iret == 0) {
        iret = v4l2_jpeg_enc_reqbuf_dst(v4l2_fd, p_ion_stream_dst);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_enc_reqbuf_dst \n\r", __func__));
        }
    }
    
    /* run dec */
    if(iret == 0) {
    
        iret = v4l2_jpeg_enc_exe(v4l2_fd, p_ion_frame_src, p_ion_stream_dst);
        if(iret == 0) {

            i = v4l2_jpeg_get_ctrl(v4l2_fd, V4L2_JEPG_CID_ENC_GET_COMPRESSION_SIZE);
            if(i < 0) {
                JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL : jpu_v4l2_get_ctrl(v4l2_fd, V4L2_JEPG_CID_ENC_GET_COMPRESSION_SIZE) \n\r", __func__ ));
                iret = -1;
            }
            else {
                if(jpegsize != NULL) *jpegsize = i;
            }

        }
        else {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL : jpu_v4l2_enc_exe \n\r", __func__ ));
        }
    }

    if(v4l2_fd >= 0) {
        v4l2_jpeg_enc_close(v4l2_fd);
    }
    

    return iret;
}

int v4l2_jpeg_decode_ion(struct v4l2_ion_buffer *p_ion_stream_src, int jpeg_size, struct v4l2_ion_frame *p_ion_frame_dst) {

    int iret = 0;
    int v4l2_fd = -1;

    /* open v4l2-fd */
    if(iret == 0) {

        v4l2_fd = v4l2_jpeg_dec_open();
        if(v4l2_fd < 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_dec_open \n\r", __func__));
            iret = -1;
        }
        else {
            v4l2_jpeg_querycap(v4l2_fd);
        }
    }

    /* set config */
    if(iret == 0) {
        iret = v4l2_jpeg_dec_set_config(v4l2_fd, p_ion_stream_src, jpeg_size, p_ion_frame_dst);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_dec_set_config \n\r", __func__));
        }
    }

    /* reqbuf src */
    if(iret == 0) {
        iret = v4l2_jpeg_dec_reqbuf_src(v4l2_fd, p_ion_stream_src);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_dec_reqbuf_src \n\r", __func__));
        }
    }

    /* reqbuf dst */
    if(iret == 0) {
        iret = v4l2_jpeg_dec_reqbuf_dst(v4l2_fd, p_ion_frame_dst);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_dec_reqbuf_dst \n\r", __func__));
        }
    }
    
    /* run dec */
    if(iret == 0) {
    
        iret = v4l2_jpeg_set_ctrl(v4l2_fd, V4L2_JEPG_CID_DEC_SET_COMPRESSION_SIZE, jpeg_size);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_set_ctrl(jpegsize) \n\r", __func__));
        }
        else {
            iret = v4l2_jpeg_dec_exe(v4l2_fd, p_ion_stream_src, p_ion_frame_dst);
            if(iret == 0) {

            }
            else {
                JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL : jpu_v4l2_enc_exe \n\r", __func__ ));
            }
        }
    
    }

    if(v4l2_fd >= 0) {
        v4l2_jpeg_dec_close(v4l2_fd);
    }
    

    return iret;
}


#if 0
int v4l2_jpeg_encode_ion_heap(struct v4l2_ion_frame *p_ion_frame_src, int quality /* rage 0~100 */, 
                         unsigned char* jpegdata, int jpegdata_max_size, int* jpegsize) {

    int v4l2_fd = -1, iret = 0, ion_fd = -1, iret1, i;
    struct v4l2_jpeg_enc_config enc_cfg;
    struct v4l2_jpeg_enc_buf enc_inbuf;
    struct v4l2_jpeg_enc_buf enc_outbuf;
    struct v4l2_ion_buffer ion_buf_stream;

    memset(&enc_inbuf, 0x00, sizeof(enc_inbuf));
    memset(&enc_outbuf, 0x00, sizeof(enc_outbuf));
    memset(&ion_buf_stream, 0x00, sizeof(ion_buf_stream));
    ion_buf_stream.shared_fd = -1;
    ion_buf_stream.buf_size = V4L2_BYTE_ALIGN(p_ion_frame_src->width * p_ion_frame_src->height, 1024);

    /* alloc stream ion buf*/
    if(iret == 0) {
        ion_fd = ion_open();
        if(ion_fd >= 0) {
            
            iret1 = ion_alloc_fd(ion_fd, ion_buf_stream.buf_size, 0, ION_HEAP_CARVEOUT_MASK,  0, &ion_buf_stream.shared_fd);
            if(iret1 < 0) {
                iret = -1;
                JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL: ion_alloc_fd bufsz=%d \n\r", ion_buf_stream.buf_size));
            }
            else {
                ion_buf_stream.vir_addr = (unsigned int)V4L2_DRIVER_MMAP(NULL, ion_buf_stream.buf_size, PROT_READ, MAP_SHARED, ion_buf_stream.shared_fd, 0);
                if(ion_buf_stream.vir_addr == 0) {
                    iret = -1;
                    JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL: ion mmap bufsz=%d \n\r", ion_buf_stream.buf_size));
                }
            }
            
            ion_close(ion_fd);
        } /* end of if(ion_fd >= 0) { */
        else {
            iret = -1;
            JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL: ion_open \n\r"));
        }
    }

    /* open v4l2-jpu */
    if(iret == 0) {
        v4l2_fd = v4l2_jpeg_enc_open();
        if(v4l2_fd < 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL: v4l2_jpeg_enc_open \n\r"));
            iret = -1;
        }
        else {
            v4l2_jpeg_querycap(v4l2_fd);
        }
    }

    /* set config */
    if(iret == 0) {

        memset(&enc_cfg, 0x00, sizeof(enc_cfg));
        enc_cfg.pic_width = p_ion_frame_src->width;
        enc_cfg.pic_height = p_ion_frame_src->height;
        enc_cfg.quality = quality;
        enc_cfg.pixel_format_in = p_ion_frame_src->pix_fmt;
        enc_cfg.pixel_format_out = V4L2_PIX_FMT_JPEG_420;
        iret = v4l2_jpeg_enc_set_config(v4l2_fd, &enc_cfg);
        if(iret != 0) {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL: v4l2_jpeg_enc_set_config \n\r"));
        }
    }

    /* alloc inbuf */
    if(iret == 0) {
        iret = v4l2_jpeg_enc_alloc_inbuf(v4l2_fd, &enc_cfg, &enc_inbuf);
        if(iret == 0) {
        
            enc_inbuf.memory_type = V4L2_MEMORY_DMABUF;
            enc_inbuf.num_planes = 1;
            enc_inbuf.start[0] = (void*)p_ion_frame_src->plane[0].shared_fd;
            enc_inbuf.length[0] = p_ion_frame_src->plane[0].buf_size;
        }
    }

    /* alloc outbuf */
    if(iret == 0) {
        iret = v4l2_jpeg_enc_alloc_outbuf(v4l2_fd, &enc_outbuf);
        if(iret == 0) {
            enc_outbuf.memory_type = V4L2_MEMORY_DMABUF;
            enc_outbuf.num_planes = 1;
            enc_outbuf.start[0] = (void*)ion_buf_stream.shared_fd;
            enc_outbuf.length[0] = ion_buf_stream.buf_size;
        }
    }

    /* run dec */
    if(iret == 0) {
    
        iret = v4l2_jpeg_enc_exe(v4l2_fd, &enc_inbuf, &enc_outbuf);
        if(iret == 0) {

            i = v4l2_jpeg_get_ctrl(v4l2_fd, V4L2_JEPG_CID_ENC_GET_COMPRESSION_SIZE);
            if(i < 0) {
                JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL : jpu_v4l2_get_ctrl(v4l2_fd, V4L2_JEPG_CID_ENC_GET_COMPRESSION_SIZE) \n\r" ));
                iret = -1;
            }
            else {

                if(i <= jpegdata_max_size)  {
                    
                    if(jpegsize!=NULL) *jpegsize = i;
                    memcpy(jpegdata, (void*)ion_buf_stream.vir_addr, i);
                    JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] SUCCESS : jpeg_size = %d \n\r", i));
                }
                else {
                   JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL : jpegbuf size is too small (jpegsz=%d bufsz=%d) \n\r", i, jpegdata_max_size)); 
                }
            }

        }
        else {
            JPUDEBUGMSG(JPUZONE_ERROR, ("[v4l2_jpeg_encode_ion] FAIL : jpu_v4l2_enc_exe \n\r"));
        }
    }

    if(v4l2_fd >= 0) {
        v4l2_jpeg_enc_close(v4l2_fd, &enc_inbuf, &enc_outbuf);
    }
    
    if(ion_buf_stream.shared_fd >= 0) {

        if(ion_buf_stream.vir_addr != 0) {
            V4L2_DRIVER_MUNMAP((void*)ion_buf_stream.vir_addr, ion_buf_stream.buf_size );
        }
        V4L2_DRIVER_CLOSE(ion_buf_stream.shared_fd);
    }

    return iret;
}
#endif
