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

#ifndef MAIN_MMEPLAYER_H__
#define MAIN_MMEPLAYER_H__

#include "MmpDefine.h"
#include "MmpDemuxer.hpp"
#include "MmpMuxer.hpp"
#include "MmpDecoderVideo.hpp"
#include "MmpEncoderVideo.hpp"
#include "MmpRenderer.hpp"
#include "MmpUtil.hpp"
#include "MmpOAL.hpp"
#include "MmpPlayer.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void* mme_player_video_start(char* filename, void* hwnd, void* hdc);
int mme_player_video_stop(void* hdl);



#ifdef __cplusplus
}
#endif

#endif
