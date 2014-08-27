#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "isp-io.h"
#include "debug.h"

/*****************************************************************************/

enum {
    IODEV_ISP,
    IODEV_SIF,
    IODEV_DXO,
    IODEV_COUNT,
};

struct ISPIODevice {
    int fd;
    size_t size;
    unsigned long phys;
    void *base;
};

/*****************************************************************************/

static const char *ioDevPath[IODEV_COUNT] = {
    "/dev/gisp-ctrl-isp",
    "/dev/gisp-ctrl-sif",
    "/dev/gisp-ctrl-dxo",
};

/*****************************************************************************/

#ifndef __USE_IOCTL_IO
// global export
unsigned char *ISPBase;
unsigned char *SIFBase;
unsigned char *DXOBase;
#endif  //__USE_IOCTL_IO

static struct ISPIODevice *ioDev[IODEV_COUNT];

/*****************************************************************************/

static int getIoDevInfo(int fd, struct gdm_isp_iodev_info *info)
{
    return ioctl(fd, ISP_CTRL_IOC_GET_DEVINFO, info);
}

static struct ISPIODevice *openISPIODevice(int devId)
{
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

static void closeISPIODevice(struct ISPIODevice *dev)
{
    ASSERT(dev);
    ASSERT(dev->fd > 0);
    ASSERT(dev->base);
    ASSERT(dev->size > 0);

    munmap(dev->base, dev->size);
    close(dev->fd);
    free(dev);
}

/*****************************************************************************/

void initISPIO(void)
{
    ioDev[IODEV_ISP] = openISPIODevice(IODEV_ISP);
    ASSERT(ioDev[IODEV_ISP]);
#ifndef __USE_IOCTL_IO
    ISPBase = (unsigned char *)ioDev[IODEV_ISP]->base;
#endif

    ioDev[IODEV_SIF] = openISPIODevice(IODEV_SIF);
    ASSERT(ioDev[IODEV_SIF]);
#ifndef __USE_IOCTL_IO
    SIFBase = (unsigned char *)ioDev[IODEV_SIF]->base;
#endif

    ioDev[IODEV_DXO] = openISPIODevice(IODEV_DXO);
    ASSERT(ioDev[IODEV_DXO]);
#ifndef __USE_IOCTL_IO
    DXOBase = (unsigned char *)ioDev[IODEV_DXO]->base;
#endif
}

void exitISPIO(void)
{
    int i;

    for (i = 0; i < IODEV_COUNT; i++)
        closeISPIODevice(ioDev[i]);

#ifndef __USE_IOCTL_IO
    ISPBase = (unsigned char *)NULL;
    SIFBase = (unsigned char *)NULL;
    DXOBase = (unsigned char *)NULL;
#endif  /*__USE_IOCTL_IO*/
}

uint32_t writeCommand(uint32_t cmd, uint32_t value)
{
    uint32_t result;
    result = ioctl(ioDev[IODEV_ISP]->fd, cmd, value);
    return result;
}

#ifdef  __USE_IOCTL_IO
void setDXORegister(unsigned int offset, unsigned int value)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = value;
    ioctl(ioDev[IODEV_DXO]->fd, ISP_CTRL_IOC_WRITE, &reg);
}

unsigned int getDXORegister(unsigned int offset)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = 0;
    ioctl(ioDev[IODEV_DXO]->fd, ISP_CTRL_IOC_READ, &reg);
    return reg.value;
}

void setISPRegister(unsigned int offset, unsigned int value)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = value;
    ioctl(ioDev[IODEV_ISP]->fd, ISP_CTRL_IOC_WRITE, &reg);
}

unsigned int getISPRegister(unsigned int offset)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = 0;
    ioctl(ioDev[IODEV_ISP]->fd, ISP_CTRL_IOC_READ, &reg);
    return reg.value;
}

void setSIFRegister(unsigned int offset, unsigned int value)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = value;
    ioctl(ioDev[IODEV_SIF]->fd, ISP_CTRL_IOC_WRITE, &reg);
}

unsigned int getSIFRegister(unsigned int offset)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = 0;
    ioctl(ioDev[IODEV_SIF]->fd, ISP_CTRL_IOC_READ, &reg);
    return reg.value;
}
#endif  //__USE_IOCTL_IO
