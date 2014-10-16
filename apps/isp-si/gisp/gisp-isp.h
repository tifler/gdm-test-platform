#ifndef __GISP_ISP_H__
#define __GISP_ISP_H__

#include <stdint.h>

/*****************************************************************************/

struct ISP;

/*****************************************************************************/

struct ISP *ISPInit(void);
void ISPExit(struct ISP *isp);
uint32_t writeCommand(uint32_t cmd, uint32_t value);
void ISPSetBT601Port(struct ISP *isp, int portId);
void ISPSetBT601Enable(struct ISP *isp, int enable);
void ISPSetBT601Size(struct ISP *isp, unsigned width, unsigned height);

#endif  /*__GISP_ISP_H__*/
