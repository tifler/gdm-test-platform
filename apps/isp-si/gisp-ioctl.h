/* 
 * Copyright (c) 2013-2014 Anapass Inc.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * GDM7243V Image Subsystem Driver Header
 *
 */

#ifndef __GDM_ISP_IOCTL_H__
#define	__GDM_ISP_IOCTL_H__

#include <linux/ioctl.h>
#include <linux/v4l2-controls.h>

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

#define ISP_CTRL_IOC_BT601          _IOWR(ISP_CTRL_IOC_MAGIC, 8, int)

/*****************************************************************************/

#define ISP_I2C_IOC_MAGIC       ('i')

typedef struct {
    unsigned short id;
    unsigned short addr;
    unsigned int length;
} __attribute__((packed)) ioctl_i2cm_info;

#define ISP_I2C_IOC_WRITE       _IOWR(ISP_I2C_IOC_MAGIC, 0, ioctl_i2cm_info)
#define ISP_I2C_IOC_READ        _IOWR(ISP_I2C_IOC_MAGIC, 1, ioctl_i2cm_info)

// we re-defined V4L2_COLORFX_* for readability.

#define V4L2_COLORFX_MONOCHROME                 V4L2_COLORFX_BW
#define V4L2_COLORFX_DESATURATION               V4L2_COLORFX_ANTIQUE
#define V4L2_COLORFX_COLORFILTER                V4L2_COLORFX_SOLARIZATION
#define V4L2_COLORFX_OVEREXPOSER                V4L2_COLORFX_SILHOUETTE
#define V4L2_COLORFX_POSTERIZER                 V4L2_COLORFX_VIVID
#define V4L2_COLORFX_VINTAGE                    V4L2_COLORFX_SKIN_WHITEN

#endif	/*__GDM_ISP_IOCTL_H__*/
