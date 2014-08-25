#ifndef	__ISP_DRIVER_H__
#define __ISP_DRIVER_H__

#include <sys/ioctl.h>

// for setup reg
typedef struct
{
	uint32_t addr;
	uint32_t data;
} reg_table;

// for event set regdata
typedef struct
{
	uint16_t	index;
	uint16_t	length;
	uint32_t	*pbuf;
} __attribute__((packed)) event_reg;

/*****************************************************************************/

#define ISP_CTRL_IOC_MAGIC      ('c')

#define ISP_CTRL_IOC_WAITINT        _IOWR(ISP_CTRL_IOC_MAGIC, 0, int)
#define ISP_CTRL_IOC_INTGEN         _IOWR(ISP_CTRL_IOC_MAGIC, 1, int)
#define ISP_CTRL_IOC_DEBUG          _IOWR(ISP_CTRL_IOC_MAGIC, 2, int)
#define ISP_CTRL_IOC_INTMASK        _IOWR(ISP_CTRL_IOC_MAGIC, 3, int)

struct gdm_isp_iodev_info {
    unsigned long phys;
    unsigned int size;
};
#define ISP_CTRL_IOC_GET_DEVINFO    _IOR(ISP_CTRL_IOC_MAGIC, 5, struct gdm_isp_iodev_info)

struct gdm_isp_iodev_reg {
    unsigned int offset;
    unsigned int value;
};
#define ISP_CTRL_IOC_READ           _IOWR(ISP_CTRL_IOC_MAGIC, 6, struct gdm_isp_iodev_reg)
#define ISP_CTRL_IOC_WRITE          _IOW(ISP_CTRL_IOC_MAGIC, 7, struct gdm_isp_iodev_reg)

/*****************************************************************************/

#define ISP_I2C_IOC_MAGIC       ('i')

typedef struct _ioctl_i2cm_info {
	unsigned short	id;
	unsigned short	addr;
	unsigned int	length;	// length
} __attribute__((packed)) ioctl_i2cm_info;

#define ISP_I2C_IOC_WRITE       _IOWR(ISP_I2C_IOC_MAGIC, 0, ioctl_i2cm_info)
#define ISP_I2C_IOC_READ        _IOWR(ISP_I2C_IOC_MAGIC, 1, ioctl_i2cm_info)


#endif

/* EOF */

