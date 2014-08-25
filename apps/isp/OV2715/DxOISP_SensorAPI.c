// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#include "DxOISP_SensorAPI.h"
#include "sensorOVT2715.h"


static void (*S_TabFunc_Initialize [])(void) = {
	DxOSensor_OVT2715_Initialize
};

static void (*S_TabFunc_Uninitialize [])(void) = {
	DxOSensor_OVT2715_Uninitialize
};

static void (*S_TabFunc_StatusGroupOpen[])(void) = {
	DxOSensor_OVT2715_GetStartGrp
};

static void (*S_TabFunc_StatusGet[])(uint16_t usOffset, uint16_t usSize, void* pBuf) = {
	DxOSensor_OVT2715_Get
};

static void (*S_TabFunc_StatusGroupClose[])(void) = {
	DxOSensor_OVT2715_GetEndGrp
};

static void (*S_TabFunc_CommandGroupOpen[])(void) = {
	DxOSensor_OVT2715_SetStartGrp
};

static void (*S_TabFunc_CommandSet[])(uint16_t usOffset, uint16_t ussize, void* pBuf) = {
	DxOSensor_OVT2715_Set
};

static void (*S_TabFunc_CommandGroupClose[])(void) = {
	DxOSensor_OVT2715_SetEndGrp
};

static void (*S_TabFunc_Fire[])(void) = {
	DxOSensor_OVT2715_Fire
};

void DxOISP_SensorInitialize(
	uint8_t     ucSensorId
) {
	S_TabFunc_Initialize[ucSensorId]();
}

void DxOISP_SensorUninitialize(
	uint8_t     ucSensorId
) {
	S_TabFunc_Uninitialize[ucSensorId]();
} 

void DxOISP_SensorStatusGroupOpen(
	uint8_t    ucSensorId
) {
	S_TabFunc_StatusGroupOpen[ucSensorId]();
} 

void DxOISP_SensorStatusGet(
	uint8_t     ucSensorId
,	uint16_t    usOffset
,	uint16_t    usSize
,	void      * pBuf
) {
	S_TabFunc_StatusGet[ucSensorId](usOffset, usSize, pBuf);
} 

void DxOISP_SensorStatusGroupClose(
	uint8_t     ucSensorId
) {
	S_TabFunc_StatusGroupClose[ucSensorId]();
} 

void DxOISP_SensorCommandGroupOpen(
	uint8_t     ucSensorId
) {
	S_TabFunc_CommandGroupOpen[ucSensorId]();
}

void DxOISP_SensorCommandSet(
	uint8_t     ucSensorId
,	uint16_t    usOffset
,	uint16_t    usSize
,	void      * pBuf
) {
	S_TabFunc_CommandSet[ucSensorId](usOffset, usSize, pBuf);
}

void DxOISP_SensorCommandGroupClose(
	uint8_t     ucSensorId
) {	
	S_TabFunc_CommandGroupClose[ucSensorId]();
}

void DxOISP_SensorFire(
	uint8_t     ucSensorId
) {	
	S_TabFunc_Fire[ucSensorId]();
} 
