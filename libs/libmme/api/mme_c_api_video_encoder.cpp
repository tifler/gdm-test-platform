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

#include "MmpDefine.h"
#include "mme_c_api.h"
#include "MmpEncoderVideo.hpp"
#include <stdio.h>

static FILE *s_fp_yuv_dump = NULL;

void* mme_video_encoder_create_object(struct mme_video_encoder_config* p_video_encoder_config) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    CMmpEncoderVideo* pVideoEncoder = NULL;
    struct MmpEncoderCreateConfig EncoderCreateConfig;

    mmpResult = CMmpEncoder::CreateVideoDefaultConfig(p_video_encoder_config->format, //MMP_U32 mmp_video_fourcc,
                                                      p_video_encoder_config->width, p_video_encoder_config->height, //MMP_U32 pic_width, MMP_U32 pic_height,
                                                      p_video_encoder_config->fps,         // MMP_U32 framerate
                                                      p_video_encoder_config->idr_period,  // MMP_U32 idr_period
                                                      p_video_encoder_config->bitrate,     // MMP_U32 bitrate,
                                                      &EncoderCreateConfig);

    /*Create Video Encoder */
    if(mmpResult == MMP_SUCCESS) {
        pVideoEncoder = (CMmpEncoderVideo*)CMmpEncoder::CreateVideoObject(&EncoderCreateConfig, p_video_encoder_config->sw_codec_use);
    }

    s_fp_yuv_dump = fopen("/mnt/enc_dump.yuv", "wb");
    
    return (void*)pVideoEncoder;
}

int mme_video_encoder_destroy_object(void* hdl) {

    CMmpEncoderVideo* pVideoEncoder = (CMmpEncoderVideo*)hdl;

    if(pVideoEncoder != NULL) {
        CMmpEncoder::DestroyObject(pVideoEncoder);
    }

    if(s_fp_yuv_dump != NULL) {
        fclose(s_fp_yuv_dump);    
    }

    return 0;
}


int mme_video_encoder_run(void* hdl, int* shared_ion_fd, int* mem_offset, 
                          unsigned char* p_enc_stream, int stream_max_size, int* enc_stream_size, unsigned int* enc_flag,
                          unsigned char* p_config_data, int config_max_size, int *config_data_size
                          ) {
    int iret = 0;
    CMmpEncoderVideo* pVideoEncoder = (CMmpEncoderVideo*)hdl;
    class mmp_buffer_videoframe* p_buf_videoframe = NULL;
    class mmp_buffer_videostream* p_buf_videostream = NULL;
    int dsi_size, stream_size;
    MMP_U32 flag = 0;
    int pic_width, pic_height;
    int pic_size[3];

    if(enc_stream_size != NULL) *enc_stream_size = 0;
    if(config_data_size!=NULL) *config_data_size = 0;
    if(p_enc_stream == NULL) iret = -1;
    
    if((pVideoEncoder != NULL) && (iret==0) ) {

        pic_width = pVideoEncoder->GetVideoPicWidth();
        pic_height = pVideoEncoder->GetVideoPicHeight();
        pic_size[0] = pic_width*pic_height;
        pic_size[1] = pic_width*pic_height/4;
        pic_size[2] = pic_width*pic_height/4;

        /* attach video frame */
        p_buf_videoframe = mmp_buffer_mgr::get_instance()->attach_media_videoframe((MMP_S32*)shared_ion_fd, (MMP_S32*)mem_offset, 
                                                                                   pic_width, pic_height );
        if(p_buf_videoframe == NULL) {
            iret = -1;
        }
        
        /* attach video stream */
        p_buf_videostream = mmp_buffer_mgr::get_instance()->attach_media_videostream(p_enc_stream, stream_max_size);
        if(p_buf_videostream == NULL) {
            iret = -1;
        }

        if(iret == 0) {

            int ii;
            if(s_fp_yuv_dump != NULL) {
                for(ii = 0; ii < 3; ii++) {
                    fwrite((void*)p_buf_videoframe->get_buf_vir_addr(ii), 1, pic_size[ii], s_fp_yuv_dump);
                }
            }

            pVideoEncoder->EncodeAu(p_buf_videoframe, p_buf_videostream);

            dsi_size = p_buf_videostream->get_dsi_size();
            stream_size = p_buf_videostream->get_stream_real_size();
            flag = p_buf_videostream->get_flag();
            
            if(dsi_size > 0) {

                if(p_config_data == NULL) {
                    memcpy(&p_enc_stream[dsi_size], &p_enc_stream[0], stream_size);
                    memcpy(&p_enc_stream[0], p_buf_videostream->get_dsi_buffer(), dsi_size);
                    flag |= MMP_MEDIASAMPMLE_FLAG_CONFIGDATA;
                    stream_size += dsi_size;
                }
                else {
                    memcpy(p_config_data, p_buf_videostream->get_dsi_buffer(), dsi_size);
                    if(config_data_size!=NULL) *config_data_size = dsi_size;
                }
            }

            if(enc_stream_size!=NULL) { *enc_stream_size = stream_size; }
            if(enc_flag != NULL) { *enc_flag = flag; }

        }
    }
    else {
        iret = -1;
    }
    
    if(p_buf_videostream != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_videostream);
    }
    
    if(p_buf_videoframe != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_videoframe);
    }

    return iret;
}
