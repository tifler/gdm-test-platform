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

#define ISP_CTRL_IOC_SEL_VSENSOR    _IOW(ISP_CTRL_IOC_MAGIC, 8, int)
#define ISP_CTRL_IOC_SEL_BT601      _IOW(ISP_CTRL_IOC_MAGIC, 9, int)

#define ISP_CTRL_IOC_ENABLE_BT601   _IOW(ISP_CTRL_IOC_MAGIC, 10, int)

struct gdm_isp_bt_size {
    unsigned int width;
    unsigned int height;
};
#define ISP_CTRL_IOC_FORMAT_BT601   _IOW(ISP_CTRL_IOC_MAGIC, 11, struct gdm_isp_bt_size)

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

/*****************************************************************************/
/* Virtual Sensor */

#define GISP_CID_BASE                           (V4L2_CID_USER_BASE | 0xe000)
#define GISP_CID_SELECT_VS_TO                   (GISP_CID_BASE + 1)
enum gisp_vs_to {
    GISP_VS_OFF,            // RS - DxO - WDMA
    GISP_VS_RESERVED,       // Reserved
    GISP_VS_TO_DXO,         // VS - DxO - WDMA
    GISP_VS_TO_BT601,       // RS - DxO - WDMA / VS - BT601
    GISP_VS_TO_WDMA_CAP,    // VS - WDMA[CAP]
    GISP_VS_TO_WDMA_VID,    // VS - WDMA[VID]
    GISP_VS_TO_WDMA_DIS,    // VS - WDMA[DIS]
    GISP_VS_TO_WDMA_FD,     // VS - WDMA[FD]
    GISP_VS_TO_COUNT
};

#define GISP_CID_SELECT_BT_FROM                 (GISP_CID_BASE + 2)
// when GISP_CID_VS_MODE is GISP_VS_OFF, this control specify 
enum gisp_bt_from {
    /* from 0~3, bt is not used. */
    GISP_BT_OFF_WDMA_CAP,   // DXO[CAP] - WDMA[CAP]
    GISP_BT_OFF_WDMA_VID,   // DXO[VID] - WDMA[VID]
    GISP_BT_OFF_WDMA_DIS,   // DXO[DIS] - WDMA[DIS]
    GISP_BT_OFF_WDMA_FD,    // DXO[FD ] - WDMA[FD ]
    /* bt is used */
    GISP_BT_FROM_DXO_CAP,   // DXO[CAP] - BT601
    GISP_BT_FROM_DXO_VID,   // DXO[VID] - BT601
    GISP_BT_FROM_DXO_DIS,   // DXO[DIS] - BT601
    GISP_BT_FROM_DXO_FD,    // DXO[FD ] - BT601
    GISP_BT_FROM_COUNT
};

#endif	/*__GDM_ISP_IOCTL_H__*/
