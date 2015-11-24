#ifndef __YUV_WRITER_H__
#define __YUV_WRITER_H__

#include <stdint.h>

/*****************************************************************************/

struct GDMBuffer;
struct GDMYUVWriter;
struct GDMImageInfo;

/*****************************************************************************/

#if 0
struct GDMPlaneInfo {
    uint32_t stride;
    uint32_t bpl;       // effective bytes per line
    uint32_t lines;
};

struct GDMImageInfo {
    uint32_t width;
    uint32_t height;
    uint32_t planeCount;
    struct GDMPlaneInfo plane[3];
};
#endif  /*0*/

/*****************************************************************************/

struct GDMYUVWriter *GYUVWriterOpen(const char *path);
void GYUVWriterClose(struct GDMYUVWriter *w);
void GYUVWriterWrite(struct GDMYUVWriter *w,
        const struct GDMImageInfo *info, struct GDMBuffer *buffer);

#endif  /*__YUV_WRITER_H__*/
