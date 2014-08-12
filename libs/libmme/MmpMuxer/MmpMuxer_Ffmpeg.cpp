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

#include "MmpMuxer_Ffmpeg.hpp"
#include "MmpUtil.hpp"


/////////////////////////////////////////////////////
// class

CMmpMuxer_Ffmpeg::CMmpMuxer_Ffmpeg(struct MmpMuxerCreateConfig* pCreateConfig) : CMmpMuxer(pCreateConfig)
,m_oc(NULL)
,m_fmt(NULL)
{
    av_register_all();

    
}

CMmpMuxer_Ffmpeg::~CMmpMuxer_Ffmpeg()
{

}

MMP_RESULT CMmpMuxer_Ffmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
   
    if(mmpResult == MMP_SUCCESS) {
        avformat_alloc_output_context2(&m_oc, NULL, NULL, (const char*)m_create_config.filename);
        if(m_oc == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_Ffmpeg::Open] FAIL: avformat_alloc_output_context2 ")));
            mmpResult = MMP_FAILURE;
        }
        else {
            m_fmt = m_oc->oformat;
        }
    }
  /* 
    if(mmpResult == MMP_SUCCESS) {
     
        if(!(fmt->flags & AVFMT_NOFILE)) {
            ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
            if (ret < 0) {
                fprintf(stderr, "Could not open '%s': %s\n", filename,    av_err2str(ret));
                return 1;
            }
        }
    }
*/
    return mmpResult;
}

MMP_RESULT CMmpMuxer_Ffmpeg::Close()
{
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpMuxer_Ffmpeg::AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size) {

    return MMP_SUCCESS;
}

MMP_RESULT CMmpMuxer_Ffmpeg::AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_U32 timestamp) {

    return MMP_SUCCESS;
}
