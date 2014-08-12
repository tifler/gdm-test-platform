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

#ifndef MMPDEFINE_OMXCOMP_H__
#define MMPDEFINE_OMXCOMP_H__

/* Audio Decoder */
#define OMX_COMPNAME_AUDIO_MP3_DECODER    "OMX.anapass.mp3.decoder"
#define OMX_COMPNAME_AUDIO_MP2_DECODER    "OMX.anapass.mpeg-L2.decoder"
#define OMX_COMPNAME_AUDIO_MP1_DECODER    "OMX.anapass.mpeg-L1.decoder"
//#define OMX_COMPNAME_AUDIO_WMAV2_DECODER  "OMX.anapass.wmav2.decoder"
#define OMX_COMPNAME_AUDIO_AC3_DECODER  "OMX.anapass.ac3.decoder"
#define OMX_COMPNAME_AUDIO_AAC_DECODER  "OMX.anapass.aac.decoder"
#define OMX_COMPNAME_AUDIO_FLAC_DECODER  "OMX.anapass.flac.decoder"
#define OMX_COMPNAME_AUDIO_ADPCM_MS_DECODER  "OMX.anapass.adpcm-ms.decoder"
#define OMX_COMPNAME_AUDIO_PCM_DECODER  "OMX.anapass.pcm.decoder"
#define OMX_COMPNAME_AUDIO_FFMPEG_DECODER  "OMX.anapass.ffmpeg_a.decoder"

/* Video Decoder */
#define OMX_COMPNAME_VIDEO_H264_DECODER   "OMX.anapass.h264.decoder"
#define OMX_COMPNAME_VIDEO_H263_DECODER   "OMX.anapass.h263.decoder"
#define OMX_COMPNAME_VIDEO_MPEG4_DECODER  "OMX.anapass.mpeg4.decoder"
#define OMX_COMPNAME_VIDEO_MPEG2_DECODER  "OMX.anapass.mpeg2.decoder"
//#define OMX_COMPNAME_VIDEO_MSMPEG4V1_DECODER  "OMX.anapass.msmpeg4v1.decoder"
//#define OMX_COMPNAME_VIDEO_MSMPEG4V2_DECODER  "OMX.anapass.msmpeg4v2.decoder"
//#define OMX_COMPNAME_VIDEO_MSMPEG4V3_DECODER  "OMX.anapass.msmpeg4v3.decoder"
#define OMX_COMPNAME_VIDEO_VC1_DECODER    "OMX.anapass.vc1.decoder"
//#define OMX_COMPNAME_VIDEO_WMV1_DECODER    "OMX.anapass.wmv1.decoder"
//#define OMX_COMPNAME_VIDEO_WMV2_DECODER    "OMX.anapass.wmv2.decoder"
//#define OMX_COMPNAME_VIDEO_WMV3_DECODER    "OMX.anapass.wmv3.decoder"
//#define OMX_COMPNAME_VIDEO_VP6_DECODER    "OMX.anapass.vp6.decoder"
//#define OMX_COMPNAME_VIDEO_VP6F_DECODER    "OMX.anapass.vp6f.decoder"
//#define OMX_COMPNAME_VIDEO_VP6A_DECODER    "OMX.anapass.vp6a.decoder"
//#define OMX_COMPNAME_VIDEO_RV30_DECODER    "OMX.anapass.rv30.decoder"
//#define OMX_COMPNAME_VIDEO_RV40_DECODER    "OMX.anapass.rv40.decoder"
//#define OMX_COMPNAME_VIDEO_SVQ1_DECODER    "OMX.anapass.svq1.decoder" //sorenson 1
//#define OMX_COMPNAME_VIDEO_SVQ3_DECODER    "OMX.anapass.svq3.decoder" //sorenson 3
#define OMX_COMPNAME_VIDEO_THEORA_DECODER  "OMX.anapass.theora.decoder" 
#define OMX_COMPNAME_VIDEO_MJPEG_DECODER  "OMX.anapass.mjpeg.decoder" 
#define OMX_COMPNAME_VIDEO_FFMPEG_DECODER  "OMX.anapass.ffmpeg_v.decoder"

/* Video Encoder */
#define OMX_COMPNAME_VIDEO_MPEG4_ENCODER  "OMX.anapass.mpeg4.encoder"
#define OMX_COMPNAME_VIDEO_H264_ENCODER  "OMX.anapass.h264.encoder"
#define OMX_COMPNAME_VIDEO_H263_ENCODER  "OMX.anapass.h263.encoder"

/* Mime Type */
#define OMX_MIMETYPE_AUDIO_MP3   "audio/mpeg"  		
#define OMX_MIMETYPE_AUDIO_MP2   "audio/mpeg-L2"  		
#define OMX_MIMETYPE_AUDIO_MP1   "audio/mpeg-L1"  		
//#define OMX_MIMETYPE_AUDIO_WMAV2 "audio/wmav2"  		
#define OMX_MIMETYPE_AUDIO_AC3   "audio/ac3"  		
#define OMX_MIMETYPE_AUDIO_AAC_MAIN  "audio/aac-main"  		
#define OMX_MIMETYPE_AUDIO_AAC  "audio/mp4a-latm"  		
#define OMX_MIMETYPE_AUDIO_FLAC  "audio/flac"  		
#define OMX_MIMETYPE_AUDIO_ADPCM_MS "audio/adpcm-ms"  		
#define OMX_MIMETYPE_AUDIO_PCM "audio/pcm"  		
#define OMX_MIMETYPE_AUDIO_AMR_NB "audio/3gpp"
#define OMX_MIMETYPE_AUDIO_AMR_WB "audio/amr-wb"
#define OMX_MIMETYPE_AUDIO_FFMPEG "audio/ffmpeg"  		

#define OMX_MIMETYPE_VIDEO_VPX   "video/x-vnd.on2.vp8"
#define OMX_MIMETYPE_VIDEO_AVC   "video/avc"
#define OMX_MIMETYPE_VIDEO_MPEG4 "video/mp4v-es"
#define OMX_MIMETYPE_VIDEO_MPEG2 "video/mpeg2"
//#define OMX_MIMETYPE_VIDEO_MSMPEG4V1 "video/msmpeg4v1"
//#define OMX_MIMETYPE_VIDEO_MSMPEG4V2 "video/msmpeg4v2"
//#define OMX_MIMETYPE_VIDEO_MSMPEG4V3 "video/msmpeg4v3"
#define OMX_MIMETYPE_VIDEO_H263  "video/3gpp"
#define OMX_MIMETYPE_VIDEO_RAW   "video/raw"
#define OMX_MIMETYPE_VIDEO_VC1   "video/vc1"
//#define OMX_MIMETYPE_VIDEO_WMV1   "video/wmv1"
//#define OMX_MIMETYPE_VIDEO_WMV2   "video/wmv2"
//#define OMX_MIMETYPE_VIDEO_WMV3   "video/wmv3"
//#define OMX_MIMETYPE_VIDEO_VP6    "video/vp6"
//#define OMX_MIMETYPE_VIDEO_VP6F    "video/vp6f"
//#define OMX_MIMETYPE_VIDEO_VP6A    "video/vp6a"
//#define OMX_MIMETYPE_VIDEO_RV30    "video/rv30"
//#define OMX_MIMETYPE_VIDEO_RV40    "video/rv40"
//#define OMX_MIMETYPE_VIDEO_SVQ1    "video/svq1" //Sorenson 1
//#define OMX_MIMETYPE_VIDEO_SVQ3    "video/svq3" //Sorenson 3
#define OMX_MIMETYPE_VIDEO_THEORA "video/theora"
#define OMX_MIMETYPE_VIDEO_MJPEG "video/mjpeg"
#define OMX_MIMETYPE_VIDEO_FFMPEG "video/ffmpeg"  		

#define OMX_MIMETYPE_IMAGE_JPEG   "image/jpeg"

struct mmp_component_desc
{
	MMP_U32 nCompID;
	MMP_STRING cComponentName;
	MMP_STRING cMineType;
	MMP_BOOL  bUse;
    MMP_U32 nOMXCodingType;
};

#ifdef __cplusplus
extern "C" {
#endif

struct mmp_component_desc* get_omx_desc(int index);

#ifdef __cplusplus
}
#endif

#endif