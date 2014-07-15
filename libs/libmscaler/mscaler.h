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

/*
 * 13MP Scaler
 */

#ifndef __MSCALER_H__
#define __MSCALER_H__

// exynos5 의 헤더를 포함하기 위함.
#include <linux/videodev2.h>

/*****************************************************************************/

#define MSCALER_MAX_WIDTH               (4208)
#define MSCALER_MAX_HEIGHT              (4120)
#define MSCALER_MIN_WIDTH               (64)
#define MSCALER_MIN_HEIGHT              (64)

// 1 << 4 = 16
#define MSCALER_HALIGN_ORDER            (4)
#define MSCALER_VALIGN_ORDER            (1)

#define VIDIOC_MSCALER_LOCK             (BASE_VIDIOC_PRIVATE + 1)
#define VIDIOC_MSCALER_TRYLOCK          (BASE_VIDIOC_PRIVATE + 2)
#define VIDIOC_MSCALER_UNLOCK           (BASE_VIDIOC_PRIVATE + 3)

/*****************************************************************************/

enum {
    PLANE_Y,
    PLANE_U,
    PLANE_V,
    MAX_PLANE_COUNT
};

enum {
    HW_FMT_422_1P,
    HW_FMT_422_2P,
    HW_FMT_422_3P,
    HW_FMT_420_2P,
    HW_FMT_420_3P,
    HW_FMT_ARGB,
    HW_FMT_RGB565,
    HW_FMT_COUNT
};

/*****************************************************************************/

struct MScalerSize {
    unsigned int width;
    unsigned int height;
};

struct MScalerRect {
    int left;
    int top;
    int width;
    int height;
};

/*
 * 1. Source Processing
 *  crop -> post processing(flip & rotate)
 *
 * 2. Scaling
 *
 * 3. Destination Processing
 *  crop -> post processing(flip & rotate)
 */

struct MScalerImageFormat {
    unsigned int width;
    unsigned int height;
    unsigned int pixelformat;
    struct MScalerRect crop;
};

struct MScalerPlane {
    unsigned int length;    /* size in bytes */
    int fd; /* ion shared fd(dmabuf fd) */
    void *base; /* mapped virtual addr */
};

struct MScalerPlaneInfo {
    int comps;
    int bpl[3];
    int stride[3];
    int width[3];
    int height[3];
};

struct MScalerImageData {
    int planes;
    struct MScalerPlane plane[MAX_PLANE_COUNT];
    int acquireFenceFd;
    int releaseFenceFd;
    void *priv;
};

struct MScalerBufferInfo {
    int planes;
    unsigned int planeSizes[MAX_PLANE_COUNT];
};

struct MScalerPixelFormat {
    const char *name;
    const char *description;
    unsigned int pixelformat;
    unsigned int planes;
    unsigned int components;
    unsigned int bitperpixel[3];
    int hw_format;  /* HW_FMT_* */
};

struct MScaler;

typedef void * MScalerHandle;

/*****************************************************************************/

MScalerHandle *MScalerOpen(const char *node);
int MScalerSetImageFormat(MScalerHandle handle,
        const struct MScalerImageFormat *srcFormat,
        const struct MScalerImageFormat *dstFormat);
int MScalerSetImageData(MScalerHandle handle,
        const struct MScalerImageData *srcData,
        const struct MScalerImageData *dstData);
int MScalerRun(MScalerHandle handle);
int MScalerWaitDone(MScalerHandle handle, int timeout);
int MScalerCancel(MScalerHandle handle);
int MScalerRunSync(MScalerHandle handle, int timeout);
void MScalerClose(MScalerHandle handle);
int MScalerTryLock(MScalerHandle handle);
int MScalerLock(MScalerHandle handle, int timeout);
int MScalerUnlock(MScalerHandle handle);
int MScalerGetBufferInfo(MScalerHandle handle,
        struct MScalerBufferInfo *buffer, int isDst);
int MScalerGetPixelFormatCount(void);
struct MScalerPixelFormat *MScalerGetPixelFormatByIndex(int index);
struct MScalerPixelFormat *MScalerGetPixelFormatByName(const char *name);
struct MScalerPixelFormat *MScalerGetPixelFormat(unsigned int pixelformat);
void MScalerGetPlaneInfo(const struct MScalerImageFormat *img,
        int plane, struct MScalerPlaneInfo *pi);

#endif  /*__MSCALER_H__*/
