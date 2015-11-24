#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>

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

static void sifSetPolarity(struct SIF *sif, unsigned mask, unsigned polarity)
{
    int ret;
    sif->polarity &= ~mask;
    sif->polarity |= (polarity & mask);
    ret = v4l2_s_ctrl(sif->fd, GISP_CID_SIF_PAR_POL, sif->polarity);
    ASSERT(ret >= 0);
}

static void sifSetSize(struct SIF *sif, unsigned width, unsigned height)
{
    int ret;
    struct v4l2_subdev_format fmt;

    memset(&fmt, 0, sizeof(fmt));

    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    fmt.format.width = width;
    fmt.format.height = height;
    DBG("sifSetSize(%u, %u)", width, height);
    ret = ioctl(sif->fd, VIDIOC_SUBDEV_S_FMT, &fmt);

    if (ret != 0) {
        ERR("VIDIOC_SUBDEV_S_FMT: Failed.");
    }
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

    // SIF 실제 크기를 제어해야 한다.
    sifSetSize(sif, conf->width, conf->height);

    if (conf->invPCLK)
        sifSetPolarity(sif, 0x1, 0x1);
}

void SIFExit(struct SIF *sif)
{
    ASSERT(sif);
    close(sif->fd);
    free(sif);
}

