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

#ifndef MMPIMAGETOOL_HPP__
#define MMPIMAGETOOL_HPP__

#include "MmpBitExtractor.hpp"

/////////////////////////////////////////////////////////////

#define JPEG_SOI_CODE 0xD8FF /* Start of Image */
#define JPEG_SOF_CODE 0xC0FF /* Start of Frame */

//////////////////////////////////////////////
// class CMmpMpeg4Parser
class CMmpImageTool
{
    /********************************************************** 
        Tool of Common
    ***********************************************************/
public:
    static MMP_BOOL IsRGB(enum MMP_FOURCC fourcc);
    static MMP_BOOL IsYUV(enum MMP_FOURCC fourcc);
    static MMP_S32 GetPlaneCount(enum MMP_FOURCC fourcc);
    static MMP_S32 GetPixelByte(enum MMP_FOURCC fourcc);
    static MMP_S32 GetPicStride(enum MMP_FOURCC fourcc, MMP_S32 pic_width);
    static MMP_S32 GetBufferStride(enum MMP_FOURCC fourcc, MMP_S32 pic_width);
    

    /********************************************************** 
        Tool of Impage Processing
    ***********************************************************/
public:
    static MMP_RESULT Flip_V(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image, enum MMP_FOURCC fourcc);
    static MMP_RESULT ConvertRGBtoYUV(MMP_S32 pic_width, MMP_S32 pic_height, 
                                      MMP_U8* p_image_rgb, enum MMP_FOURCC fourcc_rgb,
                                      MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v,enum MMP_FOURCC fourcc_yuv, MMP_S32 y_stride);

    
    /********************************************************** 
        Tool of JPEG
    ***********************************************************/
public:
    static MMP_RESULT Jpeg_GetWidthHeight(MMP_U8* filename, MMP_OUT MMP_U32 *pic_width, MMP_OUT MMP_U32* pic_height);
    static MMP_RESULT Jpeg_GetWidthHeight(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *pic_width, MMP_OUT MMP_U32* pic_height);

    /* search SOF (Start of Frame) */
    static MMP_RESULT Jpeg_Get_SOF_Offset(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *sof_offset);


    static int Jpeg_makeExif(unsigned char *exifOut,
                          unsigned char *thumb_buf,
                          unsigned int thumb_size,
                          unsigned int *size,
                          bool useMainbufForThumb);

    
    /********************************************************** 
        Tool of BMP
    ***********************************************************/
private:
    enum {
        BMP_HEADER_MARKER = (('M'<<8 )|'B')
    };
public:

    static MMP_RESULT Bmp_SaveFile(MMP_CHAR* bmp_filename, MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image, MMP_U32 fourcc);
    static MMP_CHAR* Bmp_GetName(enum MMP_FOURCC fourcc, MMP_CHAR* buf);
    
    
};
#endif

