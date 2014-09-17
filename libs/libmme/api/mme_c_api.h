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

#ifndef MME_C_API_H__
#define MME_C_API_H__

#define MMEMAKEFOURCC(ch0, ch1, ch2, ch3)                           \
                ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) |   \
                ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))
#define MME_FOURCC_VIDEO_H263     MMEMAKEFOURCC('H','2','6','3')
#define MME_FOURCC_VIDEO_H264    MMEMAKEFOURCC('H','2','6','4')
#define MME_FOURCC_VIDEO_MPEG4    MMEMAKEFOURCC('M','P','4','V')

#ifdef WIN32
#define MME_DRIVER_OPEN  kernel_driver_open
#define MME_DRIVER_CLOSE kernel_driver_close
#define MME_DRIVER_WRITE kernel_driver_write
#define MME_DRIVER_IOCTL kernel_driver_ioctl
#define MME_DRIVER_MMAP  kernel_driver_mmap
#define MME_DRIVER_MUNMAP  kernel_driver_munmap

#else
#define MME_DRIVER_OPEN    open
#define MME_DRIVER_CLOSE   close
#define MME_DRIVER_WRITE   write
#define MME_DRIVER_MMAP    mmap
#define MME_DRIVER_MUNMAP  munmap
#define MME_DRIVER_IOCTL   ioctl

#endif


struct mme_video_encoder_config {
    int width;
    int height;
    unsigned int format;
    int bitrate;
    int fps;
    int idr_period; 
    int sw_codec_use; /* if 1, use FFMpeg Encoder */
};

struct mme_muxer_config {
    char filename[256];
    struct mme_video_encoder_config video_encoder_config;
};


#ifdef __cplusplus
extern "C" {
#endif

/* util */
int mme_util_get_vpu_fd(void);
unsigned char* mme_util_get_vpu_instance_pool_buffer(void);
unsigned int mme_util_get_vpu_reg_vir_addr(void);
void* mme_util_get_vpu_common_buffer(void);

void mme_util_sleep(int milesec);
unsigned int mme_util_get_tick_count();
long long mme_util_get_tick_count_us();

void mme_util_split_file_name(char* file_path_name, char* filename); /* e.g  IN:/mnt/abcde.dat OUT: abcde */

/* Muxer */
void* mme_muxer_create_object(struct mme_muxer_config* p_muxer_config);
int mme_muxer_destroy_object(void* hdl);
int mme_muxer_add_video_config(void* hdl, unsigned char* p_config_data, int config_data_size);
int mme_muxer_add_video_data(void* hdl, unsigned char* p_video_stream, int video_stream_size, unsigned int flag);

/* encoder */
void* mme_video_encoder_create_object(struct mme_video_encoder_config* p_video_encoder_config);
int mme_video_encoder_destroy_object(void* hdl);
int mme_video_encoder_run(void* hdl, 
                          int* shared_ion_fd, int* mem_offset, 
                          unsigned char* p_enc_stream, int stream_max_size, int* enc_stream_size, unsigned int* flag,
                          unsigned char* p_config_data, int config_max_size, int *config_data_size
                          );

#ifdef __cplusplus
}
#endif

#endif