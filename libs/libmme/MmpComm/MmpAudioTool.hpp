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

#ifndef _MMPAUDIOTOOL_HPP__
#define _MMPAUDIOTOOL_HPP__

#include "../MmpGlobal/MmpDefine.h"


typedef enum {
    MMP_AC3_ACMOD_DUALMONO = 0,
    MMP_AC3_ACMOD_MONO,
    MMP_AC3_ACMOD_STEREO,
    MMP_AC3_ACMOD_3F,
    MMP_AC3_ACMOD_2F1R,
    MMP_AC3_ACMOD_3F1R,
    MMP_AC3_ACMOD_2F2R,
    MMP_AC3_ACMOD_3F2R
}MmpAC3ChannelMode;
/**
 * @struct AC3HeaderInfo
 * Coded AC-3 header values up to the lfeon element, plus derived values.
 */
typedef struct {
    /** @defgroup coded Coded elements
     * @{
     */
    unsigned short sync_word;
    unsigned short crc1;
    unsigned char fscod;
    unsigned char frmsizecod;
    unsigned char bsid;
    unsigned char bsmod;
    unsigned char acmod;
    unsigned char cmixlev;
    unsigned char surmixlev;
    unsigned char dsurmod;
    unsigned char lfeon;
    /** @} */

    /** @defgroup derived Derived values
     * @{
     */
    unsigned char halfratecod;
    unsigned short sample_rate;
    unsigned int bit_rate;
    unsigned char channels;
    unsigned short frame_size;
    /** @} */
}MmpAC3HeaderInfo;


class CMmpAudioTool
{
public:
    static MMP_RESULT Ac3ParseHeader(unsigned char* buf, int bufSize, MmpAC3HeaderInfo *hdr);
};

#endif

