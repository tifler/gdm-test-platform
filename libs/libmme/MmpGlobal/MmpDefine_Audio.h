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

#ifndef MMPDEFINE_AUDIO_H__
#define MMPDEFINE_AUDIO_H__


#define MMP_WAVE_FORMAT_PCM                    1  
#define MMP_WAVE_FORMAT_ADPCM                  2  
#define MMP_WAVE_FORMAT_MPEGLAYER3             0x0055 // ISO/MPEG Layer3 Format Tag 
#define MMP_WAVE_FORMAT_MPEGLAYER2             0x0050 // ISO/MPEG Layer2 Format Tag 
#define MMP_WAVE_FORMAT_BSAC                   0xD000 // Korea TDMB Audio Bsac

#define MMP_WAVE_FORMAT_AAC                    0x00FF // China TDMB Audio AAXC
#define MMP_WAVE_FORMAT_EAAC                   0x00FE // China TDMB Audio AAXC
#define MMP_WAVE_FORMAT_AAC_LC                 0x706D //Format: AAC(Advanced Audio Codec) version: Version2  Profile: LC

#define MMP_WAVE_FORMAT_RA_COOK                0x6F63 //Real Audio  Cook 
#define MMP_WAVE_FORMAT_RA_RAAC                0x504d //Real Audio  Raac
#define MMP_WAVE_FORMAT_RA_SIPR                0x6973 //Real Audio  SIPR

#define MMP_WAVE_FORMAT_AC3                    0x2000   //AC3
#define MMP_WAVE_FORMAT_DTS                    0x2001  //DTS 
#define MMP_WAVE_FORMAT_WMA2                   0x0161  //WMA 2
#define MMP_WAVE_FORMAT_WMA3                   0x0162  //WMA 3

#define MMP_WAVE_MY_EXTEND                     0xFF00   //I don't knwo what the wave number is..
#define MMP_WAVE_FORMAT_FLAC                   (MMP_WAVE_MY_EXTEND+1)  //FLAC
#define MMP_WAVE_FORMAT_ADPCM_MS               (MMP_WAVE_MY_EXTEND+2) 
#define MMP_WAVE_FORMAT_FFMPEG                 (MMP_WAVE_MY_EXTEND+3) 
#define MMP_WAVE_FORMAT_MPEGLAYER1             (MMP_WAVE_MY_EXTEND+4)  // ISO/MPEG Layer1 Format Tag 


typedef enum MMP_AUDIO_AACSTREAMFORMATTYPE {
    MMP_AUDIO_AACStreamFormatMP2ADTS = 0, /**< AAC Audio Data Transport Stream 2 format */
    MMP_AUDIO_AACStreamFormatMP4ADTS,     /**< AAC Audio Data Transport Stream 4 format */
    MMP_AUDIO_AACStreamFormatMP4LOAS,     /**< AAC Low Overhead Audio Stream format */
    MMP_AUDIO_AACStreamFormatMP4LATM,     /**< AAC Low overhead Audio Transport Multiplex */
    MMP_AUDIO_AACStreamFormatADIF,        /**< AAC Audio Data Interchange Format */
    MMP_AUDIO_AACStreamFormatMP4FF,       /**< AAC inside MPEG-4/ISO File Format */
    MMP_AUDIO_AACStreamFormatRAW,         /**< AAC Raw Format */
    MMP_AUDIO_AACStreamFormatKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_AUDIO_AACStreamFormatVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_AUDIO_AACStreamFormatMax = 0x7FFFFFFF
} MMP_AUDIO_AACSTREAMFORMATTYPE;

typedef struct _MMPWAVEFORMATEX
{
    unsigned short        wFormatTag;         /* format type */
    unsigned short        nChannels;          /* number of channels (i.e. mono, stereo...) */
    MMP_U32       nSamplesPerSec;     /* sample rate */
    MMP_U32       nAvgBytesPerSec;    /* for buffer estimation */
    unsigned short        nBlockAlign;        /* block size of data */
    unsigned short        wBitsPerSample;     /* number of bits per sample of mono data */
    unsigned short        cbSize;             /* the count in bytes of the size of */
                                    /* extra information (after cbSize) */
}MMPWAVEFORMATEX;
#define MMPWAVEFORMATEX_CAL_nAvgBytesPerSec(wf) (wf.nSamplesPerSec*wf.nChannels*(wf.wBitsPerSample/8))

#endif
