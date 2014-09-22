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

#ifndef MMP_SIMPLE_HEAP_HPP__
#define MMP_SIMPLE_HEAP_HPP__

#include "MmpDefine.h"
#include "mmp_oal_mutex.hpp"

class mmp_simple_heap {

private:
    struct alloc_frame {
        
        void* ptr;
        MMP_U32 size;
        MMP_BOOL balloc;
    };

    MMP_U32 m_alloc_frame_count;
    MMP_U32 m_alloc_frame_bytesize;
    MMP_U32 m_frame_count;
    struct alloc_frame *m_frame;
    class mmp_oal_mutex* m_p_mutex;

public:
    mmp_simple_heap(MMP_U32 frame_max_count = 64);
    ~mmp_simple_heap();

    void* alloc(MMP_U32 allocsize);
    MMP_RESULT free(void* ptr);

};

#endif
