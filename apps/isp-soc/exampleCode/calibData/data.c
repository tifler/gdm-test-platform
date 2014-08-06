// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#ifndef INC
	#define INC(f) <f>
#endif
#include <stdint.h>
#include <string.h>

#ifndef assertf
	#ifdef NDEBUG
		#define assertf(c, sprintf_args)
	#else
		#include <stdlib.h>
		extern int DXO_PRINTF(const char *format, ...);
		#define assertf(c, sprintf_args) if (!(c)) { \
			DXO_PRINTF("assertion failed at %s:%d: ", __FILE__, __LINE__); \
			DXO_PRINTF sprintf_args; \
			DXO_PRINTF("\n"); \
			exit(-1); \
		};
	#endif // NDEBUG
	#define assertf0(c, f)                     assertf(c, (f))
	#define assertf1(c, f, a0)                 assertf(c, (f, a0))
	#define assertf2(c, f, a0, a1)             assertf(c, (f, a0, a1))
	#define assertf3(c, f, a0, a1, a2)         assertf(c, (f, a0, a1, a2))
	#define assertf4(c, f, a0, a1, a2, a3)     assertf(c, (f, a0, a1, a2, a3))
	#define assertf5(c, f, a0, a1, a2, a3, a4) assertf(c, (f, a0, a1, a2, a3, a4))
#endif // assertf

#include INC(DATA_SYMS_H_DIR/isp/syms.h)



///////////////////////////////////////////////////////////////////////////////
//////   Version information   ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define REV rev_
#define DATE __
#define SENSOR _LSI_3H5

#define STRING(a) #a
#define CONCAT(a,b,c) STRING(a ## b ## c)
#define CONCATSTRING(_a,_b,_c) CONCAT(_a, _b,_c)

static const char versionInformation[] = CONCATSTRING(REV, SENSOR, DATE);

const char* DataNS(getVersion)(void) {
	return versionInformation;
}

///////////////////////////////////////////////////////////////////////////////
//////   Global Code (procs)   ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include INC(DATA_ROOT_DIR/glb.h)

#define DataPointer(base, offset, type) ((type*) (((uint8_t*) base) + offset))

static int32_t DataInterpolate(
	const int32_t i_iValLow
,	const int32_t i_iValHi
,	const int32_t i_iPreScale
) {
	return (i_iValLow + ((i_iPreScale * (i_iValHi - i_iValLow) + (1 << 7)) >> 8));
};

///////////////////////////////////////////////////////////////////////////////
//////   Default Tuning Sliders   /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void DataNS(SetDefaultTuning)(
	DataNS(ts_Tuning) * const o_pstTuning
) {
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_HORIZON ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLUO    ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLASH   ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] =   0 ;
	o_pstTuning->stColor.cSaturation                    [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] =   0 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_HORIZON   ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_TUNGSTEN  ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_TL84      ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_COOLWHITE ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_D45       ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_DAYLIGHT  ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_CLOUDY    ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_SHADE     ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasBlue                   [DxOISP_FINE_ILLUMINANT_FLASH     ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_HORIZON   ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_TUNGSTEN  ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_TL84      ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_COOLWHITE ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_D45       ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_DAYLIGHT  ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_CLOUDY    ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_SHADE     ]                                    = 128 ;
	o_pstTuning->stColor.ucWbBiasRed                    [DxOISP_FINE_ILLUMINANT_FLASH     ]                                    = 128 ;
	o_pstTuning->stExposure.cBrightness                                                                                        =   0 ;
	o_pstTuning->stExposure.cContrast                                                                                          =   0 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucBlackLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucGamma                     [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 128 ;
	o_pstTuning->stExposure.ucWhiteLevel                [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 128 ;
	o_pstTuning->stLensCorrection.ucColorFringing       [DxOISP_COARSE_ILLUMINANT_HORIZON ]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucColorFringing       [DxOISP_COARSE_ILLUMINANT_TUNGSTEN]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucColorFringing       [DxOISP_COARSE_ILLUMINANT_FLUO    ]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucColorFringing       [DxOISP_COARSE_ILLUMINANT_OUTDOOR ]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucColorFringing       [DxOISP_COARSE_ILLUMINANT_FLASH   ]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_HIGHLIGHT     ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_STANDARD      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_HORIZON ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_TUNGSTEN] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLUO    ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_OUTDOOR ] = 179 ;
	o_pstTuning->stLensCorrection.ucLensShading         [DxOISP_COARSE_GAIN_LOWLIGHT      ][DxOISP_COARSE_ILLUMINANT_FLASH   ] = 179 ;
	o_pstTuning->stLensCorrection.ucSharpness           [DxOISP_COARSE_ILLUMINANT_HORIZON ]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucSharpness           [DxOISP_COARSE_ILLUMINANT_TUNGSTEN]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucSharpness           [DxOISP_COARSE_ILLUMINANT_FLUO    ]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucSharpness           [DxOISP_COARSE_ILLUMINANT_OUTDOOR ]                                    = 128 ;
	o_pstTuning->stLensCorrection.ucSharpness           [DxOISP_COARSE_ILLUMINANT_FLASH   ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucChrominanceNoise   [DxOISP_FINE_GAIN_ISORANGE1       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucChrominanceNoise   [DxOISP_FINE_GAIN_ISORANGE2       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucChrominanceNoise   [DxOISP_FINE_GAIN_ISORANGE3       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucChrominanceNoise   [DxOISP_FINE_GAIN_ISORANGE4       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucChrominanceNoise   [DxOISP_FINE_GAIN_ISORANGE5       ]                                    = 192 ;
	o_pstTuning->stNoiseAndDetails.ucGrain              [DxOISP_FINE_GAIN_ISORANGE1       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucGrain              [DxOISP_FINE_GAIN_ISORANGE2       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucGrain              [DxOISP_FINE_GAIN_ISORANGE3       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucGrain              [DxOISP_FINE_GAIN_ISORANGE4       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucGrain              [DxOISP_FINE_GAIN_ISORANGE5       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucLuminanceNoise     [DxOISP_FINE_GAIN_ISORANGE1       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucLuminanceNoise     [DxOISP_FINE_GAIN_ISORANGE2       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucLuminanceNoise     [DxOISP_FINE_GAIN_ISORANGE3       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucLuminanceNoise     [DxOISP_FINE_GAIN_ISORANGE4       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucLuminanceNoise     [DxOISP_FINE_GAIN_ISORANGE5       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucSegmentation       [DxOISP_FINE_GAIN_ISORANGE1       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucSegmentation       [DxOISP_FINE_GAIN_ISORANGE2       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucSegmentation       [DxOISP_FINE_GAIN_ISORANGE3       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucSegmentation       [DxOISP_FINE_GAIN_ISORANGE4       ]                                    = 128 ;
	o_pstTuning->stNoiseAndDetails.ucSegmentation       [DxOISP_FINE_GAIN_ISORANGE5       ]                                    = 128 ;
	o_pstTuning->stSensorDefectCorrection.ucBrightPixels[DxOISP_EXPOSURE_LOW              ]                                    = 128 ;
	o_pstTuning->stSensorDefectCorrection.ucBrightPixels[DxOISP_EXPOSURE_MEDIUM           ]                                    = 128 ;
	o_pstTuning->stSensorDefectCorrection.ucBrightPixels[DxOISP_EXPOSURE_HIGH             ]                                    = 128 ;
	o_pstTuning->stSensorDefectCorrection.ucDarkPixels  [DxOISP_EXPOSURE_LOW              ]                                    = 128 ;
	o_pstTuning->stSensorDefectCorrection.ucDarkPixels  [DxOISP_EXPOSURE_MEDIUM           ]                                    = 128 ;
	o_pstTuning->stSensorDefectCorrection.ucDarkPixels  [DxOISP_EXPOSURE_HIGH             ]                                    = 128 ;
};

///////////////////////////////////////////////////////////////////////////////
//////   Correction Data Selection   //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void DataNS(SetData)(
	DataNS(ts_Tuning) const * const i_pstTuning
,	void * const io_pstParams
,	const uint32_t i_uiParamsSize
,	void * const o_pstData
,	const uint32_t i_uiDataSize
) {
	assertf0(i_uiParamsSize == siz6984AF240E1BAB18, "invalid params size");
	assertf0(i_uiDataSize == siz21B109E7453A1912, "invalid data size");
	if (*DataPointer(io_pstParams, off5A14AE564D9ACCD7, uint32_t) < 3932160) {
		*DataPointer(io_pstParams, off2012BC949B9717AA, uint8_t) = DxOISP_EXPOSURE_LOW;
	} else if (
		(*DataPointer(io_pstParams, off5A14AE564D9ACCD7, uint32_t) >= 3932160)
	&&	(*DataPointer(io_pstParams, off5A14AE564D9ACCD7, uint32_t) < 7864320)
	) {
		*DataPointer(io_pstParams, off2012BC949B9717AA, uint8_t) = DxOISP_EXPOSURE_MEDIUM;
	} else if (*DataPointer(io_pstParams, off5A14AE564D9ACCD7, uint32_t) >= 7864320) {
		*DataPointer(io_pstParams, off2012BC949B9717AA, uint8_t) = DxOISP_EXPOSURE_HIGH;
	} else {
		memset(DataPointer(io_pstParams, off2012BC949B9717AA, uint8_t), 0xff, sizeof(uint8_t));
	};
	if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 768) {
		*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t) = DxOISP_COARSE_GAIN_HIGHLIGHT;
	} else if (
		(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 768)
	&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1536)
	) {
		*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t) = DxOISP_COARSE_GAIN_STANDARD;
	} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 1536) {
		*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t) = DxOISP_COARSE_GAIN_LOWLIGHT;
	} else {
		memset(DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t), 0xff, sizeof(uint8_t));
	};
	if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 384) {
		*DataPointer(io_pstParams, off78F666B443964182, uint8_t) = DxOISP_FINE_GAIN_ISORANGE1;
	} else if (
		(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 384)
	&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 768)
	) {
		*DataPointer(io_pstParams, off78F666B443964182, uint8_t) = DxOISP_FINE_GAIN_ISORANGE2;
	} else if (
		(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 768)
	&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1536)
	) {
		*DataPointer(io_pstParams, off78F666B443964182, uint8_t) = DxOISP_FINE_GAIN_ISORANGE3;
	} else if (
		(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 1536)
	&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1792)
	) {
		*DataPointer(io_pstParams, off78F666B443964182, uint8_t) = DxOISP_FINE_GAIN_ISORANGE4;
	} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 1792) {
		*DataPointer(io_pstParams, off78F666B443964182, uint8_t) = DxOISP_FINE_GAIN_ISORANGE5;
	} else {
		memset(DataPointer(io_pstParams, off78F666B443964182, uint8_t), 0xff, sizeof(uint8_t));
	};
	if (*DataPointer(io_pstParams, offEC06C59C8A903120, uint16_t) == 0) {
		if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cst96A59C80C04E7EE6) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_HORIZON;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_HORIZON;
		} else if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cst3ACAE5BA1A3DEBBF) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_TUNGSTEN;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_TUNGSTEN;
		} else if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cst9F42D104E8A7CCEF) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_FLUO;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_TL84;
		} else if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cstBB1399C006CC0DA1) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_FLUO;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_COOLWHITE;
		} else if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cstFBE8D761276939D6) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_OUTDOOR;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_D45;
		} else if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cstA4AB4BC96991F018) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_OUTDOOR;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_DAYLIGHT;
		} else if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cstC37093FCFDF4DFB2) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_OUTDOOR;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_CLOUDY;
		} else if (*DataPointer(io_pstParams, off0AC6BE04B687F2FB, uint8_t) == cstF3F72EC2CE89FDCF) {
			*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_OUTDOOR;
			*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_SHADE;
		} else {
			memset(DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t), 0xff, sizeof(uint8_t));
			memset(DataPointer(io_pstParams, offF515352592D73D41, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else {
		*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t) = DxOISP_COARSE_ILLUMINANT_FLASH;
		*DataPointer(io_pstParams, offF515352592D73D41, uint8_t) = DxOISP_FINE_ILLUMINANT_FLASH;
	};

	*DataPointer(io_pstParams, off6DE0B24764418BAE, int8_t)  = i_pstTuning->stColor.cSaturation                    [*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t)][*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t)];
	*DataPointer(io_pstParams, offC429E27AF74BE0E7, uint8_t) = i_pstTuning->stColor.ucWbBiasBlue                   [*DataPointer(io_pstParams, offF515352592D73D41, uint8_t)];
	*DataPointer(io_pstParams, off9D55841E55C90F54, uint8_t) = i_pstTuning->stColor.ucWbBiasRed                    [*DataPointer(io_pstParams, offF515352592D73D41, uint8_t)];
	*DataPointer(io_pstParams, off49A048A1198E393B, int8_t)  = i_pstTuning->stExposure.cBrightness                 ;
	*DataPointer(io_pstParams, off927BEE01B8013520, int8_t)  = i_pstTuning->stExposure.cContrast                   ;
	*DataPointer(io_pstParams, off55FCC61C9F086314, uint8_t) = i_pstTuning->stExposure.ucBlackLevel                [*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t)][*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t)];
	*DataPointer(io_pstParams, off2CE2D5E2871EC624, uint8_t) = i_pstTuning->stExposure.ucGamma                     [*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t)][*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t)];
	*DataPointer(io_pstParams, offBF508F59316EAC03, uint8_t) = i_pstTuning->stExposure.ucWhiteLevel                [*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t)][*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t)];
	*DataPointer(io_pstParams, off5C6AD11961F1CF0A, uint8_t) = i_pstTuning->stLensCorrection.ucColorFringing       [*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t)];
	*DataPointer(io_pstParams, offB3540292659F9106, uint8_t) = i_pstTuning->stLensCorrection.ucLensShading         [*DataPointer(io_pstParams, offB1749822242ED7CE, uint8_t)][*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t)];
	*DataPointer(io_pstParams, offC0A8A3FA319F002C, uint8_t) = i_pstTuning->stLensCorrection.ucSharpness           [*DataPointer(io_pstParams, offC981DA2FA42CAFF9, uint8_t)];
	*DataPointer(io_pstParams, off22AEE8C6FE90BDC1, uint8_t) = i_pstTuning->stNoiseAndDetails.ucChrominanceNoise   [*DataPointer(io_pstParams, off78F666B443964182, uint8_t)];
	*DataPointer(io_pstParams, offD8E106D3AECEF788, uint8_t) = i_pstTuning->stNoiseAndDetails.ucGrain              [*DataPointer(io_pstParams, off78F666B443964182, uint8_t)];
	*DataPointer(io_pstParams, offB09A2D3D2C9C0187, uint8_t) = i_pstTuning->stNoiseAndDetails.ucLuminanceNoise     [*DataPointer(io_pstParams, off78F666B443964182, uint8_t)];
	*DataPointer(io_pstParams, off6DDAAF48981565F6, uint8_t) = i_pstTuning->stNoiseAndDetails.ucSegmentation       [*DataPointer(io_pstParams, off78F666B443964182, uint8_t)];
	*DataPointer(io_pstParams, off7448646EB7714750, uint8_t) = i_pstTuning->stSensorDefectCorrection.ucBrightPixels[*DataPointer(io_pstParams, off2012BC949B9717AA, uint8_t)];
	*DataPointer(io_pstParams, offBE543516ED1E4911, uint8_t) = i_pstTuning->stSensorDefectCorrection.ucDarkPixels  [*DataPointer(io_pstParams, off2012BC949B9717AA, uint8_t)];

	{
		static const int16_t rom4C3047C1068C3766[11] = { 508, -657, 641, 1296, 598, 1252, 316, 968, 277, 925, 0 };
		memcpy(DataPointer(o_pstData, off364B0A53A85859CA, int16_t), rom4C3047C1068C3766, sizeof(int16_t) * 11);
		*DataPointer(o_pstData, off191C7B215A403BC2, int16_t) = -512;
		*DataPointer(o_pstData, off037908438DF1F214, uint16_t) = 128;
		*DataPointer(o_pstData, offC874BF09F555D08D, uint8_t) = 64;
		*DataPointer(o_pstData, off89EFD5107D63628D, uint8_t) = 64;
	};
	{
		int16_t k0 = 0, pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			int16_t varA9FEA0E185898D1F[11];
			int16_t varA66B793A5BBD3CB7[11];
			int16_t varAA23D5DA90AE492E[11];
			int16_t varE9A20D1401BD2712[11];
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			static const int16_t romA9FEA0E185898D1F[11] = { 211, -186, 175, 359, 161, 344, 85, 267, 73, 252, 0 };
			static const int16_t romA66B793A5BBD3CB7[11] = { 690, -625, 580, 1195, 572, 1185, 279, 887, 272, 877, 0 };
			static const int16_t romAA23D5DA90AE492E[11] = { 211, -186, 175, 359, 161, 344, 85, 267, 73, 252, 0 };
			static const int16_t romE9A20D1401BD2712[11] = { 690, -625, 580, 1195, 572, 1185, 279, 887, 272, 877, 0 };
			memcpy(stInterp0[idx0].varA9FEA0E185898D1F, romA9FEA0E185898D1F, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varA66B793A5BBD3CB7, romA66B793A5BBD3CB7, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varAA23D5DA90AE492E, romAA23D5DA90AE492E, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varE9A20D1401BD2712, romE9A20D1401BD2712, sizeof(int16_t) * 11);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			static const int16_t romA9FEA0E185898D1F[11] = { 159, -140, 132, 270, 121, 258, 64, 201, 55, 189, 0 };
			static const int16_t romA66B793A5BBD3CB7[11] = { 518, -471, 438, 900, 431, 894, 211, 668, 205, 661, 0 };
			static const int16_t romAA23D5DA90AE492E[11] = { 159, -140, 132, 270, 121, 258, 64, 201, 55, 189, 0 };
			static const int16_t romE9A20D1401BD2712[11] = { 518, -471, 438, 900, 431, 894, 211, 668, 205, 661, 0 };
			memcpy(stInterp0[idx0].varA9FEA0E185898D1F, romA9FEA0E185898D1F, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varA66B793A5BBD3CB7, romA66B793A5BBD3CB7, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varAA23D5DA90AE492E, romAA23D5DA90AE492E, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varE9A20D1401BD2712, romE9A20D1401BD2712, sizeof(int16_t) * 11);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			static const int16_t romA9FEA0E185898D1F[11] = { 65, -58, 55, 112, 50, 108, 27, 83, 23, 79, 0 };
			static const int16_t romA66B793A5BBD3CB7[11] = { 246, -259, 243, 499, 245, 500, 118, 371, 119, 372, 0 };
			static const int16_t romAA23D5DA90AE492E[11] = { 65, -58, 55, 112, 50, 108, 27, 83, 23, 79, 0 };
			static const int16_t romE9A20D1401BD2712[11] = { 246, -259, 243, 499, 245, 500, 118, 371, 119, 372, 0 };
			memcpy(stInterp0[idx0].varA9FEA0E185898D1F, romA9FEA0E185898D1F, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varA66B793A5BBD3CB7, romA66B793A5BBD3CB7, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varAA23D5DA90AE492E, romAA23D5DA90AE492E, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varE9A20D1401BD2712, romE9A20D1401BD2712, sizeof(int16_t) * 11);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			static const int16_t romA9FEA0E185898D1F[11] = { 42, -37, 35, 71, 32, 69, 17, 53, 14, 50, 0 };
			static const int16_t romA66B793A5BBD3CB7[11] = { 129, -107, 98, 204, 96, 200, 47, 151, 45, 148, 0 };
			static const int16_t romAA23D5DA90AE492E[11] = { 42, -37, 35, 71, 32, 69, 17, 53, 14, 50, 0 };
			static const int16_t romE9A20D1401BD2712[11] = { 129, -107, 98, 204, 96, 200, 47, 151, 45, 148, 0 };
			memcpy(stInterp0[idx0].varA9FEA0E185898D1F, romA9FEA0E185898D1F, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varA66B793A5BBD3CB7, romA66B793A5BBD3CB7, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varAA23D5DA90AE492E, romAA23D5DA90AE492E, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varE9A20D1401BD2712, romE9A20D1401BD2712, sizeof(int16_t) * 11);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			static const int16_t romA9FEA0E185898D1F[11] = { 27, -23, 22, 44, 19, 42, 11, 33, 9, 31, 0 };
			static const int16_t romA66B793A5BBD3CB7[11] = { 54, -23, 19, 42, 17, 39, 9, 30, 7, 28, 0 };
			static const int16_t romAA23D5DA90AE492E[11] = { 27, -23, 22, 44, 19, 42, 11, 33, 9, 31, 0 };
			static const int16_t romE9A20D1401BD2712[11] = { 54, -23, 19, 42, 17, 39, 9, 30, 7, 28, 0 };
			memcpy(stInterp0[idx0].varA9FEA0E185898D1F, romA9FEA0E185898D1F, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varA66B793A5BBD3CB7, romA66B793A5BBD3CB7, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varAA23D5DA90AE492E, romAA23D5DA90AE492E, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varE9A20D1401BD2712, romE9A20D1401BD2712, sizeof(int16_t) * 11);
			pos0++;
			idx0++;
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, off0929E29666AC0BE7, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].varA9FEA0E185898D1F[k0], stInterp0[1].varA9FEA0E185898D1F[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, offBFA88677D00AB10F, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].varA66B793A5BBD3CB7[k0], stInterp0[1].varA66B793A5BBD3CB7[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, offD95EF7EC9187B17C, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].varAA23D5DA90AE492E[k0], stInterp0[1].varAA23D5DA90AE492E[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, off66C35C18FDFDD2C9, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].varE9A20D1401BD2712[k0], stInterp0[1].varE9A20D1401BD2712[k0], scale0);
		};
	};
	{
		static const int16_t romC3BC5A4091B13D9E[6] = { 870, -83, -63, -105, 50, 779 };
		static const int16_t rom7FE3CD0FDD722B26[4] = { 153, 58, -86, -42 };
		static const int8_t rom62DD8031D41DE880[4] = { 0, 127, 127, 0 };
		static const int8_t rom224CD0F6114C2069[4] = { 80, -10, 40, 30 };
		static const int8_t rom0E89709503C46F87[4] = { 80, 20, 0, 90 };
		static const int16_t romE0C03C2D0B0D3037[11] = {
			   0,  482, -394, -853, -227, -680
		,	-176, -622,  -26, -450, 2047
		};
		memcpy(DataPointer(o_pstData, off52576E07DEA0F03A, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 6);
		memcpy(DataPointer(o_pstData, offD4FA5788C0AF585C, int16_t), rom7FE3CD0FDD722B26, sizeof(int16_t) * 4);
		memcpy(DataPointer(o_pstData, off8AEDF5AF665BE36A, int8_t), rom62DD8031D41DE880, sizeof(int8_t) * 4);
		memcpy(DataPointer(o_pstData, offDCEA3C8881702352, int8_t), rom224CD0F6114C2069, sizeof(int8_t) * 4);
		memcpy(DataPointer(o_pstData, offAB57123FCEE818F6, int8_t), rom0E89709503C46F87, sizeof(int8_t) * 4);
		memcpy(DataPointer(o_pstData, off28635CD5E80CBC97, int16_t), romE0C03C2D0B0D3037, sizeof(int16_t) * 11);
		*DataPointer(o_pstData, off6DB031D67C563BB1, uint8_t) = 17;
	};
	{
		int16_t pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			uint16_t var001B59A35339EAE7;
			uint16_t var0BE9B5C299308605;
			uint16_t var220CEAAC487EF565;
			uint16_t var7355E788C4E0F158;
			uint16_t varAD40A7F538F99963;
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			stInterp0[idx0].var001B59A35339EAE7 = 128;
			stInterp0[idx0].var0BE9B5C299308605 = 512;
			stInterp0[idx0].var220CEAAC487EF565 = 256;
			stInterp0[idx0].var7355E788C4E0F158 = 256;
			stInterp0[idx0].varAD40A7F538F99963 = 64;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			stInterp0[idx0].var001B59A35339EAE7 = 256;
			stInterp0[idx0].var0BE9B5C299308605 = 410;
			stInterp0[idx0].var220CEAAC487EF565 = 384;
			stInterp0[idx0].var7355E788C4E0F158 = 384;
			stInterp0[idx0].varAD40A7F538F99963 = 256;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			stInterp0[idx0].var001B59A35339EAE7 = 307;
			stInterp0[idx0].var0BE9B5C299308605 = 384;
			stInterp0[idx0].var220CEAAC487EF565 = 333;
			stInterp0[idx0].var7355E788C4E0F158 = 333;
			stInterp0[idx0].varAD40A7F538F99963 = 307;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			stInterp0[idx0].var001B59A35339EAE7 = 205;
			stInterp0[idx0].var0BE9B5C299308605 = 307;
			stInterp0[idx0].var220CEAAC487EF565 = 230;
			stInterp0[idx0].var7355E788C4E0F158 = 230;
			stInterp0[idx0].varAD40A7F538F99963 = 205;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			stInterp0[idx0].var001B59A35339EAE7 = 128;
			stInterp0[idx0].var0BE9B5C299308605 = 256;
			stInterp0[idx0].var220CEAAC487EF565 = 205;
			stInterp0[idx0].var7355E788C4E0F158 = 205;
			stInterp0[idx0].varAD40A7F538F99963 = 128;
			pos0++;
			idx0++;
		};
		*DataPointer(o_pstData, off61088D58C4939161, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var001B59A35339EAE7, stInterp0[1].var001B59A35339EAE7, scale0);
		*DataPointer(o_pstData, off07E0C527E7C3CC52, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var0BE9B5C299308605, stInterp0[1].var0BE9B5C299308605, scale0);
		*DataPointer(o_pstData, offCC40BECE1DC6BE74, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var220CEAAC487EF565, stInterp0[1].var220CEAAC487EF565, scale0);
		*DataPointer(o_pstData, off3C149DA53C36F9F9, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var7355E788C4E0F158, stInterp0[1].var7355E788C4E0F158, scale0);
		*DataPointer(o_pstData, off2EF7A782029187F0, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].varAD40A7F538F99963, stInterp0[1].varAD40A7F538F99963, scale0);
	};
	*DataPointer(o_pstData, off25969CE92612E23B, uint16_t) = 957;
	*DataPointer(o_pstData, off4E1C1ECCC52281CE, uint16_t) = 511;
	*DataPointer(o_pstData, off7A245B1B3F059734, uint8_t) = 4;
	*DataPointer(o_pstData, off227A6CF59975AD06, uint16_t) = 896;
	{
		int16_t k0 = 0, pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			int16_t varC6654282119044E3[11];
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) < 2) {
				static const int16_t romC6654282119044E3[11] = { 20, 129, -99, -210, -93, -204, -46, -154, -42, -147, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
				static const int16_t romC6654282119044E3[11] = { 10, 65, -50, -106, -47, -103, -23, -78, -22, -74, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else {
				memset(stInterp0[idx0].varC6654282119044E3, 0xff, sizeof(int16_t) * 11);
			};
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) < 2) {
				static const int16_t romC6654282119044E3[11] = { 27, 175, -131, -280, -125, -271, -61, -205, -57, -197, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
				static const int16_t romC6654282119044E3[11] = { 13, 88, -66, -141, -63, -136, -31, -103, -28, -99, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else {
				memset(stInterp0[idx0].varC6654282119044E3, 0xff, sizeof(int16_t) * 11);
			};
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) < 2) {
				static const int16_t romC6654282119044E3[11] = { 53, 208, -152, -327, -140, -312, -70, -238, -62, -224, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
				static const int16_t romC6654282119044E3[11] = { 27, 104, -75, -164, -71, -156, -35, -119, -31, -113, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else {
				memset(stInterp0[idx0].varC6654282119044E3, 0xff, sizeof(int16_t) * 11);
			};
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) < 2) {
				static const int16_t romC6654282119044E3[11] = { 106, 211, -144, -315, -128, -295, -66, -228, -54, -209, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
				static const int16_t romC6654282119044E3[11] = { 53, 105, -71, -157, -64, -146, -32, -113, -27, -104, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else {
				memset(stInterp0[idx0].varC6654282119044E3, 0xff, sizeof(int16_t) * 11);
			};
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) < 2) {
				static const int16_t romC6654282119044E3[11] = { 124, 239, -162, -356, -143, -332, -74, -257, -60, -235, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
				static const int16_t romC6654282119044E3[11] = { 63, 119, -81, -177, -71, -166, -37, -128, -30, -117, 2047 };
				memcpy(stInterp0[idx0].varC6654282119044E3, romC6654282119044E3, sizeof(int16_t) * 11);
			} else {
				memset(stInterp0[idx0].varC6654282119044E3, 0xff, sizeof(int16_t) * 11);
			};
			pos0++;
			idx0++;
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, off09F58568C0961674, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].varC6654282119044E3[k0], stInterp0[1].varC6654282119044E3[k0], scale0);
		};
	};
	{
		static const uint8_t romFB1DAFB61FD361AF[4] = { 32, 32, 32, 32 };
		memcpy(DataPointer(o_pstData, off2CFF0C826A806A4F, uint8_t), romFB1DAFB61FD361AF, sizeof(uint8_t) * 4);
		*DataPointer(o_pstData, off5D69FE15AF4A6A31, uint8_t) = 8;
		*DataPointer(o_pstData, off3789CE9D395B9B70, uint8_t) = 56;
	};
	{
		static const int16_t rom0A7740E2FD1982BF[11] = { 233, -392, 375, 766, 404, 794, 182, 570, 207, 599, 2047 };
		*DataPointer(o_pstData, offBCFBF82B751D46A5, uint16_t) = 204;
		*DataPointer(o_pstData, off69EF9C696D9721F6, uint16_t) = 204;
		*DataPointer(o_pstData, offCDFE6268DF68A92F, uint8_t) = 0;
		memcpy(DataPointer(o_pstData, off1193FE580F35AD13, int16_t), rom0A7740E2FD1982BF, sizeof(int16_t) * 11);
	};
	{
		uint8_t varF49C798236EE28A0;
		uint8_t var33C37CA355945C49;
		uint8_t var520D51AB0EAF1057;
		varF49C798236EE28A0 = 1;
		var33C37CA355945C49 = 0;
		var520D51AB0EAF1057 = 1;
		ProcNS(insert_AA_MultiPhase)(
			*DataPointer(io_pstParams, off25C2C9BDE3113654, uint8_t)
		,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
		,	varF49C798236EE28A0
		,	var33C37CA355945C49
		,	var520D51AB0EAF1057
		,	DataPointer(o_pstData, offDE5B5255C2CDDD01, uint16_t)
		,	DataPointer(o_pstData, off09D6CF73F628C7EE, uint8_t)
		,	DataPointer(o_pstData, off4A355A5BF3430FF2, uint8_t)
		);
	};
	*DataPointer(o_pstData, offA5CF297EBEDEA6D9, uint16_t) = 432;
	*DataPointer(o_pstData, offB97D5BFCBE8D2C29, uint16_t) = 300;
	*DataPointer(o_pstData, off9BC2F85786ABF49C, uint16_t) = 640;
	*DataPointer(o_pstData, offED20A34B46FD8E50, uint16_t) = 768;
	*DataPointer(o_pstData, off343A60E572860B16, uint16_t) = 4;
	*DataPointer(o_pstData, off95DB90E4D1D7AFA2, uint16_t) = 10;
	*DataPointer(o_pstData, offB63EBB94BD9812F4, uint16_t) = 100;
	*DataPointer(o_pstData, off4D061C9617FF16DC, uint16_t) = 166;
	{
		uint16_t var58236B0960720119;
		uint16_t varA7CBDB2215E68B80;
		uint16_t var30593EEAA702758C;
		uint16_t varA5A1FB94F606049A;
		if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) == 1) {
			int16_t pos0 = 0, idx0 = 0;
			int32_t scale0 = 0;
			struct {
				uint16_t var58236B0960720119;
				uint16_t varA7CBDB2215E68B80;
				uint16_t var30593EEAA702758C;
				uint16_t varA5A1FB94F606049A;
			} stInterp0[2];
			if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
				pos0 = 0;
				scale0 = 0;
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
				pos0 = 0;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
				pos0 = 1;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
				pos0 = 2;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
				pos0 = 3;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
			} else {
				pos0 = 3;
				scale0 = 256;
			};
			if (idx0 < 2 && pos0 == 0) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 1) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 2) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 3) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 4) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			var58236B0960720119 = (uint16_t) DataInterpolate(stInterp0[0].var58236B0960720119, stInterp0[1].var58236B0960720119, scale0);
			varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp0[0].varA7CBDB2215E68B80, stInterp0[1].varA7CBDB2215E68B80, scale0);
			var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp0[0].var30593EEAA702758C, stInterp0[1].var30593EEAA702758C, scale0);
			varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp0[0].varA5A1FB94F606049A, stInterp0[1].varA5A1FB94F606049A, scale0);
		} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
			int16_t pos0 = 0, idx0 = 0;
			int32_t scale0 = 0;
			struct {
				uint16_t var58236B0960720119;
				uint16_t varA7CBDB2215E68B80;
				uint16_t var30593EEAA702758C;
				uint16_t varA5A1FB94F606049A;
			} stInterp0[2];
			if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
				pos0 = 0;
				scale0 = 0;
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
				pos0 = 0;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
				pos0 = 1;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
				pos0 = 2;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
				pos0 = 3;
				scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
			} else {
				pos0 = 3;
				scale0 = 256;
			};
			if (idx0 < 2 && pos0 == 0) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 1) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 63;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 2) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 63;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 3) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 63;
					stInterp1[idx1].var30593EEAA702758C = 63;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 4) {
				int16_t pos1 = 0, idx1 = 0;
				int32_t scale1 = 0;
				struct {
					uint16_t var58236B0960720119;
					uint16_t varA7CBDB2215E68B80;
					uint16_t var30593EEAA702758C;
					uint16_t varA5A1FB94F606049A;
				} stInterp1[2];
				if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 215) {
					pos1 = 0;
					scale1 = 0;
				} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
					pos1 = 0;
					scale1 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 215) << 8) / (258 - 215);
				} else {
					pos1 = 0;
					scale1 = 256;
				};
				if (idx1 < 2 && pos1 == 0) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				if (idx1 < 2 && pos1 == 1) {
					stInterp1[idx1].var58236B0960720119 = 64;
					stInterp1[idx1].varA7CBDB2215E68B80 = 64;
					stInterp1[idx1].var30593EEAA702758C = 64;
					stInterp1[idx1].varA5A1FB94F606049A = 64;
					pos1++;
					idx1++;
				};
				stInterp0[idx0].var58236B0960720119 = (uint16_t) DataInterpolate(stInterp1[0].var58236B0960720119, stInterp1[1].var58236B0960720119, scale1);
				stInterp0[idx0].varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp1[0].varA7CBDB2215E68B80, stInterp1[1].varA7CBDB2215E68B80, scale1);
				stInterp0[idx0].var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp1[0].var30593EEAA702758C, stInterp1[1].var30593EEAA702758C, scale1);
				stInterp0[idx0].varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp1[0].varA5A1FB94F606049A, stInterp1[1].varA5A1FB94F606049A, scale1);
				pos0++;
				idx0++;
			};
			var58236B0960720119 = (uint16_t) DataInterpolate(stInterp0[0].var58236B0960720119, stInterp0[1].var58236B0960720119, scale0);
			varA7CBDB2215E68B80 = (uint16_t) DataInterpolate(stInterp0[0].varA7CBDB2215E68B80, stInterp0[1].varA7CBDB2215E68B80, scale0);
			var30593EEAA702758C = (uint16_t) DataInterpolate(stInterp0[0].var30593EEAA702758C, stInterp0[1].var30593EEAA702758C, scale0);
			varA5A1FB94F606049A = (uint16_t) DataInterpolate(stInterp0[0].varA5A1FB94F606049A, stInterp0[1].varA5A1FB94F606049A, scale0);
		} else {
			memset(&(var58236B0960720119), 0xff, sizeof(uint16_t));
			memset(&(varA7CBDB2215E68B80), 0xff, sizeof(uint16_t));
			memset(&(var30593EEAA702758C), 0xff, sizeof(uint16_t));
			memset(&(varA5A1FB94F606049A), 0xff, sizeof(uint16_t));
		};
		ProcNS(fmt_BP_4C)(
			var58236B0960720119
		,	varA7CBDB2215E68B80
		,	var30593EEAA702758C
		,	varA5A1FB94F606049A
		,	DataPointer(o_pstData, off61DBD0FCC7878509, int16_t)
		,	DataPointer(o_pstData, offB893FE0A1F33D531, int16_t)
		,	DataPointer(o_pstData, off391256F68C8523A3, int16_t)
		,	DataPointer(o_pstData, offA012AF71CAC79FD8, int16_t)
		);
	};
	ProcNS(pedestal_BP_4C)(
		*DataPointer(io_pstParams, off1199BA25F4253836, uint16_t)
	,	DataPointer(o_pstData, off61DBD0FCC7878509, int16_t)
	,	DataPointer(o_pstData, offB893FE0A1F33D531, int16_t)
	,	DataPointer(o_pstData, off391256F68C8523A3, int16_t)
	,	DataPointer(o_pstData, offA012AF71CAC79FD8, int16_t)
	);
	{
		uint16_t var8C0A0E5FB536582C;
		*DataPointer(o_pstData, off11D7DAB5B2D4AD49, uint16_t) = 300;
		var8C0A0E5FB536582C = 40;
		*DataPointer(o_pstData, offCB897050647C7EB6, uint16_t) = 523;
		*DataPointer(o_pstData, off228F6B9C478099FA, uint16_t) = 523;
		*DataPointer(o_pstData, offFA0063C02DBB9E08, uint16_t) = 358;
		*DataPointer(o_pstData, off875E764F0544C327, uint16_t) = 600;
		ProcNS(fmt_CF)(
			var8C0A0E5FB536582C
		,	DataPointer(o_pstData, off11D7DAB5B2D4AD49, uint16_t)
		,	DataPointer(o_pstData, offCB897050647C7EB6, uint16_t)
		,	DataPointer(o_pstData, off228F6B9C478099FA, uint16_t)
		,	DataPointer(o_pstData, offFA0063C02DBB9E08, uint16_t)
		,	DataPointer(o_pstData, off875E764F0544C327, uint16_t)
		,	DataPointer(o_pstData, offE975C72C1A0DAAA1, uint16_t)
		,	DataPointer(o_pstData, offECC0A7DFA0ACEF63, uint16_t)
		);
	};
	{
		int16_t var80D16623254D39DB[9];
		int16_t k0 = 0, pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			int16_t var80D16623254D39DB[9];
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 258) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 362) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 258) << 8) / (362 - 258);
		} else if (*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) < 433) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t) - 362) << 8) / (433 - 362);
		} else {
			pos0 = 1;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			int16_t k1 = 0, pos1 = 0, idx1 = 0;
			int32_t scale1 = 0;
			struct {
				int16_t var80D16623254D39DB[9];
			} stInterp1[2];
			if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
				pos1 = 0;
				scale1 = 0;
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1152) {
				pos1 = 0;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1152 - 512);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1664) {
				pos1 = 1;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1152) << 8) / (1664 - 1152);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
				pos1 = 2;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1664) << 8) / (4096 - 1664);
			} else {
				pos1 = 2;
				scale1 = 256;
			};
			if (idx1 < 2 && pos1 == 0) {
				static const int16_t rom80D16623254D39DB[9] = {
					 873, -325, -36
				,	-109,  595,  26
				,	  28, -360, 844
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 1) {
				static const int16_t rom80D16623254D39DB[9] = {
					801, -263, -25
				,	-81,  568,  25
				,	 29, -317, 801
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 2) {
				static const int16_t rom80D16623254D39DB[9] = {
					692, -171,  -9
				,	-39,  530,  22
				,	 18, -275, 769
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 3) {
				static const int16_t rom80D16623254D39DB[9] = {
					620, -108,   0
				,	-13,  500,  25
				,	 30, -209, 690
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			for (k1 = 0; k1 < 9; k1++) {
				stInterp0[idx0].var80D16623254D39DB[k1] = (int16_t) DataInterpolate(stInterp1[0].var80D16623254D39DB[k1], stInterp1[1].var80D16623254D39DB[k1], scale1);
			};
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			int16_t k1 = 0, pos1 = 0, idx1 = 0;
			int32_t scale1 = 0;
			struct {
				int16_t var80D16623254D39DB[9];
			} stInterp1[2];
			if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
				pos1 = 0;
				scale1 = 0;
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1152) {
				pos1 = 0;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1152 - 512);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1664) {
				pos1 = 1;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1152) << 8) / (1664 - 1152);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
				pos1 = 2;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1664) << 8) / (4096 - 1664);
			} else {
				pos1 = 2;
				scale1 = 256;
			};
			if (idx1 < 2 && pos1 == 0) {
				static const int16_t rom80D16623254D39DB[9] = {
					899, -373, -14
				,	-95,  591,  15
				,	 26, -384, 871
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 1) {
				static const int16_t rom80D16623254D39DB[9] = {
					824, -307,  -5
				,	-68,  565,  15
				,	 26, -340, 825
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 2) {
				static const int16_t rom80D16623254D39DB[9] = {
					710, -209,  11
				,	-28,  528,  13
				,	 15, -296, 793
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 3) {
				static const int16_t rom80D16623254D39DB[9] = {
					636, -142,  18
				,	 -3,  499,  17
				,	 28, -227, 711
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			for (k1 = 0; k1 < 9; k1++) {
				stInterp0[idx0].var80D16623254D39DB[k1] = (int16_t) DataInterpolate(stInterp1[0].var80D16623254D39DB[k1], stInterp1[1].var80D16623254D39DB[k1], scale1);
			};
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			int16_t k1 = 0, pos1 = 0, idx1 = 0;
			int32_t scale1 = 0;
			struct {
				int16_t var80D16623254D39DB[9];
			} stInterp1[2];
			if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
				pos1 = 0;
				scale1 = 0;
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1152) {
				pos1 = 0;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1152 - 512);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1664) {
				pos1 = 1;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1152) << 8) / (1664 - 1152);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
				pos1 = 2;
				scale1 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1664) << 8) / (4096 - 1664);
			} else {
				pos1 = 2;
				scale1 = 256;
			};
			if (idx1 < 2 && pos1 == 0) {
				static const int16_t rom80D16623254D39DB[9] = {
					 944, -434,   2
				,	-103,  648, -33
				,	  18, -356, 849
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 1) {
				static const int16_t rom80D16623254D39DB[9] = {
					864, -362,  10
				,	-75,  617, -30
				,	 20, -313, 805
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 2) {
				static const int16_t rom80D16623254D39DB[9] = {
					742, -257,  27
				,	-32,  575, -31
				,	 10, -272, 774
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 3) {
				static const int16_t rom80D16623254D39DB[9] = {
					663, -184,  32
				,	 -6,  540, -22
				,	 24, -206, 694
				};
				memcpy(stInterp1[idx1].var80D16623254D39DB, rom80D16623254D39DB, sizeof(int16_t) * 9);
				pos1++;
				idx1++;
			};
			for (k1 = 0; k1 < 9; k1++) {
				stInterp0[idx0].var80D16623254D39DB[k1] = (int16_t) DataInterpolate(stInterp1[0].var80D16623254D39DB[k1], stInterp1[1].var80D16623254D39DB[k1], scale1);
			};
			pos0++;
			idx0++;
		};
		for (k0 = 0; k0 < 9; k0++) {
			var80D16623254D39DB[k0] = (int16_t) DataInterpolate(stInterp0[0].var80D16623254D39DB[k0], stInterp0[1].var80D16623254D39DB[k0], scale0);
		};
		ProcNS(fmt_M)(
			*DataPointer(io_pstParams, offC429E27AF74BE0E7, uint8_t)
		,	*DataPointer(io_pstParams, off9D55841E55C90F54, uint8_t)
		,	var80D16623254D39DB
		,	DataPointer(o_pstData, offDC73D15CD1527EA3, int16_t)
		);
	};
	{
		int16_t var6B5CC2E17F5C32BB[11];
		int16_t varACAED283F41E87B9[11];
		int16_t varAE96BD3F751512E1[11];
		int16_t var84C7885DF5877462[11];
		int16_t varAA8C6B01FD937132[11];
		int16_t varE5A300604CF5FEF5[11];
		int16_t k0 = 0, pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			int16_t var6B5CC2E17F5C32BB[11];
			int16_t varACAED283F41E87B9[11];
			int16_t varAE96BD3F751512E1[11];
			int16_t var84C7885DF5877462[11];
			int16_t varAA8C6B01FD937132[11];
			int16_t varE5A300604CF5FEF5[11];
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off461389A4D919AD0A, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off461389A4D919AD0A, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off461389A4D919AD0A, uint16_t) - 256) << 8) / (512 - 256);
		} else {
			pos0 = 0;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			int16_t k1 = 0, pos1 = 0, idx1 = 0;
			int32_t scale1 = 0;
			struct {
				int16_t var6B5CC2E17F5C32BB[11];
				int16_t varACAED283F41E87B9[11];
				int16_t varAE96BD3F751512E1[11];
				int16_t var84C7885DF5877462[11];
				int16_t varAA8C6B01FD937132[11];
				int16_t varE5A300604CF5FEF5[11];
			} stInterp1[2];
			if (*DataPointer(io_pstParams, off0D6A04FD29F5B816, uint32_t) < 4587520) {
				pos1 = 0;
				scale1 = 0;
			} else if (*DataPointer(io_pstParams, off0D6A04FD29F5B816, uint32_t) < 42205184) {
				pos1 = 0;
				scale1 = (int32_t)(((uint64_t)(*DataPointer(io_pstParams, off0D6A04FD29F5B816, uint32_t) - 4587520) << 8) / (42205184 - 4587520));
			} else {
				pos1 = 0;
				scale1 = 256;
			};
			if (idx1 < 2 && pos1 == 0) {
				static const int16_t rom6B5CC2E17F5C32BB[11] = { 0, 674, -547, -1204, -500, -1149, -242, -872, -210, -819, 2047 };
				static const int16_t romACAED283F41E87B9[11] = { 0, 674, -547, -1204, -500, -1149, -242, -872, -210, -819, 2047 };
				static const int16_t romAE96BD3F751512E1[11] = { 0, 674, -547, -1204, -500, -1149, -242, -872, -210, -819, 2047 };
				static const int16_t rom84C7885DF5877462[11] = { 0, 674, -547, -1204, -500, -1149, -242, -872, -210, -819, 2047 };
				static const int16_t romAA8C6B01FD937132[11] = { 0, 674, -547, -1204, -500, -1149, -242, -872, -210, -819, 2047 };
				static const int16_t romE5A300604CF5FEF5[11] = { 0, 674, -547, -1204, -500, -1149, -242, -872, -210, -819, 2047 };
				memcpy(stInterp1[idx1].var6B5CC2E17F5C32BB, rom6B5CC2E17F5C32BB, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varACAED283F41E87B9, romACAED283F41E87B9, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varAE96BD3F751512E1, romAE96BD3F751512E1, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].var84C7885DF5877462, rom84C7885DF5877462, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varAA8C6B01FD937132, romAA8C6B01FD937132, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varE5A300604CF5FEF5, romE5A300604CF5FEF5, sizeof(int16_t) * 11);
				pos1++;
				idx1++;
			};
			if (idx1 < 2 && pos1 == 1) {
				static const int16_t rom6B5CC2E17F5C32BB[11] = { 0, 523, -393, -889, -329, -818, -167, -638, -123, -567, 2047 };
				static const int16_t romACAED283F41E87B9[11] = { 0, 523, -393, -889, -329, -818, -167, -638, -123, -567, 2047 };
				static const int16_t romAE96BD3F751512E1[11] = { 0, 523, -393, -889, -329, -818, -167, -638, -123, -567, 2047 };
				static const int16_t rom84C7885DF5877462[11] = { 0, 523, -393, -889, -329, -818, -167, -638, -123, -567, 2047 };
				static const int16_t romAA8C6B01FD937132[11] = { 0, 523, -393, -889, -329, -818, -167, -638, -123, -567, 2047 };
				static const int16_t romE5A300604CF5FEF5[11] = { 0, 523, -393, -889, -329, -818, -167, -638, -123, -567, 2047 };
				memcpy(stInterp1[idx1].var6B5CC2E17F5C32BB, rom6B5CC2E17F5C32BB, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varACAED283F41E87B9, romACAED283F41E87B9, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varAE96BD3F751512E1, romAE96BD3F751512E1, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].var84C7885DF5877462, rom84C7885DF5877462, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varAA8C6B01FD937132, romAA8C6B01FD937132, sizeof(int16_t) * 11);
				memcpy(stInterp1[idx1].varE5A300604CF5FEF5, romE5A300604CF5FEF5, sizeof(int16_t) * 11);
				pos1++;
				idx1++;
			};
			for (k1 = 0; k1 < 11; k1++) {
				stInterp0[idx0].var6B5CC2E17F5C32BB[k1] = (int16_t) DataInterpolate(stInterp1[0].var6B5CC2E17F5C32BB[k1], stInterp1[1].var6B5CC2E17F5C32BB[k1], scale1);
			};
			for (k1 = 0; k1 < 11; k1++) {
				stInterp0[idx0].varACAED283F41E87B9[k1] = (int16_t) DataInterpolate(stInterp1[0].varACAED283F41E87B9[k1], stInterp1[1].varACAED283F41E87B9[k1], scale1);
			};
			for (k1 = 0; k1 < 11; k1++) {
				stInterp0[idx0].varAE96BD3F751512E1[k1] = (int16_t) DataInterpolate(stInterp1[0].varAE96BD3F751512E1[k1], stInterp1[1].varAE96BD3F751512E1[k1], scale1);
			};
			for (k1 = 0; k1 < 11; k1++) {
				stInterp0[idx0].var84C7885DF5877462[k1] = (int16_t) DataInterpolate(stInterp1[0].var84C7885DF5877462[k1], stInterp1[1].var84C7885DF5877462[k1], scale1);
			};
			for (k1 = 0; k1 < 11; k1++) {
				stInterp0[idx0].varAA8C6B01FD937132[k1] = (int16_t) DataInterpolate(stInterp1[0].varAA8C6B01FD937132[k1], stInterp1[1].varAA8C6B01FD937132[k1], scale1);
			};
			for (k1 = 0; k1 < 11; k1++) {
				stInterp0[idx0].varE5A300604CF5FEF5[k1] = (int16_t) DataInterpolate(stInterp1[0].varE5A300604CF5FEF5[k1], stInterp1[1].varE5A300604CF5FEF5[k1], scale1);
			};
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			static const int16_t rom6B5CC2E17F5C32BB[11] = { 0, 482, -394, -853, -227, -680, -176, -622, -26, -450, 2047 };
			static const int16_t romACAED283F41E87B9[11] = { 0, 482, -394, -853, -227, -680, -176, -622, -26, -450, 2047 };
			static const int16_t romAE96BD3F751512E1[11] = { 0, 482, -394, -853, -227, -680, -176, -622, -26, -450, 2047 };
			static const int16_t rom84C7885DF5877462[11] = { 0, 482, -394, -853, -227, -680, -176, -622, -26, -450, 2047 };
			static const int16_t romAA8C6B01FD937132[11] = { 0, 482, -394, -853, -227, -680, -176, -622, -26, -450, 2047 };
			static const int16_t romE5A300604CF5FEF5[11] = { 0, 482, -394, -853, -227, -680, -176, -622, -26, -450, 2047 };
			memcpy(stInterp0[idx0].var6B5CC2E17F5C32BB, rom6B5CC2E17F5C32BB, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varACAED283F41E87B9, romACAED283F41E87B9, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varAE96BD3F751512E1, romAE96BD3F751512E1, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].var84C7885DF5877462, rom84C7885DF5877462, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varAA8C6B01FD937132, romAA8C6B01FD937132, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].varE5A300604CF5FEF5, romE5A300604CF5FEF5, sizeof(int16_t) * 11);
			pos0++;
			idx0++;
		};
		for (k0 = 0; k0 < 11; k0++) {
			var6B5CC2E17F5C32BB[k0] = (int16_t) DataInterpolate(stInterp0[0].var6B5CC2E17F5C32BB[k0], stInterp0[1].var6B5CC2E17F5C32BB[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			varACAED283F41E87B9[k0] = (int16_t) DataInterpolate(stInterp0[0].varACAED283F41E87B9[k0], stInterp0[1].varACAED283F41E87B9[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			varAE96BD3F751512E1[k0] = (int16_t) DataInterpolate(stInterp0[0].varAE96BD3F751512E1[k0], stInterp0[1].varAE96BD3F751512E1[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			var84C7885DF5877462[k0] = (int16_t) DataInterpolate(stInterp0[0].var84C7885DF5877462[k0], stInterp0[1].var84C7885DF5877462[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			varAA8C6B01FD937132[k0] = (int16_t) DataInterpolate(stInterp0[0].varAA8C6B01FD937132[k0], stInterp0[1].varAA8C6B01FD937132[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			varE5A300604CF5FEF5[k0] = (int16_t) DataInterpolate(stInterp0[0].varE5A300604CF5FEF5[k0], stInterp0[1].varE5A300604CF5FEF5[k0], scale0);
		};
		ProcNS(tune_TC_C)(
			*DataPointer(io_pstParams, off927BEE01B8013520, int8_t)
		,	var6B5CC2E17F5C32BB
		,	varACAED283F41E87B9
		,	varAE96BD3F751512E1
		,	var84C7885DF5877462
		,	varAA8C6B01FD937132
		,	varE5A300604CF5FEF5
		);
		ProcNS(tune_TC_B)(
			*DataPointer(io_pstParams, off49A048A1198E393B, int8_t)
		,	var6B5CC2E17F5C32BB
		,	varACAED283F41E87B9
		,	varAE96BD3F751512E1
		,	var84C7885DF5877462
		,	varAA8C6B01FD937132
		,	varE5A300604CF5FEF5
		);
		ProcNS(fmt_TC)(
			var6B5CC2E17F5C32BB
		,	varACAED283F41E87B9
		,	varAE96BD3F751512E1
		,	var84C7885DF5877462
		,	varAA8C6B01FD937132
		,	varE5A300604CF5FEF5
		,	DataPointer(o_pstData, off9CD622DC5C109C12, int16_t)
		,	DataPointer(o_pstData, offDD323B291D280E82, int16_t)
		,	DataPointer(o_pstData, off88324F22D387E652, int16_t)
		);
	};
	{
		int16_t pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			uint16_t var429BFD5760E61A32;
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			stInterp0[idx0].var429BFD5760E61A32 = 0;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			stInterp0[idx0].var429BFD5760E61A32 = 0;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			stInterp0[idx0].var429BFD5760E61A32 = 328;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			stInterp0[idx0].var429BFD5760E61A32 = 655;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			stInterp0[idx0].var429BFD5760E61A32 = 655;
			pos0++;
			idx0++;
		};
		*DataPointer(o_pstData, off94B6EB1CB8BA9870, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var429BFD5760E61A32, stInterp0[1].var429BFD5760E61A32, scale0);
	};
	{
		int16_t pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			uint16_t var8C3F00454E03E403;
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			stInterp0[idx0].var8C3F00454E03E403 = 10;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			stInterp0[idx0].var8C3F00454E03E403 = 10;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			stInterp0[idx0].var8C3F00454E03E403 = 10;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			stInterp0[idx0].var8C3F00454E03E403 = 10;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			stInterp0[idx0].var8C3F00454E03E403 = 10;
			pos0++;
			idx0++;
		};
		*DataPointer(o_pstData, off3FFD84B1A729E10C, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var8C3F00454E03E403, stInterp0[1].var8C3F00454E03E403, scale0);
	};
	ProcNS(fmt_DD)(
		DataPointer(o_pstData, off3FFD84B1A729E10C, uint16_t)
	,	DataPointer(o_pstData, off1B37B924A14DBE04, uint16_t)
	);
	{
		uint8_t varAB478F3EFC840EEB[siz3CB939AC8A5E4B97];
		uint8_t varB5F072D6E5347FEF[siz3CB939AC8A5E4B97];
		uint16_t var0F7A0536073F7CD5[33];
		uint16_t var4466E9B746DAD95C[33];
		int16_t k0 = 0, pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			uint8_t var697658B4667B26D1;
			uint8_t var49E679770C7EB827;
			uint16_t var8C8D1BE8066B6E49[825];
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			static const uint16_t rom8C8D1BE8066B6E49[825] = {
				215, 225, 236, 248, 261, 269, 276, 282, 287, 294, 302, 307, 312, 312, 312, 315, 317, 315, 312, 312, 312, 305, 297, 292, 287, 279, 271, 264, 256, 248, 241, 230, 220
			,	223, 234, 246, 257, 269, 279, 289, 297, 305, 312, 320, 328, 335, 337, 338, 339, 340, 339, 338, 337, 335, 326, 317, 311, 305, 296, 287, 276, 266, 257, 248, 238, 228
			,	230, 243, 256, 266, 276, 289, 302, 312, 323, 330, 338, 348, 358, 361, 364, 364, 364, 364, 364, 361, 358, 348, 338, 330, 323, 312, 302, 289, 276, 266, 256, 246, 236
			,	238, 251, 264, 275, 287, 301, 315, 328, 340, 349, 358, 370, 381, 384, 387, 388, 389, 388, 387, 384, 381, 371, 361, 351, 340, 328, 315, 301, 287, 276, 266, 256, 246
			,	246, 259, 271, 284, 297, 312, 328, 343, 358, 369, 379, 392, 404, 407, 410, 412, 415, 412, 410, 407, 404, 394, 384, 371, 358, 343, 328, 312, 297, 287, 276, 266, 256
			,	251, 264, 276, 291, 305, 321, 338, 355, 371, 383, 394, 408, 422, 426, 430, 433, 435, 433, 430, 426, 422, 411, 399, 387, 374, 356, 338, 321, 305, 293, 282, 270, 259
			,	256, 269, 282, 297, 312, 330, 348, 366, 384, 397, 410, 425, 440, 445, 451, 453, 456, 453, 451, 445, 440, 428, 415, 402, 389, 369, 348, 330, 312, 300, 287, 274, 261
			,	259, 273, 287, 302, 317, 337, 356, 376, 397, 411, 425, 442, 458, 463, 468, 472, 476, 472, 468, 463, 458, 444, 430, 416, 402, 380, 358, 339, 320, 306, 292, 279, 266
			,	261, 276, 292, 307, 323, 343, 364, 387, 410, 425, 440, 458, 476, 481, 486, 492, 497, 492, 486, 481, 476, 461, 445, 430, 415, 392, 369, 348, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 344, 366, 388, 410, 426, 443, 461, 479, 484, 489, 494, 499, 494, 489, 484, 479, 463, 448, 433, 417, 394, 371, 349, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 389, 410, 428, 445, 463, 481, 486, 492, 497, 502, 497, 492, 486, 481, 466, 451, 435, 420, 397, 374, 351, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 390, 412, 430, 448, 466, 484, 489, 494, 500, 507, 500, 494, 489, 484, 468, 453, 436, 420, 397, 374, 351, 328, 312, 297, 283, 269
			,	261, 276, 292, 307, 323, 346, 369, 392, 415, 433, 451, 468, 486, 492, 497, 504, 512, 504, 497, 492, 486, 471, 456, 438, 420, 397, 374, 351, 328, 312, 297, 282, 266
			,	259, 274, 289, 303, 317, 340, 364, 385, 407, 425, 443, 460, 476, 481, 486, 494, 502, 495, 489, 484, 479, 462, 445, 429, 412, 390, 369, 346, 323, 308, 294, 279, 264
			,	256, 271, 287, 300, 312, 335, 358, 379, 399, 417, 435, 451, 466, 471, 476, 484, 492, 486, 481, 476, 471, 453, 435, 420, 404, 384, 364, 340, 317, 305, 292, 276, 261
			,	251, 266, 282, 296, 310, 330, 351, 371, 392, 408, 425, 442, 458, 463, 468, 475, 481, 476, 471, 466, 461, 444, 428, 412, 397, 376, 356, 334, 312, 300, 287, 271, 256
			,	246, 261, 276, 292, 307, 325, 343, 364, 384, 399, 415, 433, 451, 456, 461, 466, 471, 466, 461, 456, 451, 435, 420, 404, 389, 369, 348, 328, 307, 294, 282, 266, 251
			,	241, 255, 269, 283, 297, 314, 330, 349, 369, 383, 397, 412, 428, 431, 435, 440, 445, 440, 435, 431, 428, 413, 399, 384, 369, 351, 333, 315, 297, 284, 271, 257, 243
			,	236, 248, 261, 274, 287, 302, 317, 335, 353, 366, 379, 392, 404, 407, 410, 415, 420, 415, 410, 407, 404, 392, 379, 364, 348, 333, 317, 302, 287, 274, 261, 248, 236
			,	228, 241, 253, 265, 276, 291, 305, 320, 335, 347, 358, 370, 381, 384, 387, 390, 394, 390, 387, 384, 381, 369, 356, 343, 330, 316, 302, 288, 274, 262, 251, 239, 228
			,	220, 233, 246, 256, 266, 279, 292, 305, 317, 328, 338, 348, 358, 361, 364, 366, 369, 366, 364, 361, 358, 346, 333, 323, 312, 300, 287, 274, 261, 251, 241, 230, 220
			,	212, 224, 236, 246, 256, 268, 279, 289, 300, 308, 317, 326, 335, 337, 338, 340, 343, 340, 338, 335, 333, 323, 312, 303, 294, 283, 271, 261, 251, 241, 230, 220, 210
			,	205, 215, 225, 236, 246, 256, 266, 274, 282, 289, 297, 305, 312, 312, 312, 315, 317, 315, 312, 310, 307, 300, 292, 284, 276, 266, 256, 248, 241, 230, 220, 210, 200
			,	197, 206, 215, 225, 236, 243, 251, 259, 266, 271, 276, 282, 287, 288, 289, 291, 292, 289, 287, 284, 282, 275, 269, 264, 259, 250, 241, 234, 228, 218, 207, 198, 189
			,	189, 197, 205, 215, 225, 230, 236, 243, 251, 253, 256, 259, 261, 264, 266, 266, 266, 264, 261, 259, 256, 251, 246, 243, 241, 233, 225, 220, 215, 205, 195, 187, 179
			};
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			memcpy(stInterp0[idx0].var8C8D1BE8066B6E49, rom8C8D1BE8066B6E49, sizeof(uint16_t) * 825);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			static const uint16_t rom8C8D1BE8066B6E49[825] = {
				215, 225, 236, 248, 261, 269, 276, 282, 287, 294, 302, 307, 312, 312, 312, 315, 317, 315, 312, 312, 312, 305, 297, 292, 287, 279, 271, 264, 256, 248, 241, 230, 220
			,	223, 234, 246, 257, 269, 279, 289, 297, 305, 312, 320, 328, 335, 337, 338, 339, 340, 339, 338, 337, 335, 326, 317, 311, 305, 296, 287, 276, 266, 257, 248, 238, 228
			,	230, 243, 256, 266, 276, 289, 302, 312, 323, 330, 338, 348, 358, 361, 364, 364, 364, 364, 364, 361, 358, 348, 338, 330, 323, 312, 302, 289, 276, 266, 256, 246, 236
			,	238, 251, 264, 275, 287, 301, 315, 328, 340, 349, 358, 370, 381, 384, 387, 388, 389, 388, 387, 384, 381, 371, 361, 351, 340, 328, 315, 301, 287, 276, 266, 256, 246
			,	246, 259, 271, 284, 297, 312, 328, 343, 358, 369, 379, 392, 404, 407, 410, 412, 415, 412, 410, 407, 404, 394, 384, 371, 358, 343, 328, 312, 297, 287, 276, 266, 256
			,	251, 264, 276, 291, 305, 321, 338, 355, 371, 383, 394, 408, 422, 426, 430, 433, 435, 433, 430, 426, 422, 411, 399, 387, 374, 356, 338, 321, 305, 293, 282, 270, 259
			,	256, 269, 282, 297, 312, 330, 348, 366, 384, 397, 410, 425, 440, 445, 451, 453, 456, 453, 451, 445, 440, 428, 415, 402, 389, 369, 348, 330, 312, 300, 287, 274, 261
			,	259, 273, 287, 302, 317, 337, 356, 376, 397, 411, 425, 442, 458, 463, 468, 472, 476, 472, 468, 463, 458, 444, 430, 416, 402, 380, 358, 339, 320, 306, 292, 279, 266
			,	261, 276, 292, 307, 323, 343, 364, 387, 410, 425, 440, 458, 476, 481, 486, 492, 497, 492, 486, 481, 476, 461, 445, 430, 415, 392, 369, 348, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 344, 366, 388, 410, 426, 443, 461, 479, 484, 489, 494, 499, 494, 489, 484, 479, 463, 448, 433, 417, 394, 371, 349, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 389, 410, 428, 445, 463, 481, 486, 492, 497, 502, 497, 492, 486, 481, 466, 451, 435, 420, 397, 374, 351, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 390, 412, 430, 448, 466, 484, 489, 494, 500, 507, 500, 494, 489, 484, 468, 453, 436, 420, 397, 374, 351, 328, 312, 297, 283, 269
			,	261, 276, 292, 307, 323, 346, 369, 392, 415, 433, 451, 468, 486, 492, 497, 504, 512, 504, 497, 492, 486, 471, 456, 438, 420, 397, 374, 351, 328, 312, 297, 282, 266
			,	259, 274, 289, 303, 317, 340, 364, 385, 407, 425, 443, 460, 476, 481, 486, 494, 502, 495, 489, 484, 479, 462, 445, 429, 412, 390, 369, 346, 323, 308, 294, 279, 264
			,	256, 271, 287, 300, 312, 335, 358, 379, 399, 417, 435, 451, 466, 471, 476, 484, 492, 486, 481, 476, 471, 453, 435, 420, 404, 384, 364, 340, 317, 305, 292, 276, 261
			,	251, 266, 282, 296, 310, 330, 351, 371, 392, 408, 425, 442, 458, 463, 468, 475, 481, 476, 471, 466, 461, 444, 428, 412, 397, 376, 356, 334, 312, 300, 287, 271, 256
			,	246, 261, 276, 292, 307, 325, 343, 364, 384, 399, 415, 433, 451, 456, 461, 466, 471, 466, 461, 456, 451, 435, 420, 404, 389, 369, 348, 328, 307, 294, 282, 266, 251
			,	241, 255, 269, 283, 297, 314, 330, 349, 369, 383, 397, 412, 428, 431, 435, 440, 445, 440, 435, 431, 428, 413, 399, 384, 369, 351, 333, 315, 297, 284, 271, 257, 243
			,	236, 248, 261, 274, 287, 302, 317, 335, 353, 366, 379, 392, 404, 407, 410, 415, 420, 415, 410, 407, 404, 392, 379, 364, 348, 333, 317, 302, 287, 274, 261, 248, 236
			,	228, 241, 253, 265, 276, 291, 305, 320, 335, 347, 358, 370, 381, 384, 387, 390, 394, 390, 387, 384, 381, 369, 356, 343, 330, 316, 302, 288, 274, 262, 251, 239, 228
			,	220, 233, 246, 256, 266, 279, 292, 305, 317, 328, 338, 348, 358, 361, 364, 366, 369, 366, 364, 361, 358, 346, 333, 323, 312, 300, 287, 274, 261, 251, 241, 230, 220
			,	212, 224, 236, 246, 256, 268, 279, 289, 300, 308, 317, 326, 335, 337, 338, 340, 343, 340, 338, 335, 333, 323, 312, 303, 294, 283, 271, 261, 251, 241, 230, 220, 210
			,	205, 215, 225, 236, 246, 256, 266, 274, 282, 289, 297, 305, 312, 312, 312, 315, 317, 315, 312, 310, 307, 300, 292, 284, 276, 266, 256, 248, 241, 230, 220, 210, 200
			,	197, 206, 215, 225, 236, 243, 251, 259, 266, 271, 276, 282, 287, 288, 289, 291, 292, 289, 287, 284, 282, 275, 269, 264, 259, 250, 241, 234, 228, 218, 207, 198, 189
			,	189, 197, 205, 215, 225, 230, 236, 243, 251, 253, 256, 259, 261, 264, 266, 266, 266, 264, 261, 259, 256, 251, 246, 243, 241, 233, 225, 220, 215, 205, 195, 187, 179
			};
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			memcpy(stInterp0[idx0].var8C8D1BE8066B6E49, rom8C8D1BE8066B6E49, sizeof(uint16_t) * 825);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			static const uint16_t rom8C8D1BE8066B6E49[825] = {
				215, 225, 236, 248, 261, 269, 276, 282, 287, 294, 302, 307, 312, 312, 312, 315, 317, 315, 312, 312, 312, 305, 297, 292, 287, 279, 271, 264, 256, 248, 241, 230, 220
			,	223, 234, 246, 257, 269, 279, 289, 297, 305, 312, 320, 328, 335, 337, 338, 339, 340, 339, 338, 337, 335, 326, 317, 311, 305, 296, 287, 276, 266, 257, 248, 238, 228
			,	230, 243, 256, 266, 276, 289, 302, 312, 323, 330, 338, 348, 358, 361, 364, 364, 364, 364, 364, 361, 358, 348, 338, 330, 323, 312, 302, 289, 276, 266, 256, 246, 236
			,	238, 251, 264, 275, 287, 301, 315, 328, 340, 349, 358, 370, 381, 384, 387, 388, 389, 388, 387, 384, 381, 371, 361, 351, 340, 328, 315, 301, 287, 276, 266, 256, 246
			,	246, 259, 271, 284, 297, 312, 328, 343, 358, 369, 379, 392, 404, 407, 410, 412, 415, 412, 410, 407, 404, 394, 384, 371, 358, 343, 328, 312, 297, 287, 276, 266, 256
			,	251, 264, 276, 291, 305, 321, 338, 355, 371, 383, 394, 408, 422, 426, 430, 433, 435, 433, 430, 426, 422, 411, 399, 387, 374, 356, 338, 321, 305, 293, 282, 270, 259
			,	256, 269, 282, 297, 312, 330, 348, 366, 384, 397, 410, 425, 440, 445, 451, 453, 456, 453, 451, 445, 440, 428, 415, 402, 389, 369, 348, 330, 312, 300, 287, 274, 261
			,	259, 273, 287, 302, 317, 337, 356, 376, 397, 411, 425, 442, 458, 463, 468, 472, 476, 472, 468, 463, 458, 444, 430, 416, 402, 380, 358, 339, 320, 306, 292, 279, 266
			,	261, 276, 292, 307, 323, 343, 364, 387, 410, 425, 440, 458, 476, 481, 486, 492, 497, 492, 486, 481, 476, 461, 445, 430, 415, 392, 369, 348, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 344, 366, 388, 410, 426, 443, 461, 479, 484, 489, 494, 499, 494, 489, 484, 479, 463, 448, 433, 417, 394, 371, 349, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 389, 410, 428, 445, 463, 481, 486, 492, 497, 502, 497, 492, 486, 481, 466, 451, 435, 420, 397, 374, 351, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 390, 412, 430, 448, 466, 484, 489, 494, 500, 507, 500, 494, 489, 484, 468, 453, 436, 420, 397, 374, 351, 328, 312, 297, 283, 269
			,	261, 276, 292, 307, 323, 346, 369, 392, 415, 433, 451, 468, 486, 492, 497, 504, 512, 504, 497, 492, 486, 471, 456, 438, 420, 397, 374, 351, 328, 312, 297, 282, 266
			,	259, 274, 289, 303, 317, 340, 364, 385, 407, 425, 443, 460, 476, 481, 486, 494, 502, 495, 489, 484, 479, 462, 445, 429, 412, 390, 369, 346, 323, 308, 294, 279, 264
			,	256, 271, 287, 300, 312, 335, 358, 379, 399, 417, 435, 451, 466, 471, 476, 484, 492, 486, 481, 476, 471, 453, 435, 420, 404, 384, 364, 340, 317, 305, 292, 276, 261
			,	251, 266, 282, 296, 310, 330, 351, 371, 392, 408, 425, 442, 458, 463, 468, 475, 481, 476, 471, 466, 461, 444, 428, 412, 397, 376, 356, 334, 312, 300, 287, 271, 256
			,	246, 261, 276, 292, 307, 325, 343, 364, 384, 399, 415, 433, 451, 456, 461, 466, 471, 466, 461, 456, 451, 435, 420, 404, 389, 369, 348, 328, 307, 294, 282, 266, 251
			,	241, 255, 269, 283, 297, 314, 330, 349, 369, 383, 397, 412, 428, 431, 435, 440, 445, 440, 435, 431, 428, 413, 399, 384, 369, 351, 333, 315, 297, 284, 271, 257, 243
			,	236, 248, 261, 274, 287, 302, 317, 335, 353, 366, 379, 392, 404, 407, 410, 415, 420, 415, 410, 407, 404, 392, 379, 364, 348, 333, 317, 302, 287, 274, 261, 248, 236
			,	228, 241, 253, 265, 276, 291, 305, 320, 335, 347, 358, 370, 381, 384, 387, 390, 394, 390, 387, 384, 381, 369, 356, 343, 330, 316, 302, 288, 274, 262, 251, 239, 228
			,	220, 233, 246, 256, 266, 279, 292, 305, 317, 328, 338, 348, 358, 361, 364, 366, 369, 366, 364, 361, 358, 346, 333, 323, 312, 300, 287, 274, 261, 251, 241, 230, 220
			,	212, 224, 236, 246, 256, 268, 279, 289, 300, 308, 317, 326, 335, 337, 338, 340, 343, 340, 338, 335, 333, 323, 312, 303, 294, 283, 271, 261, 251, 241, 230, 220, 210
			,	205, 215, 225, 236, 246, 256, 266, 274, 282, 289, 297, 305, 312, 312, 312, 315, 317, 315, 312, 310, 307, 300, 292, 284, 276, 266, 256, 248, 241, 230, 220, 210, 200
			,	197, 206, 215, 225, 236, 243, 251, 259, 266, 271, 276, 282, 287, 288, 289, 291, 292, 289, 287, 284, 282, 275, 269, 264, 259, 250, 241, 234, 228, 218, 207, 198, 189
			,	189, 197, 205, 215, 225, 230, 236, 243, 251, 253, 256, 259, 261, 264, 266, 266, 266, 264, 261, 259, 256, 251, 246, 243, 241, 233, 225, 220, 215, 205, 195, 187, 179
			};
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			memcpy(stInterp0[idx0].var8C8D1BE8066B6E49, rom8C8D1BE8066B6E49, sizeof(uint16_t) * 825);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			static const uint16_t rom8C8D1BE8066B6E49[825] = {
				215, 225, 236, 248, 261, 269, 276, 282, 287, 294, 302, 307, 312, 312, 312, 315, 317, 315, 312, 312, 312, 305, 297, 292, 287, 279, 271, 264, 256, 248, 241, 230, 220
			,	223, 234, 246, 257, 269, 279, 289, 297, 305, 312, 320, 328, 335, 337, 338, 339, 340, 339, 338, 337, 335, 326, 317, 311, 305, 296, 287, 276, 266, 257, 248, 238, 228
			,	230, 243, 256, 266, 276, 289, 302, 312, 323, 330, 338, 348, 358, 361, 364, 364, 364, 364, 364, 361, 358, 348, 338, 330, 323, 312, 302, 289, 276, 266, 256, 246, 236
			,	238, 251, 264, 275, 287, 301, 315, 328, 340, 349, 358, 370, 381, 384, 387, 388, 389, 388, 387, 384, 381, 371, 361, 351, 340, 328, 315, 301, 287, 276, 266, 256, 246
			,	246, 259, 271, 284, 297, 312, 328, 343, 358, 369, 379, 392, 404, 407, 410, 412, 415, 412, 410, 407, 404, 394, 384, 371, 358, 343, 328, 312, 297, 287, 276, 266, 256
			,	251, 264, 276, 291, 305, 321, 338, 355, 371, 383, 394, 408, 422, 426, 430, 433, 435, 433, 430, 426, 422, 411, 399, 387, 374, 356, 338, 321, 305, 293, 282, 270, 259
			,	256, 269, 282, 297, 312, 330, 348, 366, 384, 397, 410, 425, 440, 445, 451, 453, 456, 453, 451, 445, 440, 428, 415, 402, 389, 369, 348, 330, 312, 300, 287, 274, 261
			,	259, 273, 287, 302, 317, 337, 356, 376, 397, 411, 425, 442, 458, 463, 468, 472, 476, 472, 468, 463, 458, 444, 430, 416, 402, 380, 358, 339, 320, 306, 292, 279, 266
			,	261, 276, 292, 307, 323, 343, 364, 387, 410, 425, 440, 458, 476, 481, 486, 492, 497, 492, 486, 481, 476, 461, 445, 430, 415, 392, 369, 348, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 344, 366, 388, 410, 426, 443, 461, 479, 484, 489, 494, 499, 494, 489, 484, 479, 463, 448, 433, 417, 394, 371, 349, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 389, 410, 428, 445, 463, 481, 486, 492, 497, 502, 497, 492, 486, 481, 466, 451, 435, 420, 397, 374, 351, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 390, 412, 430, 448, 466, 484, 489, 494, 500, 507, 500, 494, 489, 484, 468, 453, 436, 420, 397, 374, 351, 328, 312, 297, 283, 269
			,	261, 276, 292, 307, 323, 346, 369, 392, 415, 433, 451, 468, 486, 492, 497, 504, 512, 504, 497, 492, 486, 471, 456, 438, 420, 397, 374, 351, 328, 312, 297, 282, 266
			,	259, 274, 289, 303, 317, 340, 364, 385, 407, 425, 443, 460, 476, 481, 486, 494, 502, 495, 489, 484, 479, 462, 445, 429, 412, 390, 369, 346, 323, 308, 294, 279, 264
			,	256, 271, 287, 300, 312, 335, 358, 379, 399, 417, 435, 451, 466, 471, 476, 484, 492, 486, 481, 476, 471, 453, 435, 420, 404, 384, 364, 340, 317, 305, 292, 276, 261
			,	251, 266, 282, 296, 310, 330, 351, 371, 392, 408, 425, 442, 458, 463, 468, 475, 481, 476, 471, 466, 461, 444, 428, 412, 397, 376, 356, 334, 312, 300, 287, 271, 256
			,	246, 261, 276, 292, 307, 325, 343, 364, 384, 399, 415, 433, 451, 456, 461, 466, 471, 466, 461, 456, 451, 435, 420, 404, 389, 369, 348, 328, 307, 294, 282, 266, 251
			,	241, 255, 269, 283, 297, 314, 330, 349, 369, 383, 397, 412, 428, 431, 435, 440, 445, 440, 435, 431, 428, 413, 399, 384, 369, 351, 333, 315, 297, 284, 271, 257, 243
			,	236, 248, 261, 274, 287, 302, 317, 335, 353, 366, 379, 392, 404, 407, 410, 415, 420, 415, 410, 407, 404, 392, 379, 364, 348, 333, 317, 302, 287, 274, 261, 248, 236
			,	228, 241, 253, 265, 276, 291, 305, 320, 335, 347, 358, 370, 381, 384, 387, 390, 394, 390, 387, 384, 381, 369, 356, 343, 330, 316, 302, 288, 274, 262, 251, 239, 228
			,	220, 233, 246, 256, 266, 279, 292, 305, 317, 328, 338, 348, 358, 361, 364, 366, 369, 366, 364, 361, 358, 346, 333, 323, 312, 300, 287, 274, 261, 251, 241, 230, 220
			,	212, 224, 236, 246, 256, 268, 279, 289, 300, 308, 317, 326, 335, 337, 338, 340, 343, 340, 338, 335, 333, 323, 312, 303, 294, 283, 271, 261, 251, 241, 230, 220, 210
			,	205, 215, 225, 236, 246, 256, 266, 274, 282, 289, 297, 305, 312, 312, 312, 315, 317, 315, 312, 310, 307, 300, 292, 284, 276, 266, 256, 248, 241, 230, 220, 210, 200
			,	197, 206, 215, 225, 236, 243, 251, 259, 266, 271, 276, 282, 287, 288, 289, 291, 292, 289, 287, 284, 282, 275, 269, 264, 259, 250, 241, 234, 228, 218, 207, 198, 189
			,	189, 197, 205, 215, 225, 230, 236, 243, 251, 253, 256, 259, 261, 264, 266, 266, 266, 264, 261, 259, 256, 251, 246, 243, 241, 233, 225, 220, 215, 205, 195, 187, 179
			};
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			memcpy(stInterp0[idx0].var8C8D1BE8066B6E49, rom8C8D1BE8066B6E49, sizeof(uint16_t) * 825);
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			static const uint16_t rom8C8D1BE8066B6E49[825] = {
				215, 225, 236, 248, 261, 269, 276, 282, 287, 294, 302, 307, 312, 312, 312, 315, 317, 315, 312, 312, 312, 305, 297, 292, 287, 279, 271, 264, 256, 248, 241, 230, 220
			,	223, 234, 246, 257, 269, 279, 289, 297, 305, 312, 320, 328, 335, 337, 338, 339, 340, 339, 338, 337, 335, 326, 317, 311, 305, 296, 287, 276, 266, 257, 248, 238, 228
			,	230, 243, 256, 266, 276, 289, 302, 312, 323, 330, 338, 348, 358, 361, 364, 364, 364, 364, 364, 361, 358, 348, 338, 330, 323, 312, 302, 289, 276, 266, 256, 246, 236
			,	238, 251, 264, 275, 287, 301, 315, 328, 340, 349, 358, 370, 381, 384, 387, 388, 389, 388, 387, 384, 381, 371, 361, 351, 340, 328, 315, 301, 287, 276, 266, 256, 246
			,	246, 259, 271, 284, 297, 312, 328, 343, 358, 369, 379, 392, 404, 407, 410, 412, 415, 412, 410, 407, 404, 394, 384, 371, 358, 343, 328, 312, 297, 287, 276, 266, 256
			,	251, 264, 276, 291, 305, 321, 338, 355, 371, 383, 394, 408, 422, 426, 430, 433, 435, 433, 430, 426, 422, 411, 399, 387, 374, 356, 338, 321, 305, 293, 282, 270, 259
			,	256, 269, 282, 297, 312, 330, 348, 366, 384, 397, 410, 425, 440, 445, 451, 453, 456, 453, 451, 445, 440, 428, 415, 402, 389, 369, 348, 330, 312, 300, 287, 274, 261
			,	259, 273, 287, 302, 317, 337, 356, 376, 397, 411, 425, 442, 458, 463, 468, 472, 476, 472, 468, 463, 458, 444, 430, 416, 402, 380, 358, 339, 320, 306, 292, 279, 266
			,	261, 276, 292, 307, 323, 343, 364, 387, 410, 425, 440, 458, 476, 481, 486, 492, 497, 492, 486, 481, 476, 461, 445, 430, 415, 392, 369, 348, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 344, 366, 388, 410, 426, 443, 461, 479, 484, 489, 494, 499, 494, 489, 484, 479, 463, 448, 433, 417, 394, 371, 349, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 389, 410, 428, 445, 463, 481, 486, 492, 497, 502, 497, 492, 486, 481, 466, 451, 435, 420, 397, 374, 351, 328, 312, 297, 284, 271
			,	261, 276, 292, 307, 323, 346, 369, 390, 412, 430, 448, 466, 484, 489, 494, 500, 507, 500, 494, 489, 484, 468, 453, 436, 420, 397, 374, 351, 328, 312, 297, 283, 269
			,	261, 276, 292, 307, 323, 346, 369, 392, 415, 433, 451, 468, 486, 492, 497, 504, 512, 504, 497, 492, 486, 471, 456, 438, 420, 397, 374, 351, 328, 312, 297, 282, 266
			,	259, 274, 289, 303, 317, 340, 364, 385, 407, 425, 443, 460, 476, 481, 486, 494, 502, 495, 489, 484, 479, 462, 445, 429, 412, 390, 369, 346, 323, 308, 294, 279, 264
			,	256, 271, 287, 300, 312, 335, 358, 379, 399, 417, 435, 451, 466, 471, 476, 484, 492, 486, 481, 476, 471, 453, 435, 420, 404, 384, 364, 340, 317, 305, 292, 276, 261
			,	251, 266, 282, 296, 310, 330, 351, 371, 392, 408, 425, 442, 458, 463, 468, 475, 481, 476, 471, 466, 461, 444, 428, 412, 397, 376, 356, 334, 312, 300, 287, 271, 256
			,	246, 261, 276, 292, 307, 325, 343, 364, 384, 399, 415, 433, 451, 456, 461, 466, 471, 466, 461, 456, 451, 435, 420, 404, 389, 369, 348, 328, 307, 294, 282, 266, 251
			,	241, 255, 269, 283, 297, 314, 330, 349, 369, 383, 397, 412, 428, 431, 435, 440, 445, 440, 435, 431, 428, 413, 399, 384, 369, 351, 333, 315, 297, 284, 271, 257, 243
			,	236, 248, 261, 274, 287, 302, 317, 335, 353, 366, 379, 392, 404, 407, 410, 415, 420, 415, 410, 407, 404, 392, 379, 364, 348, 333, 317, 302, 287, 274, 261, 248, 236
			,	228, 241, 253, 265, 276, 291, 305, 320, 335, 347, 358, 370, 381, 384, 387, 390, 394, 390, 387, 384, 381, 369, 356, 343, 330, 316, 302, 288, 274, 262, 251, 239, 228
			,	220, 233, 246, 256, 266, 279, 292, 305, 317, 328, 338, 348, 358, 361, 364, 366, 369, 366, 364, 361, 358, 346, 333, 323, 312, 300, 287, 274, 261, 251, 241, 230, 220
			,	212, 224, 236, 246, 256, 268, 279, 289, 300, 308, 317, 326, 335, 337, 338, 340, 343, 340, 338, 335, 333, 323, 312, 303, 294, 283, 271, 261, 251, 241, 230, 220, 210
			,	205, 215, 225, 236, 246, 256, 266, 274, 282, 289, 297, 305, 312, 312, 312, 315, 317, 315, 312, 310, 307, 300, 292, 284, 276, 266, 256, 248, 241, 230, 220, 210, 200
			,	197, 206, 215, 225, 236, 243, 251, 259, 266, 271, 276, 282, 287, 288, 289, 291, 292, 289, 287, 284, 282, 275, 269, 264, 259, 250, 241, 234, 228, 218, 207, 198, 189
			,	189, 197, 205, 215, 225, 230, 236, 243, 251, 253, 256, 259, 261, 264, 266, 266, 266, 264, 261, 259, 256, 251, 246, 243, 241, 233, 225, 220, 215, 205, 195, 187, 179
			};
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			memcpy(stInterp0[idx0].var8C8D1BE8066B6E49, rom8C8D1BE8066B6E49, sizeof(uint16_t) * 825);
			pos0++;
			idx0++;
		};
		*DataPointer(&(varB5F072D6E5347FEF), off72A2AFA2754D827F, uint8_t) = (uint8_t) DataInterpolate(stInterp0[0].var697658B4667B26D1, stInterp0[1].var697658B4667B26D1, scale0);
		*DataPointer(&(varB5F072D6E5347FEF), offAE4DC7189CD4BF5D, uint8_t) = (uint8_t) DataInterpolate(stInterp0[0].var49E679770C7EB827, stInterp0[1].var49E679770C7EB827, scale0);
		for (k0 = 0; k0 < 825; k0++) {
			DataPointer(&(varB5F072D6E5347FEF), offDB2BC1B6C7EA99EA, uint16_t)[k0] = (uint16_t) DataInterpolate(stInterp0[0].var8C8D1BE8066B6E49[k0], stInterp0[1].var8C8D1BE8066B6E49[k0], scale0);
		};
		ProcNS(zoom_DD_mkBuf)(
			&(varB5F072D6E5347FEF)
		,	var0F7A0536073F7CD5
		,	sizeof(var0F7A0536073F7CD5)/sizeof(var0F7A0536073F7CD5[0])
		,	var4466E9B746DAD95C
		,	sizeof(var4466E9B746DAD95C)/sizeof(var4466E9B746DAD95C[0])
		);
		ProcNS(zoom_DD)(
			*DataPointer(io_pstParams, off42DC478ECF8A6E60, uint16_t)
		,	*DataPointer(io_pstParams, off3E263E9FF3CE8FA3, uint16_t)
		,	*DataPointer(io_pstParams, off25C2C9BDE3113654, uint8_t)
		,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
		,	&(varB5F072D6E5347FEF)
		,	var0F7A0536073F7CD5
		,	sizeof(var0F7A0536073F7CD5)/sizeof(var0F7A0536073F7CD5[0])
		,	var4466E9B746DAD95C
		,	sizeof(var4466E9B746DAD95C)/sizeof(var4466E9B746DAD95C[0])
		,	&(varAB478F3EFC840EEB)
		);
		ProcNS(geom_DD)(
			DataPointer(io_pstParams, offE073442BDF64D249, void)
		,	*DataPointer(io_pstParams, off25C2C9BDE3113654, uint8_t)
		,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
		,	&(varAB478F3EFC840EEB)
		,	DataPointer(o_pstData, off34AECBA5B522966F, void)
		);
	};
	*DataPointer(o_pstData, offC3D21FE5CE039E97, int16_t) = -2048;
	*DataPointer(o_pstData, offE50963D91DE73CC5, uint16_t) = 2047;
	{
		int16_t varC0A238AC481333F4[5];
		int16_t varB054AEA50B761A0A[5];
		if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) == 1) {
			if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 384) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-120, 0, 0, 0,    0, 0
				,	   0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -48, -37, -32, 14, 15 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (
				(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 384)
			&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 768)
			) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-32, 0, 0, 0,    0, 0
				,	  0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -35, -31, -27, 16, 29 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (
				(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 768)
			&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1536)
			) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-32, 0, 0, 0,    0, 0
				,	  0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -22, -24, -22, 17, 43 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (
				(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 1536)
			&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 3072)
			) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-32, 0, 0, 0,    0, 0
				,	  0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -9, -18, -16, 18, 57 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 3072) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-8, 0, 0, 0,    0, 0
				,	 0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { 4, -12, -11, 19, 72 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else {
				memset(DataPointer(o_pstData, offE03550918FF836A9, int16_t), 0xff, sizeof(int16_t) * 11);
				memset(varC0A238AC481333F4, 0xff, sizeof(int16_t) * 5);
				memset(varB054AEA50B761A0A, 0xff, sizeof(int16_t) * 5);
			};
		} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
			if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 384) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-120, 0, 0, 0,    0, 0
				,	   0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -48, -37, -32, 14, 15 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (
				(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 384)
			&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 768)
			) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-32, 0, 0, 0,    0, 0
				,	  0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -35, -31, -27, 16, 29 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (
				(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 768)
			&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1536)
			) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-32, 0, 0, 0,    0, 0
				,	  0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -22, -24, -22, 17, 43 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (
				(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 1536)
			&&	(*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 3072)
			) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-32, 0, 0, 0,    0, 0
				,	  0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { -9, -18, -16, 18, 57 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) >= 3072) {
				static const int16_t romC8A4358DF95CCB98[11] = {
					-8, 0, 0, 0,    0, 0
				,	 0, 0, 0, 0, 2047
				};
				static const int16_t romC0A238AC481333F4[5] = { 4, -12, -11, 19, 72 };
				static const int16_t romB054AEA50B761A0A[5] = { 0, 0, 0, 32, 64 };
				memcpy(DataPointer(o_pstData, offE03550918FF836A9, int16_t), romC8A4358DF95CCB98, sizeof(int16_t) * 11);
				memcpy(varC0A238AC481333F4, romC0A238AC481333F4, sizeof(int16_t) * 5);
				memcpy(varB054AEA50B761A0A, romB054AEA50B761A0A, sizeof(int16_t) * 5);
			} else {
				memset(DataPointer(o_pstData, offE03550918FF836A9, int16_t), 0xff, sizeof(int16_t) * 11);
				memset(varC0A238AC481333F4, 0xff, sizeof(int16_t) * 5);
				memset(varB054AEA50B761A0A, 0xff, sizeof(int16_t) * 5);
			};
		} else {
			memset(DataPointer(o_pstData, offE03550918FF836A9, int16_t), 0xff, sizeof(int16_t) * 11);
			memset(varC0A238AC481333F4, 0xff, sizeof(int16_t) * 5);
			memset(varB054AEA50B761A0A, 0xff, sizeof(int16_t) * 5);
		};
		ProcNS(tune_GF_soshy2)(
			*DataPointer(io_pstParams, offB09A2D3D2C9C0187, uint8_t)
		,	*DataPointer(io_pstParams, offC0A8A3FA319F002C, uint8_t)
		,	DataPointer(o_pstData, offE03550918FF836A9, int16_t)
		,	varC0A238AC481333F4
		,	varB054AEA50B761A0A
		);
		ProcNS(fmt_GF_soshy2)(
			varC0A238AC481333F4
		,	varB054AEA50B761A0A
		,	DataPointer(o_pstData, offE03550918FF836A9, int16_t)
		,	DataPointer(o_pstData, offE6DF88272CC89D7D, int16_t)
		,	DataPointer(o_pstData, offDEAA77338B908F8B, int16_t)
		);
	};
	{
		static const uint8_t rom412D29D5FB8C8317[7] = { 3, 3, 3, 3, 3, 3, 3 };
		static const uint8_t rom650F8B22212D39A4[4] = { 3, 3, 3, 3 };
		memcpy(DataPointer(o_pstData, off2517B5974DC9E786, uint8_t), rom412D29D5FB8C8317, sizeof(uint8_t) * 7);
		memcpy(DataPointer(o_pstData, off7BD5F8E98F4C0B00, uint8_t), rom650F8B22212D39A4, sizeof(uint8_t) * 4);
		*DataPointer(o_pstData, off40DFCC82F4A95B14, uint8_t) = 6;
	};
	ProcNS(insert_scales_SIFI)(
		*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t)
	,	*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t)
	,	DataPointer(o_pstData, off7E81C9DB4DD24681, uint16_t)
	,	DataPointer(o_pstData, offF2AADEE3113B26F2, uint16_t)
	);
	{
		int16_t k0 = 0, pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			int16_t var1C6F51F1861E6CFF[11];
			int16_t var3ABB6C48E6C9EF1D[11];
			uint16_t varFFDD3B2D24B6778E;
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			static const int16_t rom1C6F51F1861E6CFF[11] = { 29, 191, -135, -283, -128, -276, -64, -209, -59, -201, 2047 };
			static const int16_t rom3ABB6C48E6C9EF1D[11] = { 14, 96, -68, -142, -65, -139, -32, -105, -30, -101, 2047 };
			memcpy(stInterp0[idx0].var1C6F51F1861E6CFF, rom1C6F51F1861E6CFF, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].var3ABB6C48E6C9EF1D, rom3ABB6C48E6C9EF1D, sizeof(int16_t) * 11);
			stInterp0[idx0].varFFDD3B2D24B6778E = 512;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			static const int16_t rom1C6F51F1861E6CFF[11] = { 43, 280, -211, -450, -200, -436, -99, -329, -91, -316, 2047 };
			static const int16_t rom3ABB6C48E6C9EF1D[11] = { 22, 140, -106, -225, -100, -218, -49, -165, -46, -158, 2047 };
			memcpy(stInterp0[idx0].var1C6F51F1861E6CFF, rom1C6F51F1861E6CFF, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].var3ABB6C48E6C9EF1D, rom3ABB6C48E6C9EF1D, sizeof(int16_t) * 11);
			stInterp0[idx0].varFFDD3B2D24B6778E = 512;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			static const int16_t rom1C6F51F1861E6CFF[11] = { 95, 602, -471, -1014, -443, -976, -219, -738, -201, -704, 2047 };
			static const int16_t rom3ABB6C48E6C9EF1D[11] = { 31, 201, -157, -338, -148, -326, -73, -246, -67, -235, 2047 };
			memcpy(stInterp0[idx0].var1C6F51F1861E6CFF, rom1C6F51F1861E6CFF, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].var3ABB6C48E6C9EF1D, rom3ABB6C48E6C9EF1D, sizeof(int16_t) * 11);
			stInterp0[idx0].varFFDD3B2D24B6778E = 768;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			static const int16_t rom1C6F51F1861E6CFF[11] = { 69, 438, -349, -760, -329, -729, -162, -550, -148, -524, 2047 };
			static const int16_t rom3ABB6C48E6C9EF1D[11] = { 46, 292, -233, -506, -219, -486, -108, -367, -99, -349, 2047 };
			memcpy(stInterp0[idx0].var1C6F51F1861E6CFF, rom1C6F51F1861E6CFF, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].var3ABB6C48E6C9EF1D, rom3ABB6C48E6C9EF1D, sizeof(int16_t) * 11);
			stInterp0[idx0].varFFDD3B2D24B6778E = 768;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			static const int16_t rom1C6F51F1861E6CFF[11] = { 107, 547, -408, -873, -382, -841, -190, -637, -172, -607, 2047 };
			static const int16_t rom3ABB6C48E6C9EF1D[11] = { 71, 365, -272, -583, -255, -561, -127, -425, -115, -405, 2047 };
			memcpy(stInterp0[idx0].var1C6F51F1861E6CFF, rom1C6F51F1861E6CFF, sizeof(int16_t) * 11);
			memcpy(stInterp0[idx0].var3ABB6C48E6C9EF1D, rom3ABB6C48E6C9EF1D, sizeof(int16_t) * 11);
			stInterp0[idx0].varFFDD3B2D24B6778E = 768;
			pos0++;
			idx0++;
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, offC06FF9F81130CC09, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].var1C6F51F1861E6CFF[k0], stInterp0[1].var1C6F51F1861E6CFF[k0], scale0);
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, off763B85D6D13433AB, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].var3ABB6C48E6C9EF1D[k0], stInterp0[1].var3ABB6C48E6C9EF1D[k0], scale0);
		};
		*DataPointer(o_pstData, offA00D28BD28522FAA, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].varFFDD3B2D24B6778E, stInterp0[1].varFFDD3B2D24B6778E, scale0);
	};
	ProcNS(tune_SIFI)(
		*DataPointer(io_pstParams, off22AEE8C6FE90BDC1, uint8_t)
	,	*DataPointer(io_pstParams, offB09A2D3D2C9C0187, uint8_t)
	,	DataPointer(o_pstData, offC06FF9F81130CC09, int16_t)
	,	DataPointer(o_pstData, off763B85D6D13433AB, int16_t)
	,	DataPointer(o_pstData, offA00D28BD28522FAA, uint16_t)
	);
	{
		static const uint16_t romA5FE85378A42AEDE[3] = { 112, 56, 64 };
		memcpy(DataPointer(o_pstData, off6BD00800B80833DA, uint16_t), romA5FE85378A42AEDE, sizeof(uint16_t) * 3);
		*DataPointer(o_pstData, offBF7B204D5199D6C5, uint16_t) = 128;
	};
	{
		uint16_t varBE76331B95DFC399;
		uint16_t varA295E0BDDE1938D1;
		int16_t k0 = 0, pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			int16_t varFC25165C9E55EFD1[11];
			uint16_t varBE76331B95DFC399;
			uint16_t varA295E0BDDE1938D1;
			uint16_t var6A2906A82A23B0A9;
			uint16_t var06BA9E06C76573E3;
		} stInterp0[2];
		if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 256) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 512) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 256) << 8) / (512 - 256);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 1024) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 512) << 8) / (1024 - 512);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 2048) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 1024) << 8) / (2048 - 1024);
		} else if (*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) < 4096) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, off4C6E08B5AFE8CDD4, uint16_t) - 2048) << 8) / (4096 - 2048);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			static const int16_t romFC25165C9E55EFD1[11] = { 752, -883, 846, 1720, 826, 1700, 414, 1282, 396, 1262, 2047 };
			memcpy(stInterp0[idx0].varFC25165C9E55EFD1, romFC25165C9E55EFD1, sizeof(int16_t) * 11);
			stInterp0[idx0].varBE76331B95DFC399 = 2400;
			stInterp0[idx0].varA295E0BDDE1938D1 = 2419;
			stInterp0[idx0].var6A2906A82A23B0A9 = 461;
			stInterp0[idx0].var06BA9E06C76573E3 = 690;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			static const int16_t romFC25165C9E55EFD1[11] = { 511, -597, 573, 1164, 561, 1152, 280, 868, 270, 856, 2047 };
			memcpy(stInterp0[idx0].varFC25165C9E55EFD1, romFC25165C9E55EFD1, sizeof(int16_t) * 11);
			stInterp0[idx0].varBE76331B95DFC399 = 4000;
			stInterp0[idx0].varA295E0BDDE1938D1 = 4019;
			stInterp0[idx0].var6A2906A82A23B0A9 = 442;
			stInterp0[idx0].var06BA9E06C76573E3 = 470;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			static const int16_t romFC25165C9E55EFD1[11] = { 304, -353, 339, 690, 332, 683, 166, 514, 160, 507, 2047 };
			memcpy(stInterp0[idx0].varFC25165C9E55EFD1, romFC25165C9E55EFD1, sizeof(int16_t) * 11);
			stInterp0[idx0].varBE76331B95DFC399 = 5600;
			stInterp0[idx0].varA295E0BDDE1938D1 = 5622;
			stInterp0[idx0].var6A2906A82A23B0A9 = 256;
			stInterp0[idx0].var06BA9E06C76573E3 = 280;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			static const int16_t romFC25165C9E55EFD1[11] = { 215, -259, 249, 508, 247, 504, 122, 378, 120, 375, 2047 };
			memcpy(stInterp0[idx0].varFC25165C9E55EFD1, romFC25165C9E55EFD1, sizeof(int16_t) * 11);
			stInterp0[idx0].varBE76331B95DFC399 = 5600;
			stInterp0[idx0].varA295E0BDDE1938D1 = 5622;
			stInterp0[idx0].var6A2906A82A23B0A9 = 59;
			stInterp0[idx0].var06BA9E06C76573E3 = 200;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			static const int16_t romFC25165C9E55EFD1[11] = { 152, -171, 164, 333, 159, 329, 80, 248, 76, 244, 2047 };
			memcpy(stInterp0[idx0].varFC25165C9E55EFD1, romFC25165C9E55EFD1, sizeof(int16_t) * 11);
			stInterp0[idx0].varBE76331B95DFC399 = 5600;
			stInterp0[idx0].varA295E0BDDE1938D1 = 5622;
			stInterp0[idx0].var6A2906A82A23B0A9 = 51;
			stInterp0[idx0].var06BA9E06C76573E3 = 140;
			pos0++;
			idx0++;
		};
		for (k0 = 0; k0 < 11; k0++) {
			DataPointer(o_pstData, offF98E3B30D0770854, int16_t)[k0] = (int16_t) DataInterpolate(stInterp0[0].varFC25165C9E55EFD1[k0], stInterp0[1].varFC25165C9E55EFD1[k0], scale0);
		};
		varBE76331B95DFC399 = (uint16_t) DataInterpolate(stInterp0[0].varBE76331B95DFC399, stInterp0[1].varBE76331B95DFC399, scale0);
		varA295E0BDDE1938D1 = (uint16_t) DataInterpolate(stInterp0[0].varA295E0BDDE1938D1, stInterp0[1].varA295E0BDDE1938D1, scale0);
		*DataPointer(o_pstData, off9CE8DDD880620928, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var6A2906A82A23B0A9, stInterp0[1].var6A2906A82A23B0A9, scale0);
		*DataPointer(o_pstData, off6DF0958DF3EFB3A8, uint16_t) = (uint16_t) DataInterpolate(stInterp0[0].var06BA9E06C76573E3, stInterp0[1].var06BA9E06C76573E3, scale0);
		ProcNS(tune_NM)(
			*DataPointer(io_pstParams, off6DDAAF48981565F6, uint8_t)
		,	DataPointer(o_pstData, offF98E3B30D0770854, int16_t)
		,	&(varBE76331B95DFC399)
		,	&(varA295E0BDDE1938D1)
		,	DataPointer(o_pstData, off9CE8DDD880620928, uint16_t)
		,	DataPointer(o_pstData, off6DF0958DF3EFB3A8, uint16_t)
		);
		ProcNS(fmt_NM)(
			varBE76331B95DFC399
		,	varA295E0BDDE1938D1
		,	DataPointer(o_pstData, offF98E3B30D0770854, int16_t)
		,	DataPointer(o_pstData, off9CE8DDD880620928, uint16_t)
		,	DataPointer(o_pstData, off6DF0958DF3EFB3A8, uint16_t)
		,	DataPointer(o_pstData, off7D7EF71161DF0A2B, int16_t)
		);
	};
	{
		uint8_t varAB478F3EFC840EEB[siz7BD8E61CD29C41DD];
		uint8_t varB5F072D6E5347FEF[siz7BD8E61CD29C41DD];
		uint16_t var0F7A0536073F7CD5[33];
		uint16_t var4466E9B746DAD95C[33];
		*DataPointer(&(varB5F072D6E5347FEF), offA0139FF16C69F210, uint8_t) = 25;
		*DataPointer(&(varB5F072D6E5347FEF), off18F0D344CD5C47B3, uint8_t) = 33;
		{
			static const uint16_t romAB8DD82D2B02350E[825] = {
				516, 513, 513, 513, 513, 511, 512, 512, 509, 507, 507, 505, 505, 505, 505, 505, 503, 505, 506, 505, 506, 507, 508, 509, 508, 510, 513, 512, 511, 512, 513, 512, 507
			,	514, 515, 515, 515, 514, 513, 513, 512, 511, 509, 508, 507, 506, 506, 506, 505, 505, 505, 505, 506, 506, 507, 508, 509, 509, 510, 512, 512, 511, 513, 515, 513, 512
			,	515, 516, 517, 516, 515, 515, 514, 513, 512, 510, 509, 509, 508, 506, 506, 506, 506, 506, 506, 506, 507, 507, 508, 510, 510, 510, 512, 513, 513, 514, 515, 515, 513
			,	519, 518, 518, 517, 515, 515, 514, 513, 512, 511, 510, 509, 508, 507, 507, 506, 506, 507, 507, 507, 507, 507, 509, 511, 511, 512, 513, 514, 514, 514, 516, 516, 515
			,	519, 518, 518, 517, 516, 515, 515, 513, 513, 512, 511, 509, 509, 508, 508, 507, 507, 507, 508, 508, 508, 509, 510, 511, 512, 513, 515, 515, 515, 515, 516, 517, 517
			,	519, 518, 517, 518, 518, 517, 516, 515, 514, 513, 512, 511, 510, 509, 509, 508, 508, 508, 508, 509, 509, 510, 511, 512, 513, 514, 515, 516, 517, 517, 517, 518, 519
			,	521, 521, 520, 519, 519, 518, 518, 517, 515, 514, 513, 512, 511, 510, 509, 508, 509, 509, 509, 510, 510, 511, 512, 513, 514, 515, 515, 517, 518, 519, 519, 519, 520
			,	522, 522, 522, 521, 520, 520, 519, 517, 516, 515, 514, 512, 512, 511, 510, 509, 510, 510, 510, 510, 511, 513, 514, 515, 515, 516, 516, 518, 519, 520, 520, 519, 520
			,	522, 522, 522, 521, 520, 520, 519, 518, 517, 516, 514, 513, 512, 511, 511, 510, 510, 510, 510, 511, 512, 513, 514, 515, 516, 517, 517, 518, 519, 520, 520, 520, 519
			,	522, 522, 522, 521, 520, 520, 520, 518, 518, 517, 515, 514, 513, 512, 511, 511, 511, 511, 511, 512, 512, 513, 514, 515, 516, 518, 518, 518, 519, 520, 520, 519, 519
			,	522, 522, 523, 523, 521, 521, 521, 520, 518, 518, 516, 515, 514, 513, 512, 512, 511, 512, 512, 513, 513, 514, 515, 516, 516, 517, 519, 519, 519, 521, 521, 519, 519
			,	522, 522, 524, 524, 522, 522, 522, 520, 519, 517, 516, 515, 514, 514, 513, 512, 512, 512, 512, 513, 514, 515, 515, 516, 517, 517, 519, 519, 520, 521, 521, 521, 520
			,	521, 523, 524, 523, 523, 522, 521, 520, 520, 518, 516, 516, 514, 513, 513, 512, 512, 512, 512, 513, 514, 515, 515, 517, 518, 518, 519, 519, 519, 520, 521, 521, 522
			,	521, 523, 524, 523, 523, 522, 521, 521, 520, 518, 517, 516, 515, 514, 513, 512, 512, 512, 512, 513, 514, 515, 515, 517, 518, 518, 519, 520, 520, 522, 522, 521, 522
			,	521, 523, 524, 524, 522, 522, 522, 521, 519, 518, 516, 515, 515, 514, 513, 512, 512, 512, 512, 513, 513, 514, 515, 516, 517, 518, 519, 521, 522, 522, 522, 521, 520
			,	521, 523, 524, 523, 522, 522, 522, 520, 518, 517, 516, 515, 514, 513, 512, 512, 512, 512, 513, 513, 514, 514, 515, 516, 517, 518, 519, 520, 521, 522, 522, 521, 520
			,	522, 523, 523, 523, 523, 522, 521, 519, 518, 518, 517, 515, 513, 512, 512, 512, 512, 511, 512, 513, 514, 514, 515, 516, 517, 517, 518, 519, 520, 521, 521, 522, 523
			,	523, 522, 521, 522, 522, 521, 520, 519, 518, 517, 516, 514, 513, 513, 512, 511, 511, 511, 511, 513, 513, 513, 515, 515, 516, 517, 518, 518, 519, 519, 520, 521, 522
			,	523, 523, 522, 522, 522, 520, 519, 519, 518, 516, 515, 513, 513, 512, 511, 510, 510, 510, 511, 512, 513, 513, 514, 515, 516, 516, 517, 518, 519, 519, 519, 520, 520
			,	524, 523, 522, 521, 520, 520, 519, 519, 517, 515, 514, 513, 513, 511, 510, 510, 510, 510, 510, 511, 511, 512, 513, 514, 515, 515, 516, 518, 519, 518, 519, 519, 518
			,	523, 521, 520, 520, 519, 519, 518, 517, 516, 514, 513, 512, 512, 511, 510, 509, 509, 509, 510, 510, 511, 512, 512, 513, 514, 514, 515, 517, 517, 516, 517, 518, 517
			,	521, 521, 520, 519, 519, 518, 517, 516, 515, 514, 513, 512, 511, 511, 510, 509, 509, 509, 509, 509, 510, 511, 511, 512, 514, 514, 515, 516, 516, 517, 517, 517, 518
			,	519, 520, 520, 519, 518, 517, 517, 516, 514, 513, 512, 511, 511, 510, 509, 509, 508, 508, 508, 509, 509, 510, 511, 512, 514, 514, 514, 515, 516, 516, 516, 517, 519
			,	517, 519, 519, 518, 517, 516, 516, 515, 514, 513, 512, 511, 510, 509, 509, 508, 507, 507, 508, 509, 509, 509, 511, 512, 513, 513, 513, 514, 515, 515, 514, 515, 518
			,	520, 517, 516, 517, 517, 517, 514, 512, 513, 512, 510, 509, 508, 509, 507, 506, 507, 507, 508, 509, 509, 508, 509, 510, 512, 512, 512, 514, 513, 513, 514, 514, 512
			};
			static const uint16_t rom60D9FC9B7955CEB5[825] = {
				509, 511, 511, 511, 511, 513, 512, 512, 515, 517, 518, 519, 519, 519, 519, 520, 521, 520, 518, 519, 518, 517, 516, 515, 516, 514, 511, 512, 513, 512, 511, 512, 518
			,	510, 509, 509, 510, 510, 511, 511, 512, 513, 515, 516, 517, 518, 518, 518, 519, 520, 519, 519, 518, 518, 517, 516, 515, 515, 514, 512, 512, 513, 511, 509, 511, 512
			,	509, 508, 507, 508, 509, 509, 510, 511, 512, 514, 515, 515, 516, 518, 518, 518, 518, 518, 519, 518, 517, 517, 516, 514, 514, 514, 512, 511, 511, 510, 509, 509, 511
			,	505, 506, 506, 507, 509, 509, 510, 511, 512, 513, 514, 515, 516, 517, 517, 518, 518, 517, 517, 517, 517, 517, 515, 513, 513, 512, 511, 510, 510, 510, 508, 508, 509
			,	505, 506, 506, 507, 508, 509, 509, 511, 511, 512, 513, 515, 515, 516, 516, 517, 517, 517, 516, 516, 516, 515, 514, 513, 512, 511, 509, 509, 510, 509, 508, 507, 507
			,	506, 506, 507, 506, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 516, 516, 516, 516, 515, 515, 514, 513, 512, 511, 510, 509, 508, 507, 507, 507, 506, 506
			,	503, 504, 504, 505, 505, 506, 506, 507, 509, 510, 511, 512, 513, 514, 515, 516, 515, 515, 515, 514, 514, 513, 512, 511, 510, 509, 509, 507, 506, 505, 505, 505, 504
			,	502, 502, 502, 503, 504, 505, 505, 507, 508, 509, 510, 512, 512, 513, 514, 515, 514, 514, 514, 514, 513, 511, 510, 509, 509, 509, 508, 507, 505, 504, 505, 505, 505
			,	503, 502, 502, 503, 504, 504, 505, 507, 507, 508, 510, 511, 512, 513, 513, 514, 514, 514, 514, 513, 512, 511, 510, 509, 508, 507, 507, 506, 505, 504, 504, 505, 505
			,	503, 502, 502, 503, 504, 504, 504, 506, 507, 507, 509, 510, 511, 512, 513, 513, 513, 513, 513, 512, 512, 511, 510, 509, 508, 507, 506, 506, 505, 504, 504, 505, 505
			,	503, 502, 501, 502, 503, 503, 503, 505, 506, 507, 508, 509, 510, 511, 512, 512, 513, 512, 512, 511, 511, 510, 509, 508, 508, 507, 505, 505, 505, 503, 504, 505, 506
			,	503, 502, 501, 501, 502, 503, 503, 504, 505, 507, 508, 509, 510, 510, 511, 512, 512, 512, 512, 511, 510, 510, 509, 508, 507, 507, 505, 505, 504, 503, 503, 504, 504
			,	503, 502, 501, 501, 502, 503, 504, 504, 505, 507, 508, 509, 510, 511, 511, 512, 512, 512, 512, 511, 510, 509, 509, 507, 506, 506, 505, 505, 505, 504, 503, 503, 502
			,	503, 502, 501, 501, 502, 502, 503, 503, 504, 506, 508, 508, 509, 510, 511, 512, 512, 512, 512, 511, 510, 509, 509, 507, 506, 506, 505, 504, 504, 503, 502, 503, 502
			,	503, 502, 501, 501, 502, 502, 502, 503, 505, 506, 508, 509, 509, 510, 511, 512, 512, 512, 512, 511, 511, 510, 509, 508, 507, 506, 505, 503, 503, 502, 502, 503, 504
			,	503, 501, 500, 501, 502, 502, 503, 504, 506, 507, 508, 509, 510, 511, 512, 512, 512, 512, 511, 511, 510, 510, 509, 508, 507, 506, 506, 504, 503, 502, 503, 503, 504
			,	502, 502, 501, 502, 502, 502, 504, 505, 506, 507, 508, 509, 511, 512, 512, 512, 512, 513, 512, 511, 510, 510, 509, 508, 507, 507, 506, 505, 504, 503, 503, 502, 502
			,	501, 502, 503, 502, 502, 503, 504, 505, 506, 507, 508, 510, 511, 511, 512, 513, 513, 513, 513, 511, 511, 511, 509, 509, 508, 507, 506, 506, 505, 505, 504, 503, 503
			,	502, 501, 502, 503, 503, 504, 505, 505, 507, 508, 509, 511, 511, 512, 513, 514, 514, 514, 513, 512, 511, 511, 510, 509, 508, 508, 507, 506, 505, 505, 505, 505, 504
			,	501, 502, 502, 504, 504, 505, 505, 506, 507, 509, 510, 511, 511, 513, 514, 515, 515, 515, 514, 513, 513, 512, 511, 510, 509, 509, 508, 506, 505, 506, 506, 505, 506
			,	501, 503, 504, 505, 505, 505, 506, 507, 508, 510, 511, 512, 512, 513, 514, 515, 515, 515, 514, 514, 513, 513, 512, 511, 510, 510, 509, 507, 507, 508, 507, 506, 507
			,	503, 504, 504, 505, 505, 506, 507, 508, 509, 510, 511, 512, 513, 513, 514, 515, 515, 515, 515, 515, 514, 513, 513, 512, 510, 510, 509, 508, 508, 507, 507, 507, 506
			,	505, 504, 504, 505, 506, 507, 508, 508, 510, 511, 512, 513, 513, 514, 515, 515, 516, 516, 516, 515, 515, 514, 513, 512, 510, 510, 510, 509, 508, 508, 508, 507, 505
			,	507, 505, 505, 506, 507, 508, 508, 509, 510, 511, 512, 513, 514, 515, 515, 516, 517, 517, 516, 515, 515, 515, 513, 512, 511, 511, 511, 510, 509, 509, 510, 509, 506
			,	504, 507, 508, 507, 507, 507, 510, 512, 511, 512, 514, 515, 516, 515, 517, 518, 517, 517, 516, 515, 515, 516, 515, 514, 512, 512, 512, 510, 511, 511, 510, 510, 512
			};
			memcpy(DataPointer(&(varB5F072D6E5347FEF), off8234BDE9D4E9BE92, uint16_t), romAB8DD82D2B02350E, sizeof(uint16_t) * 825);
			memcpy(DataPointer(&(varB5F072D6E5347FEF), offF071B9D687F900A9, uint16_t), rom60D9FC9B7955CEB5, sizeof(uint16_t) * 825);
		};
		ProcNS(zoom_OCT_mkBuf)(
			&(varB5F072D6E5347FEF)
		,	var0F7A0536073F7CD5
		,	sizeof(var0F7A0536073F7CD5)/sizeof(var0F7A0536073F7CD5[0])
		,	var4466E9B746DAD95C
		,	sizeof(var4466E9B746DAD95C)/sizeof(var4466E9B746DAD95C[0])
		);
		ProcNS(zoom_OCT)(
			*DataPointer(io_pstParams, off42DC478ECF8A6E60, uint16_t)
		,	*DataPointer(io_pstParams, off3E263E9FF3CE8FA3, uint16_t)
		,	*DataPointer(io_pstParams, off25C2C9BDE3113654, uint8_t)
		,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
		,	&(varB5F072D6E5347FEF)
		,	var0F7A0536073F7CD5
		,	sizeof(var0F7A0536073F7CD5)/sizeof(var0F7A0536073F7CD5[0])
		,	var4466E9B746DAD95C
		,	sizeof(var4466E9B746DAD95C)/sizeof(var4466E9B746DAD95C[0])
		,	&(varAB478F3EFC840EEB)
		);
		ProcNS(geom_OCT)(
			DataPointer(io_pstParams, offE073442BDF64D249, void)
		,	*DataPointer(io_pstParams, off25C2C9BDE3113654, uint8_t)
		,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
		,	&(varAB478F3EFC840EEB)
		,	DataPointer(o_pstData, off05EA905432D36B48, void)
		);
	};
	{
		static const int16_t romD982B76DB565D59D[8] = {
			 153
		,	 301
		,	  58
		,	 289
		,	-289
		,	 365
		,	-365
		,	   0
		};
		memcpy(DataPointer(o_pstData, off21C9EC031BB96F91, int16_t), romD982B76DB565D59D, sizeof(int16_t) * 8);
	};
	ProcNS(tune_RGB2YCbCr)(
		*DataPointer(io_pstParams, off6DE0B24764418BAE, int8_t)
	,	DataPointer(o_pstData, off21C9EC031BB96F91, int16_t)
	);
	if (*DataPointer(io_pstParams, off5A14AE564D9ACCD7, uint32_t) < 6553600) {
		static const int16_t rom0B557242314CD042[11] = { 2047, -821, 722, -176, 224, -272, 330, -109, -120, 2047, 1 };
		static const int16_t rom198D3C8888634E96[11] = { 1804, -106, -185, -123, -254, -209, -176, -160, -199, -241, 2047 };
		memcpy(DataPointer(o_pstData, off99A070EBDFD8DD92, int16_t), rom0B557242314CD042, sizeof(int16_t) * 11);
		memcpy(DataPointer(o_pstData, offA468FF2C4F639760, int16_t), rom198D3C8888634E96, sizeof(int16_t) * 11);
		*DataPointer(o_pstData, off742D91E430674A95, uint16_t) = 256;
		{
			int16_t var13B0D1C4939A1E14[11];
			int16_t var9B6F21F4D7484843[11];
			static const int16_t rom13B0D1C4939A1E14[11] = { 0, 128, 0, 0, 0, 0, 0, 0, 0, 0, 2047 };
			static const int16_t rom9B6F21F4D7484843[11] = { 0, 624, -500, -1054, -520, -1071, -238, -773, -251, -795, 2047 };
			memcpy(var13B0D1C4939A1E14, rom13B0D1C4939A1E14, sizeof(int16_t) * 11);
			memcpy(var9B6F21F4D7484843, rom9B6F21F4D7484843, sizeof(int16_t) * 11);
			ProcNS(tune_fmt_T)(
				*DataPointer(io_pstParams, off6A60D8E3CECF0711, uint8_t)
			,	*DataPointer(io_pstParams, off3A7BAC33C1EF3F2E, uint16_t)
			,	*DataPointer(io_pstParams, off2CF7B1BAA7C8CF56, uint8_t)
			,	*DataPointer(io_pstParams, off55FCC61C9F086314, uint8_t)
			,	*DataPointer(io_pstParams, off2CE2D5E2871EC624, uint8_t)
			,	*DataPointer(io_pstParams, offBF508F59316EAC03, uint8_t)
			,	var13B0D1C4939A1E14
			,	var9B6F21F4D7484843
			,	DataPointer(o_pstData, off243D68F65EAA72B4, int16_t)
			,	DataPointer(o_pstData, offFD74C6D54265022E, int16_t)
			,	DataPointer(o_pstData, off785533655D3CE6A6, uint8_t)
			);
		};
	} else if (*DataPointer(io_pstParams, off5A14AE564D9ACCD7, uint32_t) >= 6553600) {
		static const int16_t rom416841FFA67003BD[11] = { 0, 128, 0, 0, 0, 0, 0, 0, 0, 0, 2047 };
		static const int16_t rom0B557242314CD042[11] = { 2047, -821, 722, -176, 224, -272, 330, -109, -120, 2047, 1 };
		static const int16_t rom198D3C8888634E96[11] = { 1804, -106, -185, -123, -254, -209, -176, -160, -199, -241, 2047 };
		memcpy(DataPointer(o_pstData, off243D68F65EAA72B4, int16_t), rom416841FFA67003BD, sizeof(int16_t) * 11);
		memcpy(DataPointer(o_pstData, off99A070EBDFD8DD92, int16_t), rom0B557242314CD042, sizeof(int16_t) * 11);
		memcpy(DataPointer(o_pstData, offA468FF2C4F639760, int16_t), rom198D3C8888634E96, sizeof(int16_t) * 11);
		*DataPointer(o_pstData, offFD74C6D54265022E, int16_t) = -512;
		*DataPointer(o_pstData, off785533655D3CE6A6, uint8_t) = 0;
		*DataPointer(o_pstData, off742D91E430674A95, uint16_t) = 256;
	} else {
		memset(DataPointer(o_pstData, off243D68F65EAA72B4, int16_t), 0xff, sizeof(int16_t) * 11);
		memset(DataPointer(o_pstData, off99A070EBDFD8DD92, int16_t), 0xff, sizeof(int16_t) * 11);
		memset(DataPointer(o_pstData, offA468FF2C4F639760, int16_t), 0xff, sizeof(int16_t) * 11);
		memset(DataPointer(o_pstData, offFD74C6D54265022E, int16_t), 0xff, sizeof(int16_t));
		memset(DataPointer(o_pstData, off785533655D3CE6A6, uint8_t), 0xff, sizeof(uint8_t));
		memset(DataPointer(o_pstData, off742D91E430674A95, uint16_t), 0xff, sizeof(uint16_t));
	};
	ProcNS(insert_WB_noGain)(
		*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t)
	,	*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t)
	,	DataPointer(o_pstData, off0B15E975E2EAA6F8, uint16_t)
	,	DataPointer(o_pstData, off691A04971BFBEAE7, uint16_t)
	,	DataPointer(o_pstData, off23B734014CB0875E, uint16_t)
	);
	*DataPointer(o_pstData, off3FE03CE6163B82D4, uint16_t) = 959;
	{
		uint8_t varAB478F3EFC840EEB[sizD7457E60CF392F9F];
		uint8_t varA1BFFF7227F07355[sizCCABFA7FDE3AC8A8];
		uint8_t var90665A6460BBD864[sizCCABFA7FDE3AC8A8];
		uint16_t var0F7A0536073F7CD5[33];
		uint16_t var4466E9B746DAD95C[33];
		int16_t pos0 = 0, idx0 = 0;
		int32_t scale0 = 0;
		struct {
			uint8_t var697658B4667B26D1;
			uint8_t var49E679770C7EB827;
		} stInterp0[2];
		if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 329) {
			pos0 = 0;
			scale0 = 0;
		} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 404) {
			pos0 = 0;
			scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 329) << 8) / (404 - 329);
		} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 578) {
			pos0 = 1;
			scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 404) << 8) / (578 - 404);
		} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 615) {
			pos0 = 2;
			scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 578) << 8) / (615 - 578);
		} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 740) {
			pos0 = 3;
			scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 615) << 8) / (740 - 615);
		} else {
			pos0 = 3;
			scale0 = 256;
		};
		if (idx0 < 2 && pos0 == 0) {
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 1) {
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 2) {
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 3) {
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			pos0++;
			idx0++;
		};
		if (idx0 < 2 && pos0 == 4) {
			stInterp0[idx0].var697658B4667B26D1 = 25;
			stInterp0[idx0].var49E679770C7EB827 = 33;
			pos0++;
			idx0++;
		};
		*DataPointer(&(var90665A6460BBD864), off3067F31313416AD2, uint8_t) = (uint8_t) DataInterpolate(stInterp0[0].var697658B4667B26D1, stInterp0[1].var697658B4667B26D1, scale0);
		*DataPointer(&(var90665A6460BBD864), off7DF5F184FBA2F506, uint8_t) = (uint8_t) DataInterpolate(stInterp0[0].var49E679770C7EB827, stInterp0[1].var49E679770C7EB827, scale0);
		{
			int16_t k0 = 0, pos0 = 0, idx0 = 0;
			int32_t scale0 = 0;
			struct {
				uint16_t var1D402F229DD24906[825];
				uint16_t var616201DDA585121F[825];
				uint16_t varBCE2AD0636E77498[825];
			} stInterp0[2];
			if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 329) {
				pos0 = 0;
				scale0 = 0;
			} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 404) {
				pos0 = 0;
				scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 329) << 8) / (404 - 329);
			} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 578) {
				pos0 = 1;
				scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 404) << 8) / (578 - 404);
			} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 615) {
				pos0 = 2;
				scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 578) << 8) / (615 - 578);
			} else if (*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) < 740) {
				pos0 = 3;
				scale0 = ((*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t) - 615) << 8) / (740 - 615);
			} else {
				pos0 = 3;
				scale0 = 256;
			};
			if (idx0 < 2 && pos0 == 0) {
				static const uint16_t rom1D402F229DD24906[825] = {
					1431, 1310, 1228, 1163, 1081, 1015,  965, 925, 895, 856, 823, 799, 779, 770, 760, 752, 750, 752, 758, 762, 773, 791, 810, 838, 872, 905, 944,  995, 1052, 1117, 1190, 1283, 1412
				,	1356, 1264, 1186, 1110, 1039,  977,  924, 881, 845, 813, 786, 764, 744, 731, 722, 716, 712, 713, 719, 725, 736, 752, 772, 797, 829, 863, 901,  949, 1003, 1066, 1139, 1215, 1304
				,	1316, 1220, 1137, 1062,  997,  937,  886, 844, 805, 774, 749, 727, 708, 694, 684, 679, 675, 676, 681, 689, 703, 719, 737, 760, 787, 821, 860,  905,  956, 1014, 1087, 1162, 1238
				,	1276, 1173, 1083, 1012,  951,  894,  847, 805, 769, 739, 713, 690, 672, 658, 649, 643, 640, 640, 644, 653, 668, 684, 702, 724, 748, 781, 823,  866,  912,  966, 1036, 1116, 1199
				,	1223, 1126, 1038,  968,  911,  857,  812, 772, 739, 708, 683, 661, 643, 629, 620, 614, 610, 610, 614, 623, 637, 652, 670, 694, 720, 749, 788,  833,  878,  929,  994, 1072, 1159
				,	1181, 1087, 1007,  937,  880,  829,  785, 747, 712, 681, 658, 637, 617, 602, 593, 586, 584, 585, 589, 598, 612, 628, 646, 669, 695, 724, 759,  802,  851,  904,  965, 1037, 1121
				,	1136, 1051,  978,  909,  850,  801,  760, 723, 687, 657, 633, 610, 592, 579, 569, 562, 560, 563, 566, 573, 586, 604, 623, 645, 672, 702, 734,  773,  821,  875,  936, 1005, 1091
				,	1094, 1019,  951,  884,  825,  777,  736, 701, 667, 637, 612, 589, 573, 562, 553, 546, 543, 545, 548, 555, 567, 584, 603, 624, 652, 683, 717,  754,  799,  850,  910,  981, 1062
				,	1071,  998,  929,  866,  809,  761,  720, 683, 650, 622, 596, 574, 559, 548, 539, 534, 532, 534, 538, 544, 555, 570, 589, 610, 636, 668, 704,  741,  784,  834,  892,  958, 1033
				,	1061,  982,  912,  851,  795,  748,  708, 669, 637, 609, 582, 562, 547, 536, 528, 525, 523, 525, 528, 532, 543, 557, 575, 599, 624, 654, 688,  725,  766,  816,  873,  936, 1007
				,	1046,  971,  903,  840,  784,  738,  698, 660, 629, 600, 574, 555, 540, 529, 523, 520, 517, 518, 521, 526, 536, 549, 566, 589, 615, 645, 678,  715,  756,  807,  865,  927,  999
				,	1039,  965,  899,  838,  780,  735,  695, 657, 626, 595, 570, 551, 537, 527, 520, 517, 514, 514, 518, 524, 533, 546, 564, 585, 611, 641, 675,  710,  752,  806,  864,  925,  996
				,	1035,  960,  896,  834,  777,  732,  694, 657, 625, 595, 568, 550, 536, 525, 517, 514, 512, 512, 516, 521, 530, 544, 562, 584, 610, 640, 672,  706,  748,  801,  861,  922,  992
				,	1037,  964,  899,  836,  780,  735,  697, 660, 627, 598, 572, 552, 538, 526, 518, 515, 513, 513, 518, 524, 533, 545, 563, 587, 615, 644, 675,  710,  751,  801,  861,  924,  991
				,	1047,  974,  911,  850,  794,  746,  706, 668, 634, 604, 579, 559, 544, 532, 524, 519, 517, 518, 523, 530, 539, 551, 570, 594, 620, 651, 681,  717,  761,  812,  869,  931,  998
				,	1065,  984,  920,  864,  806,  756,  714, 676, 643, 614, 589, 567, 551, 539, 530, 525, 523, 525, 529, 535, 545, 559, 579, 601, 626, 656, 686,  724,  771,  824,  879,  943, 1020
				,	1087, 1002,  933,  876,  821,  770,  726, 689, 657, 627, 601, 579, 562, 549, 539, 534, 533, 535, 539, 547, 557, 572, 592, 612, 638, 667, 698,  739,  785,  839,  896,  965, 1050
				,	1116, 1029,  957,  896,  841,  789,  742, 707, 676, 643, 617, 596, 579, 566, 556, 550, 548, 550, 555, 562, 575, 591, 609, 631, 656, 685, 717,  758,  806,  861,  920,  991, 1078
				,	1147, 1054,  982,  922,  864,  811,  761, 723, 694, 664, 637, 616, 598, 586, 577, 570, 566, 568, 573, 581, 594, 609, 628, 651, 676, 702, 735,  776,  827,  883,  945, 1017, 1097
				,	1177, 1090, 1015,  953,  895,  840,  790, 747, 714, 688, 663, 640, 622, 610, 600, 594, 591, 592, 597, 606, 617, 633, 652, 674, 699, 728, 763,  807,  858,  912,  978, 1052, 1132
				,	1230, 1139, 1059,  990,  931,  876,  826, 781, 744, 717, 693, 671, 652, 638, 629, 623, 620, 621, 628, 636, 647, 662, 681, 702, 729, 762, 800,  845,  894,  952, 1023, 1096, 1176
				,	1281, 1190, 1110, 1034,  968,  912,  864, 818, 777, 746, 720, 699, 681, 668, 660, 654, 649, 650, 657, 665, 675, 691, 710, 732, 761, 796, 835,  877,  928,  991, 1064, 1140, 1223
				,	1328, 1247, 1169, 1091, 1017,  955,  906, 863, 821, 782, 754, 731, 713, 702, 693, 687, 683, 683, 688, 697, 708, 724, 745, 770, 800, 834, 874,  919,  976, 1043, 1110, 1193, 1299
				,	1412, 1317, 1230, 1150, 1076, 1010,  956, 912, 871, 832, 800, 773, 753, 740, 729, 722, 722, 721, 726, 736, 748, 767, 789, 816, 846, 879, 922,  974, 1034, 1100, 1170, 1264, 1398
				,	1537, 1402, 1296, 1208, 1136, 1068, 1011, 964, 920, 888, 853, 820, 803, 783, 769, 762, 760, 761, 770, 781, 793, 815, 832, 860, 893, 928, 975, 1030, 1089, 1153, 1236, 1351, 1511
				};
				static const uint16_t rom616201DDA585121F[825] = {
					1603, 1475, 1377, 1299, 1220, 1141, 1080, 1035,  994, 952, 917, 889, 866, 850, 838, 827, 822, 824, 832, 839, 852, 876, 902, 933, 966, 1004, 1051, 1108, 1171, 1235, 1317, 1426, 1560
				,	1520, 1412, 1322, 1241, 1163, 1091, 1030,  982,  940, 899, 863, 835, 814, 797, 785, 777, 772, 773, 780, 789, 803, 824, 849, 878, 913,  952,  998, 1052, 1113, 1182, 1263, 1356, 1459
				,	1472, 1360, 1266, 1185, 1108, 1041,  984,  934,  889, 849, 815, 787, 766, 748, 736, 729, 725, 725, 731, 742, 757, 778, 800, 829, 863,  904,  950, 1001, 1060, 1128, 1208, 1294, 1392
				,	1430, 1310, 1211, 1128, 1054,  990,  936,  887,  841, 802, 770, 742, 721, 703, 690, 683, 679, 679, 686, 697, 713, 733, 755, 782, 817,  857,  904,  955, 1010, 1077, 1153, 1238, 1338
				,	1373, 1258, 1163, 1081, 1009,  948,  894,  845,  800, 763, 732, 705, 683, 664, 652, 645, 640, 641, 648, 659, 674, 694, 716, 744, 778,  815,  863,  915,  970, 1035, 1108, 1192, 1288
				,	1329, 1215, 1122, 1042,  974,  914,  860,  811,  767, 730, 700, 674, 650, 631, 619, 613, 609, 610, 616, 627, 642, 661, 685, 713, 746,  782,  827,  880,  937, 1000, 1073, 1155, 1247
				,	1286, 1177, 1085, 1007,  941,  882,  827,  778,  736, 700, 669, 642, 620, 602, 590, 582, 579, 580, 586, 597, 613, 632, 655, 685, 718,  754,  798,  848,  904,  968, 1039, 1118, 1211
				,	1243, 1143, 1055,  981,  915,  854,  799,  752,  711, 675, 643, 617, 595, 579, 568, 559, 555, 557, 563, 573, 589, 608, 631, 660, 693,  731,  775,  824,  878,  940, 1009, 1089, 1182
				,	1212, 1117, 1032,  960,  894,  832,  779,  733,  691, 655, 623, 597, 576, 561, 550, 543, 539, 541, 546, 555, 570, 589, 612, 640, 673,  712,  756,  805,  860,  920,  988, 1067, 1159
				,	1196, 1096, 1013,  941,  875,  815,  762,  715,  673, 638, 606, 580, 560, 545, 535, 529, 527, 528, 533, 541, 555, 573, 596, 625, 657,  695,  739,  787,  841,  902,  970, 1047, 1135
				,	1182, 1084, 1002,  929,  862,  803,  751,  703,  663, 627, 596, 571, 550, 535, 526, 521, 518, 519, 524, 533, 546, 563, 585, 614, 647,  684,  727,  775,  828,  890,  959, 1035, 1123
				,	1172, 1078,  998,  926,  858,  799,  746,  698,  658, 622, 591, 566, 546, 532, 522, 517, 514, 514, 519, 528, 541, 559, 581, 609, 642,  679,  722,  769,  823,  887,  956, 1030, 1118
				,	1167, 1074,  995,  923,  855,  796,  744,  697,  656, 620, 589, 564, 544, 530, 519, 514, 512, 513, 517, 526, 539, 556, 579, 607, 640,  678,  719,  766,  821,  884,  954, 1028, 1111
				,	1168, 1077,  999,  927,  858,  799,  747,  700,  658, 622, 591, 566, 547, 531, 521, 515, 513, 514, 520, 529, 541, 559, 582, 611, 644,  681,  722,  769,  823,  886,  957, 1032, 1116
				,	1178, 1088, 1009,  938,  870,  809,  756,  708,  667, 631, 600, 575, 555, 539, 529, 522, 519, 520, 527, 536, 549, 567, 590, 619, 652,  689,  730,  777,  832,  897,  966, 1042, 1130
				,	1192, 1100, 1021,  951,  882,  820,  766,  719,  678, 643, 612, 586, 565, 549, 539, 531, 528, 529, 536, 545, 558, 577, 601, 628, 660,  697,  739,  787,  843,  908,  977, 1054, 1147
				,	1211, 1118, 1038,  967,  900,  837,  782,  735,  695, 659, 628, 601, 580, 564, 552, 546, 543, 544, 550, 560, 573, 591, 616, 643, 675,  712,  755,  805,  861,  924,  993, 1074, 1167
				,	1244, 1147, 1063,  990,  923,  861,  805,  757,  716, 679, 648, 622, 601, 585, 573, 566, 563, 565, 570, 579, 594, 614, 637, 664, 697,  734,  776,  827,  886,  950, 1020, 1100, 1192
				,	1279, 1177, 1091, 1018,  950,  887,  829,  780,  739, 704, 673, 646, 625, 609, 597, 590, 586, 587, 593, 602, 618, 638, 661, 688, 721,  757,  800,  851,  910,  976, 1049, 1130, 1221
				,	1317, 1214, 1128, 1054,  985,  922,  863,  812,  770, 735, 704, 677, 656, 639, 628, 620, 616, 617, 623, 634, 648, 667, 691, 719, 752,  790,  834,  886,  944, 1009, 1085, 1171, 1266
				,	1373, 1265, 1177, 1099, 1027,  964,  904,  852,  809, 772, 741, 713, 692, 676, 665, 657, 654, 654, 661, 671, 685, 704, 728, 756, 789,  829,  875,  928,  986, 1053, 1131, 1218, 1318
				,	1420, 1318, 1229, 1144, 1068, 1004,  947,  894,  848, 810, 778, 751, 730, 714, 703, 696, 692, 693, 699, 709, 723, 742, 766, 794, 828,  869,  916,  967, 1027, 1097, 1178, 1265, 1358
				,	1462, 1377, 1291, 1201, 1120, 1053,  996,  944,  898, 857, 823, 795, 773, 758, 746, 738, 735, 736, 741, 752, 767, 786, 811, 839, 874,  915,  962, 1016, 1080, 1152, 1233, 1322, 1416
				,	1553, 1449, 1358, 1268, 1185, 1115, 1054, 1001,  955, 915, 878, 848, 825, 809, 796, 788, 786, 786, 792, 803, 819, 839, 864, 893, 929,  970, 1017, 1075, 1141, 1215, 1296, 1394, 1522
				,	1707, 1534, 1424, 1340, 1257, 1181, 1116, 1061, 1014, 976, 937, 906, 884, 863, 849, 843, 840, 839, 847, 861, 876, 897, 922, 952, 990, 1030, 1076, 1139, 1208, 1281, 1360, 1477, 1664
				};
				static const uint16_t romBCE2AD0636E77498[825] = {
					1669, 1520, 1410, 1324, 1237, 1163, 1097, 1041, 1000,  951, 916, 891, 865, 846, 829, 818, 812, 815, 827, 835, 847, 869, 903, 937,  974, 1022, 1079, 1138, 1198, 1276, 1370, 1490, 1648
				,	1575, 1454, 1355, 1271, 1191, 1112, 1043,  986,  942,  903, 866, 839, 814, 794, 781, 772, 769, 771, 777, 787, 803, 825, 852, 883,  920,  969, 1025, 1088, 1154, 1224, 1311, 1414, 1533
				,	1528, 1396, 1297, 1221, 1140, 1061,  995,  939,  892,  854, 821, 792, 767, 746, 734, 728, 726, 726, 731, 744, 762, 782, 806, 835,  872,  921,  974, 1037, 1103, 1172, 1258, 1353, 1460
				,	1483, 1339, 1238, 1161, 1079, 1004,  945,  895,  848,  808, 776, 747, 723, 703, 690, 684, 681, 681, 687, 700, 717, 738, 762, 792,  829,  873,  924,  982, 1047, 1120, 1198, 1290, 1407
				,	1408, 1283, 1185, 1101, 1025,  959,  902,  854,  807,  769, 738, 709, 684, 667, 654, 646, 641, 642, 649, 662, 679, 699, 723, 755,  792,  833,  880,  935, 1002, 1075, 1146, 1234, 1350
				,	1349, 1235, 1142, 1059,  986,  924,  870,  820,  775,  737, 705, 677, 652, 632, 620, 613, 608, 609, 617, 629, 647, 668, 691, 724,  763,  801,  844,  898,  965, 1040, 1114, 1195, 1289
				,	1301, 1198, 1106, 1023,  950,  889,  835,  788,  745,  706, 674, 646, 621, 602, 589, 581, 577, 580, 586, 597, 617, 639, 662, 694,  733,  773,  815,  863,  924,  999, 1079, 1159, 1247
				,	1250, 1159, 1070,  989,  920,  859,  807,  759,  717,  680, 647, 619, 595, 579, 567, 558, 554, 556, 562, 572, 590, 612, 637, 668,  706,  749,  792,  839,  894,  965, 1047, 1131, 1225
				,	1215, 1126, 1041,  963,  896,  838,  787,  737,  696,  660, 626, 597, 576, 561, 550, 542, 539, 540, 545, 554, 570, 592, 618, 649,  685,  727,  773,  822,  876,  943, 1023, 1108, 1201
				,	1210, 1107, 1017,  943,  877,  821,  771,  720,  678,  642, 607, 579, 560, 545, 534, 528, 525, 526, 531, 541, 556, 575, 601, 633,  667,  709,  755,  804,  857,  919,  998, 1083, 1171
				,	1191, 1093, 1005,  929,  863,  807,  757,  709,  669,  630, 597, 570, 549, 535, 525, 520, 517, 517, 523, 532, 546, 565, 590, 622,  657,  697,  742,  791,  845,  909,  986, 1069, 1158
				,	1166, 1085, 1003,  923,  857,  801,  751,  704,  664,  626, 593, 567, 545, 531, 521, 517, 514, 514, 519, 527, 542, 561, 585, 617,  651,  690,  734,  781,  838,  905,  982, 1067, 1155
				,	1171, 1086, 1004,  923,  854,  800,  751,  702,  660,  623, 590, 564, 544, 529, 518, 514, 511, 512, 517, 525, 539, 557, 582, 616,  650,  688,  732,  779,  835,  900,  978, 1061, 1141
				,	1178, 1091, 1010,  930,  858,  803,  755,  706,  664,  626, 592, 566, 547, 531, 519, 514, 511, 512, 519, 529, 542, 559, 586, 619,  655,  692,  735,  783,  837,  902,  984, 1066, 1144
				,	1187, 1100, 1023,  944,  872,  814,  764,  715,  674,  636, 603, 576, 555, 539, 527, 520, 517, 518, 524, 535, 550, 569, 597, 628,  664,  704,  745,  791,  848,  917,  998, 1077, 1154
				,	1204, 1113, 1035,  959,  886,  825,  773,  726,  686,  648, 616, 588, 565, 548, 537, 529, 525, 527, 534, 545, 559, 581, 609, 638,  673,  715,  759,  805,  862,  935, 1013, 1092, 1179
				,	1231, 1136, 1054,  980,  907,  843,  790,  746,  704,  664, 633, 604, 580, 563, 551, 544, 540, 542, 550, 562, 576, 598, 625, 653,  688,  730,  774,  825,  884,  957, 1036, 1117, 1208
				,	1265, 1171, 1086, 1010,  940,  872,  814,  769,  727,  687, 655, 627, 603, 587, 575, 566, 562, 564, 572, 583, 600, 623, 647, 676,  713,  751,  795,  847,  913,  991, 1069, 1149, 1232
				,	1308, 1205, 1120, 1045,  974,  904,  841,  792,  753,  715, 682, 654, 631, 615, 603, 593, 588, 590, 598, 610, 628, 649, 673, 704,  739,  776,  820,  876,  946, 1022, 1097, 1181, 1279
				,	1363, 1248, 1164, 1088, 1014,  943,  876,  826,  786,  750, 718, 688, 664, 649, 635, 625, 620, 622, 631, 644, 659, 680, 708, 738,  773,  812,  858,  920,  988, 1059, 1137, 1226, 1332
				,	1423, 1306, 1218, 1138, 1061,  993,  926,  870,  827,  788, 756, 728, 704, 686, 673, 664, 660, 662, 671, 683, 699, 721, 747, 777,  813,  856,  909,  971, 1034, 1105, 1190, 1285, 1402
				,	1473, 1373, 1278, 1187, 1108, 1043,  982,  919,  866,  826, 794, 767, 744, 725, 713, 704, 701, 705, 713, 725, 740, 761, 785, 816,  854,  898,  958, 1017, 1078, 1153, 1238, 1342, 1466
				,	1545, 1450, 1350, 1249, 1166, 1098, 1039,  978,  919,  875, 840, 809, 786, 770, 758, 751, 748, 749, 755, 767, 783, 805, 832, 865,  904,  953, 1012, 1072, 1137, 1214, 1304, 1406, 1520
				,	1664, 1534, 1423, 1326, 1240, 1163, 1103, 1045,  988,  941, 900, 863, 837, 820, 806, 800, 799, 798, 804, 817, 836, 862, 891, 927,  970, 1023, 1075, 1135, 1211, 1290, 1378, 1490, 1641
				,	1809, 1619, 1496, 1415, 1321, 1234, 1178, 1115, 1059, 1017, 966, 922, 900, 878, 855, 849, 847, 852, 868, 877, 898, 928, 955, 998, 1045, 1097, 1140, 1195, 1288, 1374, 1454, 1596, 1833
				};
				memcpy(stInterp0[idx0].var1D402F229DD24906, rom1D402F229DD24906, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].var616201DDA585121F, rom616201DDA585121F, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].varBCE2AD0636E77498, romBCE2AD0636E77498, sizeof(uint16_t) * 825);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 1) {
				static const uint16_t rom1D402F229DD24906[825] = {
					1467, 1331, 1237, 1168, 1095, 1034,  986, 939, 904, 871, 839, 811, 790, 783, 771, 764, 760, 755, 762, 773, 785, 805, 823, 850, 883, 917,  962, 1011, 1073, 1139, 1199, 1287, 1414
				,	1377, 1277, 1190, 1118, 1051,  989,  939, 896, 859, 826, 796, 775, 756, 742, 732, 725, 722, 720, 727, 737, 747, 766, 787, 811, 841, 876,  917,  962, 1017, 1084, 1155, 1237, 1331
				,	1336, 1232, 1147, 1073, 1004,  947,  899, 857, 821, 788, 760, 739, 720, 704, 695, 689, 684, 684, 690, 699, 712, 730, 751, 773, 800, 834,  874,  917,  967, 1034, 1111, 1186, 1262
				,	1303, 1186, 1099, 1027,  959,  904,  859, 819, 785, 753, 725, 702, 683, 668, 658, 652, 648, 648, 653, 663, 677, 695, 714, 735, 762, 794,  834,  876,  922,  986, 1058, 1130, 1208
				,	1252, 1143, 1055,  986,  922,  866,  823, 785, 751, 720, 695, 672, 652, 638, 628, 621, 618, 618, 623, 633, 646, 663, 681, 704, 732, 764,  800,  843,  891,  947, 1014, 1088, 1168
				,	1213, 1111, 1021,  950,  891,  839,  796, 758, 725, 695, 670, 648, 627, 612, 602, 595, 593, 594, 598, 608, 621, 636, 656, 680, 707, 739,  773,  817,  866,  920,  983, 1055, 1139
				,	1168, 1073,  989,  920,  862,  813,  771, 733, 701, 671, 645, 623, 604, 588, 578, 570, 567, 569, 573, 582, 596, 612, 632, 656, 683, 715,  751,  790,  837,  891,  950, 1021, 1114
				,	1126, 1037,  962,  896,  839,  790,  749, 712, 680, 650, 623, 602, 584, 569, 559, 551, 547, 549, 554, 562, 576, 593, 612, 636, 663, 695,  732,  769,  813,  866,  925,  995, 1084
				,	1103, 1017,  941,  879,  823,  773,  732, 696, 664, 633, 607, 585, 567, 553, 544, 536, 532, 534, 540, 549, 561, 578, 597, 621, 648, 680,  717,  755,  796,  848,  906,  973, 1051
				,	1091, 1001,  925,  864,  811,  762,  718, 681, 649, 618, 593, 571, 553, 539, 530, 525, 522, 524, 528, 536, 549, 564, 584, 608, 635, 665,  701,  739,  781,  830,  888,  956, 1031
				,	1076,  989,  914,  852,  800,  755,  712, 672, 638, 610, 585, 563, 545, 530, 522, 519, 516, 517, 520, 527, 540, 556, 576, 598, 625, 654,  689,  727,  771,  821,  879,  944, 1017
				,	1066,  986,  912,  848,  796,  749,  707, 670, 636, 608, 582, 559, 541, 527, 520, 516, 514, 514, 517, 523, 535, 552, 571, 594, 621, 652,  686,  723,  767,  819,  876,  939, 1011
				,	1069,  983,  909,  848,  796,  749,  706, 668, 636, 607, 580, 557, 539, 526, 518, 515, 512, 512, 515, 521, 533, 549, 568, 593, 621, 652,  686,  723,  766,  815,  874,  939, 1011
				,	1069,  983,  912,  852,  800,  755,  712, 672, 638, 609, 583, 561, 542, 527, 520, 515, 512, 513, 517, 524, 536, 551, 571, 596, 623, 654,  689,  727,  769,  817,  876,  944, 1017
				,	1073,  992,  922,  862,  809,  762,  720, 681, 646, 616, 591, 569, 550, 535, 526, 520, 517, 519, 524, 531, 544, 559, 579, 603, 631, 661,  695,  733,  777,  826,  886,  956, 1031
				,	1088, 1004,  933,  874,  821,  773,  728, 689, 656, 626, 601, 580, 560, 545, 534, 527, 525, 527, 532, 539, 551, 567, 586, 610, 638, 668,  702,  742,  787,  839,  899,  967, 1044
				,	1111, 1024,  950,  891,  839,  788,  744, 704, 670, 638, 612, 592, 572, 557, 547, 540, 537, 539, 546, 553, 564, 581, 601, 623, 650, 680,  715,  756,  803,  857,  917,  986, 1066
				,	1143, 1051,  976,  914,  859,  809,  766, 727, 690, 657, 628, 607, 590, 576, 565, 559, 555, 557, 564, 571, 583, 600, 618, 641, 668, 699,  735,  775,  821,  876,  939, 1011, 1095
				,	1177, 1080, 1004,  941,  883,  832,  787, 746, 710, 678, 650, 627, 609, 596, 586, 580, 576, 577, 583, 591, 603, 618, 637, 661, 689, 720,  756,  796,  843,  899,  962, 1037, 1126
				,	1204, 1118, 1044,  976,  914,  862,  813, 769, 733, 702, 677, 653, 633, 621, 612, 605, 601, 601, 607, 616, 628, 644, 664, 689, 717, 747,  783,  826,  874,  930,  998, 1076, 1164
				,	1242, 1168, 1095, 1021,  953,  899,  848, 803, 766, 733, 707, 683, 664, 652, 644, 637, 632, 632, 637, 646, 658, 675, 696, 720, 749, 781,  817,  862,  912,  970, 1044, 1126, 1213
				,	1309, 1218, 1139, 1066,  995,  936,  883, 839, 800, 767, 740, 715, 696, 683, 674, 668, 664, 664, 668, 677, 689, 706, 727, 751, 781, 813,  850,  896,  950, 1014, 1091, 1172, 1257
				,	1359, 1272, 1195, 1118, 1044,  983,  928, 881, 841, 807, 779, 755, 733, 718, 709, 701, 698, 699, 704, 714, 727, 742, 762, 787, 817, 852,  891,  939,  998, 1069, 1147, 1222, 1303
				,	1427, 1342, 1262, 1181, 1103, 1037,  983, 930, 888, 852, 821, 798, 777, 762, 751, 742, 739, 740, 747, 756, 771, 788, 807, 830, 862, 901,  944,  995, 1058, 1130, 1204, 1293, 1408
				,	1554, 1427, 1325, 1247, 1168, 1095, 1041, 983, 936, 901, 866, 846, 826, 809, 796, 790, 787, 785, 792, 798, 815, 839, 855, 876, 912, 956, 1004, 1055, 1118, 1190, 1262, 1383, 1577
				};
				static const uint16_t rom616201DDA585121F[825] = {
					1603, 1475, 1377, 1299, 1220, 1141, 1080, 1035,  994, 952, 917, 889, 866, 850, 838, 827, 822, 824, 832, 839, 852, 876, 902, 933, 966, 1004, 1051, 1108, 1171, 1235, 1317, 1426, 1560
				,	1520, 1412, 1322, 1241, 1163, 1091, 1030,  982,  940, 899, 863, 835, 814, 797, 785, 777, 772, 773, 780, 789, 803, 824, 849, 878, 913,  952,  998, 1052, 1113, 1182, 1263, 1356, 1459
				,	1472, 1360, 1266, 1185, 1108, 1041,  984,  934,  889, 849, 815, 787, 766, 748, 736, 729, 725, 725, 731, 742, 757, 778, 800, 829, 863,  904,  950, 1001, 1060, 1128, 1208, 1294, 1392
				,	1430, 1310, 1211, 1128, 1054,  990,  936,  887,  841, 802, 770, 742, 721, 703, 690, 683, 679, 679, 686, 697, 713, 733, 755, 782, 817,  857,  904,  955, 1010, 1077, 1153, 1238, 1338
				,	1373, 1258, 1163, 1081, 1009,  948,  894,  845,  800, 763, 732, 705, 683, 664, 652, 645, 640, 641, 648, 659, 674, 694, 716, 744, 778,  815,  863,  915,  970, 1035, 1108, 1192, 1288
				,	1329, 1215, 1122, 1042,  974,  914,  860,  811,  767, 730, 700, 674, 650, 631, 619, 613, 609, 610, 616, 627, 642, 661, 685, 713, 746,  782,  827,  880,  937, 1000, 1073, 1155, 1247
				,	1286, 1177, 1085, 1007,  941,  882,  827,  778,  736, 700, 669, 642, 620, 602, 590, 582, 579, 580, 586, 597, 613, 632, 655, 685, 718,  754,  798,  848,  904,  968, 1039, 1118, 1211
				,	1243, 1143, 1055,  981,  915,  854,  799,  752,  711, 675, 643, 617, 595, 579, 568, 559, 555, 557, 563, 573, 589, 608, 631, 660, 693,  731,  775,  824,  878,  940, 1009, 1089, 1182
				,	1212, 1117, 1032,  960,  894,  832,  779,  733,  691, 655, 623, 597, 576, 561, 550, 543, 539, 541, 546, 555, 570, 589, 612, 640, 673,  712,  756,  805,  860,  920,  988, 1067, 1159
				,	1196, 1096, 1013,  941,  875,  815,  762,  715,  673, 638, 606, 580, 560, 545, 535, 529, 527, 528, 533, 541, 555, 573, 596, 625, 657,  695,  739,  787,  841,  902,  970, 1047, 1135
				,	1182, 1084, 1002,  929,  862,  803,  751,  703,  663, 627, 596, 571, 550, 535, 526, 521, 518, 519, 524, 533, 546, 563, 585, 614, 647,  684,  727,  775,  828,  890,  959, 1035, 1123
				,	1172, 1078,  998,  926,  858,  799,  746,  698,  658, 622, 591, 566, 546, 532, 522, 517, 514, 514, 519, 528, 541, 559, 581, 609, 642,  679,  722,  769,  823,  887,  956, 1030, 1118
				,	1167, 1074,  995,  923,  855,  796,  744,  697,  656, 620, 589, 564, 544, 530, 519, 514, 512, 513, 517, 526, 539, 556, 579, 607, 640,  678,  719,  766,  821,  884,  954, 1028, 1111
				,	1168, 1077,  999,  927,  858,  799,  747,  700,  658, 622, 591, 566, 547, 531, 521, 515, 513, 514, 520, 529, 541, 559, 582, 611, 644,  681,  722,  769,  823,  886,  957, 1032, 1116
				,	1178, 1088, 1009,  938,  870,  809,  756,  708,  667, 631, 600, 575, 555, 539, 529, 522, 519, 520, 527, 536, 549, 567, 590, 619, 652,  689,  730,  777,  832,  897,  966, 1042, 1130
				,	1192, 1100, 1021,  951,  882,  820,  766,  719,  678, 643, 612, 586, 565, 549, 539, 531, 528, 529, 536, 545, 558, 577, 601, 628, 660,  697,  739,  787,  843,  908,  977, 1054, 1147
				,	1211, 1118, 1038,  967,  900,  837,  782,  735,  695, 659, 628, 601, 580, 564, 552, 546, 543, 544, 550, 560, 573, 591, 616, 643, 675,  712,  755,  805,  861,  924,  993, 1074, 1167
				,	1244, 1147, 1063,  990,  923,  861,  805,  757,  716, 679, 648, 622, 601, 585, 573, 566, 563, 565, 570, 579, 594, 614, 637, 664, 697,  734,  776,  827,  886,  950, 1020, 1100, 1192
				,	1279, 1177, 1091, 1018,  950,  887,  829,  780,  739, 704, 673, 646, 625, 609, 597, 590, 586, 587, 593, 602, 618, 638, 661, 688, 721,  757,  800,  851,  910,  976, 1049, 1130, 1221
				,	1317, 1214, 1128, 1054,  985,  922,  863,  812,  770, 735, 704, 677, 656, 639, 628, 620, 616, 617, 623, 634, 648, 667, 691, 719, 752,  790,  834,  886,  944, 1009, 1085, 1171, 1266
				,	1373, 1265, 1177, 1099, 1027,  964,  904,  852,  809, 772, 741, 713, 692, 676, 665, 657, 654, 654, 661, 671, 685, 704, 728, 756, 789,  829,  875,  928,  986, 1053, 1131, 1218, 1318
				,	1420, 1318, 1229, 1144, 1068, 1004,  947,  894,  848, 810, 778, 751, 730, 714, 703, 696, 692, 693, 699, 709, 723, 742, 766, 794, 828,  869,  916,  967, 1027, 1097, 1178, 1265, 1358
				,	1462, 1377, 1291, 1201, 1120, 1053,  996,  944,  898, 857, 823, 795, 773, 758, 746, 738, 735, 736, 741, 752, 767, 786, 811, 839, 874,  915,  962, 1016, 1080, 1152, 1233, 1322, 1416
				,	1553, 1449, 1358, 1268, 1185, 1115, 1054, 1001,  955, 915, 878, 848, 825, 809, 796, 788, 786, 786, 792, 803, 819, 839, 864, 893, 929,  970, 1017, 1075, 1141, 1215, 1296, 1394, 1522
				,	1707, 1534, 1424, 1340, 1257, 1181, 1116, 1061, 1014, 976, 937, 906, 884, 863, 849, 843, 840, 839, 847, 861, 876, 897, 922, 952, 990, 1030, 1076, 1139, 1208, 1281, 1360, 1477, 1664
				};
				static const uint16_t romBCE2AD0636E77498[825] = {
					1709, 1569, 1462, 1380, 1292, 1210, 1138, 1084, 1041,  987, 946, 919, 890, 870, 859, 850, 840, 840, 855, 865, 881, 909, 939,  965,  999, 1047, 1101, 1169, 1240, 1311, 1391, 1520, 1735
				,	1598, 1500, 1402, 1306, 1218, 1145, 1080, 1026,  982,  934, 892, 863, 838, 816, 803, 795, 790, 790, 799, 812, 830, 855, 881,  914,  954,  999, 1047, 1108, 1181, 1254, 1337, 1443, 1576
				,	1527, 1437, 1342, 1245, 1161, 1091, 1029,  973,  926,  883, 844, 812, 788, 767, 752, 744, 740, 742, 750, 763, 781, 805, 830,  863,  906,  949,  996, 1054, 1123, 1197, 1282, 1374, 1468
				,	1494, 1374, 1272, 1185, 1108, 1041,  979,  924,  876,  834, 797, 765, 740, 722, 705, 695, 693, 694, 703, 716, 733, 757, 783,  814,  855,  899,  949, 1005, 1067, 1138, 1223, 1316, 1414
				,	1456, 1321, 1214, 1130, 1057,  993,  934,  881,  836,  792, 755, 725, 700, 682, 666, 656, 652, 653, 662, 675, 691, 714, 742,  774,  812,  857,  906,  962, 1022, 1087, 1169, 1263, 1363
				,	1397, 1272, 1173, 1091, 1016,  952,  895,  844,  799,  758, 722, 690, 665, 645, 629, 620, 617, 619, 627, 639, 656, 679, 707,  739,  777,  820,  868,  924,  984, 1051, 1126, 1214, 1316
				,	1342, 1227, 1134, 1054,  982,  919,  861,  808,  763,  725, 688, 657, 632, 611, 596, 587, 583, 585, 592, 604, 622, 647, 675,  707,  745,  788,  836,  890,  949, 1016, 1091, 1177, 1282
				,	1301, 1193, 1101, 1022,  954,  892,  834,  781,  736,  695, 659, 628, 603, 584, 572, 562, 557, 559, 565, 576, 594, 619, 648,  680,  719,  762,  810,  863,  921,  987, 1060, 1145, 1245
				,	1263, 1161, 1074,  999,  931,  868,  810,  758,  713,  674, 637, 605, 580, 564, 552, 544, 540, 542, 547, 557, 574, 597, 626,  658,  695,  739,  788,  840,  899,  965, 1035, 1115, 1210
				,	1236, 1138, 1051,  976,  909,  846,  790,  737,  693,  654, 618, 586, 563, 548, 537, 529, 527, 529, 533, 543, 558, 579, 607,  640,  678,  722,  770,  822,  881,  944, 1013, 1094, 1185
				,	1218, 1123, 1038,  962,  895,  832,  776,  725,  680,  641, 605, 575, 552, 537, 527, 521, 518, 519, 525, 534, 548, 568, 594,  627,  666,  710,  757,  808,  868,  931, 1002, 1084, 1181
				,	1210, 1115, 1032,  957,  888,  824,  769,  719,  674,  634, 599, 570, 548, 532, 522, 517, 514, 514, 520, 529, 544, 563, 589,  622,  661,  703,  748,  801,  859,  926,  999, 1080, 1185
				,	1210, 1115, 1029,  952,  883,  822,  767,  716,  671,  630, 595, 568, 545, 529, 520, 514, 512, 512, 517, 526, 541, 560, 586,  620,  658,  700,  747,  799,  855,  921,  996, 1077, 1173
				,	1223, 1119, 1032,  954,  885,  826,  772,  720,  674,  634, 600, 571, 548, 533, 522, 515, 513, 514, 519, 529, 543, 562, 588,  622,  661,  703,  750,  803,  859,  924,  999, 1080, 1169
				,	1231, 1126, 1041,  968,  899,  838,  783,  729,  683,  645, 609, 579, 558, 541, 529, 521, 518, 520, 526, 536, 551, 571, 599,  633,  670,  711,  758,  812,  870,  934, 1010, 1094, 1185
				,	1236, 1138, 1054,  982,  914,  850,  793,  740,  697,  658, 621, 591, 568, 551, 538, 531, 528, 529, 536, 546, 561, 582, 611,  645,  680,  723,  770,  824,  883,  949, 1026, 1108, 1201
				,	1263, 1161, 1074,  999,  931,  868,  810,  758,  714,  676, 641, 609, 583, 565, 553, 546, 544, 545, 551, 562, 578, 601, 629,  661,  697,  740,  788,  842,  904,  973, 1047, 1130, 1227
				,	1301, 1193, 1105, 1029,  960,  897,  838,  786,  740,  700, 665, 634, 608, 587, 575, 568, 564, 566, 573, 584, 603, 626, 653,  684,  720,  763,  814,  870,  931,  999, 1074, 1161, 1263
				,	1337, 1231, 1141, 1064,  993,  929,  868,  814,  769,  728, 693, 663, 637, 617, 604, 595, 590, 591, 600, 612, 630, 653, 680,  713,  750,  793,  844,  899,  960, 1029, 1108, 1197, 1296
				,	1391, 1277, 1185, 1105, 1029,  962,  902,  846,  801,  763, 729, 698, 672, 653, 640, 632, 627, 628, 636, 648, 666, 688, 716,  748,  788,  832,  881,  936,  996, 1067, 1153, 1245, 1342
				,	1443, 1332, 1240, 1153, 1070, 1007,  949,  892,  846,  806, 770, 739, 714, 695, 682, 674, 670, 672, 679, 688, 707, 731, 758,  792,  830,  874,  926,  982, 1041, 1115, 1205, 1301, 1408
				,	1500, 1391, 1296, 1205, 1123, 1057,  996,  939,  892,  850, 812, 781, 757, 739, 725, 716, 713, 714, 720, 731, 748, 772, 799,  832,  870,  916,  970, 1026, 1087, 1169, 1258, 1353, 1468
				,	1583, 1462, 1363, 1272, 1185, 1112, 1047,  990,  944,  899, 861, 830, 803, 784, 772, 763, 760, 760, 767, 781, 797, 818, 846,  881,  921,  968, 1019, 1077, 1145, 1227, 1316, 1414, 1527
				,	1668, 1548, 1443, 1347, 1254, 1177, 1112, 1054, 1005,  957, 919, 888, 859, 838, 826, 818, 814, 814, 822, 838, 855, 876, 906,  941,  982, 1029, 1080, 1141, 1218, 1301, 1391, 1500, 1628
				,	1771, 1636, 1520, 1426, 1332, 1245, 1181, 1123, 1067, 1019, 982, 952, 924, 899, 885, 876, 872, 872, 881, 899, 919, 944, 973, 1002, 1041, 1094, 1153, 1214, 1292, 1385, 1474, 1598, 1780
				};
				memcpy(stInterp0[idx0].var1D402F229DD24906, rom1D402F229DD24906, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].var616201DDA585121F, rom616201DDA585121F, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].varBCE2AD0636E77498, romBCE2AD0636E77498, sizeof(uint16_t) * 825);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 2) {
				static const uint16_t rom1D402F229DD24906[825] = {
					1364, 1253, 1169, 1096, 1031,  974, 914, 874, 848, 810, 779, 759, 744, 736, 727, 720, 714, 715, 724, 729, 743, 757, 771, 802, 841, 878, 919,  965, 1021, 1087, 1163, 1249, 1350
				,	1317, 1208, 1127, 1056,  989,  931, 880, 838, 806, 777, 750, 730, 715, 705, 696, 691, 687, 687, 691, 698, 712, 727, 745, 768, 798, 834, 873,  915,  965, 1030, 1106, 1195, 1299
				,	1277, 1165, 1081, 1011,  944,  887, 843, 804, 769, 742, 720, 700, 686, 673, 663, 659, 656, 656, 659, 667, 682, 698, 714, 734, 759, 793, 833,  874,  922,  984, 1057, 1143, 1248
				,	1223, 1118, 1030,  961,  901,  848, 804, 770, 738, 709, 687, 668, 655, 641, 632, 627, 623, 624, 627, 636, 651, 669, 682, 700, 727, 758, 796,  838,  886,  942, 1008, 1092, 1189
				,	1161, 1070,  986,  920,  869,  819, 776, 742, 711, 684, 663, 645, 630, 615, 607, 604, 599, 599, 604, 612, 626, 643, 657, 676, 703, 730, 765,  806,  856,  905,  967, 1047, 1133
				,	1129, 1031,  953,  892,  840,  791, 751, 719, 689, 664, 643, 625, 607, 593, 586, 583, 579, 579, 584, 592, 605, 620, 635, 657, 682, 707, 740,  780,  826,  878,  937, 1011, 1098
				,	1087,  996,  926,  864,  810,  766, 726, 693, 666, 644, 622, 601, 587, 576, 568, 560, 558, 560, 563, 572, 585, 597, 615, 639, 663, 688, 718,  755,  798,  852,  910,  977, 1059
				,	1036,  967,  903,  841,  788,  746, 707, 675, 649, 627, 606, 586, 573, 562, 553, 546, 543, 545, 549, 559, 570, 582, 599, 622, 645, 673, 703,  739,  782,  831,  888,  951, 1022
				,	1012,  946,  884,  824,  771,  729, 696, 665, 637, 613, 591, 575, 561, 549, 541, 535, 533, 535, 540, 547, 558, 571, 587, 607, 631, 661, 691,  725,  767,  815,  869,  930, 1000
				,	1001,  930,  866,  808,  757,  717, 684, 653, 625, 599, 577, 562, 549, 539, 530, 524, 523, 526, 530, 536, 547, 560, 575, 595, 620, 648, 678,  710,  750,  797,  849,  907,  973
				,	 993,  924,  857,  798,  749,  711, 676, 642, 616, 592, 571, 556, 543, 532, 523, 519, 517, 519, 524, 531, 542, 553, 568, 588, 611, 638, 668,  700,  739,  787,  839,  896,  961
				,	 988,  920,  857,  798,  747,  707, 673, 641, 614, 590, 569, 554, 541, 529, 519, 516, 514, 515, 520, 528, 539, 551, 567, 585, 608, 635, 664,  695,  735,  783,  837,  893,  955
				,	 982,  914,  856,  799,  746,  705, 672, 643, 615, 589, 567, 551, 538, 526, 518, 514, 513, 514, 519, 526, 536, 550, 566, 586, 607, 633, 662,  694,  732,  780,  838,  895,  953
				,	 985,  918,  859,  803,  752,  709, 675, 646, 618, 592, 570, 553, 540, 528, 520, 516, 514, 516, 523, 529, 538, 551, 568, 589, 611, 637, 665,  697,  735,  782,  841,  902,  973
				,	1000,  932,  871,  815,  763,  718, 683, 653, 625, 598, 577, 560, 545, 534, 527, 523, 521, 522, 526, 535, 544, 556, 574, 593, 616, 644, 673,  704,  744,  790,  845,  907,  984
				,	1018,  943,  882,  826,  774,  729, 691, 659, 632, 607, 586, 569, 552, 541, 535, 531, 528, 528, 532, 540, 550, 562, 579, 599, 623, 650, 678,  710,  753,  802,  854,  921, 1003
				,	1033,  959,  894,  840,  790,  744, 702, 670, 644, 618, 598, 579, 561, 551, 545, 541, 538, 538, 543, 550, 560, 572, 590, 612, 636, 659, 688,  725,  769,  816,  871,  943, 1029
				,	1063,  984,  917,  862,  815,  763, 719, 687, 659, 634, 613, 593, 577, 565, 557, 553, 551, 553, 557, 564, 575, 590, 607, 628, 649, 672, 705,  746,  791,  840,  898,  966, 1051
				,	1103, 1017,  945,  885,  837,  787, 740, 704, 675, 653, 631, 612, 596, 582, 573, 568, 565, 567, 574, 580, 590, 606, 622, 642, 666, 692, 725,  764,  810,  863,  925,  999, 1088
				,	1140, 1053,  977,  913,  865,  818, 770, 729, 698, 675, 652, 633, 618, 604, 596, 591, 588, 588, 593, 601, 611, 626, 642, 660, 686, 717, 750,  789,  836,  891,  962, 1044, 1129
				,	1198, 1097, 1020,  960,  903,  852, 803, 761, 727, 699, 677, 657, 640, 629, 623, 618, 615, 615, 620, 628, 638, 652, 669, 688, 713, 745, 782,  825,  875,  933, 1003, 1084, 1179
				,	1253, 1153, 1070, 1002,  936,  883, 839, 796, 758, 726, 705, 683, 663, 655, 650, 642, 638, 641, 647, 655, 665, 679, 695, 718, 745, 776, 817,  858,  909,  971, 1045, 1131, 1221
				,	1280, 1206, 1126, 1049,  982,  927, 882, 837, 797, 765, 737, 715, 697, 686, 677, 670, 668, 671, 675, 683, 697, 712, 729, 753, 783, 816, 856,  900,  957, 1024, 1100, 1187, 1276
				,	1343, 1262, 1188, 1113, 1043,  982, 928, 880, 842, 811, 779, 755, 737, 721, 710, 705, 705, 706, 709, 718, 734, 750, 770, 795, 828, 864, 901,  955, 1022, 1086, 1153, 1246, 1365
				,	1473, 1334, 1255, 1184, 1103, 1032, 973, 924, 884, 860, 832, 799, 777, 762, 749, 745, 745, 738, 742, 757, 775, 790, 808, 836, 872, 911, 948, 1019, 1093, 1140, 1204, 1316, 1480
				};
				static const uint16_t rom616201DDA585121F[825] = {
					1603, 1475, 1377, 1299, 1220, 1141, 1080, 1035,  994, 952, 917, 889, 866, 850, 838, 827, 822, 824, 832, 839, 852, 876, 902, 933, 966, 1004, 1051, 1108, 1171, 1235, 1317, 1426, 1560
				,	1520, 1412, 1322, 1241, 1163, 1091, 1030,  982,  940, 899, 863, 835, 814, 797, 785, 777, 772, 773, 780, 789, 803, 824, 849, 878, 913,  952,  998, 1052, 1113, 1182, 1263, 1356, 1459
				,	1472, 1360, 1266, 1185, 1108, 1041,  984,  934,  889, 849, 815, 787, 766, 748, 736, 729, 725, 725, 731, 742, 757, 778, 800, 829, 863,  904,  950, 1001, 1060, 1128, 1208, 1294, 1392
				,	1430, 1310, 1211, 1128, 1054,  990,  936,  887,  841, 802, 770, 742, 721, 703, 690, 683, 679, 679, 686, 697, 713, 733, 755, 782, 817,  857,  904,  955, 1010, 1077, 1153, 1238, 1338
				,	1373, 1258, 1163, 1081, 1009,  948,  894,  845,  800, 763, 732, 705, 683, 664, 652, 645, 640, 641, 648, 659, 674, 694, 716, 744, 778,  815,  863,  915,  970, 1035, 1108, 1192, 1288
				,	1329, 1215, 1122, 1042,  974,  914,  860,  811,  767, 730, 700, 674, 650, 631, 619, 613, 609, 610, 616, 627, 642, 661, 685, 713, 746,  782,  827,  880,  937, 1000, 1073, 1155, 1247
				,	1286, 1177, 1085, 1007,  941,  882,  827,  778,  736, 700, 669, 642, 620, 602, 590, 582, 579, 580, 586, 597, 613, 632, 655, 685, 718,  754,  798,  848,  904,  968, 1039, 1118, 1211
				,	1243, 1143, 1055,  981,  915,  854,  799,  752,  711, 675, 643, 617, 595, 579, 568, 559, 555, 557, 563, 573, 589, 608, 631, 660, 693,  731,  775,  824,  878,  940, 1009, 1089, 1182
				,	1212, 1117, 1032,  960,  894,  832,  779,  733,  691, 655, 623, 597, 576, 561, 550, 543, 539, 541, 546, 555, 570, 589, 612, 640, 673,  712,  756,  805,  860,  920,  988, 1067, 1159
				,	1196, 1096, 1013,  941,  875,  815,  762,  715,  673, 638, 606, 580, 560, 545, 535, 529, 527, 528, 533, 541, 555, 573, 596, 625, 657,  695,  739,  787,  841,  902,  970, 1047, 1135
				,	1182, 1084, 1002,  929,  862,  803,  751,  703,  663, 627, 596, 571, 550, 535, 526, 521, 518, 519, 524, 533, 546, 563, 585, 614, 647,  684,  727,  775,  828,  890,  959, 1035, 1123
				,	1172, 1078,  998,  926,  858,  799,  746,  698,  658, 622, 591, 566, 546, 532, 522, 517, 514, 514, 519, 528, 541, 559, 581, 609, 642,  679,  722,  769,  823,  887,  956, 1030, 1118
				,	1167, 1074,  995,  923,  855,  796,  744,  697,  656, 620, 589, 564, 544, 530, 519, 514, 512, 513, 517, 526, 539, 556, 579, 607, 640,  678,  719,  766,  821,  884,  954, 1028, 1111
				,	1168, 1077,  999,  927,  858,  799,  747,  700,  658, 622, 591, 566, 547, 531, 521, 515, 513, 514, 520, 529, 541, 559, 582, 611, 644,  681,  722,  769,  823,  886,  957, 1032, 1116
				,	1178, 1088, 1009,  938,  870,  809,  756,  708,  667, 631, 600, 575, 555, 539, 529, 522, 519, 520, 527, 536, 549, 567, 590, 619, 652,  689,  730,  777,  832,  897,  966, 1042, 1130
				,	1192, 1100, 1021,  951,  882,  820,  766,  719,  678, 643, 612, 586, 565, 549, 539, 531, 528, 529, 536, 545, 558, 577, 601, 628, 660,  697,  739,  787,  843,  908,  977, 1054, 1147
				,	1211, 1118, 1038,  967,  900,  837,  782,  735,  695, 659, 628, 601, 580, 564, 552, 546, 543, 544, 550, 560, 573, 591, 616, 643, 675,  712,  755,  805,  861,  924,  993, 1074, 1167
				,	1244, 1147, 1063,  990,  923,  861,  805,  757,  716, 679, 648, 622, 601, 585, 573, 566, 563, 565, 570, 579, 594, 614, 637, 664, 697,  734,  776,  827,  886,  950, 1020, 1100, 1192
				,	1279, 1177, 1091, 1018,  950,  887,  829,  780,  739, 704, 673, 646, 625, 609, 597, 590, 586, 587, 593, 602, 618, 638, 661, 688, 721,  757,  800,  851,  910,  976, 1049, 1130, 1221
				,	1317, 1214, 1128, 1054,  985,  922,  863,  812,  770, 735, 704, 677, 656, 639, 628, 620, 616, 617, 623, 634, 648, 667, 691, 719, 752,  790,  834,  886,  944, 1009, 1085, 1171, 1266
				,	1373, 1265, 1177, 1099, 1027,  964,  904,  852,  809, 772, 741, 713, 692, 676, 665, 657, 654, 654, 661, 671, 685, 704, 728, 756, 789,  829,  875,  928,  986, 1053, 1131, 1218, 1318
				,	1420, 1318, 1229, 1144, 1068, 1004,  947,  894,  848, 810, 778, 751, 730, 714, 703, 696, 692, 693, 699, 709, 723, 742, 766, 794, 828,  869,  916,  967, 1027, 1097, 1178, 1265, 1358
				,	1462, 1377, 1291, 1201, 1120, 1053,  996,  944,  898, 857, 823, 795, 773, 758, 746, 738, 735, 736, 741, 752, 767, 786, 811, 839, 874,  915,  962, 1016, 1080, 1152, 1233, 1322, 1416
				,	1553, 1449, 1358, 1268, 1185, 1115, 1054, 1001,  955, 915, 878, 848, 825, 809, 796, 788, 786, 786, 792, 803, 819, 839, 864, 893, 929,  970, 1017, 1075, 1141, 1215, 1296, 1394, 1522
				,	1707, 1534, 1424, 1340, 1257, 1181, 1116, 1061, 1014, 976, 937, 906, 884, 863, 849, 843, 840, 839, 847, 861, 876, 897, 922, 952, 990, 1030, 1076, 1139, 1208, 1281, 1360, 1477, 1664
				};
				static const uint16_t romBCE2AD0636E77498[825] = {
					1582, 1438, 1323, 1243, 1166, 1089, 1026,  981, 940, 900, 865, 840, 819, 798, 784, 775, 772, 776, 786, 793, 801, 822, 849, 882, 916,  957, 1011, 1061, 1118, 1188, 1271, 1388, 1551
				,	1475, 1364, 1266, 1181, 1106, 1039,  979,  932, 891, 852, 818, 793, 774, 756, 744, 736, 732, 734, 743, 752, 764, 784, 808, 838, 873,  912,  960, 1010, 1069, 1139, 1221, 1318, 1430
				,	1420, 1306, 1208, 1126, 1053,  988,  935,  889, 847, 809, 778, 754, 734, 716, 705, 699, 694, 694, 702, 712, 728, 747, 768, 797, 831,  869,  913,  964, 1023, 1091, 1170, 1258, 1360
				,	1366, 1250, 1150, 1070, 1001,  940,  893,  849, 806, 769, 741, 716, 696, 679, 667, 659, 657, 657, 662, 673, 688, 708, 730, 758, 790,  826,  873,  924,  978, 1042, 1114, 1200, 1302
				,	1302, 1193, 1102, 1027,  961,  904,  855,  810, 769, 735, 709, 684, 662, 647, 635, 626, 624, 626, 631, 641, 656, 675, 696, 724, 756,  791,  838,  888,  940, 1000, 1067, 1148, 1247
				,	1263, 1149, 1062,  992,  926,  870,  822,  778, 739, 707, 682, 657, 633, 617, 607, 599, 597, 600, 605, 614, 629, 646, 668, 696, 729,  764,  804,  852,  906,  967, 1031, 1107, 1206
				,	1215, 1109, 1023,  954,  891,  837,  790,  748, 713, 681, 654, 630, 607, 591, 581, 573, 570, 573, 578, 588, 604, 621, 643, 671, 703,  736,  775,  820,  873,  935, 1000, 1077, 1170
				,	1157, 1074,  996,  928,  867,  813,  766,  725, 689, 658, 631, 606, 585, 571, 561, 553, 548, 550, 557, 567, 582, 599, 622, 649, 678,  713,  755,  802,  850,  907,  973, 1049, 1141
				,	1122, 1051,  975,  907,  849,  793,  748,  709, 673, 640, 611, 587, 568, 555, 546, 538, 534, 536, 542, 551, 564, 581, 604, 631, 660,  697,  738,  783,  831,  885,  950, 1020, 1102
				,	1105, 1029,  954,  888,  831,  778,  733,  693, 657, 624, 594, 572, 553, 540, 532, 527, 524, 525, 529, 537, 550, 567, 589, 615, 645,  681,  721,  764,  813,  867,  931, 1002, 1073
				,	1094, 1013,  940,  874,  816,  767,  721,  680, 645, 613, 585, 563, 544, 531, 523, 519, 517, 518, 521, 530, 542, 559, 580, 605, 635,  671,  710,  754,  802,  856,  922,  994, 1071
				,	1088, 1008,  937,  870,  813,  761,  717,  678, 642, 609, 581, 559, 541, 528, 520, 516, 514, 513, 517, 527, 538, 555, 576, 601, 632,  667,  706,  749,  796,  850,  917,  988, 1069
				,	1088, 1008,  937,  870,  811,  760,  717,  678, 640, 607, 579, 555, 539, 526, 518, 515, 512, 511, 515, 524, 536, 553, 574, 601, 631,  664,  703,  746,  794,  849,  917,  985, 1054
				,	1089, 1011,  939,  873,  815,  764,  719,  678, 642, 610, 581, 558, 541, 528, 519, 515, 512, 512, 517, 525, 538, 556, 577, 604, 634,  668,  706,  749,  797,  854,  923,  990, 1057
				,	1101, 1022,  951,  887,  826,  774,  728,  686, 650, 618, 589, 566, 548, 534, 525, 520, 517, 518, 523, 533, 546, 564, 586, 610, 640,  676,  715,  756,  806,  865,  931, 1001, 1074
				,	1128, 1034,  963,  902,  838,  784,  739,  696, 660, 627, 600, 578, 557, 542, 534, 528, 525, 526, 532, 541, 555, 573, 595, 620, 649,  685,  724,  764,  817,  879,  942, 1016, 1102
				,	1149, 1054,  979,  918,  857,  802,  753,  711, 676, 644, 616, 591, 571, 557, 547, 540, 537, 539, 545, 554, 567, 585, 609, 634, 664,  698,  737,  780,  835,  896,  958, 1037, 1135
				,	1184, 1083, 1004,  943,  885,  826,  774,  731, 695, 664, 636, 610, 590, 577, 566, 559, 555, 557, 563, 572, 586, 606, 628, 654, 685,  720,  760,  804,  858,  920,  984, 1063, 1160
				,	1228, 1118, 1034,  970,  909,  848,  797,  755, 717, 686, 659, 634, 614, 600, 588, 580, 576, 577, 584, 594, 609, 629, 650, 675, 707,  742,  784,  829,  884,  947, 1017, 1095, 1187
				,	1263, 1160, 1075, 1003,  940,  881,  828,  785, 748, 717, 688, 663, 644, 628, 615, 609, 605, 605, 611, 623, 637, 657, 680, 704, 736,  773,  813,  863,  920,  983, 1059, 1142, 1232
				,	1321, 1212, 1124, 1046,  982,  926,  870,  822, 783, 753, 723, 696, 676, 661, 650, 643, 641, 642, 647, 656, 671, 692, 715, 740, 772,  811,  855,  907,  962, 1026, 1105, 1192, 1294
				,	1382, 1271, 1180, 1096, 1025,  969,  916,  866, 821, 787, 758, 730, 710, 697, 686, 678, 675, 676, 683, 692, 706, 726, 748, 775, 808,  846,  895,  947, 1003, 1072, 1153, 1241, 1340
				,	1428, 1339, 1252, 1160, 1081, 1019,  964,  916, 871, 831, 798, 771, 750, 735, 723, 716, 715, 715, 721, 731, 746, 766, 788, 819, 854,  893,  938,  989, 1055, 1133, 1213, 1305, 1418
				,	1525, 1423, 1327, 1234, 1150, 1076, 1019,  971, 926, 886, 850, 820, 795, 778, 766, 761, 761, 760, 764, 776, 793, 813, 837, 870, 909,  952,  996, 1049, 1119, 1200, 1279, 1384, 1546
				,	1677, 1515, 1398, 1316, 1227, 1134, 1078, 1028, 980, 950, 911, 871, 842, 824, 815, 811, 811, 807, 808, 826, 846, 860, 889, 921, 964, 1011, 1057, 1127, 1196, 1265, 1344, 1470, 1680
				};
				memcpy(stInterp0[idx0].var1D402F229DD24906, rom1D402F229DD24906, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].var616201DDA585121F, rom616201DDA585121F, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].varBCE2AD0636E77498, romBCE2AD0636E77498, sizeof(uint16_t) * 825);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 3) {
				static const uint16_t rom1D402F229DD24906[825] = {
					1398, 1303, 1227, 1157, 1087, 1017,  961, 920, 883, 852, 826, 802, 777, 766, 754, 739, 733, 740, 747, 754, 773, 795, 818, 844, 865, 899,  954, 1005, 1070, 1137, 1211, 1298, 1368
				,	1344, 1250, 1174, 1104, 1026,  959,  911, 870, 838, 809, 779, 758, 739, 724, 715, 707, 702, 704, 709, 716, 730, 752, 777, 802, 829, 861,  901,  948, 1006, 1078, 1157, 1239, 1324
				,	1299, 1201, 1123, 1051,  974,  914,  871, 831, 797, 768, 741, 720, 705, 689, 678, 674, 672, 670, 674, 684, 697, 715, 737, 762, 791, 823,  858,  905,  960, 1025, 1102, 1179, 1269
				,	1258, 1155, 1071,  997,  930,  878,  838, 799, 761, 730, 708, 687, 670, 656, 646, 641, 640, 638, 642, 654, 668, 682, 699, 723, 753, 784,  823,  868,  918,  982, 1051, 1125, 1221
				,	1225, 1119, 1027,  951,  892,  846,  807, 769, 733, 704, 680, 659, 641, 627, 620, 614, 609, 610, 616, 627, 641, 655, 669, 691, 722, 754,  791,  831,  880,  945, 1012, 1086, 1172
				,	1183, 1078,  991,  917,  863,  819,  778, 740, 706, 680, 658, 637, 620, 605, 596, 590, 587, 588, 593, 602, 616, 631, 649, 672, 699, 731,  767,  805,  851,  908,  976, 1048, 1123
				,	1135, 1039,  959,  892,  838,  792,  750, 712, 681, 658, 636, 614, 597, 583, 573, 567, 565, 565, 570, 580, 593, 610, 629, 653, 677, 707,  745,  783,  823,  877,  942, 1014, 1098
				,	1100, 1013,  938,  874,  819,  769,  728, 693, 664, 640, 617, 595, 576, 564, 554, 548, 545, 547, 552, 562, 575, 592, 610, 632, 656, 686,  724,  766,  806,  856,  919,  992, 1077
				,	1084,  993,  918,  856,  801,  752,  714, 681, 650, 625, 603, 581, 563, 551, 541, 534, 534, 536, 540, 547, 558, 576, 596, 617, 641, 672,  707,  749,  792,  838,  899,  969, 1050
				,	1074,  969,  893,  837,  785,  742,  703, 667, 639, 613, 589, 569, 551, 539, 531, 525, 524, 525, 529, 536, 546, 561, 583, 605, 630, 662,  694,  731,  775,  820,  878,  950, 1028
				,	1057,  957,  882,  829,  780,  735,  692, 657, 631, 603, 579, 560, 543, 530, 523, 520, 517, 517, 521, 528, 539, 554, 573, 597, 623, 652,  685,  722,  766,  811,  868,  942, 1028
				,	1050,  954,  883,  830,  778,  732,  690, 655, 627, 598, 576, 556, 540, 529, 520, 516, 514, 514, 518, 525, 536, 552, 571, 593, 620, 647,  679,  719,  763,  809,  867,  939, 1022
				,	1039,  951,  885,  828,  775,  731,  691, 654, 625, 598, 575, 554, 538, 528, 518, 514, 512, 512, 517, 524, 534, 549, 568, 591, 617, 646,  678,  718,  761,  808,  867,  936, 1007
				,	1046,  959,  888,  829,  777,  734,  696, 658, 628, 601, 575, 556, 540, 528, 519, 514, 512, 513, 518, 526, 536, 550, 570, 594, 619, 650,  684,  721,  760,  808,  868,  934, 1010
				,	1057,  968,  894,  839,  790,  745,  706, 666, 634, 608, 584, 564, 547, 535, 525, 518, 516, 519, 524, 532, 542, 557, 577, 601, 628, 658,  692,  725,  766,  817,  874,  942, 1021
				,	1062,  977,  906,  851,  803,  755,  711, 673, 643, 619, 595, 574, 557, 544, 533, 527, 524, 527, 532, 539, 550, 566, 585, 608, 633, 662,  697,  732,  774,  827,  886,  955, 1036
				,	1084,  996,  927,  868,  816,  769,  724, 688, 660, 634, 608, 584, 567, 555, 545, 539, 535, 537, 542, 550, 561, 577, 599, 620, 644, 675,  710,  747,  791,  840,  905,  979, 1051
				,	1119, 1025,  952,  889,  835,  791,  748, 709, 676, 648, 624, 601, 584, 570, 561, 556, 552, 554, 559, 565, 578, 595, 615, 636, 663, 695,  728,  766,  815,  871,  935, 1008, 1084
				,	1145, 1057,  979,  913,  860,  814,  771, 729, 695, 667, 642, 620, 603, 589, 582, 575, 571, 574, 578, 584, 596, 612, 631, 654, 683, 714,  747,  785,  836,  899,  965, 1036, 1117
				,	1178, 1096, 1017,  949,  894,  844,  797, 756, 722, 691, 665, 644, 626, 613, 605, 597, 593, 595, 601, 608, 619, 634, 654, 679, 706, 738,  774,  814,  867,  928,  995, 1073, 1160
				,	1254, 1145, 1064,  999,  938,  884,  830, 786, 751, 720, 694, 671, 653, 642, 634, 625, 623, 625, 630, 637, 648, 665, 684, 708, 735, 771,  812,  854,  906,  966, 1036, 1121, 1220
				,	1305, 1187, 1110, 1046,  979,  921,  869, 824, 783, 751, 725, 701, 683, 671, 663, 655, 651, 654, 659, 667, 681, 698, 714, 737, 768, 806,  848,  889,  942, 1002, 1081, 1176, 1269
				,	1335, 1248, 1170, 1092, 1024,  967,  915, 870, 826, 790, 762, 737, 717, 704, 695, 690, 686, 686, 690, 699, 717, 732, 749, 772, 804, 844,  884,  931,  991, 1056, 1138, 1235, 1339
				,	1425, 1329, 1245, 1158, 1081, 1023,  965, 917, 874, 835, 803, 778, 757, 744, 733, 728, 728, 727, 730, 741, 758, 776, 795, 821, 854, 892,  934,  989, 1050, 1119, 1203, 1310, 1446
				,	1583, 1416, 1321, 1241, 1138, 1069, 1017, 967, 920, 881, 846, 824, 803, 783, 775, 770, 772, 770, 775, 790, 808, 827, 849, 879, 911, 952, 1000, 1049, 1103, 1170, 1262, 1398, 1604
				};
				static const uint16_t rom616201DDA585121F[825] = {
					1603, 1475, 1377, 1299, 1220, 1141, 1080, 1035,  994, 952, 917, 889, 866, 850, 838, 827, 822, 824, 832, 839, 852, 876, 902, 933, 966, 1004, 1051, 1108, 1171, 1235, 1317, 1426, 1560
				,	1520, 1412, 1322, 1241, 1163, 1091, 1030,  982,  940, 899, 863, 835, 814, 797, 785, 777, 772, 773, 780, 789, 803, 824, 849, 878, 913,  952,  998, 1052, 1113, 1182, 1263, 1356, 1459
				,	1472, 1360, 1266, 1185, 1108, 1041,  984,  934,  889, 849, 815, 787, 766, 748, 736, 729, 725, 725, 731, 742, 757, 778, 800, 829, 863,  904,  950, 1001, 1060, 1128, 1208, 1294, 1392
				,	1430, 1310, 1211, 1128, 1054,  990,  936,  887,  841, 802, 770, 742, 721, 703, 690, 683, 679, 679, 686, 697, 713, 733, 755, 782, 817,  857,  904,  955, 1010, 1077, 1153, 1238, 1338
				,	1373, 1258, 1163, 1081, 1009,  948,  894,  845,  800, 763, 732, 705, 683, 664, 652, 645, 640, 641, 648, 659, 674, 694, 716, 744, 778,  815,  863,  915,  970, 1035, 1108, 1192, 1288
				,	1329, 1215, 1122, 1042,  974,  914,  860,  811,  767, 730, 700, 674, 650, 631, 619, 613, 609, 610, 616, 627, 642, 661, 685, 713, 746,  782,  827,  880,  937, 1000, 1073, 1155, 1247
				,	1286, 1177, 1085, 1007,  941,  882,  827,  778,  736, 700, 669, 642, 620, 602, 590, 582, 579, 580, 586, 597, 613, 632, 655, 685, 718,  754,  798,  848,  904,  968, 1039, 1118, 1211
				,	1243, 1143, 1055,  981,  915,  854,  799,  752,  711, 675, 643, 617, 595, 579, 568, 559, 555, 557, 563, 573, 589, 608, 631, 660, 693,  731,  775,  824,  878,  940, 1009, 1089, 1182
				,	1212, 1117, 1032,  960,  894,  832,  779,  733,  691, 655, 623, 597, 576, 561, 550, 543, 539, 541, 546, 555, 570, 589, 612, 640, 673,  712,  756,  805,  860,  920,  988, 1067, 1159
				,	1196, 1096, 1013,  941,  875,  815,  762,  715,  673, 638, 606, 580, 560, 545, 535, 529, 527, 528, 533, 541, 555, 573, 596, 625, 657,  695,  739,  787,  841,  902,  970, 1047, 1135
				,	1182, 1084, 1002,  929,  862,  803,  751,  703,  663, 627, 596, 571, 550, 535, 526, 521, 518, 519, 524, 533, 546, 563, 585, 614, 647,  684,  727,  775,  828,  890,  959, 1035, 1123
				,	1172, 1078,  998,  926,  858,  799,  746,  698,  658, 622, 591, 566, 546, 532, 522, 517, 514, 514, 519, 528, 541, 559, 581, 609, 642,  679,  722,  769,  823,  887,  956, 1030, 1118
				,	1167, 1074,  995,  923,  855,  796,  744,  697,  656, 620, 589, 564, 544, 530, 519, 514, 512, 513, 517, 526, 539, 556, 579, 607, 640,  678,  719,  766,  821,  884,  954, 1028, 1111
				,	1168, 1077,  999,  927,  858,  799,  747,  700,  658, 622, 591, 566, 547, 531, 521, 515, 513, 514, 520, 529, 541, 559, 582, 611, 644,  681,  722,  769,  823,  886,  957, 1032, 1116
				,	1178, 1088, 1009,  938,  870,  809,  756,  708,  667, 631, 600, 575, 555, 539, 529, 522, 519, 520, 527, 536, 549, 567, 590, 619, 652,  689,  730,  777,  832,  897,  966, 1042, 1130
				,	1192, 1100, 1021,  951,  882,  820,  766,  719,  678, 643, 612, 586, 565, 549, 539, 531, 528, 529, 536, 545, 558, 577, 601, 628, 660,  697,  739,  787,  843,  908,  977, 1054, 1147
				,	1211, 1118, 1038,  967,  900,  837,  782,  735,  695, 659, 628, 601, 580, 564, 552, 546, 543, 544, 550, 560, 573, 591, 616, 643, 675,  712,  755,  805,  861,  924,  993, 1074, 1167
				,	1244, 1147, 1063,  990,  923,  861,  805,  757,  716, 679, 648, 622, 601, 585, 573, 566, 563, 565, 570, 579, 594, 614, 637, 664, 697,  734,  776,  827,  886,  950, 1020, 1100, 1192
				,	1279, 1177, 1091, 1018,  950,  887,  829,  780,  739, 704, 673, 646, 625, 609, 597, 590, 586, 587, 593, 602, 618, 638, 661, 688, 721,  757,  800,  851,  910,  976, 1049, 1130, 1221
				,	1317, 1214, 1128, 1054,  985,  922,  863,  812,  770, 735, 704, 677, 656, 639, 628, 620, 616, 617, 623, 634, 648, 667, 691, 719, 752,  790,  834,  886,  944, 1009, 1085, 1171, 1266
				,	1373, 1265, 1177, 1099, 1027,  964,  904,  852,  809, 772, 741, 713, 692, 676, 665, 657, 654, 654, 661, 671, 685, 704, 728, 756, 789,  829,  875,  928,  986, 1053, 1131, 1218, 1318
				,	1420, 1318, 1229, 1144, 1068, 1004,  947,  894,  848, 810, 778, 751, 730, 714, 703, 696, 692, 693, 699, 709, 723, 742, 766, 794, 828,  869,  916,  967, 1027, 1097, 1178, 1265, 1358
				,	1462, 1377, 1291, 1201, 1120, 1053,  996,  944,  898, 857, 823, 795, 773, 758, 746, 738, 735, 736, 741, 752, 767, 786, 811, 839, 874,  915,  962, 1016, 1080, 1152, 1233, 1322, 1416
				,	1553, 1449, 1358, 1268, 1185, 1115, 1054, 1001,  955, 915, 878, 848, 825, 809, 796, 788, 786, 786, 792, 803, 819, 839, 864, 893, 929,  970, 1017, 1075, 1141, 1215, 1296, 1394, 1522
				,	1707, 1534, 1424, 1340, 1257, 1181, 1116, 1061, 1014, 976, 937, 906, 884, 863, 849, 843, 840, 839, 847, 861, 876, 897, 922, 952, 990, 1030, 1076, 1139, 1208, 1281, 1360, 1477, 1664
				};
				static const uint16_t romBCE2AD0636E77498[825] = {
					1778, 1640, 1535, 1455, 1364, 1276, 1207, 1150, 1096, 1041,  998,  964, 939, 917, 898, 886, 874, 876, 892, 902, 917, 949,  985, 1021, 1061, 1111, 1173, 1234, 1297, 1365, 1458, 1582, 1715
				,	1674, 1561, 1467, 1379, 1290, 1210, 1141, 1081, 1030,  978,  932,  898, 872, 850, 833, 824, 819, 820, 831, 841, 858, 885,  919,  958, 1001, 1050, 1106, 1168, 1236, 1314, 1407, 1509, 1615
				,	1629, 1501, 1401, 1314, 1228, 1149, 1081, 1022,  970,  920,  874,  839, 813, 792, 775, 767, 764, 765, 772, 785, 805, 830,  859,  898,  943,  994, 1050, 1112, 1180, 1258, 1347, 1439, 1542
				,	1571, 1446, 1343, 1253, 1167, 1090, 1024,  966,  910,  862,  822,  787, 758, 737, 722, 713, 710, 709, 717, 731, 751, 777,  805,  842,  886,  936,  996, 1058, 1124, 1200, 1286, 1376, 1479
				,	1491, 1388, 1289, 1194, 1113, 1041,  976,  915,  859,  813,  777,  743, 713, 690, 676, 668, 663, 663, 671, 685, 704, 728,  757,  796,  839,  884,  942, 1007, 1074, 1151, 1235, 1323, 1423
				,	1450, 1340, 1239, 1148, 1069,  997,  931,  873,  820,  774,  736,  704, 675, 652, 637, 630, 626, 626, 634, 648, 667, 691,  720,  756,  799,  846,  902,  964, 1033, 1111, 1193, 1280, 1373
				,	1413, 1295, 1195, 1109, 1029,  955,  890,  832,  781,  737,  698,  666, 640, 619, 604, 595, 592, 593, 600, 613, 632, 657,  686,  721,  763,  811,  865,  924,  993, 1072, 1153, 1241, 1339
				,	1375, 1256, 1158, 1075,  997,  922,  857,  800,  750,  706,  668,  636, 610, 591, 577, 568, 564, 565, 572, 584, 603, 627,  656,  691,  732,  781,  834,  894,  964, 1037, 1115, 1208, 1320
				,	1334, 1225, 1130, 1046,  968,  895,  833,  776,  725,  682,  644,  612, 587, 568, 556, 548, 545, 546, 552, 563, 579, 603,  631,  666,  707,  757,  811,  871,  941, 1011, 1088, 1179, 1286
				,	1305, 1198, 1106, 1022,  943,  873,  812,  753,  703,  661,  623,  593, 569, 550, 538, 531, 529, 531, 536, 545, 561, 584,  612,  647,  687,  735,  789,  848,  918,  988, 1065, 1153, 1249
				,	1284, 1183, 1093, 1008,  930,  860,  796,  738,  690,  647,  610,  581, 557, 538, 528, 522, 518, 520, 526, 536, 551, 573,  600,  635,  676,  721,  774,  835,  904,  975, 1052, 1141, 1240
				,	1280, 1176, 1087, 1003,  924,  852,  789,  734,  685,  641,  604,  575, 551, 534, 523, 517, 514, 515, 521, 531, 546, 568,  594,  628,  669,  715,  767,  829,  897,  971, 1051, 1137, 1234
				,	1278, 1173, 1084, 1001,  921,  848,  788,  733,  682,  638,  601,  572, 550, 533, 520, 515, 512, 512, 517, 527, 542, 564,  592,  625,  667,  713,  764,  824,  892,  967, 1050, 1134, 1227
				,	1268, 1177, 1090, 1004,  924,  853,  792,  736,  685,  641,  605,  576, 553, 535, 522, 517, 514, 514, 520, 531, 546, 568,  595,  630,  672,  717,  768,  828,  895,  969, 1050, 1137, 1236
				,	1281, 1188, 1101, 1017,  937,  867,  805,  746,  696,  652,  616,  586, 562, 543, 531, 524, 521, 522, 529, 540, 556, 577,  605,  639,  681,  728,  779,  837,  903,  980, 1063, 1149, 1247
				,	1311, 1203, 1114, 1034,  954,  882,  819,  761,  711,  668,  631,  599, 573, 555, 542, 534, 531, 533, 540, 551, 567, 589,  618,  652,  693,  741,  792,  849,  916,  996, 1080, 1166, 1267
				,	1336, 1230, 1139, 1057,  977,  902,  838,  781,  733,  689,  650,  617, 591, 573, 559, 551, 547, 549, 556, 568, 585, 608,  638,  672,  712,  760,  812,  872,  941, 1017, 1099, 1189, 1292
				,	1369, 1264, 1172, 1085, 1006,  935,  867,  808,  760,  716,  676,  643, 618, 599, 585, 576, 572, 574, 581, 592, 610, 635,  664,  698,  740,  787,  839,  902,  973, 1047, 1130, 1222, 1323
				,	1409, 1298, 1205, 1120, 1041,  971,  902,  840,  789,  746,  707,  673, 648, 628, 614, 604, 600, 603, 610, 621, 640, 665,  694,  730,  771,  817,  870,  933, 1005, 1084, 1169, 1260, 1360
				,	1467, 1348, 1251, 1168, 1089, 1014,  944,  882,  830,  787,  747,  713, 686, 665, 650, 641, 638, 639, 647, 660, 679, 702,  732,  768,  811,  859,  913,  977, 1049, 1127, 1215, 1309, 1409
				,	1542, 1412, 1310, 1223, 1142, 1067,  996,  932,  878,  834,  794,  759, 733, 711, 696, 687, 683, 686, 694, 706, 725, 748,  777,  815,  859,  909,  966, 1029, 1100, 1177, 1266, 1365, 1470
				,	1596, 1475, 1372, 1275, 1189, 1117, 1051,  986,  927,  880,  840,  807, 780, 759, 745, 736, 731, 733, 740, 753, 772, 797,  825,  861,  907,  959, 1017, 1077, 1149, 1230, 1319, 1420, 1525
				,	1645, 1543, 1445, 1343, 1254, 1179, 1110, 1048,  991,  938,  894,  861, 834, 813, 798, 788, 784, 784, 792, 808, 828, 853,  883,  919,  963, 1015, 1072, 1136, 1213, 1295, 1385, 1488, 1593
				,	1741, 1622, 1527, 1429, 1332, 1252, 1180, 1118, 1063, 1010,  964,  926, 895, 874, 858, 847, 844, 846, 854, 871, 892, 918,  950,  988, 1032, 1080, 1138, 1205, 1284, 1370, 1458, 1568, 1708
				,	1909, 1707, 1602, 1526, 1413, 1317, 1252, 1191, 1134, 1090, 1045, 1001, 967, 941, 922, 914, 913, 915, 923, 942, 962, 986, 1018, 1058, 1111, 1159, 1213, 1273, 1351, 1451, 1530, 1652, 1863
				};
				memcpy(stInterp0[idx0].var1D402F229DD24906, rom1D402F229DD24906, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].var616201DDA585121F, rom616201DDA585121F, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].varBCE2AD0636E77498, romBCE2AD0636E77498, sizeof(uint16_t) * 825);
				pos0++;
				idx0++;
			};
			if (idx0 < 2 && pos0 == 4) {
				static const uint16_t rom1D402F229DD24906[825] = {
					1469, 1310, 1202, 1137, 1054,  983,  932, 903, 880, 833, 803, 788, 771, 760, 757, 747, 736, 748, 755, 755, 767, 777, 803, 832, 854, 892, 937, 1002, 1070, 1116, 1211, 1323, 1384
				,	1359, 1245, 1161, 1086, 1003,  945,  902, 865, 833, 799, 770, 750, 735, 724, 716, 709, 704, 706, 714, 721, 732, 749, 771, 797, 829, 859, 891,  944, 1004, 1066, 1152, 1254, 1366
				,	1307, 1191, 1108, 1032,  959,  907,  865, 828, 791, 763, 738, 715, 701, 688, 679, 676, 674, 672, 678, 690, 701, 719, 738, 761, 794, 826, 858,  901,  954, 1017, 1095, 1186, 1290
				,	1273, 1143, 1049,  979,  919,  866,  822, 790, 757, 729, 704, 683, 669, 655, 645, 642, 640, 638, 643, 658, 671, 687, 704, 724, 752, 785, 823,  866,  915,  971, 1040, 1122, 1217
				,	1217, 1101, 1009,  939,  884,  833,  790, 758, 728, 700, 678, 659, 643, 629, 617, 611, 609, 608, 613, 629, 643, 658, 673, 695, 725, 753, 788,  835,  883,  937, 1000, 1076, 1181
				,	1174, 1066,  979,  907,  858,  812,  768, 734, 705, 678, 658, 638, 619, 606, 595, 588, 587, 586, 591, 602, 616, 634, 651, 674, 702, 731, 765,  811,  860,  912,  972, 1041, 1135
				,	1126, 1029,  946,  878,  832,  792,  748, 713, 687, 659, 633, 614, 598, 585, 574, 567, 565, 564, 568, 579, 593, 609, 630, 655, 682, 711, 744,  786,  833,  884,  939, 1005, 1097
				,	1100,  999,  918,  859,  812,  771,  731, 695, 669, 640, 613, 595, 578, 564, 555, 548, 546, 547, 552, 562, 575, 589, 608, 634, 663, 694, 726,  764,  808,  855,  914,  979, 1057
				,	1076,  979,  902,  846,  798,  753,  715, 681, 653, 625, 599, 579, 563, 550, 539, 535, 533, 535, 542, 549, 559, 575, 595, 619, 646, 678, 712,  749,  791,  842,  900,  962, 1037
				,	1058,  958,  888,  834,  783,  738,  701, 668, 638, 612, 585, 564, 551, 540, 530, 526, 525, 526, 530, 538, 548, 562, 584, 608, 633, 666, 704,  739,  780,  831,  882,  947, 1034
				,	1059,  947,  878,  828,  772,  729,  691, 658, 630, 604, 579, 556, 542, 531, 525, 520, 517, 518, 523, 531, 541, 555, 574, 597, 625, 658, 694,  730,  770,  817,  872,  941, 1028
				,	1047,  944,  877,  825,  768,  725,  689, 656, 628, 602, 578, 555, 538, 526, 520, 518, 514, 515, 520, 526, 537, 553, 570, 593, 622, 654, 689,  725,  763,  811,  870,  937, 1018
				,	1036,  945,  878,  820,  767,  725,  689, 657, 627, 600, 576, 555, 539, 526, 519, 516, 514, 513, 516, 523, 533, 549, 569, 592, 618, 650, 687,  723,  763,  810,  865,  932, 1010
				,	1028,  947,  883,  825,  771,  729,  696, 663, 631, 603, 579, 561, 543, 528, 520, 515, 511, 512, 518, 524, 535, 551, 572, 596, 622, 654, 687,  723,  766,  814,  871,  935, 1005
				,	1039,  958,  893,  839,  784,  740,  706, 671, 641, 612, 586, 568, 550, 534, 526, 519, 515, 518, 524, 532, 544, 558, 578, 601, 631, 662, 691,  728,  771,  822,  880,  945, 1017
				,	1058,  969,  901,  852,  799,  752,  713, 677, 651, 621, 593, 574, 555, 541, 533, 525, 524, 526, 530, 540, 552, 566, 587, 609, 639, 668, 699,  736,  781,  833,  887,  954, 1030
				,	1073,  982,  917,  871,  816,  768,  729, 692, 661, 633, 607, 585, 566, 552, 543, 538, 537, 536, 541, 552, 563, 578, 600, 620, 648, 679, 713,  752,  799,  851,  906,  973, 1036
				,	1094, 1015,  947,  890,  835,  786,  748, 712, 679, 649, 625, 605, 585, 570, 560, 555, 553, 553, 557, 569, 582, 595, 615, 637, 667, 696, 728,  769,  818,  873,  931, 1000, 1063
				,	1119, 1048,  976,  910,  858,  810,  764, 726, 696, 667, 641, 622, 605, 589, 579, 573, 571, 573, 577, 587, 600, 615, 635, 659, 688, 715, 746,  790,  839,  893,  957, 1029, 1096
				,	1157, 1078, 1007,  947,  890,  839,  791, 749, 717, 689, 665, 644, 629, 614, 605, 599, 594, 598, 603, 610, 620, 636, 659, 684, 712, 743, 778,  820,  872,  928,  992, 1071, 1164
				,	1238, 1124, 1046,  988,  927,  873,  826, 786, 751, 718, 695, 673, 653, 643, 636, 629, 625, 626, 632, 640, 650, 668, 690, 713, 744, 780, 813,  853,  909,  969, 1039, 1124, 1223
				,	1302, 1186, 1098, 1029,  968,  915,  866, 822, 783, 750, 724, 701, 683, 672, 664, 656, 655, 657, 660, 672, 684, 700, 720, 743, 776, 809, 839,  886,  949, 1010, 1092, 1176, 1239
				,	1322, 1251, 1173, 1093, 1023,  962,  909, 862, 826, 794, 760, 734, 720, 708, 696, 688, 687, 688, 690, 704, 719, 733, 755, 781, 812, 840, 873,  933, 1005, 1071, 1153, 1240, 1315
				,	1408, 1319, 1243, 1164, 1086, 1018,  959, 908, 874, 841, 804, 778, 762, 751, 737, 729, 729, 727, 733, 744, 756, 776, 803, 825, 854, 891, 927,  984, 1054, 1124, 1205, 1313, 1456
				,	1567, 1391, 1303, 1239, 1156, 1082, 1015, 960, 918, 885, 857, 837, 812, 796, 784, 778, 780, 776, 784, 789, 791, 822, 852, 867, 902, 955, 995, 1030, 1089, 1161, 1251, 1387, 1596
				};
				static const uint16_t rom616201DDA585121F[825] = {
					1603, 1475, 1377, 1299, 1220, 1141, 1080, 1035,  994, 952, 917, 889, 866, 850, 838, 827, 822, 824, 832, 839, 852, 876, 902, 933, 966, 1004, 1051, 1108, 1171, 1235, 1317, 1426, 1560
				,	1520, 1412, 1322, 1241, 1163, 1091, 1030,  982,  940, 899, 863, 835, 814, 797, 785, 777, 772, 773, 780, 789, 803, 824, 849, 878, 913,  952,  998, 1052, 1113, 1182, 1263, 1356, 1459
				,	1472, 1360, 1266, 1185, 1108, 1041,  984,  934,  889, 849, 815, 787, 766, 748, 736, 729, 725, 725, 731, 742, 757, 778, 800, 829, 863,  904,  950, 1001, 1060, 1128, 1208, 1294, 1392
				,	1430, 1310, 1211, 1128, 1054,  990,  936,  887,  841, 802, 770, 742, 721, 703, 690, 683, 679, 679, 686, 697, 713, 733, 755, 782, 817,  857,  904,  955, 1010, 1077, 1153, 1238, 1338
				,	1373, 1258, 1163, 1081, 1009,  948,  894,  845,  800, 763, 732, 705, 683, 664, 652, 645, 640, 641, 648, 659, 674, 694, 716, 744, 778,  815,  863,  915,  970, 1035, 1108, 1192, 1288
				,	1329, 1215, 1122, 1042,  974,  914,  860,  811,  767, 730, 700, 674, 650, 631, 619, 613, 609, 610, 616, 627, 642, 661, 685, 713, 746,  782,  827,  880,  937, 1000, 1073, 1155, 1247
				,	1286, 1177, 1085, 1007,  941,  882,  827,  778,  736, 700, 669, 642, 620, 602, 590, 582, 579, 580, 586, 597, 613, 632, 655, 685, 718,  754,  798,  848,  904,  968, 1039, 1118, 1211
				,	1243, 1143, 1055,  981,  915,  854,  799,  752,  711, 675, 643, 617, 595, 579, 568, 559, 555, 557, 563, 573, 589, 608, 631, 660, 693,  731,  775,  824,  878,  940, 1009, 1089, 1182
				,	1212, 1117, 1032,  960,  894,  832,  779,  733,  691, 655, 623, 597, 576, 561, 550, 543, 539, 541, 546, 555, 570, 589, 612, 640, 673,  712,  756,  805,  860,  920,  988, 1067, 1159
				,	1196, 1096, 1013,  941,  875,  815,  762,  715,  673, 638, 606, 580, 560, 545, 535, 529, 527, 528, 533, 541, 555, 573, 596, 625, 657,  695,  739,  787,  841,  902,  970, 1047, 1135
				,	1182, 1084, 1002,  929,  862,  803,  751,  703,  663, 627, 596, 571, 550, 535, 526, 521, 518, 519, 524, 533, 546, 563, 585, 614, 647,  684,  727,  775,  828,  890,  959, 1035, 1123
				,	1172, 1078,  998,  926,  858,  799,  746,  698,  658, 622, 591, 566, 546, 532, 522, 517, 514, 514, 519, 528, 541, 559, 581, 609, 642,  679,  722,  769,  823,  887,  956, 1030, 1118
				,	1167, 1074,  995,  923,  855,  796,  744,  697,  656, 620, 589, 564, 544, 530, 519, 514, 512, 513, 517, 526, 539, 556, 579, 607, 640,  678,  719,  766,  821,  884,  954, 1028, 1111
				,	1168, 1077,  999,  927,  858,  799,  747,  700,  658, 622, 591, 566, 547, 531, 521, 515, 513, 514, 520, 529, 541, 559, 582, 611, 644,  681,  722,  769,  823,  886,  957, 1032, 1116
				,	1178, 1088, 1009,  938,  870,  809,  756,  708,  667, 631, 600, 575, 555, 539, 529, 522, 519, 520, 527, 536, 549, 567, 590, 619, 652,  689,  730,  777,  832,  897,  966, 1042, 1130
				,	1192, 1100, 1021,  951,  882,  820,  766,  719,  678, 643, 612, 586, 565, 549, 539, 531, 528, 529, 536, 545, 558, 577, 601, 628, 660,  697,  739,  787,  843,  908,  977, 1054, 1147
				,	1211, 1118, 1038,  967,  900,  837,  782,  735,  695, 659, 628, 601, 580, 564, 552, 546, 543, 544, 550, 560, 573, 591, 616, 643, 675,  712,  755,  805,  861,  924,  993, 1074, 1167
				,	1244, 1147, 1063,  990,  923,  861,  805,  757,  716, 679, 648, 622, 601, 585, 573, 566, 563, 565, 570, 579, 594, 614, 637, 664, 697,  734,  776,  827,  886,  950, 1020, 1100, 1192
				,	1279, 1177, 1091, 1018,  950,  887,  829,  780,  739, 704, 673, 646, 625, 609, 597, 590, 586, 587, 593, 602, 618, 638, 661, 688, 721,  757,  800,  851,  910,  976, 1049, 1130, 1221
				,	1317, 1214, 1128, 1054,  985,  922,  863,  812,  770, 735, 704, 677, 656, 639, 628, 620, 616, 617, 623, 634, 648, 667, 691, 719, 752,  790,  834,  886,  944, 1009, 1085, 1171, 1266
				,	1373, 1265, 1177, 1099, 1027,  964,  904,  852,  809, 772, 741, 713, 692, 676, 665, 657, 654, 654, 661, 671, 685, 704, 728, 756, 789,  829,  875,  928,  986, 1053, 1131, 1218, 1318
				,	1420, 1318, 1229, 1144, 1068, 1004,  947,  894,  848, 810, 778, 751, 730, 714, 703, 696, 692, 693, 699, 709, 723, 742, 766, 794, 828,  869,  916,  967, 1027, 1097, 1178, 1265, 1358
				,	1462, 1377, 1291, 1201, 1120, 1053,  996,  944,  898, 857, 823, 795, 773, 758, 746, 738, 735, 736, 741, 752, 767, 786, 811, 839, 874,  915,  962, 1016, 1080, 1152, 1233, 1322, 1416
				,	1553, 1449, 1358, 1268, 1185, 1115, 1054, 1001,  955, 915, 878, 848, 825, 809, 796, 788, 786, 786, 792, 803, 819, 839, 864, 893, 929,  970, 1017, 1075, 1141, 1215, 1296, 1394, 1522
				,	1707, 1534, 1424, 1340, 1257, 1181, 1116, 1061, 1014, 976, 937, 906, 884, 863, 849, 843, 840, 839, 847, 861, 876, 897, 922, 952, 990, 1030, 1076, 1139, 1208, 1281, 1360, 1477, 1664
				};
				static const uint16_t romBCE2AD0636E77498[825] = {
					1819, 1686, 1567, 1473, 1389, 1302, 1228, 1176, 1124, 1073, 1026,  987,  959, 937, 917, 905, 902, 906, 915, 924, 947,  976, 1007, 1053, 1094, 1136, 1190, 1265, 1343, 1415, 1509, 1635, 1800
				,	1735, 1612, 1507, 1413, 1324, 1242, 1172, 1113, 1059, 1006,  957,  921,  894, 870, 852, 843, 839, 840, 849, 861, 881,  908,  940,  983, 1030, 1075, 1130, 1197, 1273, 1353, 1446, 1553, 1671
				,	1679, 1551, 1446, 1355, 1265, 1183, 1116, 1054,  995,  943,  896,  861,  833, 809, 791, 782, 778, 779, 788, 802, 823,  850,  880,  920,  968, 1021, 1078, 1139, 1210, 1289, 1379, 1476, 1578
				,	1621, 1490, 1381, 1289, 1202, 1123, 1058,  995,  934,  883,  840,  804,  776, 752, 735, 725, 721, 723, 731, 745, 767,  794,  825,  861,  907,  962, 1021, 1084, 1153, 1228, 1314, 1414, 1527
				,	1545, 1429, 1325, 1230, 1146, 1070, 1004,  941,  881,  832,  791,  757,  727, 703, 687, 677, 673, 674, 683, 697, 718,  744,  774,  811,  857,  907,  966, 1033, 1105, 1181, 1266, 1365, 1478
				,	1498, 1382, 1274, 1178, 1102, 1026,  955,  894,  838,  789,  749,  715,  685, 661, 645, 635, 633, 635, 642, 656, 677,  702,  732,  770,  815,  864,  922,  991, 1064, 1140, 1224, 1320, 1429
				,	1455, 1336, 1228, 1134, 1059,  985,  912,  850,  798,  751,  710,  675,  647, 625, 609, 599, 596, 598, 605, 619, 640,  666,  695,  733,  777,  824,  884,  951, 1023, 1097, 1177, 1273, 1389
				,	1405, 1294, 1193, 1103, 1024,  947,  879,  817,  764,  718,  677,  644,  617, 596, 580, 571, 567, 568, 576, 589, 609,  634,  664,  701,  743,  793,  851,  916,  987, 1059, 1138, 1235, 1350
				,	1365, 1260, 1163, 1076,  996,  917,  851,  792,  738,  693,  651,  619,  593, 573, 558, 550, 546, 547, 555, 567, 585,  608,  639,  675,  717,  769,  827,  889,  958, 1034, 1117, 1212, 1322
				,	1341, 1232, 1136, 1052,  971,  894,  829,  768,  715,  671,  629,  598,  573, 553, 540, 533, 530, 531, 537, 548, 565,  589,  619,  655,  697,  747,  806,  868,  936, 1012, 1095, 1187, 1299
				,	1329, 1216, 1120, 1036,  953,  879,  813,  751,  701,  657,  617,  585,  560, 541, 529, 522, 519, 520, 526, 537, 554,  576,  606,  642,  683,  732,  789,  852,  921,  997, 1079, 1171, 1289
				,	1314, 1212, 1117, 1030,  946,  871,  805,  746,  695,  651,  612,  580,  554, 537, 525, 517, 514, 515, 521, 531, 548,  571,  601,  636,  677,  725,  779,  840,  913,  994, 1077, 1166, 1277
				,	1307, 1208, 1116, 1026,  944,  869,  802,  744,  693,  647,  608,  578,  553, 534, 522, 515, 512, 513, 518, 528, 544,  568,  598,  635,  674,  722,  777,  839,  911,  989, 1070, 1160, 1267
				,	1311, 1209, 1117, 1029,  948,  875,  807,  748,  696,  650,  612,  582,  557, 536, 524, 517, 514, 515, 521, 532, 549,  572,  601,  637,  680,  727,  782,  846,  915,  991, 1074, 1164, 1272
				,	1328, 1226, 1130, 1043,  963,  888,  819,  759,  708,  661,  622,  592,  566, 546, 533, 525, 521, 523, 530, 541, 559,  583,  611,  647,  691,  738,  791,  854,  926, 1005, 1088, 1178, 1289
				,	1361, 1247, 1147, 1061,  980,  902,  833,  774,  724,  677,  636,  605,  579, 558, 545, 536, 532, 535, 542, 554, 571,  595,  625,  660,  702,  750,  804,  868,  940, 1020, 1100, 1191, 1307
				,	1391, 1272, 1171, 1084, 1002,  925,  855,  796,  745,  698,  658,  625,  598, 577, 563, 555, 551, 552, 561, 573, 590,  613,  644,  680,  722,  771,  827,  892,  962, 1039, 1121, 1216, 1331
				,	1414, 1301, 1205, 1116, 1035,  961,  888,  827,  774,  727,  686,  653,  627, 606, 590, 581, 577, 579, 587, 599, 617,  641,  671,  708,  751,  802,  858,  921,  993, 1072, 1158, 1251, 1358
				,	1450, 1335, 1242, 1157, 1072,  994,  921,  859,  807,  760,  719,  685,  658, 637, 622, 612, 607, 609, 618, 629, 649,  673,  703,  740,  785,  834,  889,  955, 1027, 1109, 1195, 1287, 1394
				,	1501, 1381, 1288, 1205, 1119, 1038,  965,  902,  849,  802,  758,  724,  698, 676, 660, 651, 647, 648, 655, 669, 687,  711,  741,  778,  825,  875,  932, 1000, 1074, 1153, 1240, 1337, 1445
				,	1580, 1446, 1345, 1256, 1172, 1095, 1020,  955,  901,  850,  806,  772,  746, 725, 708, 698, 695, 695, 703, 717, 736,  762,  792,  828,  874,  928,  986, 1053, 1128, 1206, 1297, 1397, 1502
				,	1630, 1516, 1416, 1315, 1224, 1148, 1077, 1013,  954,  902,  859,  824,  795, 774, 758, 748, 744, 744, 752, 767, 787,  812,  844,  882,  927,  980, 1038, 1103, 1180, 1261, 1354, 1454, 1557
				,	1667, 1579, 1490, 1387, 1291, 1213, 1143, 1077, 1020,  966,  921,  884,  852, 829, 814, 804, 800, 801, 808, 824, 845,  870,  903,  942,  985, 1035, 1096, 1164, 1242, 1327, 1415, 1515, 1624
				,	1779, 1659, 1560, 1462, 1367, 1288, 1216, 1147, 1088, 1037,  993,  953,  920, 895, 878, 870, 867, 869, 876, 890, 910,  936,  971, 1011, 1056, 1105, 1164, 1239, 1318, 1397, 1484, 1595, 1729
				,	1943, 1757, 1633, 1537, 1447, 1362, 1288, 1219, 1154, 1115, 1073, 1026, 1000, 970, 948, 945, 943, 941, 951, 965, 977, 1001, 1039, 1085, 1135, 1185, 1238, 1317, 1405, 1474, 1564, 1691, 1846
				};
				memcpy(stInterp0[idx0].var1D402F229DD24906, rom1D402F229DD24906, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].var616201DDA585121F, rom616201DDA585121F, sizeof(uint16_t) * 825);
				memcpy(stInterp0[idx0].varBCE2AD0636E77498, romBCE2AD0636E77498, sizeof(uint16_t) * 825);
				pos0++;
				idx0++;
			};
			for (k0 = 0; k0 < 825; k0++) {
				DataPointer(&(var90665A6460BBD864), offE4957D396891E4D4, uint16_t)[k0] = (uint16_t) DataInterpolate(stInterp0[0].var1D402F229DD24906[k0], stInterp0[1].var1D402F229DD24906[k0], scale0);
			};
			for (k0 = 0; k0 < 825; k0++) {
				DataPointer(&(var90665A6460BBD864), off0A00D8E9E646578E, uint16_t)[k0] = (uint16_t) DataInterpolate(stInterp0[0].var616201DDA585121F[k0], stInterp0[1].var616201DDA585121F[k0], scale0);
			};
			for (k0 = 0; k0 < 825; k0++) {
				DataPointer(&(var90665A6460BBD864), offC2C54FEB365A3B17, uint16_t)[k0] = (uint16_t) DataInterpolate(stInterp0[0].varBCE2AD0636E77498[k0], stInterp0[1].varBCE2AD0636E77498[k0], scale0);
			};
		};
		ProcNS(insert_ACLS)(
			*DataPointer(io_pstParams, off66AD17FFC8ABA1DE, uint8_t)
		,	DataPointer(io_pstParams, offB9383DCFC1062E42, void)
		,	*DataPointer(io_pstParams, off8892D56F1507C7D7, uint16_t)
		,	*DataPointer(io_pstParams, off9A01961F7C1A701F, uint16_t)
		,	*DataPointer(io_pstParams, offC6A480DA42962482, uint16_t)
		,	*DataPointer(io_pstParams, offA03CCC9D08680991, uint16_t)
		,	*DataPointer(io_pstParams, off4702F8CAE6500DFF, uint8_t)
		,	*DataPointer(io_pstParams, off62EA37AD90D10A4F, uint8_t)
		,	*DataPointer(io_pstParams, off41919F2DB508C108, uint16_t)
		,	*DataPointer(io_pstParams, offE918750F4079C56D, uint16_t)
		,	&(var90665A6460BBD864)
		);
		ProcNS(tune_V_noGain)(
			*DataPointer(io_pstParams, offC2CFE0FF981F3945, uint16_t)
		,	*DataPointer(io_pstParams, off89B11A00B367FF2A, uint16_t)
		,	*DataPointer(io_pstParams, offB3540292659F9106, uint8_t)
		,	&(var90665A6460BBD864)
		);
		ProcNS(conv_V_CLS)(
			&(var90665A6460BBD864)
		);
		ProcNS(zoom_V_mkBuf)(
			&(var90665A6460BBD864)
		,	var0F7A0536073F7CD5
		,	sizeof(var0F7A0536073F7CD5)/sizeof(var0F7A0536073F7CD5[0])
		,	var4466E9B746DAD95C
		,	sizeof(var4466E9B746DAD95C)/sizeof(var4466E9B746DAD95C[0])
		);
		ProcNS(zoom_V)(
			*DataPointer(io_pstParams, off42DC478ECF8A6E60, uint16_t)
		,	*DataPointer(io_pstParams, off3E263E9FF3CE8FA3, uint16_t)
		,	*DataPointer(io_pstParams, off25C2C9BDE3113654, uint8_t)
		,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
		,	&(var90665A6460BBD864)
		,	var0F7A0536073F7CD5
		,	sizeof(var0F7A0536073F7CD5)/sizeof(var0F7A0536073F7CD5[0])
		,	var4466E9B746DAD95C
		,	sizeof(var4466E9B746DAD95C)/sizeof(var4466E9B746DAD95C[0])
		,	&(varA1BFFF7227F07355)
		);
		ProcNS(fmt_V)(
			&(varA1BFFF7227F07355)
		,	&(varAB478F3EFC840EEB)
		);
		ProcNS(geom_V)(
			DataPointer(io_pstParams, offE073442BDF64D249, void)
		,	*DataPointer(io_pstParams, off25C2C9BDE3113654, uint8_t)
		,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
		,	&(varAB478F3EFC840EEB)
		,	DataPointer(o_pstData, off9F90FCBA71066093, void)
		);
	};
	if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) == 1) {
		*DataPointer(o_pstData, off08EB7F44CB9440B9, uint8_t) = 1;
		*DataPointer(o_pstData, offF19E629E22AED353, int16_t) = -13;
		*DataPointer(o_pstData, offEAB1711DEA7904A1, uint16_t) = 64;
		*DataPointer(o_pstData, off869846B355DCDEAA, uint16_t) = 64;
		*DataPointer(o_pstData, off84804DAE5089AA04, uint16_t) = 64;
		*DataPointer(o_pstData, offEF62A010DF5C18D8, uint16_t) = 341;
		*DataPointer(o_pstData, off9BE7EDC726A22A1D, uint16_t) = 20;
		*DataPointer(o_pstData, off4B1431808ACBEE25, uint16_t) = 35;
	} else if (*DataPointer(io_pstParams, off0DA62FF6DA2B4363, uint8_t) >= 2) {
		*DataPointer(o_pstData, off08EB7F44CB9440B9, uint8_t) = 1;
		*DataPointer(o_pstData, offF19E629E22AED353, int16_t) = -13;
		*DataPointer(o_pstData, offEAB1711DEA7904A1, uint16_t) = 64;
		*DataPointer(o_pstData, off869846B355DCDEAA, uint16_t) = 511;
		*DataPointer(o_pstData, off84804DAE5089AA04, uint16_t) = 511;
		*DataPointer(o_pstData, offEF62A010DF5C18D8, uint16_t) = 341;
		*DataPointer(o_pstData, off9BE7EDC726A22A1D, uint16_t) = 20;
		*DataPointer(o_pstData, off4B1431808ACBEE25, uint16_t) = 35;
	} else {
		memset(DataPointer(o_pstData, off08EB7F44CB9440B9, uint8_t), 0xff, sizeof(uint8_t));
		memset(DataPointer(o_pstData, offF19E629E22AED353, int16_t), 0xff, sizeof(int16_t));
		memset(DataPointer(o_pstData, offEAB1711DEA7904A1, uint16_t), 0xff, sizeof(uint16_t));
		memset(DataPointer(o_pstData, off869846B355DCDEAA, uint16_t), 0xff, sizeof(uint16_t));
		memset(DataPointer(o_pstData, off84804DAE5089AA04, uint16_t), 0xff, sizeof(uint16_t));
		memset(DataPointer(o_pstData, offEF62A010DF5C18D8, uint16_t), 0xff, sizeof(uint16_t));
		memset(DataPointer(o_pstData, off9BE7EDC726A22A1D, uint16_t), 0xff, sizeof(uint16_t));
		memset(DataPointer(o_pstData, off4B1431808ACBEE25, uint16_t), 0xff, sizeof(uint16_t));
	};
	*DataPointer(o_pstData, offE87C28FAFB1509AD, uint16_t) = 512;
	if (*DataPointer(io_pstParams, offC0AEC4FACF853083, uint8_t) == DxOISP_FORMAT_YUV422) {
		if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else {
			memset(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t), 0xff, sizeof(uint8_t));
			memset(DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, offC0AEC4FACF853083, uint8_t) == DxOISP_FORMAT_YUV420)
	||	(*DataPointer(io_pstParams, offC0AEC4FACF853083, uint8_t) == DxOISP_FORMAT_YUV422_NV)
	) {
		if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off28EA4CF58220131A, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 1;
			*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 0;
		} else {
			memset(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t), 0xff, sizeof(uint8_t));
			memset(DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, offC0AEC4FACF853083, uint8_t) == DxOISP_FORMAT_RGB565)
	||	(*DataPointer(io_pstParams, offC0AEC4FACF853083, uint8_t) == DxOISP_FORMAT_RGB888)
	||	(*DataPointer(io_pstParams, offC0AEC4FACF853083, uint8_t) == DxOISP_FORMAT_RAW)
	) {
		static const int16_t rom3B8354204C5EE770[6] = {
			0, 255
		,	0, 255
		,	0, 255
		};
		static const int16_t romC3BC5A4091B13D9E[9] = {
			512, -176, -366
		,	512,    0,  718
		,	512,  907,    0
		};
		static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
		memcpy(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
		memcpy(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
		memcpy(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
		*DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t) = 0;
		*DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t) = 1;
	} else {
		memset(DataPointer(o_pstData, off59D7D518686C7F68, int16_t), 0xff, sizeof(int16_t) * 6);
		memset(DataPointer(o_pstData, off9A98DFC625533F84, int16_t), 0xff, sizeof(int16_t) * 9);
		memset(DataPointer(o_pstData, off456DBF9F9E372B4B, uint8_t), 0xff, sizeof(uint8_t) * 3);
		memset(DataPointer(o_pstData, off757F0BC35AEB7A0E, uint8_t), 0xff, sizeof(uint8_t));
		memset(DataPointer(o_pstData, offC9B5C6E7E5C10222, uint8_t), 0xff, sizeof(uint8_t));
	};
	if (*DataPointer(io_pstParams, off2E6C4E66852D5015, uint8_t) == DxOISP_FORMAT_YUV422) {
		if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
		} else {
			memset(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, off642326DAB3B79724, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, off2E6C4E66852D5015, uint8_t) == DxOISP_FORMAT_YUV420)
	||	(*DataPointer(io_pstParams, off2E6C4E66852D5015, uint8_t) == DxOISP_FORMAT_YUV422_NV)
	) {
		if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off9DA137804FF1FD88, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 1;
		} else {
			memset(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, off642326DAB3B79724, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, off2E6C4E66852D5015, uint8_t) == DxOISP_FORMAT_RGB565)
	||	(*DataPointer(io_pstParams, off2E6C4E66852D5015, uint8_t) == DxOISP_FORMAT_RGB888)
	||	(*DataPointer(io_pstParams, off2E6C4E66852D5015, uint8_t) == DxOISP_FORMAT_RAW)
	) {
		static const int16_t rom3B8354204C5EE770[6] = {
			0, 255
		,	0, 255
		,	0, 255
		};
		static const int16_t romC3BC5A4091B13D9E[9] = {
			512, -176, -366
		,	512,    0,  718
		,	512,  907,    0
		};
		static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
		memcpy(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
		memcpy(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
		memcpy(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
		*DataPointer(o_pstData, off642326DAB3B79724, uint8_t) = 0;
	} else {
		memset(DataPointer(o_pstData, off8E1B3CDB0F6A4B40, int16_t), 0xff, sizeof(int16_t) * 6);
		memset(DataPointer(o_pstData, off23BCC749B61B9970, int16_t), 0xff, sizeof(int16_t) * 9);
		memset(DataPointer(o_pstData, offB24AA659BDE72176, uint8_t), 0xff, sizeof(uint8_t) * 3);
		memset(DataPointer(o_pstData, off642326DAB3B79724, uint8_t), 0xff, sizeof(uint8_t));
	};
	if (*DataPointer(io_pstParams, off5B7F4ED540BD595E, uint8_t) == DxOISP_FORMAT_YUV422) {
		if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
		} else {
			memset(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, off5B7F4ED540BD595E, uint8_t) == DxOISP_FORMAT_YUV420)
	||	(*DataPointer(io_pstParams, off5B7F4ED540BD595E, uint8_t) == DxOISP_FORMAT_YUV422_NV)
	) {
		if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off17CB440F063E9173, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 1;
		} else {
			memset(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, off5B7F4ED540BD595E, uint8_t) == DxOISP_FORMAT_RGB565)
	||	(*DataPointer(io_pstParams, off5B7F4ED540BD595E, uint8_t) == DxOISP_FORMAT_RGB888)
	||	(*DataPointer(io_pstParams, off5B7F4ED540BD595E, uint8_t) == DxOISP_FORMAT_RAW)
	) {
		static const int16_t rom3B8354204C5EE770[6] = {
			0, 255
		,	0, 255
		,	0, 255
		};
		static const int16_t romC3BC5A4091B13D9E[9] = {
			512, -176, -366
		,	512,    0,  718
		,	512,  907,    0
		};
		static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
		memcpy(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
		memcpy(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
		memcpy(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
		*DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t) = 0;
	} else {
		memset(DataPointer(o_pstData, off18EF0BE20E8C434F, int16_t), 0xff, sizeof(int16_t) * 6);
		memset(DataPointer(o_pstData, off4B9ACA366ABD420E, int16_t), 0xff, sizeof(int16_t) * 9);
		memset(DataPointer(o_pstData, offB0AE9E688E765B73, uint8_t), 0xff, sizeof(uint8_t) * 3);
		memset(DataPointer(o_pstData, offA19F7C5C59E44D33, uint8_t), 0xff, sizeof(uint8_t));
	};
	if (*DataPointer(io_pstParams, offD55E6982B626DEC0, uint8_t) == DxOISP_FORMAT_YUV422) {
		if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
		} else {
			memset(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, offD55E6982B626DEC0, uint8_t) == DxOISP_FORMAT_YUV420)
	||	(*DataPointer(io_pstParams, offD55E6982B626DEC0, uint8_t) == DxOISP_FORMAT_YUV422_NV)
	) {
		if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512,   0,   0
			,	  0, 512,   0
			,	  0,   0, 512
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_601CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440,   0,   0
			,	  0, 450,   0
			,	  0,   0, 450
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709FS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				   0, 255
			,	-128, 127
			,	-128, 127
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709FU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				0, 255
			,	0, 255
			,	0, 255
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				512, -61, -109
			,	  0, 522,   59
			,	  0,  38,  525
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709CS) {
			static const int16_t rom3B8354204C5EE770[6] = {
				  16, 235
			,	-112, 112
			,	-112, 112
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 0, 0 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else if (*DataPointer(io_pstParams, off04AA0A17E79F13A0, uint8_t) == DxOISP_ENCODING_YUV_709CU) {
			static const int16_t rom3B8354204C5EE770[6] = {
				16, 235
			,	16, 240
			,	16, 240
			};
			static const int16_t romC3BC5A4091B13D9E[9] = {
				440, -52, -94
			,	  0, 458,  52
			,	  0,  34, 461
			};
			static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 16, 128, 128 };
			memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
			memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
			memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
			*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 1;
		} else {
			memset(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), 0xff, sizeof(int16_t) * 6);
			memset(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), 0xff, sizeof(int16_t) * 9);
			memset(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), 0xff, sizeof(uint8_t) * 3);
			memset(DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t), 0xff, sizeof(uint8_t));
		};
	} else if (
		(*DataPointer(io_pstParams, offD55E6982B626DEC0, uint8_t) == DxOISP_FORMAT_RGB565)
	||	(*DataPointer(io_pstParams, offD55E6982B626DEC0, uint8_t) == DxOISP_FORMAT_RGB888)
	||	(*DataPointer(io_pstParams, offD55E6982B626DEC0, uint8_t) == DxOISP_FORMAT_RAW)
	) {
		static const int16_t rom3B8354204C5EE770[6] = {
			0, 255
		,	0, 255
		,	0, 255
		};
		static const int16_t romC3BC5A4091B13D9E[9] = {
			512, -176, -366
		,	512,    0,  718
		,	512,  907,    0
		};
		static const uint8_t rom2AB8F1EFDFD5A2B2[3] = { 0, 0, 0 };
		memcpy(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), rom3B8354204C5EE770, sizeof(int16_t) * 6);
		memcpy(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), romC3BC5A4091B13D9E, sizeof(int16_t) * 9);
		memcpy(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), rom2AB8F1EFDFD5A2B2, sizeof(uint8_t) * 3);
		*DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t) = 0;
	} else {
		memset(DataPointer(o_pstData, offE59A9C4D99FD2282, int16_t), 0xff, sizeof(int16_t) * 6);
		memset(DataPointer(o_pstData, off3C06CB6AEE2F1786, int16_t), 0xff, sizeof(int16_t) * 9);
		memset(DataPointer(o_pstData, offD4CFAFF6DA40C562, uint8_t), 0xff, sizeof(uint8_t) * 3);
		memset(DataPointer(o_pstData, off7E530E14F6DE8714, uint8_t), 0xff, sizeof(uint8_t));
	};
	{
		uint32_t var5D3988EEB6042FBD;
		var5D3988EEB6042FBD = 4096;
		ProcNS(insert_Bypass)(
			*DataPointer(io_pstParams, off328569863E317B8B, uint32_t)
		,	var5D3988EEB6042FBD
		,	DataPointer(o_pstData, offF3166A81B502588C, uint32_t)
		);
	};
	ProcNS(insert_RIY)(
		DataPointer(io_pstParams, offBF9271F064677650, void)
	,	DataPointer(io_pstParams, offE3CCCE8E61E56329, void)
	,	DataPointer(io_pstParams, offD21CB324FFCE9543, void)
	,	DataPointer(o_pstData, off18FD5F09AC2F10A3, uint16_t)
	,	DataPointer(o_pstData, off9A36A04C162108CC, uint16_t)
	,	DataPointer(o_pstData, off23C0B3D895BD8B65, uint16_t)
	);
	*DataPointer(o_pstData, offF13814C20E4B75DA, int8_t) = 100;
	*DataPointer(o_pstData, offF8F6372532BB03C1, uint8_t) = 66;
	*DataPointer(o_pstData, offCAFFBB858893C1EA, uint8_t) = 0;
	{
		static const int16_t rom395071DE165C05F5[6] = { 0, 0, 0, 0, 0, 0 };
		static const uint16_t romC2ACB3C414CB1F13[6] = { 139, 202, 238, 372, 437, 11 };
		static const uint8_t rom8DA209DE22E003E0[6] = { 21, 12, 7, 7, 3, 5 };
		static const uint8_t rom0556386AEC0F6E03[3] = { 7, 7, 7 };
		static const uint8_t romE4B3FBBEB59A0B95[3] = { 25, 25, 25 };
		static const int16_t romA5E1A1E59ABD8CBE[6] = { 0, 0, 0, 0, 0, 0 };
		memcpy(DataPointer(o_pstData, off1879A6019D38C2F7, int16_t), rom395071DE165C05F5, sizeof(int16_t) * 6);
		*DataPointer(o_pstData, off9E9F726287E75ADF, int16_t) = 128;
		memcpy(DataPointer(o_pstData, offB69BE98E3CEA70AF, uint16_t), romC2ACB3C414CB1F13, sizeof(uint16_t) * 6);
		memcpy(DataPointer(o_pstData, offC25C0155B7E54DCB, uint8_t), rom8DA209DE22E003E0, sizeof(uint8_t) * 6);
		memcpy(DataPointer(o_pstData, offD4507F7266F21127, uint8_t), rom0556386AEC0F6E03, sizeof(uint8_t) * 3);
		memcpy(DataPointer(o_pstData, offA217244AFDFA0CE8, uint8_t), romE4B3FBBEB59A0B95, sizeof(uint8_t) * 3);
		memcpy(DataPointer(o_pstData, off482A82FBF5EE0FB7, int16_t), romA5E1A1E59ABD8CBE, sizeof(int16_t) * 6);
		*DataPointer(o_pstData, off478083CE1F7EF677, int16_t) = 0;
	};
};
