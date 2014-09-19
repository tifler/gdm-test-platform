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

#ifndef _MMPDECODERMFC_HPP__
#define _MMPDECODERMFC_HPP__

#include "MmpOAL.hpp"
#include "MmpDecoder.hpp"

#if 0//(MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC)

#include "SsbSipMfcApi.h"
#include "csc.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libavresample/audio_convert.h"
}

#include "sec_format.h"


#define MMPDECODER_MFC_STREAMBUF_COUNT 2

class CMmpDecoderMfc
{
protected:
    void* m_hMFCHandle;
    SSBSIP_MFC_CODEC_TYPE m_MFCCodecType;

    MMP_U32 m_nDecStreamBufferIndex;
    MMP_U32 m_nNextStreamBufferIndex;
    MMP_PTR  m_pStreamBuffer[MMPDECODER_MFC_STREAMBUF_COUNT];
    MMP_PTR  m_pStreamPhyBuffer[MMPDECODER_MFC_STREAMBUF_COUNT];
    MMP_U32  m_nStreamBufferSize[MMPDECODER_MFC_STREAMBUF_COUNT];
    
    
    SSBSIP_MFC_IMG_RESOLUTION m_MFCImgResol;

    MMP_BOOL m_bDecodeDSI;

    CSC_METHOD m_csc_method;
    void * m_csc_handle;
    MMP_BOOL m_csc_set_format;

private:
    MMP_U32 m_nDecFrameCount;
    MMP_U32 m_nDecFrameCount_Before;
    MMP_BOOL m_bDecServiceRun;
    MMPOALTASK_HANDLE m_mmp_task_hdl;
    MMPOALSEMAPHORE_HANDLE m_sema_dec_start;
    MMPOALSEMAPHORE_HANDLE m_sema_dec_end;

    MMP_U8* m_extra_data;
    MMP_U32 m_extra_data_size;
    
protected:
    CMmpDecoderMfc(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderMfc();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static void Service_DecodeNonBlockStub(void* parm);
    void Service_DecodeNonBlock();

    //void PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext);

    MMP_BOOL Make_WMV_Stream_MetaData(MMP_U8 *pInputStream, MMP_U32 *pStreamSize, SSBSIP_MFC_CODEC_TYPE mfcCodeingType);
    MMP_RESULT Make_WMV_Stream_MetaData(MMP_IN SSBSIP_MFC_CODEC_TYPE mfcCodeingType, 
                                        MMP_IN MMP_U32 width, MMP_IN MMP_U32 height, 
                                        MMP_IN MMP_U8* seq_header, MMP_IN MMP_U32 seq_hdr_size,
                                        MMP_OUT MMP_U8 *pMetaData, MMP_OUT MMP_U32 *pMetaDataSize );

    MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize, MMP_U8* szCodecName);
    MMP_RESULT DecodeAu_Block(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
    MMP_RESULT DecodeAu_NonBlock(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);

    
};

#endif /* #if (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC) */
#endif

