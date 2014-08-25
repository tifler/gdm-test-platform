#ifndef __ISP_IO_H__
#define __ISP_IO_H__

#include <stdint.h>
#include "isp-ioctl.h"

/*****************************************************************************/

#define DRIVER_MSG_OFF  \
    do { writeCommand(ISP_CTRL_IOC_DEBUG, 0); gdebug1 = 0; } while(0)
#define DRIVER_MSG_ON   \
    do { writeCommand(ISP_CTRL_IOC_DEBUG, 1); gdebug1 = 1; } while(0)
#define DRIVER_INT_WAIT         writeCommand(ISP_CTRL_IOC_WAITINT, -1)
#define DRIVER_SET_INTMASK(a)   writeCommand(ISP_CTRL_IOC_INTMASK, a)
#define I2C_WRITE(a)            writeCommand(ISP_I2C_IOC_WRITE, (uint32_t)(a))
#define I2C_READ(a)             writeCommand(ISP_I2C_IOC_READ, (uint32_t)(a))

#ifdef   __USE_IOCTL_IO
//[[!__USE_IOCTL_IO
#define ISP_WRITE(a, v)         setISPRegister(a, v)
#define ISP_READ(a)             getISPRegister(a)
#define SIF_WRITE(a, v)         setSIFRegister(a, v)
#define SIF_READ(a)             getSIFRegister(a)
#define DXO_WRITE(a, v)         setDXORegister(a, v)
#define DXO_READ(a)             getDXORegister(a)
//]]!__USE_IOCTL_IO
#else
//[[__USE_IOCTL_IO
#define ISP_WRITE(a, v) \
    (*(volatile uint32_t *)((char *)ISPBase + (a)) = ((uint32_t)(v)))
#define ISP_READ(a)     \
    (*(volatile uint32_t *)((char *)ISPBase + (a)))
#define SIF_WRITE(a, v) \
    (*(volatile uint32_t *)((char *)SIFBase + (a)) = ((uint32_t)(v)))
#define SIF_READ(a)     \
    (*(volatile uint32_t *)((char *)SIFBase + (a)))
#define DXO_WRITE(a, v) \
    (*(volatile uint32_t *)((char *)DXOBase + (a)) = ((uint32_t)(v)))
#define DXO_READ(a)     \
    (*(volatile uint32_t *)((char *)DXOBase + (a)))
//]]__USE_IOCTL_IO
#endif

/*****************************************************************************/

extern int gdebug1;

#ifdef   __USE_IOCTL_IO
void setDXORegister(unsigned int offset, unsigned int value);
unsigned int getDXORegister(unsigned int offset);
void setISPRegister(unsigned int offset, unsigned int value);
unsigned int getISPRegister(unsigned int offset);
void setSIFRegister(unsigned int offset, unsigned int value);
unsigned int getSIFRegister(unsigned int offset);
#else
extern unsigned char *ISPBase;
extern unsigned char *SIFBase;
extern unsigned char *DXOBase;
#endif  /*!__USE_IOCTL_IO*/

uint32_t writeCommand(uint32_t cmd, uint32_t value);

/*****************************************************************************/

void initISPIO(void);
void exitISPIO(void);

#endif  /*__ISP_IO_H__*/
