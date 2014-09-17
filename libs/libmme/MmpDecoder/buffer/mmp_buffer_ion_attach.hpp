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

#ifndef MMP_BUFFER_ION_ATTACH_HPP__
#define MMP_BUFFER_ION_ATTACH_HPP__

#include "mmp_buffer.hpp"

class mmp_buffer_ion_attach : public mmp_buffer {

friend class mmp_buffer;

private:

#if (MMP_OS == MMP_OS_WIN32)
    int m_phyaddr_fd;
#endif
    
protected:
    mmp_buffer_ion_attach(struct mmp_buffer_create_config *p_create_config);
    virtual ~mmp_buffer_ion_attach();

    virtual MMP_RESULT open();
    virtual MMP_RESULT close();


};

#endif
