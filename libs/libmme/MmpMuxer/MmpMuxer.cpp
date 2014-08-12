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

#include "MmpMuxer.hpp"
#include "MmpMuxer_ammf.hpp"
#include "MmpMuxer_Ffmpeg.hpp"
#include "MmpMuxer_rawstream.hpp"

/////////////////////////////////////////////////////////
// Create/Destroy Object

CMmpMuxer* CMmpMuxer::CreateObject(struct MmpMuxerCreateConfig* pCreateConfig)
{
    CMmpMuxer* pObj=NULL;
		       
    switch(pCreateConfig->type) {

      case MMP_MUXER_TYPE_ANAPASS_MULTIMEDIA_FORMAT:
          pObj=new CMmpMuxer_ammf(pCreateConfig);
          break;

      case MMP_MUXER_TYPE_RAWSTREAM:
          pObj=new CMmpMuxer_rawstream(pCreateConfig);
          break;

      case MMP_MUXER_TYPE_FFMPEG:
      default:
          pObj=new CMmpMuxer_Ffmpeg(pCreateConfig);
          break;
    }

	if(pObj==NULL) {
        return (CMmpMuxer*)NULL;
	}

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpMuxer*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpMuxer::DestroyObject(CMmpMuxer* pObj)
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

CMmpMuxer::CMmpMuxer(struct MmpMuxerCreateConfig* pCreateConfig) :
m_create_config(*pCreateConfig)
{

}

CMmpMuxer::~CMmpMuxer()
{

}

MMP_RESULT CMmpMuxer::Open()
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpMuxer::Close()
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpMuxer::AddVideoData(MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_U32 timestamp) {

    return this->AddMediaData(MMP_MEDIATYPE_VIDEO, buffer, buf_size, flag, timestamp);
}
