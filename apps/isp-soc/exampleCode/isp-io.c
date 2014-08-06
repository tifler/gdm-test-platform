#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "isp.h"
#include "isp-io.h"
#include "debug.h"

/*****************************************************************************/

static const char *ioDevPath[] = {
    "/dev/gisp-ctrl-isp",
    "/dev/gisp-ctrl-sif",
    "/dev/gisp-ctrl-dxo",
};

struct ISPIODeviceInfo {
    unsigned long phys;
    unsigned int size;
};

/*****************************************************************************/

static int getIoDevInfo(int fd, struct gdm_isp_iodev_info *info)
{
    return ioctl(fd, ISP_CTRL_IOC_GET_DEVINFO, info);
}

/*****************************************************************************/

struct ISPIODevice *openISPIODevice(int devId)
{
    int i;
    int ret;
    struct ISPIODevice *dev;
    struct gdm_isp_iodev_info info;

    dev = (struct ISPIODevice *)calloc(1, sizeof(*dev));
    ASSERT(dev);

    dev->fd = open(ioDevPath[devId], O_RDWR);
    ASSERT(dev->fd > 0);
    ret = getIoDevInfo(dev->fd, &info);
    ASSERT(ret == 0);
    dev->size = info.size;
    dev->phys = info.phys;

    dev->base = mmap(
            NULL, dev->size, PROT_READ|PROT_WRITE, MAP_SHARED, dev->fd, 0);
    ASSERT(dev->base != MAP_FAILED);

    return dev;
}

void closeISPIODevice(struct ISPIODevice *dev)
{
    ASSERT(dev);
    ASSERT(dev->fd > 0);
    ASSERT(dev->base);
    ASSERT(dev->size > 0);

    munmap(dev->base, dev->size);
    close(dev->fd);
    free(dev);
}
