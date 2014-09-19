#ifndef MME_JPU_COMPILEHELPER_H__
#define MME_JPU_COMPILEHELPER_H__

#define MAX_NUM_INSTANCE JPU_MAX_NUM_INSTANCE

#define CBCR_SEPARATED   JPU_CBCR_SEPARATED
#define CBCR_INTERLEAVE  JPU_CBCR_INTERLEAVE
#define CRCB_INTERLEAVE  JPU_CRCB_INTERLEAVE

#define FORMAT_224       JPU_FORMAT_224
#define FORMAT_400       JPU_FORMAT_400
#define FORMAT_420       JPU_FORMAT_420
#define FORMAT_422       JPU_FORMAT_422
#define FORMAT_444       JPU_FORMAT_444

typedef JPU_FrameBuffer FrameBuffer;

#define MIRDIR_NONE JPU_MIRDIR_NONE
#define MIRDIR_VER  JPU_MIRDIR_VER
#define MIRDIR_HOR  JPU_MIRDIR_HOR
#define MIRDIR_HOR_VER JPU_MIRDIR_HOR_VER

#define ENABLE_LOGGING JPU_ENABLE_LOGGING
#define DISABLE_LOGGING  JPU_DISABLE_LOGGING

#define EncConfigParam JPU_EncConfigParam
#define ENC_CFG JPU_ENC_CFG
#define BufInfo JPU_BufInfo

#define API_VERSION JPU_API_VERSION

#endif
