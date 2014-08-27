#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include "DxOISP.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
//#include <string.h>

#include "isp-io.h"
#include "debug.h"

//#include "reg640x480.c"

#define SYSTEM_FREQUENCY   32	//100

#define	IMAGE_WIDTH			640
#define	IMAGE_HEIGHT		480

#define INPUT_SENSOR		1


extern gIspDev;
int			initdone=0;

uint32_t        I2C_Write                    ( uint16_t usOffset, uint8_t* ptucData, uint8_t ucSize );
uint32_t        I2C_Read                     ( uint16_t usOffset, uint8_t* ptucData, uint8_t ucSize );

int DxOISP_Printf(const char *format, ...) {
	int val;
	va_list args;
	va_start (args  , format);
	val = vfprintf(stdout, format, args  );
	va_end   (args );
	return val;
}

void DxOISP_PixelStuckHandler(void) {
	// function called by DxO firmware in case of trouble
	// here is defined to the status dump of the internal ISP
	ISP_PRINTF("isp error\n");
	DxOISP_DumpStatus();
}

void
fvtsBench_setupOutput
(	uint32_t        outputId
,	uint16_t		Width
,	uint16_t        height 
)
{ 
	//
	// CODE YOUR FUNCTION HERE
	//
#if 1	
	unsigned int unImgFormat = 1;
#elif 0 //1p
	//0x00 YUYV
	//0x10 UYVY
	//0x20 YVYU
	//0x30 VYUY
	
	unsigned int unImgFormat = 0x2 | (0x30);
#else 
	//0x00 UV
	//0x01 VU
	unsigned int unImgFormat = 0x1;
#endif
	unsigned int unImageSZ =(unsigned int)height;
	unsigned int unWidth = (unsigned int)Width;
	
	unImageSZ = (unImageSZ<<16) | unWidth;	
	
	if(outputId==0) //Display
	{
		ISP_WRITE( 0x3308,unImageSZ);     //ISP_VD_DP_IMG_SIZE
		ISP_WRITE( 0x330c, unWidth);     //ISP_VD_DP_IMG_STRID
		ISP_WRITE( 0x3310, 0xd8000000);//unBaseAddrYD[unScenarioIdx]); //ISP_VD_DP_BASE_YADDR
		ISP_WRITE( 0x3314, 0xd804B000);//unBaseAddrUD[unScenarioIdx]); //ISP_VD_DP_BASE_UADDR
		ISP_WRITE( 0x3318, 0xd6000000);//unBaseAddrVD[unScenarioIdx]); //ISP_VD_DP_BASE_VADDR
		ISP_WRITE( 0x3304, unImgFormat); //ISP_VD_DP_IMG_TYPE
		
		//ISP_WRITE( 0x3300, 0x0f01);      //ISP_VD_DP_CONTROL	
//[[tifler:comment out]]
#if 1
		ISP_WRITE( 0x3300, 0x0f10);      //ISP_VD_DP_CONTROL	
		ISP_WRITE( 0x3300, 0x0f11);      //ISP_VD_DP_CONTROL	
#endif  /*0*/

		// for vsensor
		ISP_WRITE( 0x1004, unImageSZ);     //ISP_VD_DP_IMG_SIZE
		ISP_WRITE( 0x1008, 0xd7000000);

		unImgFormat = 3;
		// for video
		ISP_WRITE( 0x3208,unImageSZ);     //ISP_VD_DP_IMG_SIZE
		ISP_WRITE( 0x320c, unWidth);     //ISP_VD_DP_IMG_STRID
		ISP_WRITE( 0x3210, 0xd9000000);//unBaseAddrYD[unScenarioIdx]); //ISP_VD_DP_BASE_YADDR
		ISP_WRITE( 0x3214, 0xd904B000);//unBaseAddrUD[unScenarioIdx]); //ISP_VD_DP_BASE_UADDR
		ISP_WRITE( 0x3218, 0xd905dc00);//unBaseAddrVD[unScenarioIdx]); //ISP_VD_DP_BASE_VADDR
		ISP_WRITE( 0x3204, unImgFormat); //ISP_VD_DP_IMG_TYPE

//[[tifler:comment out]]
#if 1
		ISP_WRITE( 0x3200, 0x0f10);      //ISP_VD_DP_CONTROL	
		ISP_WRITE( 0x3200, 0x0f11);      //ISP_VD_DP_CONTROL	
#endif  /*0*/

#if	0
		ISP_WRITE( 0x3108,unImageSZ);     //ISP_VD_DP_IMG_SIZE
		ISP_WRITE( 0x310c, unWidth);     //ISP_VD_DP_IMG_STRID
		ISP_WRITE( 0x3110, 0xd4200000);//unBaseAddrYD[unScenarioIdx]); //ISP_VD_DP_BASE_YADDR
		ISP_WRITE( 0x3114, 0xd5200000);//unBaseAddrUD[unScenarioIdx]); //ISP_VD_DP_BASE_UADDR
		ISP_WRITE( 0x3118, 0xd6200000);//unBaseAddrVD[unScenarioIdx]); //ISP_VD_DP_BASE_VADDR
		ISP_WRITE( 0x3104, unImgFormat); //ISP_VD_DP_IMG_TYPE
		
		ISP_WRITE( 0x3100, 0x0f10);      //ISP_VD_DP_CONTROL	
		ISP_WRITE( 0x3100, 0x0f11);      //ISP_VD_DP_CONTROL	

		ISP_WRITE( 0x3208,unImageSZ);     //ISP_VD_DP_IMG_SIZE
		ISP_WRITE( 0x320c, unWidth);     //ISP_VD_DP_IMG_STRID
		ISP_WRITE( 0x3210, 0xd4400000);//unBaseAddrYD[unScenarioIdx]); //ISP_VD_DP_BASE_YADDR
		ISP_WRITE( 0x3214, 0xd5400000);//unBaseAddrUD[unScenarioIdx]); //ISP_VD_DP_BASE_UADDR
		ISP_WRITE( 0x3218, 0xd6400000);//unBaseAddrVD[unScenarioIdx]); //ISP_VD_DP_BASE_VADDR
		ISP_WRITE( 0x3204, unImgFormat); //ISP_VD_DP_IMG_TYPE
		
		ISP_WRITE( 0x3200, 0x0f10);      //ISP_VD_DP_CONTROL	
		ISP_WRITE( 0x3200, 0x0f11);      //ISP_VD_DP_CONTROL	

		ISP_WRITE( 0x3408,unImageSZ);     //ISP_VD_DP_IMG_SIZE
		ISP_WRITE( 0x340c, unWidth);     //ISP_VD_DP_IMG_STRID
		ISP_WRITE( 0x3410, 0xd4600000);//unBaseAddrYD[unScenarioIdx]); //ISP_VD_DP_BASE_YADDR
		ISP_WRITE( 0x3414, 0xd5600000);//unBaseAddrUD[unScenarioIdx]); //ISP_VD_DP_BASE_UADDR
		ISP_WRITE( 0x3418, 0xd6600000);//unBaseAddrVD[unScenarioIdx]); //ISP_VD_DP_BASE_VADDR
		ISP_WRITE( 0x3404, unImgFormat); //ISP_VD_DP_IMG_TYPE
		
		ISP_WRITE( 0x3400, 0x0f10);      //ISP_VD_DP_CONTROL	
		ISP_WRITE( 0x3400, 0x0f11);      //ISP_VD_DP_CONTROL	
#endif
	}
#if	0
	else if(outputId==1) //Video
	{
		ISP_WRITE( 0x3208,unImageSZ);     //ISP_VD_CP_IMG_SIZE
		ISP_WRITE( 0x320c, unWidth);     //ISP_VD_CP_IMG_STRID
		ISP_WRITE( 0x3210, unBaseAddrYV[unScenarioIdx]); //ISP_VD_CP_BASE_YADDR
		ISP_WRITE( 0x3214, unBaseAddrUV[unScenarioIdx]); //ISP_VD_CP_BASE_UADDR
		ISP_WRITE( 0x3218, unBaseAddrVV[unScenarioIdx]); //ISP_VD_CP_BASE_VADDR
		ISP_WRITE( 0x3204, unImgFormat); //ISP_VD_CP_IMG_TYPE
		
		ISP_WRITE( 0x3200, 0x0f04);      //ISP_VD_CP_CONTROL		
	}
	else if(outputId==2) //Capture
	{
		ISP_WRITE( 0x3108,unImageSZ);     //ISP_WD_CP_IMG_SIZE
		ISP_WRITE( 0x310c, unWidth);     //ISP_WD_CP_IMG_STRID
		ISP_WRITE( 0x3110, unBaseAddrYC[unScenarioIdx]); //ISP_WD_CP_BASE_YADDR
		ISP_WRITE( 0x3114, unBaseAddrUC[unScenarioIdx]); //ISP_WD_CP_BASE_UADDR
		ISP_WRITE( 0x3118, unBaseAddrVC[unScenarioIdx]); //ISP_WD_CP_BASE_VADDR
		ISP_WRITE( 0x3104, unImgFormat); //ISP_WD_CP_IMG_TYPE
		
		ISP_WRITE( 0x3100, 0x0f04);      //ISP_WD_CP_CONTROL		
	}
	else if(outputId==3) //FaceDetection
	{
		ISP_WRITE( 0x3408,unImageSZ);     //ISP_VD_FD_IMG_SIZE
		ISP_WRITE( 0x340c, unWidth);     //ISP_VD_FD_IMG_STRID
		ISP_WRITE( 0x3410, unBaseAddrYF[unScenarioIdx]); //ISP_VD_FD_BASE_YADDR
		ISP_WRITE( 0x3414, unBaseAddrUF[unScenarioIdx]); //ISP_VD_FD_BASE_UADDR
		ISP_WRITE( 0x3418, unBaseAddrVF[unScenarioIdx]); //ISP_VD_FD_BASE_VADDR
		ISP_WRITE( 0x3404, unImgFormat); //ISP_VD_FD_IMG_TYPE
		
		ISP_WRITE( 0x3400, 0x0f04);      //ISP_VD_FD_CONTROL		
	}	
#endif
}

int shell_isp_init(char *data)
{
	uint8_t          ucMode;
	uint16_t         usTime = 4 << DxOISP_MIN_INTERFRAME_TIME_FRAC_PART ; 
	uint32_t         uiFreq = SYSTEM_FREQUENCY << DxOISP_SYSTEM_CLOCK_FRAC_PART ;
	ts_DxOIspCmd     stIspCmd;// = {0,};
	event_reg		event;
	uint8_t			i2c_data;

	// sensor reset
	SIF_WRITE(0x10, 0x830);
	usleep(100000);
	SIF_WRITE(0x10, 0x820);

	// for isp setting
	SIF_WRITE(0x304, 0x0);	// vsync normal
	
	//DRIVER_SET_INTMASK((0xff7e<<16)|0xffff);

	//ISP_WRITE(0x108, 0xff7e);	// int enable dxo only
	//ISP_WRITE(0x118, 0xffff);

#ifdef	INPUT_SENSOR
	// for input debug
	SIF_WRITE(0x20, 0x1);	// sensor input enable

	SIF_WRITE(0x300, 1);	// 0:  high resolution 1: front sensor

	SIF_WRITE(0x320, (IMAGE_HEIGHT<<16)|IMAGE_WIDTH);
	SIF_WRITE(0x310, 0x00);	// eof = vsync rear sof=href start
	
	SIF_WRITE(0x330, 0x0);	// input ctrl vsync sync action
	
	SIF_WRITE(0x14, 0x2);	// mclk 1/2 div
#else
	// vsensor setting
	ISP_WRITE(0x04, 0);
	ISP_WRITE(0x00, 2);

	SIF_WRITE(0x04, 0x0);
	SIF_WRITE(0x300, 0x0);
	SIF_WRITE(0x304, 0x0);
	SIF_WRITE(0x308, 0x0);
	SIF_WRITE(0x310, 0x0);
	SIF_WRITE(0x314, 0x0);
#endif
    //[[tifler:driver에서 처리하도록 여기서는 막음]]
	//fvtsBench_setupOutput(0, IMAGE_WIDTH, IMAGE_HEIGHT);


#if 0
	// for bt601
	ISP_WRITE( 0x2008, (IMAGE_HEIGHT<<16) | IMAGE_WIDTH);
	ISP_WRITE( 0x2010, 0x200020);
	ISP_WRITE( 0x2014, 0x200020);
	ISP_WRITE( 0x2018, 0x200020);
	ISP_WRITE( 0x2004, 0x1);
	ISP_WRITE( 0x2000, 0x11);
	ISP_WRITE( 0x004, 0x6);

	SIF_WRITE( 0x010, 0x20);
#endif

#if	0
	// set event reg data
	event.index		= 0;
	event.length	= sizeof(reg640x480_init);
	event.pbuf		= (uint32_t *)reg640x480_init;
	ioctl(gIspDev, ISP_IOCTL_SET_EVENTREG, &event);

	event.index		= 1;
	event.length	= sizeof(reg640x480_proc);
	event.pbuf		= (uint32_t *)reg640x480_proc;
	ioctl(gIspDev, ISP_IOCTL_SET_EVENTREG, &event);
#endif



	ISP_PRINTF("isp Init in\n");
	// DxO Part
	DxOISP_Init(uiFreq);

    //[[tifler
	//DRIVER_SET_INTMASK((0xff7e<<16)|0xffff);
	DRIVER_SET_INTMASK((0xff7e<<16)|0xfffb);    // enable only display
    //]]tifler

	ISP_WRITE(0x108, 0xff7e);	// int enable dxo only
    //[[tifler
	//ISP_WRITE(0x118, 0xffff);
	ISP_WRITE(0x118, 0xfffb);
    //]]tifler


	ISP_PRINTF("isp Init done\n");
	//DxOISP_CommandSet(DxOISP_CMD_OFFSET(stAsync.stSystem.uiSystemClock)           , sizeof(uiFreq), (uint8_t*)&uiFreq     ) ;
	DxOISP_CommandSet(DxOISP_CMD_OFFSET(stAsync.stSystem.usMinInterframeTime)     , sizeof(usTime), (uint8_t*)&usTime     ) ;

	stIspCmd.stSync.stControl.eMode                  = DxOISP_MODE_IDLE;
	stIspCmd.stSync.stControl.ucCalibrationSelection = 0;
	stIspCmd.stSync.stControl.ucSourceSelection      = 0;
	
	stIspCmd.stSync.stCameraControl.stAe.eMode	= DxOISP_AE_MODE_MANUAL;

	DxOISP_CommandGroupOpen();
	DxOISP_CommandSet(DxOISP_CMD_OFFSET(stSync.stControl.ucSourceSelection)       , sizeof(uint8_t), (uint8_t*)&stIspCmd.stSync.stControl.ucSourceSelection     );
	DxOISP_CommandSet(DxOISP_CMD_OFFSET(stSync.stControl.ucCalibrationSelection)  , sizeof(uint8_t), (uint8_t*)&stIspCmd.stSync.stControl.ucCalibrationSelection);
	DxOISP_CommandSet(DxOISP_CMD_OFFSET(stSync.stControl.eMode)                   , sizeof(uint8_t), (uint8_t*)&stIspCmd.stSync.stControl.eMode                 );
	DxOISP_CommandGroupClose();
	
	//DxOISP_Event();
	ISP_PRINTF("isp mode wait in\n");
	do {
		DxOISP_StatusGroupOpen() ;
		DxOISP_StatusGet(DxOISP_STATUS_OFFSET(stSync.stSystem.eMode), sizeof(ucMode), &ucMode) ;
		DxOISP_StatusGroupClose() ;
	} while(ucMode != DxOISP_MODE_IDLE) ;

	ISP_PRINTF("isp mode idle\n");

	//
	// Set preview VGA to VGA on output display
	//
	stIspCmd.stSync.stControl.ucInputDecimation                                   = 1;
	stIspCmd.stSync.stControl.eImageOrientation										= DxOISP_FLIP_OFF_MIRROR_OFF;
	stIspCmd.stSync.stControl.isTNREnabled											= 0;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].isEnabled           = 1;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].eFormat             = DxOISP_FORMAT_YUV422 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].eEncoding           = DxOISP_ENCODING_YUV_601FU ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].eOrder              = DxOISP_YUV422ORDER_YUYV ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].stCrop.usXAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].stCrop.usXAddrEnd   = IMAGE_WIDTH-1;//1919;//639 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].stCrop.usYAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].stCrop.usYAddrEnd   = IMAGE_HEIGHT-1;//1079;//479 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].usSizeX             = IMAGE_WIDTH;//1920;//640 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_DISPLAY].usSizeY             = IMAGE_HEIGHT;//1080;//480 ;
	stIspCmd.stSync.stControl.usFrameRate                                         = 8 << DxOISP_FPS_FRAC_PART ;
	
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].isEnabled           = 0;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].eFormat             = DxOISP_FORMAT_YUV422 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].eEncoding           = DxOISP_ENCODING_YUV_601FS ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].eOrder              = DxOISP_YUV422ORDER_YUYV ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].stCrop.usXAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].stCrop.usXAddrEnd   = IMAGE_WIDTH-1;//1919;//639 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].stCrop.usYAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].stCrop.usYAddrEnd   = IMAGE_HEIGHT-1;//1079;//479 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].usSizeX             = IMAGE_WIDTH;//1920;//640 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_CAPTURE].usSizeY             = IMAGE_HEIGHT;//1080;//480 ;

	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD     ].isEnabled           = 0;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].eFormat             = DxOISP_FORMAT_YUV422 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].eEncoding           = DxOISP_ENCODING_YUV_601FS ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].eOrder              = DxOISP_YUV422ORDER_YUYV ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].stCrop.usXAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].stCrop.usXAddrEnd   = IMAGE_WIDTH-1;//1919;//639 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].stCrop.usYAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].stCrop.usYAddrEnd   = IMAGE_HEIGHT-1;//1079;//479 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].usSizeX             = IMAGE_WIDTH;//1920;//640 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_FD].usSizeY             = IMAGE_HEIGHT;//1080;//480 ;

    //[[tifler]]
	//stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO  ].isEnabled           = 1;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO  ].isEnabled           = 0;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].eFormat             = DxOISP_FORMAT_YUV422 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].eEncoding           = DxOISP_ENCODING_YUV_601FU ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].eOrder              = DxOISP_YUV422ORDER_YUYV ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].stCrop.usXAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].stCrop.usXAddrEnd   = IMAGE_WIDTH-1;//1919;//639 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].stCrop.usYAddrStart = 0 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].stCrop.usYAddrEnd   = IMAGE_HEIGHT-1;//1079;//479 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].usSizeX             = IMAGE_WIDTH;//1920;//640 ;
	stIspCmd.stSync.stControl.stOutput[DxOISP_OUTPUT_VIDEO].usSizeY             = IMAGE_HEIGHT;//1080;//480 ;

	stIspCmd.stSync.stCameraControl.stAe.eMode	= DxOISP_AE_MODE_MANUAL;
	stIspCmd.stSync.stCameraControl.stAwb.eMode	= DxOISP_AWB_MODE_MANUAL;

	//
	// First go to preview
	//
	stIspCmd.stSync.stControl.eMode = DxOISP_MODE_PREVIEW ;
	DxOISP_CommandGroupOpen() ;
	DxOISP_CommandSet(DxOISP_CMD_OFFSET(stSync),sizeof(stIspCmd.stSync.stControl), &stIspCmd.stSync.stControl) ;
	DxOISP_CommandSet(DxOISP_CMD_OFFSET(stSync.stCameraControl),sizeof(stIspCmd.stSync.stCameraControl), &stIspCmd.stSync.stCameraControl) ;
	DxOISP_CommandGroupClose() ;

//	usleep(1000);
//	DxOISP_Event();
	ISP_PRINTF("isp mode wait in\n");
	do {
		DxOISP_StatusGroupOpen() ;
		DxOISP_StatusGet(DxOISP_STATUS_OFFSET(stSync.stSystem.eMode), sizeof(uint8_t), &ucMode) ;
		DxOISP_StatusGroupClose() ;
	} while(ucMode != stIspCmd.stSync.stControl.eMode );

	ISP_PRINTF("isp init done\n");

#ifndef	INPUT_SENSOR
	// for vsensor
	ISP_WRITE( 0x1000,1);     //ISP_VD_DP_IMG_SIZE
#endif
	
	initdone = 1;

	return 0;
}

int shell_isp_geter(char *data)
{
	uint32_t	cnt;
	uint8_t		ucmode;

	DxOISP_PixelStuckHandler();

	DxOISP_StatusGroupOpen() ;
	DxOISP_StatusGet(DxOISP_STATUS_OFFSET(stSync.stSystem.uiFrameCount), sizeof(uint32_t), &cnt) ;
	DxOISP_StatusGet(DxOISP_STATUS_OFFSET(stSync.stSystem.eMode), sizeof(uint8_t), &ucmode) ;
	DxOISP_StatusGroupClose() ;

	ISP_PRINTF("Frame Cnt 0x%x\n", cnt);
	ISP_PRINTF("Mode %d\n", ucmode);

	return 0;
}

extern int gIspDev;
int shell_isp_vsensorstart(char *data)
{
	//ISP_WRITE( 0x1000,4);     //ISP_VD_DP_IMG_SIZE
	//ISP_WRITE( 0x1000,1);     //ISP_VD_DP_IMG_SIZE
	//DxOISP_PostEvent();
	//ioctl(gIspDev, ISP_IOCTL_I2CM_SET_ADDR, NULL);


	ISP_WRITE( 0x2008, 0x1e00280);
	ISP_WRITE( 0x2010, 0x200020);
	ISP_WRITE( 0x2014, 0x200020);
	ISP_WRITE( 0x2018, 0x200020);
	ISP_WRITE( 0x2004, 0x1);
	ISP_WRITE( 0x2000, 0x11);
	ISP_WRITE( 0x004, 0x6);

	SIF_WRITE( 0x010, 0x20);

	ISP_PRINTF("grabber output enable\n");


	return 0;
}

/* EOF */

