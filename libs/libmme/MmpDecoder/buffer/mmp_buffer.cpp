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

#include "mmp_buffer.hpp"
#include "mmp_buffer_ion_stream.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_buffer* mmp_buffer::create_object(struct mmp_buffer_create_object *p_create_object) {

	class mmp_buffer* p_obj = NULL;

    if( MMP_BUFFER_IS_DMA(p_create_object->type) == 1) {
        p_obj = new class mmp_buffer_ion_stream(p_create_object);
    }
    else {
    	p_obj = NULL;//new class mmp_buffer_ion(p_create_object);
    }
	if(p_obj!=NULL) {
        
		if(p_obj->open() != MMP_SUCCESS)    
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    return p_obj;
}

MMP_RESULT mmp_buffer::destroy_object(class mmp_buffer* p_obj) {

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_SUCCESS;
}


class mmp_buffer_addr operator+(class mmp_buffer_addr &A, MMP_U32 value) {

    class mmp_buffer_addr B;

    B.m_vir_addr = A.m_vir_addr + value;
    B.m_phy_addr = A.m_phy_addr + value;

    B.m_size = A.m_size - value;
    B.m_shared_fd = A.m_shared_fd;

    return B;
}

/**********************************************************
class member
**********************************************************/

mmp_buffer::mmp_buffer(struct mmp_buffer_create_object *p_create_object) :
m_create_object(*p_create_object) 
,m_buf_addr(0,0,p_create_object->size,-1)
{

}

mmp_buffer::~mmp_buffer() {

}

