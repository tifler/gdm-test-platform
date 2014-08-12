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

#include "MmpAACTool.hpp"
#include "MmpBitExtractor.hpp"
#include "../MmpComm/MmpUtil.hpp"

MMP_RESULT CMmpAacTool::UnpackADTSHeader(unsigned char* adts_stream, int adts_stream_size, unsigned char* aac_stream, int* aac_stream_size)
{
    SMmpADTSHeader adtsHeader;
    SMmpADTSHeader *fhADTS=&adtsHeader;
    CMmpBitExtractor be;
    unsigned long dummy, code;
    int offset;

    be.Start(adts_stream, adts_stream_size);

    /* verify that first 12 bits of header are syncword */
    code=be.Pop_BitCode(dummy, 12);
    if(code!=0x0fff)
    {
        //Not ADTS Format
        return MMP_FAILURE;
    }

    /* fixed fields - should not change from frame to frame */ 
    fhADTS->id =               (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->layer =            (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->protectBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->profile =          (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->sampRateIdx =      (unsigned char)be.Pop_BitCode(dummy, 4);
	fhADTS->privateBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->channelConfig =    (unsigned char)be.Pop_BitCode(dummy, 3);
	fhADTS->origCopy =         (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->home =             (unsigned char)be.Pop_BitCode(dummy, 1);

    /* variable fields - can change from frame to frame */ 
	fhADTS->copyBit =          (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->copyStart =        (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->frameLength =      (int)be.Pop_BitCode(dummy, 13);
	fhADTS->bufferFull =       (int)be.Pop_BitCode(dummy, 11);
	fhADTS->numRawDataBlocks = (unsigned char)be.Pop_BitCode(dummy, 2) + 1;

    /* note - MPEG4 spec, correction 1 changes how CRC is handled when protectBit == 0 and numRawDataBlocks > 1 */
	if (fhADTS->protectBit == 0)
		fhADTS->crcCheckWord = (int)be.Pop_BitCode(dummy, 16);

    offset=be.GetCurByteIndex();
    *aac_stream_size=adts_stream_size-offset;
    memcpy(aac_stream, &adts_stream[offset], *aac_stream_size);

  //  MMPDEBUGMSG(1, (TEXT("SampleRate: %d \n\r"), fhADTS->sampRateIdx));
    return MMP_SUCCESS;
}

MMP_RESULT CMmpAacTool::UnpackADTSHeaderAndMakeCbData(unsigned char* adts_stream, int adts_stream_size, unsigned char* cb, int* cbsize)
{
    SMmpADTSHeader adtsHeader;
    return CMmpAacTool::UnpackADTSHeaderAndMakeCbData(adts_stream, adts_stream_size, &adtsHeader, cb, cbsize);
}

MMP_RESULT CMmpAacTool::UnpackADTSHeaderAndMakeCbData(unsigned char* adts_stream, int adts_stream_size, SMmpADTSHeader *fhADTS, unsigned char* cb, int* cbsize)
{
    //SMmpADTSHeader adtsHeader;
    //SMmpADTSHeader *fhADTS=&adtsHeader;
    CMmpBitExtractor be;
    unsigned long dummy, code;
    
    be.Start(adts_stream, adts_stream_size);

    /* verify that first 12 bits of header are syncword */
    code=be.Pop_BitCode(dummy, 12);
    if(code!=0x0fff)
    {
        //Not ADTS Format
        return MMP_FAILURE;
    }

    /* fixed fields - should not change from frame to frame */ 
    fhADTS->id =               (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->layer =            (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->protectBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->profile =          (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->sampRateIdx =      (unsigned char)be.Pop_BitCode(dummy, 4);
	fhADTS->privateBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->channelConfig =    (unsigned char)be.Pop_BitCode(dummy, 3);
	fhADTS->origCopy =         (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->home =             (unsigned char)be.Pop_BitCode(dummy, 1);

    /* variable fields - can change from frame to frame */ 
	fhADTS->copyBit =          (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->copyStart =        (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->frameLength =      (int)be.Pop_BitCode(dummy, 13);
	fhADTS->bufferFull =       (int)be.Pop_BitCode(dummy, 11);
	fhADTS->numRawDataBlocks = (unsigned char)be.Pop_BitCode(dummy, 2) + 1;

    /* note - MPEG4 spec, correction 1 changes how CRC is handled when protectBit == 0 and numRawDataBlocks > 1 */
	if (fhADTS->protectBit == 0)
		fhADTS->crcCheckWord = (int)be.Pop_BitCode(dummy, 16);

    unsigned char object_type, ch, samplerate_index;

    *cbsize=2;

    switch(fhADTS->profile)
    {
        case 0: object_type=1; break; //Main Profile
        case 1: object_type=2; break; //LC (Low Complexity) Profile
        case 2: object_type=3; break; //SSR (Scalable Sample Rate ) Profile
        default:
                //object_type=4  LTP
                //object_type=5  SBR
                object_type=fhADTS->profile+1;
    }

    ch=fhADTS->channelConfig;
    samplerate_index=fhADTS->sampRateIdx;

    cb[0]=((object_type<<3)&0xF8)|((samplerate_index>>1)&0x07);
    cb[1]=((ch<<3)&0x78)|((samplerate_index<<7)&0x80);
    
    return MMP_SUCCESS;
}

int CMmpAacTool::GetSamplingFreq(int index)
{
    int SamplingFrequencyIndex[16] = {96000, 88200, 64000, 48000, 44100, 32000, 24000,
                                     22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0 };
   
    if(index<0 || index>=16) return 0;

    return SamplingFrequencyIndex[index];
}