#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include "DxOISP.h"

void none_sensor_initialize(void)
{
	return;
}

void none_sensor_get ( uint16_t usOffset, uint16_t usSize, void* pBuf )
{
	usOffset = usSize;
	return;
}

/* EOF */

