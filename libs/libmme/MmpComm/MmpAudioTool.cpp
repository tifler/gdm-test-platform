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

#include "MmpAudioTool.hpp"
#include "MmpBitExtractor.hpp"

const unsigned short MMP_AC3_FREQS[3] = { 48000, 44100, 32000 };
/* possible bitrates */
const unsigned short MMP_AC3_BITRATETAB[19] = {
    32, 40, 48, 56, 64, 80, 96, 112, 128,
    160, 192, 224, 256, 320, 384, 448, 512, 576, 640
};
const unsigned char MMP_AC3_CHANNELAS[8] = {
    2, 1, 2, 3, 3, 4, 4, 5
};

/**
 * Possible frame sizes.
 * from ATSC A/52 Table 5.18 Frame Size Code Table.
 */
const unsigned short MMP_AC3_FRAME_SIZES[38][3] = {
    { 64,   69,   96   },
    { 64,   70,   96   },
    { 80,   87,   120  },
    { 80,   88,   120  },
    { 96,   104,  144  },
    { 96,   105,  144  },
    { 112,  121,  168  },
    { 112,  122,  168  },
    { 128,  139,  192  },
    { 128,  140,  192  },
    { 160,  174,  240  },
    { 160,  175,  240  },
    { 192,  208,  288  },
    { 192,  209,  288  },
    { 224,  243,  336  },
    { 224,  244,  336  },
    { 256,  278,  384  },
    { 256,  279,  384  },
    { 320,  348,  480  },
    { 320,  349,  480  },
    { 384,  417,  576  },
    { 384,  418,  576  },
    { 448,  487,  672  },
    { 448,  488,  672  },
    { 512,  557,  768  },
    { 512,  558,  768  },
    { 640,  696,  960  },
    { 640,  697,  960  },
    { 768,  835,  1152 },
    { 768,  836,  1152 },
    { 896,  975,  1344 },
    { 896,  976,  1344 },
    { 1024, 1114, 1536 },
    { 1024, 1115, 1536 },
    { 1152, 1253, 1728 },
    { 1152, 1254, 1728 },
    { 1280, 1393, 1920 },
    { 1280, 1394, 1920 },
};


MMP_RESULT CMmpAudioTool::Ac3ParseHeader(unsigned char* buf, int bufSize, MmpAC3HeaderInfo *hdr)
{
    CMmpBitExtractor be;
    be.Start(buf, bufSize);
    unsigned long dummy;

    memset(hdr, 0, sizeof(MmpAC3HeaderInfo));

    
    hdr->sync_word = (unsigned short)be.Pop_BitCode(dummy, 16);
    if(hdr->sync_word != 0x0B77)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_SYNC;
    }

    /* read ahead to bsid to make sure this is AC-3, not E-AC-3 */
    hdr->bsid = (unsigned char)be.Query_BitCode(dummy, 29)&0x1F; //show_bits_long(&gbc, 29) & 0x1F;
    if(hdr->bsid > 10)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_BSID;
    }

    hdr->crc1 = (unsigned short)be.Pop_BitCode(dummy, 16);//get_bits(&gbc, 16);
    hdr->fscod = (unsigned char)be.Pop_BitCode(dummy, 2); //get_bits(&gbc, 2);
    if(hdr->fscod == 3)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_SAMPLE_RATE;
    }

    hdr->frmsizecod = (unsigned char)be.Pop_BitCode(dummy, 6);//get_bits(&gbc, 6);
    if(hdr->frmsizecod > 37)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_FRAME_SIZE;
    }

    dummy=be.Pop_BitCode(dummy, 5);//skip_bits(&gbc, 5); // skip bsid, already got it

    hdr->bsmod = (unsigned char)be.Pop_BitCode(dummy, 3);//get_bits(&gbc, 3);
    hdr->acmod = (unsigned char)be.Pop_BitCode(dummy, 3);//get_bits(&gbc, 3);
    if((hdr->acmod & 1) && hdr->acmod != MMP_AC3_ACMOD_MONO) 
    {
        hdr->cmixlev = (unsigned char)be.Pop_BitCode(dummy, 2);///get_bits(&gbc, 2);
    }
    if(hdr->acmod & 4) 
    {
        hdr->surmixlev = (unsigned char)be.Pop_BitCode(dummy, 2);//get_bits(&gbc, 2);
    }
    if(hdr->acmod == MMP_AC3_ACMOD_STEREO) 
    {
        hdr->dsurmod = (unsigned char)be.Pop_BitCode(dummy, 3);//get_bits(&gbc, 2);
    }
    hdr->lfeon = (unsigned char)be.Pop_BitCode(dummy, 1);//get_bits1(&gbc);

    hdr->halfratecod = MMP_MAX(hdr->bsid, 8) - 8;
    hdr->sample_rate = MMP_AC3_FREQS[hdr->fscod] >> hdr->halfratecod;
    hdr->bit_rate = (MMP_AC3_BITRATETAB[hdr->frmsizecod>>1] * 1000) >> hdr->halfratecod;
    hdr->channels = MMP_AC3_CHANNELAS[hdr->acmod] + hdr->lfeon;
    hdr->frame_size = MMP_AC3_FRAME_SIZES[hdr->frmsizecod][hdr->fscod] * 2;

    return MMP_SUCCESS;
}

