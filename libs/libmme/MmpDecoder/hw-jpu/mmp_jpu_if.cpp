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


#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "mmp_jpu_if.hpp"
#include "mmp_jpu_if_ana.hpp"

#if (JPU_PLATFORM_V4L2_ENABLE != 1)

/**********************************************************
create/destroy object
**********************************************************/

class mmp_jpu_if* mmp_jpu_if::create_object() {
    
    struct mmp_jpu_if_create_config vpu_create_config;
    memset(&vpu_create_config, 0x00, sizeof(struct mmp_jpu_if_create_config));

    return mmp_jpu_if::create_object(&vpu_create_config);
}

class mmp_jpu_if* mmp_jpu_if::create_object(struct mmp_jpu_if_create_config* p_create_config) {

	class mmp_jpu_if* p_obj = NULL;

#if (MMP_JPU == MMP_JPU_ANAPASS)
	p_obj = new class mmp_jpu_if_ana(p_create_config);

#elif (MMP_JPU == MMP_JPU_CNM)
	p_obj = new class mmp_jpu_if_cnm(p_create_config);
#elif (MMP_JPU == MMP_JPU_V4L2)
	p_obj = NULL;
#else
#error "ERROR : select MMP_JPU "
#endif
	if(p_obj!=NULL) {
        
		if(p_obj->open( ) != MMP_ErrorNone)    
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    return p_obj;
}

MMP_RESULT mmp_jpu_if::destroy_object(class mmp_jpu_if* p_obj) {

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_SUCCESS;
}

/**********************************************************
class members
**********************************************************/

mmp_jpu_if::mmp_jpu_if(struct mmp_jpu_if_create_config* p_create_config) :
m_create_config(*p_create_config)
{

}

mmp_jpu_if::~mmp_jpu_if() {

}


#endif
