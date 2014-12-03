#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "gisp-isp.h"
#include "gisp-iodev.h"
#include "gdm-isp-ioctl.h"
#include "debug.h"

/*****************************************************************************/

#define ISP_IODEV_PATH                  "/dev/gisp-ctrl-isp"

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
