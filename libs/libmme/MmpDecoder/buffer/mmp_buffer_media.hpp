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

#ifndef MMP_BUFFER_MEDIA_HPP__
#define MMP_BUFFER_MEDIA_HPP__

#include "mmp_buffer.hpp"
#include "MmpPlayerDef.h"

class mmp_buffer_media {

friend class CLASS_BUFFER_MGR;

protected:

    enum {
        VIDEO_FRAME = 0,
        VIDEO_STREAM
    };

    MMP_S32 m_type;
    MMP_U32 m_flag;
    MMP_S64 m_pts;
    MMP_U32 m_coding_duration;
    
protected:
    mmp_buffer_media(MMP_S32 type);
    virtual ~mmp_buffer_media();

public:
    inline void set_pts(MMP_S64 pts) { m_pts = pts; }
    inline MMP_S64 get_pts() { return m_pts; }
    
    inline void set_flag(MMP_U32 flag) { m_flag = flag; }
    inline void or_flag(MMP_U32 flag) { m_flag |= flag; }
    inline MMP_U32 get_flag() { return m_flag; }
    
    inline void set_coding_dur(MMP_U32 dur) { m_coding_duration = dur; } 
    inline MMP_U32 get_coding_dur() { return m_coding_duration; }

};

#endif

