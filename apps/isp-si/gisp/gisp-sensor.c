#include "DxOISP_SensorAPI.h"
#include "gisp-sensor.h"
#include "OV2715/sensorOVT2715.h"
#include "VIGEN/vigen.h"
#include "debug.h"

/*****************************************************************************/

static struct SENSOR *dxoSensors[SENSOR_ID_COUNT] = {
#ifndef DXO_SENSOR_DYNAMIC
    [SENSOR_ID_OVT2715] = &OVT2715_Sensor,
    [SENSOR_ID_VIGEN] = &VIGEN_Sensor,
#endif  /*DXO_SENSOR_DYNAMIC*/
};

/*****************************************************************************/

#ifdef  DXO_SENSOR_DYNAMIC
static int isDynamic = 1;
#else
static int isDynamic = 0;
#endif  /*DXO_SENSOR_DYNAMIC*/

static inline struct SENSOR *getSensor(uint8_t sensorId)
{
    if (isDynamic) {
        ASSERT(! "Dynamic sensor loading is *NOT* implemented");
    }

    return dxoSensors[sensorId];
}

/*****************************************************************************/

void DxOISP_SensorInitialize(uint8_t sensorId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    DBG("=====>> SensorInit(%u) <<=====", sensorId);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->init);
    sensor->api->init(sensor);
}

void DxOISP_SensorUninitialize(uint8_t sensorId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->exit);
    sensor->api->exit(sensor);
}

void DxOISP_SensorCommandGroupOpen(uint8_t sensorId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->commandGroupOpen);
    sensor->api->commandGroupOpen(sensor);
}

void DxOISP_SensorCommandSet(
        uint8_t sensorId, uint16_t offset, uint16_t size, void *buffer)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->commandSet);
    sensor->api->commandSet(sensor, offset, size, buffer);
}

void DxOISP_SensorCommandGroupClose(uint8_t sensorId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->commandGroupClose);
    sensor->api->commandGroupClose(sensor);
}

void DxOISP_SensorStatusGroupOpen(uint8_t sensorId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->statusGroupOpen);
    sensor->api->statusGroupOpen(sensor);
}

void DxOISP_SensorStatusGet(
        uint8_t sensorId, uint16_t offset, uint16_t size, void *buffer)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->statusGet);
    sensor->api->statusGet(sensor, offset, size, buffer);
}

void DxOISP_SensorStatusGroupClose(uint8_t sensorId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->statusGroupClose);
    sensor->api->statusGroupClose(sensor);
}

void DxOISP_SensorFire(uint8_t sensorId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->fire);
    sensor->api->fire(sensor);
}

/*****************************************************************************/

int SensorGetMode(uint8_t sensorId, uint32_t modeId, struct SENSOR_MODE *mode)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->getSensorMode);
    return sensor->api->getSensorMode(modeId, mode);
}

void SensorSetMode(uint8_t sensorId, uint32_t modeId)
{
    struct SENSOR *sensor;
    ASSERT(sensorId < SENSOR_ID_COUNT);
    sensor = getSensor(sensorId);
    ASSERT(sensor);
    ASSERT(sensor->api);
    ASSERT(sensor->api->setSensorMode);
    sensor->api->setSensorMode(modeId);
}

#ifdef  DXO_SENSOR_DYNAMIC
// *NOT* Completed
typedef void (*ModuleInitFunc_t)(struct SENSOR *sensor);

void LoadSensorModule(uint8_t sensorId, const char *path)
{
    struct SENSOR *sensor;
    ModuleInitFunc_t initFunc;

    ASSERT(sensorId < SENSOR_ID_COUNT);

    sensor = dxoSensors[sensorId];
    ASSERT(sensor);

    sensor->id = sensorId;
    sensor->module = dlopen(path, RTLD_LAZY);
    ASSERT(sensor->module);

    initFunc = (ModuleInitFunc_t)dlsym(sensor->module, "initSensorModule");
    ASSERT(initFunc);

    initFunc(sensor);
}
#endif  /*DXO_SENSOR_DYNAMIC*/

