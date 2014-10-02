#ifndef __IMAGE_INFO_H__
#define __IMAGE_INFO_H__

#include <stdint.h>

/*****************************************************************************/

struct GDMImagePlaneInfo {
    uint32_t offset;        // memory offset
    uint32_t stride;        // bytes per line
    uint32_t bpl;           // effective bytes per line(exclude align pad)
    uint32_t lines;
};

struct GDMImageInfo {
    /* input parameter */
    uint32_t width;
    uint32_t height;
    uint32_t pixelFormat;
    uint32_t align;
    /* output parameter */
    uint32_t planeCount;
    struct GDMImagePlaneInfo plane[3];
};

/*****************************************************************************/

void GDMGetImageInfo(struct GDMImageInfo *info);

#endif  /*__IMAGE_INFO_H__*/
