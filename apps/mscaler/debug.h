/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef  NDEBUG
#undef  NDEBUG
#endif  /*NDEBUG*/

#include <assert.h>
#define ASSERT                  assert

#ifdef  ANDROID

#include <log/log.h>

#define DIE(fmt, ...)       \
    do { ALOGE("[%s:%d] " fmt , __func__, __LINE__, ## __VA_ARGS__); assert(0); } while(0)

#define ERR(fmt, ...)       \
    do { ALOGE("[%s:%d] " fmt , __func__, __LINE__, ## __VA_ARGS__); } while(0)

#define INFO(fmt, ...)       \
    do { ALOGI("[%s:%d] " fmt , __func__, __LINE__, ## __VA_ARGS__); } while(0)

#define DBG(fmt, ...)       \
    do { ALOGD("[%s:%d] " fmt , __func__, __LINE__, ## __VA_ARGS__); } while(0)

#define TRACE_FUNC              \
    do { ALOGD("[TRACE_FUNC >>>>> %s:%d]", __func__, __LINE__); } while(0)

#define TRACE_LINE              \
    do { ALOGD("[TRACE_LINE >>>>> %s:%d]", __FILE__, __LINE__); } while(0)

#else

#include <stdio.h>

#define __dbg_output(fmt, ...)       \
    do { fprintf(stderr, \
            "[%s:%d] " fmt "\n", __func__, __LINE__, ## __VA_ARGS__); } while(0)

#define DIE(fmt, ...)       \
    do { __dbg_output(fmt, ## __VA_ARGS__); assert(0); } while(0)

#define ERR                     __dbg_output
#define INFO                    __dbg_output
#define DBG                     __dbg_output

#define TRACE_FUNC              \
    do { fprintf(stderr, \
            "[TRACE_FUNC >>>>> %s:%d]\n", __func__, __LINE__); } while(0)

#define TRACE_LINE              \
    do { fprintf(stderr, \
            "[TRACE_LINE >>>>> %s:%d]\n", __FILE__, __LINE__); } while(0)

#endif  /*ANDROID*/

#define __CHECK_LINE__          TRACE_LINE
#define __CHECK_FUNC__          TRACE_FUNC

#endif  /*__DEBUG_H__*/
