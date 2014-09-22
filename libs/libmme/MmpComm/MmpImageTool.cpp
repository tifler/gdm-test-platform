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
#include "MmpImageTool.hpp"
#include "MmpUtil.hpp"
#include <math.h>

/********************************************************** 
    Tool of Common
***********************************************************/

MMP_BOOL CMmpImageTool::IsRGB(enum MMP_FOURCC fourcc) {

    MMP_BOOL bflag;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
	    case MMP_FOURCC_IMAGE_RGB565  : 
	    case MMP_FOURCC_IMAGE_BGR565  : 
	    case MMP_FOURCC_IMAGE_ARGB1555: 
	    case MMP_FOURCC_IMAGE_BGRA1555: 
	    case MMP_FOURCC_IMAGE_RGBA1555: 
	    case MMP_FOURCC_IMAGE_ABGR1555: 
	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_RGBA8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
    }

    return bflag;
}

MMP_BOOL CMmpImageTool::IsYUV(enum MMP_FOURCC fourcc) {

    MMP_BOOL bflag;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YV12  : 
	    case MMP_FOURCC_IMAGE_I420  : 
	        bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
    }

    return bflag;
}

MMP_S32 CMmpImageTool::GetPlaneCount(enum MMP_FOURCC fourcc) {

    MMP_S32 plane_count = 0;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
	    case MMP_FOURCC_IMAGE_RGB565  : 
	    case MMP_FOURCC_IMAGE_BGR565  : 
	    case MMP_FOURCC_IMAGE_ARGB1555: 
	    case MMP_FOURCC_IMAGE_BGRA1555: 
	    case MMP_FOURCC_IMAGE_RGBA1555: 
	    case MMP_FOURCC_IMAGE_ABGR1555: 
	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_RGBA8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            plane_count = 1;
            break;

        case MMP_FOURCC_IMAGE_YV12:
        case MMP_FOURCC_IMAGE_I420:
        case MMP_FOURCC_IMAGE_YUV444P3:
            plane_count = 3;
            break;

        case MMP_FOURCC_IMAGE_YUV444Packed:
            plane_count = 1;
            break;

        default:
            plane_count = 0;
    }

    return plane_count;
}

MMP_S32 CMmpImageTool::GetPixelByte(enum MMP_FOURCC fourcc) {

    MMP_S32 pixel_byte = 1;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
            pixel_byte = 3;
            break;

	    case MMP_FOURCC_IMAGE_RGB565  : 
	    case MMP_FOURCC_IMAGE_BGR565  : 
	    case MMP_FOURCC_IMAGE_ARGB1555: 
	    case MMP_FOURCC_IMAGE_BGRA1555: 
	    case MMP_FOURCC_IMAGE_RGBA1555: 
	    case MMP_FOURCC_IMAGE_ABGR1555: 
            pixel_byte = 2;
            break;

	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_RGBA8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            pixel_byte = 4;
            break;

        case MMP_FOURCC_IMAGE_YV12:
        case MMP_FOURCC_IMAGE_I420:
        default:
            pixel_byte = 1;
    }

    return pixel_byte;
}

MMP_S32 CMmpImageTool::GetPicStride(enum MMP_FOURCC fourcc, MMP_S32 pic_width) {

    MMP_S32 stride;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
            stride = MMP_BYTE_ALIGN(pic_width*3, 4); 
            break;

	    case MMP_FOURCC_IMAGE_RGB565  : 
	    case MMP_FOURCC_IMAGE_BGR565  : 
            stride = pic_width*2; 
            break;

	    case MMP_FOURCC_IMAGE_ARGB1555: 
	    case MMP_FOURCC_IMAGE_BGRA1555: 
	    case MMP_FOURCC_IMAGE_RGBA1555: 
	    case MMP_FOURCC_IMAGE_ABGR1555: 
            stride = pic_width*2; 
            break;

	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_RGBA8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            stride = pic_width*4; 
            break;
            
        case MMP_FOURCC_IMAGE_YV12:
        case MMP_FOURCC_IMAGE_I420:
        default:
            stride = pic_width;
    }

    return stride;
}

MMP_S32 CMmpImageTool::GetBufferStride(enum MMP_FOURCC fourcc, MMP_S32 pic_width) {

    MMP_S32 stride;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
            stride = MMP_BYTE_ALIGN(pic_width*3, 4); 
            break;

	    case MMP_FOURCC_IMAGE_RGB565  : 
	    case MMP_FOURCC_IMAGE_BGR565  : 
            stride = pic_width*2; 
            break;

	    case MMP_FOURCC_IMAGE_ARGB1555: 
	    case MMP_FOURCC_IMAGE_BGRA1555: 
	    case MMP_FOURCC_IMAGE_RGBA1555: 
	    case MMP_FOURCC_IMAGE_ABGR1555: 
            stride = pic_width*2; 
            break;

	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_RGBA8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            stride = pic_width*4; 
            break;
            
        case MMP_FOURCC_IMAGE_YV12:
        case MMP_FOURCC_IMAGE_I420:
            stride = MMP_BYTE_ALIGN(pic_width, 16); 
            break;

        default:
            stride = pic_width;
    }

    return stride;
}

/********************************************************** 
        Tool of Impage Processing
***********************************************************/
MMP_RESULT CMmpImageTool::Flip_V(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image, enum MMP_FOURCC fourcc) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stride;
    MMP_U8* tmpbuf = NULL;
    MMP_S32 h, flip_height;
    MMP_U8 *p_image_top, *p_image_bottom;

    /* check stride */
    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_BGR888:
        case MMP_FOURCC_IMAGE_RGB888: stride = MMP_BYTE_ALIGN(pic_width*3,4); break;

        case MMP_FOURCC_IMAGE_ARGB8888: stride = pic_width*4; break;
        default:
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpImageTool::%s] FAIL: not support format (%c%c%c%c) "), MMP_CLASS_FUNC, 
                                                     MMPGETFOURCCARG(fourcc)
                                                     ));
    }

    if(mmpResult == MMP_SUCCESS) {

        tmpbuf = (MMP_U8*)MMP_MALLOC(stride);
        if(tmpbuf != NULL) {
            
            flip_height = pic_height/2;
            p_image_top = p_image;
            p_image_bottom = p_image + stride*(pic_height-1);
            for(h = 0; h < flip_height; h++) {
                memcpy(tmpbuf, p_image_top, stride);
                memcpy(p_image_top, p_image_bottom, stride);
                memcpy(p_image_bottom, tmpbuf, stride);

                p_image_top += stride;
                p_image_bottom -= stride;
            }
        }
    }

    if(tmpbuf != NULL) {
        MMP_FREE(tmpbuf);
    }

    return mmpResult;
}

MMP_RESULT CMmpImageTool::ConvertRGBtoYUV(MMP_S32 pic_width, MMP_S32 pic_height, 
                                      MMP_U8* p_image_rgb, enum MMP_FOURCC fourcc_rgb,
                                      MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v,enum MMP_FOURCC fourcc_yuv, MMP_S32 y_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1, *p_image_src2;
    MMP_U8 *p_line_src1, *p_line_src2;
    
    MMP_U8 *p_image_dest_y, *p_image_dest_u, *p_image_dest_v;
    MMP_U8 *p_line_dest_y, *p_line_dest_u, *p_line_dest_v;

    MMP_S32 y;
    MMP_S32 r[4], g[4], b[4];
    MMP_S32 u[4], v[4];

    MMP_S32 i;
    MMP_BOOL is_end_col;

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }
    if(CMmpImageTool::IsYUV(fourcc_yuv) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }

    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888:
                        b[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        r[0] = p_line_src1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:
                        r[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        b[0] = p_line_src1[2];
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                }

                r[0]&=0xff, g[0]&=0xff, b[0]&=0xff;
                y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;

                (*p_line_dest_y) = (MMP_U8)y;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
        }
    }

    /* convert U, V */
    /*
         u0, u1 
         u2  u3   ==> U
    */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_src2 = p_image_src1 + rgb_stride;
        p_image_dest_u = p_image_u;
        p_image_dest_v = p_image_v;
        for(h = 0; h < pic_height; h+=2) {
        
            p_line_src1 = p_image_src1;
            p_line_src2 = p_image_src2;
            
            /* if odd height, h is last line */
            if( ((pic_height%2)==1) && (h==(pic_height-1)) ){
                p_line_src2 = p_line_src1;
            }

            p_line_dest_u = p_image_dest_u;
            p_line_dest_v = p_image_dest_v;
            for(w = 0; w < pic_width; w+=2 ) {
             
                /* if odd height, h is last line */
                if( ((pic_width%2)==1) && (w==(pic_width-1)) ){
                    is_end_col = MMP_TRUE;
                }
                else {
                    is_end_col = MMP_FALSE;
                }

                /*
                       u0, u1 
                       u2  u3   ==> U
                 */
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888:
                        b[0] = p_line_src1[0], g[0] = p_line_src1[1], r[0] = p_line_src1[2];
                        b[2] = p_line_src2[0], g[2] = p_line_src2[1], r[2] = p_line_src2[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = p_line_src1[3], g[1] = p_line_src1[4], r[1] = p_line_src1[5];
                            b[3] = p_line_src2[3], g[3] = p_line_src2[4], r[3] = p_line_src2[5];
                        }
                        else {
                            b[1] = b[0], g[1] = g[0], r[1] = r[0];
                            b[3] = b[2], g[3] = g[2], r[3] = r[2];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:
                        r[0] = p_line_src1[0], g[0] = p_line_src1[1], b[0] = p_line_src1[2];
                        r[2] = p_line_src2[0], g[2] = p_line_src2[1], b[2] = p_line_src2[2];
                        if(is_end_col == MMP_FALSE) {
                            r[1] = p_line_src1[3], g[1] = p_line_src1[4], b[1] = p_line_src1[5];
                            r[3] = p_line_src2[3], g[3] = p_line_src2[4], b[3] = p_line_src2[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                            r[3] = r[2], g[3] = g[2], b[3] = b[2];
                        }
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                        r[1] = g[1] = b[1] = 0;
                        r[2] = g[2] = b[2] = 0;
                        r[3] = g[3] = b[3] = 0;
                }

                for(i = 0; i < 4; i++) {
                    r[i]&=0xff, g[i]&=0xff, b[i]&=0xff;
                    u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
                    v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
                }
         
                (*p_line_dest_u) = (MMP_U8)((u[0] + u[1] + u[2] + u[3])/4);
                (*p_line_dest_v) = (MMP_U8)((v[0] + v[1] + v[2] + v[3])/4);
            
                p_line_src1 += rgb_pixel_byte*2;
                p_line_src2 += rgb_pixel_byte*2;
                p_line_dest_u++;
                p_line_dest_v++;
            }

            p_image_src1 += rgb_stride*2;
            p_image_src2 += rgb_stride*2;

            p_image_dest_u += y_stride/2;
            p_image_dest_v += y_stride/2;
        }
    }

    return mmpResult;
}

/********************************************************** 
    Tool of JPEG
***********************************************************/
MMP_RESULT CMmpImageTool::Jpeg_GetWidthHeight(MMP_U8* filename, MMP_OUT MMP_U32 *pic_width, MMP_OUT MMP_U32* pic_height) {

    FILE* fp;
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* jpegdata = NULL;
    MMP_U32 jpegsize = 0;


    fp = fopen((const char*)filename, "rb");
    if(fp != NULL) {

        fseek(fp, 0, SEEK_END);
        jpegsize = (MMP_U32)ftell(fp);
        fseek(fp, 0, SEEK_SET);

        jpegdata = (MMP_U8*)malloc(jpegsize);
        if(jpegdata != NULL) {
            fread(jpegdata, 1, jpegsize, fp);
        }
        fclose(fp);
    }

    if(jpegdata != NULL) {

        mmpResult = CMmpImageTool::Jpeg_GetWidthHeight(jpegdata, jpegsize, pic_width, pic_height);
        free(jpegdata);
    }
    
    return mmpResult;
}

MMP_RESULT CMmpImageTool::Jpeg_GetWidthHeight(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *pic_width, MMP_OUT MMP_U32* pic_height) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 sof_offset;
    MMP_U8* p;
    MMP_U16 w,h;

    if(pic_width){ *pic_width = 0; }
    if(pic_height){ *pic_height = 0; }

    mmpResult = CMmpImageTool::Jpeg_Get_SOF_Offset(jpegdata, jpegsize, &sof_offset);
    if(mmpResult == MMP_SUCCESS) {
        
        p = &jpegdata[sof_offset];
        p+=5; /* skip code,len,sampling precision */
        memcpy(&h, p, 2); p+=2; /*width*/
        memcpy(&w, p, 2); p+=2; /*height*/

        w = MMP_SWAP_U16(w);
        h = MMP_SWAP_U16(h);

        if(pic_width){ *pic_width = ((MMP_U32)w)&0xFFFF; }
        if(pic_height){ *pic_height = ((MMP_U32)h)&0xFFFF; }
    }

    return mmpResult;
}

/* search SOF (Start of Frame) */
MMP_RESULT CMmpImageTool::Jpeg_Get_SOF_Offset(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *sof_offset) {
    
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U8* p;
    MMP_U16 code, len;
    MMP_S32 remain_byte;
    MMP_U32 offset;
    

    p = jpegdata;
    memcpy(&code, p, 2);
    if(code ==  JPEG_SOI_CODE ) {
        p+=2;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    if(mmpResult == MMP_SUCCESS) {

        mmpResult = MMP_FAILURE;
        offset = 2;
        remain_byte = (MMP_S32)jpegsize;
        while(remain_byte > 4) {
            
            memcpy(&code, p, 2); p+=2;
            memcpy(&len, p, 2); 

            len = MMP_SWAP_U16(len);

            if(code == JPEG_SOF_CODE) {
                mmpResult = MMP_SUCCESS;
                if(sof_offset) *sof_offset = offset;
                break;
            }
            else {
                p += len;
                remain_byte -= 2+len;
                offset += 2+len;
            }
        }

    }

    return mmpResult;
}



/********************************************
Exit Def
********************************************/
#define EXIF_LOG2(x)                    (log((double)(x)) / log(2.0))
#define APEX_FNUM_TO_APERTURE(x)        ((int)(EXIF_LOG2((double)(x)) * 2 + 0.5))
#define APEX_EXPOSURE_TO_SHUTTER(x)     ((x) >= 1 ?                                 \
                                        (int)(-(EXIF_LOG2((double)(x)) + 0.5)) :    \
                                        (int)(-(EXIF_LOG2((double)(x)) - 0.5)))
#define APEX_ISO_TO_FILMSENSITIVITY(x)  ((int)(EXIF_LOG2((x) / 3.125) + 0.5))

#define NUM_SIZE                    2
#define IFD_SIZE                    12
#define OFFSET_SIZE                 4

#define NUM_0TH_IFD_TIFF            10
#define NUM_0TH_IFD_EXIF            22
#define NUM_0TH_IFD_GPS             10
#define NUM_1TH_IFD_TIFF            9

/* Type */
#define EXIF_TYPE_BYTE              1
#define EXIF_TYPE_ASCII             2
#define EXIF_TYPE_SHORT             3
#define EXIF_TYPE_LONG              4
#define EXIF_TYPE_RATIONAL          5
#define EXIF_TYPE_UNDEFINED         7
#define EXIF_TYPE_SLONG             9
#define EXIF_TYPE_SRATIONAL         10

#define EXIF_FILE_SIZE              28800

/* 0th IFD TIFF Tags */
#define EXIF_TAG_IMAGE_WIDTH                    0x0100
#define EXIF_TAG_IMAGE_HEIGHT                   0x0101
#define EXIF_TAG_MAKE                           0x010f
#define EXIF_TAG_MODEL                          0x0110
#define EXIF_TAG_ORIENTATION                    0x0112
#define EXIF_TAG_SOFTWARE                       0x0131
#define EXIF_TAG_DATE_TIME                      0x0132
#define EXIF_TAG_YCBCR_POSITIONING              0x0213
#define EXIF_TAG_EXIF_IFD_POINTER               0x8769
#define EXIF_TAG_GPS_IFD_POINTER                0x8825

/* 0th IFD Exif Private Tags */
#define EXIF_TAG_EXPOSURE_TIME                  0x829A
#define EXIF_TAG_FNUMBER                        0x829D
#define EXIF_TAG_EXPOSURE_PROGRAM               0x8822
#define EXIF_TAG_ISO_SPEED_RATING               0x8827
#define EXIF_TAG_EXIF_VERSION                   0x9000
#define EXIF_TAG_DATE_TIME_ORG                  0x9003
#define EXIF_TAG_DATE_TIME_DIGITIZE             0x9004
#define EXIF_TAG_SHUTTER_SPEED                  0x9201
#define EXIF_TAG_APERTURE                       0x9202
#define EXIF_TAG_BRIGHTNESS                     0x9203
#define EXIF_TAG_EXPOSURE_BIAS                  0x9204
#define EXIF_TAG_MAX_APERTURE                   0x9205
#define EXIF_TAG_METERING_MODE                  0x9207
#define EXIF_TAG_FLASH                          0x9209
#define EXIF_TAG_FOCAL_LENGTH                   0x920A
#define EXIF_TAG_USER_COMMENT                   0x9286
#define EXIF_TAG_COLOR_SPACE                    0xA001
#define EXIF_TAG_PIXEL_X_DIMENSION              0xA002
#define EXIF_TAG_PIXEL_Y_DIMENSION              0xA003
#define EXIF_TAG_EXPOSURE_MODE                  0xA402
#define EXIF_TAG_WHITE_BALANCE                  0xA403
#define EXIF_TAG_SCENCE_CAPTURE_TYPE            0xA406

/* 0th IFD GPS Info Tags */
#define EXIF_TAG_GPS_VERSION_ID                 0x0000
#define EXIF_TAG_GPS_LATITUDE_REF               0x0001
#define EXIF_TAG_GPS_LATITUDE                   0x0002
#define EXIF_TAG_GPS_LONGITUDE_REF              0x0003
#define EXIF_TAG_GPS_LONGITUDE                  0x0004
#define EXIF_TAG_GPS_ALTITUDE_REF               0x0005
#define EXIF_TAG_GPS_ALTITUDE                   0x0006
#define EXIF_TAG_GPS_TIMESTAMP                  0x0007
#define EXIF_TAG_GPS_PROCESSING_METHOD          0x001B
#define EXIF_TAG_GPS_DATESTAMP                  0x001D

/* 1th IFD TIFF Tags */
#define EXIF_TAG_COMPRESSION_SCHEME             0x0103
#define EXIF_TAG_X_RESOLUTION                   0x011A
#define EXIF_TAG_Y_RESOLUTION                   0x011B
#define EXIF_TAG_RESOLUTION_UNIT                0x0128
#define EXIF_TAG_JPEG_INTERCHANGE_FORMAT        0x0201
#define EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN    0x0202

typedef enum {
    EXIF_ORIENTATION_UP     = 1,
    EXIF_ORIENTATION_90     = 6,
    EXIF_ORIENTATION_180    = 3,
    EXIF_ORIENTATION_270    = 8,
} ExifOrientationType;

typedef enum {
    EXIF_SCENE_STANDARD,
    EXIF_SCENE_LANDSCAPE,
    EXIF_SCENE_PORTRAIT,
    EXIF_SCENE_NIGHT,
} CamExifSceneCaptureType;

typedef enum {
    EXIF_METERING_UNKNOWN,
    EXIF_METERING_AVERAGE,
    EXIF_METERING_CENTER,
    EXIF_METERING_SPOT,
    EXIF_METERING_MULTISPOT,
    EXIF_METERING_PATTERN,
    EXIF_METERING_PARTIAL,
    EXIF_METERING_OTHER     = 255,
} CamExifMeteringModeType;

typedef enum {
    EXIF_EXPOSURE_AUTO,
    EXIF_EXPOSURE_MANUAL,
    EXIF_EXPOSURE_AUTO_BRACKET,
} CamExifExposureModeType;

typedef enum {
    EXIF_WB_AUTO,
    EXIF_WB_MANUAL,
} CamExifWhiteBalanceType;

/* Values */
#define EXIF_DEF_MAKER          "SAMSUNG"
#define EXIF_DEF_MODEL          "SAMSUNG"
#define EXIF_DEF_SOFTWARE       "SAMSUNG"
#define EXIF_DEF_EXIF_VERSION   "0220"
#define EXIF_DEF_USERCOMMENTS   "User comments"

#define EXIF_DEF_YCBCR_POSITIONING  1   /* centered */
#define EXIF_DEF_FNUMBER_NUM        26  /* 2.6 */
#define EXIF_DEF_FNUMBER_DEN        10
#define EXIF_DEF_EXPOSURE_PROGRAM   3   /* aperture priority */
#define EXIF_DEF_FOCAL_LEN_NUM      278 /* 2.78mm */
#define EXIF_DEF_FOCAL_LEN_DEN      100
#define EXIF_DEF_FLASH              0   /* O: off, 1: on*/
#define EXIF_DEF_COLOR_SPACE        1
#define EXIF_DEF_EXPOSURE_MODE      EXIF_EXPOSURE_AUTO
#define EXIF_DEF_APEX_DEN           10

#define EXIF_DEF_COMPRESSION        6
#define EXIF_DEF_RESOLUTION_NUM     72
#define EXIF_DEF_RESOLUTION_DEN     1
#define EXIF_DEF_RESOLUTION_UNIT    2   /* inches */

typedef struct {
    MMP_U32 num;
    MMP_U32 den;
} rational_t;

typedef struct {
    MMP_S32 num;
    MMP_S32 den;
} srational_t;

typedef struct {
    bool enableGps;
    bool enableThumb;

    unsigned char maker[32];
    unsigned char model[32];
    unsigned char software[32];
    unsigned char exif_version[4];
    unsigned char date_time[20];
    unsigned char user_comment[150];

    MMP_U32 width;
    MMP_U32 height;
    MMP_U32 widthThumb;
    MMP_U32 heightThumb;

    MMP_U16 orientation;
    MMP_U16 ycbcr_positioning;
    MMP_U16 exposure_program;
    MMP_U16 iso_speed_rating;
    MMP_U16 metering_mode;
    MMP_U16 flash;
    MMP_U16 color_space;
    MMP_U16 exposure_mode;
    MMP_U16 white_balance;
    MMP_U16 scene_capture_type;

    rational_t exposure_time;
    rational_t fnumber;
    rational_t aperture;
    rational_t max_aperture;
    rational_t focal_length;

    srational_t shutter_speed;
    srational_t brightness;
    srational_t exposure_bias;

    unsigned char gps_latitude_ref[2];
    unsigned char gps_longitude_ref[2];

    MMP_U8 gps_version_id[4];
    MMP_U8 gps_altitude_ref;

    rational_t gps_latitude[3];
    rational_t gps_longitude[3];
    rational_t gps_altitude;
    rational_t gps_timestamp[3];
    unsigned char gps_datestamp[11];
    unsigned char gps_processing_method[100];

    rational_t x_resolution;
    rational_t y_resolution;
    MMP_U16 resolution_unit;
    MMP_U16 compression_scheme;
} exif_attribute_t;

inline void writeExifIfd(unsigned char **pCur,
                             unsigned short tag,
                             unsigned short type,
                             unsigned int count,
                             MMP_U32 value)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, &value, 4);
    *pCur += 4;
}

inline void writeExifIfd(unsigned char **pCur,
                                         unsigned short tag,
                                         unsigned short type,
                                         unsigned int count,
                                         unsigned char *pValue)
{
    char buf[4] = { 0,};

    memcpy(buf, pValue, count);
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, buf, 4);
    *pCur += 4;
}

inline void writeExifIfd(unsigned char **pCur,
                                         unsigned short tag,
                                         unsigned short type,
                                         unsigned int count,
                                         unsigned char *pValue,
                                         unsigned int *offset,
                                         unsigned char *start)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, offset, 4);
    *pCur += 4;
    memcpy(start + *offset, pValue, count);
    *offset += count;
}

inline void writeExifIfd(unsigned char **pCur,
                                         unsigned short tag,
                                         unsigned short type,
                                         unsigned int count,
                                         rational_t *pValue,
                                         unsigned int *offset,
                                         unsigned char *start)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, offset, 4);
    *pCur += 4;
    memcpy(start + *offset, pValue, 8 * count);
    *offset += 8 * count;
}

static void setExifFixedAttribute(exif_attribute_t *exifInfo)
{
    //char property[PROPERTY_VALUE_MAX];

    //2 0th IFD TIFF Tags
    //3 Maker
    //property_get("ro.product.brand", property, EXIF_DEF_MAKER);
    //strncpy((char *)exifInfo->maker, property, sizeof(exifInfo->maker) - 1);
    //exifInfo->maker[sizeof(exifInfo->maker) - 1] = '\0';
    strcpy((char*)exifInfo->maker, "MAKER: hthwang@anapass.com");

    //3 Model
    //property_get("ro.product.model", property, EXIF_DEF_MODEL);
    //strncpy((char *)exifInfo->model, property, sizeof(exifInfo->model) - 1);
    //exifInfo->model[sizeof(exifInfo->model) - 1] = '\0';
    strcpy((char*)exifInfo->model, "MODEL: hthwang@anapass.com");

    //3 Software
    //property_get("ro.build.id", property, EXIF_DEF_SOFTWARE);
    //strncpy((char *)exifInfo->software, property, sizeof(exifInfo->software) - 1);
    //exifInfo->software[sizeof(exifInfo->software) - 1] = '\0';
    strcpy((char*)exifInfo->software, "SOFTWARE: hthwang@anapass.com");

    //3 YCbCr Positioning
    exifInfo->ycbcr_positioning = EXIF_DEF_YCBCR_POSITIONING;

    //2 0th IFD Exif Private Tags
    //3 F Number
    exifInfo->fnumber.num = EXIF_DEF_FNUMBER_NUM;
    exifInfo->fnumber.den = EXIF_DEF_FNUMBER_DEN;
    //3 Exposure Program
    exifInfo->exposure_program = EXIF_DEF_EXPOSURE_PROGRAM;
    //3 Exif Version
    memcpy(exifInfo->exif_version, EXIF_DEF_EXIF_VERSION, sizeof(exifInfo->exif_version));
    //3 Aperture
    MMP_U32 av = APEX_FNUM_TO_APERTURE((double)exifInfo->fnumber.num/exifInfo->fnumber.den);
    exifInfo->aperture.num = av*EXIF_DEF_APEX_DEN;
    exifInfo->aperture.den = EXIF_DEF_APEX_DEN;
    //3 Maximum lens aperture
    exifInfo->max_aperture.num = exifInfo->aperture.num;
    exifInfo->max_aperture.den = exifInfo->aperture.den;
    //3 Lens Focal Length
#if 0
    if (m_camera_id == CAMERA_ID_BACK)
        exifInfo->focal_length.num = BACK_CAMERA_FOCAL_LENGTH;
    else
        exifInfo->focal_length.num = FRONT_CAMERA_FOCAL_LENGTH;
    exifInfo->focal_length.den = EXIF_DEF_FOCAL_LEN_DEN;
#else
    exifInfo->focal_length.num = 1;
    exifInfo->focal_length.den = 1;
#endif

    
    //3 User Comments
    strcpy((char *)exifInfo->user_comment, EXIF_DEF_USERCOMMENTS);
    //3 Color Space information
    exifInfo->color_space = EXIF_DEF_COLOR_SPACE;
    //3 Exposure Mode
    exifInfo->exposure_mode = EXIF_DEF_EXPOSURE_MODE;

    //2 0th IFD GPS Info Tags
    unsigned char gps_version[4] = { 0x02, 0x02, 0x00, 0x00 };
    memcpy(exifInfo->gps_version_id, gps_version, sizeof(gps_version));

    //2 1th IFD TIFF Tags
    exifInfo->compression_scheme = EXIF_DEF_COMPRESSION;
    exifInfo->x_resolution.num = EXIF_DEF_RESOLUTION_NUM;
    exifInfo->x_resolution.den = EXIF_DEF_RESOLUTION_DEN;
    exifInfo->y_resolution.num = EXIF_DEF_RESOLUTION_NUM;
    exifInfo->y_resolution.den = EXIF_DEF_RESOLUTION_DEN;
    exifInfo->resolution_unit = EXIF_DEF_RESOLUTION_UNIT;
}

int CMmpImageTool::Jpeg_makeExif(unsigned char *exifOut,
                          unsigned char *thumb_buf,
                          unsigned int thumb_size,
                          unsigned int *size,
                          bool useMainbufForThumb)
{
    unsigned char *pCur, *pApp1Start, *pIfdStart, *pGpsIfdPtr, *pNextIfdOffset;
    unsigned int tmp, LongerTagOffest = 0;
    pApp1Start = pCur = exifOut;

    exif_attribute_t exifInfoObj;
    exif_attribute_t *exifInfo =&exifInfoObj;

    setExifFixedAttribute(exifInfo);

    //2 Exif Identifier Code & TIFF Header
    pCur += 4;  // Skip 4 Byte for APP1 marker and length
    unsigned char ExifIdentifierCode[6] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
    memcpy(pCur, ExifIdentifierCode, 6);
    pCur += 6;

    /* Byte Order - little endian, Offset of IFD - 0x00000008.H */
    unsigned char TiffHeader[8] = { 0x49, 0x49, 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00 };
    memcpy(pCur, TiffHeader, 8);
    pIfdStart = pCur;
    pCur += 8;

    //2 0th IFD TIFF Tags
    if (exifInfo->enableGps)
        tmp = NUM_0TH_IFD_TIFF;
    else
        tmp = NUM_0TH_IFD_TIFF - 1;

    memcpy(pCur, &tmp, NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffest += 8 + NUM_SIZE + tmp*IFD_SIZE + OFFSET_SIZE;

    writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG,
                 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG,
                 1, exifInfo->height);
    writeExifIfd(&pCur, EXIF_TAG_MAKE, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->maker) + 1, exifInfo->maker, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MODEL, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->model) + 1, exifInfo->model, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT,
                 1, exifInfo->orientation);
    writeExifIfd(&pCur, EXIF_TAG_SOFTWARE, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->software) + 1, exifInfo->software, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_YCBCR_POSITIONING, EXIF_TYPE_SHORT,
                 1, exifInfo->ycbcr_positioning);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_IFD_POINTER, EXIF_TYPE_LONG,
                 1, LongerTagOffest);
    if (exifInfo->enableGps) {
        pGpsIfdPtr = pCur;
        pCur += IFD_SIZE;   // Skip a ifd size for gps IFD pointer
    }

    pNextIfdOffset = pCur;  // Skip a offset size for next IFD offset
    pCur += OFFSET_SIZE;

    //2 0th IFD Exif Private Tags
    pCur = pIfdStart + LongerTagOffest;

    tmp = NUM_0TH_IFD_EXIF;
    memcpy(pCur, &tmp , NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffest += NUM_SIZE + NUM_0TH_IFD_EXIF*IFD_SIZE + OFFSET_SIZE;

    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_TIME, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->exposure_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_FNUMBER, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->fnumber, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_PROGRAM, EXIF_TYPE_SHORT,
                 1, exifInfo->exposure_program);
    writeExifIfd(&pCur, EXIF_TAG_ISO_SPEED_RATING, EXIF_TYPE_SHORT,
                 1, exifInfo->iso_speed_rating);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_VERSION, EXIF_TYPE_UNDEFINED,
                 4, exifInfo->exif_version);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_ORG, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_DIGITIZE, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SHUTTER_SPEED, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->shutter_speed, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_APERTURE, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->aperture, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_BRIGHTNESS, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->brightness, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_BIAS, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->exposure_bias, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MAX_APERTURE, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->max_aperture, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_METERING_MODE, EXIF_TYPE_SHORT,
                 1, exifInfo->metering_mode);
    writeExifIfd(&pCur, EXIF_TAG_FLASH, EXIF_TYPE_SHORT,
                 1, exifInfo->flash);
    writeExifIfd(&pCur, EXIF_TAG_FOCAL_LENGTH, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->focal_length, &LongerTagOffest, pIfdStart);
    char code[8] = { 0x00, 0x00, 0x00, 0x49, 0x49, 0x43, 0x53, 0x41 };
    int commentsLen = strlen((char *)exifInfo->user_comment) + 1;
    memmove(exifInfo->user_comment + sizeof(code), exifInfo->user_comment, commentsLen);
    memcpy(exifInfo->user_comment, code, sizeof(code));
    writeExifIfd(&pCur, EXIF_TAG_USER_COMMENT, EXIF_TYPE_UNDEFINED,
                 commentsLen + sizeof(code), exifInfo->user_comment, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_COLOR_SPACE, EXIF_TYPE_SHORT,
                 1, exifInfo->color_space);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_X_DIMENSION, EXIF_TYPE_LONG,
                 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_Y_DIMENSION, EXIF_TYPE_LONG,
                 1, exifInfo->height);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_MODE, EXIF_TYPE_LONG,
                 1, exifInfo->exposure_mode);
    writeExifIfd(&pCur, EXIF_TAG_WHITE_BALANCE, EXIF_TYPE_LONG,
                 1, exifInfo->white_balance);
    writeExifIfd(&pCur, EXIF_TAG_SCENCE_CAPTURE_TYPE, EXIF_TYPE_LONG,
                 1, exifInfo->scene_capture_type);
    tmp = 0;
    memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
    pCur += OFFSET_SIZE;

    //2 0th IFD GPS Info Tags
    if (exifInfo->enableGps) {
        writeExifIfd(&pGpsIfdPtr, EXIF_TAG_GPS_IFD_POINTER, EXIF_TYPE_LONG,
                     1, LongerTagOffest); // GPS IFD pointer skipped on 0th IFD

        pCur = pIfdStart + LongerTagOffest;

        if (exifInfo->gps_processing_method[0] == 0) {
            // don't create GPS_PROCESSING_METHOD tag if there isn't any
            tmp = NUM_0TH_IFD_GPS - 1;
        } else {
            tmp = NUM_0TH_IFD_GPS;
        }
        memcpy(pCur, &tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        LongerTagOffest += NUM_SIZE + tmp*IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_GPS_VERSION_ID, EXIF_TYPE_BYTE,
                     4, exifInfo->gps_version_id);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE_REF, EXIF_TYPE_ASCII,
                     2, exifInfo->gps_latitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_latitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE_REF, EXIF_TYPE_ASCII,
                     2, exifInfo->gps_longitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_longitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE_REF, EXIF_TYPE_BYTE,
                     1, exifInfo->gps_altitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->gps_altitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_TIMESTAMP, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_timestamp, &LongerTagOffest, pIfdStart);
        tmp = strlen((char*)exifInfo->gps_processing_method);
        if (tmp > 0) {
            if (tmp > 100) {
                tmp = 100;
            }
            static const char ExifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };
            unsigned char tmp_buf[100+sizeof(ExifAsciiPrefix)];
            memcpy(tmp_buf, ExifAsciiPrefix, sizeof(ExifAsciiPrefix));
            memcpy(&tmp_buf[sizeof(ExifAsciiPrefix)], exifInfo->gps_processing_method, tmp);
            writeExifIfd(&pCur, EXIF_TAG_GPS_PROCESSING_METHOD, EXIF_TYPE_UNDEFINED,
                         tmp+sizeof(ExifAsciiPrefix), tmp_buf, &LongerTagOffest, pIfdStart);
        }
        writeExifIfd(&pCur, EXIF_TAG_GPS_DATESTAMP, EXIF_TYPE_ASCII,
                     11, exifInfo->gps_datestamp, &LongerTagOffest, pIfdStart);
        tmp = 0;
        memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
        pCur += OFFSET_SIZE;
    }

    //2 1th IFD TIFF Tags

    unsigned char *thumbBuf = thumb_buf;
    unsigned int thumbSize = thumb_size;

    if (exifInfo->enableThumb && (thumbBuf != NULL) && (thumbSize > 0)) {
        tmp = LongerTagOffest;
        memcpy(pNextIfdOffset, &tmp, OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD

        pCur = pIfdStart + LongerTagOffest;

        tmp = NUM_1TH_IFD_TIFF;
        memcpy(pCur, &tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        LongerTagOffest += NUM_SIZE + NUM_1TH_IFD_TIFF*IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG,
                     1, exifInfo->widthThumb);
        writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG,
                     1, exifInfo->heightThumb);
        writeExifIfd(&pCur, EXIF_TAG_COMPRESSION_SCHEME, EXIF_TYPE_SHORT,
                     1, exifInfo->compression_scheme);
        writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT,
                     1, exifInfo->orientation);
        writeExifIfd(&pCur, EXIF_TAG_X_RESOLUTION, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->x_resolution, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_Y_RESOLUTION, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->y_resolution, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_RESOLUTION_UNIT, EXIF_TYPE_SHORT,
                     1, exifInfo->resolution_unit);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT, EXIF_TYPE_LONG,
                     1, LongerTagOffest);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN, EXIF_TYPE_LONG,
                     1, thumbSize);

        tmp = 0;
        memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
        pCur += OFFSET_SIZE;

        memcpy(pIfdStart + LongerTagOffest, thumbBuf, thumbSize);
        LongerTagOffest += thumbSize;
    } else {
        tmp = 0;
        memcpy(pNextIfdOffset, &tmp, OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD
    }

    unsigned char App1Marker[2] = { 0xff, 0xe1 };
    memcpy(pApp1Start, App1Marker, 2);
    pApp1Start += 2;

    *size = 10 + LongerTagOffest;
    tmp = *size - 2;    // APP1 Maker isn't counted
    unsigned char size_mm[2] = {(tmp >> 8) & 0xFF, tmp & 0xFF};
    memcpy(pApp1Start, size_mm, 2);

    //ALOGD("makeExif X");

    return 0;
}


/********************************************************** 
        Tool of BMP
***********************************************************/

MMP_RESULT CMmpImageTool::Bmp_SaveFile(MMP_CHAR* bmp_filename, MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image, MMP_U32 fourcc) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    FILE* fp = NULL;
    MMPBITMAPFILEHEADER BmpFileHeader;
    MMPBITMAPINFOHEADER BmpInfoHeader;
    MMP_S32 stride, wrsz;
    
    /* file open */
    if(mmpResult == MMP_SUCCESS) {
        fp = fopen(bmp_filename, "wb");
        if(fp == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: open file(%s)"), MMP_CLASS_FUNC, bmp_filename ));
        }
    }

    /* check stride */
    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_BGR888:
        case MMP_FOURCC_IMAGE_RGB888: stride = MMP_BYTE_ALIGN(pic_width*3,4); break;
        case MMP_FOURCC_IMAGE_ARGB8888: stride = pic_width*4; break;
        default:
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpImageTool::%s] FAIL: not support format (%c%c%c%c) "), MMP_CLASS_FUNC, 
                                                     MMPGETFOURCCARG(fourcc)
                                                     ));
    }

    /* write file header */
    if(mmpResult == MMP_SUCCESS) {
        BmpFileHeader.bfType=BMP_HEADER_MARKER;
        BmpFileHeader.bfSize=sizeof(MMPBITMAPFILEHEADER)+sizeof(MMPBITMAPINFOHEADER)+ stride*pic_height;
        BmpFileHeader.bfOffBits=sizeof(MMPBITMAPFILEHEADER)+sizeof(MMPBITMAPINFOHEADER);
        wrsz = fwrite(&BmpFileHeader, 1, sizeof(MMPBITMAPFILEHEADER), fp);
        if(wrsz != sizeof(MMPBITMAPFILEHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: write bmp file header"), MMP_CLASS_FUNC ));
        }
    }

    /* write infoheader */
    if(mmpResult == MMP_SUCCESS) {
        memset(&BmpInfoHeader, 0, sizeof(BmpInfoHeader) );
        BmpInfoHeader.biSize = sizeof(BmpInfoHeader);
        BmpInfoHeader.biWidth = pic_width;
        BmpInfoHeader.biHeight = -pic_height; /* Default : Top=>Bottom */
        
        switch(fourcc) {
            case MMP_FOURCC_IMAGE_BGR888:
            case MMP_FOURCC_IMAGE_RGB888:
                BmpInfoHeader.biPlanes = 1;
                BmpInfoHeader.biBitCount = 24;
                BmpInfoHeader.biCompression = MMP_BI_RGB;
                break;

            case MMP_FOURCC_IMAGE_ARGB8888:
                BmpInfoHeader.biPlanes = 1;
                BmpInfoHeader.biBitCount = 32;
                BmpInfoHeader.biCompression = MMP_BI_RGB;
                break;
        }

        BmpInfoHeader.biSizeImage = stride*pic_height;
        wrsz = fwrite(&BmpInfoHeader, 1, sizeof(MMPBITMAPINFOHEADER), fp);
        if(wrsz != sizeof(MMPBITMAPINFOHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: write bmp info header"), MMP_CLASS_FUNC ));
        }
    }
    
    /* write image data */
    if(mmpResult == MMP_SUCCESS) {

        switch(fourcc) {

            
            case MMP_FOURCC_IMAGE_RGB888: /* RGB888 은  Bitmap File 은  BGR BRG 으로 저장해야 한다. C:\MediaSample\Bmp\BLUE_RGB888.bmp.bin  RED_RGB888.bmp.bin 참고*/
                {
                    MMP_U8 r, g, b;
                    MMP_S32 w, h, ip;
                    MMP_U8* line_dest = (MMP_U8*)MMP_MALLOC(stride), *line_dest1;
                    MMP_U8* line_src = p_image, *line_src1;
                    if(line_dest != NULL) {
                        

                        for(h = 0; h < pic_height; h++) {
                            
                            line_dest1 = line_dest;
                            line_src1 = line_src;
                            for(ip = 0, w = 0; w < pic_width; w++) {

                                r= *line_src1; line_src1++;
                                g = *line_src1; line_src1++;
                                b = *line_src1; line_src1++;

                                /* B,G, R 순으로 저장해야 한다. */
                                *line_dest1 = b; line_dest1++;
                                *line_dest1 = g; line_dest1++;
                                *line_dest1 = r; line_dest1++;
                            }

                            line_src += stride;
                            fwrite(line_dest, 1, stride, fp);
                        }
                    
                        MMP_FREE(line_dest);
                    }
                    else {
                        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: alloc BGR888 line sz=%d"), MMP_CLASS_FUNC, stride ));
                    }
                }
                break;

            case MMP_FOURCC_IMAGE_ARGB8888:
            case MMP_FOURCC_IMAGE_BGR888:
            default:
                wrsz = fwrite(p_image, 1, BmpInfoHeader.biSizeImage, fp);
                if(wrsz != BmpInfoHeader.biSizeImage) {
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: write image data (wrtten_sz = %d / %d"), MMP_CLASS_FUNC, wrsz,  BmpInfoHeader.biSizeImage));
                }
        }
        
    }

    if(fp != NULL) {
       fclose(fp);
    }

   return mmpResult;
}

MMP_CHAR* CMmpImageTool::Bmp_GetName(enum MMP_FOURCC fourcc, MMP_CHAR* buf) {
    
    
    switch(fourcc) {
        case MMP_FOURCC_IMAGE_RGB888  :  strcpy(buf, "RGB888");   break;
	    case MMP_FOURCC_IMAGE_BGR888  :  strcpy(buf, "BGR888");   break;
	    case MMP_FOURCC_IMAGE_RGB565  :  strcpy(buf, "RGB565");   break;
	    case MMP_FOURCC_IMAGE_BGR565  :  strcpy(buf, "BGR565");   break;
	    case MMP_FOURCC_IMAGE_ARGB1555:  strcpy(buf, "ARGB1555");   break;
	    case MMP_FOURCC_IMAGE_BGRA1555:  strcpy(buf, "BGRA1555");   break;
	    case MMP_FOURCC_IMAGE_RGBA1555:  strcpy(buf, "RGBA1555");   break;
	    case MMP_FOURCC_IMAGE_ABGR1555:  strcpy(buf, "ABGR1555");   break;
	    case MMP_FOURCC_IMAGE_ARGB8888:  strcpy(buf, "ARGB8888");   break; 
	    case MMP_FOURCC_IMAGE_BGRA8888:  strcpy(buf, "BGRA8888");   break; 
	    case MMP_FOURCC_IMAGE_RGBA8888:  strcpy(buf, "RGBA8888");   break; 
	    case MMP_FOURCC_IMAGE_ABGR8888:  strcpy(buf, "ABGR8888");   break; 

        default:  strcpy(buf, "---");   break;

    }

    return buf;
}

