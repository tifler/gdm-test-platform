// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#ifndef __DxOISP_CALIB_DATA_SELECTION_H__
#define __DxOISP_CALIB_DATA_SELECTION_H__

#include <stdint.h>
#ifndef INC
#define  INC(f) <f>
#endif
#include INC(CHIP_VENDOR_PATH_ISP_INC_DIR/DxOISP.h)

void getCalibDataTab(
	uint8_t                         ucCalibrationSelection
,	ts_DxOIspAsyncCmd const * const i_pstTuning
,	void * const                    io_pstParams
,	const uint32_t                  i_uiParamsSize
,	void * const                    o_pstData
,	const uint32_t                  i_uiDataSize
);

const char* getCalibDataVersionTab(
	uint8_t ucCalibrationSelection
);

void* getCCParamTab(
	uint8_t ucCalibrationSelection
);

#endif //__DxOISP_CALIB_DATA_SELECTION_H__
