#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <gisp-ioctl.h>
#include <linux/videodev2.h>

#include "gisp-sif.h"
#include "gisp-sensor.h"
#include "gisp-iodev.h"
#include "debug.h"

/*****************************************************************************/

#define SIF_IODEV_PATH                  "/dev/gisp-ctrl-sif"

#define SIF_WRITE(a, v) \
    (*(volatile uint32_t *)((char *)SIFBase + (a)) = ((uint32_t)(v)))
#define SIF_READ(a)     \
    (*(volatile uint32_t *)((char *)SIFBase + (a)))

/*****************************************************************************/

struct SIF {
    struct IODevice *io;
};

/*****************************************************************************/

static unsigned char *SIFBase;

/*****************************************************************************/

struct SIF *SIFInit(void)
{
    struct SIF *sif;

    sif = (struct SIF *)calloc(1, sizeof(*sif));
    ASSERT(sif);

    sif->io = openIODevice(SIF_IODEV_PATH);
    ASSERT(sif->io);

    SIFBase = (unsigned char *)sif->io->mapBase;

    // sensor reset
    SIF_WRITE(0x10, 0x830);
    usleep(100000);
    SIF_WRITE(0x10, 0x820);

    // for isp setting
    SIF_WRITE(0x304, 0x0);  // vsync normal

    // for input debug
    SIF_WRITE(0x20, 0x1);   // sensor input enable

    SIF_WRITE(0x300, 1);    // 0:  high resolution 1: front sensor

    //SIF_WRITE(0x320, (IMAGE_HEIGHT<<16)|IMAGE_WIDTH);
    SIF_WRITE(0x310, 0x00); // eof = vsync rear sof=href start

    SIF_WRITE(0x330, 0x0);  // input ctrl vsync sync action

    SIF_WRITE(0x14, 0x2);   // mclk 1/2 div

    return sif;
}

void SIFSetConfig(struct SIF *sif, const struct SIFConfig *conf)
{
    int i;
    struct SENSOR_MODE mode;

    for (i = 0; ; i++) {
        if (SensorGetMode(conf->id, i, &mode) < 0) {
            ASSERT(! "Sensor Not Found.");
        }

        if (mode.width == conf->width &&
                mode.height == conf->height && mode.fps == conf->fps) {
            DBG("Sensor Found.");
            break;
        }
    }

    SensorSetMode(conf->id, i);
    SIF_WRITE(0x320, (conf->height << 16) | conf->width);
}

void SIFExit(struct SIF *sif)
{
    ASSERT(sif);
    closeIODevice(sif->io);
    free(sif);
}

