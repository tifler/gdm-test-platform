// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#include <stdint.h>

#define  INC(f) <f>
#include INC(CHIP_VENDOR_PATH_ISP_INC_DIR/DxOISP.h)

extern void SetData_OV8820_isp(
	ts_DxOIspAsyncCmd const * const i_pstTuning
,	void * const                    io_pstParams
,	const uint32_t                  i_uiParamsSize
,	void * const                    o_pstData
,	const uint32_t                  i_uiDataSize);

static void (*S_TabFunc_calibData []) (
	ts_DxOIspAsyncCmd const * const i_pstTuning
,	void * const                    io_pstParams
,	const uint32_t                  i_uiParamsSize
,	void * const                    o_pstData
,	const uint32_t                  i_uiDataSize
) = {
	SetData_OV8820_isp
};

void getCalibDataTab(
	uint8_t                         ucCalibrationSelection
,	ts_DxOIspAsyncCmd const * const i_pstTuning
,	void * const                    io_pstParams
,	const uint32_t                  i_uiParamsSize
,	void * const                    o_pstData
,	const uint32_t                  i_uiDataSize
) {
	S_TabFunc_calibData[ucCalibrationSelection](
		i_pstTuning
	,	io_pstParams
	,	i_uiParamsSize
	,	o_pstData
	,	i_uiDataSize);
}

extern const char *getVersion_OV8820_isp (void);

static const char* (*S_versionFunc_calibData []) (void) = {
	getVersion_OV8820_isp
};

const char* getCalibDataVersionTab(
	uint8_t ucCalibrationSelection
) {
	return S_versionFunc_calibData[ucCalibrationSelection]() ;
}
