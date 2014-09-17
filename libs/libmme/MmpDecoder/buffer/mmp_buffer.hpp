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

#ifndef _MMP_BUFFER_HPP__
#define _MMP_BUFFER_HPP__

#include "MmpDefine.h"
#include "mmp_buffer_def.h"

class mmp_buffer_addr {

friend class mmp_buffer_addr operator+(class mmp_buffer_addr& A, MMP_U32 value);

public:
    mmp_buffer_addr() : m_vir_addr(0), m_phy_addr(0), m_size(0),  m_shared_fd(-1) { }
    mmp_buffer_addr(MMP_U32 vir_addr, MMP_U32 phy_addr, MMP_S32 size, MMP_S32 shared_fd) : m_vir_addr(vir_addr), m_phy_addr(phy_addr), m_size(size), m_shared_fd(shared_fd) { }

    MMP_U32 m_vir_addr;
    MMP_U32 m_phy_addr;
    MMP_S32 m_size;
    MMP_S32 m_shared_fd;
    MMP_U32 m_type;
};


struct mmp_buffer_create_config {
    MMP_U32 type;
    
    MMP_S32 size;
    MMP_S32 pic_width;
    MMP_S32 pic_height;

    /*attach info */
    MMP_S32 attach_shared_fd;
    MMP_U32 attach_phy_addr;
    MMP_U32 attach_vir_addr;
    MMP_S32 attach_offset;
};

class CLASS_BUFFER_MGR;
class mmp_buffer {

friend class CLASS_BUFFER_MGR;

public:
    enum {
        ION = 0,
        ION_ATTACH,
        HEAP,
        HEAP_ATTACH,
        EXT,
        TYPE_MAX
    };

protected:
    static MMP_U32 s_instance_index; 

private:
    static class mmp_buffer* create_object(struct mmp_buffer_create_config *p_create_config);
    static MMP_RESULT destroy_object(class mmp_buffer* p_obj);

protected:
    struct mmp_buffer_create_config m_create_config;
    class mmp_buffer_addr m_buf_addr;
    
protected:
    mmp_buffer(struct mmp_buffer_create_config *p_create_config);
    virtual ~mmp_buffer();

    virtual MMP_RESULT open() = 0;
    virtual MMP_RESULT close() = 0;

protected:
    MMP_U32 get_phy_addr_from_shared_fd(MMP_S32 shared_fd);
    MMP_U32 get_vir_addr_from_phy_addr(MMP_U32 phy_addr, MMP_S32 size);

public:
    virtual MMP_U32 get_phy_addr() { return m_buf_addr.m_phy_addr;}
    virtual MMP_U32 get_vir_addr() { return m_buf_addr.m_vir_addr;}

    virtual class mmp_buffer_addr get_buf_addr() { return m_buf_addr; }
    virtual MMP_S32 get_buf_size() { return m_buf_addr.m_size; }
    virtual MMP_S32 get_buf_shared_fd() { return m_buf_addr.m_shared_fd; }
};

#endif

