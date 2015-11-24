//=============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
//=============================================================================

#ifndef __DXO_ISP_H__
#define __DXO_ISP_H__

#include <stdint.h>
#include <stddef.h>

//=============================================================================
//
// Constant definitions:
//
//=============================================================================

typedef enum {
	DxOISP_MODE_STANDBY = 0
,	DxOISP_MODE_IDLE
,	DxOISP_MODE_PREVIEW
,	DxOISP_MODE_CAPTURE_A
,	DxOISP_MODE_CAPTURE_B
,	DxOISP_MODE_VIDEOREC
,	DxOISP_NB_MODE
} te_DxOISPMode;

typedef enum {
	DxOISP_CHANNEL_GR = 0
,	DxOISP_CHANNEL_R
,	DxOISP_CHANNEL_B 
,	DxOISP_CHANNEL_GB
,	DxOISP_NB_CHANNEL               // Used for array dimensions
} te_DxOISPChannel;

typedef enum {
	DxOISP_OUTPUT_DISPLAY = 0
,	DxOISP_OUTPUT_VIDEO
,	DxOISP_OUTPUT_CAPTURE
,	DxOISP_OUTPUT_FD
,	DxOISP_NB_OUTPUT                // Used for array dimensions
} te_DxOISPOutput;

typedef enum {
	DxOISP_FLIP_OFF_MIRROR_OFF = 0
,	DxOISP_FLIP_OFF_MIRROR_ON
,	DxOISP_FLIP_ON_MIRROR_OFF
,	DxOISP_FLIP_ON_MIRROR_ON
,	DxOISP_NB_ORIENTATION
} te_DxOISPImageOrientation;

typedef enum {
	DxOISP_FORMAT_YUV422 = 0
,	DxOISP_FORMAT_YUV422_NV
,	DxOISP_FORMAT_YUV420
,	DxOISP_FORMAT_RGB565
,	DxOISP_FORMAT_RGB888
,	DxOISP_FORMAT_RAW
,	DxOISP_NB_FORMAT
} te_DxOISPFormat;

typedef enum {
	DxOISP_RGBORDER_RGB = 0
,	DxOISP_RGBORDER_RBG
,	DxOISP_RGBORDER_GRB
,	DxOISP_RGBORDER_GBR
,	DxOISP_RGBORDER_BGR
,	DxOISP_RGBORDER_BRG
,	DxOISP_NB_RGBORDER
} te_DxOISPRgbOrder;

typedef enum {
	DxOISP_YUV422ORDER_YUYV = 0
,	DxOISP_YUV422ORDER_YYUV
,	DxOISP_YUV422ORDER_UYYV
,	DxOISP_YUV422ORDER_UVYY
,	DxOISP_YUV422ORDER_UYVY
,	DxOISP_YUV422ORDER_YUVY
,	DxOISP_YUV422ORDER_YVYU
,	DxOISP_YUV422ORDER_YYVU
,	DxOISP_YUV422ORDER_VYYU
,	DxOISP_YUV422ORDER_VUYY
,	DxOISP_YUV422ORDER_VYUY
,	DxOISP_YUV422ORDER_YVUY
,	DxOISP_NB_YUV422ORDER
} te_DxOISPYuv422Order;

typedef enum {
	DxOISP_YUV420ORDER_YYU = 0
,	DxOISP_YUV420ORDER_YUY
,	DxOISP_YUV420ORDER_UYY
,	DxOISP_YUV420ORDER_YYV
,	DxOISP_YUV420ORDER_YVY
,	DxOISP_YUV420ORDER_VYY
,	DxOISP_NB_YUV420ORDER
} te_DxOISPYuv420Order;

typedef enum {
	DxOISP_ENCODING_YUV_601FS = 0
,	DxOISP_ENCODING_YUV_601FU
,	DxOISP_ENCODING_YUV_601CU
,	DxOISP_ENCODING_YUV_601CS
,	DxOISP_ENCODING_YUV_709FS
,	DxOISP_ENCODING_YUV_709FU
,	DxOISP_ENCODING_YUV_709CU
,	DxOISP_ENCODING_YUV_709CS
,	DxOISP_NB_ENCODING
} te_DxOISPEncoding;

typedef enum {
	DxOISP_SENSOR_PHASE_GRBG = 0
,	DxOISP_SENSOR_PHASE_RGGB
,	DxOISP_SENSOR_PHASE_BGGR
,	DxOISP_SENSOR_PHASE_GBRG
,	DxOISP_NB_SENSOR_PHASE
} te_DxOISPSensorPhase;

typedef enum {
	DxOISP_AFK_MODE_AUTO = 0
,	DxOISP_AFK_MODE_MANUAL
,	DxOISP_NB_AFK_MODE
} te_DxOISPAfkMode;

typedef enum {
	DxOISP_AFK_FREQ_0HZ = 0
,	DxOISP_AFK_FREQ_50HZ
,	DxOISP_AFK_FREQ_60HZ
,	DxOISP_NB_AFK_FREQ
} te_DxOISPAfkFrequency;

typedef enum {
	DxOISP_AE_MODE_AUTO = 0
,	DxOISP_AE_MODE_MANUAL 
,	DxOISP_AE_MODE_ISO_PRIORITY
,	DxOISP_AE_MODE_ISO_PRIORITY_NO_BANDING
,	DxOISP_AE_MODE_SHUTTER_SPEED_PRIORITY
,	DxOISP_NB_AE_MODE
} te_DxOISPAeMode;

typedef enum {
	DxOISP_AWB_MODE_AUTO = 0
,	DxOISP_AWB_MODE_MANUAL  
,	DxOISP_AWB_MODE_PRESET
,	DxOISP_NB_AWB_MODE
} te_DxOISPAwbMode;

typedef enum {
	DxOISP_AWB_PRESET_HORIZON
,	DxOISP_AWB_PRESET_TUNGSTEN
,	DxOISP_AWB_PRESET_TL84
,	DxOISP_AWB_PRESET_COOLWHITE
,	DxOISP_AWB_PRESET_D45
,	DxOISP_AWB_PRESET_DAYLIGHT
,	DxOISP_AWB_PRESET_CLOUDY
,	DxOISP_AWB_PRESET_SHADE
,	DxOISP_NB_AWB_PRESET
} te_DxOISPAwbPreset;

typedef enum {
	DxOISP_FLASH_MODE_AUTO = 0
,	DxOISP_FLASH_MODE_MANUAL
,	DxOISP_FLASH_MODE_OFF
,	DxOISP_FLASH_MODE_ON
,	DxOISP_NB_FLASH_MODE
} te_DxOISPFlashMode;

typedef enum {
	DxOISP_AF_MODE_AUTO = 0
,	DxOISP_AF_MODE_ONESHOT
,	DxOISP_AF_MODE_MANUAL
,	DxOISP_AF_MODE_HYPERFOCAL
,	DxOISP_NB_AF_MODE
} te_DxOISPAfMode;

typedef enum {
	DxOISP_AF_EVENT_NO_EVENT = 0
,	DxOISP_AF_EVENT_UPDATE_FOCUS_A
,	DxOISP_AF_EVENT_UPDATE_FOCUS_B
,	DxOISP_NB_AF_EVENT
} te_DxOISPAfEvent;

typedef enum {
	DxOISP_AF_OFF = 0
,	DxOISP_AF_FOCUS_FAILED
,	DxOISP_AF_FOCUSING
,	DxOISP_AF_FOCUSED
} te_DxOISPAfConvergence;

typedef enum {
	DxOISP_NO_ERROR    = 0
,	DxOISP_PIXEL_STUCK = 1
,	DxOISP_SHORT_FRAME = 2
} te_DxOISPError;

typedef enum {
	DxOISP_COARSE_GAIN_LOWLIGHT = 0
,	DxOISP_COARSE_GAIN_STANDARD
,	DxOISP_COARSE_GAIN_HIGHLIGHT
,	DxOISP_NB_COARSE_GAIN                 // Used for array dimensions
} te_DxOISPCoarseGain;

typedef enum {
	DxOISP_COARSE_ILLUMINANT_HORIZON = 0
,	DxOISP_COARSE_ILLUMINANT_TUNGSTEN
,	DxOISP_COARSE_ILLUMINANT_FLUO
,	DxOISP_COARSE_ILLUMINANT_OUTDOOR
,	DxOISP_COARSE_ILLUMINANT_FLASH
,	DxOISP_NB_COARSE_ILLUMINANT           // Used for array dimensions
} te_DxOISPCoarseIlluminant;

typedef enum {
	DxOISP_FINE_ILLUMINANT_HORIZON = 0
,	DxOISP_FINE_ILLUMINANT_TUNGSTEN
,	DxOISP_FINE_ILLUMINANT_TL84
,	DxOISP_FINE_ILLUMINANT_COOLWHITE
,	DxOISP_FINE_ILLUMINANT_D45
,	DxOISP_FINE_ILLUMINANT_DAYLIGHT
,	DxOISP_FINE_ILLUMINANT_CLOUDY
,	DxOISP_FINE_ILLUMINANT_SHADE
,	DxOISP_FINE_ILLUMINANT_FLASH
,	DxOISP_NB_FINE_ILLUMINANT             // Used for array dimensions
} te_DxOISPfineIlluminant;

typedef enum {
	DxOISP_FINE_GAIN_ISORANGE1 = 0
,	DxOISP_FINE_GAIN_ISORANGE2
,	DxOISP_FINE_GAIN_ISORANGE3
,	DxOISP_FINE_GAIN_ISORANGE4
,	DxOISP_FINE_GAIN_ISORANGE5
,	DxOISP_NB_FINE_GAIN                   // Used for array dimensions
} te_DxOISPFineGain;

typedef enum {
	DxOISP_EXPOSURE_LOW = 0
,	DxOISP_EXPOSURE_MEDIUM
,	DxOISP_EXPOSURE_HIGH
,	DxOISP_NB_EXPOSURE_TIME               // Used for array dimensions
} te_DxOISPExposureTime;

#define DxOISP_NB_FACES                          6
#define DxOISP_FPS_FRAC_PART                     4
#define DxOISP_EVBIAS_FRAC_PART                  5 
#define DxOISP_EXPOSURETIME_FRAC_PART           16
#define DxOISP_APERTURE_FRAC_PART                8
#define DxOISP_GAIN_FRAC_PART                    8
#define DxOISP_WBSCALE_FRAC_PART                 8
#define DxOISP_ZOOMFOCAL_FRAC_PART               8
#define DxOISP_SYSTEM_CLOCK_FRAC_PART           16
#define DxOISP_MIN_INTERFRAME_TIME_FRAC_PART     8
#define DxOISP_MAX_BANDWIDTH_FRAC_PART           4

//=============================================================================
//
// Type definitions:
//
//=============================================================================

typedef struct {
	uint16_t  usXAddrStart;
	uint16_t  usYAddrStart;
	uint16_t  usXAddrEnd;
	uint16_t  usYAddrEnd;
} ts_DxOISPCrop;

typedef struct {
	uint8_t       isEnabled;
	uint8_t       eFormat;   // te_DxOISPFormat
	uint8_t       eOrder;    // te_DxOISPRgbOrder, te_DxOISPYuv422Order or te_DxOISPYuv420Order according to eFormat
	uint8_t       eEncoding; // te_DxOISPEncoding
	ts_DxOISPCrop stCrop;
	uint16_t      usSizeX;
	uint16_t      usSizeY;
} ts_DxOISPOutput;

typedef struct {
	ts_DxOISPCrop stFaces  [DxOISP_NB_FACES];
	uint8_t       ucWeights[DxOISP_NB_FACES]; 
} ts_DxOISPRoi;

typedef struct {
	struct {
		uint8_t          eMode;                  // te_DxOISPMode
		uint8_t          ucSourceSelection;      // Sensor selection
		uint8_t          ucCalibrationSelection; // Calibration data selection
		uint8_t          ucNbCaptureFrame;       // Number of frames processed in capture mode
		uint8_t          isTNREnabled;           // Temporal Noise Reduction enabled
		uint8_t          eImageOrientation;      // te_DxOISPImageOrientation
		uint8_t          ucInputDecimation;      // decimation applied by the sensor and/or the ISP (depending on sensor capabilities) before processing (must be >= 1)
		uint16_t         usFrameRate;            // FPS expressed in 12.4
		uint16_t         usZoomFocal;            // Ratio between focal distance and sensor diagonal size expressed in 8.8
		uint16_t         usFrameToFrameLimit;    // Maximum movement of any frame corner relative to width/height expressed in 0.16 (0xFFFF means inifnity)
		ts_DxOISPOutput  stOutput[DxOISP_NB_OUTPUT];
		uint32_t         uiFrameBufferBaseAddr;  // Start address of the frame buffer
		uint32_t         uiFrameBufferSize;      // Frame buffer size in Bytes
	} stControl;
	struct {
		struct {
			uint8_t      eMode;                              // te_DxOISPAfkMode
			struct {
				uint8_t  eFrequency;                         // te_DxOISPAfkFrequency
			} stForced;
		} stAfk;
		struct {
			uint8_t      eMode;                              // te_DxOISPAeMode
			struct {
				int8_t   cEvBias;                            // no unit expressed in 3.5
				uint8_t  isLock;                             // Boolean
				uint32_t uiMaxExposureTime;                  // ms expressed in 16.16
				uint32_t uiMaxExposureTimeCapture;           // ms expressed in 16.16
			} stControl;
			struct {
				uint32_t uiExposureTime;                     // ms expressed in 16.16
				uint16_t usAperture;                         // F number expressed in 8.8
				uint16_t usIspGain;                          // Expressed in 8.8
				uint16_t usIspTcGain;                        // Expressed in 8.8
				uint16_t usAnalogGain [DxOISP_NB_CHANNEL];   // Expressed in 8.8  
				uint16_t usDigitalGain[DxOISP_NB_CHANNEL];   // Expressed in 8.8  
			} stForced;
			ts_DxOISPRoi     stRoi;                          // Region Of Interest
		} stAe;
		struct {
			uint8_t      eMode;                              // te_DxOISPAwbMode
			struct {
				uint8_t  ePreset;                            // te_DxOISPAwbPreset
				uint16_t usWbRedScale;                       // Expressed in 8.8
				uint16_t usWbBlueScale;                      // Expressed in 8.8
			} stForced;
		} stAwb;
		struct {
			uint8_t      eMode;                              // te_DxOISPFlashMode
			struct {
				uint16_t usPower;                            // Lux at 1m on optical axis expressed in 16.0 (0 = off)
			} stForced;
		} stFlash;
		struct {
			uint8_t      eMode;                              // te_DxOISPAfMode
			uint8_t      eEvent;                             // te_DxOISPAfEvent
			struct { 
				uint16_t usPosition;                         // AF register
			} stForced;
			ts_DxOISPRoi     stRoi;                          // Region of interest
		} stAf;
	} stCameraControl;
} ts_DxOIspSyncCmd;

typedef struct {
	struct {
		uint32_t  uiSystemClock;           // Frequency of the DxOISP. (in MHz expressed in 16.16)
		uint32_t  uiMaxBandwidth;          // Maximum instantaneous bandwidth supported (in MBit/s expressed in 24.4)
		uint16_t  usMinInterframeTime;     // Interrupt reaction time + Execution time until start of frame. (in ms expressed in 8.8)
	} stSystem;
	struct {
		uint8_t   ucLensShading   [DxOISP_NB_COARSE_GAIN][DxOISP_NB_COARSE_ILLUMINANT];
		uint8_t   ucColorFringing [DxOISP_NB_COARSE_ILLUMINANT];
		uint8_t   ucSharpness     [DxOISP_NB_COARSE_ILLUMINANT];
	} stLensCorrection;
	struct {
		uint8_t   ucBrightPixels     [DxOISP_NB_EXPOSURE_TIME];
		uint8_t   ucDarkPixels       [DxOISP_NB_EXPOSURE_TIME];
	} stSensorDefectCorrection;
	struct {
		uint8_t   ucSegmentation     [DxOISP_NB_FINE_GAIN];
		uint8_t   ucGrain            [DxOISP_NB_FINE_GAIN];
		uint8_t   ucLuminanceNoise   [DxOISP_NB_FINE_GAIN];
		uint8_t   ucChrominanceNoise [DxOISP_NB_FINE_GAIN];
	} stNoiseAndDetails;
	struct {
		uint8_t   ucWbBiasRed  [DxOISP_NB_FINE_ILLUMINANT];
		uint8_t   ucWbBiasBlue [DxOISP_NB_FINE_ILLUMINANT];
		int8_t    cSaturation [DxOISP_NB_COARSE_GAIN][DxOISP_NB_COARSE_ILLUMINANT]; 
	} stColor;
	struct {
		int8_t    cBrightness;
		int8_t    cContrast;
		uint8_t   ucBlackLevel[DxOISP_NB_COARSE_GAIN][DxOISP_NB_COARSE_ILLUMINANT];
		uint8_t   ucWhiteLevel[DxOISP_NB_COARSE_GAIN][DxOISP_NB_COARSE_ILLUMINANT];
		uint8_t   ucGamma     [DxOISP_NB_COARSE_GAIN][DxOISP_NB_COARSE_ILLUMINANT];
	} stExposure;
	uint8_t   isDebugEnable;  // Only available for dbg library 
} ts_DxOIspAsyncCmd;

typedef struct {
	struct {
		uint16_t usAnalogGain [DxOISP_NB_CHANNEL];   // Expressed in 8.8
		uint16_t usDigitalGain[DxOISP_NB_CHANNEL];   // Expressed in 8.8
		uint16_t usIspDigitalGain;                   // Expressed in 8.8
		uint32_t uiExposureTime;                     // ms expressed in 16.16
		uint16_t usAperture;                         // F number expressed in 8.8
		uint16_t usCurrentLuminance;                 // Exposure Level as seen by AE
	} stAe;
	struct {
		uint8_t  eSelectedPreset;                    // te_DxOISPAwbPreset
		uint16_t usWbRedScale;                       // Expressed in 8.8
		uint16_t usWbBlueScale;                      // Expressed in 8.8
	} stAwb;
	struct {
		uint8_t  eConvergence;                       // te_DxOISPAfConvergence
		uint16_t usPosition;                         // AF register
		uint32_t uiSharpness;                        // Level of Sharpness
	} stAf;
	struct {
		uint8_t  isRequiredForCapture;               // Boolean: if 1 the flash will be fired for the next capture
		uint16_t usPower;                            // Lux at 1m on optical axis expressed in 16.0 (0 = off)
	} stFlash;
	struct {
		uint8_t  eFrequency;                         // te_DxOISPAfkFrequency
	} stAfk;
	uint8_t      ucSensorDownsamplingX;
	uint8_t      ucSensorDownsamplingY;
	uint8_t      ucSensorBinningX;
	uint8_t      ucSensorBinningY;
	uint8_t      ucISPDownsamplingX;
	uint8_t      ucISPDownsamplingY;
	uint8_t      ucISPBinningX;
	uint8_t      ucISPBinningY;
	uint16_t     usPedestal;                         // Minimal value of a pixel
	uint16_t     usZoomFocal;                        // Ratio between focal distance and sensor diagonal size expressed in 8.8
} ts_DxOIspImageInfo;

typedef struct {
	struct {
		uint8_t        eMode;                 // te_DxOISPMode
		uint8_t        isTNREnabled;          // Temporal Noise Reduction enabled
		uint8_t        ucErrorCode;           // te_DxOISPError
		uint16_t       usFrameRate;           // Fps expressed in (12.4)
		uint32_t       uiFrameCount;
	} stSystem;
	ts_DxOIspImageInfo stImageInfo;
	ts_DxOISPOutput    stOutput[DxOISP_NB_OUTPUT];
} ts_DxOIspSyncStatus;

typedef struct {
	struct {
		struct {
			uint16_t usX;
			uint16_t usY;
		} stMaxSize[DxOISP_NB_OUTPUT];
		uint8_t  ucRawPixelWidth;
	} stHardware;
	struct {
		uint8_t  isAfk;
		uint8_t  isAe;
		uint8_t  isAwb;
		uint8_t  isFlash;
		uint8_t  isAf;
	} stCameraControl;
} ts_DxOIspCapabilities;

typedef struct {
	uint8_t      enableInterframeCmdOnly;       // Boolean value
	struct {
		uint8_t  ucPixelWidth;                  // Pixel width in bits (8, 10 or 12)
		uint8_t  ePhase;                        // te_DxOISPSensorPhase
		uint8_t  isPhaseStatic;                 // Is the phase the same when orientation change
		uint8_t  eNaturalOrientation;           // te_DxOISPImageOrientation natural orientation for users
		uint16_t usBayerWidth;                  // In Bayer, native sensor width
		uint16_t usBayerHeight;                 // In Bayer, native sensor height
		uint16_t usMinHorBlanking;              // Minimum horizontal blanking expressed in pixels
		uint16_t usMinVertBlanking;             // Minimum vertical blanking expressed in lines
		uint8_t  ucMaxHorDownsamplingFactor;    // Maximal horizontal downsampling that can be provided by the sensor
		uint8_t  ucMaxVertDownsamplingFactor;   // Maximal vertical downsampling that can be provided by the sensor
		uint8_t  ucMaxDelayImageFeature;        // Delay max in frame to reach a new crop/downsampling/blanking configuration
		uint8_t  ucMaxDelayImageQuality;        // Delay max in frame to reach a given image quality
		uint8_t  ucNbUpperDummyLines;           // Number of dummy lines sent by the sensor on top of the image to the DxO ISP
		uint8_t  ucNbLeftDummyColumns;          // Number of dummy columns sent by the sensor on the left of the image to the DxO ISP
		uint8_t  ucNbLowerDummyLines;           // Number of dummy lines sent by the sensor on bottom of the image to the DxO ISP
		uint8_t  ucNbRightDummyColumns;         // Number of dummy columns sent by the sensor on the right of the image to the DxO ISP
	} stImageFeature;
	struct {
		uint16_t  usIso;                        // ISO for AGain = DGain = 1 
		uint16_t  usMaxAGain;                   // Maximum analog gain set to the sensor. expressed in 8.8
		uint16_t  usMaxDGain;                   // Maximum digital gain set to the sensor. expressed in 8.8
	} stShootingParam;
	struct {
		uint16_t usMaxPower;                    // In Cd on optical axis (16.0) (0 = no flash)
	} stFlash;
	struct {
		uint16_t usMin;                         // AF register min value
		uint16_t usMax;                         // AF register max value if (min = max) then no mechanical AF
		struct {
			uint16_t  usMinDefocus;             // Minimal defocus position 
			uint16_t  usMidDefocus;             // Intermediate position to define the direction of a new search
			uint16_t  usMaxDefocus;             // Maximal defocus positions, if max is 0 control params values are applied
		} stPerUnit;
	} stAf;               
	struct {
		uint16_t usMin;                         // Ratio between focal distance and sensor diagonal size expressed in 8.8
		uint16_t usMax;                         // Ratio between focal distance and sensor diagonal size expressed in 8.8
		uint16_t usSensorDiagSize;              // sensor diagonal size in millimeter express in 8.8
	} stZoomFocal;                              // If (min = max) then no optical zoom
	struct {
		uint16_t usMin;                         // F number expressed in 8.8
		uint16_t usMax;                         // F number expressed in 8.8
	} stAperture;
} ts_DxOIspSensorCapabilities;

typedef struct {
	ts_DxOIspCapabilities         stIspCapabilities;
	ts_DxOIspSensorCapabilities   stSensorCapabilities;
} ts_DxOIspAsyncStatus;

//
// Command zone:
//
typedef struct {
	ts_DxOIspSyncCmd         stSync;
	ts_DxOIspAsyncCmd        stAsync;
} ts_DxOIspCmd;
 
//
// Status zone:
//
typedef struct {
	ts_DxOIspSyncStatus      stSync;
	ts_DxOIspAsyncStatus     stAsync;
} ts_DxOIspStatus;

//=============================================================================
//
// Macros:
//
//=============================================================================

#define DxOISP_CMD_OFFSET(x)         offsetof(ts_DxOIspCmd   , x)
#define DxOISP_STATUS_OFFSET(x)      offsetof(ts_DxOIspStatus, x) 

//=============================================================================
//
// Function prototypes:
//
//=============================================================================

void     DxOISP_Init                (uint32_t  uiSystemClock);
void     DxOISP_Event               (void);
void     DxOISP_PostEvent           (void);

void     DxOISP_CommandGroupOpen    (void);
void     DxOISP_CommandSet          (uint16_t usOffset, uint16_t usSize, void* pBuf);
uint32_t DxOISP_CommandGroupClose   (void);

void     DxOISP_StatusGroupOpen     (void);
void     DxOISP_StatusGet           (uint16_t usOffset, uint16_t usSize, void* pBuf);
void     DxOISP_StatusGroupClose    (void);

void     DxOISP_DumpStatus          (void);

typedef struct {
	uint32_t       uiHwRevisionId;
	const char*    pcFirmware;
	const char*    pcCalib;
} ts_DxOISPReleaseInfo;

void     DxOISP_GetVersion(uint8_t ucCalibrationSelection, ts_DxOISPReleaseInfo * pstReleaseInfo);

//Helper
void DxOISP_ComputeFrameBufferSize(
	const ts_DxOIspSyncCmd*   pstIspSyncCmd 
,	      uint32_t*           uiBufferSize     //return the frame size requested in Byte
);

//=============================================================================
//
// Callback: (to be implemented by customer)
//
//=============================================================================
void DxOISP_PixelStuckHandler(void);

int DxOISP_Printf(const char *format, ...);


#endif //__DXO_ISP_H__
