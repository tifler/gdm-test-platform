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

    struct mmp_buffer_create_object buffer_create_object;
    class mmp_buffer* p_mmp_buffer;

    buffer_create_object.type = MMP_BUFFER_TYPE_DMA;
    buffer_create_object.size = buffer_size;

    p_mmp_buffer = mmp_buffer::create_object(&buffer_create_object);
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

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::alloc_video_buffer(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U32 format) {
    
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_object buffer_create_object;

    class mmp_buffer_videoframe* p_mmp_videoframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 plane_size[MMP_MEDIASAMPLE_PLANE_COUNT];
    MMP_S32 buffer_width, buffer_height, luma_size, chroma_size;
    MMP_S32 i;
        
    p_mmp_videoframe = new class mmp_buffer_videoframe;
    if(p_mmp_videoframe != NULL) {
    
        p_mmp_videoframe->m_format = format;
        p_mmp_videoframe->m_pic_width = pic_width;
        p_mmp_videoframe->m_pic_height = pic_height;
        
        
        switch(p_mmp_videoframe->m_format) {

            case MMP_FOURCC_VIDEO_I420: 
                p_mmp_videoframe->m_plane_count = 3;        

                buffer_width = MMP_BYTE_ALIGN(p_mmp_videoframe->m_pic_width, 16);
                buffer_height = MMP_BYTE_ALIGN(p_mmp_videoframe->m_pic_height, 16);
                luma_size = buffer_width*buffer_height;
                chroma_size = luma_size>>2;

                plane_size[0] = luma_size;
                plane_size[1] = chroma_size;
                plane_size[2] = chroma_size;
                break;

            default:
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_buffer_mgr_ex1::alloc_video_buffer] FAIL: not supported video format 0x%08x "), p_mmp_videoframe->m_format ));
                break;
        }

        /* alloc mmp buffer per plane */
        if(mmpResult == MMP_SUCCESS) {

            for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                
                buffer_create_object.type = MMP_BUFFER_TYPE_DMA;
                buffer_create_object.size = plane_size[i];

                p_mmp_buf = mmp_buffer::create_object(&buffer_create_object);
                if(p_mmp_buf == NULL) {
                    mmpResult = MMP_FAILURE;
                    break;
                }
                else {
                    p_mmp_videoframe->m_p_mmp_buffer[i] = p_mmp_buf;
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
    
    return p_mmp_videoframe;
}

MMP_RESULT mmp_buffer_mgr_ex1::free_video_buffer(class mmp_buffer_videoframe* p_mmp_videoframe) {

    
    MMP_S32 i;

    if(p_mmp_videoframe != NULL) {
    
        for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
            this->free_buffer(p_mmp_videoframe->m_p_mmp_buffer[i]);
        }

        delete p_mmp_videoframe;
    }

    return MMP_SUCCESS;
}


