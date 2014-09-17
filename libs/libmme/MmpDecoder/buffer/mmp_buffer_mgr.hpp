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

#ifndef _MMP_BUFFER_MGR_HPP__
#define _MMP_BUFFER_MGR_HPP__

#include "MmpDefine.h"
#include "mmp_buffer_def.h"
#include "mmp_buffer.hpp"
#include "mmp_buffer_videoframe.hpp"
#include "mmp_buffer_videostream.hpp"

class mmp_singleton_mgr;

class mmp_buffer_mgr {

friend class mmp_singleton_mgr;
private:
    static class mmp_buffer_mgr* s_p_instance;

private:
    static MMP_RESULT create_instance();
    static MMP_RESULT destroy_instance();

public:
    static class mmp_buffer_mgr* get_instance();

protected:
    mmp_buffer_mgr();
    virtual ~mmp_buffer_mgr();

    virtual MMP_RESULT open() = 0;
    virtual MMP_RESULT close() = 0;

public:

    virtual class mmp_buffer* alloc_dma_buffer(MMP_S32 buffer_size) = 0;
    virtual class mmp_buffer* attach_dma_buffer(class mmp_buffer_addr buf_addr) = 0;
    virtual MMP_RESULT free_buffer(class mmp_buffer* p_mmp_buffer) = 0;
    virtual MMP_RESULT free_buffer(class mmp_buffer_addr buf_addr) = 0;
    virtual class mmp_buffer* get_buffer(MMP_S32 shared_fd) = 0;
    virtual class mmp_buffer_addr get_buffer_addr(MMP_S32 shared_fd) = 0;

    virtual class mmp_buffer_videoframe* alloc_media_videoframe(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U32 format=MMP_FOURCC_VIDEO_I420) = 0;
    virtual class mmp_buffer_videoframe* attach_media_videoframe(MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset,
                                                                 MMP_S32 pic_width, MMP_S32 pic_height, MMP_U32 format=MMP_FOURCC_VIDEO_I420) = 0;
    
    virtual class mmp_buffer_videostream* alloc_media_videostream(MMP_S32 stream_max_size, MMP_U32 buf_type=mmp_buffer::HEAP) = 0;
    virtual class mmp_buffer_videostream* attach_media_videostream(MMP_U8* p_stream_data, MMP_S32 stream_size) = 0;

    virtual MMP_RESULT free_media_buffer(class mmp_buffer_media*) = 0;

    virtual void print_info() = 0;
};


#endif


