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

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "ion_api.h"
#include "mme_c_api.h"

#define PLANE_COUNT 3

struct mme_ion_frame {
    int shared_fd[PLANE_COUNT];
    int plane_size[PLANE_COUNT];
    int vir_addr[PLANE_COUNT];
    int mem_offset[PLANE_COUNT];
};


/* sig int */
static int s_enc_run = 1;
static void mme_sig_int(int sig) {
    
    s_enc_run = 0;
}


#ifdef WIN32
int main_mme_enc(int argc, char* argv[]) 
#else
int main(int argc, char* argv[]) 
#endif
{
    void* enc_hdl = NULL;
    void* muxer_hdl = NULL;

    int i;
    int iret = 0, ret;
    int ion_fd = -1;
    
    unsigned char* p_enc_stream = NULL;
    unsigned char* p_config_data = NULL;
    int enc_stream_size, enc_stream_max_size;
    int config_data_size, config_max_size;
    unsigned int enc_flag;


    char yuv_filename[128];
    char szbuffer[128];
    FILE *fp_yuv = NULL; 
    
    struct mme_video_encoder_config video_encoder_config;
    struct mme_muxer_config muxer_config;
    struct mme_ion_frame ion_frame;

    
    if(argc < 5) {
        printf("\n--------------------------------------------------------------------------------------\n\n");
        fprintf(stderr, "!!!!! ERROR: arg count must be 3  ex)mme_enc [width] [height] [YUV filename] [h264/mpeg4/h263] [swcodec] \n");
        printf("\n--------------------------------------------------------------------------------------\n");
        return -1;
    }
    
#ifndef WIN32
    signal(SIGINT, mme_sig_int);
#endif

    /* init param */
    for(i = 0; i < PLANE_COUNT; i++)  {
        ion_frame.shared_fd[i] = -1;
        ion_frame.vir_addr[i] = 0;
        ion_frame.mem_offset[i] = 0;
    }

    memset(&video_encoder_config, 0x00, sizeof(video_encoder_config));
    video_encoder_config.width = 1280;
    video_encoder_config.height = 720;
    video_encoder_config.format = MME_FOURCC_VIDEO_H264;
    video_encoder_config.fps = 24;
    video_encoder_config.bitrate = 4*1024*1024; /* 1M bps */
    video_encoder_config.idr_period = 10; /* I Frmae Period */
    video_encoder_config.sw_codec_use = 0; /* if 1, use FFMpeg Encoder */

    /* check width/height */
    video_encoder_config.width = atoi(argv[1]);
    video_encoder_config.height = atoi(argv[2]);
    if(  (video_encoder_config.width>=16) && (video_encoder_config.width<=1920) ) {
        /* Nothing to do */
    }
    else {
        iret = -1;
    }
    if(  (video_encoder_config.height>=16) && (video_encoder_config.height<=1088) ) {
        /* Nothing to do */
    }
    else {
        iret = -1;
    }
    
    ion_frame.plane_size[0] = video_encoder_config.width*video_encoder_config.height;
    ion_frame.plane_size[1] = ion_frame.plane_size[0]/4;
    ion_frame.plane_size[2] = ion_frame.plane_size[1];

    /* YUV File Open */
    if(iret == 0) {
        strcpy(yuv_filename, argv[3] );
        fp_yuv = fopen(yuv_filename, "rb");
        if(fp_yuv == NULL) {
            fprintf(stderr, "FAIL: fopen YUV File (%s) \n", yuv_filename);
            iret = -1;
        }
    }

    /* enc format */
    if(iret == 0) {
        if(strcmp(argv[4], "h264") == 0) {
            video_encoder_config.format = MME_FOURCC_VIDEO_H264;
        }
        else if(strcmp(argv[4], "mpeg4") == 0) {
            video_encoder_config.format = MME_FOURCC_VIDEO_MPEG4;
        }
        else if(strcmp(argv[4], "h263") == 0) {
            video_encoder_config.format = MME_FOURCC_VIDEO_H263;
        }
        else  {
            fprintf(stderr, "FAIL: set enc format (h264/mpeg4/h263) \n");
            iret = -1;
        }
    }

    if(argc > 5) {
        video_encoder_config.sw_codec_use = atoi(argv[5]);
        if( (video_encoder_config.sw_codec_use == 1) || (video_encoder_config.sw_codec_use == 0) ) {
            /* Nothing to do */
        }
        else {
            video_encoder_config.sw_codec_use = 0;
        }
    }
    
    /* alloc stream buffer */
    if(iret == 0) {
        enc_stream_max_size = video_encoder_config.width*video_encoder_config.height*3/2;
        p_enc_stream = (unsigned char*)malloc(enc_stream_max_size);
        if(p_enc_stream == NULL) {
            fprintf(stderr, "FAIL: malloc  encstream ");
            iret = -1;
        }
        config_max_size = 4096;
        p_config_data = (unsigned char*)malloc(config_max_size);
    }

    /* alloc ion */
    if(iret == 0) {
        ion_fd = ion_open();
        if(ion_fd >= 0) {
            for(i = 0; i < PLANE_COUNT; i++) {
                ret = ion_alloc_fd(ion_fd, ion_frame.plane_size[i], 0, ION_HEAP_CARVEOUT_MASK,  0, &ion_frame.shared_fd[i]);
                if(ret < 0) {
                    iret = -1;
                    break;
                }
                else {
                    ion_frame.vir_addr[i] = (unsigned int)MME_DRIVER_MMAP(NULL, ion_frame.plane_size[i], (PROT_READ | PROT_WRITE), MAP_SHARED, ion_frame.shared_fd[i], 0);
                }
            }
        }
    }

    /* create video encoder */
    if(iret == 0) {
        enc_hdl = mme_video_encoder_create_object(&video_encoder_config);
        if(enc_hdl == NULL) {
            iret = -1;
        }
    }

    /* create muxer */
    if(iret == 0) {
        
#ifdef WIN32
        strcpy(muxer_config.filename, "D:\\work\\");
#else
        strcpy(muxer_config.filename, "/tmp/");
#endif
        mme_util_split_file_name(yuv_filename, szbuffer); /* e.g  IN:/mnt/abcde.dat   OUT: abcde */
        strcat(muxer_config.filename, szbuffer);

        if(video_encoder_config.format == MME_FOURCC_VIDEO_H264) {
            strcat(muxer_config.filename, ".yuv.h264.mp4");
        }
        else if(video_encoder_config.format == MME_FOURCC_VIDEO_MPEG4) {
            strcat(muxer_config.filename, ".yuv.mpeg4.mp4");
        }
        else if(video_encoder_config.format == MME_FOURCC_VIDEO_H263) {
            strcat(muxer_config.filename, ".yuv.h263.mov");
        }
        else {
            strcat(muxer_config.filename, ".mp4");
        }
        

        memcpy(&muxer_config.video_encoder_config, &video_encoder_config, sizeof(video_encoder_config));
        muxer_hdl = mme_muxer_create_object(&muxer_config);
        if(muxer_hdl == NULL) {
            iret = -1;
        }
    }

    /* encoding */
    if(iret == 0) {
    
        unsigned int cur_tick, before_tick, start_tick;
        int fps_sum = 0;
        int enc_byte_sum = 0;
        int fps, bitrate;
        unsigned int t1, t2, enc_dur = 0L;
    
        start_tick = mme_util_get_tick_count();
        before_tick = start_tick;
        while(s_enc_run) {

            for(i = 0; i < PLANE_COUNT; i++) {
                if(fread((void*)ion_frame.vir_addr[i], 1, ion_frame.plane_size[i], fp_yuv) != ion_frame.plane_size[i]) {
                    s_enc_run = 0;
                    break;
                }

                ion_sync_fd(ion_fd, ion_frame.shared_fd[i] );
            }

            if(s_enc_run == 0) break;

            t1 = mme_util_get_tick_count();
            mme_video_encoder_run(enc_hdl, ion_frame.shared_fd, ion_frame.mem_offset, 
                                  p_enc_stream, enc_stream_max_size, &enc_stream_size, &enc_flag,
                                  p_config_data, config_max_size, &config_data_size
                                  );
            t2 = mme_util_get_tick_count();

            if(config_data_size > 0) {
                mme_muxer_add_video_config(muxer_hdl, p_config_data, config_data_size);
            }
            
            if(enc_stream_size > 0) {

                enc_dur += (t2-t1);
                
                mme_muxer_add_video_data(muxer_hdl, p_enc_stream, enc_stream_size, enc_flag);

                enc_byte_sum += enc_stream_size;
                fps_sum++;
            }

            cur_tick = mme_util_get_tick_count();
            if( (cur_tick - before_tick) > 1000) {

                if(enc_dur > 0) {
                    fps = fps_sum*1000/enc_dur;
                    bitrate = enc_byte_sum*8*1000/enc_dur;
                }
                else {
                    fps = fps_sum;
                    bitrate = enc_byte_sum*8;
                }
                
                printf("%d. avg_fps=%d fps_sum=%d enc_bitrate=%d \n", (cur_tick-start_tick)/1000, fps,  fps_sum, bitrate );

                enc_dur =  0;
                fps_sum = 0;
                enc_byte_sum = 0;
                before_tick = cur_tick;
            }

            mme_util_sleep(1);
        }
    }

    /* destroy video encoder */
    if(enc_hdl != NULL) {
        mme_video_encoder_destroy_object(enc_hdl);
    }

    /* destory muxer */
    if(muxer_hdl != NULL) {
        mme_muxer_destroy_object(muxer_hdl);
    }


    /* YUV File Close*/
    if(fp_yuv != NULL) {
        fclose(fp_yuv);
        fp_yuv = NULL;
    }

    /* free ion */
    for(i = 0; i < PLANE_COUNT; i++) {
        if(ion_frame.shared_fd[i] >= 0) {

            if(ion_frame.vir_addr[i]!=0) {
                MME_DRIVER_MUNMAP((void*)ion_frame.vir_addr[i], ion_frame.plane_size[i]);
            }

            ion_close(ion_frame.shared_fd[i]);
        }
    }

    /* free stream buffer */
    if(p_enc_stream != NULL) {
        free(p_enc_stream);
    }
    if(p_config_data != NULL) {
        free(p_config_data);
    }

    if(ion_fd >= 0) {
        ion_close(ion_fd);
    }

    return 0;
}
