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

#include "v4l2_jpeg_api.h"
#include <fcntl.h>
#include <sys/mman.h>

int v4l2_jpeg_open(const char* nodename) {

    int fd = -1;

    fd = V4L2_DRIVER_OPEN(nodename, O_RDWR);
    if(fd < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: open  %s", __func__, nodename));
    }
    
    return fd;
}

int v4l2_jpeg_querycap(int fd)
{
    struct v4l2_capability cap;
    int ret = 0;

    ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_QUERYCAP, &cap);

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        JPUDEBUGMSG(JPUZONE_INFO, ("[%s]: does not support streaming \n\r", __func__));
        ret = -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
        JPUDEBUGMSG(JPUZONE_INFO, ("[%s]: does not support output \n\r", __func__));
        ret = -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        JPUDEBUGMSG(JPUZONE_INFO, ("[%s]: does not support capture \n\r", __func__));
        ret = -1;
    }

    JPUDEBUGMSG(JPUZONE_INFO, ("[%s]: cap.capabilities = 0x%08x  \n\r", __func__, cap.capabilities));

    return ret;
}


int v4l2_jpeg_stream_on(int fd, enum v4l2_buf_type type)
{
    int ret = 0;

    ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_stream_on \n\r", __func__ ));
        return -1;
    }

    return ret;
}

int v4l2_jpeg_stream_off(int fd, enum v4l2_buf_type type)
{
    int ret = 0;

    ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_STREAMOFF, &type);
    if (ret < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: v4l2_jpeg_stream_off \n\r", __func__ ));
        return -1;
    }

    return ret;
}

int v4l2_jpeg_get_ctrl(int fd, unsigned int id) {
    struct v4l2_control ctrl;
    int ret = 0;

    ctrl.id = id;

    ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_G_CTRL, &ctrl);
    if (ret < 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: ioctl - ioctl : cid(%d) \n\r", __func__ , ctrl.id));
        return -1;
    }

    return ctrl.value;
}

int v4l2_jpeg_set_ctrl(int fd, unsigned int cid, int value) {

    struct v4l2_control vc;
    int ret = 0;

    vc.id = cid;
    vc.value = value;

    ret = V4L2_DRIVER_IOCTL(fd, VIDIOC_S_CTRL, &vc);
    if (ret != 0) {
        JPUDEBUGMSG(JPUZONE_ERROR, ("[%s] FAIL: ioctl - cid(%d), value(%d) \n\r", __func__ , cid, value ));
        return -1;
    }

    return ret;
}

