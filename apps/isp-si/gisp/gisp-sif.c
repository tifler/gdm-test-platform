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
#include "regs-isp.h"
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

struct SIF *SIFInit(int useBT601)
{
    struct SIF *sif;

    sif = (struct SIF *)calloc(1, sizeof(*sif));
    ASSERT(sif);

    sif->io = openIODevice(SIF_IODEV_PATH);
    ASSERT(sif->io);

    SIFBase = (unsigned char *)sif->io->mapBase;

    // sensor reset
    SIF_WRITE(REG_SIF_FPGA_CON, 0x30 | (useBT601 ? 0 : 0x800));
    usleep(100000);
    SIF_WRITE(REG_SIF_FPGA_CON, 0x20 | (useBT601 ? 0 : 0x800));

    SIF_WRITE(REG_SIF_PAR_SENSOR_INVERT, 0x0);  // vsync normal
    SIF_WRITE(REG_SIF_CONT_SENSOR, 1);   // sensor input enable
    SIF_WRITE(REG_SIF_SEL_SENSOR, 1);   // PAR Front Sensor

    //SIF_WRITE(0x320, (IMAGE_HEIGHT<<16)|IMAGE_WIDTH);
    SIF_WRITE(REG_SIF_PAR_VSYNC_POINT, 0x00); // eof = vsync rear sof=href start
    SIF_WRITE(REG_SIF_PAR_ENB_BYPASS, 0x0);  // input ctrl vsync sync action
    SIF_WRITE(REG_SIF_FPGA_CLOCK, 0x2);   // mclk 1/2 div

    return sif;
}

void SIFSetConfig(struct SIF *sif, const struct SIFConfig *conf)
{
    int i;
    struct SENSOR_MODE mode;

    for (i = 0; ; i++) {
        if (SensorGetMode(conf->id, i, &mode) < 0) {
            ASSERT(! "Sensor Mode Not Found.");
        }

        if (mode.width == conf->width &&
                mode.height == conf->height && mode.fps == conf->fps) {
            DBG("Sensor Mode Found.(%u, %u, %u)",
                    mode.width, mode.height, mode.fps);
            break;
        }
    }

    SensorSetMode(conf->id, i);
    SIF_WRITE(REG_SIF_PAR_IMAGE_SIZE, (conf->height << 16) | conf->width);
}

void SIFExit(struct SIF *sif)
{
    ASSERT(sif);
    closeIODevice(sif->io);
    free(sif);
}

