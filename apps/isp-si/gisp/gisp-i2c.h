#ifndef __GISP_I2C_H__
#define __GISP_I2C_H__

#include <stdint.h>
#include "gdm-isp-ioctl.h"

/*****************************************************************************/

#define I2C_WRITE(a)            writeCommand(ISP_I2C_IOC_WRITE, (uint32_t)(a))
#define I2C_READ(a)             writeCommand(ISP_I2C_IOC_READ, (uint32_t)(a))

/*****************************************************************************/

extern uint32_t writeCommand(uint32_t cmd, uint32_t value);

#endif  /*__GISP_I2C_GISP_I2C_H__*/
