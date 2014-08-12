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

#ifndef _MMPDEMUXERBUFFER_HPP__
#define _MMPDEMUXERBUFFER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "TemplateList.hpp"

struct queue_packet {
    MMP_S32 mediatype;
    MMP_U8* data;
    MMP_U32 size;
    MMP_S64 pts;
    MMP_S32 flags;
};

class CMmpDemuxerBuffer
{
private:
    TCircular_Queue<struct queue_packet> *m_p_queue_media[MMP_MEDIATYPE_MAX];
    
protected:
    CMmpDemuxerBuffer();
    virtual ~CMmpDemuxerBuffer();

    virtual void queue_buffering(void) = 0;
    MMP_S32 queue_get_empty_streamindex(void);
    void queue_add(struct queue_packet* p_pack);
    MMP_RESULT queue_get(CMmpMediaSample* pMediaSample);
    MMP_RESULT queue_clear();
};


#endif 