#ifndef	__ISP_DRIVER_H__
#define __ISP_DRIVER_H__

#if	0
// isp register map
#define ISP_VIRTUAL_BASE		0xED100000
#define ISP_VIRTUAL_REGION		0x50000
#define SIF_VIRTUAL_BASE		0xED350000
#define SIF_VIRTUAL_REGION		0x4000

#define DXOISP_REG_VIR_BASE		(0x00800000)
#define ISP_REG_VIR_BASE		(0x00A40000)
#define SIF_REG_VIR_BASE		(0x00AC0000)


#define SIF_WRITE(a, d)			*(volatile int *)((char *)gsif_base+a)=(d)
#define SIF_READ(a)				*(volatile int *)((char *)gsif_base+a)

#define DXOISP_WRITE(a, d)		*(volatile int *)((char *)gdxo_base+a)=(d)
#define DXOISP_READ(a)			*(volatile int *)((char *)gdxo_base+a)

#define ISP_WRITE(a, d)			*(volatile int *)((char *)gisp_base+a)=(d)
#define ISP_READ(a)				*(volatile int *)((char *)gisp_base+a)
#endif

// for i2c master define


// for isp driver define
typedef enum
{
	IOCTL_SET_OFFSET = 0,
	IOCTL_READ,
	IOCTL_WRITE,
	IOCTL_WAITINT,
	IOCTL_INTGEN,
	IOCTL_RUN_EVENT,
	IOCTL_I2CM_WRITE,
	IOCTL_I2CM_READ,
	IOCTL_DEBUG,
	IOCTL_INTMASK,
	IOCTL_REG_IO,
	IOCTL_SET_EVENTREG,
	IOCTL_GET_DEVINFO,
	IOCTL_END,
}ISP_IO_IOCTL;

typedef enum
{
	CMD_DXO_WRITE = 0,
	CMD_DXO_READ,
	CMD_SIF_WRITE,
	CMD_SIF_READ,
	CMD_ISP_WRITE,
	CMD_ISP_READ,
	CMD_END
}REG_CMD_LIST;

// for ioctl read/write structure
typedef struct
{
	uint16_t		cmd;
	uint16_t		length;
	uint32_t		offset;
	uint32_t		*pbuf;
} __attribute__((packed)) ioctl_reg_io;

// for i2cm structure
typedef struct
{
	unsigned short	id;
	unsigned short	addr;
	unsigned int	length;	// length
} __attribute__((packed)) ioctl_i2cm_info;

// for setup reg
typedef struct
{
	uint32_t addr;
	uint32_t data;
}reg_table;

// for event set regdata
typedef struct
{
	uint16_t	index;
	uint16_t	length;
	uint32_t	*pbuf;
} __attribute__((packed)) event_reg;


#if 0

// for ioctl command define
#define	ISP_IOCTL_MAGIC	'A'

#define ISP_IOCTL_SET_OFFSET	_IO(ISP_IOCTL_MAGIC, IOCTL_SET_OFFSET)
#define ISP_IOCTL_READ			_IO(ISP_IOCTL_MAGIC, IOCTL_READ)
#define ISP_IOCTL_WRITE			_IO(ISP_IOCTL_MAGIC, IOCTL_WRITE)
#define ISP_IOCTL_WAITINT		_IO(ISP_IOCTL_MAGIC, IOCTL_WAITINT)
#define ISP_IOCTL_INTGEN		_IO(ISP_IOCTL_MAGIC, IOCTL_INTGEN)
#define ISP_IOCTL_RUN_EVENT		_IO(ISP_IOCTL_MAGIC, IOCTL_RUN_EVENT)
#define ISP_IOCTL_I2CM_WRITE	_IO(ISP_IOCTL_MAGIC, IOCTL_I2CM_WRITE)
#define ISP_IOCTL_I2CM_READ		_IO(ISP_IOCTL_MAGIC, IOCTL_I2CM_READ)
#define ISP_IOCTL_DEBUG			_IO(ISP_IOCTL_MAGIC, IOCTL_DEBUG)
#define ISP_IOCTL_INTMASK		_IO(ISP_IOCTL_MAGIC, IOCTL_INTMASK)
#define ISP_IOCTL_REG_IO		_IO(ISP_IOCTL_MAGIC, IOCTL_REG_IO)
#define ISP_IOCTL_SET_EVENTREG	_IO(ISP_IOCTL_MAGIC, IOCTL_SET_EVENTREG)

#define ISP_IOCTL_GET_DEVINFO   _IO(ISP_IOCTL_MAGIC, IOCTL_GET_DEVINFO)

#else

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

#if 0
typedef struct {
    unsigned short id;
    unsigned short addr;
    unsigned int length;
} __attribute__((packed)) ioctl_i2cm_info;
#endif  /*0*/

#define ISP_I2C_IOC_WRITE       _IOWR(ISP_I2C_IOC_MAGIC, 0, ioctl_i2cm_info)
#define ISP_I2C_IOC_READ        _IOWR(ISP_I2C_IOC_MAGIC, 1, ioctl_i2cm_info)


#endif  /*0*/

#define ISP_IRQ_NUMBER0			(67+32)
#define ISP_IRQ_NUMBER1			(68+32)

#endif

/* EOF */

