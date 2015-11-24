#include <stdint.h>

extern void* getCCParams_OV8820             (void);

static void* (*S_TabFunc_CCparams []) (void) = {
	getCCParams_OV8820
};

void* getCCParamTab(uint8_t ucCalibrationSelection)
{
	return S_TabFunc_CCparams[ucCalibrationSelection]();
}
