#include "vpuapi_mme.h"
#include "vpuapifunc.h"

#if 0
typedef struct CodecInst {
	int inUse;			// DO NOT modify line of inUse variable it must be first line of CodecInst structure
	int instIndex;
	int coreIdx;
	int codecMode;
	int codecModeAux;	
	int loggingEnable;
	union {
		EncInfo encInfo;
		DecInfo decInfo;
	} CodecInfo;
} CodecInst;
#endif


int VPU_GetCodecInstanceIndex(void* CodecHdl) {
    struct CodecInst* pCodecInst = (struct CodecInst*)CodecHdl;
    return pCodecInst->instIndex;
}

int VPU_GetCodecInstanceUse(void* CodecHdl) {
    struct CodecInst* pCodecInst = (struct CodecInst*)CodecHdl;
    return pCodecInst->inUse;
}

void VPU_GetCodecInfo(int idx, struct mmp_video_hw_codec_instance_info *p_info) {

    CodecInst * pCodecInst = 0;
    vpu_instance_pool_t *vip;
	vip = (vpu_instance_pool_t *)vdi_get_instance_pool(0);

    pCodecInst = (CodecInst *)vip->codecInstPool[idx];

    p_info->instance_index = idx;
    p_info->instance_max_count = VPU_MAX_NUM_INSTANCE;
    p_info->is_use = pCodecInst->inUse;
    //p_info->is_decoder = 

}

