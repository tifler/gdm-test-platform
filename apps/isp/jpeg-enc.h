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

#ifdef	SUPPORT_JPEG_ENC
struct GDMJPEGEncoder *GJPEGEncOpen(void);
void GJPEGEncClose(struct GDMJPEGEncoder *e);
void GJPEGEncEncodeFrame(struct GDMJPEGEncoder *e,
        const struct GDMBuffer *src, struct GDMBuffer *dst,
        struct GDMJPEGEncConfig *conf);
#else
struct GDMJPEGEncoder *GJPEGEncOpen(void)
{
	return (struct GDMJPEGEncoder *)0;
}

void GJPEGEncClose(struct GDMJPEGEncoder *e)
{
}

void GJPEGEncEncodeFrame(struct GDMJPEGEncoder *e,
        const struct GDMBuffer *src, struct GDMBuffer *dst,
        struct GDMJPEGEncConfig *conf)
{
}
#endif	/*SUPPORT_JPEG_ENC*/

#endif  /*__JPEG_ENC_H__*/
