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

#ifndef MMPJPEGTOOL_HPP__
#define MMPJPEGTOOL_HPP__

#include "MmpBitExtractor.hpp"

/////////////////////////////////////////////////////////////

#define JPEG_SOI_CODE 0xD8FF /* Start of Image */
#define JPEG_SOF_CODE 0xC0FF /* Start of Frame */

//////////////////////////////////////////////
// class CMmpMpeg4Parser
class CMmpJpegTool
{
public:
    static MMP_RESULT GetWidthHeight(MMP_U8* filename, MMP_OUT MMP_U32 *pic_width, MMP_OUT MMP_U32* pic_height);
    static MMP_RESULT GetWidthHeight(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *pic_width, MMP_OUT MMP_U32* pic_height);

    /* search SOF (Start of Frame) */
    static MMP_RESULT Get_SOF_Offset(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *sof_offset);


    static int CMmpJpegTool::makeExif(unsigned char *exifOut,
                          unsigned char *thumb_buf,
                          unsigned int thumb_size,
                          unsigned int *size,
                          bool useMainbufForThumb);

};
#endif

