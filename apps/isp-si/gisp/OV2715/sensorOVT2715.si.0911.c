// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "registerMapOVT2715.h"
#include "gisp-sensor.h"
#include "DxOISP_SensorAPI.h"
#include "gisp-ioctl.h"
#include "gisp-i2c.h"
#include "debug.h"

/*****************************************************************************/

#define	I2C_BUF_LENGTH	512
#define gI2C_buf                        (priv->i2cBuffer)

// XXX tifler
// Each sensor has slave address and can be accessed via unified i2c bus or
// individual one.
// So, we must distinguish access method by fd and slave address.
//
// Ex-1)
//  Front Sensor: i2c-0 attached, slave addr is 0x30
//  Back  Sensor: i2c-1 attached, slave addr is 0x40
// Ex-2)
//  Front Sensor: i2c-0 attached, slave addr is 0x30
//  Back  Sensor: i2c-0 attached, slave addr is 0x40
struct OVT2715Private {
    int fd;         // i2c master device
    int slaveAddr;  // i2c slave address for sensor.
    int inputMode;
    int inputModeDevice;
    char i2cBuffer[I2C_BUF_LENGTH];
};

// XXX We *MUST* remove this variable.!!!!
//     REENTRANCY is an art. But this variable heads REENTRANCY off.
static struct SENSOR *THIS_SENSOR;
static struct OVT2715Private defaultPrivate = {
    .inputMode = INPUTMODE_OV2715_640x480_6FPS,
    .inputModeDevice = INPUTMODE_OV2715,
};

/*****************************************************************************/

//extern int gIspDev;
//extern int priv->inputMode;	// For setting different input mode : OV2715, VIGEN, VSENSOR
//extern int priv->inputModeDevice;

//#define	I2C_BUF_LENGTH	512
//static char		gI2C_buf[I2C_BUF_LENGTH];


//#define FLASH_ENABLE   0
//#undef FLASH_ENABLE

//#define FORCE_FULL_FRAME

//#define I2C_DEBUG_ENABLE
//#define SENSOR_DEBUG_ENABLE

#define USE_SENSOR_GAIN
//#undef USE_SENSOR_GAIN  //use the real gain model

//del #define OV8820_DevAddr      (0x6c)
//del #define CCIC_TWSI_PORT_IDX      3

#ifndef NDEBUG
#define CHECK_SENSOR_BEFORE_START
#endif

#define SPECIAL_SENSOR_DEBUG
//#undef SPECIAL_SENSOR_DEBUG

#if defined(SPECIAL_SENSOR_DEBUG)
#define PRINTF     printf
#define PRINTF_I2C printf 
#else
#define PRINTF(...)
#define PRINTF_I2C(...)
#endif

#define I2C_SENSOR_ADDRESS                                              0x10
#define I2C_MASTER_INSTANCE_ID                                             0
#define I2C_MASTER_EXTENDED_ADDR                                           0
#define DXO_RAW_MULTIPLE                                                   8
#define DXOSENSOR_EXT_CLK                                                  4
#ifndef PLATFORM_CCIC_SENSOR_MCLK
#define PLATFORM_CCIC_SENSOR_MCLK                                         27
#endif
#define EXPO_TIME_FRAC_PART                                               16
#define CLOCK_DIV_VALUE                                                  100
#define GAIN_FRAC_PART                                                     8
#define NB_MIPI_LANES                                                      2
#define NB_FRAME_TO_IGNORE                                                 2

#define ov2715_write(a,b)                writeDevice((a),(b),1)
#define ov2715_write2(a,b)                writeDevice2((a),(b),1)
#define ov2715_read(a)                   readDevice( (a), 1 )
#define WRITE_BY_CACHE(offset,value)     writeCacheDevice ( offset, value, Size_##offset, cache_##offset  )
#define READ_BY_CACHE(offset)            readCacheDevice  ( offset       , Size_##offset, cache_##offset  )  
#define WRITE_DEVICE(offset,value)       writeDevice      ( offset, value, Size_##offset  )
#define READ_DEVICE(offset)              readDevice       ( offset       , Size_##offset  )


#define SHIFT_RND(x,n)                    (((x)+((n)!=0?1<<((n)-1):0) )>>(n))
#define DIV_UP(x,y)                       (((x)+(y)-1)/(y))
#define MIN(x,y)                          ((x)<(y)?(x):(y))
#define MAX(x,y)                          ((x)>(y)?(x):(y))

/*****************************************************************************/
/* types definitons                                                          */
/*****************************************************************************/
typedef struct {
	uint16_t     usAF                   ;
	uint16_t     usAG                   ;
	uint16_t     usDG[DxOISP_NB_CHANNEL];
	uint16_t     usYstart               ;
	uint16_t     usYstop                ;
	uint16_t     usXstart               ;
	uint16_t     usXstop                ;
	uint8_t      ucIncX                 ;
	uint8_t      ucIncY                 ;
	uint16_t     usOutSizeX             ;
	uint16_t     usOutSizeY             ;
	uint16_t     usOffsetH              ;
	uint16_t     usOffsetV              ;
	uint16_t     usLineLength           ;
	uint16_t     usFrameLength          ;
	uint32_t     uiExpoTime             ;
	uint8_t      ucTimingRegX           ;
	uint8_t      ucTimingRegY           ;
	uint8_t      ucPsRamCtrl0           ;
	uint8_t      isStreaming            ;
} ts_sensorRegister;

typedef struct {
	uint8_t ucX;
	uint8_t ucY;
} ts_Decim;

#define NO_ACCESS       -1
#define RW_ACCESS        0
#define SIZE_CACHE_DATA     (INTEGRATION_TIME_REG \
                          + ANALOG_GAIN_REG      \
                          + DIGITAL_GAIN_REG     \
                          + GEOMETRY_REG         \
                          + FRAME_RATE_REG       \
                          + MODE_SELECT_REG      \
                          + AF_REG               \
                          + CROP_REG             \
                          + TIMING_REG)

typedef struct {
	int32_t    iLastAccess;
	uint16_t   usData;
} ts_cache;

/*****************************************************************************/
/* static variables definitons                                               */
/*****************************************************************************/
static ts_SENSOR_Cmd       S_stCmd                     ;
static ts_SENSOR_Status    S_stStatus                  ;
static ts_sensorRegister   S_stRegister                ; // structure accessed by the set and get function
static uint32_t            S_uiSensorPixelClock        ;
static ts_cache            S_cache[SIZE_CACHE_DATA]    ;
static uint32_t            S_uiAppliedIT;
static uint16_t            S_usAppliedAG;
static uint16_t            S_usAppliedDG;
static uint8_t             S_ucCropConfigIdx;
static uint8_t             S_ucAppliedCropConfigIdx;
static uint8_t             S_isStartIgnoringFrame;
static uint32_t            S_uiOldFrameCount;
static uint32_t            S_uiCmdSendToFrameIdx;
#ifdef FLASH_ENABLE
static uint16_t            S_usFlashPower;
#endif

/*****************************************************************************/
/* Static functions prototype declaration                                    */
/*****************************************************************************/
static uint32_t        I2C_Write                    ( uint16_t usOffset, uint8_t* ptucData, uint8_t ucSize );
static uint32_t        I2C_Read                     ( uint16_t usOffset, uint8_t* ptucData, uint8_t ucSize );
static void            writeDevice                  ( uint16_t offset, uint16_t usValue, uint8_t size                );
static void            writeDevice2                  ( uint16_t offset, uint16_t usValue, uint8_t size                );
static void            writeCacheDevice             ( uint16_t offset, uint16_t usValue, uint8_t size, uint8_t ucIdx );
static uint16_t        readDevice                   ( uint16_t offset, uint8_t  size                                 );
static uint16_t        readCacheDevice              ( uint16_t offset, uint8_t  size                 , uint8_t ucIdx );
static inline void     initSensorCmd                ( ts_SENSOR_Cmd* pttsCmd, ts_SENSOR_Status* pttsStatus );
static inline void     setStaticSensorConfiguration ( int sel                                              );
static uint32_t        computePixelClock            ( void                                                 );
static inline void     applySettingsToSensor        ( void                                                 );
static void            setFlash                     ( uint16_t usPower                                     );
#if defined(CHECK_SENSOR_BEFORE_START)
static inline void     autoCheckPluggedSensor       ( void                                                 );
#endif

static inline void     initCache                    ( void                                                 );
static inline void     historyMngt                  ( uint8_t ucMode                                       ); 


//STATUS
static inline void     getCapabilities              ( ts_SENSOR_Status*        pttsStatus                  );
static inline void     getDigitalGain               ( ts_SENSOR_Status*        pttsStatus                  );
static inline void     getAnalogGain                ( ts_SENSOR_Status*        pttsStatus                  );
static inline void     getImageFeature              ( ts_SENSOR_Status*        pttsStatus                  );
static inline void     getFrameRate                 ( ts_SENSOR_Status*        pttsStatus                  );
static void            setExposureTime              ( ts_sensorRegister*       pstRegister                 );
static inline void     getExposureTime              ( ts_SENSOR_Status*        pttsStatus                  );
static inline void     getMode                      ( ts_SENSOR_Status*        pttsStatus                  );
static inline void     getPixelClock                ( ts_SENSOR_Status*        pttsStatus                  );
static inline void     getFrameValidTime            ( ts_SENSOR_Status*        ptttStatus                  );

//CMD
static void            setGain                      ( ts_sensorRegister*       pstRegister, uint16_t usAGain[] , uint16_t usDGain[]    );
static void            setExposureTime              ( ts_sensorRegister*       pstRegister                                             );
static void            setCrop                      ( ts_sensorRegister*       pstRegister                                             );
static void            setOrientation               ( ts_sensorRegister*       pstRegister, uint8_t  ucOrientation                     );
static inline void     setFrameRate                 ( ts_sensorRegister*       pstRegister, uint16_t usFrameRate                       );
static inline void     setBinning                   ( ts_sensorRegister*       pstRegister, uint8_t  isBinning                         );
static inline ts_Decim setSkipping                  ( ts_sensorRegister*       pstRegister, uint8_t ucSkipX, uint8_t ucSkipY           );
static void            setDecimation                ( ts_sensorRegister*       pstRegister, uint8_t ucDecimX,uint8_t ucDecimY          );
static inline void     setOutputSize                ( ts_sensorRegister*       pstRegister, ts_Decim stSkipping                        );
static inline void     setMode                      ( ts_sensorRegister*       pstRegister, uint8_t                  isStreaming       );
static void            setMinimalBlanking           ( ts_sensorRegister*       pstRegister, uint16_t usMinVertBlk, uint32_t usMinHorBlk);

/*****************************************************************************/
/* Static functions definition                                               */
/*****************************************************************************/
static void assertf0(int a, char *p)
{
	if(a == 0)
	{
		printf ("%s", p) ;
		assert(0);
	}
}

static void assertf1(int a, char *p, unsigned int i)
{
	if(a == 0)
	{
		printf ("%s, %d",p, i) ;
		assert(0);
	}
}

static void assertf2(int a, char *p, unsigned int i, unsigned int j)
{
	if(a == 0)
	{
		printf ("%s, %d, %d",p, i, j) ;
		assert(0);
	}
}


static uint32_t I2C_Write ( uint16_t usOffset, uint8_t* ptucData, uint8_t ucSize ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	if(priv->inputModeDevice == INPUTMODE_OV2715)
	{
		ioctl_i2cm_info	*pi2cm_info;

		if(ucSize >= (I2C_BUF_LENGTH-sizeof(ioctl_i2cm_info)))
		{
			printf("i2c write data overflow %d\n", ucSize);
			return 1;
		}

		pi2cm_info = (ioctl_i2cm_info *)gI2C_buf;

		pi2cm_info->id		= 0x6c;
		pi2cm_info->addr	= usOffset;
		pi2cm_info->length	= ucSize;

		memcpy(&gI2C_buf[sizeof(ioctl_i2cm_info)], ptucData, ucSize);

		I2C_WRITE(gI2C_buf);
	}
	return 0;
}

static uint32_t I2C_Read ( uint16_t usOffset,uint8_t* ptucData, uint8_t ucSize ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	if(priv->inputModeDevice == INPUTMODE_OV2715)
	{
		ioctl_i2cm_info	*pi2cm_info;

		if(ucSize >= (I2C_BUF_LENGTH-sizeof(ioctl_i2cm_info)))
		{
			printf("i2c read data overflow %d\n", ucSize);
			return 1;
		}

		pi2cm_info = (ioctl_i2cm_info *)gI2C_buf;

		pi2cm_info->id		= 0x6c;
		pi2cm_info->addr	= usOffset;
		pi2cm_info->length	= ucSize;

	//	memcpy(ptucData, &gI2C_buf[sizeof(ioctl_i2cm_info)], ucSize);

		I2C_READ(gI2C_buf);

		memcpy(ptucData, &gI2C_buf[sizeof(ioctl_i2cm_info)], ucSize);
	}
	return 0;

}


static void writeDevice( uint16_t usOffset, uint16_t usValue, uint8_t ucSize ) {
	uint32_t uiError;
	uint8_t  ptucData[2];

	switch(ucSize) {
		case 1:
			ptucData[0] =             (uint8_t)usValue;   
			ptucData[1] =                            0;
			break;
		case 2:
			ptucData[0] = (( usValue & 0xff00 ) >> 8 );
			ptucData[1] = (  usValue & 0x00ff        );
			break;
		default:
			assertf1(0,"Illegal length = %d", ucSize );
			break;
	}
	uiError = I2C_Write( usOffset, ptucData, ucSize );
	
	#ifdef I2C_DEBUG_ENABLE
		if(ucSize == 1) {
			PRINTF_I2C("0x%04X=0x%02X\n", usOffset, ptucData[0]);
		}	
		else {
			PRINTF_I2C("0x%04X=0x%02X\n", usOffset, ptucData[0]);
			PRINTF_I2C("0x%04X=0x%02X\n", usOffset+1, ptucData[1]);
		}	

	#endif
	
	assertf1(!uiError,"write device error (err=%lu)", uiError );
}

static void writeDevice2( uint16_t usOffset, uint16_t usValue, uint8_t ucSize ) {
	uint32_t uiError;
	uint8_t  ptucData[2], rdbuf[2];

	switch(ucSize) {
		case 1:
			ptucData[0] =             (uint8_t)usValue;   
			ptucData[1] =                            0;
			break;
		case 2:
			ptucData[0] = (( usValue & 0xff00 ) >> 8 );
			ptucData[1] = (  usValue & 0x00ff        );
			break;
		default:
			assertf1(0,"Illegal length = %d", ucSize );
			break;
	}
	uiError = I2C_Write( usOffset, ptucData, ucSize );

	while(1)
	{
		I2C_Read( usOffset, rdbuf, ucSize);
		if(ucSize==1)
		{
			if(ptucData[0]!=rdbuf[0])I2C_Write( usOffset, ptucData, ucSize);
			if(ptucData[0]==rdbuf[0])break;
		}
		else if(ucSize==2)
		{
			if((ptucData[0]!=rdbuf[0])||(ptucData[1]!=rdbuf[1]))I2C_Write(usOffset, ptucData, ucSize);
			if((ptucData[0]==rdbuf[0])&&(ptucData[1]!=rdbuf[1]))break;
		}
	}
}


static uint16_t readDevice( uint16_t usOffset, uint8_t ucSize ) {
	uint32_t uiError;
	uint16_t usValue = 0;
	uint8_t  ptucData[2];

	uiError = I2C_Read( usOffset, ptucData, ucSize );
	#ifdef I2C_DEBUG_ENABLE
		PRINTF_I2C("I2C READ: [0x%04X]:[%02X|%02X]\n", usOffset, ucSize==1?0x00:ptucData[0], ucSize==1?ptucData[0]:ptucData[1]); 
	#endif
	assertf2(!uiError,"reading device error (err=%lu (0x%08x))", uiError , uiError ); 

	switch(ucSize) {
		case 1:
			usValue = ptucData[0];
			break;
		case 2:
			usValue  = (ptucData[0]      << 8);
			usValue |= (ptucData[1] & 0x00ff ); 
			break;
		default:
			assertf1(0,"Illegal length = %d", ucSize );
			break;
	}
	return usValue;
}


static void writeCacheDevice( uint16_t usOffset, uint16_t usValue, uint8_t ucSize, uint8_t ucIdx ) {
	switch (S_cache[ucIdx].iLastAccess) {
		case NO_ACCESS:
			writeDevice(usOffset,usValue,ucSize);
			S_cache[ucIdx].usData = usValue;
			S_cache[ucIdx].iLastAccess = RW_ACCESS;
			break;
		case RW_ACCESS:
			if ( usValue != S_cache[ucIdx].usData ) {
				writeDevice(usOffset,usValue,ucSize);
				S_cache[ucIdx].usData = usValue;
			}
			break;
		default:
			assertf2(0,"unexpected access status [%d]=%lu", ucIdx, S_cache[ucIdx].iLastAccess ); 
			break;
	}
}


static uint16_t readCacheDevice( uint16_t usOffset, uint8_t ucSize, uint8_t ucIdx ) {
	uint16_t usTmp;
	switch (S_cache[ucIdx].iLastAccess) {
		case NO_ACCESS:
			usTmp = readDevice(usOffset,ucSize);
			S_cache[ucIdx].usData = usTmp;
		case RW_ACCESS:
			S_cache[ucIdx].iLastAccess = RW_ACCESS;
			break;
		default:
			assertf2(0,"unexpected access status [%d]=%lu", ucIdx, S_cache[ucIdx].iLastAccess ); 
			break;
	}
	return S_cache[ucIdx].usData;
}


static inline void getCapabilities( ts_SENSOR_Status* pttsStatus ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	switch(priv->inputMode)
	{

		case INPUTMODE_OV2715_640x480_6FPS :
		case INPUTMODE_OV2715_640x480_8FPS :

			pttsStatus->stCapabilities.stImageFeature.ucMaxHorDownsamplingFactor  = 1; //Only binning is used as the frame rate is enough
			pttsStatus->stCapabilities.stImageFeature.ucMaxVertDownsamplingFactor = 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerWidth                = 640>>1;	//p X_OUTPUT_SIZE_MAX   >> 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerHeight               = 480>>1;   //p Y_OUTPUT_SIZE_MAX   >> 1;
			pttsStatus->stCapabilities.stShootingParam.usMaxAGain                 = 16<<GAIN_FRAC_PART;	//p 62<<GAIN_FRAC_PART;
			pttsStatus->stCapabilities.stShootingParam.usIso                      = 64;
			pttsStatus->stCapabilities.stShootingParam.usMaxDGain                 = (0xfff<<GAIN_FRAC_PART)/0x400;
			pttsStatus->stCapabilities.stImageFeature.usMinHorBlanking            = MIN_LINE_BLANKING_PCK;  
			pttsStatus->stCapabilities.stImageFeature.usMinVertBlanking           = MIN_FRAME_BLANKING_LINE;  
			pttsStatus->stCapabilities.stImageFeature.ucPixelWidth                = 10;
			pttsStatus->stCapabilities.stImageFeature.ePhase                      = DxOISP_SENSOR_PHASE_GRBG;
			pttsStatus->stCapabilities.stImageFeature.eNaturalOrientation         = DxOISP_FLIP_OFF_MIRROR_OFF;
			pttsStatus->stCapabilities.stImageFeature.isPhaseStatic               = 1;
			pttsStatus->stCapabilities.stImageFeature.ucMaxDelayImageFeature      = NB_FRAME_TO_IGNORE;
			pttsStatus->stCapabilities.stImageFeature.ucNbUpperDummyLines         = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLeftDummyColumns        = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLowerDummyLines         = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbRightDummyColumns       = 0;
			pttsStatus->usPedestal                                                = 0;
			pttsStatus->stCapabilities.stAf.usMin                                 = MIN_AF_POSITION;
			pttsStatus->stCapabilities.stAf.usMax                                 = MAX_AF_POSITION;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMinDefocus                = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMidDefocus                = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMaxDefocus                = 0;
			pttsStatus->stCapabilities.stAperture.usMin                           =
			pttsStatus->stCapabilities.stAperture.usMax                           = 2007;
			pttsStatus->stCapabilities.stZoomFocal.usMin                          = 
			pttsStatus->stCapabilities.stZoomFocal.usMax                          = 0; 
			pttsStatus->stCapabilities.stFlash.usMaxPower                         = 0;
			pttsStatus->stCapabilities.enableInterframeCmdOnly                    = 0;
			pttsStatus->isImageQualityStable                                      = 1;
			pttsStatus->isFrameInvalid                                            = 0;
			pttsStatus->stAppliedCmd.usAperture                                   = 2007; //Fixed aperture
			pttsStatus->eCurrentPhase                                             = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_OV2715_1920x1080_10FPS:

			pttsStatus->stCapabilities.stImageFeature.ucMaxHorDownsamplingFactor  = 1; //Only binning is used as the frame rate is enough
			pttsStatus->stCapabilities.stImageFeature.ucMaxVertDownsamplingFactor = 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerWidth				  = 1920>>1;	//p X_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerHeight 			  = 1080>>1;	//p Y_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stShootingParam.usMaxAGain				  = 16<<GAIN_FRAC_PART; //p 62<<GAIN_FRAC_PART;
			pttsStatus->stCapabilities.stShootingParam.usIso					  = 64;
			pttsStatus->stCapabilities.stShootingParam.usMaxDGain				  = (0xfff<<GAIN_FRAC_PART)/0x400;
			pttsStatus->stCapabilities.stImageFeature.usMinHorBlanking			  = MIN_LINE_BLANKING_PCK;	
			pttsStatus->stCapabilities.stImageFeature.usMinVertBlanking 		  = MIN_FRAME_BLANKING_LINE;  
			pttsStatus->stCapabilities.stImageFeature.ucPixelWidth				  = 10;
			pttsStatus->stCapabilities.stImageFeature.ePhase					  = DxOISP_SENSOR_PHASE_GRBG;
			pttsStatus->stCapabilities.stImageFeature.eNaturalOrientation		  = DxOISP_FLIP_OFF_MIRROR_OFF;
			pttsStatus->stCapabilities.stImageFeature.isPhaseStatic 			  = 1;
			pttsStatus->stCapabilities.stImageFeature.ucMaxDelayImageFeature	  = NB_FRAME_TO_IGNORE;
			pttsStatus->stCapabilities.stImageFeature.ucNbUpperDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLeftDummyColumns		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLowerDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbRightDummyColumns 	  = 0;
			pttsStatus->usPedestal												  = 0;
			pttsStatus->stCapabilities.stAf.usMin								  = MIN_AF_POSITION;
			pttsStatus->stCapabilities.stAf.usMax								  = MAX_AF_POSITION;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMinDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMidDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMaxDefocus				  = 0;
			pttsStatus->stCapabilities.stAperture.usMin 						  =
			pttsStatus->stCapabilities.stAperture.usMax 						  = 2007;
			pttsStatus->stCapabilities.stZoomFocal.usMin						  = 
			pttsStatus->stCapabilities.stZoomFocal.usMax						  = 0; 
			pttsStatus->stCapabilities.stFlash.usMaxPower						  = 0;
			pttsStatus->stCapabilities.enableInterframeCmdOnly					  = 0;
			pttsStatus->isImageQualityStable									  = 1;
			pttsStatus->isFrameInvalid											  = 0;
			pttsStatus->stAppliedCmd.usAperture 								  = 2007; //Fixed aperture
			pttsStatus->eCurrentPhase											  = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_VIGEN_640x480:

			pttsStatus->stCapabilities.stImageFeature.ucMaxHorDownsamplingFactor  = 1; //Only binning is used as the frame rate is enough
			pttsStatus->stCapabilities.stImageFeature.ucMaxVertDownsamplingFactor = 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerWidth				  = 640>>1;	//p X_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerHeight 			  = 480>>1;	//p Y_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stShootingParam.usMaxAGain				  = 1<<GAIN_FRAC_PART; //p 62<<GAIN_FRAC_PART;
			pttsStatus->stCapabilities.stShootingParam.usIso					  = 100;
			pttsStatus->stCapabilities.stShootingParam.usMaxDGain				  = (0xfff<<GAIN_FRAC_PART)/0x400;
			pttsStatus->stCapabilities.stImageFeature.usMinHorBlanking			  = 100;	
			pttsStatus->stCapabilities.stImageFeature.usMinVertBlanking 		  = 10;  
			pttsStatus->stCapabilities.stImageFeature.ucPixelWidth				  = 10;
			pttsStatus->stCapabilities.stImageFeature.ePhase					  = DxOISP_SENSOR_PHASE_GRBG;
			pttsStatus->stCapabilities.stImageFeature.eNaturalOrientation		  = DxOISP_FLIP_OFF_MIRROR_OFF;
			pttsStatus->stCapabilities.stImageFeature.isPhaseStatic 			  = 1;
			pttsStatus->stCapabilities.stImageFeature.ucMaxDelayImageFeature	  = NB_FRAME_TO_IGNORE;
			pttsStatus->stCapabilities.stImageFeature.ucNbUpperDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLeftDummyColumns		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLowerDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbRightDummyColumns 	  = 0;
			pttsStatus->usPedestal												  = 0;
			pttsStatus->stCapabilities.stAf.usMin								  = 0;
			pttsStatus->stCapabilities.stAf.usMax								  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMinDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMidDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMaxDefocus				  = 0;
			pttsStatus->stCapabilities.stAperture.usMin 						  =
			pttsStatus->stCapabilities.stAperture.usMax 						  = 0;
			pttsStatus->stCapabilities.stZoomFocal.usMin						  = 
			pttsStatus->stCapabilities.stZoomFocal.usMax						  = 0; 
			pttsStatus->stCapabilities.stFlash.usMaxPower						  = 0;
			pttsStatus->stCapabilities.enableInterframeCmdOnly					  = 0;
			pttsStatus->isImageQualityStable									  = 1;
			pttsStatus->isFrameInvalid											  = 0;
			pttsStatus->stAppliedCmd.usAperture 								  = 0; //Fixed aperture
			pttsStatus->eCurrentPhase											  = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_VIGEN_1920x1080:

			pttsStatus->stCapabilities.stImageFeature.ucMaxHorDownsamplingFactor  = 1; //Only binning is used as the frame rate is enough
			pttsStatus->stCapabilities.stImageFeature.ucMaxVertDownsamplingFactor = 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerWidth				  = 1920>>1;	//p X_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerHeight 			  = 1080>>1;	//p Y_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stShootingParam.usMaxAGain				  = 1<<GAIN_FRAC_PART; //p 62<<GAIN_FRAC_PART;
			pttsStatus->stCapabilities.stShootingParam.usIso					  = 100;
			pttsStatus->stCapabilities.stShootingParam.usMaxDGain				  = (0xfff<<GAIN_FRAC_PART)/0x400;
			pttsStatus->stCapabilities.stImageFeature.usMinHorBlanking			  = 100;	
			pttsStatus->stCapabilities.stImageFeature.usMinVertBlanking 		  = 10;  
			pttsStatus->stCapabilities.stImageFeature.ucPixelWidth				  = 10;
			pttsStatus->stCapabilities.stImageFeature.ePhase					  = DxOISP_SENSOR_PHASE_GRBG;
			pttsStatus->stCapabilities.stImageFeature.eNaturalOrientation		  = DxOISP_FLIP_OFF_MIRROR_OFF;
			pttsStatus->stCapabilities.stImageFeature.isPhaseStatic 			  = 1;
			pttsStatus->stCapabilities.stImageFeature.ucMaxDelayImageFeature	  = NB_FRAME_TO_IGNORE;
			pttsStatus->stCapabilities.stImageFeature.ucNbUpperDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLeftDummyColumns		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLowerDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbRightDummyColumns 	  = 0;
			pttsStatus->usPedestal												  = 0;
			pttsStatus->stCapabilities.stAf.usMin								  = 0;
			pttsStatus->stCapabilities.stAf.usMax								  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMinDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMidDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMaxDefocus				  = 0;
			pttsStatus->stCapabilities.stAperture.usMin 						  =
			pttsStatus->stCapabilities.stAperture.usMax 						  = 0;
			pttsStatus->stCapabilities.stZoomFocal.usMin						  = 
			pttsStatus->stCapabilities.stZoomFocal.usMax						  = 0; 
			pttsStatus->stCapabilities.stFlash.usMaxPower						  = 0;
			pttsStatus->stCapabilities.enableInterframeCmdOnly					  = 0;
			pttsStatus->isImageQualityStable									  = 1;
			pttsStatus->isFrameInvalid											  = 0;
			pttsStatus->stAppliedCmd.usAperture 								  = 2007; //Fixed aperture
			pttsStatus->eCurrentPhase											  = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_VIGEN_4208x3120:

			pttsStatus->stCapabilities.stImageFeature.ucMaxHorDownsamplingFactor  = 1; //Only binning is used as the frame rate is enough
			pttsStatus->stCapabilities.stImageFeature.ucMaxVertDownsamplingFactor = 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerWidth				  = 4208>>1;	//p X_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerHeight 			  = 3120>>1;	//p Y_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stShootingParam.usMaxAGain				  = 1<<GAIN_FRAC_PART; //p 62<<GAIN_FRAC_PART;
			pttsStatus->stCapabilities.stShootingParam.usIso					  = 100;
			pttsStatus->stCapabilities.stShootingParam.usMaxDGain				  = (0xfff<<GAIN_FRAC_PART)/0x400;
			pttsStatus->stCapabilities.stImageFeature.usMinHorBlanking			  = 100;	
			pttsStatus->stCapabilities.stImageFeature.usMinVertBlanking 		  = 10;  
			pttsStatus->stCapabilities.stImageFeature.ucPixelWidth				  = 10;
			pttsStatus->stCapabilities.stImageFeature.ePhase					  = DxOISP_SENSOR_PHASE_RGGB;//DxOISP_SENSOR_PHASE_GRBG;
			pttsStatus->stCapabilities.stImageFeature.eNaturalOrientation		  = DxOISP_FLIP_OFF_MIRROR_OFF;
			pttsStatus->stCapabilities.stImageFeature.isPhaseStatic 			  = 1;
			pttsStatus->stCapabilities.stImageFeature.ucMaxDelayImageFeature	  = NB_FRAME_TO_IGNORE;
			pttsStatus->stCapabilities.stImageFeature.ucNbUpperDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLeftDummyColumns		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLowerDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbRightDummyColumns 	  = 0;
			pttsStatus->usPedestal												  = 0;
			pttsStatus->stCapabilities.stAf.usMin								  = 0;
			pttsStatus->stCapabilities.stAf.usMax								  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMinDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMidDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMaxDefocus				  = 0;
			pttsStatus->stCapabilities.stAperture.usMin 						  =
			pttsStatus->stCapabilities.stAperture.usMax 						  = 0;
			pttsStatus->stCapabilities.stZoomFocal.usMin						  = 
			pttsStatus->stCapabilities.stZoomFocal.usMax						  = 0; 
			pttsStatus->stCapabilities.stFlash.usMaxPower						  = 0;
			pttsStatus->stCapabilities.enableInterframeCmdOnly					  = 0;
			pttsStatus->isImageQualityStable									  = 1;
			pttsStatus->isFrameInvalid											  = 0;
			pttsStatus->stAppliedCmd.usAperture 								  = 0; //Fixed aperture
			pttsStatus->eCurrentPhase											  = DxOISP_SENSOR_PHASE_RGGB;

		break;

		case INPUTMODE_V_SENSOR :

			pttsStatus->stCapabilities.stImageFeature.ucMaxHorDownsamplingFactor  = 1; //Only binning is used as the frame rate is enough
			pttsStatus->stCapabilities.stImageFeature.ucMaxVertDownsamplingFactor = 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerWidth				  = 4208>>1;	//p X_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stImageFeature.usBayerHeight 			  = 3120>>1;	//p Y_OUTPUT_SIZE_MAX	>> 1;
			pttsStatus->stCapabilities.stShootingParam.usMaxAGain				  = 1<<GAIN_FRAC_PART; //p 62<<GAIN_FRAC_PART;
			pttsStatus->stCapabilities.stShootingParam.usIso					  = 100;
			pttsStatus->stCapabilities.stShootingParam.usMaxDGain				  = (0xfff<<GAIN_FRAC_PART)/0x400;
			pttsStatus->stCapabilities.stImageFeature.usMinHorBlanking			  = 100;	
			pttsStatus->stCapabilities.stImageFeature.usMinVertBlanking 		  = 10;  
			pttsStatus->stCapabilities.stImageFeature.ucPixelWidth				  = 10;
			pttsStatus->stCapabilities.stImageFeature.ePhase					  = DxOISP_SENSOR_PHASE_GRBG;
			pttsStatus->stCapabilities.stImageFeature.eNaturalOrientation		  = DxOISP_FLIP_OFF_MIRROR_OFF;
			pttsStatus->stCapabilities.stImageFeature.isPhaseStatic 			  = 1;
			pttsStatus->stCapabilities.stImageFeature.ucMaxDelayImageFeature	  = NB_FRAME_TO_IGNORE;
			pttsStatus->stCapabilities.stImageFeature.ucNbUpperDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLeftDummyColumns		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbLowerDummyLines		  = 0;
			pttsStatus->stCapabilities.stImageFeature.ucNbRightDummyColumns 	  = 0;
			pttsStatus->usPedestal												  = 0;
			pttsStatus->stCapabilities.stAf.usMin								  = 0;
			pttsStatus->stCapabilities.stAf.usMax								  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMinDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMidDefocus				  = 0;
			pttsStatus->stCapabilities.stAf.stPerUnit.usMaxDefocus				  = 0;
			pttsStatus->stCapabilities.stAperture.usMin 						  =
			pttsStatus->stCapabilities.stAperture.usMax 						  = 0;
			pttsStatus->stCapabilities.stZoomFocal.usMin						  = 
			pttsStatus->stCapabilities.stZoomFocal.usMax						  = 0; 
			pttsStatus->stCapabilities.stFlash.usMaxPower						  = 0;
			pttsStatus->stCapabilities.enableInterframeCmdOnly					  = 0;
			pttsStatus->isImageQualityStable									  = 1;
			pttsStatus->isFrameInvalid											  = 0;
			pttsStatus->stAppliedCmd.usAperture 								  = 0; //Fixed aperture
			pttsStatus->eCurrentPhase											  = DxOISP_SENSOR_PHASE_GRBG;

		break;

		default :

		break;

	}
		
}


static inline void getDigitalGain( ts_SENSOR_Status* pttsStatus ) {
	pttsStatus->stAppliedCmd.stShootingParam.usDGain[DxOISP_CHANNEL_GB] =
	pttsStatus->stAppliedCmd.stShootingParam.usDGain[DxOISP_CHANNEL_GR] = 
	pttsStatus->stAppliedCmd.stShootingParam.usDGain[DxOISP_CHANNEL_R ] = 
	pttsStatus->stAppliedCmd.stShootingParam.usDGain[DxOISP_CHANNEL_B ] = S_usAppliedDG >> 2;
}

#define BIT(w)   (1U<<(w))
// provide a mask of w bits. Does not generate warning with w from 0 to 32.
#define MSK(w)   ( ( (w)>>5 ? 0U : BIT((w)&0x1f) ) - 1 )

static void setGain(ts_sensorRegister* pstRegister, uint16_t usAGain[], uint16_t usDGain[]) 
{

	int i;
	unsigned int uiMult2 = 0;
	uint32_t uiTotalGain[DxOISP_NB_CHANNEL];
	uint32_t uiTotalGainMin = 0xffffffff;
	uint16_t usAgain;
	uint16_t usGainTmp;
	uint32_t uiInvGain;
	uint16_t usAgainA0 = 0;	//p..
	uint16_t usAgainB7 = 0;
	uint16_t usAgainB6 = 0;
	uint16_t usAgainB5 = 0;
	uint16_t usAgainB4 = 0;
	uint16_t usAgainB3_0 = 0;	//..p


	for (i=0; i<DxOISP_NB_CHANNEL; i++) {
		uiTotalGain[i] = (usAGain[i] * usDGain[i]) >> GAIN_FRAC_PART;
		uiTotalGainMin = MIN(uiTotalGain[i], uiTotalGainMin);
	}
	
	//Compute Analog gain
	#ifdef USE_SENSOR_GAIN
		usGainTmp = uiTotalGainMin>>4;
		while(usGainTmp > 31) {
			uiMult2++;
			usGainTmp>>=1;
		}
		if(uiMult2>5) { //Saturation to max analog gain
			uiMult2   = 5;
			usGainTmp = 31;
		}
		pstRegister->usAG = (MSK(uiMult2) << 4) + (usGainTmp-16);
		
		//compute real analog gain applied
		usAgain = ((1<<uiMult2) * usGainTmp)<<4; //8.8
		
	#else
		pstRegister->usAG = MIN(uiTotalGainMin>>4, 0x3FF);
		usAgain = pstRegister->usAG << 4;
	#endif

		uiInvGain = (1 << (GAIN_FRAC_PART * 2)) / usAgain;
		for (i=0; i<DxOISP_NB_CHANNEL; i++) {
			pstRegister->usDG[i] = SHIFT_RND(uiTotalGain[i] * uiInvGain, (GAIN_FRAC_PART-2));
		}

}

static inline void getAnalogGain( ts_SENSOR_Status* pttsStatus ) {


	unsigned int i;
	uint16_t usReg = S_usAppliedAG;
	uint16_t usAgainA0 = 0;	//p..
	uint16_t usAgainB7 = 0;
	uint16_t usAgainB6 = 0;
	uint16_t usAgainB5 = 0;
	uint16_t usAgainB4 = 0;
	uint16_t usAgainB3_0 = 0;	//..p
	
	#ifdef USE_SENSOR_GAIN
		#if 0
		uint8_t ucBitField = (uint8_t)(usReg>>4);
		uint8_t ucMult    = 0;
		for(i=0;i<5;i++) {
			if(ucBitField&(1<<i)) ucMult++;
		}
		
		if(ucMult==0)
			ucMult = 1;
		else
			ucMult = 1<<ucMult;
			
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_GR] = 
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_B]  = 
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_R]  =
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_GB] = (((usReg&0x000f) + 16) *  ucMult)<<4;
		#endif
		usAgainA0 = (usReg) >> 8;
		usAgainB7 = ((usReg)&0x00FF) >> 7;
		usAgainB6 = ((usReg)&0x007F) >> 6;
		usAgainB5 = ((usReg)&0x003F) >> 5;
		usAgainB4 = ((usReg)&0x001F) >> 4;
		usAgainB3_0 = (usReg)&0x000F;

		usReg = (usAgainA0+1)*(usAgainB7+1)*(usAgainB6+1)*(usAgainB5+1)*(usAgainB4+1)*(usAgainB3_0/16+1);

		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_GR] = 
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_B]  = 
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_R]  =
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_GB] = usReg;
	#else

		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_GR] = 
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_B]  = 
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_R]  =
		pttsStatus->stAppliedCmd.stShootingParam.usAGain[DxOISP_CHANNEL_GB] = usReg<<4;
	#endif

}


static void setCrop( ts_sensorRegister* pstRegister ) {
	assertf1( S_stCmd.stImageFeature.usYBayerEnd < S_stStatus.stCapabilities.stImageFeature.usBayerHeight
	        , "usYBayerEnd = %d", S_stCmd.stImageFeature.usYBayerEnd );
	assertf1( S_stCmd.stImageFeature.usXBayerEnd < S_stStatus.stCapabilities.stImageFeature.usBayerWidth
	        , "usXBayerEnd = %d", S_stCmd.stImageFeature.usXBayerEnd );

	pstRegister->usYstop   = S_stCmd.stImageFeature.usYBayerEnd   * 2 + 1;
	pstRegister->usXstop   = S_stCmd.stImageFeature.usXBayerEnd   * 2 + 1;
	pstRegister->usYstart  = S_stCmd.stImageFeature.usYBayerStart * 2    ;
	pstRegister->usXstart  = S_stCmd.stImageFeature.usXBayerStart * 2    ; 
}


static void setOrientation(ts_sensorRegister* pstRegister, uint8_t ucOrientation ) {


}


static uint32_t computePixelClock( void ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

uint32_t PCLK = 0;

/* Define input pixel clock */

	switch(priv->inputMode)
	{
		case INPUTMODE_OV2715_640x480_6FPS :
            PCLK = 0x1C << 4;
            break;

		case INPUTMODE_OV2715_640x480_8FPS :

			PCLK = 18<<4;

			break;

		case INPUTMODE_OV2715_1920x1080_10FPS :

			PCLK = 18<<4;

			break;


		case INPUTMODE_VIGEN_640x480 :

			PCLK = 18<<4;

			break;


		case INPUTMODE_VIGEN_1920x1080 :

			PCLK = 18<<4;

			break;

		case INPUTMODE_VIGEN_4208x3120 :

			PCLK = 18<<4;

			break;


		case INPUTMODE_V_SENSOR :

			PCLK = 36<<4;

			break;

		default :

			PCLK = 18<<4;

			break;
	}

	return PCLK;
		

}


static inline void getImageFeature( ts_SENSOR_Status* pttsStatus ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;
	ts_SENSOR_Cmd *pCmd = &pttsStatus->stAppliedCmd;

	switch(priv->inputMode)
	{

		case INPUTMODE_OV2715_640x480_6FPS :
		case INPUTMODE_OV2715_640x480_8FPS :

			pCmd->stImageFeature.usXBayerStart = 0;//READ_BY_CACHE( Offset_x_addr_start ) / 2;
			pCmd->stImageFeature.usYBayerStart = 0;//READ_BY_CACHE( Offset_y_addr_start ) / 2;

			pCmd->stImageFeature.eOrientation  = DxOISP_FLIP_OFF_MIRROR_OFF;
			
			pCmd->stImageFeature.usMinVertBlanking = 28; //READ_BY_CACHE( Offset_frame_length_lines ) - READ_BY_CACHE( Offset_y_output_size );
			pCmd->stImageFeature.uiMinHorBlanking  = 20; //READ_BY_CACHE( Offset_line_length_pck )    - READ_BY_CACHE( Offset_x_output_size );

			pttsStatus->ucHorSkippingFactor                                  =
			pCmd->stImageFeature.ucHorDownsamplingFactor  = 1; //((READ_BY_CACHE(Offset_x_odd_even_inc)>>4) + 1 ) / 2;
			pttsStatus->ucVertSkippingFactor                                 =
			pCmd->stImageFeature.ucVertDownsamplingFactor = 1; //((READ_BY_CACHE(Offset_y_odd_even_inc)>>4) + 1 ) / 2;

			pCmd->stImageFeature.usXBayerEnd   = 319;//pCmd->stImageFeature.usXBayerStart + READ_BY_CACHE(Offset_x_output_size) * pCmd->stImageFeature.ucHorDownsamplingFactor  / 2 - 1;
			pCmd->stImageFeature.usYBayerEnd   = 239;//pCmd->stImageFeature.usYBayerStart + READ_BY_CACHE(Offset_y_output_size) * pCmd->stImageFeature.ucVertDownsamplingFactor / 2 - 1;

		#if 0	//pjo no use for OV2715
			if ( READ_BY_CACHE(Offset_timing_tc_reg21)&0x01) {
				pttsStatus->ucHorSkippingFactor  >>= 1;
				pttsStatus->ucVertSkippingFactor >>= 1;
			}
		#endif
			pttsStatus->eCurrentPhase = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_OV2715_1920x1080_10FPS :

			pCmd->stImageFeature.usXBayerStart = 0;//READ_BY_CACHE( Offset_x_addr_start ) / 2;
			pCmd->stImageFeature.usYBayerStart = 0;//READ_BY_CACHE( Offset_y_addr_start ) / 2;

			pCmd->stImageFeature.eOrientation  = DxOISP_FLIP_OFF_MIRROR_OFF;
			
			pCmd->stImageFeature.usMinVertBlanking = 28; //READ_BY_CACHE( Offset_frame_length_lines ) - READ_BY_CACHE( Offset_y_output_size );
			pCmd->stImageFeature.uiMinHorBlanking  = 20; //READ_BY_CACHE( Offset_line_length_pck )    - READ_BY_CACHE( Offset_x_output_size );

			pttsStatus->ucHorSkippingFactor                                  =
			pCmd->stImageFeature.ucHorDownsamplingFactor  = 1; //((READ_BY_CACHE(Offset_x_odd_even_inc)>>4) + 1 ) / 2;
			pttsStatus->ucVertSkippingFactor                                 =
			pCmd->stImageFeature.ucVertDownsamplingFactor = 1; //((READ_BY_CACHE(Offset_y_odd_even_inc)>>4) + 1 ) / 2;

			pCmd->stImageFeature.usXBayerEnd   = 959;//pCmd->stImageFeature.usXBayerStart + READ_BY_CACHE(Offset_x_output_size) * pCmd->stImageFeature.ucHorDownsamplingFactor  / 2 - 1;
			pCmd->stImageFeature.usYBayerEnd   = 539;//pCmd->stImageFeature.usYBayerStart + READ_BY_CACHE(Offset_y_output_size) * pCmd->stImageFeature.ucVertDownsamplingFactor / 2 - 1;

		#if 0	//pjo no use for OV2715
			if ( READ_BY_CACHE(Offset_timing_tc_reg21)&0x01) {
				pttsStatus->ucHorSkippingFactor  >>= 1;
				pttsStatus->ucVertSkippingFactor >>= 1;
			}
		#endif
			pttsStatus->eCurrentPhase = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_VIGEN_640x480 :

			pCmd->stImageFeature.usXBayerStart = 0;//READ_BY_CACHE( Offset_x_addr_start ) / 2;
			pCmd->stImageFeature.usYBayerStart = 0;//READ_BY_CACHE( Offset_y_addr_start ) / 2;

			pCmd->stImageFeature.eOrientation  = DxOISP_FLIP_OFF_MIRROR_OFF;
			
			pCmd->stImageFeature.usMinVertBlanking = 10; //READ_BY_CACHE( Offset_frame_length_lines ) - READ_BY_CACHE( Offset_y_output_size );
			pCmd->stImageFeature.uiMinHorBlanking  = 10; //READ_BY_CACHE( Offset_line_length_pck )    - READ_BY_CACHE( Offset_x_output_size );

			pttsStatus->ucHorSkippingFactor                                  =
			pCmd->stImageFeature.ucHorDownsamplingFactor  = 1; //((READ_BY_CACHE(Offset_x_odd_even_inc)>>4) + 1 ) / 2;
			pttsStatus->ucVertSkippingFactor                                 =
			pCmd->stImageFeature.ucVertDownsamplingFactor = 1; //((READ_BY_CACHE(Offset_y_odd_even_inc)>>4) + 1 ) / 2;

			pCmd->stImageFeature.usXBayerEnd   = 319;//pCmd->stImageFeature.usXBayerStart + READ_BY_CACHE(Offset_x_output_size) * pCmd->stImageFeature.ucHorDownsamplingFactor  / 2 - 1;
			pCmd->stImageFeature.usYBayerEnd   = 239;//pCmd->stImageFeature.usYBayerStart + READ_BY_CACHE(Offset_y_output_size) * pCmd->stImageFeature.ucVertDownsamplingFactor / 2 - 1;

		#if 0	//pjo no use for OV2715
			if ( READ_BY_CACHE(Offset_timing_tc_reg21)&0x01) {
				pttsStatus->ucHorSkippingFactor  >>= 1;
				pttsStatus->ucVertSkippingFactor >>= 1;
			}
		#endif
			pttsStatus->eCurrentPhase = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_VIGEN_1920x1080:


			pCmd->stImageFeature.usXBayerStart = 0;//READ_BY_CACHE( Offset_x_addr_start ) / 2;
			pCmd->stImageFeature.usYBayerStart = 0;//READ_BY_CACHE( Offset_y_addr_start ) / 2;

			pCmd->stImageFeature.eOrientation  = DxOISP_FLIP_OFF_MIRROR_OFF;
			
			pCmd->stImageFeature.usMinVertBlanking = 10; //READ_BY_CACHE( Offset_frame_length_lines ) - READ_BY_CACHE( Offset_y_output_size );
			pCmd->stImageFeature.uiMinHorBlanking  = 10; //READ_BY_CACHE( Offset_line_length_pck )    - READ_BY_CACHE( Offset_x_output_size );

			pttsStatus->ucHorSkippingFactor                                  =
			pCmd->stImageFeature.ucHorDownsamplingFactor  = 1; //((READ_BY_CACHE(Offset_x_odd_even_inc)>>4) + 1 ) / 2;
			pttsStatus->ucVertSkippingFactor                                 =
			pCmd->stImageFeature.ucVertDownsamplingFactor = 1; //((READ_BY_CACHE(Offset_y_odd_even_inc)>>4) + 1 ) / 2;

			pCmd->stImageFeature.usXBayerEnd   = 959;//pCmd->stImageFeature.usXBayerStart + READ_BY_CACHE(Offset_x_output_size) * pCmd->stImageFeature.ucHorDownsamplingFactor  / 2 - 1;
			pCmd->stImageFeature.usYBayerEnd   = 539;//pCmd->stImageFeature.usYBayerStart + READ_BY_CACHE(Offset_y_output_size) * pCmd->stImageFeature.ucVertDownsamplingFactor / 2 - 1;

		#if 0	//pjo no use for OV2715
			if ( READ_BY_CACHE(Offset_timing_tc_reg21)&0x01) {
				pttsStatus->ucHorSkippingFactor  >>= 1;
				pttsStatus->ucVertSkippingFactor >>= 1;
			}
		#endif
			pttsStatus->eCurrentPhase = DxOISP_SENSOR_PHASE_GRBG;

		break;

		case INPUTMODE_VIGEN_4208x3120:


			pCmd->stImageFeature.usXBayerStart = 0;//READ_BY_CACHE( Offset_x_addr_start ) / 2;
			pCmd->stImageFeature.usYBayerStart = 0;//READ_BY_CACHE( Offset_y_addr_start ) / 2;

			pCmd->stImageFeature.eOrientation  = DxOISP_FLIP_OFF_MIRROR_OFF;
			
			pCmd->stImageFeature.usMinVertBlanking = 10; //READ_BY_CACHE( Offset_frame_length_lines ) - READ_BY_CACHE( Offset_y_output_size );
			pCmd->stImageFeature.uiMinHorBlanking  = 10; //READ_BY_CACHE( Offset_line_length_pck )    - READ_BY_CACHE( Offset_x_output_size );

			pttsStatus->ucHorSkippingFactor                                  =
			pCmd->stImageFeature.ucHorDownsamplingFactor  = 1; //((READ_BY_CACHE(Offset_x_odd_even_inc)>>4) + 1 ) / 2;
			pttsStatus->ucVertSkippingFactor                                 =
			pCmd->stImageFeature.ucVertDownsamplingFactor = 1; //((READ_BY_CACHE(Offset_y_odd_even_inc)>>4) + 1 ) / 2;

			pCmd->stImageFeature.usXBayerEnd   = 2103;//pCmd->stImageFeature.usXBayerStart + READ_BY_CACHE(Offset_x_output_size) * pCmd->stImageFeature.ucHorDownsamplingFactor  / 2 - 1;
			pCmd->stImageFeature.usYBayerEnd   = 1559;//pCmd->stImageFeature.usYBayerStart + READ_BY_CACHE(Offset_y_output_size) * pCmd->stImageFeature.ucVertDownsamplingFactor / 2 - 1;

		#if 0	//pjo no use for OV2715
			if ( READ_BY_CACHE(Offset_timing_tc_reg21)&0x01) {
				pttsStatus->ucHorSkippingFactor  >>= 1;
				pttsStatus->ucVertSkippingFactor >>= 1;
			}
		#endif
			pttsStatus->eCurrentPhase = DxOISP_SENSOR_PHASE_RGGB;

		break;

		case INPUTMODE_V_SENSOR :

			pCmd->stImageFeature.usXBayerStart = 0;//READ_BY_CACHE( Offset_x_addr_start ) / 2;
			pCmd->stImageFeature.usYBayerStart = 0;//READ_BY_CACHE( Offset_y_addr_start ) / 2;

			pCmd->stImageFeature.eOrientation  = DxOISP_FLIP_OFF_MIRROR_OFF;
			
			pCmd->stImageFeature.usMinVertBlanking = 10; //READ_BY_CACHE( Offset_frame_length_lines ) - READ_BY_CACHE( Offset_y_output_size );
			pCmd->stImageFeature.uiMinHorBlanking  = 10; //READ_BY_CACHE( Offset_line_length_pck )    - READ_BY_CACHE( Offset_x_output_size );

			pttsStatus->ucHorSkippingFactor                                  =
			pCmd->stImageFeature.ucHorDownsamplingFactor  = 1; //((READ_BY_CACHE(Offset_x_odd_even_inc)>>4) + 1 ) / 2;
			pttsStatus->ucVertSkippingFactor                                 =
			pCmd->stImageFeature.ucVertDownsamplingFactor = 1; //((READ_BY_CACHE(Offset_y_odd_even_inc)>>4) + 1 ) / 2;

			pCmd->stImageFeature.usXBayerEnd   = 2103;//pCmd->stImageFeature.usXBayerStart + READ_BY_CACHE(Offset_x_output_size) * pCmd->stImageFeature.ucHorDownsamplingFactor  / 2 - 1;
			pCmd->stImageFeature.usYBayerEnd   = 1559;//pCmd->stImageFeature.usYBayerStart + READ_BY_CACHE(Offset_y_output_size) * pCmd->stImageFeature.ucVertDownsamplingFactor / 2 - 1;

		#if 0	//pjo no use for OV2715
			if ( READ_BY_CACHE(Offset_timing_tc_reg21)&0x01) {
				pttsStatus->ucHorSkippingFactor  >>= 1;
				pttsStatus->ucVertSkippingFactor >>= 1;
			}
		#endif
			pttsStatus->eCurrentPhase = DxOISP_SENSOR_PHASE_GRBG;

		break;
	}

}


static uint16_t computeFps(uint16_t usLineLenght, uint16_t usFrameLenght) {
	uint32_t uiImageSize = usLineLenght * usFrameLenght;
	return 1000 * DIV_UP( 1000 * (uint64_t)S_uiSensorPixelClock << 8, uiImageSize ) >> 8;
}

static inline void getFrameRate( ts_SENSOR_Status* pttsStatus ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	if(priv->inputModeDevice == INPUTMODE_OV2715){
		pttsStatus->stAppliedCmd.stImageFeature.usFrameRate = computeFps(READ_BY_CACHE( Offset_line_length_pck ), READ_BY_CACHE( Offset_frame_length_lines ));
#if 0
        DBG("pttsStatus->stAppliedCmd.stImageFeature.usFrameRate = %u( >> 4 = %u)",
                pttsStatus->stAppliedCmd.stImageFeature.usFrameRate,
                pttsStatus->stAppliedCmd.stImageFeature.usFrameRate >> 4);
#endif  /*0*/
	}
	else {
		pttsStatus->stAppliedCmd.stImageFeature.usFrameRate = 6 << DxOISP_FPS_FRAC_PART;
	}

}


static inline void setFrameRate(ts_sensorRegister* pstRegister, uint16_t usNewFrameRate ) {
	uint32_t uiResult;
	assertf0( usNewFrameRate!=0, "unexpected frame rate set to 0!!!" );

    //DBG("NewFrameRate = %u", usNewFrameRate);
	uiResult = 1000 * DIV_UP( 1000 * (uint64_t)S_uiSensorPixelClock << 8, pstRegister->usLineLength * usNewFrameRate ) >> 8;
	pstRegister->usFrameLength = MIN(MAX_FRAME_LENGTH_LINE, MAX(MIN_FRAME_LENGTH_LINE, uiResult));
}


static inline void getExposureTime( ts_SENSOR_Status* pttsStatus ) { 

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	if(priv->inputModeDevice == INPUTMODE_OV2715) {
		
		uint32_t uiLineLenghtPixClk = READ_BY_CACHE(Offset_line_length_pck);
		uint32_t uiLineLength       = uiLineLenghtPixClk << 12;
		uint64_t uiLineTime         = ((uint64_t)uiLineLength<<4) / S_uiSensorPixelClock;        //in ns expressed in 52.12

		pttsStatus->stAppliedCmd.stShootingParam.uiExpoTime = (((uint64_t)S_uiAppliedIT * uiLineTime)/1000);
	}
	else {	
		pttsStatus->stAppliedCmd.stShootingParam.uiExpoTime = 30<<EXPO_TIME_FRAC_PART;
	}

}


static inline void setBinning (ts_sensorRegister* pstRegister, uint8_t isBinning ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	if(priv->inputModeDevice == INPUTMODE_OV2715) {
		pstRegister->ucTimingRegX = isBinning ? (pstRegister->ucTimingRegX|0x01) : (pstRegister->ucTimingRegX&(~0x01));
		pstRegister->ucTimingRegY = isBinning ? (pstRegister->ucTimingRegY|0x01) : (pstRegister->ucTimingRegY&(~0x01));
		pstRegister->ucPsRamCtrl0  = 0;	//(!isBinning) << 1;	//pjo no use for OV2715
	}
	else {
		pstRegister->ucTimingRegX = 0;
		pstRegister->ucTimingRegY = 0;
		pstRegister->ucPsRamCtrl0  = 0;	//(!isBinning) << 1;	//pjo no use for OV2715
	}
		
}

static inline ts_Decim setSkipping(ts_sensorRegister* pstRegister, uint8_t ucNewSkipX,uint8_t ucNewSkipY ) {
	
	ts_Decim stSkip;
	
	stSkip.ucX = MIN( S_stStatus.stCapabilities.stImageFeature.ucMaxHorDownsamplingFactor,  ucNewSkipX );
	stSkip.ucY = MIN( S_stStatus.stCapabilities.stImageFeature.ucMaxVertDownsamplingFactor, ucNewSkipY );

	pstRegister->ucIncX  = ((2 * stSkip.ucX - 1)<<4) + 1;
	pstRegister->ucIncY  = ((2 * stSkip.ucY - 1)<<4) + 1;


	return stSkip;
}

static void setDecimation(ts_sensorRegister* pstRegister, uint8_t ucDecimX, uint8_t ucDecimY ) {
	uint8_t isBinning=0;

	assertf1(((ucDecimX>=1) && (ucDecimX<=4)),"ucDecimX (%d) is uncorrect!", ucDecimX );
	assertf1(((ucDecimY>=1) && (ucDecimY<=4)),"ucDecimY (%d) is uncorrect!", ucDecimY );

	if ((ucDecimX>=2) && (ucDecimX%2 == 0) && (ucDecimY>=2) && (ucDecimY%2 == 0)) isBinning = 1;
	setBinning(pstRegister, isBinning);
	setOutputSize(pstRegister, setSkipping(pstRegister,ucDecimX,ucDecimY));
}

typedef struct {
	uint16_t usStartX;
	uint16_t usStartY;
	uint16_t usStopX;
	uint16_t usStopY;
	uint16_t usOffsetX;
	uint16_t usOffsetY;
	uint16_t usOutSizeX;
	uint16_t usOutSizeY;
	uint16_t isbBinning;
} ts_CropConfig;



static ts_CropConfig S_stCropConf[] = {
	{0  , 0  , 640, 480,   0,   0, 640, 480, 0} //
,	{0  , 0  , 640, 480,   0,   0, 640, 480, 0} //
,	{0  , 0  , 640, 480,   0,   0, 640, 480, 0} //
,	{0  , 0  , 640, 480,   0,   0, 640, 480, 0} //
};
static inline void setOutputSize(ts_sensorRegister* pstRegister, ts_Decim stSkip ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;
	unsigned int i;
	uint8_t isBinning = stSkip.ucX == 2;
	uint8_t ucIsResultOk[sizeof(S_stCropConf)/sizeof(ts_CropConfig)];
	uint16_t usDist     [sizeof(S_stCropConf)/sizeof(ts_CropConfig)];
	uint16_t usTmpOutSizeX = DIV_UP(pstRegister->usXstop  - pstRegister->usXstart +1, stSkip.ucX) ;
	uint16_t usTmpOutSizeY = DIV_UP(pstRegister->usYstop  - pstRegister->usYstart +1, stSkip.ucY) ;

	switch(priv->inputMode)
	{
		case INPUTMODE_OV2715_640x480_6FPS :
		case INPUTMODE_OV2715_640x480_8FPS :

			for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {

					S_stCropConf[i].usStopX    = 640;
					S_stCropConf[i].usStopY    = 480;
					S_stCropConf[i].usOutSizeX = 640;
					S_stCropConf[i].usOutSizeY = 480;
				}

			break;

		case INPUTMODE_OV2715_1920x1080_10FPS :

			for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {

					S_stCropConf[i].usStopX    = 1920;
					S_stCropConf[i].usStopY    = 1080;
					S_stCropConf[i].usOutSizeX = 1920;
					S_stCropConf[i].usOutSizeY = 1080;
				}

			break;

		case INPUTMODE_VIGEN_640x480 :

			for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {

					S_stCropConf[i].usStopX    = 640;
					S_stCropConf[i].usStopY    = 480;
					S_stCropConf[i].usOutSizeX = 640;
					S_stCropConf[i].usOutSizeY = 480;
				}


			break;

		case INPUTMODE_VIGEN_1920x1080 :

			for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {

					S_stCropConf[i].usStopX    = 1920;
					S_stCropConf[i].usStopY    = 1080;
					S_stCropConf[i].usOutSizeX = 1920;
					S_stCropConf[i].usOutSizeY = 1080;
				}

			break;

		case INPUTMODE_VIGEN_4208x3120 :

			for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {

					S_stCropConf[i].usStopX    = 4208;
					S_stCropConf[i].usStopY    = 3120;
					S_stCropConf[i].usOutSizeX = 4208;
					S_stCropConf[i].usOutSizeY = 3120;
				}


			break;

		case INPUTMODE_V_SENSOR :

			for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {

					S_stCropConf[i].usStopX    = 4208;
					S_stCropConf[i].usStopY    = 3120;
					S_stCropConf[i].usOutSizeX = 4208;
					S_stCropConf[i].usOutSizeY = 3120;
				}

			break;

		default :

			break;

	}

	for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {
		ucIsResultOk[i] = 0;
		if(	S_stCropConf[i].isbBinning == isBinning
		&&	S_stCropConf[i].usStartX   <= pstRegister->usXstart
		&&	S_stCropConf[i].usStopX    >= pstRegister->usXstop
		&&	S_stCropConf[i].usStartY   <= pstRegister->usYstart
		&&	S_stCropConf[i].usStopY    >= pstRegister->usYstop
		&&	S_stCropConf[i].usOutSizeX >= usTmpOutSizeX
		&&	S_stCropConf[i].usOutSizeY >= usTmpOutSizeY) {
			ucIsResultOk[i] = 1;
			usDist[i]       = S_stCropConf[i].usOutSizeX - usTmpOutSizeX;
		}
	}

	uint16_t usMin = 0xffff;
	for(i=0;i<sizeof(S_stCropConf)/sizeof(ts_CropConfig);i++) {
		if(ucIsResultOk[i] && usDist[i] < usMin) {
			S_ucCropConfigIdx = i;
			usMin    = usDist[i];
		}
	}

	pstRegister->usXstart   = S_stCropConf[S_ucCropConfigIdx].usStartX;
	pstRegister->usYstart   = S_stCropConf[S_ucCropConfigIdx].usStartY;
	pstRegister->usXstop    = S_stCropConf[S_ucCropConfigIdx].usStopX;
	pstRegister->usYstop    = S_stCropConf[S_ucCropConfigIdx].usStopY;
	pstRegister->usOutSizeX = S_stCropConf[S_ucCropConfigIdx].usOutSizeX;
	pstRegister->usOutSizeY = S_stCropConf[S_ucCropConfigIdx].usOutSizeY;
	pstRegister->usOffsetH  = S_stCropConf[S_ucCropConfigIdx].usOffsetX;
	pstRegister->usOffsetV  = S_stCropConf[S_ucCropConfigIdx].usOffsetY;

#if 0
	printf("Applied XS:%d YS:%d XE:%d YE:%d outX:%d outY:%d offX:%d offY:%d\n"
	,	pstRegister->usXstart
	,	pstRegister->usYstart
	,	pstRegister->usXstop
	,	pstRegister->usYstop
	,	pstRegister->usOutSizeX
	,	pstRegister->usOutSizeY
	,	pstRegister->usOffsetH
	,	pstRegister->usOffsetV);
#endif
}


static inline void setMode(ts_sensorRegister* pstRegister, uint8_t isStreaming ) {
	pstRegister->isStreaming = isStreaming;
}

static void setFlash( uint16_t usPower ) {
	//DeviceFireFlash( usPower );
}

static inline void getMode ( ts_SENSOR_Status* pttsStatus ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	if(priv->inputModeDevice == INPUTMODE_OV2715) {

		uint8_t usStreamingMode = 0;
		usStreamingMode = READ_BY_CACHE(Offset_mode_select);

		if(usStreamingMode == 0x42)
			pttsStatus->stAppliedCmd.isSensorStreaming = 0;
		else
			pttsStatus->stAppliedCmd.isSensorStreaming = 1;
	}
	else {
		pttsStatus->stAppliedCmd.isSensorStreaming = 1;
	}

}

static void setMinimalBlanking(ts_sensorRegister* pstRegister, uint16_t usMinVertBlk, uint32_t uiMinHorBlk ) {
	uint32_t uiLineTime;
	uint16_t usVertical;
	//uint16_t usHorizontal = MAX( usMinHorBlk,  MIN_LINE_BLANKING_PCK);
	//This rule rule has been determined by experiences
//p	uint32_t uiHorizontal = MAX( uiMinHorBlk,  ((pstRegister->usOutSizeX * 17236 >> 16) - 30));
	uint32_t uiHorizontal = uiMinHorBlk;

	//p assertf1(uiHorizontal + pstRegister->usOutSizeX < (1<<16), "computed line length %d must be inferior to 65536", uiHorizontal + pstRegister->usOutSizeX);
	
	pstRegister->usLineLength = uiHorizontal + pstRegister->usOutSizeX;
	pstRegister->usLineLength = MAX( MIN_LINE_LENGTH_PCK, pstRegister->usLineLength );
	pstRegister->usLineLength = MIN( MAX_LINE_LENGTH_PCK, pstRegister->usLineLength );
	
	//forbidden value for usLineLength: 0x1c/0x1d 0x5c/0x5d, 0x9c/0x9d and 0xdc/0xdd
	if((pstRegister->usLineLength & 0x003e) == 0x1c) {
		pstRegister->usLineLength += 2;
	}

	uiLineTime = (pstRegister->usLineLength << 15) / (S_uiSensorPixelClock<<3);         //in ns expressed in 24.8
	usVertical = MAX((DIV_UP((uint32_t)usMinVertBlk<<12,uiLineTime)*1000) >> 12, MIN_FRAME_BLANKING_LINE) ;
	pstRegister->usFrameLength = usVertical + pstRegister->usOutSizeY;
	
	pstRegister->usFrameLength = MAX( MIN_FRAME_LENGTH_LINE, pstRegister->usFrameLength );
#if (MAX_FRAME_LENGTH_LINE != 0xFFFF)
	pstRegister->usFrameLength = MIN( MAX_FRAME_LENGTH_LINE, pstRegister->usFrameLength );
#endif
}

static void setAF(ts_sensorRegister* pstRegister)
{
	uint16_t usAfPos;
	usAfPos = MIN(MAX(S_stCmd.usAfPosition, MIN_AF_POSITION), MAX_AF_POSITION);
	pstRegister->usAF = ((((usAfPos&0x000f)<<4) + 0xf)<<8) + ((usAfPos>>4)&0x3f);
	
}

static void getAF( ts_SENSOR_Status* pttsStatus )
{
	uint16_t usReg = READ_BY_CACHE(Offset_Af);
	pttsStatus->stAppliedCmd.usAfPosition = ((usReg&0xf000)>>12) + ((usReg&0x003f)<<4);
}

static void setExposureTime( ts_sensorRegister* pstRegister )
{
	uint32_t uiLineLength        = pstRegister->usLineLength << 12;
	uint32_t uiLineTime          = uiLineLength / (S_uiSensorPixelClock);        //in ns expressed in 24.8
	uint16_t usNewframeRate      = 0;

	//Note: we remove 4 LSB bits to avoid a bug of Ov8820
	pstRegister->uiExpoTime = (((((uint64_t)S_stCmd.stShootingParam.uiExpoTime<<12)*1000) /uiLineTime)>>16) & 0x000ffff0; //16.4
	
#if MIN_INTEGRATION_TIME != 0
	pstRegister->uiExpoTime  = MAX( MIN_INTEGRATION_TIME, pstRegister->uiExpoTime );
#endif
	
	//Determine new Frame_length_lines Min for this exposure Time
	pstRegister->usFrameLength    = MAX ((pstRegister->uiExpoTime>>4) + 20, pstRegister->usFrameLength );
	
	usNewframeRate = computeFps(pstRegister->usLineLength, pstRegister->usFrameLength);
    //DBG("usNewframeRate = %u( >> 4 = %u)", usNewframeRate, usNewframeRate >> 4);

	if (usNewframeRate > S_stCmd.stImageFeature.usFrameRate)
		setFrameRate(pstRegister, S_stCmd.stImageFeature.usFrameRate);
}

static inline void getPixelClock ( ts_SENSOR_Status* pttsStatus ) {
	pttsStatus->uiPixelClock = S_uiSensorPixelClock << 12; //28.4 to 16.16
}

#if defined(CHECK_SENSOR_BEFORE_START)
static void inline autoCheckPluggedSensor (void)  {
	uint16_t usRevNumber   = readDevice(0x302A, 1);
//	assertf2( REG_REVISION_NUMBER == usRevNumber, "unexpected rev nb   (0x%04X instead of 0x%04X", usRevNumber, REG_REVISION_NUMBER );
}
#endif


static inline void initSensorCmd ( ts_SENSOR_Cmd*  pttsCmd, ts_SENSOR_Status* pttsStatus ) {
	#define DEFAULT_FRAMERATE                   10<<4
	#define DEFAULT_EXPOSURE_TIME               30<<EXPO_TIME_FRAC_PART
	#define DEFAULT_ANALOG_GAIN                 1<<GAIN_FRAC_PART
	#define DEFAULT_DIGITAL_GAIN                1<<GAIN_FRAC_PART

	pttsCmd->stImageFeature.eOrientation             = DxOISP_FLIP_OFF_MIRROR_OFF;
	pttsCmd->stImageFeature.usXBayerEnd              = pttsStatus->stCapabilities.stImageFeature.usBayerWidth  - 1;
	pttsCmd->stImageFeature.usYBayerEnd              = pttsStatus->stCapabilities.stImageFeature.usBayerHeight - 1;
	pttsCmd->stImageFeature.usXBayerStart            = 0;
	pttsCmd->stImageFeature.usYBayerStart            = 0;
	pttsCmd->stImageFeature.ucHorDownsamplingFactor  = 1;
	pttsCmd->stImageFeature.ucVertDownsamplingFactor = 1;
	
	pttsCmd->stShootingParam.usAGain[0]              = 
	pttsCmd->stShootingParam.usAGain[1]              =
	pttsCmd->stShootingParam.usAGain[2]              = 
	pttsCmd->stShootingParam.usAGain[3]              = DEFAULT_ANALOG_GAIN;
	pttsCmd->stShootingParam.usDGain[0]              = 
	pttsCmd->stShootingParam.usDGain[1]              =
	pttsCmd->stShootingParam.usDGain[2]              =
	pttsCmd->stShootingParam.usDGain[3]              = DEFAULT_DIGITAL_GAIN;

	pttsCmd->stImageFeature.usMinVertBlanking        = pttsStatus->stCapabilities.stImageFeature.usMinVertBlanking;
	pttsCmd->stImageFeature.uiMinHorBlanking         = pttsStatus->stCapabilities.stImageFeature.usMinHorBlanking;

	pttsCmd->stShootingParam.uiExpoTime              = DEFAULT_EXPOSURE_TIME; 
    //[[tifler:test]]
	//pttsCmd->stImageFeature.usFrameRate              = DEFAULT_FRAMERATE;
	pttsCmd->stImageFeature.usFrameRate              = 5 << 4;
	pttsCmd->usAfPosition                            = MIN_AF_POSITION;

}

static inline void  setStaticSensorConfiguration ( int nbLanes ) {

    struct OVT2715Private *priv = (struct OVT2715Private *)THIS_SENSOR->priv;

	switch(priv->inputMode)
	{
		case INPUTMODE_OV2715_640x480_6FPS :
            ov2715_write2( 0x4740, 0x00);
            ov2715_write2( 0x3103, 0x93);
            ov2715_write( 0x3008, 0x82);
            ov2715_write2( 0x3017, 0x7f);
            ov2715_write2( 0x3018, 0xfc);
            ov2715_write2( 0x3706, 0x61);
            ov2715_write2( 0x3712, 0x0c);
            ov2715_write2( 0x3630, 0x6d);

            ov2715_write2( 0x3621, 0x04);
            ov2715_write2( 0x3604, 0x60);
            ov2715_write2( 0x3603, 0xa7);
            ov2715_write2( 0x3631, 0x26);
            ov2715_write2( 0x3600, 0x04);
            ov2715_write2( 0x3620, 0x37);
            ov2715_write2( 0x3623, 0x00);
            ov2715_write2( 0x3702, 0x9e);
            ov2715_write2( 0x3703, 0x5c);
            ov2715_write2( 0x3704, 0x40);
            ov2715_write2( 0x370d, 0x0f);
            ov2715_write2( 0x3713, 0x9f);
            ov2715_write2( 0x3714, 0x4c);
            ov2715_write2( 0x3710, 0x9e);

            ov2715_write2( 0x3605, 0x05);
            ov2715_write2( 0x3606, 0x3f);
            ov2715_write2( 0x302d, 0x90);
            ov2715_write2( 0x370b, 0x40);
            ov2715_write2( 0x3716, 0x31);
            ov2715_write2( 0x3707, 0x52);

            ov2715_write2( 0x5181, 0x20);
            ov2715_write2( 0x518f, 0x00);
            ov2715_write2( 0x4301, 0xff);
            ov2715_write2( 0x4303, 0x00);
            ov2715_write2( 0x3a00, 0x78);
            ov2715_write2( 0x300f, 0x88);  
            ov2715_write2( 0x3011, 0x16);
            ov2715_write2( 0x3a1a, 0x06);
            ov2715_write2( 0x3a18, 0x00);
            ov2715_write2( 0x3a19, 0x7a);
            ov2715_write2( 0x3a13, 0x54);
            ov2715_write2( 0x382e, 0x0f);
            ov2715_write2( 0x381a, 0x1a);
            ov2715_write2( 0x401d, 0x02);

            ov2715_write2( 0x3a0f, 0x40);
            ov2715_write2( 0x3a10, 0x38);
            ov2715_write2( 0x3a1b, 0x48);
            ov2715_write2( 0x3a1e, 0x30);
            ov2715_write2( 0x3a11, 0x90);
            ov2715_write2( 0x3a1f, 0x10);

            ov2715_write2( 0x5680, 0x05);
            ov2715_write2( 0x5681, 0x00);
            ov2715_write2( 0x5682, 0x07);
            ov2715_write2( 0x5683, 0x80);
            ov2715_write2( 0x5684, 0x02);
            ov2715_write2( 0x5685, 0x58);
            ov2715_write2( 0x5686, 0x04);
            ov2715_write2( 0x5687, 0x38);


            ov2715_write2( 0x3800, 0x01);
            ov2715_write2( 0x3801, 0xBe);

#if 1
            ov2715_write2( 0x3802, 0x00);
            ov2715_write2( 0x3803, 0x0a);
            ov2715_write2( 0x3804, 0x02);
            ov2715_write2( 0x3805, 0x80);
            ov2715_write2( 0x3806, 0x01);
            ov2715_write2( 0x3807, 0xe0);
#endif
            ov2715_write2( 0x3808, 0x02);
            ov2715_write2( 0x3809, 0x80);
            ov2715_write2( 0x380A, 0x01);
            ov2715_write2( 0x380B, 0xE0);
#if	0
            ov2715_write2( 0x380C, 0x09);
            ov2715_write2( 0x380D, 0x74);
            ov2715_write2( 0x380E, 0x04);
            ov2715_write2( 0x380F, 0x50);
#endif

            ov2715_write2( 0x3a00, 0x5c);
            ov2715_write2( 0x5001, 0x4f);
            ov2715_write2( 0x3406, 0x01);

            ov2715_write2( 0x3400, 0x03);
            ov2715_write2( 0x3401, 0x40);
            ov2715_write2( 0x3402, 0x04);
            ov2715_write2( 0x3403, 0x00);
            ov2715_write2( 0x3404, 0x05);
            ov2715_write2( 0x3405, 0xa0);

            ov2715_write2( 0x3818, 0xA0);	//Vertical Flip
            ov2715_write2( 0x370d, 0x0f);
            ov2715_write2( 0x3621, 0xc4);
            ov2715_write2( 0x4708, 0x03);	//Vsync Polarity Inversion
            ov2715_write2( 0x3500, 0x00);	//Exposure Time High
            ov2715_write2( 0x3501, 0x20);	//Exposure Time Mid
            ov2715_write2( 0x3503, 0x07);	// Manual Framerate, Manual Exposure
            ov2715_write2( 0x350C, 0x03);//15);	// Manaul Frame Length High
            ov2715_write2( 0x350D, 0x00);	// Manual Frame Length Low

            ov2715_write2( 0x300F, 0x88);
            ov2715_write2( 0x3010, 0x20);
            ov2715_write2( 0x3011, 0x36);
            ov2715_write2( 0x3012, 0x01);

            break;

		case INPUTMODE_OV2715_640x480_8FPS :

			ov2715_write2( 0x4740, 0x00);
			ov2715_write2( 0x3103, 0x93);
			ov2715_write( 0x3008, 0x82);
			ov2715_write2( 0x3017, 0x7f);
			ov2715_write2( 0x3018, 0xfc);
			ov2715_write2( 0x3706, 0x61);
			ov2715_write2( 0x3712, 0x0c);
			ov2715_write2( 0x3630, 0x6d);

			ov2715_write2( 0x3621, 0x04);
			ov2715_write2( 0x3604, 0x60);
			ov2715_write2( 0x3603, 0xa7);
			ov2715_write2( 0x3631, 0x26);
			ov2715_write2( 0x3600, 0x04);
			ov2715_write2( 0x3620, 0x37);
			ov2715_write2( 0x3623, 0x00);
			ov2715_write2( 0x3702, 0x9e);
			ov2715_write2( 0x3703, 0x5c);
			ov2715_write2( 0x3704, 0x40);
			ov2715_write2( 0x370d, 0x0f);
			ov2715_write2( 0x3713, 0x9f);
			ov2715_write2( 0x3714, 0x4c);
			ov2715_write2( 0x3710, 0x9e);

			ov2715_write2( 0x3605, 0x05);
			ov2715_write2( 0x3606, 0x3f);
			ov2715_write2( 0x302d, 0x90);
			ov2715_write2( 0x370b, 0x40);
			ov2715_write2( 0x3716, 0x31);
			ov2715_write2( 0x3707, 0x52);

			ov2715_write2( 0x5181, 0x20);
			ov2715_write2( 0x518f, 0x00);
			ov2715_write2( 0x4301, 0xff);
			ov2715_write2( 0x4303, 0x00);
			ov2715_write2( 0x3a00, 0x78);
			ov2715_write2( 0x300f, 0x88);  
			ov2715_write2( 0x3011, 0x16);
			ov2715_write2( 0x3a1a, 0x06);
			ov2715_write2( 0x3a18, 0x00);
			ov2715_write2( 0x3a19, 0x7a);
			ov2715_write2( 0x3a13, 0x54);
			ov2715_write2( 0x382e, 0x0f);
			ov2715_write2( 0x381a, 0x1a);
			ov2715_write2( 0x401d, 0x02);
			
			ov2715_write2( 0x3a0f, 0x40);
			ov2715_write2( 0x3a10, 0x38);
			ov2715_write2( 0x3a1b, 0x48);
			ov2715_write2( 0x3a1e, 0x30);
			ov2715_write2( 0x3a11, 0x90);
			ov2715_write2( 0x3a1f, 0x10);

			ov2715_write2( 0x5680, 0x05);
			ov2715_write2( 0x5681, 0x00);
			ov2715_write2( 0x5682, 0x07);
			ov2715_write2( 0x5683, 0x80);
			ov2715_write2( 0x5684, 0x02);
			ov2715_write2( 0x5685, 0x58);
			ov2715_write2( 0x5686, 0x04);
			ov2715_write2( 0x5687, 0x38);


			ov2715_write2( 0x3800, 0x01);
			ov2715_write2( 0x3801, 0xba);


#if 1
			ov2715_write2( 0x3802, 0x00);
			ov2715_write2( 0x3803, 0x0b);
			ov2715_write2( 0x3804, 0x02);
			ov2715_write2( 0x3805, 0x80);
			ov2715_write2( 0x3806, 0x01);
			ov2715_write2( 0x3807, 0xe0);
#endif
			ov2715_write2( 0x3808, 0x02);
			ov2715_write2( 0x3809, 0x80);
			ov2715_write2( 0x380A, 0x01);
			ov2715_write2( 0x380B, 0xE0);
#if	0
			ov2715_write2( 0x380C, 0x09);
			ov2715_write2( 0x380D, 0x74);
			ov2715_write2( 0x380E, 0x04);
			ov2715_write2( 0x380F, 0x50);
#endif

			ov2715_write2( 0x3a00, 0x5c);
			ov2715_write2( 0x5001, 0x4f);
			ov2715_write2( 0x3406, 0x01);
			
			ov2715_write2( 0x3400, 0x03);
			ov2715_write2( 0x3401, 0x60);
			ov2715_write2( 0x3402, 0x04);
			ov2715_write2( 0x3403, 0x00);
			ov2715_write2( 0x3404, 0x06);
			ov2715_write2( 0x3405, 0x00);

			ov2715_write2( 0x3621, 0xd4);
			ov2715_write2( 0x3811, 0x01);
			ov2715_write2( 0x3818, 0xe0);	//Vertical Flip
			ov2715_write2( 0x370d, 0x0f);

		/*
			ov2715_write2( 0x3621, 0xD4);
			ov2715_write2( 0x3811, 0x03);
			ov2715_write2( 0x3818, 0xE1);	//Vertical Flip
			ov2715_write2( 0x370d, 0x0F);
		*/

			ov2715_write2( 0x4708, 0x03);	//Vsync Polarity Inversion
			ov2715_write2( 0x3500, 0x00);	//Exposure Time High
			ov2715_write2( 0x3503, 0x07);	// Manual Framerate, Manual Exposure

            //[[tifler:20140916 To remove StuckHandler Error]]
#if 0
			ov2715_write2( 0x350C, 0x01);//15);	// Manaul Frame Length High
			//ov2715_write2( 0x350D, 0x00);	// Manual Frame Length Low
#else
			ov2715_write2( 0x350C, 0x02);//15);	// Manaul Frame Length High
			ov2715_write2( 0x350D, 0x00);	// Manual Frame Length Low
#endif

			ov2715_write2( 0x3501, 0x20);	//Exposure Time Mid
			
			ov2715_write2( 0x300F, 0x88);	//In clock 18 out 27
			ov2715_write2( 0x3010, 0x20);
			ov2715_write2( 0x3011, 0x36);
			ov2715_write2( 0x3012, 0x01);

		break;


		case INPUTMODE_OV2715_1920x1080_10FPS :

			ov2715_write2( 0x4740, 0x00);
			ov2715_write2( 0x3103, 0x93);
			ov2715_write( 0x3008, 0x82);
			ov2715_write2( 0x3017, 0x7f);
			ov2715_write2( 0x3018, 0xfc);
			ov2715_write2( 0x3706, 0x61);
			ov2715_write2( 0x3712, 0x0c);
			ov2715_write2( 0x3630, 0x6d);

			ov2715_write2( 0x3621, 0x04);
			ov2715_write2( 0x3604, 0x60);
			ov2715_write2( 0x3603, 0xa7);
			ov2715_write2( 0x3631, 0x26);
			ov2715_write2( 0x3600, 0x04);
			ov2715_write2( 0x3620, 0x37);
			ov2715_write2( 0x3623, 0x00);
			ov2715_write2( 0x3702, 0x9e);
			ov2715_write2( 0x3703, 0x5c);
			ov2715_write2( 0x3704, 0x40);
			ov2715_write2( 0x370d, 0x0f);
			ov2715_write2( 0x3713, 0x9f);
			ov2715_write2( 0x3714, 0x4c);
			ov2715_write2( 0x3710, 0x9e);

			ov2715_write2( 0x3605, 0x05);
			ov2715_write2( 0x3606, 0x3f);
			ov2715_write2( 0x302d, 0x90);
			ov2715_write2( 0x370b, 0x40);
			ov2715_write2( 0x3716, 0x31);
			ov2715_write2( 0x3707, 0x52);

			ov2715_write2( 0x5181, 0x20);
			ov2715_write2( 0x518f, 0x00);
			ov2715_write2( 0x4301, 0xff);
			ov2715_write2( 0x4303, 0x00);
			ov2715_write2( 0x3a00, 0x78);
			ov2715_write2( 0x300f, 0x88);  
			ov2715_write2( 0x3011, 0x16);
			ov2715_write2( 0x3a1a, 0x06);
			ov2715_write2( 0x3a18, 0x00);
			ov2715_write2( 0x3a19, 0x7a);
			ov2715_write2( 0x3a13, 0x54);
			ov2715_write2( 0x382e, 0x0f);
			ov2715_write2( 0x381a, 0x1a);
			ov2715_write2( 0x401d, 0x02);
			
			ov2715_write2( 0x3a0f, 0x40);
			ov2715_write2( 0x3a10, 0x38);
			ov2715_write2( 0x3a1b, 0x48);
			ov2715_write2( 0x3a1e, 0x30);
			ov2715_write2( 0x3a11, 0x90);
			ov2715_write2( 0x3a1f, 0x10);

			ov2715_write2( 0x5680, 0x05);
			ov2715_write2( 0x5681, 0x00);
			ov2715_write2( 0x5682, 0x07);
			ov2715_write2( 0x5683, 0x80);
			ov2715_write2( 0x5684, 0x07);
			ov2715_write2( 0x5685, 0xA0);
			ov2715_write2( 0x5686, 0x04);
			ov2715_write2( 0x5687, 0x43);

			ov2715_write2( 0x3800, 0x01);
			ov2715_write2( 0x3801, 0xBA);
			
#if 1
			ov2715_write2( 0x3802, 0x00);
			ov2715_write2( 0x3803, 0x0A);
			ov2715_write2( 0x3804, 0x07);
			ov2715_write2( 0x3805, 0x80);
			ov2715_write2( 0x3806, 0x04);
			ov2715_write2( 0x3807, 0x38);
#endif
			ov2715_write2( 0x3808, 0x07);
			ov2715_write2( 0x3809, 0x80);
			ov2715_write2( 0x380A, 0x04);
			ov2715_write2( 0x380B, 0x38);
#if	0
			ov2715_write2( 0x380C, 0x09);
			ov2715_write2( 0x380D, 0x74);
			ov2715_write2( 0x380E, 0x04);
			ov2715_write2( 0x380F, 0x50);
#endif

			ov2715_write2( 0x3a00, 0x5c);
			ov2715_write2( 0x5001, 0x4f);
			ov2715_write2( 0x3406, 0x01);


			ov2715_write2( 0x3400, 0x03);
			ov2715_write2( 0x3401, 0x60);
			ov2715_write2( 0x3402, 0x04);
			ov2715_write2( 0x3403, 0x00);
			ov2715_write2( 0x3404, 0x06);
			ov2715_write2( 0x3405, 0x00);

			ov2715_write2( 0x3621, 0x14);
			ov2715_write2( 0x3811, 0x03);
			ov2715_write2( 0x3818, 0xe0);	//Vertical Flip
			ov2715_write2( 0x370d, 0x0f);

		/*
			ov2715_write2( 0x3621, 0xD4);
			ov2715_write2( 0x3811, 0x03);
			ov2715_write2( 0x3818, 0xE1);	//Vertical Flip
			ov2715_write2( 0x370d, 0x0F);
		*/

			ov2715_write2( 0x4708, 0x03);	//Vsync Polarity Inversion
			ov2715_write2( 0x3500, 0x00);	//Exposure Time High
			ov2715_write2( 0x3503, 0x07);	// Manual Framerate, Manual Exposure
			ov2715_write2( 0x350C, 0x01);//15);	// Manaul Frame Length High
			ov2715_write2( 0x350D, 0x00);	// Manual Frame Length Low

			ov2715_write2( 0x3501, 0x20);	//Exposure Time Mid
			
			ov2715_write2( 0x300F, 0x88);	//In clock 18 out 27
			ov2715_write2( 0x3010, 0x20);
			ov2715_write2( 0x3011, 0x36);
			ov2715_write2( 0x3012, 0x01);

		/*
			ov2715_write2( 0x3400, 0x04);
			ov2715_write2( 0x3401, 0x00);
			ov2715_write2( 0x3402, 0x04);
			ov2715_write2( 0x3403, 0x00);
			ov2715_write2( 0x3404, 0x04);
			ov2715_write2( 0x3405, 0x00);

			ov2715_write2( 0x3621, 0x14);
			ov2715_write2( 0x3811, 0x03);
			ov2715_write2( 0x3818, 0xE0);	//Vertical Flip
			ov2715_write2( 0x370d, 0x0F);

			ov2715_write2( 0x4708, 0x03);	//Vsync Polarity Inversion
			ov2715_write2( 0x3500, 0x00);	//Exposure Time High
			ov2715_write2( 0x3503, 0x07);	// Manual Framerate, Manual Exposure
			ov2715_write2( 0x350C, 0x00);//15);	// Manaul Frame Length High
			ov2715_write2( 0x350D, 0x20);	// Manual Frame Length Low

			ov2715_write2( 0x3501, 0x20);	//Exposure Time Mid
			
			ov2715_write2( 0x300F, 0x88);	//In clock 18 out 27
			ov2715_write2( 0x3010, 0x20);
			ov2715_write2( 0x3011, 0x36);
			ov2715_write2( 0x3012, 0x01);
		*/
		break;

		default :

		break;

	}


}

static uint8_t S_ucGrpIdx = 0;
static uint8_t S_isNewGrpCreated = 0;

static inline void openSensorGroup(void)
{

	if(	S_isNewGrpCreated == 0 && 
	(	S_ucCropConfigIdx                    != S_ucAppliedCropConfigIdx
	||	S_stRegister.ucPsRamCtrl0            != 0	//READ_BY_CACHE(Offset_psram_ctrl0)	//pjo no use for OV2715
	||	S_stRegister.uiExpoTime              != (uint32_t)(READ_BY_CACHE(Offset_ExposureMSB) << 16) + READ_BY_CACHE(Offset_Exposure)
	||	S_stRegister.usAG                    != READ_BY_CACHE(Offset_AG) 
	||	S_stRegister.usDG[DxOISP_CHANNEL_GR] != READ_BY_CACHE(Offset_DG_G)
	||	S_stRegister.usLineLength            != READ_BY_CACHE(Offset_line_length_pck)
	||	S_stRegister.usFrameLength           != READ_BY_CACHE(Offset_frame_length_lines)))
	{
		WRITE_DEVICE(Offset_group_access, S_ucGrpIdx);
		S_isNewGrpCreated = 1;
	}

}

static inline void closeSensorGroup(void)
{

	if(S_isNewGrpCreated) {
		S_isNewGrpCreated = 0;
		WRITE_DEVICE(Offset_group_access, (0x10 + S_ucGrpIdx));
		WRITE_DEVICE(Offset_group_access, (0xa0 + S_ucGrpIdx));
		S_uiCmdSendToFrameIdx = S_stCmd.uiFrameCount;
		//S_ucGrpIdx = (S_ucGrpIdx+1)%3; Omnivision says we can use the same group every time
	}

}

static inline uint8_t canSendCmdToSensor(void) {
	if(S_isStartIgnoringFrame) return 0; //Do not send any command when sensor is not in a stable state
	
	//if we are in the frame (not in the VBlank)
	if(S_uiOldFrameCount == S_stCmd.uiFrameCount) {
		if(S_stCmd.uiFrameCount > S_uiCmdSendToFrameIdx+1) //Send commands every 2 frames
			return 1; 
		else
			return 0;
	} else { //if we are in the vertical blanking
		if(
			S_ucCropConfigIdx          != S_ucAppliedCropConfigIdx
		||	S_stRegister.usLineLength  != READ_BY_CACHE(Offset_line_length_pck)
		||	S_stCmd.uiFrameCount        > S_uiCmdSendToFrameIdx+1)
			return 1; //we send cmd only if we change the image geometry (not in case of exposure modification)
		else
			return 0;
	}
}

extern int initdone;
static inline void applySettingsToSensor( void ) 
{
	uint8_t ucMode = 0;
	uint8_t ucStreamingMode = (uint8_t)READ_BY_CACHE(Offset_mode_select);
	uint32_t uiFrameCount = S_stCmd.uiFrameCount;

	if(ucStreamingMode == 0x42) {
		ucMode = 0;
	}
	else {
		ucMode = 1;
	}

//	if(initdone)ucMode = 1;
#if 0
	//	WRITE_BY_CACHE( Offset_Af                      ,                         S_stRegister.usAF);			//pjo use but not ported yet
		
		if(canSendCmdToSensor()) {
			if(ucMode) {
				openSensorGroup();
			}

	//		WRITE_BY_CACHE( Offset_timing_tc_reg21          ,               S_stRegister.ucTimingRegX );		//pjo no use
	//		WRITE_BY_CACHE( Offset_timing_tc_reg20          ,               S_stRegister.ucTimingRegY );		//pjo no use
	//		WRITE_BY_CACHE( Offset_psram_ctrl0              ,               S_stRegister.ucPsRamCtrl0 );		//pjo no use

	//		WRITE_BY_CACHE( Offset_ExposureMSB              ,            (S_stRegister.uiExpoTime>>16));	//pjo use but not ported yet
	//		WRITE_BY_CACHE( Offset_Exposure                 ,                 S_stRegister.uiExpoTime );		//pjo use but not ported yet
	//		WRITE_BY_CACHE( Offset_AG                       ,                       S_stRegister.usAG );			//pjo use but not ported yet
	//		WRITE_BY_CACHE( Offset_DG_G                     ,    S_stRegister.usDG[DxOISP_CHANNEL_GR] );		//pjo use but not ported yet
	//		WRITE_BY_CACHE( Offset_DG_R                     ,     S_stRegister.usDG[DxOISP_CHANNEL_R] );		//pjo use but not ported yet
	//		WRITE_BY_CACHE( Offset_DG_B                     ,     S_stRegister.usDG[DxOISP_CHANNEL_B] );		//pjo use but not ported yet
		
			closeSensorGroup();
		}
#endif
	
	if(!ucMode && S_stRegister.isStreaming) {
		S_stStatus.isSensorFireNeeded = 1;

#ifdef SENSOR_DEBUG_ENABLE
		ISP_PRINTF("Sensor Fire Needed Set!\n");
#endif

	}else {
		if(!S_stRegister.isStreaming && ucMode) {
			S_uiCmdSendToFrameIdx = (uint32_t)-1;
		}
		
		if(S_stRegister.isStreaming == 0)
			WRITE_BY_CACHE( Offset_mode_select , 0x42 );
		
	}
#ifdef SENSOR_DEBUG_ENABLE
	ISP_PRINTF("Sensor streaming mode = %d\n",S_stRegister.isStreaming);
#endif

	S_ucAppliedCropConfigIdx = S_ucCropConfigIdx;
	historyMngt(ucMode);
}

static inline void initCache ( void ) {
	uint32_t i;
	for (i=0; i<SIZE_CACHE_DATA; i++) {
		S_cache[i].iLastAccess = NO_ACCESS;
	}	
}

static inline void historyMngt(uint8_t ucMode)
{
	//S_uiIT[0]: latest command
	//S_uiIT[1]: oldest command
	static uint32_t S_uiIT[2];
	static uint16_t S_usAGain[2];
	static uint16_t S_usDGain[2];
	static uint16_t S_usOldLineLength;
	static uint16_t S_usOldOutputSizeX, S_usOldOutputSizeY;
	static uint8_t  S_ignoreFrameCnt       = 0;

	if(ucMode == 0) {
		S_uiIT   [1] = S_uiIT   [0] = ((READ_BY_CACHE(Offset_ExposureMSB)<<16) +  READ_BY_CACHE(Offset_Exposure))>>4;
		S_usAGain[1] = S_usAGain[0] = READ_BY_CACHE(Offset_AG);
		S_usDGain[1] = S_usDGain[0] = READ_BY_CACHE(Offset_DG_G);
	} else if( S_stCmd.uiFrameCount != S_uiOldFrameCount) {
		S_uiIT   [1] = S_uiIT   [0];
		S_uiIT   [0] = ((READ_BY_CACHE(Offset_ExposureMSB)<<16) +  READ_BY_CACHE(Offset_Exposure))>>4;
		S_usAGain[1] = S_usAGain[0]; 
		S_usAGain[0] = READ_BY_CACHE(Offset_AG);
		if(S_uiIT[1]   != S_uiIT   [0]
		|| S_usAGain[1]!= S_usAGain[0]) {
			S_usDGain[1] = S_usDGain[0]; 
			S_usDGain[0] = READ_BY_CACHE(Offset_DG_G);
		} else {
			S_usDGain[1] = S_usDGain[0] = READ_BY_CACHE(Offset_DG_G);
		}
	}
	S_uiAppliedIT     = S_uiIT[1];
	S_usAppliedAG     = S_usAGain[1];
	S_usAppliedDG     = S_usDGain[1];

	if(S_uiAppliedIT != S_uiIT[0] || S_usAppliedAG != S_usAGain[0]) {
		S_stStatus.isImageQualityStable = 0;
	} else {
		S_stStatus.isImageQualityStable = 1;
	}

	if (ucMode == 1
	&& 	(S_stRegister.usOutSizeX  != S_usOldOutputSizeX 
	||	 S_stRegister.usOutSizeY  != S_usOldOutputSizeY 
	||	 S_stRegister.usLineLength!= S_usOldLineLength )) {

#ifdef SENSOR_DEBUG_ENABLE
		ISP_PRINTF("S_stRegister.usOutSizeX = %d\n", S_stRegister.usOutSizeX);
		ISP_PRINTF("S_usOldOutputSizeX = %d\n", S_usOldOutputSizeX);
		
		ISP_PRINTF("S_stRegister.usOutSizeY = %d\n", S_stRegister.usOutSizeY);
		ISP_PRINTF("S_usOldOutputSizeY = %d\n", S_usOldOutputSizeY);

		ISP_PRINTF("S_stRegister.usLineLength = %d\n", S_stRegister.usLineLength);
		ISP_PRINTF("S_usOldLineLength = %d\n", S_usOldLineLength);
#endif
	
		S_isStartIgnoringFrame = 1;
	}
	else if(S_stStatus.isSensorFireNeeded) {

#ifdef SENSOR_DEBUG_ENABLE
		ISP_PRINTF("isSensorFireNeeded = 0x%x\n", (unsigned int)S_stStatus.isSensorFireNeeded);
#endif
		S_isStartIgnoringFrame = 1;
	}

	if(S_isStartIgnoringFrame && S_uiOldFrameCount != S_stCmd.uiFrameCount) {

#ifdef SENSOR_DEBUG_ENABLE

		ISP_PRINTF("S_isStartIgnoringFrame = 0x%x\n", (unsigned int)S_isStartIgnoringFrame);
		ISP_PRINTF("S_uiOldFrameCount = 0x%x\n", S_uiOldFrameCount);
		ISP_PRINTF("S_stCmd.uiFrameCount = 0x%x\n", S_stCmd.uiFrameCount);
#endif
		S_ignoreFrameCnt++;
		S_stStatus.isFrameInvalid = 1;

#ifdef SENSOR_DEBUG_ENABLE
		ISP_PRINTF("FrameInvalid = %d\n", S_stStatus.isFrameInvalid);
#endif

		if(S_ignoreFrameCnt == NB_FRAME_TO_IGNORE) {
			S_ignoreFrameCnt = 0;
			S_isStartIgnoringFrame = 0;
		}
	} else {
		S_stStatus.isFrameInvalid = 0;
	}

	S_uiOldFrameCount  = S_stCmd.uiFrameCount;
	S_usOldLineLength  = S_stRegister.usLineLength;
	S_usOldOutputSizeX = S_stRegister.usOutSizeX;
	S_usOldOutputSizeY = S_stRegister.usOutSizeY;
}

static inline void getFrameValidTime ( ts_SENSOR_Status* pttsStatus ) {
	uint16_t usSizeY         = READ_BY_CACHE( Offset_y_output_size );
	uint16_t usLineLengthPck = READ_BY_CACHE( Offset_line_length_pck );
	pttsStatus->uiFrameValidTime =  ((uint64_t)(usSizeY * usLineLengthPck) << 20) / ((pttsStatus->uiPixelClock>>12)*1000);
}

/*****************************************************************************/
/* Exported functions definition                                             */
/*****************************************************************************/

void DxOSensor_OVT2715_Initialize (struct SENSOR *sensor) {

    struct OVT2715Private *priv;

    __CHECK_LINE__;

    ASSERT(!THIS_SENSOR);

    THIS_SENSOR = sensor;

    priv = calloc(1, sizeof(*priv));
    ASSERT(priv);

    //priv->fd = open(I2C_DEVICE, O_RDWR);
    //ASSERT(priv->fd > 0);

    sensor->priv = priv;

    priv->inputMode = defaultPrivate.inputMode;
	priv->inputModeDevice = defaultPrivate.inputModeDevice;
	
	// Init all the global varibles
	memset( &S_stCmd,      0, sizeof(S_stCmd) );
	memset( &S_stStatus,   0, sizeof(S_stStatus) );
	memset( &S_stRegister, 0, sizeof(S_stRegister) );
	memset( &S_cache,      0, sizeof(ts_cache) );

	S_uiSensorPixelClock     = 0;
	S_uiAppliedIT            = 0;
	S_usAppliedAG            = 0;
	S_usAppliedDG            = 0;
	S_ucCropConfigIdx        = 0;
	S_ucAppliedCropConfigIdx = (uint8_t) -1;
	S_isStartIgnoringFrame   = 0;
	S_uiOldFrameCount        = (uint32_t)-1;
	S_uiCmdSendToFrameIdx    = (uint32_t)-1;
#ifdef FLASH_ENABLE
	S_usFlashPower           = 0;
#endif
	initCache();
	setStaticSensorConfiguration (NB_MIPI_LANES);
	getCapabilities    ( &S_stStatus);
	S_uiSensorPixelClock = computePixelClock(); 
	
	initSensorCmd      ( &S_stCmd, &S_stStatus);
	setCrop            ( &S_stRegister);
	setOrientation     ( &S_stRegister, S_stCmd.stImageFeature.eOrientation);
	setDecimation      ( &S_stRegister, S_stCmd.stImageFeature.ucHorDownsamplingFactor,
	                     S_stCmd.stImageFeature.ucVertDownsamplingFactor);
	setGain            ( &S_stRegister, S_stCmd.stShootingParam.usAGain,
	                     S_stCmd.stShootingParam.usDGain);

	setMinimalBlanking ( &S_stRegister, S_stCmd.stImageFeature.usMinVertBlanking,S_stCmd.stImageFeature.uiMinHorBlanking);
	setExposureTime    ( &S_stRegister );
	setAF              ( &S_stRegister );
	applySettingsToSensor();

	ISP_PRINTF("Sensor Init Done\n");


#ifdef FLASH_ENABLE
	setFlash(0);
	S_usFlashPower = 0;
#endif
}

void DxOSensor_OVT2715_Uninitialize (struct SENSOR *sensor) {
    struct OVT2715Private *priv = (struct OVT2715Private *)sensor->priv;
    //close(priv->fd);
    free(priv);
    sensor->priv = NULL;
    THIS_SENSOR = (struct SENSOR *)NULL;
	//DeviceDeinit();
	return;
}

void DxOSensor_OVT2715_SetStartGrp (struct SENSOR *sensor) {
	return;
}


void DxOSensor_OVT2715_Set (
        struct SENSOR *sensor, uint16_t usOffset, uint16_t usSize, void* pBuf )
{
	uint16_t usTmpSize = usSize;
	if ( (usOffset + usSize)  > sizeof(S_stCmd)) {
		assertf1(( (usOffset + usSize)  <= sizeof(S_stCmd)), " uncorrect offset (%d)", (usOffset + usSize) );
		usTmpSize = sizeof(S_stCmd) - usOffset;
	}
	memcpy((uint8_t*)&S_stCmd + usOffset,pBuf, usTmpSize );
}

void DxOSensor_OVT2715_SetEndGrp(struct SENSOR *sensor)
{
	setMode              ( &S_stRegister, S_stCmd.isSensorStreaming                      );
	setCrop              ( &S_stRegister);
	setOrientation       ( &S_stRegister, S_stCmd.stImageFeature.eOrientation            );
	setDecimation        ( &S_stRegister, S_stCmd.stImageFeature.ucHorDownsamplingFactor,
	                       S_stCmd.stImageFeature.ucVertDownsamplingFactor);
	setGain              ( &S_stRegister, S_stCmd.stShootingParam.usAGain,
	                       S_stCmd.stShootingParam.usDGain);
#ifndef FORCE_FULL_FRAME
	setMinimalBlanking   ( &S_stRegister, S_stCmd.stImageFeature.usMinVertBlanking,
	                       S_stCmd.stImageFeature.uiMinHorBlanking        );
#endif
	setExposureTime      ( &S_stRegister);
	
	if(S_stRegister.usFrameLength&0x1)
		S_stRegister.usFrameLength++;
	
	setAF                ( &S_stRegister);

#ifdef FLASH_ENABLE
	if(S_stCmd.usFlashPower != S_usFlashPower) {
		setFlash(S_stCmd.usFlashPower);
		S_usFlashPower = S_stCmd.usFlashPower;
	}
#endif


	applySettingsToSensor();
}

void DxOSensor_OVT2715_Get (
        struct SENSOR *sensor, uint16_t usOffset, uint16_t usSize, void* pBuf) {
	uint16_t usTmpSize = usSize;
	if ( (usOffset + usSize)  > sizeof(S_stStatus)) {
		assertf1(( (usOffset + usSize) <= sizeof(S_stStatus)), " uncorrect offset (%d)", (usOffset + usSize) );
		usTmpSize = sizeof(S_stStatus) -usOffset;
	}
	memcpy(pBuf,(uint8_t *)&S_stStatus+usOffset,usTmpSize);
}

void DxOSensor_OVT2715_GetEndGrp (struct SENSOR *sensor) {
	return;
}

void DxOSensor_OVT2715_GetStartGrp (struct SENSOR *sensor)
{
	S_stStatus.uiPixelClock         = S_uiSensorPixelClock<<12;

	getMode            ( &S_stStatus );
	getImageFeature    ( &S_stStatus );
	getFrameRate       ( &S_stStatus );
	getExposureTime    ( &S_stStatus );
	getAnalogGain      ( &S_stStatus );
	getDigitalGain     ( &S_stStatus );
	getPixelClock      ( &S_stStatus );
	getFrameValidTime  ( &S_stStatus );
	getAF              ( &S_stStatus );
	
#ifdef FLASH_ENABLE
	S_stStatus.stAppliedCmd.usFlashPower = S_stCmd.usFlashPower;
#else
	S_stStatus.stAppliedCmd.usFlashPower = 0;
#endif
}

void DxOSensor_OVT2715_Fire (struct SENSOR *sensor)
{
    struct OVT2715Private *priv = (struct OVT2715Private *)sensor->priv;

	if(priv->inputModeDevice == INPUTMODE_OV2715) {

//		if(initdone==0)
		{
			if(S_stRegister.isStreaming){
				WRITE_BY_CACHE( Offset_mode_select, 0x02 );	
			}
			else {
				WRITE_BY_CACHE( Offset_mode_select, 0x42 );
			}
		}
	}

	S_stStatus.isSensorFireNeeded = 0;
}

/*****************************************************************************/

#define ARRAY_SIZE(a)                       (sizeof(a) / sizeof(a[0]))

struct OV2715_SENSOR_MODE {
    struct SENSOR_MODE mode;
    uint32_t inputMode;
    uint32_t inputModeDevice;
};

static struct OV2715_SENSOR_MODE OV2715_Modes[] = {
    {
        .mode = {
            .width = 640,
            .height = 480,
            .fps = 6,
        },
        .inputMode = INPUTMODE_OV2715_640x480_6FPS,
        .inputModeDevice = INPUTMODE_OV2715,
    },
    {
        .mode = {
            .width = 640,
            .height = 480,
            .fps = 8,
        },
        .inputMode = INPUTMODE_OV2715_640x480_8FPS,
        .inputModeDevice = INPUTMODE_OV2715,
    },
    {
        .mode = {
            .width = 1920,
            .height = 1080,
            .fps = 10,
        },
        .inputMode = INPUTMODE_OV2715_1920x1080_10FPS,
        .inputModeDevice = INPUTMODE_OV2715,
    },
};

static int DxOSensor_OVT2715_GetSensorMode(
        uint32_t modeId, struct SENSOR_MODE *mode)
{
    if (modeId >= ARRAY_SIZE(OV2715_Modes))
        return -1;

    memcpy(mode, &OV2715_Modes[modeId].mode, sizeof(*mode));

    return 0;
}

static void DxOSensor_OVT2715_SetSensorMode(uint32_t modeId)
{
    ASSERT(modeId < ARRAY_SIZE(OV2715_Modes));
    defaultPrivate.inputMode = OV2715_Modes[modeId].inputMode;
    defaultPrivate.inputModeDevice = OV2715_Modes[modeId].inputModeDevice;
}

/*****************************************************************************/

static struct SENSOR_API OVT2715_SensorAPI = {
    .init = DxOSensor_OVT2715_Initialize,
    .exit = DxOSensor_OVT2715_Uninitialize,
    .commandGroupOpen = DxOSensor_OVT2715_SetStartGrp,
    .commandSet = DxOSensor_OVT2715_Set,
    .commandGroupClose = DxOSensor_OVT2715_SetEndGrp,
    .statusGroupOpen = DxOSensor_OVT2715_GetStartGrp,
    .statusGet = DxOSensor_OVT2715_Get,
    .statusGroupClose = DxOSensor_OVT2715_GetEndGrp,
    .fire = DxOSensor_OVT2715_Fire,
    .getSensorMode = DxOSensor_OVT2715_GetSensorMode,
    .setSensorMode = DxOSensor_OVT2715_SetSensorMode,
};

/*****************************************************************************/

struct SENSOR OVT2715_Sensor = {
    .name = "OVT2715 2M-Sensor",
    .api = &OVT2715_SensorAPI,
};
