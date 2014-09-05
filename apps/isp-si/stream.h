#ifndef __GISP_STREAM_H__
#define __GISP_STREAM_H__

#include <stdint.h>

/*****************************************************************************/

enum {
    GISP_PORT_CAP,
    GISP_PORT_VID,
    GISP_PORT_DIS,
    GISP_PORT_FD,
    GISP_PORT_COUNT,
};


/*****************************************************************************/

struct STREAM {
    int fd;
    int port;
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    int bufferCount;
    struct GDMBuffer **buffers;
    struct v4l2_format fmt;
}:

#endif  /*__GISP_STREAM_H__*/
