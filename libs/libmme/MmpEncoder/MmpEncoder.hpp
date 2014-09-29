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

#ifndef MMPENCODER_HPP__
#define MMPENCODER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "MmpMediaInfo.hpp"
#include "mmp_buffer_mgr.hpp"

struct MmpEncoderProperty_AnapassVPU{

    //YUV_SRC_IMG           \\naslab\006.stream\images\08.ENC_TEST\ski_1280x720.yuv	// source YUV image file
    MMP_U32 FRAME_NUMBER_ENCODED;//  1000   			// number of frames to be encoded
    MMP_U32 PICTURE_WIDTH;//         1280   			// picture width
    MMP_U32 PICTURE_HEIGHT;//        720   			// picture height
    MMP_U32 FRAME_RATE;//            30   			// frame rate
    MMP_U32 CONSTRAINED_INTRA;//     0   			// constrained_intra_pred_flag
    //;-----------------------
    //; DEBLKING FILTER
    //;-----------------------
    MMP_U32 DISABLE_DEBLK;//         0  			// disable_deblk (0 : enable, 1 : disable, 2 : disable at slice boundary)
    MMP_U32 DEBLK_ALPHA;//           0   			// deblk_filter_offset_alpha (-6 ~ 6)
    MMP_U32 DEBLK_BETA;//            0   			// deblk_filter_offset_beta  (-6 ~ 6)
    MMP_U32 CHROMA_QP_OFFSET;//      2   			// chroma_qp_offset (-12 ~ 12)
    MMP_U32 PIC_QP_Y;//              12   			// pic_qp_y (0 ~ 51)
    MMP_U32 GOP_PIC_NUMBER;//        30   			// GOP picture number (0 : only first I, 1 : all I, 3 : I,P,P,I,)
    //;-----------------------
    //; SLICE STRUCTURE
    //;-----------------------
    MMP_U32 SLICE_MODE;//            0   			// slice mode (0 : one slice, 1 : multiple slice)
    MMP_U32 SLICE_SIZE_MODE;//       0   			// slice size mode (0 : slice bit number, 1 : slice mb number)
    MMP_U32 SLICE_SIZE_NUMBER;//     8192   			// slice size number (bit count or mb number)
    //;-----------------------
    //; RATE CONTROL
    //;-----------------------
    MMP_U32 RATE_CONTROL_ENABLE;//   1   			// rate control enable
    MMP_U32 BIT_RATE_KBPS;//         3200 	// bit rate in kbps (ignored if rate control disable)
    MMP_U32 DELAY_IN_MS;//           0   			// delay in ms (initial decoder buffer delay) (0 : ignore)
    MMP_U32 VBV_BUFFER_SIZE;//       0   			// reference decoder buffer size in bits (0 : ignore)
    //;-----------------------
    //; ERROR RESILIENCE
    //;-----------------------
    MMP_U32 INTRA_MB_REFRESH;//      0  			// Intra MB Refresh (0 - None, 1 ~ MbNum-1)
    //;-----------------------
    //; ADDITIONAL PARAMETER
    //;-----------------------
    MMP_U32 SEARCH_RANGE;//            1   // 3: 16x16, 2:32x16, 1:64x32, 0:128x64, H.263(Short Header : always 0)
    MMP_U32 ME_USE_ZERO_PMV;//         1   // 0: PMV_ENABLE, 1: PMV_DISABLE
    MMP_U32 WEIGHT_INTRA_COST;//       0   // Intra weight when compare Intra and Inter

};

class CMmpEncoder
{
public:
    static CMmpEncoder* CreateVideoObject(struct MmpEncoderCreateConfig *pCreateConfig, MMP_BOOL bForceFFMpeg = MMP_FALSE);
    static CMmpEncoder* CreateVideoObject(MMP_U32 mmp_video_fourcc,
                                          MMP_U32 pic_width, MMP_U32 pic_height,
                                          MMP_U32 framerate, MMP_U32 idr_period, MMP_U32 bitrate,
                                          MMP_BOOL bForceFFMpeg = MMP_FALSE); 

    static CMmpEncoder* CreateVideoObject(MMP_U32 mmp_video_fourcc,
                                          struct MmpEncoderProperty_AnapassVPU prop,
                                          MMP_BOOL bForceFFMpeg = MMP_FALSE); 

    static MMP_RESULT DestroyObject(CMmpEncoder* pObj);

    static MMP_RESULT CreateVideoDefaultConfig(MMP_U32 mmp_video_fourcc,
                                          MMP_U32 pic_width, MMP_U32 pic_height,
                                          MMP_U32 framerate, MMP_U32 idr_period, MMP_U32 bitrate,
                                          MMP_OUT struct MmpEncoderCreateConfig *pMmpEncoderCreateConfig
                                          ); 

protected:
    CMmpMediaInfo* m_pMediaInfo;
	CMmpMediaInfo m_MediaInfo;

	MMP_U32 m_nFormat;
	MMP_U32 m_nStreamType;

	MMP_BOOL m_bConfigDSI;

    MMP_U32 m_nClassStartTick;

protected:
    CMmpEncoder(CMmpMediaInfo* pMediaInfo);
    CMmpEncoder(MMP_U32 nFormat, MMP_U32 nStreamType);
    virtual ~CMmpEncoder();

    virtual MMP_RESULT Open();
//	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize) = 0;
    virtual MMP_RESULT Close();

public:
	virtual MMP_RESULT EncodeAu(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {return MMP_FAILURE;}
	
    //virtual int GetPicWidth() { return 0; }
    //virtual int GetPicHeight() { return 0; }

	//virtual MMP_U32 GetAudioSampleRate() { return 0; }
	//virtual MMP_U32 GetAudioChannelCount() { return 0; }

	//MMP_BOOL IsConfigDSI() { return m_bConfigDSI; }
	//virtual MMP_RESULT GetWF(MMPWAVEFORMATEX* pwf) {return MMP_FAILURE;}
	
    virtual MMP_RESULT Flush() {return MMP_FAILURE;}
    MMP_U32 GetFormat() { return m_nFormat;}
   
};

#endif

