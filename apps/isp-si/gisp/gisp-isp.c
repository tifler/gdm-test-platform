#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "gisp-isp.h"
#include "gisp-iodev.h"
#include "gdm-isp-ioctl.h"
#include "debug.h"

/*****************************************************************************/

#define ISP_IODEV_PATH                  "/dev/gisp-ctrl-isp"

#define ISP_WRITE(a, v) \
    (*(volatile uint32_t *)((char *)ISPBase + (a)) = ((uint32_t)(v)))
#define ISP_READ(a)     \
    (*(volatile uint32_t *)((char *)ISPBase + (a)))

/*****************************************************************************/

struct ISP {
    struct IODevice *io;
};

/*****************************************************************************/

static unsigned char *ISPBase;
static int ISPFd;

/*****************************************************************************/

struct ISP *ISPInit(void)
{
    struct ISP *isp;

    isp = (struct ISP *)calloc(1, sizeof(*isp));
    ASSERT(isp);

    isp->io = openIODevice(ISP_IODEV_PATH);
    ASSERT(isp->io);

    ISPBase = (unsigned char *)isp->io->mapBase;
    ISPFd = isp->io->fd;

    ISP_WRITE(0x14, 0xff);  // h/w reset
    usleep(10000);
    ISP_WRITE(0x14, 0);

    ISP_WRITE(0x10c, 0);
    ISP_WRITE(0x11c, 0);

    return isp;
}

void ISPExit(struct ISP *isp)
{
    ASSERT(isp);
    closeIODevice(isp->io);
    free(isp);
}

uint32_t writeCommand(uint32_t cmd, uint32_t value)
{
    uint32_t result;
    ASSERT(ISPFd > 0 && "You MUST call ISPInit() before writeCommand()");
    result = ioctl(ISPFd, cmd, value);
    return result;
}

void ISPSetBT601Port(struct ISP *isp, int portId)
{
    int ret;
    ret = ioctl(isp->io->fd, ISP_CTRL_IOC_SEL_BT601, portId);
    ASSERT(ret == 0);
}

void ISPSetBT601Enable(struct ISP *isp, int enable)
{
    int ret;
    ret = ioctl(isp->io->fd, ISP_CTRL_IOC_ENABLE_BT601, enable);
    ASSERT(ret == 0);
}

void ISPSetBT601Size(struct ISP *isp, unsigned width, unsigned height)
{
    int ret;
    struct gdm_isp_bt_size btSize;

    btSize.width = width;
    btSize.height = height;
    ret = ioctl(isp->io->fd, ISP_CTRL_IOC_FORMAT_BT601, &btSize);
    ASSERT(ret == 0);
}
