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

#include "MmpDemuxer.hpp"
#include "MmpDemuxer_Ffmpeg.hpp"
#include "MmpDemuxer_ammf.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////
// Create/Destroy Object

CMmpDemuxer* CMmpDemuxer::CreateObject(struct MmpDemuxerCreateConfig* pCreateConfig)
{
    CMmpDemuxer* pObj=NULL;
    MMP_CHAR szExt[32];

    CMmpUtil::SplitExt((MMP_CHAR*)pCreateConfig->filename, szExt);
    CMmpUtil::MakeLower(szExt);

    if(strcmp(szExt, "ammf") == 0) {
    
        pObj=new CMmpDemuxer_ammf(pCreateConfig);
    }
    else {
		       
        switch(pCreateConfig->type) {

          //case MMP_DEMUXER_TYPE_MJPEG_STREAM:
          //    pObj=new CMmpDemuxer_MjpegStream(pCreateConfig);
          //    break;

          case MMP_DEMUXER_TYPE_AMMF:
              pObj=new CMmpDemuxer_ammf(pCreateConfig);
              break;

          case MMP_DEMUXER_TYPE_FFMPEG:
          default:
              pObj=new CMmpDemuxer_Ffmpeg(pCreateConfig);
              break;
        }

    }

	if(pObj==NULL) {
        return (CMmpDemuxer*)NULL;
	}

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpDemuxer*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpDemuxer::DestroyObject(CMmpDemuxer* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////
// class

CMmpDemuxer::CMmpDemuxer(struct MmpDemuxerCreateConfig* pCreateConfig) :
m_create_config(*pCreateConfig)
{

}

CMmpDemuxer::~CMmpDemuxer()
{

}

MMP_RESULT CMmpDemuxer::Open()
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpDemuxer::Close()
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpDemuxer::GetNextAudioData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packet_pts) {

    return this->GetNextMediaData(MMP_MEDIATYPE_AUDIO, buffer, buf_max_size, buf_size, packet_pts);
}

MMP_RESULT CMmpDemuxer::GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packet_pts) {

    return this->GetNextMediaData(MMP_MEDIATYPE_VIDEO, buffer, buf_max_size, buf_size, packet_pts);
}

MMP_RESULT CMmpDemuxer::GetNextVideoData(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_U8* buffer;
    MMP_S32 buf_max_size;
    MMP_S32 stream_size;
    MMP_S64 packet_pts;
    MMP_RESULT mmpResult; 

    p_buf_videostream->set_stream_size(0);

    buffer = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    buf_max_size = p_buf_videostream->get_buf_size();

    mmpResult = this->GetNextMediaData(MMP_MEDIATYPE_VIDEO, buffer, (MMP_U32)buf_max_size, (MMP_U32*)&stream_size, &packet_pts);
    if(mmpResult == MMP_SUCCESS) {
        p_buf_videostream->set_stream_size(stream_size);
        p_buf_videostream->set_pts(packet_pts);
    }

    return mmpResult;
}
    
MMP_RESULT CMmpDemuxer::GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size) {

    return this->GetMediaExtraData(MMP_MEDIATYPE_VIDEO, buffer, buf_max_size, buf_size);
}

MMP_RESULT CMmpDemuxer::GetVideoExtraData(class mmp_buffer_videostream* p_buf_videstream) {

    class mmp_buffer_addr buf_addr;
    MMP_S32 stream_size = 0;
    MMP_RESULT mmpResult;

    buf_addr = p_buf_videstream->get_buf_addr();

    mmpResult = this->GetMediaExtraData(MMP_MEDIATYPE_VIDEO, (MMP_U8*)buf_addr.m_vir_addr, buf_addr.m_size, (MMP_U32*)&stream_size);
    if(mmpResult == MMP_SUCCESS) {
        p_buf_videstream->set_stream_size(stream_size);
    }
    else {
        p_buf_videstream->set_stream_size(0);
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer::GetAudioExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size) {

    return this->GetMediaExtraData(MMP_MEDIATYPE_AUDIO, buffer, buf_max_size, buf_size);
}

    
MMP_RESULT CMmpDemuxer::GetNextData(CMmpMediaSample* pMediaSample) {

    this->queue_buffering();
    return  this->queue_get(pMediaSample);
}