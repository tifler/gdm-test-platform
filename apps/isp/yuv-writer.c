#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include "yuv-writer.h"
#include "image-info.h"
#include "file-io.h"
#include "gdm-buffer.h"
#include "debug.h"

/*****************************************************************************/

struct GDMYUVWriter {
    int fd;
    uint32_t writeImages;
    struct GDMImageInfo image;
};

/*****************************************************************************/

static void writeImagePlane(int fd,
        void *bufptr, const struct GDMImageInfo *info, int plane)
{
    int ret;
    int line;
    int nwrite = 0;
    char *ptr = (char *)bufptr;

    ASSERT(fd > 0);
    ASSERT(bufptr);
    ASSERT(info);

    for (line = 0; line < info->plane[plane].lines; line++) {
        ret = safeWrite(fd, ptr, info->plane[plane].bpl);
        ASSERT(ret == info->plane[plane].bpl);

        nwrite += ret;
        ptr += info->plane[plane].stride;
    }
}

/*****************************************************************************/

struct GDMYUVWriter *GYUVWriterOpen(const char *path)
{
    struct GDMYUVWriter *w;

    ASSERT(path);

    w = calloc(1, sizeof(*w));
    ASSERT(w);

    w->fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0644);
    ASSERT(w->fd > 0);

    return w;
}

void GYUVWriterClose(struct GDMYUVWriter *w)
{
    close(w->fd);
    free(w);
}

void GYUVWriterWrite(struct GDMYUVWriter *w,
        const struct GDMImageInfo *info, struct GDMBuffer *buffer)
{
    int i;

    ASSERT(w);
    ASSERT(info);
    ASSERT(buffer);
    ASSERT(info->planeCount == buffer->planeCount);

    if (w->writeImages == 0) {
        memcpy(&w->image, info, sizeof(*info));
    }
    else if (memcmp(&w->image, info, sizeof(*info)) != 0) {
        ASSERT(!"First image and the others must be equal...");
    }

    for (i = 0; i < buffer->planeCount; i++) {
        writeImagePlane(w->fd,
                buffer->plane[i].base + info->plane[i].offset, info, i);
    }
    w->writeImages++;
    DBG("Write Image = %d", w->writeImages);
}
