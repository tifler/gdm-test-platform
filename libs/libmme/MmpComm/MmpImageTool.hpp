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
public:
#if 0
    struct image_buffer {

        MMP_S32 width;
        MMP_S32 height;
        enum MMP_FOURCC fourcc;
        union {
            MMP_U8* data;
            MMP_U8* plane[3];
        }m;
        union {
            MMP_S32 stride;
            MMP_S32 plane_stride[3];
        }s;
    };
#endif

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
    
    static MMP_RESULT ConvertRGBtoYUV420_P3(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                            MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                            MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride);

    static MMP_RESULT ConvertRGBtoYCbCr422_P2(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                            MMP_U8* p_image_y, MMP_U8* p_image_uv,
                                            MMP_S32 y_stride, MMP_S32 uv_stride);

    static MMP_RESULT ConvertRGBtoYCbCr420_P2(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                            MMP_U8* p_image_y, MMP_U8* p_image_uv,
                                            MMP_S32 y_stride, MMP_S32 uv_stride);

    static MMP_RESULT ConvertRGBtoYUV444_P1(MMP_U8* p_rgb_data, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc_rgb,
                                        MMP_U8* p_yuv_data, MMP_S32 yuv_stride);

    static MMP_RESULT ConvertRGBtoGREY(MMP_U8* p_rgb_data, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_grey_data, MMP_S32 grey_stride);

    static MMP_RESULT ConvertYUV420P3toYUV444P1(MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride);

    static MMP_RESULT ConvertYUV422P2toYUV444P1(MMP_U8* yuv420_y, MMP_U8* yuv420_uv, 
                                                MMP_S32 y_stride, MMP_S32 uv_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride);

    static MMP_RESULT ConvertYCbCr420P2toYUV444P1(MMP_U8* yuv420_y, MMP_U8* yuv420_uv, 
                                                  MMP_S32 y_stride, MMP_S32 uv_stride,
                                                  MMP_S32 pic_width, MMP_S32 pic_height, 
                                                  MMP_U8* yuv444, MMP_S32 yuv444_stride);

    static MMP_RESULT ConvertYUV444P1toYUV420P3(MMP_U8* yuv444, MMP_S32 yuv444_stride, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride
                                                ); 
                                                
    static MMP_RESULT ConvertYUV422P2toYUV420P3(MMP_U8* yuv422_y, MMP_U8* yuv422_uv, 
                                                MMP_S32 yuv422_y_stride,  MMP_S32 yuv422_uv_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride
                                                ); 
    
    static MMP_RESULT ConvertColor(MMP_U8* p_src_data, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc_src,
                                   MMP_U8* p_dst_data, enum MMP_FOURCC fourcc_dst, MMP_S32 dst_stride);


    
    /********************************************************** 
        Tool of JPEG
    ***********************************************************/
public:
    static MMP_RESULT Jpeg_GetWidthHeight(MMP_U8* filename, MMP_OUT MMP_S32 *pic_width, MMP_OUT MMP_S32* pic_height);
    static MMP_RESULT Jpeg_GetWidthHeightFourcc(MMP_U8* jpegdata, MMP_S32 jpegsize, MMP_OUT MMP_S32 *pic_width, MMP_OUT MMP_S32* pic_height, enum MMP_FOURCC* pfourcc);

    /* search SOF (Start of Frame) */
    static MMP_RESULT Jpeg_Get_SOF_Offset(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *sof_offset);


    static int Jpeg_makeExif(unsigned char *exifOut,
                          unsigned char *thumb_buf,
                          unsigned int thumb_size,
                          unsigned int *size,
                          bool useMainbufForThumb);

    static MMP_RESULT Jpeg_Decode_libjpeg_RGB(MMP_U8* jpegdata, MMP_S32 jpegsize, 
                                          MMP_S32 *pic_width, MMP_S32* pic_height, enum MMP_FOURCC *fourcc, MMP_U8* p_decoded_data);
    
    
    static MMP_RESULT Jpeg_Encode_libjpeg_YUV444_P1(MMP_U8* p_src_data, 
                                         MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 src_stride,  MMP_S32 quality,
                                         MMP_U8* jpegdata, MMP_S32 jpegdata_max_size, MMP_S32 *jpegsize);
                                          
        
    
    /********************************************************** 
        Tool of BMP
    ***********************************************************/
private:
    enum {
        BMP_HEADER_MARKER = (('M'<<8 )|'B')
    };
public:

    static MMP_CHAR* Bmp_GetName(enum MMP_FOURCC fourcc, MMP_CHAR* buf);
    static MMP_RESULT Bmp_SaveFile(MMP_CHAR* bmp_filename, MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image, MMP_U32 fourcc);
    static MMP_RESULT Bmp_LoadInfo(MMP_CHAR* bmp_filename, MMP_S32* pic_width, MMP_S32* pic_height, enum MMP_FOURCC* fourcc, MMP_S32* stride);
    static MMP_RESULT Bmp_LoadFile(MMP_CHAR* bmp_filename, MMP_U8* p_image, MMP_S32 image_max_size);
    

    /********************************************************** 
        Tool of YUV File
    ***********************************************************/
    static MMP_RESULT YUV_LoadFile_YUV420P3(MMP_CHAR* yuv_filename, MMP_S32 pic_width, MMP_S32 pic_height, 
                                            MMP_U8* p_data_y, MMP_U8* p_data_u, MMP_U8* p_data_v, 
                                            MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride);
    
};
#endif

