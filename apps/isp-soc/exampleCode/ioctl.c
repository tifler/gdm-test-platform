/*--------------------------------------------------------------------------
/	Project name	: 
/	Copyright		: Anapass, All rights reserved.
/	File name		: ioctl.c
/	Description		:
/
/	
/	
/	
/	Name		Date			Action
/	thlee		14.07.18		Create
---------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------------------------
													INCLUDE
 ------------------------------------------------------------------------------------------------------------------ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "isp.h"
#include "define.h"
#include "debug.h"
#include "ioctl.h"

#include "isp-io.h"

/* --------------------------------------------------------------------------------------------------------------------
													LOCAL CONSTANTS
 ------------------------------------------------------------------------------------------------------------------ */
/* --------------------------------------------------------------------------------------------------------------------
													LOCAL DATA TYPES
 ------------------------------------------------------------------------------------------------------------------ */
/* --------------------------------------------------------------------------------------------------------------------
													LOCAL FUNCTION PROTOTYPES
 ------------------------------------------------------------------------------------------------------------------ */
/* --------------------------------------------------------------------------------------------------------------------
													LOCAL VARIABLES
 ------------------------------------------------------------------------------------------------------------------ */
#if 0
int			gIspDev;
#else
static struct ISPIODevice *ioDev[IODEV_COUNT];

// global export
unsigned char *ISPBase;
unsigned char *SIFBase;
unsigned char *DXOBase;
#endif  /*0*/

/* --------------------------------------------------------------------------------------------------------------------
													FUNCTIONS
 ------------------------------------------------------------------------------------------------------------------ */
int isp_ioctl_init(void)
{
#if 0
	// isp device open
	gIspDev = open(ISP_IO_NAME, O_RDWR);	// | O_NDELAY);

	if(gIspDev < 0)
	{
		ISP_PRINTF("isp driver open error\n");
	}

	return gIspDev;
#else
    ioDev[IODEV_ISP] = openISPIODevice(IODEV_ISP);
    ASSERT(ioDev[IODEV_ISP]);
    ISPBase = (unsigned char *)ioDev[IODEV_ISP]->base;

    ioDev[IODEV_SIF] = openISPIODevice(IODEV_SIF);
    ASSERT(ioDev[IODEV_SIF]);
    SIFBase = (unsigned char *)ioDev[IODEV_SIF]->base;

    ioDev[IODEV_DXO] = openISPIODevice(IODEV_DXO);
    ASSERT(ioDev[IODEV_DXO]);
    DXOBase = (unsigned char *)ioDev[IODEV_DXO]->base;

    return 0;
#endif  /*0*/
}

void isp_ioctl_close(void)
{
#if 0
	close(gIspDev);
#else
    int i;

    for (i = 0; i < IODEV_COUNT; i++)
        closeISPIODevice(ioDev[i]);

    ISPBase = (unsigned char *)NULL;
    SIFBase = (unsigned char *)NULL;
    DXOBase = (unsigned char *)NULL;
#endif  /*0*/
}

#if 0
void isp_RegWrite(uint32_t cmd, uint32_t offset, uint32_t value)
{
	ioctl_reg_io	reg;

	reg.cmd			= cmd;
	reg.length		= 1;
	reg.offset		= offset;
	reg.pbuf		= (uint32_t *)value;

	ioctl(gIspDev, ISP_IOCTL_REG_IO, &reg);
}

int isp_RegRead(uint32_t cmd, uint32_t offset)
{
	uint32_t		buf;
	ioctl_reg_io	reg;

	reg.cmd		= cmd;
	reg.length	= 1;
	reg.offset	= offset;
	reg.pbuf	= &buf;

	ioctl(gIspDev, ISP_IOCTL_REG_IO, &reg);

	return buf;
}
#endif  /*0*/

uint32_t isp_cmd_write(uint32_t cmd, uint32_t value)
{
#if 0
	uint32_t	result;
	result = ioctl(gIspDev, cmd, value);
	return result;
#else
	uint32_t	result;
	result = ioctl(ioDev[IODEV_ISP]->fd, cmd, value);
	return result;
#endif  /*0*/
}

void isp_setRegister(const int uiOffset, int uiValue) 
{
#if 0
	ioctl(gIspDev, ISP_IOCTL_SET_OFFSET, uiOffset);
	ioctl(gIspDev, ISP_IOCTL_WRITE, uiValue);
#else
    printf("============= setRegister(0x%x, 0x%x) ========\n", uiOffset, uiValue);
    ASSERT(! "isp_setRegister is not implemented");
    //TODO
#endif  /*0*/
}

#if 0
int isp_getRegister(const int uiOffset) 
{
	int uiResult;
	ioctl(gIspDev, ISP_IOCTL_SET_OFFSET, uiOffset);
	uiResult = ioctl(gIspDev, ISP_IOCTL_READ, 0);
	return uiResult;
}
#endif  /*0*/

void isp_setDXORegister(unsigned int offset, unsigned int value)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = value;
	ioctl(ioDev[IODEV_DXO]->fd, ISP_CTRL_IOC_WRITE, &reg);
}

unsigned int isp_getDXORegister(unsigned int offset)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = 0;
	ioctl(ioDev[IODEV_DXO]->fd, ISP_CTRL_IOC_READ, &reg);
    return reg.value;
}

void isp_setISPRegister(unsigned int offset, unsigned int value)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = value;
	ioctl(ioDev[IODEV_ISP]->fd, ISP_CTRL_IOC_WRITE, &reg);
}

unsigned int isp_getISPRegister(unsigned int offset)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = 0;
	ioctl(ioDev[IODEV_ISP]->fd, ISP_CTRL_IOC_READ, &reg);
    return reg.value;
}

void isp_setSIFRegister(unsigned int offset, unsigned int value)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = value;
	ioctl(ioDev[IODEV_SIF]->fd, ISP_CTRL_IOC_WRITE, &reg);
}

unsigned int isp_getSIFRegister(unsigned int offset)
{
    struct gdm_isp_iodev_reg reg;
    reg.offset = offset;
    reg.value = 0;
	ioctl(ioDev[IODEV_SIF]->fd, ISP_CTRL_IOC_READ, &reg);
    return reg.value;
}

/* EOF */

