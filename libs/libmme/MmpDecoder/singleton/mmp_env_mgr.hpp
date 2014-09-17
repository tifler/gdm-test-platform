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

#ifndef MMP_ENV_MGR_HPP__
#define MMP_ENV_MGR_HPP__

#include "MmpDefine.h"

class mmp_singleton_mgr;
class mmp_vpu_dev;

class mmp_env_mgr {

friend class mmp_singleton_mgr;
friend class mmp_vpu_dev;

private:
    static class mmp_env_mgr* s_p_instance;

private:
    static MMP_RESULT create_instance();
	static MMP_RESULT destroy_instance();

public:
    static class mmp_env_mgr* get_instance();


private:
    MMP_S32 m_vpu_fd;
    MMP_U8* m_vpu_instance_pool_buffer;
    MMP_U32 m_vpu_reg_vir_addr;
    void*   m_vpu_common_buffer;

private:
    mmp_env_mgr();
    virtual ~mmp_env_mgr();

    virtual MMP_RESULT open();
    virtual MMP_RESULT close();

private:
    inline void set_vpu_fd(MMP_S32 fd) { m_vpu_fd = fd; }
    inline void set_vpu_instance_pool_buffer(MMP_U8* ptr) { m_vpu_instance_pool_buffer = ptr; }
    inline void set_vpu_reg_vir_addr(MMP_U32 addr) { m_vpu_reg_vir_addr = addr; }
    inline void set_vpu_common_buffer(void* ptr) { m_vpu_common_buffer = ptr; }

public:
    inline MMP_S32 get_vpu_fd() { return m_vpu_fd; }
    inline MMP_U8* get_vpu_instance_pool_buffer() { return m_vpu_instance_pool_buffer; }
    inline MMP_U32 get_vpu_reg_vir_addr() { return m_vpu_reg_vir_addr; }
    inline void* get_vpu_common_buffer() { return m_vpu_common_buffer; }

};

#endif
