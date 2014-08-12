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

#include "MmpRenderer_FileWriter.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpRenderer_FileWriter Member Functions


CMmpRenderer_FileWriter::CMmpRenderer_FileWriter(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
{

}

CMmpRenderer_FileWriter::~CMmpRenderer_FileWriter()
{

}

MMP_RESULT CMmpRenderer_FileWriter::Open()
{
    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_FileWriter::Close()
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_FileWriter::Render(CMmpMediaSampleDecodeResult* pDecResult)
{

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_FileWriter::RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_FileWriter::RenderYUV420Planar] +++ ")));
    return MMP_SUCCESS;
}
