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

#ifndef MMPENCODERVIDEO_FFMPEG_HPP__
#define MMPENCODERVIDEO_FFMPEG_HPP__

#include "MmpEncoderVideo.hpp"
#include "MmpEncoderFfmpeg.hpp"

class CMmpEncoderVideo_Ffmpeg : public CMmpEncoderVideo, CMmpEncoderFfmpeg
{
friend class CMmpEncoder;

private:
    MMP_U8* m_temp_picture_buffer;
    MMP_U32 m_nEncodedStreamCount;
   
    
protected:
    CMmpEncoderVideo_Ffmpeg(struct MmpEncoderCreateConfig *pCreateConfig);
    virtual ~CMmpEncoderVideo_Ffmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_U8* GetClassName() { return (const MMP_U8*)"Ffmpeg";}
public:
    virtual MMP_RESULT EncodeAu(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult);
    

};

#endif

