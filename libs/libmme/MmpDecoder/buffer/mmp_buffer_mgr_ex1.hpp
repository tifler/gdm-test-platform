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

#ifndef _MMP_BUFFER_MGR_EX1_HPP__
#define _MMP_BUFFER_MGR_EX1_HPP__

#include "mmp_buffer_mgr.hpp"
#include "TemplateList.hpp"
#include "mmp_oal_mutex.hpp"

class mmp_buffer_mgr_ex1 : public mmp_buffer_mgr {

friend class mmp_buffer_mgr;

private:
    TList<class mmp_buffer*> m_list_buffer;
    class mmp_oal_mutex* m_p_mutex;

private:
    mmp_buffer_mgr_ex1();
    virtual ~mmp_buffer_mgr_ex1();

    virtual MMP_RESULT open();
    virtual MMP_RESULT close();

public:

    virtual class mmp_buffer* alloc_dma_buffer(MMP_S32 buffer_size);
    virtual MMP_RESULT free_buffer(class mmp_buffer* p_mmp_buffer);
    virtual MMP_RESULT free_buffer(class mmp_buffer_addr buf_addr);
    virtual class mmp_buffer* get_buffer(MMP_S32 shared_fd);
    virtual class mmp_buffer_addr get_buffer_addr(MMP_S32 shared_fd);

    virtual class mmp_buffer_videoframe* alloc_video_buffer(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U32 format);
    virtual MMP_RESULT free_video_buffer(class mmp_buffer_videoframe*);
};

#endif


