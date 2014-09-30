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

#include "mmp_buffer_mgr_ex1.hpp"
#include "MmpUtil.hpp"
#include "mmp_lock.hpp"

mmp_buffer_mgr_ex1::mmp_buffer_mgr_ex1() :
m_p_mutex(NULL)
{

}

mmp_buffer_mgr_ex1::~mmp_buffer_mgr_ex1() {

}

MMP_RESULT mmp_buffer_mgr_ex1::open() {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    MMPDEBUGMSG(1, (TEXT("[mmp_buffer_mgr_ex1::open]  ")));

    m_p_mutex = mmp_oal_mutex::create_object();
    if(m_p_mutex == NULL) {
        mmpResult = MMP_FAILURE;    
    }

    return mmpResult;
}

MMP_RESULT mmp_buffer_mgr_ex1::close() {

    bool flag;
    class mmp_buffer* p_mmp_buffer;

    flag = m_list_buffer.GetFirst(p_mmp_buffer);
    while(flag) {
        mmp_buffer::destroy_object(p_mmp_buffer);
        flag = m_list_buffer.GetNext(p_mmp_buffer);
    }
    m_list_buffer.Del_All();

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
        m_p_mutex = NULL;
    }

    MMPDEBUGMSG(1, (TEXT("[mmp_buffer_mgr_ex1::close]  ")));

    return MMP_SUCCESS;
}

class mmp_buffer* mmp_buffer_mgr_ex1::alloc_dma_buffer(MMP_S32 buffer_size) {

    class mmp_lock autolock(m_p_mutex);

    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer* p_mmp_buffer;

    memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
    buffer_create_config.type = mmp_buffer::ION;
    buffer_create_config.size = buffer_size;

    p_mmp_buffer = mmp_buffer::create_object(&buffer_create_config);
    if(p_mmp_buffer != NULL) {
        m_list_buffer.Add(p_mmp_buffer);
    }

    return p_mmp_buffer;
}

class mmp_buffer* mmp_buffer_mgr_ex1::attach_dma_buffer(class mmp_buffer_addr buf_addr) {

    class mmp_lock autolock(m_p_mutex);

    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer* p_mmp_buffer;

    buffer_create_config.type = mmp_buffer::ION_ATTACH;
    buffer_create_config.size = buf_addr.m_size;
    buffer_create_config.attach_shared_fd = buf_addr.m_shared_fd;
    buffer_create_config.attach_phy_addr = buf_addr.m_phy_addr;
    buffer_create_config.attach_offset = 0;

    p_mmp_buffer = mmp_buffer::create_object(&buffer_create_config);
    if(p_mmp_buffer != NULL) {
        m_list_buffer.Add(p_mmp_buffer);
    }

    return p_mmp_buffer;
}

MMP_RESULT mmp_buffer_mgr_ex1::free_buffer(class mmp_buffer* p_mmp_buffer) {

    class mmp_lock autolock(m_p_mutex);


    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer_temp;
    MMP_S32 idx;

    idx = 0;
    flag = m_list_buffer.GetFirst(p_mmp_buffer_temp);
    while(flag) {
        
        if(p_mmp_buffer_temp == p_mmp_buffer) {
             mmp_buffer::destroy_object(p_mmp_buffer);
             mmpResult = MMP_SUCCESS;
             break;
        }
        flag = m_list_buffer.GetNext(p_mmp_buffer_temp);
        idx++;
    }
    m_list_buffer.Del(idx);
    
    return MMP_FAILURE;
}

MMP_RESULT mmp_buffer_mgr_ex1::free_buffer(class mmp_buffer_addr buf_addr) {

    class mmp_lock autolock(m_p_mutex);


    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer_temp;
    MMP_S32 idx;

    idx = 0;
    flag = m_list_buffer.GetFirst(p_mmp_buffer_temp);
    while(flag) {
        
        if( (p_mmp_buffer_temp->get_phy_addr() == buf_addr.m_phy_addr) 
            && (p_mmp_buffer_temp->get_vir_addr() == buf_addr.m_vir_addr) 
            )
        {
                         
             mmp_buffer::destroy_object(p_mmp_buffer_temp);
             mmpResult = MMP_SUCCESS;
             break;
        }
        flag = m_list_buffer.GetNext(p_mmp_buffer_temp);
        idx++;
    }
    m_list_buffer.Del(idx);
    
    return MMP_FAILURE;
}

class mmp_buffer* mmp_buffer_mgr_ex1::get_buffer(MMP_S32 shared_fd) {

    class mmp_lock autolock(m_p_mutex);

    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer = NULL;
    class mmp_buffer* p_mmp_buffer_temp;

    flag = m_list_buffer.GetFirst(p_mmp_buffer_temp);
    while(flag) {
        
        if(p_mmp_buffer_temp->get_buf_shared_fd() == shared_fd) {
             p_mmp_buffer = p_mmp_buffer_temp;
             break;
        }
        flag = m_list_buffer.GetNext(p_mmp_buffer_temp);
    }
    
    return p_mmp_buffer;
}

class mmp_buffer_addr mmp_buffer_mgr_ex1::get_buffer_addr(MMP_S32 shared_fd) {

    class mmp_buffer* p_mmp_buf;
    class mmp_buffer_addr buf_addr;

    p_mmp_buf = this->get_buffer(shared_fd);
    if(p_mmp_buf != NULL) {
        buf_addr = p_mmp_buf->get_buf_addr();
    }

    return buf_addr;
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::alloc_media_videoframe(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U32 fourcc, 
                                                                        MMP_U32 type, MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset) {

    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    MMP_S32 err_cnt = 0;

    class mmp_buffer_videoframe* p_mmp_videoframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 plane_size[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 stride[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 buffer_height_arr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 buffer_width, buffer_height, luma_size, chroma_size;
    MMP_S32 i;

    if(type == mmp_buffer::ION_ATTACH) {
        if( (shared_ion_fd == NULL) || (ion_mem_offset == NULL) ) {
            /* Error  */
            err_cnt++;
        }
    }

    if(err_cnt == 0) {
        
        p_mmp_videoframe = new class mmp_buffer_videoframe;
        if(p_mmp_videoframe != NULL) {
        
            p_mmp_videoframe->m_fourcc = fourcc;
            p_mmp_videoframe->m_pic_width = pic_width;
            p_mmp_videoframe->m_pic_height = pic_height;
            
            
            switch(p_mmp_videoframe->m_fourcc) {

                case MMP_FOURCC_IMAGE_YUV420_P3: 
                    p_mmp_videoframe->m_plane_count = 3;        

                    buffer_width = MMP_BYTE_ALIGN(p_mmp_videoframe->m_pic_width, 16);
                    buffer_height = MMP_BYTE_ALIGN(p_mmp_videoframe->m_pic_height, 16);
                    
                    stride[0] = buffer_width;
                    stride[1] = buffer_width>>1;
                    stride[2] = buffer_width>>1;
                
                    buffer_height_arr[0] = buffer_height;
                    buffer_height_arr[1] = buffer_height>>1;
                    buffer_height_arr[2] = buffer_height>>1;
                    
                    luma_size = buffer_width*buffer_height;
                    chroma_size = luma_size>>2;

                    plane_size[0] = luma_size;
                    plane_size[1] = chroma_size;
                    plane_size[2] = chroma_size;

                    break;

                default:
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_buffer_mgr_ex1::attach_media_videoframe] FAIL: not supported video format %c%c%c%c "), MMPGETFOURCCARG(p_mmp_videoframe->m_fourcc) ));
                    break;
            }

            /* alloc mmp buffer per plane */
            if(mmpResult == MMP_SUCCESS) {

                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    
                    buffer_create_config.type = type;
                    buffer_create_config.size = plane_size[i];

                    if(type == mmp_buffer::ION_ATTACH) {
                        buffer_create_config.attach_shared_fd = shared_ion_fd[i];
                        buffer_create_config.attach_phy_addr = 0;
                        buffer_create_config.attach_offset = ion_mem_offset[i];
                    }

                    p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
                    if(p_mmp_buf == NULL) {
                        mmpResult = MMP_FAILURE;
                        break;
                    }
                    else {
                        p_mmp_videoframe->m_p_mmp_buffer[i] = p_mmp_buf;
                        p_mmp_videoframe->m_buf_stride[i] = stride[i];
                        p_mmp_videoframe->m_buf_height[i] = buffer_height_arr[i];
                    }
                }
            }

        }
        else {
            /* FAIL: new instance */
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {
        
            m_p_mutex->lock();

            /* register mmp buffer */
            for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                m_list_buffer.Add(p_mmp_videoframe->m_p_mmp_buffer[i]);
            }

            m_p_mutex->unlock();

        }
        else {
            
            if(p_mmp_videoframe != NULL) {
        
                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    if(p_mmp_videoframe->m_p_mmp_buffer[i] != NULL) {
                        mmp_buffer::destroy_object(p_mmp_videoframe->m_p_mmp_buffer[i]);
                    }
                }

                delete p_mmp_videoframe;
                p_mmp_videoframe = NULL;
            }
        }

    }
    
    return p_mmp_videoframe;
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::alloc_media_videoframe(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U32 format) {
    
    return this->alloc_media_videoframe(pic_width, pic_height, format, mmp_buffer::ION, NULL, NULL);
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::attach_media_videoframe(MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset, MMP_S32 pic_width, MMP_S32 pic_height, MMP_U32 format) {

    return this->alloc_media_videoframe(pic_width, pic_height, format, mmp_buffer::ION_ATTACH, shared_ion_fd, ion_mem_offset);
}

class mmp_buffer_videoframe* alloc_media_videoframe(MMP_S32 pic_width, MMP_S32 pic_height);

class mmp_buffer_videoframe* attach_media_videoframe(MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset);


class mmp_buffer_videostream* mmp_buffer_mgr_ex1::alloc_media_videostream(MMP_S32 stream_max_size, MMP_U32 buf_type, MMP_U8* p_stream_data) {

    MMP_S32 err_cnt = 0;
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer_videostream* p_mmp_videostream = NULL;

    if(buf_type == mmp_buffer::HEAP_ATTACH) {
        if(p_stream_data==NULL) {
            err_cnt++;        
        }
    }
        
    if(err_cnt == 0) {
        p_mmp_videostream = new class mmp_buffer_videostream;
        if(p_mmp_videostream != NULL) {

            memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
                
            buffer_create_config.type = buf_type;
            buffer_create_config.size = stream_max_size;

            if(buf_type == mmp_buffer::HEAP_ATTACH) {
                buffer_create_config.attach_vir_addr = (MMP_U32)p_stream_data;
                buffer_create_config.attach_offset = 0;
            }

            p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
            if(p_mmp_buf != NULL) {

                m_p_mutex->lock();
        
                p_mmp_videostream->m_p_mmp_buffer = p_mmp_buf;
                m_list_buffer.Add(p_mmp_videostream->m_p_mmp_buffer);
                
                m_p_mutex->unlock();
            }
        }
    }
    
    return p_mmp_videostream;
}

class mmp_buffer_videostream* mmp_buffer_mgr_ex1::alloc_media_videostream(MMP_S32 stream_max_size, MMP_U32 buf_type) {

    return this->alloc_media_videostream(stream_max_size, buf_type, NULL);
}

class mmp_buffer_videostream* mmp_buffer_mgr_ex1::attach_media_videostream(MMP_U8* p_stream_data, MMP_S32 stream_size) {
    return this->alloc_media_videostream(stream_size, mmp_buffer::HEAP_ATTACH, p_stream_data);
}

class mmp_buffer_imagestream* mmp_buffer_mgr_ex1::alloc_media_imagestream(MMP_S32 stream_max_size, MMP_U32 buf_type) {

    MMP_S32 err_cnt = 0;
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer_imagestream* p_mmp_imagestream = NULL;

    //if(buf_type == mmp_buffer::HEAP_ATTACH) {
    //    if(p_stream_data==NULL) {
    //        err_cnt++;        
    //    }
    //}
        
    if(err_cnt == 0) {
        p_mmp_imagestream = new class mmp_buffer_imagestream;
        if(p_mmp_imagestream != NULL) {

            memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
                
            buffer_create_config.type = buf_type;
            buffer_create_config.size = stream_max_size;

            //if(buf_type == mmp_buffer::HEAP_ATTACH) {
            //    buffer_create_config.attach_vir_addr = (MMP_U32)p_stream_data;
            //    buffer_create_config.attach_offset = 0;
            //}

            p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
            if(p_mmp_buf != NULL) {

                m_p_mutex->lock();
        
                p_mmp_imagestream->m_p_mmp_buffer = p_mmp_buf;
                m_list_buffer.Add(p_mmp_imagestream->m_p_mmp_buffer);
                
                m_p_mutex->unlock();
            }
        }
    }
    
    return p_mmp_imagestream;
}

class mmp_buffer_imagestream* mmp_buffer_mgr_ex1::alloc_media_imagestream(MMP_CHAR* image_file_name, MMP_U32 buf_type) {

    FILE* fp;
    class mmp_buffer_imagestream* p_buf_imagestream = NULL;
    MMP_S32 file_size, rdsz;

    fp = fopen(image_file_name, "rb");
    if(fp != NULL) {
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        p_buf_imagestream = this->alloc_media_imagestream(MMP_BYTE_ALIGN(file_size, 1024), buf_type);
        if(p_buf_imagestream != NULL) {
            rdsz = fread((void*)p_buf_imagestream->get_buf_vir_addr(), 1, file_size, fp);
            if(rdsz != file_size) {
                this->free_media_buffer(p_buf_imagestream);
                p_buf_imagestream = NULL;
            }
            else {
                p_buf_imagestream->sync_buf();
                p_buf_imagestream->set_stream_size(file_size);
                p_buf_imagestream->m_fp_imagefile = fp;
            }
        }
    }
    
    return p_buf_imagestream;
}

class mmp_buffer_imageframe* mmp_buffer_mgr_ex1::alloc_media_imageframe(MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc, 
                                                                        MMP_U32 type, MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset) {

    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    MMP_S32 err_cnt = 0;

    class mmp_buffer_imageframe* p_mmp_imageframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 buffer_stride[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 buffer_height_arr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 i;

    if(type == mmp_buffer::ION_ATTACH) {
        if( (shared_ion_fd == NULL) || (ion_mem_offset == NULL) ) {
            /* Error  */
            err_cnt++;
        }
    }

    if(err_cnt == 0) {
        
        p_mmp_imageframe = new class mmp_buffer_imageframe;
        if(p_mmp_imageframe != NULL) {
        
            p_mmp_imageframe->m_fourcc = fourcc;
            p_mmp_imageframe->m_pic_width = pic_width;
            p_mmp_imageframe->m_pic_height = pic_height;
            
            
            switch(p_mmp_imageframe->m_fourcc) {

                case MMP_FOURCC_IMAGE_YVU420_P3: 
                case MMP_FOURCC_IMAGE_YUV420_P3: 
                    p_mmp_imageframe->m_plane_count = 3;        

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width);
                      /* Note : gdm_dss suppose that chroma stride is half of luma stride */
                    buffer_stride[1] = buffer_stride[0]>>1;//MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width/2);
                    buffer_stride[2] = buffer_stride[1];
                
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    buffer_height_arr[1] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height/2);
                    buffer_height_arr[2] = buffer_height_arr[1];
                    
                    break;

                case MMP_FOURCC_IMAGE_YCbCr422_P2:  /* 16 bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16 */
                case MMP_FOURCC_IMAGE_YCrCb422_P2:  /* 16 bit Y/CrCb 4:2:2 Plane 2, V4L2_PIX_FMT_NV61 */ 
    
                    p_mmp_imageframe->m_plane_count = 2;        

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width);
                    buffer_stride[1] = buffer_stride[0];
                                    
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    buffer_height_arr[1] = buffer_height_arr[0];
                    
                    break;
               
               case MMP_FOURCC_IMAGE_YUV444_P1:
                    p_mmp_imageframe->m_plane_count = 1;

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width*3);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

               case MMP_FOURCC_IMAGE_GREY:  /*  8  Greyscale     */
                    p_mmp_imageframe->m_plane_count = 1;

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

               case MMP_FOURCC_IMAGE_RGBA8888:  /* RGB 32 Bit*/
                    p_mmp_imageframe->m_plane_count = 1;
                    
                    buffer_stride[0] = MMP_BYTE_ALIGN(p_mmp_imageframe->m_pic_width*4, 4);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

                case MMP_FOURCC_IMAGE_BGR888:
                case MMP_FOURCC_IMAGE_RGB888:  /* RGB 24 Bit*/
                    p_mmp_imageframe->m_plane_count = 1;
                    
                    buffer_stride[0] = MMP_BYTE_ALIGN(p_mmp_imageframe->m_pic_width*3, 4);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

                default:
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL: not supported video format %c%c%c%c "), MMP_CLASS_NAME, MMP_CLASS_FUNC,
                                              MMPGETFOURCCARG(p_mmp_imageframe->m_fourcc) ));
                    break;
            }

            /* alloc mmp buffer per plane */
            if(mmpResult == MMP_SUCCESS) {

                for(i = 0; i < p_mmp_imageframe->m_plane_count; i++) {
                    
                    buffer_create_config.type = type;
                    buffer_create_config.size = buffer_stride[i] * buffer_height_arr[i];

                    if(type == mmp_buffer::ION_ATTACH) {
                        buffer_create_config.attach_shared_fd = shared_ion_fd[i];
                        buffer_create_config.attach_phy_addr = 0;
                        buffer_create_config.attach_offset = ion_mem_offset[i];
                    }

                    p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
                    if(p_mmp_buf == NULL) {
                        mmpResult = MMP_FAILURE;
                        break;
                    }
                    else {
                        p_mmp_imageframe->m_p_mmp_buffer[i] = p_mmp_buf;
                        p_mmp_imageframe->m_buf_stride[i] = buffer_stride[i];
                        p_mmp_imageframe->m_buf_height[i] = buffer_height_arr[i];
                    }
                }
            }

        }
        else {
            /* FAIL: new instance */
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {
        
            m_p_mutex->lock();

            /* register mmp buffer */
            for(i = 0; i < p_mmp_imageframe->m_plane_count; i++) {
                m_list_buffer.Add(p_mmp_imageframe->m_p_mmp_buffer[i]);
            }

            m_p_mutex->unlock();

        }
        else {
            
            if(p_mmp_imageframe != NULL) {
        
                for(i = 0; i < p_mmp_imageframe->m_plane_count; i++) {
                    if(p_mmp_imageframe->m_p_mmp_buffer[i] != NULL) {
                        mmp_buffer::destroy_object(p_mmp_imageframe->m_p_mmp_buffer[i]);
                    }
                }

                delete p_mmp_imageframe;
                p_mmp_imageframe = NULL;
            }
        }

    }
    
    return p_mmp_imageframe;
}

class mmp_buffer_imageframe* mmp_buffer_mgr_ex1::alloc_media_imageframe(MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc) {
    
    return this->alloc_media_imageframe(pic_width, pic_height, fourcc, mmp_buffer::ION, NULL, NULL);
}

class mmp_buffer_imageframe* mmp_buffer_mgr_ex1::attach_media_imageframe(MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc) {

    return this->alloc_media_imageframe(pic_width, pic_height, fourcc, mmp_buffer::ION_ATTACH, shared_ion_fd, ion_mem_offset);
}

MMP_RESULT mmp_buffer_mgr_ex1::free_media_buffer(class mmp_buffer_media* p_buf_media) {
    
    MMP_S32 i;
    

    if(p_buf_media != NULL) {
    
        

        switch(p_buf_media->m_type) {

            case mmp_buffer_media::VIDEO_FRAME :
                {
                    class mmp_buffer_videoframe* p_mmp_videoframe = (class mmp_buffer_videoframe*)p_buf_media;

                    for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                        this->free_buffer(p_mmp_videoframe->m_p_mmp_buffer[i]);
                    }
                
                }
                break;

            case mmp_buffer_media::VIDEO_STREAM :
                {
                    class mmp_buffer_videostream* p_mmp_videostream = (class mmp_buffer_videostream*)p_buf_media;

                    this->free_buffer(p_mmp_videostream->m_p_mmp_buffer);
                }
                break;
        }

        delete p_buf_media;
    }

    return MMP_SUCCESS;
}

void mmp_buffer_mgr_ex1::print_info() {

    class mmp_lock autolock(m_p_mutex);
    
    MMP_S32 idx;
    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer = NULL;
    class mmp_buffer_addr buf_addr;
    MMP_S32 alloc_size[mmp_buffer::TYPE_MAX] = { 0, 0, 0};
    MMP_S32 buf_count[mmp_buffer::TYPE_MAX] = { 0, 0, 0};
    
    
    idx = 0;
    flag = m_list_buffer.GetFirst(p_mmp_buffer);
    while(flag) {
        buf_addr = p_mmp_buffer->get_buf_addr();
        MMPDEBUGMSG(1, (TEXT("\t%d. type=%d vir=0x%08x phy=0x%08x sz=%d fd=(%d 0x%08x)"), 
                      idx, buf_addr.m_type, 
                      buf_addr.m_vir_addr, buf_addr.m_phy_addr, buf_addr.m_size,
                      buf_addr.m_shared_fd, buf_addr.m_shared_fd
                      ));

        buf_count[buf_addr.m_type] ++;
        alloc_size[buf_addr.m_type] +=  buf_addr.m_size;
        idx++;
        flag = m_list_buffer.GetNext(p_mmp_buffer);
    }
    
    MMPDEBUGMSG(1, (TEXT("ION - cnt=%d   sz=%d "), buf_count[mmp_buffer::ION], alloc_size[mmp_buffer::ION] ));
    MMPDEBUGMSG(1, (TEXT("HEAP- cnt=%d   sz=%d "), buf_count[mmp_buffer::HEAP], alloc_size[mmp_buffer::HEAP] ));

}

