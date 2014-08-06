// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "DxOISP.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <time.h>

#include "isp.h"
#include "define.h"
#include "shell.h"
#include "debug.h"
#include "ioctl.h"

int			gThreadState;
pthread_t	gWaitIntThreadHandle, gWorkThreadHandle;
pthread_attr_t	gWaitIntAttr;
int			gRcvIntValue	= 0;
int			gRcvIntCnt		= 0;

struct timeval	tv;

void DxOISP_PixelStuckHandler(void);

//extern int	gIspDev;

int			gdebug1;

uint32_t        I2C_Write                    ( uint16_t usOffset, uint8_t* ptucData, uint8_t ucSize );
uint32_t        I2C_Read                     ( uint16_t usOffset, uint8_t* ptucData, uint8_t ucSize );

extern int sensorStreaming;
extern int initdone;
#if	0
void isp_setRegister(const int uiOffset, int uiValue) 
{
	ioctl(gIspDev, ISP_IOCTL_SET_OFFSET, uiOffset);
	ioctl(gIspDev, ISP_IOCTL_WRITE, uiValue);

}

int isp_getRegister(const int uiOffset) 
{
	int uiResult;
	ioctl(gIspDev, ISP_IOCTL_SET_OFFSET, uiOffset);
	uiResult = ioctl(gIspDev, ISP_IOCTL_READ, 0);
	return uiResult;
}
#endif

void *WaitInterrupthread(void *data)
{
	unsigned int	intmask;
	uint64_t		time;
	static int		vsensor_done = 0;
	uint8_t			i2c_data = 0;
	int				loop;
	static	int		regtype = 2;
	static int		event_cnt = 0;

    uint32_t loopCount;
    time_t currT, prevT;
    struct timeval __tv;

	ISP_PRINTF("Wait Int Thread Run\n");

    gettimeofday(&__tv);
    prevT = currT = __tv.tv_sec;

	while(gThreadState==THREAD_RUN)
	{
		//intmask = DRIVER_INT_WAIT;
		//intmask = ioctl(gIspDev, ISP_IOCTL_WAITINT, NULL);
		intmask = DRIVER_INT_WAIT;
		//ISP_PRINTF("Int Pending Reg 0x%x\n", intmask);

		if(intmask&0x800000)
		{
			// vsensor start
			ISP_PRINTF("Vsensor int rcv\n");
			vsensor_done = 1;
		}
		
		if(intmask&0x40000000)
		{
			//gettimeofday(&tv, NULL);
			//time = tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
			//ISP_PRINTF("0x%x, 0x%x EOF Int\n", (unsigned int)(time>>32), (unsigned int)time);
		}

		if(intmask&0x18000)
		{
			if(gdebug1 == 0)
			{
				//isp_setRegister(SIF_BASE_ADDR+0x10, 0x024);
#if	1
				if((initdone)&&(event_cnt<2))
				{
					//ISP_PRINTF("Vsync in\n");

					//i2c_data = 0x42;
					//I2C_Write(0x3008, &i2c_data, 1);
					do {
						i2c_data = 0x42;
						I2C_Write(0x3008, &i2c_data, 1);
						i2c_data = 0;
						I2C_Read(0x3008, &i2c_data, 1);
						if(i2c_data!=0x42)ISP_PRINTF("i2cm stop error\n");
					}while(i2c_data!=0x42);

					//ioctl(gIspDev, ISP_IOCTL_DEBUG, 1);			// driver debug on
				}
#endif

				//isp_setRegister(SIF_BASE_ADDR+0x20, 0x0);	// sensor input enable

#if	0
				if(event_cnt < 2)
				{
					DxOISP_Event();
				}
				else
				{
					// 640x480 seting
					//ioctl(gIspDev, ISP_IOCTL_RUN_EVENT, regtype);
					//if(regtype==0)regtype++;
					
					// dualpath
                    //[[tifler
					//ioctl(gIspDev, ISP_IOCTL_RUN_EVENT, 3);
					DRIVER_RUN_EVENT(3);
                    //]]tifler
					//ioctl(gIspDev, ISP_IOCTL_RUN_EVENT, regtype);
					//if(regtype==2)regtype++;

				}
#else
				DxOISP_Event();

				//if(initdone)DxOISP_PostEvent();

#endif

				//isp_setRegister(SIF_BASE_ADDR+0x20, 0x1);	// sensor input enable

#if	1
				if((initdone)&&(event_cnt<2))
				{
					//i2c_data = 0x02;
					//I2C_Write(0x3008, &i2c_data, 1);
					//ISP_PRINTF("Vsync out\n");
					do {
						i2c_data = 0x02;
						I2C_Write(0x3008, &i2c_data, 1);
						i2c_data = 0;
						I2C_Read(0x3008, &i2c_data, 1);
						if(i2c_data!=0x02)ISP_PRINTF("i2cm start error\n");
					}while(i2c_data!=0x02);
				
					if(initdone)event_cnt++;
				}
#endif
				//isp_setRegister(SIF_BASE_ADDR+0x10, 0x020);

#if	0
				gettimeofday(&tv, NULL);
				time = tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
				ISP_PRINTF("0x%x, 0x%x Event Call\n", (unsigned int)(time>>32), (unsigned int)time);
#endif
				//DxOISP_PostEvent();
				if(vsensor_done)
				{
					vsensor_done = 0;
                    //[[tifler
					//isp_setRegister(ISP_BASE_ADDR+ 0x1000,4);     //ISP_VD_DP_IMG_SIZE
					ISP_WRITE(0x1000,4);     //ISP_VD_DP_IMG_SIZE
					//isp_setRegister(ISP_BASE_ADDR+ 0x1000,1);     //ISP_VD_DP_IMG_SIZE
					ISP_WRITE(0x1000,1);     //ISP_VD_DP_IMG_SIZE
                    //]]tifler
					ISP_PRINTF("Vsensor start\n");
				}
				//DxOISP_PostEvent();
			}
			else
			{

			//isp_setRegister(SIF_BASE_ADDR+0x10, 0x024);

			//isp_setRegister(SIF_BASE_ADDR+0x10, 0x020);
			}
		}

		//gRcvIntValue = intmask;
		//gRcvIntCnt++;

		//isp_setRegister(ISP_BASE_ADDR+0x108, 0xbffe);
		//isp_setRegister(ISP_BASE_ADDR+0x118, 0);
        loopCount++;

        gettimeofday(&__tv);
        currT = __tv.tv_sec;
        if (currT != prevT) {
            ISP_PRINTF("%d FPS\n", loopCount);
            loopCount = 0;
            prevT = currT;
        }
	}

	ISP_PRINTF("Wait Int Thread end\n");
}

void *WorkIntthread(void *data)
{
	unsigned int	intmask = 0;

	while(gThreadState==THREAD_RUN)
	{
		printf("cnt %d\n", intmask);
		intmask++;
		usleep(50000);
	}
}

static void isp_init(void)
{
    ISP_PRINTF("ISP SUBSYSTEM RESET\n");
	ISP_WRITE(0x14, 0xff);	// h/w reset
	usleep(10000);
	ISP_WRITE(0x14, 0);

    ISP_PRINTF("Clear IRQ Pending Flags\n");
    ISP_WRITE(0x10c, 0);
    ISP_WRITE(0x11c, 0);

    ISP_PRINTF("Sensor Reset\n");
    SIF_WRITE(0x10, 0x830);
    usleep(100000);
    SIF_WRITE(0x10, 0x820);
}

int main ( int argc, char** argv ) {
#if	0
	int				result;

	// isp device open
	gIspDev = open(ISP_IO_NAME, O_RDWR);	// | O_NDELAY);

	if(gIspDev < 0)
	{
		printf("isp driver open error\n");
		return 1;
	}

	// wait int thread run
	pthread_attr_init(&gWaitIntAttr);
	pthread_attr_setschedpolicy(&gWaitIntAttr, SCHED_FIFO);

	gThreadState = THREAD_RUN;
	result = pthread_create(&gWaitIntThreadHandle, &gWaitIntAttr, WaitInterrupthread, NULL);
	if(result < 0)
	{
		printf("Int Thread Create Error\n");
		close(gIspDev);
		return 1;
	}
	pthread_detach(gWaitIntThreadHandle);

	result = debug_init();
	if(result < 0)
	{
		close(gIspDev);
		return 1;
	}
	
	isp_setRegister(ISP_BASE_ADDR+0x14, 0xff);	// h/w reset
	usleep(10);
	isp_setRegister(ISP_BASE_ADDR+0x14, 0);
	
	ioctl(gIspDev, ISP_IOCTL_DEBUG, 0);			// driver debug off

	ISP_PRINTF("isp Firmware Ver %s\n", ISP_FIRMWARE_VER);

	
	while(1)
	{
		if(shell_main()==SHELL_RETURN_EXIT)break;
	}

	gThreadState = THREAD_STOP;
	sleep(1);
	pthread_attr_destroy(&gWaitIntAttr);
	// close isp dev
	close(gIspDev);
#endif

#define __CHECK_LINE__              ISP_PRINTF("===== %d =====\n", __LINE__)
	int				result;

	gdebug1 = 0;

	result = isp_ioctl_init();
	if(result < 0)return 1;

    // tifler
    isp_init();

	// wait int thread run
	pthread_attr_init(&gWaitIntAttr);
	pthread_attr_setschedpolicy(&gWaitIntAttr, SCHED_FIFO);

	gThreadState = THREAD_RUN;
	result = pthread_create(&gWaitIntThreadHandle, &gWaitIntAttr, WaitInterrupthread, NULL);
	if(result < 0)
	{
		printf("Int Thread Create Error\n");
		isp_ioctl_close();
		return 1;
	}
	pthread_detach(gWaitIntThreadHandle);

	result = debug_init();
	if(result < 0)
	{
		isp_ioctl_close();
		return 1;
	}

#if 0
    ISP_PRINTF("===== ISP RESET : ");
    //[[tifler
	//isp_setRegister(ISP_BASE_ADDR+0x14, 0xff);	// h/w reset
	ISP_WRITE(0x14, 0xff);	// h/w reset
	usleep(10000);
	//isp_setRegister(ISP_BASE_ADDR+0x14, 0);
	ISP_WRITE(0x14, 0);
    //]]tifler
    ISP_PRINTF("DONE =====\n");
#endif  /*0*/

	//ioctl(gIspDev, ISP_IOCTL_DEBUG, 0);			// driver debug off
    DRIVER_MSG_OFF;

	ISP_PRINTF("isp Firmware Ver %s\n", ISP_FIRMWARE_VER);
	
	while(1)
	{
		if(shell_main()==SHELL_RETURN_EXIT)break;
	}

	gThreadState = THREAD_STOP;
	sleep(1);
	pthread_attr_destroy(&gWaitIntAttr);
	isp_ioctl_close();
}

/* EOF */

