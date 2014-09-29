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

#ifndef V4L2_ION_TOOL_H__
#define V4L2_ION_TOOL_H__

#include "v4l2_api_def.h"

#ifdef __cplusplus
extern "C" {
#endif

int v4l2_ion_alloc_yuv_frame(int pic_width, int pic_height, unsigned int pix_fmt, struct v4l2_ion_frame* p_ion_frame);
void v4l2_ion_free_yuv_frame(struct v4l2_ion_frame* p_ion_frame);

int v4l2_ion_alloc_stream(int pic_width, int pic_height, struct v4l2_ion_buffer* p_ion_buf);
void v4l2_ion_free_stream(struct v4l2_ion_buffer* p_ion_buf);

#ifdef __cplusplus
}
#endif

#endif
