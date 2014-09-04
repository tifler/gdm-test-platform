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

#ifndef _MMP_BUFFER_DEF_H__
#define _MMP_BUFFER_DEF_H__

#define CLASS_BUFFER_MGR mmp_buffer_mgr_ex1

/*
    MMP Buffer Type

    0xABCDEFGH
    
        GH - Buffer USE
        F  - Is DMA
*/
#define MMP_BUFFER_DMA_BIT 8

#define MMP_BUFFER_DMA    (1<<MMP_BUFFER_DMA_BIT)
#define MMP_BUFFER_NO_DMA (0<<MMP_BUFFER_DMA_BIT)

#define MMP_BUFFER_USE_UNKNOWN      (1<<0)
#define MMP_BUFFER_USE_VIDEO_STREAM (1<<1)
#define MMP_BUFFER_USE_VIDEO_FRAME  (1<<2)

#define MMP_BUFFER_TYPE_DMA               (MMP_BUFFER_DMA|MMP_BUFFER_USE_UNKNOWN)
#define MMP_BUFFER_TYPE_DMA_VIDEO_STREAM  (MMP_BUFFER_DMA|MMP_BUFFER_USE_VIDEO_STREAM)
#define MMP_BUFFER_TYPE_DMA_VIDEO_FRAME   (MMP_BUFFER_DMA|MMP_BUFFER_USE_VIDEO_FRAME)

#define MMP_BUFFER_IS_DMA(type) ((type&MMP_BUFFER_DMA)==MMP_BUFFER_DMA)
#define MMP_BUFFER_IS_VIDEO_FRAME(type) ((type&MMP_BUFFER_USE_VIDEO_FRAME)==MMP_BUFFER_USE_VIDEO_FRAME)

/*
    Video Frame Buffer 
*/
#define MMP_BUFFER_VIDEOFRAME_MAX_PLANE 3

#endif

