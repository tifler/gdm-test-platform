#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/videodev2.h>

#include "v4l2.h"
#include "gdm-isp-ioctl.h"
#include "gisp-sif.h"
#include "gisp-sensor.h"
#include "gisp-iodev.h"
#include "regs-isp.h"
#include "debug.h"

/*****************************************************************************/

#define SIF_SUBDEV_PATH                  "/dev/v4l-subdev1"

/*****************************************************************************/

enum {
    SIF_POL_VSYNC,
    SIF_POL_HSYNC,
    SIF_POL_PCLK,
    SIF_POL_COUNT,
};

/*****************************************************************************/

struct SIF {
    int fd;
    unsigned polarity;
};

/*****************************************************************************/

static void setPolarity(struct SIF *sif, unsigned mask, unsigned polarity)
{
    int ret;
    sif->polarity &= ~mask;
    sif->polarity |= (polarity & mask);
    ret = v4l2_s_ctrl(sif->fd, GISP_CID_SIF_PAR_POL, sif->polarity);
    ASSERT(ret >= 0);
}

/*****************************************************************************/

struct SIF *SIFInit(void)
{
    struct SIF *sif;

    sif = (struct SIF *)calloc(1, sizeof(*sif));
    ASSERT(sif);

    sif->fd = open(SIF_SUBDEV_PATH, O_RDWR);
    ASSERT(sif->fd > 0);

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

    if (conf->invPCLK)
        setPolarity(sif, 0x1, 0x1);
}

void SIFExit(struct SIF *sif)
{
    ASSERT(sif);
    close(sif->fd);
    free(sif);
}

