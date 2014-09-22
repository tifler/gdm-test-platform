#include <stdlib.h>
#include <sys/mman.h>

#include "ion.h"
#include "ion_api.h"
#include "gdm-buffer.h"
#include "debug.h"

/*****************************************************************************/

struct IonHelper {
    int fd;
    unsigned int flags;
    unsigned int heapMask;
    struct ion_handle *handle[3];
};

/*****************************************************************************/

#if 0
static struct IonHelper *openIonHelper(void)
{
    struct IonHelper *ion;

    ion = calloc(1, sizeof(*ion));
    ASSERT(ion);

    ion->flags = 0;
    ion->heapMask = (1 << ION_HEAP_TYPE_CARVEOUT);
    ion->fd = ion_open();
    ASSERT(ion->fd > 0);

    return ion;
}

static void closeIonHelper(struct IonHelper *ion)
{
    close(ion->fd);
    free(ion);
}

/*****************************************************************************/

struct GDMContigMemory {
    int fd;
    unsigned int size;
    void *base;
    void *priv;
};

struct GDMContigMemory *__allocContigMemory(
        struct IonHelper *ion, unsigned int size, unsigned long flags)
{
    struct GDMContigMemory *buf;

    buf = calloc(1, sizeof(*buf));
    buf->
}

struct GDMContigMemory *allocContigMemory(unsigned int size, unsigned long flags)
{
    struct IonHelper *ion;
    struct GDMContigMemory *buf;

    ion = openIonHelper();
    return __allocContigMemory(ion, size, flags);
}
#endif  /*0*/
/*****************************************************************************/

struct GDMBuffer *allocContigMemory(
        int planes, const unsigned int *planeSizes, unsigned long flags)
{
    int i;
    int ret;
    struct GDMBuffer *buf;
    struct IonHelper *ion;

    ASSERT(planes > 0);
    ASSERT(planes <= GDM_MAX_PLANES);
    ASSERT(planeSizes);

    ion = calloc(1, sizeof(*ion));
    ASSERT(ion);

    buf = calloc(1, sizeof(*buf));
    ASSERT(buf);

    for (i = 0; i < GDM_MAX_PLANES; i++)
        buf->plane[i].fd = -1;

    ion->flags = 0;
    ion->heapMask = (1 << ION_HEAP_TYPE_CARVEOUT);
    ion->fd = ion_open();
    ASSERT(ion->fd > 0);

    buf->planeCount = planes;

    // allocate memory from ion device for each planes
    for (i = 0; i < planes; i++) {
        DBG("Plane[%d] Size = %d", i, planeSizes[i]);
        ret = ion_alloc(ion->fd, planeSizes[i],
                0, ion->heapMask, ion->flags, &ion->handle[i]);
        if (ret) {
            ERR("ion_alloc() failed.");
            break;
        }

        if (flags & GDM_BUF_FLAG_MAP) {
            ret = ion_map(ion->fd, ion->handle[i], planeSizes[i],
                    PROT_READ | PROT_WRITE, MAP_SHARED, 0,
                    (unsigned char **)&buf->plane[i].base, &buf->plane[i].fd);
            if (ret) {
                ERR("ion_map() failed.");
                break;
            }
        }
        else {
            ret = ion_share(ion->fd, ion->handle[i], &buf->plane[i].fd);
            if (ret) {
                ERR("ion_share() failed.");
                break;
            }
        }
        buf->plane[i].length = planeSizes[i];
    }

    if (ret) {
        for (--i ; i >= 0; i--) {
            close(buf->plane[i].fd);
            buf->plane[i].fd = 0;
            ion_free(ion->fd, ion->handle[i]);
        }
        ion_close(ion->fd);
        free(ion);
        free(buf);
    }
    else {
        buf->priv = ion;
    }

    return buf;
}

void freeContigMemory(struct GDMBuffer *buf)
{
    int i;
    struct IonHelper *ion = (struct IonHelper *)buf->priv;

    ASSERT(ion);

    for (i = 0; i < buf->planeCount; i++) {
        munmap(buf->plane[i].base, buf->plane[i].length);
        close(buf->plane[i].fd);
        ion_free(ion->fd, ion->handle[i]);
        buf->plane[i].base = NULL;
        buf->plane[i].length = 0;
        buf->plane[i].fd = -1;
    }

    ion_close(ion->fd);
    free(ion);
    free(buf);
}

