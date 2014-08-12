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

#ifndef MMPDEFINE_FFMPEG_H__
#define MMPDEFINE_FFMPEG_H__

#define MMP_FFMPEG_PACKET_HEADER_KEY 0xAAAACCCC

#define MMP_FFMPEG_PACKET_TYPE_AVCodecContext       0x10
#define MMP_FFMPEG_PACKET_TYPE_MMPBITMAPINFOHEADER  0x11

struct mmp_ffmpeg_packet_header {
    MMP_U32 key;
    MMP_U32 payload_type;
    MMP_U32 hdr_size;
    MMP_U32 payload_size;
    MMP_U32 extra_data_size;
    MMP_U32 packet_size;
};

#endif
