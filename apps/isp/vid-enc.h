#ifndef __VID_ENC_H__
#define __VID_ENC_H__

struct GDMBuffer;
struct GDMEncoder;

/*****************************************************************************/

#define GDM_ENC_FLAG_THREAD                     (1 << 0)

/*****************************************************************************/

enum {
    GDM_VENC_FMT_H264,
    GDM_VENC_FMT_MPEG4,
    GDM_VENC_FMT_H263,
};

struct GDMEncConfig {
    unsigned int width;
    unsigned int height;
    unsigned int gopSize;
    unsigned int bitrate;
    unsigned int fps;
    unsigned int encFormat;
    int useSWEnc;
};

/*****************************************************************************/

#ifdef	SUPPORT_VID_ENC
struct GDMEncoder *GEncOpen(
        const struct GDMEncConfig *conf, const char *path, unsigned long flags);
void GEncClose(struct GDMEncoder *e);
void GEncStart(struct GDMEncoder *e);
void GEncStop(struct GDMEncoder *e);
int GEncEncodeFrame(struct GDMEncoder *e, struct GDMBuffer *frame);
#else
struct GDMEncoder *GEncOpen(
        const struct GDMEncConfig *conf, const char *path, unsigned long flags)
{
	return (struct GDMEncoder *)0;
}

void GEncClose(struct GDMEncoder *e)
{
}

void GEncStart(struct GDMEncoder *e)
{
}

void GEncStop(struct GDMEncoder *e)
{
}

int GEncEncodeFrame(struct GDMEncoder *e, struct GDMBuffer *frame)
{
	return 0;
}
#endif	/*SUPPORT_VID_ENC*/

#endif  /*__VID_ENC_H__*/
