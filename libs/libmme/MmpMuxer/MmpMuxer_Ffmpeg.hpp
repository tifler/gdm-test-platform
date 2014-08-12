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

#ifndef MMPMUXER_FFMPEG_HPP__
#define MMPMUXER_FFMPEG_HPP__

#include "MmpMuxer.hpp"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/url.h"
}

class CMmpMuxer_Ffmpeg : public CMmpMuxer
{
friend class CMmpMuxer;

private:
     AVFormatContext *m_oc;
      AVOutputFormat *m_fmt;
    
protected:
    CMmpMuxer_Ffmpeg(struct MmpMuxerCreateConfig* pCreateConfig);
    virtual ~CMmpMuxer_Ffmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    
public:
    virtual MMP_RESULT AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size);
    virtual MMP_RESULT AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_U32 timestamp);
};


#endif //#ifndef _MMPDEMUXER_HPP__

