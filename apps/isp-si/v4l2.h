#ifndef __GISP_V4L2_H__
#define __GISP_V4L2_H__

#include <linux/videodev2.h>

/*****************************************************************************/

struct GDMBuffer;

/*****************************************************************************/

int v4l2_enum_fmt(int fd, unsigned int pixelFormat);
int v4l2_s_fmt(int fd,
        int width, int height,
        unsigned int pixelFormat, struct v4l2_format *fmtbuf);
int v4l2_querycap(int fd);
int v4l2_enum_input(int fd, int index, char *buf, int buflen);
int v4l2_s_input(int fd, int index);
int v4l2_reqbufs(int fd, int bufferCount);
int v4l2_qbuf(int fd,
        int width, int height, const struct GDMBuffer *buffer, int index);
int v4l2_dqbuf(int fd, int planeCount);
int v4l2_streamon(int fd);
int v4l2_streamoff(int fd);
int v4l2_g_ctrl(int fd, unsigned int id, int *value);
int v4l2_s_ctrl(int fd, unsigned int id, int value);

/*****************************************************************************/

#endif  /*__V4L2_H__*/
