//------------------------------------------------------------------------------
// File: vpurun.c
//
// Copyright (c) 2006~2010, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vpuapi.h"
#include "regdefine.h"
#include "mixer.h"
#include "vpuio.h"
#include "vpuhelper.h"
#include "vpurun.h"
#include "vpuapifunc.h"
#ifdef SUPPORT_FFMPEG_DEMUX
#include "theora_parser.h"
#endif

#ifdef SUPPORT_FFMPEG_DEMUX 
#include <libavformat/avformat.h> 
#include <libavcodec/avcodec.h>
#endif

//#define ENC_SOURCE_FRAME_DISPLAY

#define VPU_ENC_TIMEOUT       1000
#define VPU_DEC_TIMEOUT       1000
#define VPU_WAIT_TIME_OUT	100		//should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
#define PARALLEL_VPU_WAIT_TIME_OUT 0 	//the value of timeout is 0 means we just check interrupt flag. do not wait any time to give a chance of an interrupt of the next core.


#if PARALLEL_VPU_WAIT_TIME_OUT > 0 
#undef VPU_DEC_TIMEOUT
#define VPU_DEC_TIMEOUT       1000
#endif


#define MAX_CHUNK_HEADER_SIZE 1024
#define MAX_DYNAMIC_BUFCOUNT	3
#define NUM_FRAME_BUF			19
#define MAX_ROT_BUF_NUM			2
#define EXTRA_FRAME_BUFFER_NUM	1

#define ENC_SRC_BUF_NUM			2
#define STREAM_BUF_SIZE		 0x300000  // max bitstream size

//#define STREAM_FILL_SIZE    (512 * 16)  //  4 * 1024 | 512 | 512+256( wrap around test )
#define STREAM_FILL_SIZE    0x2000  //  4 * 1024 | 512 | 512+256( wrap around test )

#define STREAM_END_SIZE			0
#define STREAM_END_SET_FLAG		0
#define STREAM_END_CLEAR_FLAG	-1
#define STREAM_READ_SIZE    (512 * 16)

//#define SUPPORT_SW_MIXER


#define FORCE_SET_VSYNC_FLAG
//#define TEST_USER_FRAME_BUFFER
#ifdef TEST_USER_FRAME_BUFFER
//#define TEST_MULTIPLE_CALL_REGISTER_FRAME_BUFFER
#endif


int DecodeTest(DecConfigParam *param)
{

	DecHandle		handle		= {0};
	DecOpenParam	decOP		= {0};
	DecInitialInfo	initialInfo = {0};
	DecOutputInfo	outputInfo	= {0};
	DecParam		decParam	= {0};
	BufInfo			bufInfo	 = {0};
	vpu_buffer_t	vbStream	= {0};
	FrameBuffer		fbPPU[MAX_ROT_BUF_NUM];
	FrameBufferAllocInfo fbAllocInfo;
	FrameBuffer		dispFrame = {0};
	SecAxiUse		secAxiUse = {0};
	RetCode			ret = RETCODE_SUCCESS;		
	int			bsSize=0, framebufSize=0, framebufWidth=0, framebufHeight=0, rotbufWidth=0, rotbufHeight=0, framebufFormat = FORMAT_420, mapType;
	int				i = 0, frameIdx = 0, ppIdx = 0, totalNumofErrMbs = 0, framebufStride = 0, streameos = 0, saveImage = 0, decodefinish = 0, decodeIdx=0;
	int				key = 0, rotStride = 0, suc = 1;
	int			 randomAccess = 0, randomAccessPos = 0, randomeAccessFlush = 0;
	BYTE			*pFileBuf = NULL;
	Uint8 *			pYuv	 =	NULL;
	osal_file_t		fchunkData =	NULL;
	osal_file_t		fpYuv	 =	NULL;
	int				kbhitRet = 0;
	int				regFrameBufCount = 0;
	int				dispDoneIdx = -1;
	int				seqInited = 0, seqInitEscape = 0;
	int				ppuEnable = 0;
	int				timeout_count = 0;
	int				int_reason = 0;
	int				instIdx, coreIdx;
//#define TEST_SEQUENCE_CHANGE_BY_HOST
#ifdef TEST_SEQUENCE_CHANGE_BY_HOST
	int	seqChangeRequest = 0;
	int seqChangedStreamEndFlag;
	PhysicalAddress seqChangedRdPtr;
	PhysicalAddress seqChangedWrPtr;
#endif
	ImageFormat imageFormat = YUV_FORMAT_I420;	
	Rect			rcPrevDisp;
	TiledMapConfig mapCfg = {0};
	DRAMConfig dramCfg = {0};
	vpu_buffer_t vbFrame[MAX_REG_FRAME] = {0,};
	FrameBuffer  fbUser[MAX_REG_FRAME]={0,};
	FrameBuffer  *pUserFrame = NULL;
	MaverickCacheConfig decCacheConfig;
	DecConfigParam decConfig;
	frame_queue_item_t* display_queue = NULL;
	

	memcpy(&decConfig, param, sizeof(DecConfigParam));

	coreIdx = decConfig.coreIdx;
	instIdx = decConfig.instNum;

	if(strlen(decConfig.yuvFileName))
		saveImage = 1;
	else
		saveImage = 0;

	fchunkData = osal_fopen(decConfig.bitstreamFileName, "rb");
	if( !fchunkData ) 
	{
		VLOG(ERR, "Can't open %s \n", decConfig.bitstreamFileName );
		goto ERR_DEC_INIT;
	}

	if (strlen(decConfig.yuvFileName)) 
	{
		fpYuv = osal_fopen(decConfig.yuvFileName, "wb");
		if (!fpYuv) 
		{
			VLOG(ERR, "Can't open %s \n", decConfig.yuvFileName );
			goto ERR_DEC_INIT;
		}		
	}


	osal_fseek(fchunkData, 0, SEEK_END);
	bsSize = osal_ftell(fchunkData);
	osal_fseek(fchunkData, 0, SEEK_SET);

	pFileBuf = osal_malloc( STREAM_FILL_SIZE );
	if (!pFileBuf)
		goto ERR_DEC_INIT;

	bufInfo.fp = (void *)fchunkData;

	bufInfo.buf = pFileBuf;
	bufInfo.size = bsSize;
	bufInfo.point = 0; 
	bufInfo.random_access_point = -1;// random access point



	ret = VPU_Init(coreIdx);
#ifdef BIT_CODE_FILE_PATH
#else
	if (ret == RETCODE_NOT_FOUND_BITCODE_PATH)
	{
		osal_file_t fpBitCode = NULL;
		Uint16 *pusBitCode;
		if (coreIdx == 0)
			fpBitCode = osal_fopen(CORE_0_BIT_CODE_FILE_PATH, "rb");
		else if (coreIdx == 1)
			fpBitCode = osal_fopen(CORE_1_BIT_CODE_FILE_PATH, "rb");
		if (fpBitCode)
		{
			pusBitCode = (Uint16 *)osal_malloc(CODE_BUF_SIZE);
			if (pusBitCode)
			{
				int count = 0;
				int code;
				while (!osal_feof(fpBitCode) || count < (CODE_BUF_SIZE/2)) {
					code = -1;
					if (osal_fscanf(fpBitCode, "%x", &code) <= 0) {
						/* matching failure or EOF */
						break;
					}
					pusBitCode[count++] = (Uint16)code;
				}
				ret = VPU_InitWithBitcode(coreIdx, pusBitCode, count);
				osal_free(pusBitCode);
			}
			osal_fclose(fpBitCode);
		}
		else
		{
			VLOG(ERR, "failed open bit_firmware file path is %s\n", CORE_0_BIT_CODE_FILE_PATH);
			goto ERR_DEC_INIT;
		}
	}
#endif
	if( ret != RETCODE_SUCCESS &&
		ret != RETCODE_CALLED_BEFORE )
	{
		suc = 0;
		VLOG(ERR, "VPU_Init failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}

	CheckVersion(coreIdx);

	if (decConfig.bitFormat > STD_VP8)
	{
		VLOG(ERR, "Invalid bitstream format mode \n" );
		goto ERR_DEC_INIT;
	}

	decOP.bitstreamFormat = decConfig.bitFormat;
	if (decOP.bitstreamFormat == STD_AVC)
		decOP.avcExtension = decConfig.avcExtension&0x1;

	vbStream.size	 = STREAM_BUF_SIZE;
	if (vdi_allocate_dma_memory(coreIdx, &vbStream) < 0)
	{
		VLOG(ERR, "fail to allocate bitstream buffer\n" );
		goto ERR_DEC_INIT;
	}

	decOP.coreIdx = coreIdx;
	decOP.bitstreamBuffer = vbStream.phys_addr;
	decOP.bitstreamBufferSize = vbStream.size;
	decOP.mp4DeblkEnable = decConfig.mp4DeblkEnable;
	decOP.mp4Class = decConfig.mpeg4Class;
	decOP.tiled2LinearEnable = (decConfig.mapType>>4)&0x1;	
	mapType = (decConfig.mapType & 0xf);
	decOP.bitstreamMode = decConfig.bitstreamMode;

	if (mapType)
	{
		//coda980 only
		decOP.wtlEnable = decConfig.wtlEnable;

        if (decOP.wtlEnable)
        {
            decConfig.rotAngle = 0;
            decConfig.mirDir = 0;
            decConfig.useRot = 0;
            decConfig.useDering = 0;
			decOP.mp4DeblkEnable = 0;
            decOP.tiled2LinearEnable = 0;
        }
	}
	decOP.cbcrInterleave = CBCR_INTERLEAVE;
	if (mapType == TILED_FRAME_MB_RASTER_MAP ||
		mapType == TILED_FIELD_MB_RASTER_MAP) {
			decOP.cbcrInterleave = 1;
	}
	decOP.bwbEnable	  = VPU_ENABLE_BWB;
	if (mapType && !decOP.tiled2LinearEnable)
	//if (mapType && !decOP.tiled2LinearEnable && !decOP.wtlEnable) //BWB could use for WTL.
		decOP.bwbEnable = 0; // if tiledmap is used. it would be better of performance to disable bwb
	decOP.frameEndian	= VPU_FRAME_ENDIAN;
	decOP.streamEndian   = VPU_STREAM_ENDIAN;

	if (decConfig.useRot || decConfig.useDering || decOP.tiled2LinearEnable) 
		ppuEnable = 1;
	else
		ppuEnable = 0;


	ret = VPU_DecOpen(&handle, &decOP);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}
	
	//VPU_DecGiveCommand(handle, ENABLE_LOGGING, 0);

	ret = WriteBsBufHelper(coreIdx, handle, &bufInfo, &vbStream, STREAM_FILL_SIZE, decConfig.checkeos, &streameos, decOP.streamEndian);
	if (ret != RETCODE_SUCCESS)
	{
		VLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}

	//ConfigSeqReport(coreIdx, handle, decOP.bitstreamFormat);
	if (decOP.bitstreamFormat == AVC_DEC) 
	{
		ret = VPU_DecGiveCommand(handle, SET_LOW_DELAY_CONFIG, &decConfig.lowDelayInfo);
		if (ret != RETCODE_SUCCESS)
		{
			VLOG(ERR, "VPU_DecGiveCommand[SET_LOW_DELAY_CONFIG] failed Error code is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}


	}

//#define TEST_DRAM_CONFIG
#ifdef TEST_DRAM_CONFIG
	dramCfg.casBit = EM_CAS;
	dramCfg.rasBit = EM_RAS;
	dramCfg.bankBit = EM_BANK;
	dramCfg.busBit = EM_WIDTH;
	ret = VPU_DecGiveCommand(handle, SET_DRAM_CONFIG, &dramCfg);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_DecGiveCommand[SET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
#endif	//TEST_DRAM_CONFIG

	ret = VPU_DecGiveCommand(handle, GET_DRAM_CONFIG, &dramCfg);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}

	if (decConfig.reorderDisable == 1)
    {
        ret = VPU_DecGiveCommand(handle, DEC_DISABLE_REORDER, 0);
        if( ret != RETCODE_SUCCESS )
        {
            VLOG(ERR, "VPU_DecGiveCommand[DEC_DISABLE_REORDER] failed Error code is 0x%x \n", ret );
            goto ERR_DEC_OPEN;
        }
    }


//#define TEST_ENABLE_DEC_THUMBNAIL_MODE
#ifdef TEST_ENABLE_DEC_THUMBNAIL_MODE
	VPU_DecGiveCommand(handle, ENABLE_DEC_THUMBNAIL_MODE, 0);
#endif

//#define  TEST_SELECT_AVC_ERROR_CONCEAL_MODE
#ifdef TEST_SELECT_AVC_ERROR_CONCEAL_MODE
	if (decOP.bitstreamMode == STD_AVC)
	{
		AVCErrorConcealMode mode = (AVC_ERROR_CONCEAL_MODE_DISABLE_CONCEAL_MISSING_REFERENCE | AVC_ERROR_CONCEAL_MODE_DISABLE_CONCEAL_WRONG_FRAME_NUM);
		VPU_DecGiveCommand(handle, DEC_SET_AVC_ERROR_CONCEAL_MODE, &mode);
	}
#endif

VPU_SEQ_INIT:
	seqInitEscape = 0;

	ret = VPU_DecSetEscSeqInit(handle, seqInitEscape);
	if (ret != RETCODE_SUCCESS)
	{
		seqInitEscape = 0;
		VLOG(WARN, "Wanning! can not to set seqInitEscape in the current bitstream mode Option \n");		
	}

	if (seqInitEscape)
	{
		// if you set to seqInitEscape option to TRUE, It is more easy to control that APP uses VPU_DecGetInitialInfo instead of VPU_DecIssueSeqInit and VPU_DecCompleteSeqInit
		ret = VPU_DecGetInitialInfo(handle, &initialInfo);
		if (ret != RETCODE_SUCCESS) 
		{
#ifdef SUPPORT_MEM_PROTECT
			if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
				PrintMemoryAccessViolationReason(coreIdx, NULL);
#endif
			VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret);
			VLOG(ERR, "Error reason is 0x%x\n", initialInfo.seqInitErrReason);
			goto ERR_DEC_OPEN;					
		}
		VPU_ClearInterrupt(coreIdx);
	}
	else
	{
		ret = VPU_DecIssueSeqInit(handle);
		if (ret != RETCODE_SUCCESS)
		{
			VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
			goto ERR_DEC_OPEN;
		}
		timeout_count = 0;
		while((kbhitRet = osal_kbhit()) == 0) 
		{	
			int_reason = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);		//wait for 10ms to save stream filling time.
			if (int_reason == (Uint32)-1) // timeout
			{
				if (timeout_count*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) 
				{
					VLOG(WARN, "\n VPU interrupt wait timeout instIdx=%d, PC=0x%x\n", instIdx, VpuReadReg(coreIdx, BIT_CUR_PC));
					//VLOG(WARN, "\n VPU interrupt wait timeout instIdx=%d\n", instIdx);
					VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
					break;
				}
				timeout_count++;
				int_reason = 0;									
			}

			CheckUserDataInterrupt(coreIdx, handle, outputInfo.indexFrameDecoded, decOP.bitstreamFormat, int_reason);

			if (int_reason)
			{
				VPU_ClearInterrupt(coreIdx);
				if (int_reason & (1<<INT_BIT_SEQ_INIT)) 
				{
					seqInited = 1;
					break;
				}
			}

			ret = WriteBsBufHelper(coreIdx, handle, &bufInfo, &vbStream, STREAM_FILL_SIZE, decConfig.checkeos, &streameos, decOP.streamEndian);
			if (ret != RETCODE_SUCCESS) 
			{
				VLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
				goto ERR_DEC_OPEN;
			}
		}

		if (seqInited)
		{
			ret = VPU_DecCompleteSeqInit(handle, &initialInfo);	
			if (ret != RETCODE_SUCCESS)
			{
#ifdef SUPPORT_MEM_PROTECT
				if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
				{
					PrintMemoryAccessViolationReason(coreIdx, NULL);
					goto ERR_DEC_OPEN;
				}
#endif
				if (decOP.bitstreamMode == BS_MODE_ROLLBACK && initialInfo.seqInitErrReason&(1<<31)) // this happens only ROLLBACK mode case
				{
					VLOG(WARN, "Seq init : Feed more stream\n");
					seqInited = 0;
					ret = WriteBsBufHelper(coreIdx, handle, &bufInfo, &vbStream, STREAM_FILL_SIZE, decConfig.checkeos, &streameos, decOP.streamEndian);
					if (ret != RETCODE_SUCCESS) 
					{
						VLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
						goto ERR_DEC_OPEN;
					}
					goto VPU_SEQ_INIT;
				}
				else if (decOP.bitstreamMode == BS_MODE_PIC_END && initialInfo.seqInitErrReason&(1<<31)) 
				{
					VLOG(ERR, "There are no header\n");
					goto ERR_DEC_OPEN;
				}
				else
				{
					VLOG(ERR, "VPU_DecCompleteSeqInit failed Error code is 0x%x \n", ret );
					VLOG(ERR, "Error reason is 0x%x \n", initialInfo.seqInitErrReason );
					goto ERR_DEC_OPEN;
				}
			}			
		}
		else
		{
			VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
			goto ERR_DEC_OPEN;
		}
	}

	framebufWidth = ((initialInfo.picWidth+15)&~15);
	if (decConfig.maxWidth) 
	{
		framebufWidth  = ((decConfig.maxWidth+15)&~15);
		if (framebufWidth <  ((initialInfo.picWidth+15)&~15))
		{
			VLOG(ERR, "maximum width is too small\n");
			goto ERR_DEC_OPEN;
		}
	}


	if (IsSupportInterlaceMode(decOP.bitstreamFormat, &initialInfo))
		framebufHeight = ((initialInfo.picHeight+31)&~31); // framebufheight must be aligned by 31 because of the number of MB height would be odd in each filed picture.
	else
		framebufHeight = ((initialInfo.picHeight+15)&~15);
	if (decConfig.maxHeight) 
	{
		if (IsSupportInterlaceMode(decOP.bitstreamFormat, &initialInfo))
		{
			framebufHeight = ((decConfig.maxHeight+31)&~31);
			if (framebufHeight <  ((initialInfo.picHeight+31)&~31))
			{
				VLOG(ERR, "maximum height is too small\n");
				goto ERR_DEC_OPEN;
			}
		}
		else
		{
			framebufHeight = ((decConfig.maxHeight+15)&~15);
			if (framebufHeight <  ((initialInfo.picHeight+15)&~15))
			{
				VLOG(ERR, "maximum height is too small\n");
				goto ERR_DEC_OPEN;
			}
		}
	} 

	if (IsSupportInterlaceMode(decOP.bitstreamFormat, &initialInfo)) 
	{
		rotbufWidth = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? ((initialInfo.picHeight+31)&~31) : ((initialInfo.picWidth+15)&~15);
		rotbufHeight = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? ((initialInfo.picWidth+15)&~15) : ((initialInfo.picHeight+31)&~31);
	}
	else
	{
		rotbufWidth = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? ((initialInfo.picHeight+15)&~15) : ((initialInfo.picWidth+15)&~15);
		rotbufHeight = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? ((initialInfo.picWidth+15)&~15) : ((initialInfo.picHeight+15)&~15);
	}

	rotStride = rotbufWidth;

	framebufFormat = FORMAT_420;




	//SaveSeqReport(coreIdx, handle, &initialInfo, decOP.bitstreamFormat);	

	framebufStride = framebufWidth;

	framebufSize = VPU_GetFrameBufSize(framebufStride, framebufHeight, mapType, framebufFormat, &dramCfg);

	VLOG(INFO, "* Dec InitialInfo =>\n instance idx: #%d, \n open instance num: #%d, \n minframeBuffercount: %u\n ", instIdx, VPU_GetOpenInstanceNum(coreIdx), initialInfo.minFrameBufferCount);
	VLOG(INFO, "picWidth: %u\n picHeight: %u\n ",initialInfo.picWidth, initialInfo.picHeight); 
	if (initialInfo.fRateDenominator)
		VLOG(INFO, "frameRate: %.2f\n ",(double)(initialInfo.fRateNumerator/(initialInfo.fRateDenominator)) );
	VLOG(INFO, "frRes: %u\n frDiv: %u\n",initialInfo.fRateNumerator, initialInfo.fRateDenominator);
	VLOG(INFO, "framebuffer format: %d, framebuffer size = %d\n", framebufFormat, framebufSize);

#ifdef CNM_FPGA_PLATFORM
	InitMixerInt();
#endif
	init_VSYNC_flag();

#ifdef SUPPORT_SW_MIXER
 	sw_mixer_close((coreIdx*MAX_NUM_VPU_CORE)+instIdx);
	if (!ppuEnable)
		sw_mixer_open((coreIdx*MAX_NUM_VPU_CORE)+instIdx, framebufStride, framebufHeight);
	else
		sw_mixer_open((coreIdx*MAX_NUM_VPU_CORE)+instIdx, rotStride, rotbufHeight);	
#endif

	pYuv = osal_malloc(framebufSize);
	if (!pYuv)
	{
		VLOG(ERR, "Fail to allocation memory for display buffer\n");
		goto ERR_DEC_OPEN;
	}
	secAxiUse.useBitEnable  = USE_BIT_INTERNAL_BUF;
	secAxiUse.useIpEnable   = USE_IP_INTERNAL_BUF;
	secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	secAxiUse.useBtpEnable  = USE_BTP_INTERNAL_BUF;
	secAxiUse.useOvlEnable  = USE_OVL_INTERNAL_BUF;

	VPU_DecGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

	// MaverickCache configure
	MaverickCache2Config(
		&decCacheConfig, 
		1, //decoder
		decOP.cbcrInterleave, // cb cr interleave
		decConfig.frameCacheBypass,
		decConfig.frameCacheBurst,
		decConfig.frameCacheMerge,
		mapType,
		decConfig.frameCacheWayShape);
	VPU_DecGiveCommand(handle, SET_CACHE_CONFIG, &decCacheConfig);
	regFrameBufCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;

#ifdef TEST_USER_FRAME_BUFFER
	decConfig.userFbAlloc = 1;
#endif
	if (decConfig.userFbAlloc) // if HOST wants to allocate framebuffer outside of API
	{
		fbAllocInfo.format          = framebufFormat;
		fbAllocInfo.cbcrInterleave  = decOP.cbcrInterleave;
		fbAllocInfo.mapType         = mapType;
		fbAllocInfo.stride          = framebufStride;
		fbAllocInfo.height          = framebufHeight;
		fbAllocInfo.num             = regFrameBufCount;
		
		fbAllocInfo.endian          = decOP.frameEndian;
		fbAllocInfo.type            = FB_TYPE_CODEC;
		// if HOST has a frame buffer which is allocated at other block such as render mode.  HOST can set it to VPU without frame buffer allocation in API		
		framebufSize = VPU_GetFrameBufSize(framebufStride, framebufHeight, fbAllocInfo.mapType, framebufFormat, &dramCfg);
		for (i=0; i<fbAllocInfo.num; i++)
		{
			vbFrame[i].size = framebufSize;
			if (vdi_allocate_dma_memory(coreIdx, &vbFrame[i]) < 0)
			{
				VLOG(ERR, "fail to allocate frame buffer\n" );
				goto ERR_DEC_OPEN;
			}
			fbUser[i].bufY = vbFrame[i].phys_addr;
			fbUser[i].bufCb = -1;
			fbUser[i].bufCr = -1;
		}
		ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, fbUser);
		if( ret != RETCODE_SUCCESS )
		{
			VLOG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}
		if (decOP.wtlEnable)
		{
			fbAllocInfo.mapType = LINEAR_FRAME_MAP;

			framebufSize = VPU_GetFrameBufSize(framebufStride, framebufHeight, fbAllocInfo.mapType, framebufFormat, &dramCfg);
			for (i=regFrameBufCount; i<regFrameBufCount*2; i++)
			{
				vbFrame[i].size = framebufSize;
				if (vdi_allocate_dma_memory(coreIdx, &vbFrame[i]) < 0)
				{
					VLOG(ERR, "fail to allocate frame buffer\n" );
					goto ERR_DEC_OPEN;
				}
				fbUser[i].bufY = vbFrame[i].phys_addr;
				fbUser[i].bufCb = -1;
				fbUser[i].bufCr = -1;
			}

			ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, &fbUser[regFrameBufCount]);
			if( ret != RETCODE_SUCCESS )
			{
				VLOG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
				goto ERR_DEC_OPEN;
			}
		}


		pUserFrame = (FrameBuffer *)fbUser;
	}
	
#ifdef TEST_SET_FRAME_DELAY
#define NUM_FRAME_DELAY	0
	decConfig.frameDelay = NUM_FRAME_DELAY;
	VPU_DecGiveCommand(handle, DEC_SET_FRAME_DELAY, &decConfig.frameDelay);
#endif


	ret = VPU_DecRegisterFrameBuffer(handle, pUserFrame, regFrameBufCount, framebufStride, framebufHeight, mapType); // frame map type (can be changed before register frame buffer)
	if( ret != RETCODE_SUCCESS )
	{
#ifdef SUPPORT_MEM_PROTECT
		if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
			PrintMemoryAccessViolationReason(coreIdx, NULL);
#endif
		VLOG(ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
	ret = VPU_DecGiveCommand(handle, GET_TILEDMAP_CONFIG, &mapCfg);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_DecGiveCommand[GET_TILEDMAP_CONFIG] failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
	if (ppuEnable) 
	{
		fbAllocInfo.format          = framebufFormat;
		fbAllocInfo.cbcrInterleave  = decOP.cbcrInterleave;
		if (decOP.tiled2LinearEnable)
			fbAllocInfo.mapType = LINEAR_FRAME_MAP;
		else
			fbAllocInfo.mapType = mapType;
	
		fbAllocInfo.stride = rotStride;
		fbAllocInfo.height = rotbufHeight;
		fbAllocInfo.num    = MAX_ROT_BUF_NUM;
		fbAllocInfo.endian = decOP.frameEndian;
		fbAllocInfo.type   = FB_TYPE_PPU;
	
		ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, fbPPU);
		if( ret != RETCODE_SUCCESS )
		{
			VLOG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}

		ppIdx = 0;

		if (decConfig.useRot)
		{
			VPU_DecGiveCommand(handle, SET_ROTATION_ANGLE, &(decConfig.rotAngle));
			VPU_DecGiveCommand(handle, SET_MIRROR_DIRECTION, &(decConfig.mirDir));
		}

		VPU_DecGiveCommand(handle, SET_ROTATOR_STRIDE, &rotStride);
	}

	

	decParam.skipframeMode = decConfig.skipframeMode;
	decParam.iframeSearchEnable = decConfig.iframeSearchEnable;
	display_queue = frame_queue_init(MAX_REG_FRAME);

	printf("Dec Start : Press enter key to show menu. instIdx=%d\n", instIdx);
	printf("		  : Press space key to stop.\n" );

	while( 1 )
	{		
#ifdef PLATFORM_LINUX
		usleep(0);
#endif
#ifdef PLATFORM_WIN32
		Sleep(0);
#endif
		if (osal_kbhit())
		{
			key = osal_getch();
			osal_flush_ch();
		}
		if (key)
		{
			if (key == '\r' || key == '\n')
			{
				ret = UI_GetUserCommand(coreIdx, handle, &decParam, &randomAccessPos);
				if (ret == UI_CMD_SKIP_FRAME_LIFECYCLE)
					continue;
				else if (ret == UI_CMD_RANDOM_ACCESS)
				{
					randomAccess = 1;
					decParam.iframeSearchEnable = 1;
				}
				key = 0;
                if (decParam.skipframeMode)
                {
                    //clear all frame buffer 
                    frame_queue_dequeue_all(display_queue);

                    for(i=0; i<regFrameBufCount; i++)
                        VPU_DecClrDispFlag(handle, i);

					//can clear all display buffer before flushing a sync frame if HOST wants clearing all remained frame in buffer.
                    ret = VPU_DecFrameBufferFlush(handle);
                    if( ret != RETCODE_SUCCESS )
                    {
                        VLOG(ERR, "VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n", ret );
                        goto ERR_DEC_OPEN;
                    }
					
                    ret = VPU_DecGiveCommand(handle, DEC_DISABLE_SKIP_REORDER, 0);
                    if( ret != RETCODE_SUCCESS )
                    {
                        VLOG(ERR, "VPU_DecGiveCommand(DEC_DISABLE_SKIP_REORDER) failed Error code is 0x%x \n", ret );
                        goto ERR_DEC_OPEN;
                    }

                }
			}
			else if (key == ' ')
				break;
		}

		if (randomeAccessFlush == 0 && randomAccess)
		{
			//assume display done
			set_VSYNC_flag();	

			//clear all frame buffer 
			frame_queue_dequeue_all(display_queue);

			for(i=0; i<regFrameBufCount; i++)
				VPU_DecClrDispFlag(handle, i);

			//can clear all display buffer before flushing a sync frame if HOST wants clearing all remained frame in buffer.
			ret = VPU_DecFrameBufferFlush(handle);
			if( ret != RETCODE_SUCCESS )
			{
				VLOG(ERR, "VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n", ret );
				goto ERR_DEC_OPEN;
			}

			ret = VPU_DecSetRdPtr(handle, decOP.bitstreamBuffer, 1);
			if( ret != RETCODE_SUCCESS )
			{
				VLOG(ERR, "VPU_DecSetRdPtr failed Error code is 0x%x \n", ret );
				goto ERR_DEC_OPEN;
			}

			if (randomAccess)
			{
				bufInfo.random_access_point = (bufInfo.size/100) * randomAccessPos;
				bufInfo.fillendbs = 0;
				streameos = 0;
				ret = WriteBsBufHelper(coreIdx, handle, &bufInfo, &vbStream, STREAM_FILL_SIZE, decConfig.checkeos, &streameos,   decOP.streamEndian);
				if( ret != RETCODE_SUCCESS )
				{
					VLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
					goto ERR_DEC_OPEN;
				}
			}
		
			randomeAccessFlush = 1;
		}
		if (ppuEnable) 
		{
			VPU_DecGiveCommand(handle, SET_ROTATOR_OUTPUT, &fbPPU[ppIdx]);
			if (decConfig.useRot)
			{
				VPU_DecGiveCommand(handle, ENABLE_ROTATION, 0);
				VPU_DecGiveCommand(handle, ENABLE_MIRRORING, 0);
			}

			if (decConfig.useDering)
				VPU_DecGiveCommand(handle, ENABLE_DERING, 0);			
		}

		//ConfigDecReport(coreIdx, handle, decOP.bitstreamFormat);		
		// Start decoding a frame.

		ret = VPU_DecStartOneFrame(handle, &decParam);
		if (ret != RETCODE_SUCCESS)
		{
			VLOG(ERR, "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}


		timeout_count = 0;
		//while((kbhitRet = osal_kbhit()) == 0)
		while(1)
		{
			int_reason = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);		//wait for 10ms to save stream filling time.
			if (int_reason == (int)-1) // timeout
			{
				if (timeout_count*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) 
				{
					VLOG(WARN, "\n VPU interrupt wait timeout instIdx=%d, PC=0x%x\n", instIdx, VpuReadReg(coreIdx, BIT_CUR_PC));
					//VLOG(WARN, "\n VPU interrupt wait timeout instIdx=%d\n", instIdx);
					VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
					break;
				}
				timeout_count++;
				int_reason = 0;					
			}
			if (decConfig.lowDelayInfo.lowDelayEn && decOP.bitstreamFormat == STD_AVC)
			{
				if (int_reason & (1<<INT_BIT_DEC_MB_ROWS)) 
				{
					VPU_ClearInterrupt(coreIdx);
					int_reason = 0;

					VPU_DecGiveCommand(handle, GET_LOW_DELAY_OUTPUT, &outputInfo);

					VLOG(INFO, "\n MB ROW interrupt is generated displayIdx=%d decodedIdx=%d picType=%d decodeSuccess=%d\n",
						outputInfo.indexFrameDisplay, outputInfo.indexFrameDecoded, outputInfo.picType, outputInfo.decodingSuccess);
#ifdef CNM_FPGA_PLATFORM
					if (outputInfo.indexFrameDisplay>=0)
					{
						if (!ppuEnable)
						{
							VPU_DecGetFrameBuffer(handle, outputInfo.indexFrameDisplay, &dispFrame);
							DisplayMixer(&dispFrame, rotbufWidth, rotbufHeight);
						}
					}
#endif
				}					
			}

			CheckUserDataInterrupt(coreIdx, handle, outputInfo.indexFrameDecoded, decOP.bitstreamFormat, int_reason);

			if (int_reason)
			{
				VPU_ClearInterrupt(coreIdx);
				if (int_reason & (1<<INT_BIT_PIC_RUN)) 
					break;
			}
			if (decConfig.lowDelayInfo.lowDelayEn && decOP.bitstreamFormat == STD_AVC)
			{
				if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY))
				{
					if (decOP.bitstreamMode != BS_MODE_PIC_END) 
						VPU_DecGiveCommand(handle, SET_DECODE_FLUSH, 0); // if you want to stop the current decoding process to avoid some delay. you can stop decoding by using this command
				}
			}

			ret = WriteBsBufHelper(coreIdx, handle, &bufInfo, &vbStream, STREAM_FILL_SIZE, decConfig.checkeos, &streameos, decOP.streamEndian);
			if (ret != RETCODE_SUCCESS) 
			{
				VLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
				goto ERR_DEC_OPEN;
			}

		}


		ret = VPU_DecGetOutputInfo(handle, &outputInfo);
		if (ret != RETCODE_SUCCESS) 
		{
			VLOG(ERR, "VPU_DecGetOutputInfo failed Error code is 0x%x , instIdx=%d\n", ret, instIdx);
#ifdef SUPPORT_MEM_PROTECT
			if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
				PrintMemoryAccessViolationReason(coreIdx, &outputInfo);
#endif
			goto ERR_DEC_OPEN;
		}

		if ((outputInfo.decodingSuccess & 0x01) == 0)
			VLOG(ERR, "VPU_DecGetOutputInfo decode fail framdIdx %d \n", frameIdx);
		else 
		{
			if (randomAccess && outputInfo.indexFrameDecoded >= 0) 
			{
				randomAccess = 0;
				decParam.iframeSearchEnable = decConfig.iframeSearchEnable; // reset the iFrameSearch mode only once decoding success.
			}
            if (decParam.iframeSearchEnable && outputInfo.indexFrameDecoded >= 0) 
            {
                decParam.iframeSearchEnable = 0; // reset the iFrameSearch mode only once I frame decoding success.
            }
            
		}
		if (decOP.bitstreamMode == BS_MODE_ROLLBACK) {
			if (outputInfo.decodingSuccess&0x10) {
				VLOG(WARN, "Feed more stream %d \n", frameIdx);
				ret = WriteBsBufHelper(coreIdx, handle, &bufInfo,
					&vbStream, STREAM_FILL_SIZE, decConfig.checkeos, &streameos, decOP.streamEndian);
				continue;
			}
		}

		if (outputInfo.sequenceChanged)
		{
			VLOG(WARN, "Sequence information has been changed\n"); //profileIdc/MbNumX/MbNumY/MaxDecFrameBuffering are changed
#ifdef TEST_SEQUENCE_CHANGE_BY_HOST
			seqChangeRequest = 1;
			seqChangedRdPtr = outputInfo.rdPtr;
			seqChangedWrPtr = outputInfo.wrPtr;
            ret = VPU_DecSetRdPtr(handle, seqChangedRdPtr, 1);
			seqChangedStreamEndFlag = outputInfo.streamEndFlag;
			VPU_DecUpdateBitstreamBuffer(handle, 1); // let f/w to know stream end condition in bitstream buffer. force to know that bitstream buffer will be empty.
			VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SET_FLAG); // set to stream end condition to pump out a delayed framebuffer.
#endif
		}

        
 		VLOG(TRACE, "#%d:%d, indexDisplay %d || picType %d || indexDecoded %d || dispFlag=0x%x, rdPtr=0x%x || wrPtr=0x%x || width=%d, || height=%d\n", 
 			instIdx, frameIdx, outputInfo.indexFrameDisplay, outputInfo.picType, outputInfo.indexFrameDecoded, outputInfo.frameDisplayFlag, outputInfo.rdPtr, outputInfo.wrPtr, outputInfo.dispPicWidth, outputInfo.dispPicHeight);

		if (decOP.bitstreamFormat == STD_AVC && decOP.avcExtension == AVC_AUX_MVC)
			VLOG(TRACE, "View index display: %d, View index decoded: %d\n", outputInfo.mvcPicInfo.viewIdxDisplay, outputInfo.mvcPicInfo.viewIdxDecoded);

		if (decOP.bitstreamFormat == STD_AVC && outputInfo.avcFpaSei.exist){
			//PrintSEI(&outputInfo);
		}

		//SaveDecReport(coreIdx, handle, &outputInfo, decOP.bitstreamFormat, ((initialInfo.picWidth+15)&~15)/16);		

		if (outputInfo.indexFrameDisplay == -1) {
#ifdef TEST_SEQUENCE_CHANGE_BY_HOST
			if (seqChangeRequest)
			{
				seqChangeRequest = 0;
				VPU_DecSetRdPtr(handle, seqChangedRdPtr, 1);
				if (seqChangedStreamEndFlag == 1)
					VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SET_FLAG);
				else
					VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_CLEAR_FLAG);
				if (seqChangedWrPtr >= seqChangedRdPtr)
					VPU_DecUpdateBitstreamBuffer(handle, seqChangedWrPtr-seqChangedRdPtr);
				else
					VPU_DecUpdateBitstreamBuffer(handle, 
					(decOP.bitstreamBuffer+decOP.bitstreamBufferSize)-seqChangedRdPtr + (seqChangedWrPtr-decOP.bitstreamBuffer));
				VPU_DecGiveCommand(handle, DEC_FREE_FRAME_BUFFER, 0x00);
				goto VPU_SEQ_INIT;
			}
			else
#endif
				decodefinish = 1;
			
		}

		if( outputInfo.numOfErrMBs ) 
		{
			totalNumofErrMbs += outputInfo.numOfErrMBs;
			VLOG(ERR, "Num of Error Mbs : %d, in Frame : %d \n", outputInfo.numOfErrMBs, frameIdx);
		}	

		if (!ppuEnable) 
		{
			if (decodefinish)
				break;

			if( outputInfo.indexFrameDisplay == -3 ||
				outputInfo.indexFrameDisplay == -2 ) // BIT doesn't have picture to be displayed
			{
				
#if defined(CNM_FPGA_PLATFORM) && defined(FPGA_LX_330)
#else
				if (outputInfo.indexFrameDecoded == -1)	// VPU did not decode a picture because there is not enough frame buffer to continue decoding
				{
					// if you can't get VSYN interrupt on your sw layer. this point is reasonable line to set VSYN flag.
					// but you need fine tune EXTRA_FRAME_BUFFER_NUM value not decoder to write being display buffer.
					if (frame_queue_count(display_queue) > 0)
						set_VSYNC_flag();	
				}
#endif
				if (check_VSYNC_flag())
				{
					clear_VSYNC_flag();

					if (frame_queue_dequeue(display_queue, &dispDoneIdx) == 0)
						VPU_DecClrDispFlag(handle, dispDoneIdx);					
				}

				continue;
			}
		}
		else
		{
			if (decodefinish)
			{
				// if PP feature has been enabled. the last picture is in PP output framebuffer.									
			}

			if (outputInfo.indexFrameDisplay == -3 ||
				outputInfo.indexFrameDisplay == -2 ) // BIT doesn't have picture to be displayed
			{				
#if defined(CNM_FPGA_PLATFORM) && defined(FPGA_LX_330)
#else
				if (outputInfo.indexFrameDecoded == -1)	// VPU did not decode a picture because there is not enough frame buffer to continue decoding
				{
					// if you can't get VSYN interrupt on your sw layer. this point is reasonable line to set VSYN flag.
					// but you need fine tuning EXTRA_FRAME_BUFFER_NUM value not decoder to write being display buffer.
					if (frame_queue_count(display_queue) > 0)
					set_VSYNC_flag();	
				}
#endif
				if (check_VSYNC_flag())
				{
					clear_VSYNC_flag();

					if (frame_queue_dequeue(display_queue, &dispDoneIdx) == 0)
						VPU_DecClrDispFlag(handle, dispDoneIdx);					
				}

				continue;
			}

			if (decodeIdx == 0) // if PP has been enabled, the first picture is saved at next time.
			{
				// save rotated dec width, height to display next decoding time.
				if (outputInfo.indexFrameDisplay >= 0)
					frame_queue_enqueue(display_queue, outputInfo.indexFrameDisplay);
				rcPrevDisp = outputInfo.rcDisplay;
				decodeIdx++;
				continue;
			}
		}

		decodeIdx++;
		// indexFrameDisplay points to the frame buffer, among ones registered, which holds
		// the output of the decoder.
		if (outputInfo.indexFrameDisplay >= 0)
			frame_queue_enqueue(display_queue, outputInfo.indexFrameDisplay);

		if (!saveImage)
		{
			if (ppuEnable)
				ppIdx = (ppIdx+1)%MAX_ROT_BUF_NUM;


#ifdef CNM_FPGA_PLATFORM
			DisplayMixer(&outputInfo.dispFrame, outputInfo.dispFrame.stride, outputInfo.dispFrame.height);		
#else
            #if 0
 			if (!SaveYuvImageHelperFormat(coreIdx, fpYuv, &outputInfo.dispFrame, mapCfg, pYuv, 
 				(ppuEnable)?rcPrevDisp:outputInfo.rcDisplay, decOP.cbcrInterleave, framebufFormat, decOP.frameEndian))
 				goto ERR_DEC_OPEN;
			#else
			vdi_read_memory(coreIdx, outputInfo.dispFrame.bufY, pYuv, framebufSize, decOP.frameEndian);			
			#endif
#ifdef SUPPORT_SW_MIXER
			sw_mixer_draw((coreIdx*MAX_NUM_VPU_CORE)+instIdx, 0, 0, outputInfo.dispFrame.stride, outputInfo.dispFrame.height, imageFormat, pYuv);
#endif
#endif

#ifdef FORCE_SET_VSYNC_FLAG
			set_VSYNC_flag();
#endif
		}
		else // store image
		{
			if (ppuEnable) 
				ppIdx = (ppIdx+1)%MAX_ROT_BUF_NUM;

			if (!SaveYuvImageHelperFormat(coreIdx, fpYuv, &outputInfo.dispFrame, mapCfg, pYuv, 
				(ppuEnable)?rcPrevDisp:outputInfo.rcDisplay, decOP.cbcrInterleave, framebufFormat, decOP.frameEndian))
				goto ERR_DEC_OPEN;

#ifdef SUPPORT_SW_MIXER
			sw_mixer_draw((coreIdx*MAX_NUM_VPU_CORE)+instIdx, 0, 0, outputInfo.dispFrame.stride, outputInfo.dispFrame.height, imageFormat, pYuv);	
#endif
#ifdef FORCE_SET_VSYNC_FLAG
			set_VSYNC_flag();
#endif
		}


		if (check_VSYNC_flag())
		{
			clear_VSYNC_flag();
			
			if (frame_queue_dequeue(display_queue, &dispDoneIdx) == 0)
				VPU_DecClrDispFlag(handle, dispDoneIdx);			
		}
				
		// save rotated dec width, height to display next decoding time.
		rcPrevDisp = outputInfo.rcDisplay;


		frameIdx++;

		if( decConfig.outNum && frameIdx >= decConfig.outNum )
			break;

		if (decodefinish)
			break;		

	}

	if (totalNumofErrMbs)
	{
		suc = 0;
		VLOG(ERR, "Total Num of Error MBs : %d\n", totalNumofErrMbs);
	}

ERR_DEC_OPEN:
	VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);
	VPU_DecClose(handle);

	if (display_queue)
	{
		frame_queue_dequeue_all(display_queue);
		frame_queue_deinit(display_queue);
	}

	VLOG(INFO, "\nDec End. Tot Frame %d\n", frameIdx);


ERR_DEC_INIT:
	if (vbStream.size)
		vdi_free_dma_memory(coreIdx, &vbStream);

	for (i=0; i<regFrameBufCount*2; i++)
	{
		if (vbFrame[i].size)
			vdi_free_dma_memory(coreIdx, &vbFrame[i]);
	}	

	if (pYuv)
		free(pYuv);

//close input file
    if (fchunkData)
    {
        osal_fclose(fchunkData);
        fchunkData = NULL;
    }
	if (pFileBuf)
		free(pFileBuf);

	if (fpYuv)
		osal_fclose(fpYuv);

	


#ifdef SUPPORT_SW_MIXER
	sw_mixer_close((coreIdx*MAX_NUM_VPU_CORE)+instIdx);
#endif

	VPU_DeInit(coreIdx);

	return suc;
}

#ifdef SUPPORT_FFMPEG_DEMUX
int FilePlayTest(DecConfigParam *param)
{

	DecHandle		handle		= {0};
	DecOpenParam	decOP		= {0};
	DecInitialInfo	initialInfo = {0};
	DecOutputInfo	outputInfo	= {0};
	DecParam		decParam	= {0};
	BufInfo			bufInfo	 = {0};
	vpu_buffer_t	vbStream	= {0};
	FrameBuffer		fbPPU[MAX_ROT_BUF_NUM];
	FrameBufferAllocInfo fbAllocInfo;
	SecAxiUse		secAxiUse = {0};
	RetCode			ret = RETCODE_SUCCESS;		
	int				i = 0, saveImage = 0, decodefinish = 0, err=0;
	int				framebufSize = 0, framebufWidth = 0, framebufHeight = 0, rotbufWidth = 0, rotbufHeight = 0, framebufFormat = FORMAT_420, mapType;
	int				framebufStride = 0, rotStride = 0, regFrameBufCount = 0;
	int				frameIdx = 0, ppIdx=0, decodeIdx=0;
	int				kbhitRet = 0,  totalNumofErrMbs = 0;
	int				dispDoneIdx = -1;
	BYTE *			pYuv	 =	NULL;
	osal_file_t		fpYuv	 =	NULL;
	int				seqInited, seqFilled, reUseChunk;
	int				hScaleFactor, vScaleFactor, scaledWidth, scaledHeight;
	int			 randomAccess = 0, randomAccessPos = 0, randomeAccessFlush = 0;
	int				ppuEnable = 0;
	int				int_reason = 0;
	int				bsfillSize = 0;
	int				size;
	int				instIdx, coreIdx;
	TiledMapConfig mapCfg;
	ImageFormat imageFormat = YUV_FORMAT_I420;
	FrameBuffer  *pUserFrame = NULL;
	vpu_buffer_t vbFrame[MAX_REG_FRAME] = {0,};
	FrameBuffer  fbUser[MAX_REG_FRAME]={0,};
	DRAMConfig dramCfg = {0};
	MaverickCacheConfig decCacheConfig;
	DecConfigParam decConfig;
	Rect		   rcPrevDisp;
	frame_queue_item_t* display_queue = NULL;
	//Theora
	tho_parser_t *  thoParser = 0; 
	BYTE *		  pThoStream	= NULL, thoSeqSize;

	AVFormatContext *ic;
	AVPacket pkt1, *pkt=&pkt1;
	AVCodecContext *ctxVideo;
	int idxVideo;
	int	chunkIdx = 0;

	BYTE *chunkData = NULL;
	int chunkSize = 0;
	BYTE *seqHeader = NULL;
	int seqHeaderSize = 0;
	BYTE *picHeader = NULL;
	int picHeaderSize = 0;

	const char *filename;

	av_register_all();


	VLOG(INFO, "ffmpeg library version is codec=0x%x, format=0x%x\n", avcodec_version(), avformat_version());
	
	memcpy(&decConfig, param, sizeof(DecConfigParam));

	filename = decConfig.bitstreamFileName;
	instIdx = decConfig.instNum;
	coreIdx = decConfig.coreIdx;

		
	ic = avformat_alloc_context();
	if (!ic)
		return 0;

	ic->flags |= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
	err = avformat_open_input(&ic, filename, NULL,  NULL);
	if (err < 0)
	{
		char str_err[128];
		av_strerror(err, str_err, 128);
		VLOG(ERR, "%s: could not open file reason=[%s]\n", filename, str_err);
		av_free(ic);
		return 0;
	}
	
	err = avformat_find_stream_info(ic,  NULL);
	if (err < 0) 
	{
		VLOG(ERR, "%s: could not find stream information\n", filename);
		goto ERR_DEC_INIT;
	}

	av_dump_format(ic, 0, filename, 0);

	// find video stream index
	idxVideo = -1;
	idxVideo = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (idxVideo < 0) 
	{
		err = -1;
		VLOG(ERR, "%s: could not find video stream information\n", filename);
		goto ERR_DEC_INIT;
	}

	ctxVideo = ic->streams[idxVideo]->codec;  

	seqHeader = osal_malloc(ctxVideo->extradata_size+MAX_CHUNK_HEADER_SIZE);	// allocate more buffer to fill the vpu specific header.
	if (!seqHeader)
	{
		VLOG(ERR, "fail to allocate the seqHeader buffer\n");
		goto ERR_DEC_INIT;
	}
	memset(seqHeader, 0x00, ctxVideo->extradata_size+MAX_CHUNK_HEADER_SIZE);

	picHeader = osal_malloc(MAX_CHUNK_HEADER_SIZE);
	if (!picHeader)
	{
		VLOG(ERR, "fail to allocate the picHeader buffer\n");
		goto ERR_DEC_INIT;
	}
	memset(picHeader, 0x00, MAX_CHUNK_HEADER_SIZE);

	if (strlen(decConfig.yuvFileName))
		saveImage = 1;
	else
		saveImage = 0;

	if (strlen(decConfig.yuvFileName)) 
	{
		fpYuv = osal_fopen(decConfig.yuvFileName, "wb");
		if (!fpYuv) 
		{
			VLOG(ERR, "Can't open %s \n", decConfig.yuvFileName);
			goto ERR_DEC_INIT;
		}
	}


	ret = VPU_Init(coreIdx);
#ifdef BIT_CODE_FILE_PATH
#else
	if (ret == RETCODE_NOT_FOUND_BITCODE_PATH)
	{
		osal_file_t fpBitCode = NULL;
		Uint16 *pusBitCode;
		if (coreIdx == 0)
			fpBitCode = osal_fopen(CORE_0_BIT_CODE_FILE_PATH, "rb");
		else if (coreIdx == 1)
			fpBitCode = osal_fopen(CORE_1_BIT_CODE_FILE_PATH, "rb");
		if (fpBitCode)
		{
			pusBitCode = (Uint16 *)osal_malloc(CODE_BUF_SIZE);
			if (pusBitCode)
			{
				int count = 0;
				int code;
				while (!osal_feof(fpBitCode) || count < (CODE_BUF_SIZE/2)) {
					code = -1;
					if (osal_fscanf(fpBitCode, "%x", &code) <= 0) {
						/* matching failure or EOF */
						break;
					}
					pusBitCode[count++] = (Uint16)code;
				}
				ret = VPU_InitWithBitcode(coreIdx, pusBitCode, count);
				osal_free(pusBitCode);
			}
			osal_fclose(fpBitCode);
		}
		else 
		{
			VLOG(ERR, "failed open bit_firmware file path is %s\n", CORE_0_BIT_CODE_FILE_PATH);
			goto ERR_DEC_INIT;
		}
	}
#endif
	if (ret != RETCODE_SUCCESS && 
		ret != RETCODE_CALLED_BEFORE) 
	{
		VLOG(ERR, "VPU_Init failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}

	CheckVersion(coreIdx);


	decOP.bitstreamFormat = fourCCToCodStd(ctxVideo->codec_tag);
	if (decOP.bitstreamFormat == -1)
		decOP.bitstreamFormat = codecIdToCodStd(ctxVideo->codec_id);

	if (decOP.bitstreamFormat == -1)
	{
		VLOG(ERR, "can not support video format in VPU tag=%c%c%c%c, codec_id=0x%x \n", ctxVideo->codec_tag>>0, ctxVideo->codec_tag>>8, ctxVideo->codec_tag>>16, ctxVideo->codec_tag>>24, ctxVideo->codec_id );
		goto ERR_DEC_INIT;
	}

	vbStream.size = STREAM_BUF_SIZE; //STREAM_BUF_SIZE;	
	vbStream.size = ((vbStream.size+1023)&~1023);
	if (vdi_allocate_dma_memory(coreIdx, &vbStream) < 0)
	{
		VLOG(ERR, "fail to allocate bitstream buffer\n" );
		goto ERR_DEC_INIT;
	}


	decOP.bitstreamBuffer = vbStream.phys_addr; 
	decOP.bitstreamBufferSize = vbStream.size;
	decOP.mp4DeblkEnable = 0;

	decOP.mp4Class = fourCCToMp4Class(ctxVideo->codec_tag);
	if (decOP.mp4Class == -1)
        decOP.mp4Class = codecIdToMp4Class(ctxVideo->codec_id);

	if(decOP.bitstreamFormat == STD_THO || decOP.bitstreamFormat == STD_VP3)
	{
		theora_parser_init((void **)&thoParser);

		pThoStream = osal_malloc(SIZE_THO_STREAM);
		if (!pThoStream)
		{
			VLOG(ERR, "pThoStream buffer osal_malloc fail\n");
			goto ERR_DEC_INIT;
		}
	}

	decOP.tiled2LinearEnable = (decConfig.mapType>>4)&0x1;
	mapType = decConfig.mapType & 0xf;
	if (mapType) 
	{
		decOP.wtlEnable = decConfig.wtlEnable;
		if (decOP.wtlEnable)
		{
            decConfig.rotAngle;
            decConfig.mirDir;
            decConfig.useRot = 0;
            decConfig.useDering = 0;
			decOP.mp4DeblkEnable = 0;
            decOP.tiled2LinearEnable = 0;
		}
	}

	decOP.cbcrInterleave = CBCR_INTERLEAVE;
	if (mapType == TILED_FRAME_MB_RASTER_MAP ||
		mapType == TILED_FIELD_MB_RASTER_MAP) {
			decOP.cbcrInterleave = 1;
	}
	decOP.bwbEnable = VPU_ENABLE_BWB;
	decOP.frameEndian  = VPU_FRAME_ENDIAN;
	decOP.streamEndian = VPU_STREAM_ENDIAN;
	decOP.bitstreamMode = decConfig.bitstreamMode;
	if (decConfig.useRot || decConfig.useDering || decOP.tiled2LinearEnable) 
		ppuEnable = 1;
	else
		ppuEnable = 0;

	ret = VPU_DecOpen(&handle, &decOP);
	if (ret != RETCODE_SUCCESS) 
	{
		VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}  	
	//VPU_DecGiveCommand(handle, ENABLE_LOGGING, 0);
	ret = VPU_DecGiveCommand(handle, GET_DRAM_CONFIG, &dramCfg);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
	VLOG(INFO, "Dec Start : Press enter key to show menu.\n" );
	VLOG(INFO, "		  : Press space key to stop.\n" );

	seqInited = 0;
	seqFilled = 0;
	bsfillSize = 0;
	reUseChunk = 0;
	display_queue = frame_queue_init(MAX_REG_FRAME);
	init_VSYNC_flag();
	while(1)
	{
		if (osal_kbhit()) 
		{
			kbhitRet = osal_getch();
			osal_fflush(stdout);			
		}		

		if (kbhitRet)
		{
			if (kbhitRet == '\r' || kbhitRet == '\n')
			#if 0	
			{
				ret = UI_GetUserCommand(coreIdx, handle, &decParam, &randomAccessPos);
				kbhitRet = 0;
				if (ret == UI_CMD_SKIP_FRAME_LIFECYCLE)
					continue;
				else if (ret == UI_CMD_RANDOM_ACCESS)
				{
					randomAccess = 1;
					decParam.iframeSearchEnable = 1;
				}
			}
			#else
			{
				ret = UI_GetUserCommand(coreIdx, handle, &decParam, &randomAccessPos);
				if (ret == UI_CMD_SKIP_FRAME_LIFECYCLE)
					continue;
				else if (ret == UI_CMD_RANDOM_ACCESS)
				{
					randomAccess = 1;
					decParam.iframeSearchEnable = 1;
				}
				kbhitRet = 0;
                if (decParam.skipframeMode)
                {
                    //clear all frame buffer 
                    frame_queue_dequeue_all(display_queue);

                    for(i=0; i<regFrameBufCount; i++)
                        VPU_DecClrDispFlag(handle, i);

					//can clear all display buffer before flushing a sync frame if HOST wants clearing all remained frame in buffer.
                    ret = VPU_DecFrameBufferFlush(handle);
                    if( ret != RETCODE_SUCCESS )
                    {
                        VLOG(ERR, "VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n", ret );
                        goto ERR_DEC_OPEN;
                    }

                    ret = VPU_DecGiveCommand(handle, DEC_DISABLE_SKIP_REORDER, 0);
                    if( ret != RETCODE_SUCCESS )
                    {
                        VLOG(ERR, "VPU_DecGiveCommand(DEC_DISABLE_SKIP_REORDER) failed Error code is 0x%x \n", ret );
                        goto ERR_DEC_OPEN;
                    }

                }
			}
			#endif
			else if (kbhitRet == ' ')
				break;
		}


		seqHeaderSize = 0;
		picHeaderSize = 0;

		if (decOP.bitstreamMode == BS_MODE_PIC_END)
		{
			if (reUseChunk)
			{
				reUseChunk = 0;
				goto FLUSH_BUFFER;			
			}
			VPU_DecSetRdPtr(handle, decOP.bitstreamBuffer, 1);	
		}

		av_init_packet(pkt);

		err = av_read_frame(ic, pkt);
		if (err < 0) 
		{
			if (pkt->stream_index == idxVideo)
				chunkIdx++;	

			if (err==AVERROR_EOF || url_feof(ic->pb)) 
			{
				bsfillSize = VPU_GBU_SIZE*2;
				chunkSize = 0;					
				VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);	//tell VPU to reach the end of stream. starting flush decoded output in VPU
				goto FLUSH_BUFFER;
			}
			continue;
		}

		if (pkt->stream_index != idxVideo)
			continue;

		
		if (randomeAccessFlush == 0 && randomAccess)
		{
			int tot_frame;
			int pos_frame;

			tot_frame = (int)ic->streams[idxVideo]->nb_frames;
			pos_frame = (tot_frame/100) * randomAccessPos;
			if (pos_frame < ctxVideo->frame_number)
				continue;			
			else
			{
				randomAccess = 0;
				//assume display done
				set_VSYNC_flag();	

				//clear all frame buffer 
				frame_queue_dequeue_all(display_queue);

				for(i=0; i<regFrameBufCount; i++)
					VPU_DecClrDispFlag(handle, i);

				//Clear all display buffer before Bitstream & Frame buffer flush
				ret = VPU_DecFrameBufferFlush(handle);
				if( ret != RETCODE_SUCCESS )
				{
					VLOG(ERR, "VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n", ret );
					goto ERR_DEC_OPEN;
				}
			}

			randomeAccessFlush = 1;
		}

		chunkData = pkt->data;
		chunkSize = pkt->size;

		
		if (!seqInited && !seqFilled)
		{
			seqHeaderSize = BuildSeqHeader(seqHeader, decOP.bitstreamFormat, ic->streams[idxVideo]);	// make sequence data as reference file header to support VPU decoder.
			switch(decOP.bitstreamFormat)
			{
			case STD_THO:
				{
					ret = thoParser->open(thoParser->handle, seqHeader, seqHeaderSize, (int *)&initialInfo.thoScaleInfo);
					if(ret < 0)
					{
						// Failed to open container
						VLOG(ERR, "Parser failure - Theora header parsing error\n");
						goto ERR_DEC_OPEN;
					}				
					thoSeqSize = theora_make_stream((void *)thoParser->handle, pThoStream, SEQ_INIT);
				}				
				break;

			case STD_VP3:
				{
					AVCodecContext *codec = ic->streams[idxVideo]->codec;
					ret = thoParser->open(thoParser->handle, seqHeader, 0,  (int *)&initialInfo.thoScaleInfo);
					if(ret < 0)
					{
						// Failed to open container
						VLOG(ERR, "Parser failure - VP3 open error\n");
						goto ERR_DEC_OPEN;
					}			
				}
				break;
			default:
				{
					size = WriteBsBufFromBufHelper(coreIdx, handle, &vbStream, seqHeader, seqHeaderSize, decOP.streamEndian);
					if (size < 0)
					{
						VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
						goto ERR_DEC_OPEN;
					}
						
					bsfillSize += size;
				}
				break;
			}
			seqFilled = 1;
		}

		// Build and Fill picture Header data which is dedicated for VPU 
		picHeaderSize = BuildPicHeader(picHeader, decOP.bitstreamFormat, ic->streams[idxVideo], pkt);	
		switch(decOP.bitstreamFormat)
		{
		case STD_THO:
		case STD_VP3:
			break;
		default:
			size = WriteBsBufFromBufHelper(coreIdx, handle, &vbStream, picHeader, picHeaderSize, decOP.streamEndian);
			if (size < 0)
			{
				VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
				goto ERR_DEC_OPEN;
			}	
			bsfillSize += size;
			break;
		}

		switch(decOP.bitstreamFormat)
		{
		case STD_VP3:
		case STD_THO:
			{
				if((ret = thoParser->read_frame((void *)thoParser->handle, chunkData, chunkSize)) < 0) 
					break;


				/* refer to 6.2.3.2 Macroblock order Matching,
				6.2.3.3  Macroblock Packing in Programmer User Guide */
				chunkSize = theora_make_stream((void *)thoParser->handle, pThoStream + thoSeqSize, PIC_RUN);

				// Loading quantization matrices and MB packed data
				size = WriteBsBufFromBufHelper(coreIdx, handle, &vbStream, pThoStream, chunkSize + thoSeqSize, decOP.streamEndian);
				if (size < 0)
				{
					VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
					goto ERR_DEC_OPEN;
				}

				bsfillSize += size;
				thoSeqSize = 0;
			}
			break;
		default:
			{
				if (decOP.bitstreamFormat == STD_RV)
				{
					int cSlice = chunkData[0] + 1;
					int nSlice =  chunkSize - 1 - (cSlice * 8);
					chunkData += (1+(cSlice*8));
					chunkSize = nSlice;
				}

				size = WriteBsBufFromBufHelper(coreIdx, handle, &vbStream, chunkData, chunkSize, decOP.streamEndian);
				if (size <0)
				{
					VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
					goto ERR_DEC_OPEN;
				}

				bsfillSize += size;
			}
			break;
		}		


		
//#define DUMP_ES_DATA
//#define DUMP_ES_WITH_SIZE
#ifdef DUMP_ES_DATA
		{
			osal_file_t fpDump;
			if (chunkIdx == 0)
				fpDump = osal_fopen("dump.dat", "wb");
			else
				fpDump = osal_fopen("dump.dat", "a+b");
			if (fpDump)
			{
				if (chunkIdx == 0) 
				{
#ifdef DUMP_ES_WITH_SIZE
					osal_fwrite(&seqHeaderSize, 4, 1, fpDump);
#endif
					osal_fwrite(seqHeader, seqHeaderSize, 1, fpDump);
				}
				if (picHeaderSize) 
				{
#ifdef DUMP_ES_WITH_SIZE
					osal_fwrite(&picHeaderSize, 4, 1, fpDump);
#endif
					osal_fwrite(picHeader, picHeaderSize, 1, fpDump);	
				}
#ifdef DUMP_ES_WITH_SIZE
				osal_fwrite(&chunkSize, 4, 1, fpDump);
#endif
				osal_fwrite(chunkData, chunkSize, 1, fpDump);					
				osal_fclose(fpDump);
			}
		}
#endif
		av_free_packet(pkt);

		chunkIdx++;


		if (!seqInited)
		{ 
			ConfigSeqReport(coreIdx, handle, decOP.bitstreamFormat);
			if (decOP.bitstreamMode == BS_MODE_PIC_END)
			{
				ret = VPU_DecGetInitialInfo(handle, &initialInfo);
				if (ret != RETCODE_SUCCESS) 
				{
#ifdef SUPPORT_MEM_PROTECT
					if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
						PrintMemoryAccessViolationReason(coreIdx, NULL); 
#endif
					VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret);
					goto ERR_DEC_OPEN;					
				}
				VPU_ClearInterrupt(coreIdx);
			}
			else
			{
				if((int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) != (1<<INT_BIT_BIT_BUF_EMPTY))
				{
					ret = VPU_DecIssueSeqInit(handle);
					if (ret != RETCODE_SUCCESS)
					{
						VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
						goto ERR_DEC_OPEN;
					}
				}
				else
				{
					// After VPU generate the BIT_EMPTY interrupt. HOST should feed the bitstream up to 1024 in case of seq_init
					if (bsfillSize < VPU_GBU_SIZE*2)
						continue;
				}
				while((kbhitRet = osal_kbhit()) == 0) 
				{	
					int_reason = VPU_WaitInterrupt(coreIdx, VPU_DEC_TIMEOUT);

					if (int_reason)
						VPU_ClearInterrupt(coreIdx);

					if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) 
						break;
				

					CheckUserDataInterrupt(coreIdx, handle, 1, decOP.bitstreamFormat, int_reason);

					if (int_reason)
					{
						if (int_reason & (1<<INT_BIT_SEQ_INIT)) 
						{
							seqInited = 1;
							break;
						}
					}
				}
				if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY) || int_reason == -1) 
				{
					bsfillSize = 0;
					continue; // go to take next chunk.
				}
				if (seqInited)
				{
					ret = VPU_DecCompleteSeqInit(handle, &initialInfo);	
					if (ret != RETCODE_SUCCESS)
					{
#ifdef SUPPORT_MEM_PROTECT
						if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
							PrintMemoryAccessViolationReason(coreIdx, NULL);
#endif
						if (initialInfo.seqInitErrReason & (1<<31)) // this case happened only ROLLBACK mode
							VLOG(ERR, "Not enough header : Parser has to feed right size of a sequence header  \n");
						VLOG(ERR, "VPU_DecCompleteSeqInit failed Error code is 0x%x \n", ret );
						goto ERR_DEC_OPEN;
					}			
				}
				else
				{
					VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret);
					goto ERR_DEC_OPEN;
				}
			}




			SaveSeqReport(coreIdx, handle, &initialInfo, decOP.bitstreamFormat);	
#ifdef REPORT_FRAMERATE
			{
				int frame_rate;
				frame_rate = GetFrameRate(coreIdx, &initialInfo, decOP.bitstreamFormat);
				VLOG(ERR, "GetFrameRate %d \n", frame_rate);
			}
#endif // #ifdef REPORT_FRAMERATE

			if (decOP.bitstreamFormat == STD_VP8)		
			{
				// For VP8 frame upsampling infomration
				static const int scale_factor_mul[4] = {1, 5, 5, 2};
				static const int scale_factor_div[4] = {1, 4, 3, 1};
				hScaleFactor = initialInfo.vp8ScaleInfo.hScaleFactor;
				vScaleFactor = initialInfo.vp8ScaleInfo.vScaleFactor;
				scaledWidth = initialInfo.picWidth * scale_factor_mul[hScaleFactor] / scale_factor_div[hScaleFactor];
				scaledHeight = initialInfo.picHeight * scale_factor_mul[vScaleFactor] / scale_factor_div[vScaleFactor];
				framebufWidth = ((scaledWidth+15)&~15);
				if (IsSupportInterlaceMode(decOP.bitstreamFormat, &initialInfo))
					framebufHeight = ((scaledHeight+31)&~31); // framebufheight must be aligned by 31 because of the number of MB height would be odd in each filed picture.
				else
					framebufHeight = ((scaledHeight+15)&~15);

				rotbufWidth = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ?
					((scaledHeight+15)&~15) : ((scaledWidth+15)&~15);
				rotbufHeight = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ?
					((scaledWidth+15)&~15) : ((scaledHeight+15)&~15);				
			}
			else
			{
				if (decConfig.maxWidth)
				{
					if (decConfig.maxWidth < initialInfo.picWidth)
					{
						VLOG(ERR, "maxWidth is too small\n");
						goto ERR_DEC_INIT;
					}
					framebufWidth = ((decConfig.maxWidth+15)&~15);
				}
				else
					framebufWidth = ((initialInfo.picWidth+15)&~15);

				if (decConfig.maxHeight)
				{
					if (decConfig.maxHeight < initialInfo.picHeight)
					{
						VLOG(ERR, "maxHeight is too small\n");
						goto ERR_DEC_INIT;
					}

					if (IsSupportInterlaceMode(decOP.bitstreamFormat, &initialInfo))
						framebufHeight = ((decConfig.maxHeight+31)&~31); // framebufheight must be aligned by 31 because of the number of MB height would be odd in each filed picture.
					else
						framebufHeight = ((decConfig.maxHeight+15)&~15);
				}
				else
				{
					if (IsSupportInterlaceMode(decOP.bitstreamFormat, &initialInfo))
						framebufHeight = ((initialInfo.picHeight+31)&~31); // framebufheight must be aligned by 31 because of the number of MB height would be odd in each filed picture.
					else
						framebufHeight = ((initialInfo.picHeight+15)&~15);
				}
				rotbufWidth = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? 
					((initialInfo.picHeight+15)&~15) : ((initialInfo.picWidth+15)&~15);
				rotbufHeight = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? 
					((initialInfo.picWidth+15)&~15) : ((initialInfo.picHeight+15)&~15);
			}

			rotStride = rotbufWidth;
			framebufStride = framebufWidth;
			framebufFormat = FORMAT_420;	
			framebufSize = VPU_GetFrameBufSize(framebufStride, framebufHeight, mapType, framebufFormat, &dramCfg);

			

#ifdef SUPPORT_SW_MIXER
			sw_mixer_close((coreIdx*MAX_NUM_VPU_CORE)+instIdx);

			if (!ppuEnable)
				sw_mixer_open((coreIdx*MAX_NUM_VPU_CORE)+instIdx, framebufStride, framebufHeight);
			else
				sw_mixer_open((coreIdx*MAX_NUM_VPU_CORE)+instIdx, rotStride, rotbufHeight);
#endif
			// the size of pYuv should be aligned 8 byte. because of C&M HPI bus system constraint.
			pYuv = osal_malloc(framebufSize);
			if (!pYuv) 
			{
				VLOG(ERR, "Fail to allocation memory for display buffer\n");
				goto ERR_DEC_INIT;
			}

			secAxiUse.useBitEnable  = USE_BIT_INTERNAL_BUF;
			secAxiUse.useIpEnable   = USE_IP_INTERNAL_BUF;
			secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
			secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
			secAxiUse.useBtpEnable  = USE_BTP_INTERNAL_BUF;
			secAxiUse.useOvlEnable  = USE_OVL_INTERNAL_BUF;
			VPU_DecGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

			// MaverickCache configure			
			MaverickCache2Config(
				&decCacheConfig, 
				1, //decoder
				decOP.cbcrInterleave, // cb cr interleave				
				decConfig.frameCacheBypass,
				decConfig.frameCacheBurst,
				decConfig.frameCacheMerge,
				mapType,
				decConfig.frameCacheWayShape);
			VPU_DecGiveCommand(handle, SET_CACHE_CONFIG, &decCacheConfig);
			
			regFrameBufCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;

#ifdef TEST_USER_FRAME_BUFFER
			decConfig.userFbAlloc = 1;
#endif
			if (decConfig.userFbAlloc) // if HOST wants to allocate framebuffer outside of API
			{
				fbAllocInfo.format          = framebufFormat;
				fbAllocInfo.cbcrInterleave  = decOP.cbcrInterleave;
				fbAllocInfo.mapType         = mapType;
				fbAllocInfo.stride          = framebufStride;
				fbAllocInfo.height          = framebufHeight;
				fbAllocInfo.num             = regFrameBufCount;

				fbAllocInfo.endian          = decOP.frameEndian;
				fbAllocInfo.type            = FB_TYPE_CODEC;
				// if HOST has a frame buffer which is allocated at other block such as render mode.  HOST can set it to VPU without frame buffer allocation in API		
				framebufSize = VPU_GetFrameBufSize(framebufStride, framebufHeight, fbAllocInfo.mapType, framebufFormat, &dramCfg);
				for (i=0; i<fbAllocInfo.num; i++)
				{
					vbFrame[i].size = framebufSize;
					if (vdi_allocate_dma_memory(coreIdx, &vbFrame[i]) < 0)
					{
						VLOG(ERR, "fail to allocate frame buffer\n" );
						goto ERR_DEC_OPEN;
					}
					fbUser[i].bufY = vbFrame[i].phys_addr;
					fbUser[i].bufCb = -1;
					fbUser[i].bufCr = -1;
				}
				ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, fbUser);
				if( ret != RETCODE_SUCCESS )
				{
					VLOG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
					goto ERR_DEC_OPEN;
				}
				if (decOP.wtlEnable)
				{
					fbAllocInfo.mapType = LINEAR_FRAME_MAP;

					framebufSize = VPU_GetFrameBufSize(framebufStride, framebufHeight, fbAllocInfo.mapType, framebufFormat, &dramCfg);
					for (i=regFrameBufCount; i<regFrameBufCount*2; i++)
					{
						vbFrame[i].size = framebufSize;
						if (vdi_allocate_dma_memory(coreIdx, &vbFrame[i]) < 0)
						{
							VLOG(ERR, "fail to allocate frame buffer\n" );
							goto ERR_DEC_OPEN;
						}
						fbUser[i].bufY = vbFrame[i].phys_addr;
						fbUser[i].bufCb = -1;
						fbUser[i].bufCr = -1;
					}

					ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, &fbUser[regFrameBufCount]);
					if( ret != RETCODE_SUCCESS )
					{
						VLOG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
						goto ERR_DEC_OPEN;
					}
				}


				pUserFrame = (FrameBuffer *)fbUser;
			}
			// Register frame buffers requested by the decoder.
			ret = VPU_DecRegisterFrameBuffer(handle, pUserFrame, regFrameBufCount, framebufStride, framebufHeight, mapType); // frame map type (can be changed before register frame buffer)
			if (ret != RETCODE_SUCCESS) 
			{
				VLOG(ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret);
				goto ERR_DEC_OPEN;
			}
			VPU_DecGiveCommand(handle, GET_TILEDMAP_CONFIG, &mapCfg);

			if (ppuEnable) 
			{
				ppIdx = 0;

				fbAllocInfo.format          = framebufFormat;
				fbAllocInfo.cbcrInterleave  = decOP.cbcrInterleave;
				if (decOP.tiled2LinearEnable)
					fbAllocInfo.mapType = LINEAR_FRAME_MAP;
				else
					fbAllocInfo.mapType = mapType;

				fbAllocInfo.stride  = rotStride;
				fbAllocInfo.height  = rotbufHeight;
				fbAllocInfo.num     = MAX_ROT_BUF_NUM;
				fbAllocInfo.endian  = decOP.frameEndian;
				fbAllocInfo.type    = FB_TYPE_PPU;
				ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, fbPPU);
				if( ret != RETCODE_SUCCESS )
				{
					VLOG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
					goto ERR_DEC_OPEN;
				}

				ppIdx = 0;

				if (decConfig.useRot)
				{
					VPU_DecGiveCommand(handle, SET_ROTATION_ANGLE, &(decConfig.rotAngle));
					VPU_DecGiveCommand(handle, SET_MIRROR_DIRECTION, &(decConfig.mirDir));
				}

				VPU_DecGiveCommand(handle, SET_ROTATOR_STRIDE, &rotStride);
			
			}


			InitMixerInt();

			seqInited = 1;			

		}

FLUSH_BUFFER:		
		
		if((int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) != (1<<INT_BIT_BIT_BUF_EMPTY) && (int_reason & (1<<INT_BIT_DEC_FIELD)) != (1<<INT_BIT_DEC_FIELD))
		{
			if (ppuEnable) 
			{
				VPU_DecGiveCommand(handle, SET_ROTATOR_OUTPUT, &fbPPU[ppIdx]);

				if (decConfig.useRot)
				{
					VPU_DecGiveCommand(handle, ENABLE_ROTATION, 0);
					VPU_DecGiveCommand(handle, ENABLE_MIRRORING, 0);
				}

				if (decConfig.useDering)
					VPU_DecGiveCommand(handle, ENABLE_DERING, 0);			
			}

			ConfigDecReport(coreIdx, handle, decOP.bitstreamFormat);
			


			// Start decoding a frame.
			ret = VPU_DecStartOneFrame(handle, &decParam);
			if (ret != RETCODE_SUCCESS) 
			{
				VLOG(ERR,  "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret);
				goto ERR_DEC_OPEN;
			}
		}
		else
		{
			if(int_reason & (1<<INT_BIT_DEC_FIELD))
			{
				VPU_ClearInterrupt(coreIdx);
				int_reason = 0;
			}
			// After VPU generate the BIT_EMPTY interrupt. HOST should feed the bitstreams than 512 byte.
			if (decOP.bitstreamMode != BS_MODE_PIC_END)
			{
				if (bsfillSize < VPU_GBU_SIZE)
					continue;
			}
		}



		while((kbhitRet = osal_kbhit()) == 0) 
		{
			int_reason = VPU_WaitInterrupt(coreIdx, VPU_DEC_TIMEOUT);
			if (int_reason == (Uint32)-1) // timeout
			{
				VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);				
				break;
			}		

			CheckUserDataInterrupt(coreIdx, handle, outputInfo.indexFrameDecoded, decOP.bitstreamFormat, int_reason);
			if(int_reason & (1<<INT_BIT_DEC_FIELD))	
			{
				if (decOP.bitstreamMode == BS_MODE_PIC_END)
				{
					PhysicalAddress rdPtr, wrPtr;
					int room;
					VPU_DecGetBitstreamBuffer(handle, &rdPtr, &wrPtr, &room);
					if (rdPtr-decOP.bitstreamBuffer < (PhysicalAddress)(chunkSize+picHeaderSize+seqHeaderSize-8))	// there is full frame data in chunk data.
						VPU_DecSetRdPtr(handle, rdPtr, 0);		//set rdPtr to the position of next field data.
					else
					{
						// do not clear interrupt until feeding next field picture.
						break;
					}
				}
			}

			if (int_reason)
				VPU_ClearInterrupt(coreIdx);

			if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) 
			{
				if (decOP.bitstreamMode == BS_MODE_PIC_END)
				{
					VLOG(ERR, "Invalid operation is occurred in pic_end mode \n");
					goto ERR_DEC_OPEN;
				}
				break;
			}


			if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;				
		}			

		if (int_reason == (Uint32)-1)
		{
			if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) 
			{
				bsfillSize = 0;
				continue; // go to take next chunk.
			}
			if(int_reason & (1<<INT_BIT_DEC_FIELD)) 
			{
				bsfillSize = 0;
				continue; // go to take next chunk.
			}		
		}

		ret = VPU_DecGetOutputInfo(handle, &outputInfo);
		if (ret != RETCODE_SUCCESS) 
		{
			VLOG(ERR,  "VPU_DecGetOutputInfo failed Error code is 0x%x \n", ret);
#ifdef SUPPORT_MEM_PROTECT
			if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
				PrintMemoryAccessViolationReason(coreIdx, &outputInfo);
#endif
			goto ERR_DEC_OPEN;
		}

		if ((outputInfo.decodingSuccess & 0x01) == 0)
		{
			VLOG(ERR, "VPU_DecGetOutputInfo decode fail framdIdx %d \n", frameIdx);
			VLOG(TRACE, "#%d, indexFrameDisplay %d || picType %d || indexFrameDecoded %d\n", 
				frameIdx, outputInfo.indexFrameDisplay, outputInfo.picType, outputInfo.indexFrameDecoded );
		}		
		else
		{
			if (randomAccess && outputInfo.indexFrameDecoded >= 0) 
			{
				randomAccess = 0;
				decParam.iframeSearchEnable = decConfig.iframeSearchEnable; // reset the iFrameSearch mode only once decoding success.
			}
            if (decParam.iframeSearchEnable && outputInfo.indexFrameDecoded >= 0) 
            {
                decParam.iframeSearchEnable = 0; // reset the iFrameSearch mode only once I frame decoding success.
            }
		}
		
		VLOG(TRACE, "#%d:%d, indexDisplay %d || picType %d || indexDecoded %d || dispFlag=0x%x, rdPtr=0x%x || wrPtr=0x%x || width=%d, || height=%d\n", 
			instIdx, frameIdx, outputInfo.indexFrameDisplay, outputInfo.picType, outputInfo.indexFrameDecoded, outputInfo.frameDisplayFlag, outputInfo.rdPtr, outputInfo.wrPtr, outputInfo.dispPicWidth, outputInfo.dispPicHeight);

		
		SaveDecReport(coreIdx, handle, &outputInfo, decOP.bitstreamFormat, ((initialInfo.picWidth+15)&~15)/16);
		if (outputInfo.chunkReuseRequired) // reuse previous chunk. that would be 1 once framebuffer is full.
			reUseChunk = 1;		

		if (outputInfo.indexFrameDisplay == -1)
			decodefinish = 1;

//#define TEST_RENEW_COMMAND
#ifdef TEST_RENEW_COMMAND
		if (decodefinish == 1)
		{
			VPU_DecUpdateBitstreamBuffer(handle, -1);
			decodefinish = 0;
			frameIdx = 0;
			decodeIdx = 0;
			int_reason = 0;
			clear_VSYNC_flag();
			frame_queue_dequeue_all(display_queue);
			av_seek_frame(ic, -1, 0, AVSEEK_FLAG_ANY);
			continue;
		}
#endif

		if (!ppuEnable) 
		{
			if (decodefinish)
				break;		

			if (outputInfo.indexFrameDisplay == -3 ||
				outputInfo.indexFrameDisplay == -2 ) // BIT doesn't have picture to be displayed 
			{		
				
#if defined(CNM_FPGA_PLATFORM) && defined(FPGA_LX_330)
#else
				if (outputInfo.indexFrameDecoded == -1)	// VPU did not decode a picture because there is not enough frame buffer to continue decoding
				{
					// if you can't get VSYN interrupt on your sw layer. this point is reasonable line to set VSYN flag.
					// but you need fine tune EXTRA_FRAME_BUFFER_NUM value not decoder to write being display buffer.
					if (frame_queue_count(display_queue) > 0 )
					set_VSYNC_flag();
					reUseChunk = 1;
					VPU_DecSetRdPtr(handle, decOP.bitstreamBuffer, 0);	
				}
#endif		
				if (check_VSYNC_flag())
				{
					clear_VSYNC_flag();

					if (frame_queue_dequeue(display_queue, &dispDoneIdx) == 0)
						VPU_DecClrDispFlag(handle, dispDoneIdx);					
				}		
				continue;
			}
		}
		else
		{
			if (decodefinish)
			{
				if (decodeIdx ==  0)
					break;
				// if PP feature has been enabled. the last picture is in PP output framebuffer.									
			}

			if (outputInfo.indexFrameDisplay == -3 ||
				outputInfo.indexFrameDisplay == -2 ) // BIT doesn't have picture to be displayed
			{				
#if defined(CNM_FPGA_PLATFORM) && defined(FPGA_LX_330)
#else
				if (outputInfo.indexFrameDecoded == -1)	// VPU did not decode a picture because there is not enough frame buffer to continue decoding
				{
					// if you can't get VSYN interrupt on your sw layer. this point is reasonable line to set VSYN flag.
					// but you need fine tuning EXTRA_FRAME_BUFFER_NUM value not decoder to write being display buffer.
					if (frame_queue_count(display_queue) > 0)
					set_VSYNC_flag();
					reUseChunk = 1;
				}
#endif		
				if (check_VSYNC_flag())
				{
					clear_VSYNC_flag();

					if (frame_queue_dequeue(display_queue, &dispDoneIdx) == 0)
						VPU_DecClrDispFlag(handle, dispDoneIdx);					
				}
				
				VPU_DecSetRdPtr(handle, decOP.bitstreamBuffer, 0);	
				continue;
			}

			if (decodeIdx == 0) // if PP has been enabled, the first picture is saved at next time.
			{
				// save rotated dec width, height to display next decoding time.
				if (outputInfo.indexFrameDisplay >= 0)
					frame_queue_enqueue(display_queue, outputInfo.indexFrameDisplay);
				rcPrevDisp = outputInfo.rcDisplay;
				decodeIdx++;
				continue;

			}
		}

		decodeIdx++;

		if (outputInfo.indexFrameDisplay >= 0)
			frame_queue_enqueue(display_queue, outputInfo.indexFrameDisplay);


		if (!saveImage)
		{			
			if (ppuEnable) 
				ppIdx = (ppIdx+1)%MAX_ROT_BUF_NUM;


#ifdef CNM_FPGA_PLATFORM
			DisplayMixer(&outputInfo.dispFrame, outputInfo.dispFrame.stride, outputInfo.dispFrame.height);		
#else
			if (!SaveYuvImageHelperFormat(coreIdx, fpYuv, &outputInfo.dispFrame, mapCfg, pYuv, 
				(ppuEnable)?rcPrevDisp:outputInfo.rcDisplay, decOP.cbcrInterleave, framebufFormat, decOP.frameEndian))
				goto ERR_DEC_OPEN;
#ifdef SUPPORT_SW_MIXER
			sw_mixer_draw((coreIdx*MAX_NUM_VPU_CORE)+instIdx, 0, 0, outputInfo.dispFrame.stride, outputInfo.dispFrame.height, imageFormat, pYuv);
#endif
#endif

#ifdef FORCE_SET_VSYNC_FLAG
			set_VSYNC_flag();
#endif

		}
		else // store image
		{
			if (ppuEnable) 
				ppIdx = (ppIdx+1)%MAX_ROT_BUF_NUM;

			if (!SaveYuvImageHelperFormat(coreIdx, fpYuv, &outputInfo.dispFrame, mapCfg, pYuv, 
				(ppuEnable)?rcPrevDisp:outputInfo.rcDisplay, decOP.cbcrInterleave, framebufFormat, decOP.frameEndian))
				goto ERR_DEC_OPEN;

#ifdef SUPPORT_SW_MIXER	
			sw_mixer_draw((coreIdx*MAX_NUM_VPU_CORE)+instIdx, 0, 0, outputInfo.dispFrame.stride, outputInfo.dispFrame.height, imageFormat, pYuv);	
#endif
#ifdef FORCE_SET_VSYNC_FLAG
			set_VSYNC_flag();
#endif
		}
		
		if (check_VSYNC_flag())
		{
			clear_VSYNC_flag();

			if (frame_queue_dequeue(display_queue, &dispDoneIdx) == 0)
				VPU_DecClrDispFlag(handle, dispDoneIdx);			
		}

		// save rotated dec width, height to display next decoding time.
		rcPrevDisp = outputInfo.rcDisplay;

		if (outputInfo.numOfErrMBs) 
		{
			totalNumofErrMbs += outputInfo.numOfErrMBs;
			VLOG(ERR, "Num of Error Mbs : %d, in Frame : %d \n", outputInfo.numOfErrMBs, frameIdx);
		}

		frameIdx++;

		if (decConfig.outNum && frameIdx == (decConfig.outNum-1)) 
			break;

		if (decodefinish)
			break;		
	}	// end of while

	if (totalNumofErrMbs) 
		VLOG(ERR, "Total Num of Error MBs : %d\n", totalNumofErrMbs);

ERR_DEC_OPEN:
	// Now that we are done with decoding, close the open instance.
	VPU_DecClose(handle);
	if (display_queue)
	{
		frame_queue_dequeue_all(display_queue);
		frame_queue_deinit(display_queue);
	}

	VLOG(INFO, "\nDec End. Tot Frame %d\n", frameIdx);

ERR_DEC_INIT:	
	if (vbStream.size)
		vdi_free_dma_memory(coreIdx, &vbStream);

	for (i=0; i<regFrameBufCount*2; i++)
	{
		if (vbFrame[i].size)
			vdi_free_dma_memory(coreIdx, &vbFrame[i]);
	}	
	if (seqHeader)
		free(seqHeader);

	if( pYuv )
		free( pYuv );

	if( fpYuv )
		osal_fclose( fpYuv );
	if (thoParser)
		thoParser->close(thoParser);

	if(pThoStream!=NULL)
		free(pThoStream);
	if ( picHeader )
		free(picHeader);


	avformat_close_input(&ic);	


#ifdef SUPPORT_SW_MIXER
	sw_mixer_close((coreIdx*MAX_NUM_VPU_CORE)+instIdx);
#endif

	VPU_DeInit(coreIdx);
	return 1;
}
#endif	//#ifdef SUPPORT_FFMPEG_DEMUX

/******************************************************************************
ENCODER test suite body
******************************************************************************/
int EncodeTest(EncConfigParam *param)
{

	EncHandle		handle		= { 0 };
	EncOpenParam	encOP		= { 0 };
	EncParam		encParam	= { 0 };
	EncInitialInfo	initialInfo	= { 0 };
	EncOutputInfo	outputInfo	= { 0 };
	SecAxiUse		secAxiUse = {0};
	vpu_buffer_t	vbStream	 = {0};
	vpu_buffer_t	vbReport	= {0};
#ifdef SUPPORT_MEM_PROTECT
	WriteMemProtectCfg memProtectCfg = {0};
#endif
	FrameBuffer		fbSrc[ENC_SRC_BUF_NUM];
	FrameBufferAllocInfo fbAllocInfo;
	RetCode			ret = RETCODE_SUCCESS;
	EncHeaderParam encHeaderParam = { 0 };
	int				i = 0, srcFrameIdx = 0, frameIdx = 0;
	int				srcFrameWidth, srcFrameHeight, srcFrameStride, srcFrameFormat = 0;
	int				framebufStride = 0, framebufWidth = 0, framebufHeight = 0, framebufFormat = 0, mapType;
	int				suc = 1, key = 0;
	Uint8 *			pYuv	= NULL;
	osal_file_t		fpYuv	= NULL;
	osal_file_t		fpBitstrm	= NULL;
	int				regFrameBufCount;
	int				int_reason = 0;
	int				timeout_count = 0;
	int				instIdx, coreIdx;
	MirrorDirection mirrorDirection;
	EncConfigParam encConfig;
	TiledMapConfig mapCfg;
	ImageFormat imageFormat = YUV_FORMAT_I420;	
	MaverickCacheConfig encCacheConfig;
#if defined(ENC_SOURCE_FRAME_DISPLAY) || defined(ENC_RECON_FRAME_DISPLAY)
	Rect rcMixer;
#endif
#ifdef SUPPORT_FFMPEG_DEMUX
	char *header_buf = NULL;				   //container header buffer
	int header_pos = 0;					 //container header buffer position
	AVOutputFormat *fmt = NULL;		 //ffmpeg
	AVFormatContext *oc = NULL;		 //ffmpeg Format I/O context.(must set to NULL)
	AVStream *video_st = NULL;		  //ffmpeg Stream structure	
#endif

	encConfig = *param;

	memset(&fbSrc[0], 0x00, sizeof(FrameBuffer)*ENC_SRC_BUF_NUM);

	instIdx = encConfig.instNum;
	coreIdx = encConfig.coreIdx;

#ifdef SUPPORT_FFMPEG_DEMUX
	// if you use container, don't open a file pointer 
	// because ffmpeg will open the file pointer in container_init function.
	if(encConfig.en_container && !encConfig.ringBufferEnable )
	{   
		// Check container file validity
		fpBitstrm = osal_fopen(encConfig.bitstreamFileName, "wb");
		if (!fpBitstrm)
		{
			 VLOG(ERR, "Can't open bitstream file %s \n", encConfig.bitstreamFileName );
			 goto ERR_ENC_INIT;
		}
		else
			osal_fclose(fpBitstrm);
		fpBitstrm = NULL;
	}
	else
	{
		fpBitstrm = osal_fopen(encConfig.bitstreamFileName, "wb");
		if( !fpBitstrm ) 
		{
			VLOG(ERR, "Can't open bitstream file %s \n", encConfig.bitstreamFileName );
			goto ERR_ENC_INIT;
		}
	}
#else
	fpBitstrm = osal_fopen(encConfig.bitstreamFileName, "wb");
	if( !fpBitstrm ) 
	{
		VLOG(ERR, "Can't open bitstream file %s \n", encConfig.bitstreamFileName );
		goto ERR_ENC_INIT;
	}
#endif /* SUPPORT_FFMPEG_DEMUX */

	ret = VPU_Init(coreIdx);
#ifdef BIT_CODE_FILE_PATH
#else
	if (ret == RETCODE_NOT_FOUND_BITCODE_PATH)
	{
		osal_file_t fpBitCode = NULL;
		Uint16 *pusBitCode;
		if (coreIdx == 0)
			fpBitCode = osal_fopen(CORE_0_BIT_CODE_FILE_PATH, "rb");
		else if (coreIdx == 1)
			fpBitCode = osal_fopen(CORE_1_BIT_CODE_FILE_PATH, "rb");
		if (fpBitCode)
		{
			pusBitCode = (Uint16 *)osal_malloc(CODE_BUF_SIZE);
			if (pusBitCode)
			{
				int count = 0;
				int code;
				while (!osal_feof(fpBitCode) || count < (CODE_BUF_SIZE/2)) {
					code = -1;
					if (osal_fscanf(fpBitCode, "%x", &code) <= 0) {
						/* matching failure or EOF */
						break;
					}
					pusBitCode[count++] = (Uint16)code;
				}
				ret = VPU_InitWithBitcode(coreIdx, pusBitCode, count);
				osal_free(pusBitCode);
			}
			osal_fclose(fpBitCode);
		}
		else 
		{
			VLOG(ERR, "failed open bit_firmware file path is %s\n", CORE_0_BIT_CODE_FILE_PATH);
			goto ERR_ENC_INIT;
		}
	}
#endif
	if( ret != RETCODE_SUCCESS && 
		ret != RETCODE_CALLED_BEFORE )
	{
		VLOG(ERR, "VPU_Init failed Error code is 0x%x \n", ret );
		goto ERR_ENC_INIT;
	}

	CheckVersion(coreIdx);
	// Fill parameters for encoding.
	encOP.bitstreamFormat = encConfig.stdMode;
	mapType = (encConfig.mapType & 0x0f);
	encOP.linear2TiledEnable = (encConfig.mapType>>4)&0x1;
	if( encConfig.stdMode == 0 )
		encOP.bitstreamFormat = STD_MPEG4;
	else if( encConfig.stdMode == 1 )
		encOP.bitstreamFormat = STD_H263;
	else if( encConfig.stdMode == 2 )
		encOP.bitstreamFormat = STD_AVC;
	else
	{
		VLOG(ERR, "Invalid bitstream format mode \n" );
		goto ERR_ENC_INIT;
	}

	if (strlen(encConfig.cfgFileName) != 0) 
		ret = getEncOpenParam(&encOP, &encConfig, NULL);
	else 
		ret = getEncOpenParamDefault(&encOP, &encConfig);
	if (ret == 0)
		goto ERR_ENC_INIT;		


	
	fpYuv = osal_fopen(encConfig.yuvFileName, "rb");
	if( !fpYuv ) 
	{
		VLOG(ERR, "Can't open yuv file %s \n", encConfig.yuvFileName );
		goto ERR_ENC_INIT;
	}
	
#ifdef CNM_FPGA_PLATFORM
	InitMixerInt();
#endif

	srcFrameWidth = ((encOP.picWidth+15)&~15);
	srcFrameStride = srcFrameWidth;

	srcFrameFormat = FORMAT_420;
	framebufFormat = FORMAT_420;
	srcFrameHeight = ((encOP.picHeight+15)&~15);


	

	framebufWidth = (encConfig.rotAngle==90||encConfig.rotAngle ==270)?srcFrameHeight:srcFrameWidth;
	framebufHeight = (encConfig.rotAngle==90||encConfig.rotAngle ==270)?srcFrameWidth:srcFrameHeight;
	framebufStride = framebufWidth;


	pYuv = osal_malloc(framebufStride*framebufHeight*3);
	if( !pYuv ) 
	{
		VLOG(ERR, "malloc() failed \n" );
		goto ERR_ENC_INIT;
	}
	

	vbStream.size	 = STREAM_BUF_SIZE;
	if (vdi_allocate_dma_memory(coreIdx, &vbStream) < 0)
	{
		VLOG(ERR, "fail to allocate bitstream buffer\n" );
		goto ERR_ENC_INIT;
	}
	encOP.bitstreamBuffer = vbStream.phys_addr;
	encOP.bitstreamBufferSize = vbStream.size;
	encOP.ringBufferEnable =  encConfig.ringBufferEnable;
	encOP.cbcrInterleave = CBCR_INTERLEAVE;
	if (mapType == TILED_FRAME_MB_RASTER_MAP ||
		mapType == TILED_FIELD_MB_RASTER_MAP) {
		encOP.cbcrInterleave = 1;
	}
	encOP.frameEndian = VPU_FRAME_ENDIAN;
	encOP.streamEndian = VPU_STREAM_ENDIAN;
	encOP.bwbEnable = VPU_ENABLE_BWB;
	encOP.lineBufIntEn =  encConfig.lineBufIntEn;
	encOP.coreIdx	  = coreIdx;


	// Open an instance and get initial information for encoding.
	ret = VPU_EncOpen(&handle, &encOP);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_EncOpen failed Error code is 0x%x \n", ret );
		goto ERR_ENC_INIT;
	}


	//VPU_EncGiveCommand(handle, ENABLE_LOGGING, 0);

	if( encConfig.useRot == 1 )
	{
		VPU_EncGiveCommand(handle, ENABLE_ROTATION, 0);
		VPU_EncGiveCommand(handle, ENABLE_MIRRORING, 0);
		VPU_EncGiveCommand(handle, SET_ROTATION_ANGLE, &encConfig.rotAngle);
		mirrorDirection = encConfig.mirDir;
		VPU_EncGiveCommand(handle, SET_MIRROR_DIRECTION, &mirrorDirection);
	}
	
	// allocate frame buffers for source frame
	secAxiUse.useBitEnable = USE_BIT_INTERNAL_BUF;
	secAxiUse.useIpEnable = USE_IP_INTERNAL_BUF;
	secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	secAxiUse.useBtpEnable = USE_BTP_INTERNAL_BUF;
	secAxiUse.useOvlEnable = USE_OVL_INTERNAL_BUF;
	VPU_EncGiveCommand(handle, SET_SEC_AXI, &secAxiUse);


	ret = VPU_EncGetInitialInfo(handle, &initialInfo);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
		goto ERR_ENC_OPEN;
	}

	VPU_ClearInterrupt(coreIdx);

	regFrameBufCount = initialInfo.minFrameBufferCount;


	// MaverickCache configure
	MaverickCache2Config(
		&encCacheConfig, 
		0, //encoder
		encOP.cbcrInterleave, // cb cr interleave
		encConfig.frameCacheBypass,
		encConfig.frameCacheBurst,
		encConfig.frameCacheMerge,
		mapType,
		encConfig.frameCacheWayShape);
	VPU_EncGiveCommand(handle, SET_CACHE_CONFIG, &encCacheConfig);

	ret = VPU_EncRegisterFrameBuffer(handle, NULL, regFrameBufCount, framebufStride, framebufHeight, mapType);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
		goto ERR_ENC_OPEN;
	}
	VPU_EncGiveCommand(handle, GET_TILEDMAP_CONFIG, &mapCfg);

	fbAllocInfo.format = srcFrameFormat;
	fbAllocInfo.cbcrInterleave = encOP.cbcrInterleave;
	if (encOP.linear2TiledEnable)
		fbAllocInfo.mapType = LINEAR_FRAME_MAP;
	else
		fbAllocInfo.mapType = mapType;

	fbAllocInfo.stride = srcFrameStride;
	fbAllocInfo.height = srcFrameHeight;
	fbAllocInfo.num = ENC_SRC_BUF_NUM;
	fbAllocInfo.endian = encOP.frameEndian;
	fbAllocInfo.type = FB_TYPE_PPU;
	
	ret = VPU_EncAllocateFrameBuffer(handle, fbAllocInfo, fbSrc);
	if( ret != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
		goto ERR_ENC_OPEN;
	}

#ifdef ENC_SOURCE_FRAME_DISPLAY
	rcMixer.left = 0; rcMixer.top = 0; rcMixer.right = srcFrameStride; rcMixer.bottom = srcFrameHeight;
#ifdef SUPPORT_SW_MIXER
	sw_mixer_open((coreIdx*MAX_NUM_VPU_CORE)+instIdx, srcFrameStride, srcFrameHeight);		
#endif
#endif
#ifdef ENC_RECON_FRAME_DISPLAY
	rcMixer.left = 0; rcMixer.top = 0; rcMixer.right = framebufStride; rcMixer.bottom = framebufHeight;
#ifdef SUPPORT_SW_MIXER
	sw_mixer_open((coreIdx*MAX_NUM_VPU_CORE)+instIdx, framebufStride, framebufHeight);
#endif
#endif

	encParam.forceIPicture = 0;
	encParam.skipPicture   = 0;
	encParam.quantParam	   = encConfig.picQpY;


	if (encOP.ringBufferEnable == 0)
	{
		encHeaderParam.buf = vbStream.phys_addr;
		encHeaderParam.size = vbStream.size;
	}

#ifdef SUPPORT_FFMPEG_DEMUX
	if (encConfig.en_container && !encConfig.ringBufferEnable)
	{
		encConfig.picWidth = encOP.picWidth;
		encConfig.picHeight= encOP.picHeight;
		if ( !container_init(encConfig, &fmt, &oc, &video_st, encOP.gopSize) )
			goto ERR_ENC_INIT;

		header_pos = 0;
		if(encOP.bitstreamFormat == STD_MPEG4 || encOP.bitstreamFormat == STD_AVC)
			header_buf = osal_malloc(CONTAINER_HEADER_SIZE);
	}
#endif /* SUPPORT_FFMPEG_DEMUX */

	if( encOP.bitstreamFormat == STD_MPEG4 ) 
	{
		encHeaderParam.headerType = VOS_HEADER;
		encHeaderParam.size = vbStream.size;
		ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
		if (ret != RETCODE_SUCCESS)
		{
			VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOS_HEADER failed Error code is 0x%x \n", ret);			
			goto ERR_ENC_OPEN;
		}
		if( encOP.ringBufferEnable == 0 )
		{
#ifdef SUPPORT_FFMPEG_DEMUX
			if (encConfig.en_container)
				container_copy_header_from_bitstream_buffer(coreIdx, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian, header_buf, &header_pos);
			else 
#endif
			{
				if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian))
					goto ERR_ENC_INIT;
			}
		}			
		encHeaderParam.headerType = VOL_HEADER;
		encHeaderParam.size = vbStream.size;
		ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
		if (ret != RETCODE_SUCCESS)
		{
			VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOL_HEADER failed Error code is 0x%x \n", ret);			
			goto ERR_ENC_OPEN;
		}
		if( encOP.ringBufferEnable == 0 )
		{
#ifdef SUPPORT_FFMPEG_DEMUX
			if (encConfig.en_container)
				container_copy_header_from_bitstream_buffer(coreIdx, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian, header_buf, &header_pos);
			else
#endif
			{
				if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian))
					goto ERR_ENC_INIT;
			}
		}
	}
	else if( encOP.bitstreamFormat == STD_AVC ) 
	{
		encHeaderParam.headerType = SPS_RBSP;
		if (encOP.ringBufferEnable == 0)
			encHeaderParam.buf  = vbStream.phys_addr;
		encHeaderParam.size = vbStream.size;
		
		ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
		if (ret != RETCODE_SUCCESS)
		{
			VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for SPS_RBSP failed Error code is 0x%x \n", ret);			
			goto ERR_ENC_OPEN;
		}
		if (encOP.ringBufferEnable == 0)
		{
#ifdef SUPPORT_FFMPEG_DEMUX
			if (encConfig.en_container)
				container_copy_header_from_bitstream_buffer(coreIdx, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian, header_buf, &header_pos);
			else
#endif
			{
				if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian))
					goto ERR_ENC_INIT;
			}
		}				
		
		encHeaderParam.headerType = PPS_RBSP;
		encHeaderParam.buf		= vbStream.phys_addr;
		encHeaderParam.pBuf	   = (BYTE *)vbStream.virt_addr;
		encHeaderParam.size	   = vbStream.size;
		ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
		if (ret != RETCODE_SUCCESS)
		{
			VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for PPS_RBSP failed Error code is 0x%x \n", ret);			
			goto ERR_ENC_OPEN;
		}
		if( encOP.ringBufferEnable == 0 )
		{
#ifdef SUPPORT_FFMPEG_DEMUX
			if (encConfig.en_container)
				container_copy_header_from_bitstream_buffer(coreIdx, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian, header_buf, &header_pos);
			else
#endif
			{
			if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encHeaderParam.buf, encHeaderParam.size, encOP.streamEndian))
				goto ERR_ENC_INIT;
			}
		}

	}



	printf("Enc Start : Press any key to stop.\n" );

	while( 1 ) 
	{
		if( osal_kbhit() )
			key = osal_getch();

		if( key )
		{
			if( key == '\r' || key == '\n')
			{
				int optionNum=0;
				printf("\n	0: stop encoding\n");
				printf("	1: RC Parameter change\n");
				printf("	others: continue\n");
				scanf("%d", &optionNum);
				if(optionNum==0)
					break;
				if(optionNum==1)
					changeRcParaTest(coreIdx, handle, fpBitstrm, &encParam, &encHeaderParam, &encConfig, &encOP);
				key = 0;
			}
			else
				break;
		}

		srcFrameIdx = (frameIdx%ENC_SRC_BUF_NUM);		
		if( !LoadYuvImageHelperFormat(coreIdx, fpYuv, pYuv, 
			&fbSrc[srcFrameIdx], 
			mapCfg,
			encOP.picWidth, 
			encOP.picHeight, 
			srcFrameStride, encOP.cbcrInterleave, srcFrameFormat,
			encOP.frameEndian) )
			break;	// must break to read last bitstream buffer
#ifdef ENC_SOURCE_FRAME_DISPLAY
		SaveYuvImageHelperFormat(coreIdx, NULL, &fbSrc[srcFrameIdx], mapCfg, pYuv, 
			rcMixer, encOP.cbcrInterleave, srcFrameFormat, encOP.frameEndian);				
#ifdef SUPPORT_SW_MIXER			
		sw_mixer_draw((coreIdx*MAX_NUM_VPU_CORE)+instIdx, 0, 0, srcFrameStride, srcFrameHeight,  imageFormat, pYuv);							
#endif
		DisplayMixer(&fbSrc[srcFrameIdx], encOP.picWidth, encOP.picHeight);
#endif
		encParam.sourceFrame = &fbSrc[srcFrameIdx];

		if( encOP.ringBufferEnable == 0)
		{
			encParam.picStreamBufferAddr = vbStream.phys_addr;	// can set the newly allocated buffer.
			encParam.picStreamBufferSize = vbStream.size;
		}


#ifdef TEST_SKIP_PIC
		{
			int picIdx = encOP.EncStdParam.avcParam.fieldFlag ? 2*frameIdx+fieldDone : frameIdx;
			encParam.skipPicture = 0;
			for (i=0; i<MAX_PIC_SKIP_NUM; i++)
			{
				if (encConfig.skip_pic_nums[i] > 0 && encConfig.skip_pic_nums[i] == picIdx)
				{
					encParam.skipPicture = 1;
					break;
				}
			}
		}
#endif

#ifdef SUPPORT_FFMPEG_DEMUX
		//for making IDR
		if (encConfig.en_container )
		{
		   if ( encOP.gopSize != 0 )
		   {
			   if (frameIdx % encOP.gopSize == 0)
				   encParam.forceIPicture = 1;
			   else
				   encParam.forceIPicture = 0;
		   }
		}
#endif


		// Start encoding a frame.	
		ret = VPU_EncStartOneFrame(handle, &encParam);
		if( ret != RETCODE_SUCCESS )
		{
			VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
			goto ERR_ENC_OPEN;
		}

		timeout_count = 0;
		while(osal_kbhit() == 0) 
		{
			int_reason = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);
			if (int_reason == (int)-1)
			{
				if (timeout_count*VPU_WAIT_TIME_OUT > VPU_ENC_TIMEOUT)
				{
					VLOG(ERR, "Error : encoder timeout happened\n");
					VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
					break;
				}
				int_reason = 0;
				timeout_count++;
			}

			if (encOP.ringBufferEnable == 1) 
			{
				ret = ReadBsRingBufHelper(coreIdx, handle, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, STREAM_READ_SIZE, encOP.streamEndian);
				if (ret != RETCODE_SUCCESS) 
				{
					VLOG(ERR, "ReadBsRingBufHelper failed Error code is 0x%x \n", ret );
					goto ERR_ENC_OPEN;
				}
			}

			if (encOP.ringBufferEnable == 0 && encOP.lineBufIntEn) 
			{
				if (int_reason & (1<<INT_BIT_BIT_BUF_FULL))
				{
#ifdef SUPPORT_FFMPEG_DEMUX
						if (encConfig.en_container)
							container_write_es(coreIdx, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 
									encOP.streamEndian, oc, video_st, 
									header_buf, &header_pos, encOP.bitstreamFormat, outputInfo.picType);
						else
#endif
						{
							if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, encOP.streamEndian))
							goto ERR_ENC_OPEN;
						}
				}					
			}


			if (int_reason)
			{
				VPU_ClearInterrupt(coreIdx);
				if (int_reason & (1<<INT_BIT_PIC_RUN)) 
					break;
			}
		}


		ret = VPU_EncGetOutputInfo(handle, &outputInfo);
		if (ret != RETCODE_SUCCESS)
		{
#ifdef SUPPORT_MEM_PROTECT
			if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
				PrintMemoryAccessViolationReason(coreIdx, NULL);
#endif
			VLOG(ERR, "VPU_EncGetOutputInfo failed Error code is 0x%x \n", ret );
			goto ERR_ENC_OPEN;
		}

		VLOG(TRACE, "Enc: %d || picType %d || reconIdx %d || rdPtr %x || wrPtr %x\n", 
					frameIdx, outputInfo.picType, outputInfo.reconFrameIndex, outputInfo.rdPtr, outputInfo.wrPtr);			
		if (encOP.ringBufferEnable == 0)
		{
			if (outputInfo.bitstreamWrapAround == 1)
			{
				VLOG(WARN, "Warnning!! BitStream buffer wrap arounded. prepare more large buffer \n");
			}
			if (outputInfo.bitstreamSize == 0)
			{
				printf("ERROR!!! bitstreamsize = 0 \n");
			}
#ifdef SUPPORT_FFMPEG_DEMUX
			if (encConfig.en_container)
			{
				container_write_es(coreIdx, outputInfo.bitstreamBuffer, outputInfo.bitstreamSize, 
						encOP.streamEndian, oc, video_st, 
						header_buf, &header_pos, encOP.bitstreamFormat, outputInfo.picType);
			}
			else
#endif
			{
				if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, outputInfo.bitstreamBuffer, outputInfo.bitstreamSize, encOP.streamEndian))
					break;
			}
		}


#ifdef ENC_RECON_FRAME_DISPLAY
		SaveYuvImageHelperFormat(coreIdx, NULL, &outputInfo.reconFrame, mapCfg, pYuv, 
			rcMixer, encOP.cbcrInterleave, framebufFormat, encOP.frameEndian);	
#ifdef SUPPORT_SW_MIXER
		sw_mixer_draw((coreIdx*MAX_NUM_VPU_CORE)+instIdx, 0, 0, framebufStride, framebufHeight, YUV_FORMAT_I420, pYuv);	
#endif
#ifdef CNM_FPGA_PLATFORM
		DisplayMixer(&outputInfo.reconFrame, 
			(encConfig.rotAngle==90||encConfig.rotAngle ==270)?encOP.picHeight:encOP.picWidth,
			(encConfig.rotAngle==90||encConfig.rotAngle ==270)?encOP.picWidth:encOP.picHeight);
#endif
#endif		

		frameIdx++;
		if (frameIdx > encConfig.outNum)
			break;
		//VLOG(INFO, "Encoded frame index: %d", frameIdx);
	}

	if( encOP.ringBufferEnable == 1 )
	{
		ret = ReadBsRingBufHelper(coreIdx, handle, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 0, encOP.streamEndian );
		if( ret != RETCODE_SUCCESS )
		{
			VLOG(ERR, "ReadBsRingBufHelper failed Error code is 0x%x \n", ret );
			goto ERR_ENC_OPEN;
		}
	}


ERR_ENC_OPEN:
	// Now that we are done with encoding, close the open instance.
	VPU_EncClose(handle);
	
	VLOG(INFO, "\nEnc End. Tot Frame %d\n" , frameIdx );

ERR_ENC_INIT:

	if (vbStream.size)
		vdi_free_dma_memory(coreIdx, &vbStream);
	if (vbReport.size)
		vdi_free_dma_memory(coreIdx, &vbReport);
	if( pYuv )
		free( pYuv );
#ifdef SUPPORT_FFMPEG_DEMUX
	if (encConfig.en_container && !encConfig.ringBufferEnable)
	{
		container_deinit(oc);
		if(header_buf)
			free(header_buf);
	}
	else
	{
		if( fpBitstrm )
			osal_fclose(fpBitstrm);
	}
#else
	if( fpBitstrm )
		osal_fclose(fpBitstrm);
#endif
	if( fpYuv )
		osal_fclose(fpYuv);
#ifdef SUPPORT_SW_MIXER
	sw_mixer_close((coreIdx*MAX_NUM_VPU_CORE)+instIdx);
#endif
	VPU_DeInit(coreIdx);
	return suc;
}

#ifdef SUPPORT_FFMPEG_DEMUX
int FileTranscodingTest(DecConfigParam *decCfgParam, EncConfigParam *encCfgParam)
{
	DecHandle dec_handle = {0};
	
	DecParam dec_param	= {0};
	DecOpenParam dec_open_param = {0};
	DecConfigParam dec_config_param = {0};
	
	DecInitialInfo dec_initial_info = {0};
	DecOutputInfo dec_output_info = {0};
	
	vpu_buffer_t	dec_vb_stream = {0};
	
	FrameBufferAllocInfo fb_alloc_info = {0};
	
	SecAxiUse sec_axi_use = {0};
	RetCode ret_code = RETCODE_SUCCESS;		
	
	int32_t i = 0, decode_finish = 0, err = 0;
	int32_t frame_buffer_width = 0, frame_buffer_height = 0;
	int32_t frame_buffer_size = 0, frame_buffer_format = FORMAT_420; 
	int32_t map_type = 0;
	int32_t frame_index = 0, pp_index=0;
	int32_t kb_hit_ret = 0,  total_num_of_err_mbs = 0;
	int32_t prev_disp_index = -1;
	int32_t seq_inited = 0, seq_filled = 0, reuse_chunk = 0;
	int32_t horizontal_scale_factor = 0, vertical_scale_factor = 0;
	int32_t scaled_width = 0, scaled_height = 0;
	int32_t random_access = 0, random_access_pos = 0;
	int32_t ppu_enable = 0;
	int32_t int_reason = 0;
	int32_t bitstream_fill_size = 0;
	int32_t size = 0;
	int32_t instance_index = 0, core_index = 0;
	int32_t rot_buffer_width = 0, rot_buffer_height = 0, rot_stride = 0;

	FrameBuffer fbPPU[MAX_ROT_BUF_NUM] = {0};
	FrameBuffer frame_buffer_user[MAX_REG_FRAME] = {0};
	FrameBuffer *p_user_frame = NULL;

	
	TiledMapConfig map_config = {0};
	DRAMConfig dram_config = {0};
	MaverickCacheConfig dec_cache_config = {0};
	Rect rect_prev_display={0};

	//Theora
	tho_parser_t *tho_parser = {0}; 
	uint8_t *p_tho_stream	= NULL;
	uint32_t tho_seq_size = 0;

	AVFormatContext *ic;
	AVPacket pkt1, *pkt=&pkt1;
	AVCodecContext *ctx_video = {0};
	
	int32_t index_video = 0;
	int32_t chunk_index = 0;
	int32_t seq_header_size = 0;
	int32_t chunk_size = 0;
	int32_t pic_header_size = 0;

	uint8_t *p_chunk_data = NULL;
	uint8_t *p_seq_header = NULL;
	uint8_t *p_pic_header = NULL;
	
	const char *p_file_name = NULL;
	//[Ken] Encoder Variable Strat///////////////
	//////////////////////////////////////////////////
	EncHandle enc_handle = {0};
	
	EncParam enc_param	= {0};
	EncOpenParam	enc_open_param = {0};
	EncConfigParam enc_config_param = {0};
	EncHeaderParam enc_header_param = {0};
	
	EncInitialInfo enc_initial_info = {0};
	EncOutputInfo enc_output_info = {0};
	
	MaverickCacheConfig enc_cache_config = {0};
	MirrorDirection mirror_direction = MIRDIR_NONE;
	int32_t timeout_count = 0;
	int32_t suc = 1, key = 0;
#ifdef SUPPORT_MEM_PROTECT
	WriteMemProtectCfg mem_protect_config = {0};
#endif
	vpu_buffer_t	vb_report = {0};
	osal_file_t		fp_bit_stream = NULL;
	vpu_buffer_t	enc_vb_stream	= {0};
	
	int32_t src_frame_width = 0, src_frame_height = 0, src_frame_stride = 0;
	int32_t src_frame_format = 0;
	int32_t frame_buffer_stride = 0, req_frame_buffer_count = 0;

	uint8_t *p_yuv = NULL;

	//[Ken] Encoder Variable End///////////////
	/////////////////////////////////////////////////

	char *header_buf = NULL;				   //container header buffer
	int header_pos = 0;					 //container header buffer position
	AVOutputFormat *fmt = NULL;		 //ffmpeg
	AVFormatContext *oc = NULL;		 //ffmpeg Format I/O context.(must set to NULL)
	AVStream *video_st = NULL;		  //ffmpeg Stream structure	 

	av_register_all();

	memcpy(&dec_config_param, decCfgParam, sizeof(DecConfigParam));

	p_file_name = dec_config_param.bitstreamFileName;
	instance_index = dec_config_param.instNum;
	core_index = dec_config_param.coreIdx;

	ic = avformat_alloc_context();
	if (!ic)
		return 0;

	ic->flags |= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
	
	err = avformat_open_input(&ic, p_file_name, NULL,  NULL);
	if (err < 0)
	{
		VLOG(ERR, "%s: could not open file\n", p_file_name);
		av_free(ic);
		return 0;
	}

	err = avformat_find_stream_info(ic,  NULL);
	if (err < 0) 
	{
		VLOG(ERR, "%s: could not find stream information\n", p_file_name);
		goto ERR_TRANS_INIT;
	}


	av_dump_format(ic, 0, p_file_name, 0);

	// find video stream index
	index_video = -1;
	index_video = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (index_video < 0) 
	{
		err = -1;
		VLOG(ERR, "%s: could not find video stream information\n", p_file_name);
		goto ERR_TRANS_INIT;
	}

	ctx_video = ic->streams[index_video]->codec;  

	p_seq_header = (uint8_t*)osal_malloc(ctx_video->extradata_size+MAX_CHUNK_HEADER_SIZE);	// allocate more buffer to fill the vpu specific header.
	if (!p_seq_header)
	{
		VLOG(ERR, "fail to allocate the p_seq_header buffer\n");
		goto ERR_TRANS_INIT;
	}

	p_pic_header = (uint8_t*)osal_malloc(MAX_CHUNK_HEADER_SIZE);
	if (!p_pic_header)
	{
		VLOG(ERR, "fail to allocate the p_pic_header buffer\n");
		goto ERR_TRANS_INIT;
	}


	ret_code = VPU_Init(core_index);	// allocate workbuffer in vpuapi through driver insterface.

	if (ret_code != RETCODE_SUCCESS && 
		ret_code != RETCODE_CALLED_BEFORE) 
	{
		VLOG(ERR, "VPU_Init failed Error code is 0x%x \n", ret_code );
		goto ERR_TRANS_INIT;
	}

	CheckVersion(core_index);

	dec_open_param.bitstreamFormat = (CodStd)fourCCToCodStd(ctx_video->codec_tag);
	if (dec_open_param.bitstreamFormat == -1)
		dec_open_param.bitstreamFormat = (CodStd)codecIdToCodStd(ctx_video->codec_id);

	if (dec_open_param.bitstreamFormat == -1)
	{
		VLOG(ERR, "can not support video format in VPU tag=%c%c%c%c, codec_id=0x%x \n", ctx_video->codec_tag>>0, ctx_video->codec_tag>>8, ctx_video->codec_tag>>16, ctx_video->codec_tag>>24, ctx_video->codec_id );
		goto ERR_TRANS_INIT;
	}

	dec_vb_stream.size = STREAM_BUF_SIZE;//0x100000
	dec_vb_stream.size = ((dec_vb_stream.size+1023)&~1023);

	if (vdi_allocate_dma_memory(core_index, &dec_vb_stream) < 0)
	{
		VLOG(ERR, "fail to allocate bitstream buffer\n" );
		goto ERR_TRANS_INIT;
	}

	dec_open_param.bitstreamBuffer = dec_vb_stream.phys_addr; 
	dec_open_param.bitstreamBufferSize = dec_vb_stream.size;
	dec_open_param.mp4DeblkEnable = 0;

	dec_open_param.mp4Class = fourCCToMp4Class(ctx_video->codec_tag);
	if (dec_open_param.mp4Class == -1)
		dec_open_param.mp4Class = codecIdToMp4Class(ctx_video->codec_id);

	if(dec_open_param.bitstreamFormat == STD_THO || dec_open_param.bitstreamFormat == STD_VP3)
	{
		theora_parser_init((void **)&tho_parser);

		p_tho_stream = (uint8_t*)osal_malloc(SIZE_THO_STREAM);
		if (!p_tho_stream)
		{
			VLOG(ERR, "p_tho_stream buffer osal_malloc fail\n");
			goto ERR_TRANS_INIT;
		}
	}

	dec_open_param.tiled2LinearEnable = (dec_config_param.mapType>>4)&0x1;
	map_type = dec_config_param.mapType & 0xf;
	

    if (map_type) 
    {
        dec_open_param.wtlEnable = dec_config_param.wtlEnable;
		if (dec_open_param.wtlEnable)
		{
            dec_config_param.rotAngle;
            dec_config_param.mirDir;
            dec_config_param.useRot = 0;
            dec_config_param.useDering = 0;
			dec_open_param.mp4DeblkEnable = 0;
			dec_open_param.tiled2LinearEnable = 0;
		}
    }

	dec_open_param.cbcrInterleave = CBCR_INTERLEAVE;

	if (map_type == TILED_FRAME_MB_RASTER_MAP ||
		map_type == TILED_FIELD_MB_RASTER_MAP) 
				dec_open_param.cbcrInterleave = 1;
	dec_open_param.bwbEnable = VPU_ENABLE_BWB;
	dec_open_param.frameEndian  = VPU_FRAME_ENDIAN;
	dec_open_param.streamEndian = VPU_STREAM_ENDIAN;
	dec_open_param.bitstreamMode = dec_config_param.bitstreamMode;

	if (dec_config_param.useRot || dec_config_param.useDering || dec_open_param.tiled2LinearEnable) 
		ppu_enable = 1;
	else
		ppu_enable = 0;

	ret_code = VPU_DecOpen(&dec_handle, &dec_open_param);
	if (ret_code != RETCODE_SUCCESS) 
	{
		VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret_code );
		goto ERR_TRANS_INIT;
	}  	

	ret_code = VPU_DecGiveCommand(dec_handle, GET_DRAM_CONFIG, &dram_config);
	if( ret_code != RETCODE_SUCCESS )
	{
		VLOG(ERR, "VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret_code );
		goto ERR_DEC_OPEN;
	}

	VLOG(INFO, "Dec Start : Press enter key to show menu.\n" );
	VLOG(INFO, "		  : Press space key to stop.\n" );

	seq_inited = 0;
	seq_filled = 0;
	bitstream_fill_size = 0;
	reuse_chunk = 0;

	while(1)
	{
		if (osal_kbhit()) 
		{
			kb_hit_ret = osal_getch();
			osal_fflush(stdout);			
		}		

		if (kb_hit_ret)
		{
			if (kb_hit_ret == '\r' || kb_hit_ret == '\n')
			{
				ret_code = UI_GetUserCommand(core_index, dec_handle, &dec_param, &random_access_pos);
				kb_hit_ret = 0;
				if (ret_code == UI_CMD_SKIP_FRAME_LIFECYCLE)
					continue;
				else if (ret_code == UI_CMD_RANDOM_ACCESS)
					random_access = 1;									
			}
			else if (kb_hit_ret == ' ')
				break;
		}

		seq_header_size = 0;
		pic_header_size = 0;

		if (dec_open_param.bitstreamMode == BS_MODE_PIC_END)
		{
			if (reuse_chunk)
			{
				reuse_chunk = 0;
				goto FLUSH_BUFFER;			
			}
			VPU_DecSetRdPtr(dec_handle, dec_open_param.bitstreamBuffer, 1);	
		}

		av_init_packet(pkt);

		err = av_read_frame(ic, pkt);

		if (err < 0) 
		{
			if (pkt->stream_index == index_video)
				chunk_index++;	

			if (err==AVERROR_EOF || url_feof(ic->pb)) 
			{
				bitstream_fill_size = VPU_GBU_SIZE*2;
				VPU_DecUpdateBitstreamBuffer(dec_handle, STREAM_END_SIZE);	//tell VPU to reach the end of stream. starting flush decoded output in VPU
				chunk_size = 0;
				goto FLUSH_BUFFER;
			}
			continue;
		}

		if (pkt->stream_index != index_video)
			continue;
		
		if (random_access)
		{
			int32_t tot_frame = 0;
			int32_t pos_frame = 0;

			tot_frame = (int32_t)ic->streams[index_video]->nb_frames;
			pos_frame = (tot_frame/100) * random_access_pos;
			
			if (pos_frame < ctx_video->frame_number)
				continue;			
			else
			{
				random_access = 0;

				if (dec_open_param.bitstreamMode != BS_MODE_PIC_END)
				{
					//clear all frame buffer except current frame
					for(i=0; i<req_frame_buffer_count; i++)
					{
						if (i != prev_disp_index)
							VPU_DecClrDispFlag(dec_handle, i);
					}


					//Clear all display buffer before Bitstream & Frame buffer flush
					ret_code = VPU_DecFrameBufferFlush(dec_handle);
					if( ret_code != RETCODE_SUCCESS )
					{
						VLOG(ERR, "VPU_DecFrameBufferFlush failed Error code is 0x%x \n", ret_code );
						goto ERR_ENC_OPEN;
					}
				}
			}
		}

		p_chunk_data = pkt->data;
		chunk_size = pkt->size;

		if (!seq_inited && !seq_filled)
		{
			seq_header_size = BuildSeqHeader(p_seq_header, dec_open_param.bitstreamFormat, ic->streams[index_video]);	// make sequence data as reference file header to support VPU decoder.
			switch(dec_open_param.bitstreamFormat)
			{
			case STD_THO:
				{
					ret_code = (RetCode)tho_parser->open(tho_parser->handle, p_seq_header, seq_header_size, (int32_t *)&dec_initial_info.thoScaleInfo);
					if(ret_code < 0)
					{
						// Failed to open container
						VLOG(ERR, "Parser failure - Theora header parsing error\n");
						goto ERR_ENC_OPEN;
					}				
					tho_seq_size = theora_make_stream((void *)tho_parser->handle, p_tho_stream, SEQ_INIT);
				}				
				break;
			case STD_VP3:
				{
					AVCodecContext *codec = ic->streams[index_video]->codec;
					ret_code = (RetCode)tho_parser->open(tho_parser->handle, p_seq_header, 0,  (int32_t *)&dec_initial_info.thoScaleInfo);
					if(ret_code < 0)
					{
						// Failed to open container
						VLOG(ERR, "Parser failure - VP3 open error\n");
						goto ERR_ENC_OPEN;
					}			
				}
				break;
			default:
				{
					size = WriteBsBufFromBufHelper(core_index, dec_handle, &dec_vb_stream, p_seq_header, seq_header_size, dec_open_param.streamEndian);
					if (size < 0)
					{
						VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
						goto ERR_ENC_OPEN;
					}
						
					bitstream_fill_size += size;
				}
				break;
			}
			seq_filled = 1;
		}
		
		
		// Build and Fill picture Header data which is dedicated for VPU 
		pic_header_size = BuildPicHeader(p_pic_header, dec_open_param.bitstreamFormat, ic->streams[index_video], pkt);				
		switch(dec_open_param.bitstreamFormat)
		{
		case STD_THO:
		case STD_VP3:
			{
				if((ret_code = tho_parser->read_frame((void *)tho_parser->handle, p_chunk_data, chunk_size)) < 0) 
					break;

				/* refer to 6.2.3.2 Macroblock order Matching,
				6.2.3.3  Macroblock Packing in Programmer User Guide */
				chunk_size = theora_make_stream((void *)tho_parser->handle, p_tho_stream + tho_seq_size, PIC_RUN);

				// Loading quantization matrices and MB packed data
				size = WriteBsBufFromBufHelper(core_index, dec_handle, &dec_vb_stream, p_tho_stream, chunk_size + tho_seq_size, dec_open_param.streamEndian);
				if (size < 0)
				{
					VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
					goto ERR_ENC_OPEN;
				}

				bitstream_fill_size += size;
				tho_seq_size = 0;
			}
			break;
		default:
			{	
				size = WriteBsBufFromBufHelper(core_index, dec_handle, &dec_vb_stream, p_pic_header, pic_header_size, dec_open_param.streamEndian);
				if (size < 0)
				{
					VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
					goto ERR_ENC_OPEN;
				}	
				bitstream_fill_size += size;

				if (dec_open_param.bitstreamFormat == STD_RV)
				{
					int32_t cSlice = p_chunk_data[0] + 1;
					int32_t nSlice =  chunk_size - 1 - (cSlice * 8);
					p_chunk_data += (1+(cSlice*8));
					chunk_size = nSlice;
				}

				size = WriteBsBufFromBufHelper(core_index, dec_handle, &dec_vb_stream, p_chunk_data, chunk_size, dec_open_param.streamEndian);
				if (size < 0)
				{
					VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
					goto ERR_ENC_OPEN;
				}

				bitstream_fill_size += size;
			}
			break;
		}

		av_free_packet(pkt);

		chunk_index++;

		if (!seq_inited)
		{ 
			ConfigSeqReport(core_index, dec_handle, dec_open_param.bitstreamFormat);
			if (dec_open_param.bitstreamMode == BS_MODE_PIC_END)
			{
				ret_code = VPU_DecGetInitialInfo(dec_handle, &dec_initial_info);
				if (ret_code != RETCODE_SUCCESS) 
				{
#ifdef SUPPORT_MEM_PROTECT
					if (ret_code == RETCODE_MEMORY_ACCESS_VIOLATION)
						PrintMemoryAccessViolationReason(core_index, NULL);
#endif
					VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret_code);
					goto ERR_ENC_OPEN;					
				}
				VPU_ClearInterrupt(core_index);
			}
			else
			{
				if((int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) != (1<<INT_BIT_BIT_BUF_EMPTY))
				{
					ret_code = VPU_DecIssueSeqInit(dec_handle);
					if (ret_code != RETCODE_SUCCESS)
					{
						VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret_code);
						goto ERR_ENC_OPEN;
					}
				}
				else
				{
					// After VPU generate the BIT_EMPTY interrupt. HOST should feed the bitstream up to 1024 in case of seq_init
					if (bitstream_fill_size < VPU_GBU_SIZE*2)
						continue;
				}
				while((kb_hit_ret = osal_kbhit()) == 0) 
				{	
					int_reason = VPU_WaitInterrupt(core_index, VPU_DEC_TIMEOUT);

					if (int_reason)
						VPU_ClearInterrupt(core_index);

					if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) 
						break;

					CheckUserDataInterrupt(core_index, dec_handle, 1, dec_open_param.bitstreamFormat, int_reason);

					if (int_reason)
					{
						if (int_reason & (1<<INT_BIT_SEQ_INIT)) 
						{
							seq_inited = 1;
							break;
						}
					}
				}
				if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY) || int_reason == -1) 
				{
					bitstream_fill_size = 0;
					continue; // go to take next chunk.
				}
				if (seq_inited)
				{
					ret_code = VPU_DecCompleteSeqInit(dec_handle, &dec_initial_info);	
					if (ret_code != RETCODE_SUCCESS)
					{
#ifdef SUPPORT_MEM_PROTECT
						if (ret_code == RETCODE_MEMORY_ACCESS_VIOLATION)
							PrintMemoryAccessViolationReason(core_index, NULL);
#endif
						if (dec_initial_info.seqInitErrReason & (1<<31)) // this case happened only ROLLBACK mode
							VLOG(ERR, "Not enough header : Parser has to feed right size of a sequence header  \n");
						VLOG(ERR, "VPU_DecCompleteSeqInit failed Error code is 0x%x \n", ret_code );
						goto ERR_ENC_OPEN;
					}			
				}
				else
				{
					VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret_code);
					goto ERR_ENC_OPEN;
				}
			}


			SaveSeqReport(core_index, dec_handle, &dec_initial_info, dec_open_param.bitstreamFormat);

			if (dec_open_param.bitstreamFormat == STD_VP8)		
			{
				// For VP8 frame upsampling infomration
				static const int32_t scale_factor_mul[4] = {1, 5, 5, 2};
				static const int32_t scale_factor_div[4] = {1, 4, 3, 1};
				horizontal_scale_factor = dec_initial_info.vp8ScaleInfo.hScaleFactor;
				vertical_scale_factor = dec_initial_info.vp8ScaleInfo.vScaleFactor;
				scaled_width = dec_initial_info.picWidth * scale_factor_mul[horizontal_scale_factor] / scale_factor_div[horizontal_scale_factor];
				scaled_height = dec_initial_info.picHeight * scale_factor_mul[vertical_scale_factor] / scale_factor_div[vertical_scale_factor];
				frame_buffer_width = ((scaled_width+15)&~15);
				if (IsSupportInterlaceMode(dec_open_param.bitstreamFormat, &dec_initial_info))
					frame_buffer_height = ((scaled_height+31)&~31); // framebufheight must be aligned by 31 because of the number of MB height would be odd in each filed picture.
				else
					frame_buffer_height = ((scaled_height+15)&~15);

				rot_buffer_width = (dec_config_param.rotAngle == 90 || dec_config_param.rotAngle == 270) ?
					((scaled_height+15)&~15) : ((scaled_width+15)&~15);
				rot_buffer_height = (dec_config_param.rotAngle == 90 || dec_config_param.rotAngle == 270) ?
					((scaled_width+15)&~15) : ((scaled_height+15)&~15);				
			}
			else
			{
				if (dec_config_param.maxWidth)
				{
					if (dec_config_param.maxWidth < dec_initial_info.picWidth)
					{
						VLOG(ERR, "maxWidth is too small\n");
						goto ERR_ENC_OPEN;
					}
					frame_buffer_width = ((dec_config_param.maxWidth+15)&~15);
				}
				else
					frame_buffer_width = ((dec_initial_info.picWidth+15)&~15);

				if (dec_config_param.maxHeight)
				{
					if (dec_config_param.maxHeight < dec_initial_info.picHeight)
					{
						VLOG(ERR, "maxHeight is too small\n");
						goto ERR_ENC_OPEN;
					}

					if (IsSupportInterlaceMode(dec_open_param.bitstreamFormat, &dec_initial_info))
						frame_buffer_height = ((dec_config_param.maxHeight+31)&~31); // framebufheight must be aligned by 31 because of the number of MB height would be odd in each filed picture.
					else
						frame_buffer_height = ((dec_config_param.maxHeight+15)&~15);
				}
				else
				{
					if (IsSupportInterlaceMode(dec_open_param.bitstreamFormat, &dec_initial_info))
						frame_buffer_height = ((dec_initial_info.picHeight+31)&~31); // framebufheight must be aligned by 31 because of the number of MB height would be odd in each filed picture.
					else
						frame_buffer_height = ((dec_initial_info.picHeight+15)&~15);
				}
				rot_buffer_width = (dec_config_param.rotAngle == 90 || dec_config_param.rotAngle == 270) ? 
					((dec_initial_info.picHeight+15)&~15) : ((dec_initial_info.picWidth+15)&~15);
				rot_buffer_height = (dec_config_param.rotAngle == 90 || dec_config_param.rotAngle == 270) ? 
					((dec_initial_info.picWidth+15)&~15) : ((dec_initial_info.picHeight+15)&~15);
			}
			rot_stride = rot_buffer_width;
			frame_buffer_stride = frame_buffer_width;
			frame_buffer_format = FORMAT_420;	
			frame_buffer_size = VPU_GetFrameBufSize(frame_buffer_stride, frame_buffer_height, map_type, frame_buffer_format, &dram_config);
			
			p_yuv = (uint8_t*)osal_malloc(frame_buffer_size);
			if (!p_yuv) 
			{
				VLOG(ERR, "Fail to allocation memory for display buffer\n");
				goto ERR_ENC_OPEN;
			}

			sec_axi_use.useBitEnable  = USE_BIT_INTERNAL_BUF;
			sec_axi_use.useIpEnable   = USE_IP_INTERNAL_BUF;
			sec_axi_use.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
			sec_axi_use.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
			sec_axi_use.useBtpEnable  = USE_BTP_INTERNAL_BUF;
			sec_axi_use.useOvlEnable  = USE_OVL_INTERNAL_BUF;
			VPU_DecGiveCommand(dec_handle, SET_SEC_AXI, &sec_axi_use);
			// MaverickCache configure			
			MaverickCache2Config(
				&dec_cache_config, 
				1, //decoder
				dec_open_param.cbcrInterleave, // cb cr interleave				
				dec_config_param.frameCacheBypass,
				dec_config_param.frameCacheBurst,
				dec_config_param.frameCacheMerge,
				map_type,
				dec_config_param.frameCacheWayShape);
			VPU_DecGiveCommand(dec_handle, SET_CACHE_CONFIG, &dec_cache_config);
			req_frame_buffer_count = dec_initial_info.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;

			// Register frame buffers requested by the decoder.
			ret_code = VPU_DecRegisterFrameBuffer(dec_handle, p_user_frame, req_frame_buffer_count, frame_buffer_stride, frame_buffer_height, map_type); // frame map type (can be changed before register frame buffer)

			if (ret_code != RETCODE_SUCCESS) 
			{
				VLOG(ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret_code);
				goto ERR_ENC_OPEN;
			}

			if (ppu_enable) 
			{
				pp_index = 0;

				fb_alloc_info.format         = frame_buffer_format;
				fb_alloc_info.cbcrInterleave = dec_open_param.cbcrInterleave;
				if (dec_open_param.tiled2LinearEnable)
					fb_alloc_info.mapType = LINEAR_FRAME_MAP;
				else
					fb_alloc_info.mapType = map_type;

				fb_alloc_info.stride    = rot_stride;
				fb_alloc_info.height    = rot_buffer_height;
				fb_alloc_info.num       = MAX_ROT_BUF_NUM;
				fb_alloc_info.endian    = dec_open_param.frameEndian;
				fb_alloc_info.type      = FB_TYPE_PPU;
				ret_code = VPU_DecAllocateFrameBuffer(dec_handle, fb_alloc_info, fbPPU);
				if( ret_code != RETCODE_SUCCESS )
				{
					VLOG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret_code );
					goto ERR_ENC_OPEN;
				}

				if (dec_config_param.useRot)
				{
					VPU_DecGiveCommand(dec_handle, SET_ROTATION_ANGLE, &(dec_config_param.rotAngle));
					VPU_DecGiveCommand(dec_handle, SET_MIRROR_DIRECTION, &(dec_config_param.mirDir));
				}

				VPU_DecGiveCommand(dec_handle, SET_ROTATOR_STRIDE, &rot_stride);

				if (dec_config_param.useRot) 
				{
					VPU_DecGiveCommand(dec_handle, ENABLE_ROTATION, 0);
					VPU_DecGiveCommand(dec_handle, ENABLE_MIRRORING, 0);
				}

				if (dec_config_param.useDering)
					VPU_DecGiveCommand(dec_handle, ENABLE_DERING, 0);
			}
			VPU_DecGiveCommand(dec_handle, GET_TILEDMAP_CONFIG, &map_config);

			InitMixerInt();

			seq_inited = 1;
			if (dec_open_param.bitstreamMode == BS_MODE_PIC_END)
				VPU_DecSetRdPtr(dec_handle, dec_open_param.bitstreamBuffer+seq_header_size, 0);	

			//this point - ken
			//[Ken] Before VPU_Init, set Encoder Start //////
			////////////////////////////////////////////////////////
			memcpy(&enc_config_param, encCfgParam, sizeof(EncConfigParam));

			//encoder and decoder has same map_type
			enc_config_param.mapType = dec_config_param.mapType;
			if(ppu_enable)
			{
				enc_config_param.picWidth = fb_alloc_info.stride;
				enc_config_param.picHeight = fb_alloc_info.height;
			}
			else
			{
				enc_config_param.picWidth = dec_initial_info.picWidth;
				enc_config_param.picHeight = dec_initial_info.picHeight;
			}
			if ( enc_config_param.en_container )
			{
				if ( dec_initial_info.fRateDenominator != 0 )
					enc_config_param.container_frame_rate = (int)((float)(dec_initial_info.fRateNumerator/dec_initial_info.fRateDenominator)+0.5);
				if (enc_config_param.container_frame_rate < 1 )
				{
					VLOG(ERR, "Invalid frame rate calculating. Frame rate will be setted to 30FPS\n");
					enc_config_param.container_frame_rate = 30;
				}
			}
			if(ic->bit_rate > 0)
				enc_config_param.kbps = ic->bit_rate/1000;
			else
				enc_config_param.kbps = dec_initial_info.bitRate;

			// if you use container, don't open a file pointer 
			// because ffmpeg will open the file pointer in container_init function.
			if(enc_config_param.en_container && !enc_config_param.ringBufferEnable )
			{
				// Check container file validity
				fp_bit_stream = osal_fopen(enc_config_param.bitstreamFileName, "wb");
				if (!fp_bit_stream)
				{
					 VLOG(ERR, "Can't open bitstream file %s \n", enc_config_param.bitstreamFileName );
					 goto ERR_DEC_OPEN;
				}
				else
					osal_fclose(fp_bit_stream);
				fp_bit_stream = NULL;
			}
			else
			{
				fp_bit_stream = osal_fopen(enc_config_param.bitstreamFileName, "wb");
				if( !fp_bit_stream ) 
				{
					VLOG(ERR, "Can't open bitstream file %s \n", enc_config_param.bitstreamFileName );
					goto ERR_DEC_OPEN;
				}
			}


			// Fill parameters for encoding.
			map_type = (enc_config_param.mapType & 0x0f);
			enc_open_param.linear2TiledEnable = (enc_config_param.mapType>>4)&0x1;

			if( enc_config_param.stdMode == 0 )
				enc_open_param.bitstreamFormat = STD_MPEG4;
			else if( enc_config_param.stdMode == 1 )
				enc_open_param.bitstreamFormat = STD_H263;
			else if( enc_config_param.stdMode == 2 )
				enc_open_param.bitstreamFormat = STD_AVC;
			else
			{
				VLOG(ERR, "Invalid bitstream format mode \n" );
				goto ERR_DEC_OPEN;
			}

			if (strlen(enc_config_param.cfgFileName) != 0) 
				ret_code = (RetCode)getEncOpenParam(&enc_open_param, &enc_config_param, NULL);
			else 
				ret_code = (RetCode)getEncOpenParamDefault(&enc_open_param, &enc_config_param);

			if (ret_code == 0)
				goto ERR_DEC_OPEN;		


			src_frame_width = ((enc_open_param.picWidth+15)&~15);
			src_frame_stride = src_frame_width;

			src_frame_format = FORMAT_420;
			frame_buffer_format = FORMAT_420;
			src_frame_height = ((enc_open_param.picHeight+15)&~15);

			frame_buffer_width = (enc_config_param.rotAngle==90||enc_config_param.rotAngle ==270)?src_frame_height:src_frame_width;
			frame_buffer_height = (enc_config_param.rotAngle==90||enc_config_param.rotAngle ==270)?src_frame_width:src_frame_height;
			frame_buffer_stride = frame_buffer_width;


			enc_vb_stream.size = STREAM_BUF_SIZE;
			if (vdi_allocate_dma_memory(core_index, &enc_vb_stream) < 0)
			{
				VLOG(ERR, "fail to allocate bitstream buffer\n" );
				goto ERR_DEC_OPEN;
			}

			enc_open_param.bitstreamBuffer = enc_vb_stream.phys_addr;
			enc_open_param.bitstreamBufferSize = enc_vb_stream.size;
			enc_open_param.ringBufferEnable = enc_config_param.ringBufferEnable;
			enc_open_param.cbcrInterleave = CBCR_INTERLEAVE;
			if (map_type == TILED_FRAME_MB_RASTER_MAP ||
				map_type == TILED_FIELD_MB_RASTER_MAP) 
					enc_open_param.cbcrInterleave = 1;
			
			enc_open_param.frameEndian = VPU_FRAME_ENDIAN;
			enc_open_param.streamEndian = VPU_STREAM_ENDIAN;
			enc_open_param.bwbEnable = VPU_ENABLE_BWB;
			enc_open_param.lineBufIntEn = enc_config_param.lineBufIntEn;
			enc_open_param.coreIdx = core_index;


			// Open an instance and get initial information for encoding.
			ret_code = VPU_EncOpen(&enc_handle, &enc_open_param);
			if( ret_code != RETCODE_SUCCESS )
			{
				VLOG(ERR, "VPU_EncOpen failed Error code is 0x%x \n", ret_code );
				goto ERR_DEC_OPEN;
			}

			if( enc_config_param.useRot == 1 )
			{
				VPU_EncGiveCommand(enc_handle, ENABLE_ROTATION, 0);
				VPU_EncGiveCommand(enc_handle, ENABLE_MIRRORING, 0);
				VPU_EncGiveCommand(enc_handle, SET_ROTATION_ANGLE, &enc_config_param.rotAngle);
				mirror_direction = (MirrorDirection)enc_config_param.mirDir;
				VPU_EncGiveCommand(enc_handle, SET_MIRROR_DIRECTION, &mirror_direction);
			}

			ret_code = VPU_EncGetInitialInfo(enc_handle, &enc_initial_info);
			if( ret_code != RETCODE_SUCCESS )
			{
				VLOG(ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret_code );
				goto ERR_ENC_OPEN;
			}

			VPU_ClearInterrupt(core_index);

			req_frame_buffer_count = enc_initial_info.minFrameBufferCount;

			// allocate frame buffers for source frame
			// TODO
			sec_axi_use.useBitEnable = USE_BIT_INTERNAL_BUF;
			sec_axi_use.useIpEnable = USE_IP_INTERNAL_BUF;
			sec_axi_use.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
			sec_axi_use.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
			sec_axi_use.useBtpEnable = USE_BTP_INTERNAL_BUF;
			sec_axi_use.useOvlEnable = USE_OVL_INTERNAL_BUF;
			VPU_EncGiveCommand(enc_handle, SET_SEC_AXI, &sec_axi_use);
			// MaverickCache configure
			MaverickCache2Config(
				&enc_cache_config, 
				0, //encoder
				enc_open_param.cbcrInterleave, // cb cr interleave
				enc_config_param.frameCacheBypass,
				enc_config_param.frameCacheBurst,
				enc_config_param.frameCacheMerge,
				map_type,
				enc_config_param.frameCacheWayShape);
			VPU_EncGiveCommand(enc_handle, SET_CACHE_CONFIG, &enc_cache_config);

			ret_code = VPU_EncRegisterFrameBuffer(enc_handle, p_user_frame, req_frame_buffer_count, frame_buffer_stride, frame_buffer_height, map_type);
			if( ret_code != RETCODE_SUCCESS )
			{
				VLOG(ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret_code );
				goto ERR_ENC_OPEN;
			}
			VPU_EncGiveCommand(enc_handle, GET_TILEDMAP_CONFIG, &map_config);


			enc_param.forceIPicture = 0;
			enc_param.skipPicture   = 0;
			enc_param.quantParam	   = enc_config_param.picQpY;


			if (enc_open_param.ringBufferEnable == 0)
			{
				enc_header_param.buf = enc_vb_stream.phys_addr;
				enc_header_param.size = enc_vb_stream.size;
			}

			if (enc_config_param.en_container && !enc_config_param.ringBufferEnable)
			{
				enc_config_param.picWidth = enc_open_param.picWidth;
				enc_config_param.picHeight= enc_open_param.picHeight;
				if ( !container_init(enc_config_param, &fmt, &oc, &video_st, enc_open_param.gopSize) )
					goto ERR_DEC_OPEN;

				header_pos = 0;
				if(enc_open_param.bitstreamFormat == STD_MPEG4 || enc_open_param.bitstreamFormat == STD_AVC)
					header_buf = (char*)osal_malloc(CONTAINER_HEADER_SIZE);
			}

			if( enc_open_param.bitstreamFormat == STD_MPEG4 ) 
			{
				enc_header_param.headerType = VOS_HEADER;
				enc_header_param.size = enc_vb_stream.size;
				VPU_EncGiveCommand(enc_handle, ENC_PUT_VIDEO_HEADER, &enc_header_param);
				if( enc_open_param.ringBufferEnable == 0 )
				{
					if (enc_config_param.en_container && !enc_config_param.ringBufferEnable)
						container_copy_header_from_bitstream_buffer(core_index, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian, header_buf, &header_pos);
					else  {
						if (!ReadBsResetBufHelper(core_index, fp_bit_stream, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian))
							goto ERR_ENC_OPEN;
					}
				}

				enc_header_param.headerType = VOL_HEADER;
				enc_header_param.size = enc_vb_stream.size;
				VPU_EncGiveCommand(enc_handle, ENC_PUT_VIDEO_HEADER, &enc_header_param); 
				if( enc_open_param.ringBufferEnable == 0 )
				{
					if (enc_config_param.en_container)
						container_copy_header_from_bitstream_buffer(core_index, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian, header_buf, &header_pos);
					else {
						if (!ReadBsResetBufHelper(core_index, fp_bit_stream, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian))
							goto ERR_ENC_OPEN;
					}
				}
			}
			else if( enc_open_param.bitstreamFormat == STD_AVC ) 
			{
				enc_header_param.headerType = SPS_RBSP;
				if (enc_open_param.ringBufferEnable == 0)
					enc_header_param.buf  = enc_vb_stream.phys_addr;
				enc_header_param.size = enc_vb_stream.size;

				VPU_EncGiveCommand(enc_handle, ENC_PUT_VIDEO_HEADER, &enc_header_param); 
				if (enc_open_param.ringBufferEnable == 0)
				{
					if (enc_config_param.en_container && !enc_config_param.ringBufferEnable)
						container_copy_header_from_bitstream_buffer(core_index, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian, header_buf, &header_pos);
					else {
						if (!ReadBsResetBufHelper(core_index, fp_bit_stream, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian))
							goto ERR_ENC_OPEN;
					}
					
				}				

				enc_header_param.headerType = PPS_RBSP;
				enc_header_param.buf		= enc_vb_stream.phys_addr;
				enc_header_param.pBuf	   = (BYTE *)enc_vb_stream.virt_addr;
				enc_header_param.size	   = enc_vb_stream.size;
				VPU_EncGiveCommand(enc_handle, ENC_PUT_VIDEO_HEADER, &enc_header_param);
				if( enc_open_param.ringBufferEnable == 0 )
				{

					if (enc_config_param.en_container && !enc_config_param.ringBufferEnable)
						container_copy_header_from_bitstream_buffer(core_index, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian, header_buf, &header_pos);
					else {
						if (!ReadBsResetBufHelper(core_index, fp_bit_stream, enc_header_param.buf, enc_header_param.size, enc_open_param.streamEndian))
							goto ERR_ENC_OPEN;
					}
					
				}
			}
			//[Ken] Before VPU_Init, set Encoder End //////
			///////////////////////////////////////////////////////
		}


FLUSH_BUFFER:		
		if((int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) != (1<<INT_BIT_BIT_BUF_EMPTY) && (int_reason & (1<<INT_BIT_DEC_FIELD)) != (1<<INT_BIT_DEC_FIELD))
		{
			if (ppu_enable) 
				VPU_DecGiveCommand(dec_handle, SET_ROTATOR_OUTPUT, &fbPPU[pp_index]);

			ConfigDecReport(core_index, dec_handle, dec_open_param.bitstreamFormat);

			// Start decoding a frame.
			ret_code = VPU_DecStartOneFrame(dec_handle, &dec_param);
			if (ret_code != RETCODE_SUCCESS) 
			{
				VLOG(ERR,  "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret_code);
				goto ERR_ENC_OPEN;
			}
		}
		else
		{
			if(int_reason & (1<<INT_BIT_DEC_FIELD))
			{
				VPU_ClearInterrupt(core_index);
				int_reason = 0;
			}
			// After VPU generate the BIT_EMPTY interrupt. HOST should feed the bitstreams than 512 byte.
			if (dec_open_param.bitstreamMode != BS_MODE_PIC_END)
			{
				if (bitstream_fill_size < VPU_GBU_SIZE)
					continue;
			}
		}

			while((kb_hit_ret = osal_kbhit()) == 0) 
			{
				int_reason = VPU_WaitInterrupt(core_index, VPU_DEC_TIMEOUT);
				if (int_reason == (Uint32)-1 ) // timeout
				{
					VPU_SWReset(core_index, SW_RESET_SAFETY, NULL);				
					break;
				}		

				CheckUserDataInterrupt(core_index, dec_handle, dec_output_info.indexFrameDecoded, dec_open_param.bitstreamFormat, int_reason);
				if(int_reason & (1<<INT_BIT_DEC_FIELD))	
				{
					if (dec_open_param.bitstreamMode == BS_MODE_PIC_END)
					{
						PhysicalAddress rdPtr, wrPtr;
						int32_t room;
						VPU_DecGetBitstreamBuffer(dec_handle, &rdPtr, &wrPtr, &room);
						if (rdPtr-dec_open_param.bitstreamBuffer < (PhysicalAddress)(chunk_size+pic_header_size+seq_header_size-8))	// there is full frame data in chunk data.
							VPU_DecSetRdPtr(dec_handle, rdPtr, 0);		//set rdPtr to the position of next field data.
						else
						{
							// do not clear interrupt until feeding next field picture.
							break;
						}
					}
				}
			
				if (int_reason)
					VPU_ClearInterrupt(core_index);

				if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) 
				{
					if (dec_open_param.bitstreamMode == BS_MODE_PIC_END)
					{
						VLOG(ERR, "Invalid operation is occurred in pic_end mode \n");
						goto ERR_ENC_OPEN;
					}
					break;
				}


				if (int_reason & (1<<INT_BIT_PIC_RUN)) 
					break;				
			}			

			if(int_reason & (1<<INT_BIT_BIT_BUF_EMPTY)) 
			{
				bitstream_fill_size = 0;
				continue; // go to take next chunk.
			}
			if(int_reason & (1<<INT_BIT_DEC_FIELD)) 
			{
				bitstream_fill_size = 0;
				continue; // go to take next chunk.
			}		

		ret_code = VPU_DecGetOutputInfo(dec_handle, &dec_output_info);
		if (ret_code != RETCODE_SUCCESS) 
		{
			VLOG(ERR,  "VPU_DecGetOutputInfo failed Error code is 0x%x \n", ret_code);
#ifdef SUPPORT_MEM_PROTECT
			if (ret_code == RETCODE_MEMORY_ACCESS_VIOLATION)
				PrintMemoryAccessViolationReason(core_index, &dec_output_info);
#endif
			goto ERR_ENC_OPEN;
		}

		if ((dec_output_info.decodingSuccess & 0x01) == 0) 
		{
			VLOG(ERR, "VPU_DecGetOutputInfo decode fail framdIdx %d \n", frame_index);
			VLOG(TRACE, "#%d, indexFrameDisplay %d || picType %d || indexFrameDecoded %d\n", 
				frame_index, dec_output_info.indexFrameDisplay, dec_output_info.picType, dec_output_info.indexFrameDecoded );
		}		

		VLOG(TRACE, "#%d:%d, indexDisplay %d || picType %d || indexDecoded %d || rdPtr=0x%x || wrPtr=0x%x || chunk_size = %d, consume=%d\n", 
			instance_index, frame_index, dec_output_info.indexFrameDisplay, dec_output_info.picType, dec_output_info.indexFrameDecoded, dec_output_info.rdPtr, dec_output_info.wrPtr, chunk_size+pic_header_size, dec_output_info.consumedByte);

		SaveDecReport(core_index, dec_handle, &dec_output_info, dec_open_param.bitstreamFormat, ((dec_initial_info.picWidth+15)&~15)/16);
		if (dec_output_info.chunkReuseRequired) // reuse previous chunk
			reuse_chunk = 1;		

		if (dec_output_info.indexFrameDisplay == -1)
			decode_finish = 1;

		if (!ppu_enable) 
		{
			if (decode_finish)
				break;		

			if( dec_output_info.indexFrameDisplay == -3 ||
				dec_output_info.indexFrameDisplay == -2 ) // BIT doesn't have picture to be displayed 
			{
				if (prev_disp_index >= 0)
					VPU_DecClrDispFlag(dec_handle, prev_disp_index);
				prev_disp_index = dec_output_info.indexFrameDisplay;

				continue;
			}
		}
		else
		{
				if (decode_finish)
				{
					if (!frame_index)
						break;
					// if PP feature has been enabled. the last picture is in PP output framebuffer.									
				}

				if (dec_output_info.indexFrameDisplay == -3 ||
					dec_output_info.indexFrameDisplay == -2 ) // BIT doesn't have picture to be displayed
				{
					if (prev_disp_index >= 0)
						VPU_DecClrDispFlag(dec_handle, prev_disp_index);
					prev_disp_index = dec_output_info.indexFrameDisplay;
					continue;
				}

				if (prev_disp_index<0) // if PP has been enabled, the first picture is saved at next time.
				{
					prev_disp_index = dec_output_info.indexFrameDisplay;
					// save rotated dec width, height to display next decoding time.
					rect_prev_display = dec_output_info.rcDisplay;
					continue;
				}
		}

		//[Ken] Transcoding Coding Start ///////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////
		enc_param.sourceFrame = &(dec_output_info.dispFrame);

		//decode frame buffer myindex is not encoder's index
		//so myindex value is fixed 2, myindex 0,1 is reconstruction frame buffer position.
		enc_param.sourceFrame->myIndex = 2;

		if( enc_open_param.ringBufferEnable == 0)
		{
			enc_param.picStreamBufferAddr = enc_vb_stream.phys_addr;	// can set the newly allocated buffer.
			enc_param.picStreamBufferSize = enc_vb_stream.size;
		}


#ifdef TEST_SKIP_PIC
		{
			int32_t picIdx = enc_open_param.EncStdParam.avcParam.fieldFlag ? 2*frame_index+field_done : frame_index;
			enc_param.skipPicture = 0;
			for (i=0; i<MAX_PIC_SKIP_NUM; i++)
			{
				if (enc_config_param.skip_pic_nums[i] > 0 && enc_config_param.skip_pic_nums[i] == picIdx)
				{
					enc_param.skipPicture = 1;
					break;
				}
			}
		}
#endif

#ifdef SUPPORT_FFMPEG_DEMUX
		//for making IDR
		if (enc_config_param.en_container )
		{
			if ( enc_open_param.gopSize != 0)
			{
				if (frame_index % enc_open_param.gopSize == 0)
					enc_param.forceIPicture = 1;
				else
					enc_param.forceIPicture = 0;
			}
		}
#endif
		ret_code = VPU_EncStartOneFrame(enc_handle, &enc_param);
		if( ret_code != RETCODE_SUCCESS )
		{
			VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret_code );
			goto ERR_ENC_OPEN;
		}

		timeout_count = 0;
		while(osal_kbhit() == 0) 
		{
			int_reason = VPU_WaitInterrupt(core_index, VPU_WAIT_TIME_OUT);
			if (int_reason == (Uint32)-1)
			{
				if (timeout_count*VPU_WAIT_TIME_OUT > VPU_ENC_TIMEOUT)
				{
					VLOG(ERR, "Error : encoder timeout happened\n");
					VPU_SWReset(core_index, SW_RESET_SAFETY, NULL);
					break;
				}
				int_reason = 0;
				timeout_count++;
			}

			if (enc_open_param.ringBufferEnable == 1) 
			{
				ret_code = ReadBsRingBufHelper(core_index, enc_handle, fp_bit_stream, enc_open_param.bitstreamBuffer, enc_open_param.bitstreamBufferSize, STREAM_READ_SIZE, enc_open_param.streamEndian);
				if (ret_code != RETCODE_SUCCESS) 
				{
					VLOG(ERR, "ReadBsRingBufHelper failed Error code is 0x%x \n", ret_code );
					goto ERR_ENC_OPEN;
				}
			}

			if (enc_open_param.ringBufferEnable == 0 && enc_open_param.lineBufIntEn) 
			{
				if (int_reason & (1<<INT_BIT_BIT_BUF_FULL))
				{
#ifdef SUPPORT_FFMPEG_DEMUX
					if (enc_config_param.en_container && !enc_config_param.ringBufferEnable)
						ret_code = (RetCode)container_write_es(core_index, enc_open_param.bitstreamBuffer, enc_open_param.bitstreamBufferSize, 
																			   enc_open_param.streamEndian, oc, video_st,
																			   header_buf, &header_pos, enc_open_param.bitstreamFormat, 
																			   enc_output_info.picType);
					else
#endif
					{
						if (!ReadBsResetBufHelper(core_index, fp_bit_stream, enc_open_param.bitstreamBuffer, enc_open_param.bitstreamBufferSize, enc_open_param.streamEndian))
							goto ERR_ENC_OPEN;
					}
				}					
			}

			if (int_reason)
			{
				VPU_ClearInterrupt(core_index);

				if (int_reason & (1<<INT_BIT_PIC_RUN)) 
					break;
			}
		}
		ret_code = VPU_EncGetOutputInfo(enc_handle, &enc_output_info);
		if (ret_code != RETCODE_SUCCESS)
		{
#ifdef SUPPORT_MEM_PROTECT
			if (ret_code == RETCODE_MEMORY_ACCESS_VIOLATION)
				PrintMemoryAccessViolationReason(core_index, NULL);
#endif
			VLOG(ERR, "VPU_EncGetOutputInfo failed Error code is 0x%x \n", ret_code );
			goto ERR_ENC_OPEN;
		}

		if (enc_open_param.ringBufferEnable == 0)
		{
			if (enc_output_info.bitstreamWrapAround == 1)
			{
				VLOG(WARN, "Warnning!! BitStream buffer wrap arounded. prepare more large buffer \n", ret_code );
			}
			if (enc_output_info.bitstreamSize == 0)
			{
				printf("ERROR!!! bitstreamsize = 0 \n");
			}
#ifdef SUPPORT_FFMPEG_DEMUX
			if (enc_config_param.en_container && !enc_config_param.ringBufferEnable)
				ret_code = (RetCode)container_write_es(core_index, enc_output_info.bitstreamBuffer, enc_output_info.bitstreamSize, 
																	   enc_open_param.streamEndian, oc, video_st,
																	   header_buf, &header_pos, enc_open_param.bitstreamFormat, 
																	   enc_output_info.picType);
			else
#endif
			{
				if (!ReadBsResetBufHelper(core_index, fp_bit_stream, enc_output_info.bitstreamBuffer, enc_output_info.bitstreamSize, enc_open_param.streamEndian))
				break;
			}
		}

#ifdef ENC_RECON_FRAME_DISPLAY
		{
			Rect rc = {0, 0, frame_buffer_stride, frame_buffer_height};
#ifdef SUPPORT_SW_MIXER
			sw_mixer_open((core_index*MAX_NUM_VPU_CORE)+instance_index, frame_buffer_stride, frame_buffer_height);
#endif
			SaveYuvImageHelperFormat(core_index, NULL, &enc_output_info.reconFrame, map_config, p_yuv, 
				rc, enc_open_param.cbcrInterleave, src_frame_format, enc_open_param.frameEndian);	
#ifdef SUPPORT_SW_MIXER
			sw_mixer_draw((core_index*MAX_NUM_VPU_CORE)+instance_index, 0, 0, frame_buffer_stride, frame_buffer_height,  YUV_FORMAT_I420, p_yuv);	
#endif
		}
#ifdef CNM_FPGA_PLATFORM
		DisplayMixer(&enc_output_info.reconFrame, 
			(enc_config_param.rotAngle==90||enc_config_param.rotAngle ==270)?enc_open_param.picHeight:enc_open_param.picWidth,
			(enc_config_param.rotAngle==90||enc_config_param.rotAngle ==270)?enc_open_param.picWidth:enc_open_param.picHeight);
#endif
#endif		


		//[Ken] Transcoding Coding End ////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////

		if (prev_disp_index >= 0) 
			VPU_DecClrDispFlag(dec_handle, prev_disp_index);


		prev_disp_index = dec_output_info.indexFrameDisplay;	
		// save rotated dec width, height to display next decoding time.
		rect_prev_display = dec_output_info.rcDisplay;

		if (dec_output_info.numOfErrMBs) 
		{
			total_num_of_err_mbs += dec_output_info.numOfErrMBs;
			VLOG(ERR, "Num of Error Mbs : %d, in Frame : %d \n", dec_output_info.numOfErrMBs, frame_index);
		}

		frame_index++;

		if (dec_config_param.outNum && frame_index == (dec_config_param.outNum-1)) 
			break;

		if (decode_finish)
			break;		
	}	// end of while

	if (total_num_of_err_mbs) 
		VLOG(ERR, "Total Num of Error MBs : %d\n", total_num_of_err_mbs);

	if( enc_open_param.ringBufferEnable == 1 )
	{
		ret_code = ReadBsRingBufHelper(core_index, enc_handle, fp_bit_stream, enc_open_param.bitstreamBuffer, enc_open_param.bitstreamBufferSize, 0, enc_open_param.streamEndian );
		if( ret_code != RETCODE_SUCCESS )
		{
			VLOG(ERR, "ReadBsRingBufHelper failed Error code is 0x%x \n", ret_code );
			goto ERR_ENC_OPEN;
		}
	}

ERR_ENC_OPEN:
	//[Ken] CLOSE Encoder Start ////////////
	/////////////////////////////////////////////
	VPU_EncClose(enc_handle);
	VLOG(INFO, "\nTRANSCODING End. Tot Frame %d\n" , frame_index );
	//[Ken] CLOSE Encoder End ////////////
	/////////////////////////////////////////////

ERR_DEC_OPEN:
	VPU_DecUpdateBitstreamBuffer(dec_handle, STREAM_END_SIZE);
	// Now that we are done with decoding, close the open instance.
	VPU_DecClose(dec_handle);


ERR_TRANS_INIT:	
	if (dec_vb_stream.size)
		vdi_free_dma_memory(core_index, &dec_vb_stream);
	
	if (p_seq_header)
		osal_free(p_seq_header);
		
	if (p_pic_header)
		osal_free(p_pic_header);

	if(p_yuv)
		osal_free( p_yuv );

	if (tho_parser)
		tho_parser->close(tho_parser);

	if(p_tho_stream!=NULL)
		osal_free(p_tho_stream);

	//[Ken] DeInit Encoder Start ////////////
	/////////////////////////////////////////////
	if (enc_vb_stream.size)
		vdi_free_dma_memory(core_index, &enc_vb_stream);

	if (vb_report.size)
		vdi_free_dma_memory(core_index, &vb_report);

#ifdef SUPPORT_FFMPEG_DEMUX
	if (enc_config_param.en_container && !enc_config_param.ringBufferEnable)
	{
		container_deinit(oc);
		if(header_buf)
			free(header_buf);
	}
	else
	{
		if( fp_bit_stream )
			osal_fclose(fp_bit_stream);
	}
#else
	if( fp_bit_stream )
		osal_fclose(fp_bit_stream);
#endif

	avformat_close_input(&ic);


#ifdef SUPPORT_SW_MIXER
	sw_mixer_close((core_index*MAX_NUM_VPU_CORE)+instance_index);
#endif

	VPU_DeInit(core_index);
	return 1;
}
#endif	//#ifdef SUPPORT_FFMPEG_DEMUX

#ifdef PLATFORM_LINUX	
#include "pthread.h"	// for MultiInstanceTest
#ifdef SUPPORT_FFMPEG_DEMUX
void *FnFilePlayTest(void *param)
{
	int ret;
	ret = FilePlayTest(param);
	return (void *)ret;
}
#endif
void *FnEncodeTest(void *param)
{
	int ret;
	ret = EncodeTest(param);
	return (void *)ret;
}
void *FnDecodeTest(void *param)
{
	int ret;
	ret = DecodeTest(param);
	return (void *)ret;
}
#endif	//PLATFORM_LINUX

int MultiInstanceTest(MultiConfigParam	*param)
{
#if defined(PLATFORM_WIN32)
	int i;
	int ret;
	HANDLE hThread[MAX_NUM_INSTANCE]={0,};
	DecConfigParam *pDecConfig;
	EncConfigParam *pEncConfig;
	
	for(i=0; i<param->numMulti && i <MAX_NUM_INSTANCE; i++)
	{
		if (param->multiMode[i] < STD_MP4_ENC || param->decConfig[i].runFilePlayTest==1) //decoder case
		{
			pDecConfig = &param->decConfig[i];		
			pDecConfig->instNum = i;
			//strcpy(pDecConfig->yuvFileName, "d:/out.yuv");

#ifdef SUPPORT_FFMPEG_DEMUX
			if (pDecConfig->bitFormat == STD_THO || pDecConfig->runFilePlayTest)
				hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FilePlayTest, (LPVOID)pDecConfig, 0, NULL);
			else
#endif
				hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DecodeTest, (LPVOID)pDecConfig, 0, NULL);			
		}
		else
		{
			pEncConfig = &param->encConfig[i];	
			pEncConfig->instNum = i;

			hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EncodeTest, (LPVOID)pEncConfig, 0, NULL);

		}

		Sleep(5000);
	}

	WaitForMultipleObjects(param->numMulti, hThread, TRUE, INFINITE);
		
	for(i=0; i<param->numMulti; i++)
	{
		GetExitCodeThread(hThread[i], &ret);
		if(ret == 0)	
			return 0;	

		if (hThread[i])
			CloseHandle(hThread[i]);
	}
	return 1;
#endif

#ifdef PLATFORM_LINUX
	int i;
	pthread_t thread_id[MAX_NUM_INSTANCE];
	void *ret[MAX_NUM_INSTANCE];
	DecConfigParam *pDecConfig;
	EncConfigParam *pEncConfig;
	
	
	for(i=0; i<param->numMulti; i++)
	{
		if (param->multiMode[i] < STD_MP4_ENC) //decoder case
		{
			pDecConfig = &param->decConfig[i];		
			pDecConfig->instNum = i;
#ifdef SUPPORT_FFMPEG_DEMUX
			if (pDecConfig->bitFormat == STD_VP8_DEC || pDecConfig->bitFormat == STD_THO_DEC)
				pthread_create(&thread_id[i], NULL, FnFilePlayTest, pDecConfig);
			else
#endif
				pthread_create(&thread_id[i], NULL, FnDecodeTest, pDecConfig);			
		}
		else
		{
			pEncConfig = &param->encConfig[i];	
			pEncConfig->instNum = i;
			pthread_create(&thread_id[i], NULL, FnEncodeTest, pEncConfig);
		}
		usleep(5000000);	// it makes delay to adjust a gab among threads
	}

	for(i=0; i<param->numMulti; i++)
	{
		pthread_join(thread_id[i], &ret[i]);
	}

	
	for(i=0; i<param->numMulti; i++)
	{
		if (ret[i] == 0)
			return 0;
	}
	return 1;
#endif	

}









