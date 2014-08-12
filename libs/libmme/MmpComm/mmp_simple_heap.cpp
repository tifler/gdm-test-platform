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


#include "mmp_simple_heap.hpp"

mmp_simple_heap::mmp_simple_heap(MMP_U32 frame_max_count) :
m_alloc_frame_count(0)
,m_alloc_frame_bytesize(0)
,m_frame_count(frame_max_count)
,m_p_mutex(NULL)
{

    m_frame = new struct alloc_frame[m_frame_count];

    memset(m_frame, 0x00, sizeof(struct alloc_frame) * m_frame_count);

    m_p_mutex = mmp_oal_mutex::create_object();
}

mmp_simple_heap::~mmp_simple_heap() {

    int i;

    for(i = 0; i < m_frame_count; i++) {
        if(m_frame[i].ptr != NULL) {
            free(m_frame[i].ptr);
        }
    }

    delete [] m_frame;

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
    }
}

void* mmp_simple_heap::alloc(MMP_U32 allocsize) {

    int i;
    void* ptr = NULL;

    m_p_mutex->lock();

    for(i = 0; i < m_frame_count; i++) {
        if(m_frame[i].ptr != NULL) {
            
            if( (m_frame[i].size == allocsize) && (m_frame[i].balloc == MMP_FALSE) ) {

                  ptr = m_frame[i].ptr;
                  m_frame[i].balloc = MMP_TRUE;  
                  break; 
            }
        }
    }

    if(ptr == NULL) {
        for(i = 0; i < m_frame_count; i++) {
        
            if(m_frame[i].ptr == NULL) {
                
                m_frame[i].ptr = malloc(allocsize);
                m_frame[i].balloc = MMP_TRUE;
                m_frame[i].size = allocsize;

                ptr = m_frame[i].ptr;

                m_alloc_frame_count++;
                m_alloc_frame_bytesize += allocsize;
                break;
            }
        }
    }

    m_p_mutex->unlock();

    return ptr;
}

MMP_RESULT mmp_simple_heap::free(void* ptr) {

    int i;
  
    m_p_mutex->lock();

     for(i = 0; i < m_frame_count; i++) {
        if(m_frame[i].ptr == ptr) {
            
            m_frame[i].balloc = MMP_FALSE;
            break;
        }
    }

    m_p_mutex->unlock();

    return MMP_SUCCESS;
}

