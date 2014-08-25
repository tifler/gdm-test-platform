#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "isp-io.h"
#include "debug.h"

static int threadRun = 1;
extern int initdone;
int gdebug1;

static void *IRQHandlerThread(void *arg)
{
	unsigned int intmask;
	uint64_t time;
	static int vsensor_done = 0;
	uint8_t	i2c_data = 0;
	int	loop;
	static int event_cnt = 0;

    uint32_t loopCount;
    time_t currT, prevT;
    struct timeval __tv;

	ISP_PRINTF("Wait Int Thread Run\n");

    gettimeofday(&__tv, NULL);
    prevT = currT = __tv.tv_sec;

	while (threadRun)
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

        gettimeofday(&__tv, NULL);
        currT = __tv.tv_sec;
        if (currT != prevT) {
            ISP_PRINTF("%d FPS\n", loopCount);
            loopCount = 0;
            prevT = currT;
        }
	}

	ISP_PRINTF("Wait Int Thread end\n");
}

/*****************************************************************************/

void resetISP(void)
{
    ISP_PRINTF("ISP SUBSYSTEM RESET\n");
    ISP_WRITE(0x14, 0xff);  // h/w reset
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

void startIRQHandlerThread(void)
{
    int ret;
    pthread_t thid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&thid, &attr, IRQHandlerThread, NULL);
    if (ret) {
        ASSERT(!"IRQ Thread Create Failed.");
    }

    pthread_attr_destroy(&attr);
}
