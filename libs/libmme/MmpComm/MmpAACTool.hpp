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

#ifndef _MMPAACTOOL_HPP__
#define _MMPAACTOOL_HPP__

#include "../MmpGlobal/MmpDefine.h"

typedef struct _ADTSHeader {
    /* fixed */
    unsigned char id;                             /* MPEG bit - should be 1 */
    unsigned char layer;                          /* MPEG layer - should be 0 */
    unsigned char protectBit;                     /* 0 = CRC word follows, 1 = no CRC word */
    unsigned char profile;                        /* 0 = main, 1 = LC, 2 = SSR, 3 = reserved */
    unsigned char sampRateIdx;                    /* sample rate index range = [0, 11] */
    unsigned char privateBit;                     /* ignore */
    unsigned char channelConfig;                  /* 0 = implicit, >0 = use default table */
    unsigned char origCopy;                       /* 0 = copy, 1 = original */
    unsigned char home;                           /* ignore */

    /* variable */
    unsigned char copyBit;                        /* 1 bit of the 72-bit copyright ID (transmitted as 1 bit per frame) */
    unsigned char copyStart;                      /* 1 = this bit starts the 72-bit ID, 0 = it does not */
    int           frameLength;                    /* length of frame */
    int           bufferFull;                     /* number of 32-bit words left in enc buffer, 0x7FF = VBR */
    unsigned char numRawDataBlocks;               /* number of raw data blocks in frame */

    /* CRC */
    int           crcCheckWord;                   /* 16-bit CRC check word (present if protectBit == 0) */
}SMmpADTSHeader;

#if 0
typedef struct _ADIFHeader {
    unsigned char copyBit;                        /* 0 = no copyright ID, 1 = 72-bit copyright ID follows immediately */
    unsigned char origCopy;                       /* 0 = copy, 1 = original */
    unsigned char home;                           /* ignore */
    unsigned char bsType;                         /* bitstream type: 0 = CBR, 1 = VBR */
    int           bitRate;                        /* bitRate: CBR = bits/sec, VBR = peak bits/frame, 0 = unknown */
    unsigned char numPCE;                         /* number of program config elements (max = 16) */
    int           bufferFull;                     /* bits left in bit reservoir */
    unsigned char copyID[ADIF_COPYID_SIZE];       /* optional 72-bit copyright ID */
}SMmpADIFHeader;
#endif

class CMmpAacTool
{
public:
    static MMP_RESULT UnpackADTSHeaderAndMakeCbData(unsigned char* adts_stream, int adts_stream_size, unsigned char* cb, int* cbsize);
    static MMP_RESULT UnpackADTSHeaderAndMakeCbData(unsigned char* adts_stream, int adts_stream_size, SMmpADTSHeader *fhADTS, unsigned char* cb, int* cbsize);
    static MMP_RESULT UnpackADTSHeader(unsigned char* adts_stream, int adts_stream_size, unsigned char* aac_stream, int* aac_stream_size);

    static int GetSamplingFreq(int index);
};

#endif