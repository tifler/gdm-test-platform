#ifndef __GISP_SENSOR_H__
#define __GISP_SENSOR_H__

#include <stdint.h>
#include "debug.h"
#ifdef  DXO_SENSOR_DYNAMIC
#include "list.h"
#endif  /*DXO_SENSOR_DYNAMIC*/

/*****************************************************************************/

enum {
    SENSOR_ID_FRONT,
    SENSOR_ID_BACK,
    SENSOR_ID_COUNT,
};

#define  ISP_PRINTF                         DBG

/*****************************************************************************/

struct SENSOR;

struct SENSOR_API {
    void (*init)(struct SENSOR *sensor);
    void (*exit)(struct SENSOR *sensor);
    void (*commandGroupOpen)(struct SENSOR *sensor);
    void (*commandSet)(struct SENSOR *sensor,
            uint16_t offset, uint16_t size, void *buffer);
    void (*commandGroupClose)(struct SENSOR *sensor);
    void (*statusGroupOpen)(struct SENSOR *sensor);
    void (*statusGet)(struct SENSOR *sensor,
            uint16_t offset, uint16_t size, void *buffer);
    void (*statusGroupClose)(struct SENSOR *sensor);
    void (*fire)(struct SENSOR *sensor);
};

struct SENSOR {
    const char *name;
    struct SENSOR_API *api;
    void *priv;
#ifdef  DXO_SENSOR_DYNAMIC
    void *module;
    struct list_head entry;
#endif  /*DXO_SENSOR_DYNAMIC*/
};

#endif  /*__GISP_SENSOR_H__*/
