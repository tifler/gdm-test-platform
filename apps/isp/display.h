#ifndef __DISPLAY_H__
#define __DISPLAY_H__

struct GDMBuffer;
struct GDMDisplay;

/*****************************************************************************/

struct GDMDispFormat {
    unsigned int pixelformat;       // V4L2_PIX_FMT_*
    unsigned int width;
    unsigned int height;
};

struct GDMDispAttr {
    int CURRENTLY_THIS_STRUCTURE_IS_NOT_USED;
};

/*****************************************************************************/

struct GDMDisplay *GDispOpen(const char *path, unsigned long flags);
void GDispClose(struct GDMDisplay *d);
int GDispSetFormat(struct GDMDisplay *d, const struct GDMDispFormat *fmt);
int GDispSetAttr(struct GDMDisplay *d, const struct GDMDispAttr *attr);
int GDispSendFrame(struct GDMDisplay *d, struct GDMBuffer *frame, int timeout);

#endif  /*__DISPLAY_H__*/
