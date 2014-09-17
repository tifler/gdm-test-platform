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

#include "mmp_env_mgr.hpp"
#include "MmpUtil.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_env_mgr* mmp_env_mgr::s_p_instance = NULL;

MMP_RESULT mmp_env_mgr::create_instance() {

    MMP_RESULT mmpResult;
	class mmp_env_mgr* p_obj = NULL;

	p_obj = new class mmp_env_mgr;
	if(p_obj!=NULL) {
        
		if(p_obj->open( ) != MMP_SUCCESS)
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    if(p_obj != NULL) {
        mmp_env_mgr::s_p_instance = p_obj;
        mmpResult = MMP_SUCCESS;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}

MMP_RESULT mmp_env_mgr::destroy_instance() {

    class mmp_env_mgr* p_obj = mmp_env_mgr::s_p_instance;

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;

        mmp_env_mgr::s_p_instance = NULL;
    }

    return MMP_SUCCESS;
}

class mmp_env_mgr* mmp_env_mgr::get_instance() {
    return mmp_env_mgr::s_p_instance;
}

/**********************************************************
class members
**********************************************************/

mmp_env_mgr::mmp_env_mgr() :

m_vpu_fd(-1)
,m_vpu_instance_pool_buffer(NULL)
,m_vpu_reg_vir_addr(0)
,m_vpu_common_buffer(NULL)

{

}

mmp_env_mgr::~mmp_env_mgr() {

}

MMP_RESULT mmp_env_mgr::open() {
    return MMP_SUCCESS;
}
 
MMP_RESULT mmp_env_mgr::close() {
    return MMP_SUCCESS;
}

