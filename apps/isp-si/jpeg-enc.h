#ifndef __JPEG_ENC_H__
#define __JPEG_ENC_H__

#include "image-info.h"

/*****************************************************************************/

struct GDMBuffer;
struct GDMJPEGEncoder;

/*****************************************************************************/

struct GDMJPEGEncConfig {
    struct GDMImageInfo image;
    unsigned int quality;
};

/*****************************************************************************/

struct GDMJPEGEncoder *GJPEGEncOpen(void);
void GJPEGEncClose(struct GDMJPEGEncoder *e);
void GJPEGEncEncodeFrame(struct GDMJPEGEncoder *e,
        const struct GDMBuffer *src, struct GDMBuffer *dst,
        struct GDMJPEGEncConfig *conf);

#endif  /*__JPEG_ENC_H__*/
