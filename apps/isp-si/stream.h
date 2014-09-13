#ifndef __GISP_STREAM_H__
#define __GISP_STREAM_H__

#include <stdint.h>

/*****************************************************************************/

enum {
    // DO NOT CHANGE ORDER.(see gisp-port.h in driver)
    STREAM_PORT_CAPTURE,
    STREAM_PORT_VIDEO,
    STREAM_PORT_DISPLAY,
    STREAM_PORT_FACEDETECT,
    STREAM_PORT_COUNT,
};

/*****************************************************************************/

struct STREAM;
struct GDMBuffer;

/*****************************************************************************/

struct STREAM *streamOpen(int port);
void streamClose(struct STREAM *stream);
int streamSetFormat(struct STREAM *stream,
        uint32_t width, uint32_t height, uint32_t pixelformat);
int streamGetBufferSize(struct STREAM *stream, uint32_t planeSizes[3]);
int streamSetBuffers(struct STREAM *stream,
        uint32_t bufferCount, struct GDMBuffer **buffers);
int streamSetCallback(struct STREAM *stream,
        int (*callback)(void *param, struct GDMBuffer *buffer, int index),
        void *callbackParam);
int streamStart(struct STREAM *stream);
void streamStop(struct STREAM *stream);

#endif  /*__GISP_STREAM_H__*/
