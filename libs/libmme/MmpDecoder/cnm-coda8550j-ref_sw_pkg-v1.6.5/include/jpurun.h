//------------------------------------------------------------------------------
// File: jpurun.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#ifndef _JPU_RUN_H_
#define _JPU_RUN_H_



#define	MAX_FILE_PATH	256




typedef struct {
	unsigned char * buf;
	int size;
	int point;
	int count;
	int fillendbs;
}JPU_BufInfo;

typedef struct
{
	char yuvFileName[MAX_FILE_PATH];
	char bitstreamFileName[MAX_FILE_PATH];
	char huffFileName[MAX_FILE_PATH];
	char qMatFileName[MAX_FILE_PATH];
	char qpFileName[MAX_FILE_PATH];
	char cfgFileName[MAX_FILE_PATH];
	char yuvTmbFileName[MAX_FILE_PATH];
	int picTmbWidth;
	int picTmbHeight;
	int thumbEn;
	int picWidth;
	int picHeight;
	int rotAngle;
	int mirDir;
	int useRot;
	int mjpgChromaFormat;
	int outNum;
	int instNum;

	int StreamEndian;
	int FrameEndian;
	int chromaInterleave;
    int bEnStuffByte;

	// altek requirement
	int encHeaderMode;


    char strStmDir[MAX_FILE_PATH];
	char strCfgDir[MAX_FILE_PATH];
    int usePartialMode;
    int partialBufNum;
    int partialHeight;
	int packedFormat;
	int RandRotMode;

}JPU_EncConfigParam;

typedef struct
{
	char yuvFileName[MAX_FILE_PATH];
	char bitstreamFileName[MAX_FILE_PATH];
    int rotAngle;
	int mirDir;
	int useRot;
	int outNum;
	int checkeos;
	int instNum;
	int StreamEndian;
	int FrameEndian;
	int chromaInterleave;
    int iHorScaleMode;
	int iVerScaleMode;
	int ThumbNailEn;
	
	//ROI
	int roiEnable;
	int roiWidth;
	int roiHeight;
	int roiOffsetX;
	int roiOffsetY;
	int roiWidthInMcu;
	int roiHeightInMcu;
	int roiOffsetXInMcu;
	int roiOffsetYInMcu;

	//packed
	int packedFormat;
    int picSizeProbing;
    int picWidth;
    int picHeight;
    int picFormat;

    int usePartialMode;
    int partialBufNum;

    int partialHeight;
	int filePlay;
	
}JPU_DecConfigParam;


enum {
	STD_JPG_ENC
};

typedef struct {
	int codecMode;
	int numMulti;
	int saveYuv;
	int  multiMode[JPU_MAX_NUM_INSTANCE];
    char multiFileName[JPU_MAX_NUM_INSTANCE][MAX_FILE_PATH];
	char multiYuvFileName[JPU_MAX_NUM_INSTANCE][MAX_FILE_PATH];	
	JPU_EncConfigParam encConfig[JPU_MAX_NUM_INSTANCE];
	JPU_DecConfigParam decConfig[JPU_MAX_NUM_INSTANCE];
}JPU_MultiConfigParam;

#if defined (__cplusplus)
extern "C" {
#endif
	int JPU_DecodeTest(JPU_DecConfigParam *param);
	int JPU_EncodeTest(JPU_EncConfigParam *param);	
	int JPU_MultiInstanceTest(JPU_MultiConfigParam	*param);
#if defined (__cplusplus)
}
#endif
#endif	/* _JPU_RUN_H_ */
