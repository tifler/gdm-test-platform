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

#ifndef MMPDECODERVPUIF_HPP__
#define MMPDECODERVPUIF_HPP__

#include "MmpDecoder.hpp"
#include "TemplateList.hpp"
#include "mmp_buffer_mgr.hpp"
#include "mmp_vpu_if.hpp"

class CMmpDecoderVpuIF 
{
private:
    enum {
        MAX_FRAMEBUFFER_COUNT = 16
    };

protected:
    struct MmpDecoderCreateConfig m_create_config;

    class mmp_vpu_if* m_p_vpu_if;
    MMP_S32 m_vpu_instance_index;

    Uint32 m_codec_idx;
    Uint32 m_version;
    Uint32 m_revision;
    Uint32 m_productId;

    int m_mapType;
    
    //MMP_U8* m_p_dsi_stream;
    //MMP_S32 m_dsi_stream_size;
    
    int m_regFrameBufCount;
    int m_framebufSize;
    int	m_reUseChunk;
    DecOpenParam m_decOP;
    DecHandle	m_DecHandle;
    //vpu_buffer_t m_vbStream;
    DecInitialInfo m_dec_init_info;
    DecOutputInfo m_output_info;
	vpu_buffer_t m_vbFrame[MAX_FRAMEBUFFER_COUNT];
    
    class mmp_buffer* m_p_stream_buffer;
    vpu_buffer_t m_vpu_stream_buffer;

    class mmp_buffer_videoframe* m_p_buf_videoframe[MAX_FRAMEBUFFER_COUNT];

    MMP_S32 m_last_int_reason;
    MMP_S32 m_input_stream_count;
        
protected:
    CMmpDecoderVpuIF(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderVpuIF();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();
    
private:
    void make_decOP_Common();
    void make_decOP_H263();
    void make_decOP_H264();
    void make_decOP_MPEG4(int mp4class);
    void make_decOP_MPEG2();
    void make_decOP_VC1();
    void make_decOP_MSMpeg4V3();
    void make_decOP_RV30();
    void make_decOP_RV40();
    void make_decOP_VP80();
    void make_decOP_Theora();

    MMP_RESULT make_seqheader_Common(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_seqheader_H264(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_seqheader_VC1(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_seqheader_DIV3(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_seqheader_RV(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_seqheader_VP8(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_seqheader_Theora(class mmp_buffer_videostream* p_buf_videostream);

    MMP_RESULT make_frameheader_Common(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_frameheader_H264(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_frameheader_VC1(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_frameheader_DIV3(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_frameheader_RV(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_frameheader_VP8(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT make_frameheader_Theora(class mmp_buffer_videostream* p_buf_videostream);

protected:
    virtual MMP_RESULT DecodeDSI(class mmp_buffer_videostream* p_buf_videostream);
    MMP_RESULT DecodeAu_PinEnd(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe);
    
};

#endif

