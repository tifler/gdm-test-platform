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

#ifndef _MMP_BUFFER_VIDEOFRAME_HPP__
#define _MMP_BUFFER_VIDEOFRAME_HPP__

#include "mmp_buffer_media.hpp"

class mmp_buffer_videoframe : public mmp_buffer_media {

friend class CLASS_BUFFER_MGR;
private:
    class mmp_buffer* m_p_mmp_buffer[MMP_MEDIASAMPLE_PLANE_COUNT];

    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;
    MMP_S32 m_plane_count;
    MMP_U32 m_format;

    MMP_S32 m_buf_stride[MMP_MEDIASAMPLE_PLANE_COUNT];
    MMP_S32 m_buf_height[MMP_MEDIASAMPLE_PLANE_COUNT];
    
private:
    mmp_buffer_videoframe();
    virtual ~mmp_buffer_videoframe();
    
public:
    
    class mmp_buffer_addr get_buf_addr(MMP_S32 frame_id);
    
    MMP_U8* get_buf_vir_addr(MMP_S32 frame_id);
    MMP_U8* get_buf_vir_addr_y() { return this->get_buf_vir_addr(MMP_MEDIASAMPLE_BUF_Y); }
    MMP_U8* get_buf_vir_addr_cb() { return this->get_buf_vir_addr(MMP_MEDIASAMPLE_BUF_CB); } 
    MMP_U8* get_buf_vir_addr_cr() { return this->get_buf_vir_addr(MMP_MEDIASAMPLE_BUF_CR); }
    MMP_S32 get_buf_shared_fd(MMP_S32 frame_id);

    MMP_U32 get_buf_phy_addr(MMP_S32 frame_id);

    inline MMP_S32 get_stride_luma() { return m_buf_stride[MMP_MEDIASAMPLE_BUF_Y]; }
    inline MMP_S32 get_stride_chroma() { return m_buf_stride[MMP_MEDIASAMPLE_BUF_CB]; }
    inline MMP_S32 get_buf_stride(MMP_S32 frame_id) { return m_buf_stride[frame_id]; }
    inline MMP_S32 get_buf_height(MMP_S32 frame_id) { return m_buf_height[frame_id]; }

    inline MMP_S32 get_pic_width() { return m_pic_width; }
    inline MMP_S32 get_pic_height() { return m_pic_height; }
};

#endif

