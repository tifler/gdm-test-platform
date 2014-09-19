//------------------------------------------------------------------------------
// File: vpurun.c
//
// Copyright (c) 2006~2010, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#include "jpuapi.h"
#include "regdefine.h"
#include "jpulog.h"
#include "mixer.h"
#include "jpulog.h"
#include "jpuhelper.h"
#include "jpurun.h"
#include "jpuapifunc.h"

#ifdef CNM_FPGA_PLATFORM
#define ENC_SOURCE_FRAME_DISPLAY
#endif

#define NUM_FRAME_BUF			MAX_FRAME
#define MAX_ROT_BUF_NUM			1
#define EXTRA_FRAME_BUFFER_NUM	0
#define ENC_SRC_BUF_NUM			1
//#define ROI_RANDOM_TEST



#ifdef PLATFORM_LINUX	
#include "pthread.h"	// for MultiInstanceTest
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



int DecodeTest(DecConfigParam *param)
{

	JpgDecHandle handle		= {0};
	JpgDecOpenParam	decOP		= {0};
	JpgDecInitialInfo initialInfo = {0};
	JpgDecOutputInfo outputInfo	= {0};
	JpgDecParam decParam	= {0};
	JpgRet ret = JPG_RET_SUCCESS;	
	FrameBuffer frameBuf[NUM_FRAME_BUF];
	jpu_buffer_t vbStream    = {0};
	BufInfo bufInfo     = {0};
	FRAME_BUF *pFrame[NUM_FRAME_BUF];
	FRAME_BUF *pDispFrame = NULL;
	Uint32 bsSize=0, framebufSize=0, framebufWidth=0, framebufHeight=0, framebufStride = 0, framebufFormat = FORMAT_420;
	int dispWidth = 0, dispHeight = 0;
	int i = 0, frameIdx = 0, ppIdx = 0, saveIdx =0, totalNumofErrMbs = 0, streameos = 0, dispImage = 0, decodefinish = 0;
	int key = 0,  suc = 1;
	Uint8 *pFileBuf = NULL;
	Uint8 *pYuv	 =	NULL;
	FILE *fchunkData =	NULL;
	FILE *fpYuv	 =	NULL;
	int kbhitRet = 0;
	int needFrameBufCount = 0, regFrameBufCount = 0;
	int rotEnable = 0;
	int int_reason = 0;
	int instIdx;
	int partPosIdx = 0;
    int partBufIdx = 0;
	int partMaxIdx = 0;
	int partialHeight = 0;

	DecConfigParam decConfig;
	
	memcpy(&decConfig, param, sizeof(DecConfigParam));
	memset(&pFrame, 0x00, sizeof(FRAME_BUF *)*NUM_FRAME_BUF);
	memset(&frameBuf, 0x00, sizeof(FrameBuffer)*NUM_FRAME_BUF);
	
	instIdx = decConfig.instNum;
	if(decConfig.usePartialMode && decConfig.roiEnable)
	{
		JLOG(ERR, "Invalid operation mode : partial and ROI mode can not be worked\n");
		goto ERR_DEC_INIT;
	}
	if(decConfig.packedFormat && decConfig.roiEnable)
	{
		JLOG(ERR, "Invalid operation mode : packed mode and ROI mode can not be worked\n");
		goto ERR_DEC_INIT;
	}
	if ((decConfig.iHorScaleMode || decConfig.iVerScaleMode) && decConfig.roiEnable)
	{
		JLOG(ERR, "Invalid operation mode : Scaler mode and ROI mode can not be worked\n");
		goto ERR_DEC_INIT;
	}
	if (decConfig.useRot && decConfig.roiEnable)
	{
		JLOG(ERR, "Invalid operation mode : Rotator mode and ROI mode can not be worked\n");
		goto ERR_DEC_INIT;
	}
	
	if(strlen(decConfig.yuvFileName))
		dispImage = 0;
	else
		dispImage = 1;

	fchunkData = fopen(decConfig.bitstreamFileName, "rb");
	if( !fchunkData ) 
	{
		JLOG(ERR, "Can't open %s \n", decConfig.bitstreamFileName );
		goto ERR_DEC_INIT;
	}

	if (strlen(decConfig.yuvFileName)) 
	{
		fpYuv = fopen(decConfig.yuvFileName, "wb");
		if (!fpYuv) 
		{
			JLOG(ERR, "Can't open %s \n", decConfig.yuvFileName );
			goto ERR_DEC_INIT;
		}		
	}

	fseek(fchunkData, 0, SEEK_END);
	bsSize = ftell(fchunkData);
	fseek(fchunkData, 0, SEEK_SET);

	pFileBuf = malloc(bsSize);
	if (!pFileBuf)
		goto ERR_DEC_INIT;

	fread(pFileBuf, bsSize, sizeof( Uint8 ), fchunkData);
	fclose(fchunkData);
	fchunkData = NULL;

	bufInfo.buf = pFileBuf;
	bufInfo.size = bsSize;
	bufInfo.point = 0;

	ret = JPU_Init();
	if (ret != JPG_RET_SUCCESS && 
		ret != JPG_RET_CALLED_BEFORE)
	{
		suc = 0;
		JLOG(ERR, "JPU_Init failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}
	
	// Open an instance and get initial information for decoding.

	vbStream.size = STREAM_BUF_SIZE;
	if (jdi_allocate_dma_memory(&vbStream) < 0)
	{
		JLOG(ERR, "fail to allocate bitstream buffer\n" );
		goto ERR_DEC_INIT;
	}

	decOP.streamEndian = decConfig.StreamEndian;
	decOP.frameEndian = decConfig.FrameEndian;
	decOP.chromaInterleave = (CbCrInterLeave)decConfig.chromaInterleave;
	decOP.bitstreamBuffer = vbStream.phys_addr;  
	decOP.bitstreamBufferSize = vbStream.size; 
	decOP.pBitStream = (BYTE *)vbStream.virt_addr; // set virtual address mapped of physical address
	decOP.thumbNailEn = decConfig.ThumbNailEn;	
	decOP.packedFormat = decConfig.packedFormat;
	decOP.roiEnable = decConfig.roiEnable;
	decOP.roiOffsetX = decConfig.roiOffsetX;
	decOP.roiOffsetY = decConfig.roiOffsetY;
	decOP.roiWidth = decConfig.roiWidth;
	decOP.roiHeight = decConfig.roiHeight;

	ret = JPU_DecOpen(&handle, &decOP);
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_DecOpen failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}
	
	//JPU_DecGiveCommand(handle, ENABLE_LOGGING, NULL);

	if (decConfig.useRot) 
		rotEnable = 1;
	else
		rotEnable = 0;

	ret = WriteJpgBsBufHelper(handle, &bufInfo, decOP.bitstreamBuffer, decOP.bitstreamBuffer+decOP.bitstreamBufferSize, 0, 0, &streameos, decOP.streamEndian);
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
	
	ret = JPU_DecGetInitialInfo(handle, &initialInfo);
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_DecGetInitialInfo failed Error code is 0x%x, inst=%d \n", ret, instIdx);
		goto ERR_DEC_OPEN;
	}
	
    if(decConfig.usePartialMode)
    {
        // disable Rotator, Scaler
        rotEnable = 0;
        decConfig.iHorScaleMode = 0;
        decConfig.iVerScaleMode = 0;
        partialHeight = (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_224) ? 16 : 8;

	    partMaxIdx  = ((initialInfo.picHeight +15)&~15) / partialHeight;
        if(partMaxIdx < decConfig.partialBufNum)
            decConfig.partialBufNum = partMaxIdx;
    }	

	if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_422)
		framebufWidth = ((initialInfo.picWidth+15)/16)*16;
	else
		framebufWidth = ((initialInfo.picWidth+7)/8)*8;

	if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_224)
		framebufHeight = ((initialInfo.picHeight+15)/16)*16;
	else
		framebufHeight = ((initialInfo.picHeight+7)/8)*8;

	if(decConfig.roiEnable)
	{
		framebufWidth  = initialInfo.roiFrameWidth ;
		framebufHeight = initialInfo.roiFrameHeight;
	}
	

	// scaler constraint when conformance test is disable
	if (framebufWidth < 128 || framebufHeight < 128)
	{
		if (decConfig.iHorScaleMode || decConfig.iVerScaleMode) 
			JLOG(WARN, "Invalid operation mode : Not supported resolution with Scaler, width=%d, height=%d\n", framebufWidth, framebufHeight);
		decConfig.iHorScaleMode = 0;
		decConfig.iVerScaleMode = 0;
	}

		
	JLOG(INFO, "* Dec InitialInfo =>\n instance #%d, \n minframeBuffercount: %u\n ", instIdx, initialInfo.minFrameBufferCount);
	JLOG(INFO, "picWidth: %u\n picHeight: %u\n roiWidth: %u\n rouHeight: %u\n ", initialInfo.picWidth, initialInfo.picHeight, initialInfo.roiFrameWidth, initialInfo.roiFrameHeight); 

    if(decConfig.usePartialMode)
    {
		JLOG(INFO, "Partial Mode Enable\n "); 
		JLOG(INFO, "Num of Buffer for Partial : %d\n ", decConfig.partialBufNum); 
		JLOG(INFO, "Num of Line for Partial   : %d\n ", partialHeight); 
    }

	framebufFormat = initialInfo.sourceFormat;

	if (decConfig.iHorScaleMode || decConfig.iVerScaleMode)
	{
		framebufHeight = ((framebufHeight+1)/2)*2;
		framebufWidth = ((framebufWidth+1)/2)*2;
	}
	
	framebufWidth  >>= decConfig.iHorScaleMode;
	framebufHeight >>= decConfig.iVerScaleMode;

	dispWidth = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? framebufHeight : framebufWidth;
	dispHeight = (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) ? framebufWidth : framebufHeight;

	if (decConfig.rotAngle == 90 || decConfig.rotAngle == 270)
	{
		framebufStride = framebufHeight;	
		framebufHeight = framebufWidth;
		framebufFormat = (framebufFormat == FORMAT_422)?FORMAT_224 : (framebufFormat==FORMAT_224) ? FORMAT_422 : framebufFormat;		
	}	
	else
	{
		framebufStride = framebufWidth;
	}
	
	if (decConfig.iHorScaleMode || decConfig.iVerScaleMode)
		framebufStride = ((framebufStride+15)/16)*16;
	
	if(decOP.packedFormat >= PACKED_FORMAT_422_YUYV && decOP.packedFormat <= PACKED_FORMAT_422_VYUY)
	{
		framebufWidth = framebufWidth*2;
		framebufStride = framebufStride*2;	
		framebufFormat = FORMAT_422;
		if (decConfig.rotAngle == 90 || decConfig.rotAngle == 270) 
			framebufFormat = FORMAT_224;
		
	}
	else if (decOP.packedFormat == PACKED_FORMAT_444)
	{
		framebufWidth = framebufWidth*3;
		framebufStride = framebufStride*3;	
		framebufFormat = FORMAT_444;		
	}
	
	

	if (decConfig.rotAngle == 90 || decConfig.rotAngle == 270)
	{
		param->picWidth = framebufHeight ; 
		param->picHeight = framebufWidth ;	
	}

	else
	{
		param->picWidth = framebufWidth ;
		param->picHeight = framebufHeight ;
	}

	framebufSize = GetFrameBufSize(framebufFormat, framebufStride, framebufHeight);
	JLOG(INFO, "framebuffer stride: %d,  width: %d, height = %d\n", framebufStride, framebufWidth, framebufHeight);
	JLOG(INFO, "framebuffer format: %d, framebuffer size = %d, packed format = %d\n", framebufFormat, framebufSize, decOP.packedFormat);
	JLOG(INFO, "display width: %d, height = %d\n", dispWidth, dispHeight);


	//Allocate frame buffer 
	regFrameBufCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;
	

	if(decConfig.usePartialMode)
	{
		if(decConfig.partialBufNum > 4)
			decConfig.partialBufNum = 4;

		regFrameBufCount *= decConfig.partialBufNum;
	}	

	needFrameBufCount = regFrameBufCount;

	AllocateFrameBuffer(instIdx, framebufFormat, framebufStride, framebufHeight, needFrameBufCount, 0);			
	
	JpgEnterLock();
	for( i = 0; i < needFrameBufCount; ++i ) 
	{
		pFrame[i] = GetFrameBuffer(instIdx, i);
		frameBuf[i].bufY = pFrame[i]->vbY.phys_addr;
		frameBuf[i].bufCb = pFrame[i]->vbCb.phys_addr;
		if (decOP.chromaInterleave == CBCR_SEPARATED)
			frameBuf[i].bufCr = pFrame[i]->vbCr.phys_addr;
		if (dispImage) 
		{
			ClearFrameBuffer(instIdx, i);
			JLOG(INFO, ".");
		}
	}
	JpgLeaveLock();

	//sw_mixer_close(instIdx);
	//sw_mixer_open(instIdx, dispWidth, dispHeight);

	pYuv = malloc(framebufSize);
	if (!pYuv)
	{
		JLOG(ERR, "Fail to allocation memory for display buffer\n");
		goto ERR_DEC_OPEN;
	}
	
	ret = JPU_DecGiveCommand(handle, SET_JPG_USE_PARTIAL_MODE,  &(decConfig.usePartialMode));
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_DecGiveCommand[SET_JPG_USE_PARTIAL_MODE] failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
	ret = JPU_DecGiveCommand(handle, SET_JPG_PARTIAL_FRAME_NUM, &(decConfig.partialBufNum));
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_DecGiveCommand[SET_JPG_PARTIAL_FRAME_NUM] failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
	ret = JPU_DecGiveCommand(handle, SET_JPG_PARTIAL_LINE_NUM,  &(partialHeight));	
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_DecGiveCommand[SET_JPG_PARTIAL_LINE_NUM] failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}

	// Register frame buffers requested by the decoder.
	ret = JPU_DecRegisterFrameBuffer(handle, frameBuf, regFrameBufCount, framebufStride);
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
	ppIdx = 0;

	printf("Dec Start : Press enter key to show menu.\n" );
	printf("          : Press space key to stop.\n" );

	while(1)
	{
		
		if (rotEnable) 
		{
			JPU_DecGiveCommand(handle, SET_JPG_ROTATION_ANGLE, &(decConfig.rotAngle));
			JPU_DecGiveCommand(handle, SET_JPG_MIRROR_DIRECTION, &(decConfig.mirDir));
			JPU_DecGiveCommand(handle, SET_JPG_ROTATOR_OUTPUT, &frameBuf[ppIdx] );
			JPU_DecGiveCommand(handle, SET_JPG_ROTATOR_STRIDE, &framebufStride);
           
			JPU_DecGiveCommand(handle, ENABLE_JPG_ROTATION, 0);
			JPU_DecGiveCommand(handle, ENABLE_JPG_MIRRORING, 0);				
		}

        JPU_DecGiveCommand(handle, SET_JPG_SCALE_HOR,  &(decConfig.iHorScaleMode));
        JPU_DecGiveCommand(handle, SET_JPG_SCALE_VER,  &(decConfig.iVerScaleMode));

        if(decConfig.usePartialMode)
		{
			partPosIdx = 0;
			partBufIdx = 0;
			outputInfo.decodingSuccess = 0;
			JPU_SWReset();
		}
		
		// Start decoding a frame.
		ret = JPU_DecStartOneFrame(handle, &decParam);
		if (ret != JPG_RET_SUCCESS && ret != JPG_RET_EOS)
		{
			if (ret == JPG_RET_BIT_EMPTY)
			{
				ret = WriteJpgBsBufHelper(handle, &bufInfo, decOP.bitstreamBuffer, decOP.bitstreamBuffer+decOP.bitstreamBufferSize, STREAM_FILL_SIZE, 0, &streameos, decOP.streamEndian);
				if( ret != JPG_RET_SUCCESS )
				{
					JLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
					goto ERR_DEC_OPEN;
				}
				continue;
			}

			JLOG(ERR, "JPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}
		if (ret == JPG_RET_EOS)
			goto JPU_END_OF_STREAM;

		while(1) 
		{
  		    if (kbhit()) {
			    key = getch();
			    fflush(stdout);
		    }

		    if (key) {
                if (key == ' ')
				{

					ret = JPU_DecIssueStop(handle);
					if( ret != JPG_RET_SUCCESS )
					{
						JLOG(ERR, "JPU_DecIssueStop failed Error code is 0x%x \n", ret );
						goto ERR_DEC_OPEN;
					}                               
					//should break after interrupt

				}
		    }
			int_reason = JPU_WaitInterrupt(JPU_INTERRUPT_TIMEOUT_MS);
			if (int_reason == -1)
			{
				JLOG(ERR, "Error : timeout happened\n");
				JPU_SWReset();
				break;
			}
			if( decConfig.usePartialMode && (int_reason & 0xf0))
			{
				partBufIdx = ((partPosIdx) % decConfig.partialBufNum);

				if ((1<<partBufIdx) & ((int_reason & 0xf0)>>4))
				{
					printf("DECODED : PARTIAL BUFFER IDX %d / POS %d / MAX POS %d / INT_REASON=0x%x\n", partBufIdx, partPosIdx+1, partMaxIdx, int_reason);

					if (dispImage)
					{
						pDispFrame = FindFrameBuffer(instIdx, frameBuf[partBufIdx].bufY);
#ifdef CNM_FPGA_PLATFORM
						SetMixerDecOutFrame(pDispFrame, framebufStride, partialHeight);			
#endif
					}
					else
					{
						saveIdx = partBufIdx;
						if (!SaveYuvPartialImageHelperFormat(fpYuv, pYuv, 
							frameBuf[saveIdx].bufY, frameBuf[saveIdx].bufCb, frameBuf[saveIdx].bufCr,
							dispWidth, dispHeight, partialHeight, framebufStride, decConfig.chromaInterleave, framebufFormat, decOP.frameEndian, partPosIdx, frameIdx, 
							decOP.packedFormat))
							goto ERR_DEC_OPEN;	

						//sw_mixer_draw(instIdx, 0, 0, dispWidth, partialHeight, framebufFormat, decOP.packedFormat, decOP.chromaInterleave, pYuv);
					}

					partPosIdx++;
					JPU_ClrStatus((1<< (INT_JPU_PARIAL_BUF0_EMPTY + partBufIdx)));

					continue;
				}
				else
				{
					JLOG(ERR, "Invalid partial interrupt : expected reason =0x%x, actual reason=0x%x \n", (1<<partBufIdx), ((int_reason & 0xF0)>>4));		
					goto ERR_DEC_OPEN;	
				}
			}
			if (int_reason & (1<<INT_JPU_DONE) || int_reason & (1<<INT_JPU_ERROR))	// Must catch PIC_DONE interrupt before catching EMPTY interrupt
			{
				// Do no clear INT_JPU_DONE and INT_JPU_ERROR interrupt. these will be cleared in JPU_DecGetOutputInfo.
				break;			
			}

			if (int_reason & (1<<INT_JPU_BIT_BUF_EMPTY))
			{
				ret = WriteJpgBsBufHelper(handle, &bufInfo, decOP.bitstreamBuffer, decOP.bitstreamBuffer+decOP.bitstreamBufferSize, STREAM_FILL_SIZE, 0, &streameos, decOP.streamEndian);
				if( ret != JPG_RET_SUCCESS )
				{
					JLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
					goto ERR_DEC_OPEN;
				}
				JPU_ClrStatus((1<<INT_JPU_BIT_BUF_EMPTY));
			}

            if (int_reason & (1<<INT_JPU_BIT_BUF_STOP)) 
			{
				ret = JPU_DecCompleteStop(handle);
				if( ret != JPG_RET_SUCCESS )
				{
					JLOG(ERR, "JPU_DecCompleteStop failed Error code is 0x%x \n", ret );
					goto ERR_DEC_OPEN;
				}
				JPU_ClrStatus((1<<INT_JPU_BIT_BUF_STOP));
                break;
			}

			if (int_reason & (1<<INT_JPU_PARIAL_OVERFLOW))
				JPU_ClrStatus((1<<INT_JPU_PARIAL_OVERFLOW));
		}

JPU_END_OF_STREAM:
		ret = JPU_DecGetOutputInfo(handle, &outputInfo);
		if (ret != JPG_RET_SUCCESS) 
		{
			JLOG(ERR, "JPU_DecGetOutputInfo failed Error code is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}		
		
#ifdef MJPEG_ERROR_CONCEAL
	    if(outputInfo.numOfErrMBs) 
		{	
			int errRstIdx, errPosX, errPosY;
			errRstIdx = (outputInfo.numOfErrMBs & 0x0F000000) >> 24;
			errPosX = (outputInfo.numOfErrMBs & 0x00FFF000) >> 12;
			errPosY = (outputInfo.numOfErrMBs & 0x00000FFF);
			JLOG(ERR, "Error restart Idx : %d, MCU x:%d, y:%d, in Frame : %d \n", errRstIdx, errPosX, errPosY, frameIdx);	
            continue;
		}
#endif

		if (outputInfo.decodingSuccess == 0 )
			JLOG(ERR, "JPU_DecGetOutputInfo decode fail framdIdx %d \n", frameIdx);

		
		JLOG(TRACE, "#%d:%d, indexFrameDisplay %d || consumedByte %d || ppIdx %d || frameStart=0x%x || ecsStart=0x%x || rdPtr=0x%x || wrPtr=0x%x || pos=%d\n", 
			instIdx, frameIdx, outputInfo.indexFrameDisplay, outputInfo.consumedByte, ppIdx, outputInfo.bytePosFrameStart, outputInfo.bytePosFrameStart+outputInfo.ecsPtr, JpuReadReg(MJPEG_BBC_RD_PTR_REG), JpuReadReg(MJPEG_BBC_WR_PTR_REG), JpuReadReg(MJPEG_BBC_CUR_POS_REG));

		if (outputInfo.indexFrameDisplay == -1)
			break;

        // YUV Dump Done when partial buffer is all displayed.
        int_reason = JPU_GetStatus();
        if(decConfig.usePartialMode && !(int_reason & 0xF0))
            goto SKIP_BUF_DUMP;
		// indexFrameDisplay points to the frame buffer, among ones registered, which holds
		// the output of the decoder.
		if (dispImage)
		{
#ifdef CNM_FPGA_PLATFORM
			if (frameIdx) 
				WaitMixerInt();	
#endif
			if (!rotEnable)
			{
				pDispFrame = FindFrameBuffer(instIdx, frameBuf[outputInfo.indexFrameDisplay].bufY);				
#ifdef CNM_FPGA_PLATFORM
				SetMixerDecOutFrame(pDispFrame, outputInfo.decPicWidth, outputInfo.decPicHeight);							
#endif
			}
			else
			{
				pDispFrame = FindFrameBuffer(instIdx, frameBuf[ppIdx].bufY);				
#ifdef CNM_FPGA_PLATFORM
				SetMixerDecOutFrame(pDispFrame, 
					(decConfig.rotAngle==90 || decConfig.rotAngle==270)?outputInfo.decPicHeight:outputInfo.decPicWidth, 
					(decConfig.rotAngle==90 || decConfig.rotAngle==270)?outputInfo.decPicWidth:outputInfo.decPicHeight);
#endif

				ppIdx = (ppIdx-regFrameBufCount+1)%MAX_ROT_BUF_NUM;		
		
			}		
		}
		else // store image
		{
			if (!rotEnable)
			{
				saveIdx = outputInfo.indexFrameDisplay;
				if (!SaveYuvImageHelperFormat(fpYuv, pYuv, 
					frameBuf[saveIdx].bufY, frameBuf[saveIdx].bufCb, frameBuf[saveIdx].bufCr,
					dispWidth, dispHeight, framebufStride, 
					decConfig.chromaInterleave, framebufFormat, decOP.frameEndian, 
					decOP.packedFormat))
					goto ERR_DEC_OPEN;	
			}
			else
			{
				saveIdx = ppIdx;

				if (!SaveYuvImageHelperFormat(fpYuv, pYuv, 
					frameBuf[saveIdx].bufY, frameBuf[saveIdx].bufCb, frameBuf[saveIdx].bufCr,
					dispWidth, dispHeight,
					framebufStride, decConfig.chromaInterleave, framebufFormat, decOP.frameEndian, 
					decOP.packedFormat))
					goto ERR_DEC_OPEN;					
				ppIdx = (ppIdx-regFrameBufCount+1)%MAX_ROT_BUF_NUM;		
			}	
			//sw_mixer_draw(instIdx, 0, 0, dispWidth, dispHeight, framebufFormat, decOP.packedFormat, decOP.chromaInterleave,  pYuv);			
		}		

SKIP_BUF_DUMP:
		if( outputInfo.numOfErrMBs ) 
		{
			int errRstIdx, errPosX, errPosY;
			errRstIdx = (outputInfo.numOfErrMBs & 0x0F000000) >> 24;
			errPosX = (outputInfo.numOfErrMBs & 0x00FFF000) >> 12;
			errPosY = (outputInfo.numOfErrMBs & 0x00000FFF);
			JLOG(ERR, "Error restart Idx : %d, MCU x:%d, y:%d, in Frame : %d \n", errRstIdx, errPosX, errPosY, frameIdx);				
		}	
		frameIdx++;

		if (decConfig.outNum && (frameIdx == decConfig.outNum)) 
			break;
		
	}
	if (totalNumofErrMbs)
	{
		suc = 0;
		JLOG(ERR, "Total Num of Error MBs : %d\n", totalNumofErrMbs);
	}

ERR_DEC_OPEN:
	// Now that we are done with decoding, close the open instance.
	ret = JPU_DecClose(handle);
	
	JLOG(INFO, "\nDec End. Tot Frame %d\n", frameIdx);


ERR_DEC_INIT:		
	FreeFrameBuffer(instIdx);

	jdi_free_dma_memory(&vbStream);
	

	if (pYuv)
		free(pYuv);

	if (pFileBuf)
		free(pFileBuf);

	if (fpYuv)
		fclose(fpYuv);
	
	//sw_mixer_close(instIdx);
	JPU_DeInit();

	return suc;
}


int EncodeTest(EncConfigParam *param)
{

	JpgEncHandle		handle		= { 0 };
	JpgEncOpenParam	encOP		= { 0 };
	JpgEncParam		encParam	= { 0 };
	JpgEncInitialInfo	initialInfo	= { 0 };
	JpgEncOutputInfo	outputInfo	= { 0 };
	JpgEncParamSet encHeaderParam = {0};
	jpu_buffer_t	vbStream     = {0};
	FRAME_BUF *		pFrame[NUM_FRAME_BUF];
	FrameBuffer		frameBuf[NUM_FRAME_BUF];
	JpgRet			ret = JPG_RET_SUCCESS;
	FRAME_BUF *		pDispFrame = NULL;
	int				i = 0, srcFrameIdx = 0, frameIdx = 0;
	int				srcFrameFormat = 0;
	int				framebufStride = 0, framebufWidth = 0, framebufHeight = 0, framebufFormat = 0;
	int				suc = 1, key = 0;
	Uint8 *			pYuv	= NULL;
	FILE *			fpYuv	= NULL;
	FILE *			fpSrcYuv = NULL;
	FILE *			fpYuvTmb = NULL;
	int				tmbEn;
	int				bsSize = 0;
	int				bsTmbSize = 0;
	Uint8           *pThumbnailFileBuf = NULL;
	Uint8           *pFileBuf = NULL;
	FILE *			fpTmbBitstrm = NULL;
	FILE *			fpBitstrm	= NULL;
	int				int_reason = 0;
	int				timeout_count = 0;
	int				instIdx;
    int             needFrameBufCount;

	int             partPosIdx = 0;
    int             partBufIdx = 0;
    int             partMaxIdx;    

    EncConfigParam encConfig;

	encConfig = *param;

	memset(&pFrame[0], 0x00, sizeof(FRAME_BUF *)*NUM_FRAME_BUF);
	memset(&frameBuf[0], 0x00, sizeof(FrameBuffer)*NUM_FRAME_BUF);

	
	instIdx = encConfig.instNum;
	tmbEn = encConfig.thumbEn;

THUMBNAIL_ENCODING:

	fpBitstrm = fopen(encConfig.bitstreamFileName, "wb");
	if( !fpBitstrm ) 
	{
		JLOG(ERR, "Can't open bitstream file %s \n", encConfig.bitstreamFileName );
		goto ERR_ENC_INIT;
	}

	ret = JPU_Init();
	if( ret != JPG_RET_SUCCESS && 
		ret != JPG_RET_CALLED_BEFORE )
	{
		JLOG(ERR, "JPU_Init failed Error code is 0x%x \n", ret );
		goto ERR_ENC_INIT;
	}

	if (strlen(encConfig.cfgFileName) != 0) 
		ret = getJpgEncOpenParam(&encOP, &encConfig, encConfig.yuvFileName);
	else 
		ret = getJpgEncOpenParamDefault(&encOP, &encConfig, tmbEn);
	if (ret == 0) 
		goto ERR_ENC_INIT;		

	vbStream.size = STREAM_BUF_SIZE;
	if (jdi_allocate_dma_memory(&vbStream) < 0)
	{
		JLOG(ERR, "fail to allocate bitstream buffer\n" );
		goto ERR_ENC_INIT;
	}

	encOP.streamEndian = encConfig.StreamEndian;
	encOP.frameEndian = encConfig.FrameEndian;
	encOP.chromaInterleave = (CbCrInterLeave)encConfig.chromaInterleave;
	encOP.bitstreamBuffer = vbStream.phys_addr;
	encOP.bitstreamBufferSize = vbStream.size;
	encOP.packedFormat = (PackedOutputFormat)encConfig.packedFormat;

	if(encOP.packedFormat)
	{
		if (encOP.sourceFormat == FORMAT_420 || encOP.sourceFormat == FORMAT_400)
		{
			JLOG(ERR, "Invalid operation mode : In case of using packed mode. sourceFormat must be FORMAT_444\n" );
			goto ERR_ENC_INIT;
		}
		
	}

	if(tmbEn)
	{
		fpYuvTmb = fopen(encConfig.yuvTmbFileName, "rb");
		if( !fpYuvTmb)
		{
			JLOG(ERR, "Can't open thumbnail yuv file %s \n", encConfig.yuvTmbFileName );
			goto ERR_ENC_INIT;
		}
	}
	else
	{
		fpYuv = fopen(encConfig.yuvFileName, "rb");
		if( !fpYuv ) 
		{
			JLOG(ERR, "Can't open yuv file %s \n", encConfig.yuvFileName );
			goto ERR_ENC_INIT;
		}
	}

 	fpSrcYuv = fopen("src.yuv", "wb");
 	if (!fpSrcYuv) 
 	{
 		JLOG(ERR, "Can't open  yuv file for source image for debug purpose\n");
 		goto ERR_ENC_INIT;
 	}

	srcFrameFormat = encOP.sourceFormat;

	if (encConfig.rotAngle == 90 || encConfig.rotAngle == 270)
		framebufFormat = (srcFrameFormat == FORMAT_422) ? FORMAT_224 : (srcFrameFormat == FORMAT_224) ? FORMAT_422 : srcFrameFormat;
	else
		framebufFormat = srcFrameFormat;

	// srcFrameFormat means that it is original source image format.
	// framebufFormat means that is is converted image format. 

    if(encConfig.usePartialMode)
    {
        // Rotator must be disable when partial mode is enable
	    encConfig.rotAngle = 0;
        encConfig.mirDir = 0;

        encConfig.partialHeight = (framebufFormat == FORMAT_420 || framebufFormat == FORMAT_224) ? 16 : 8;
        partMaxIdx  = ((encOP.picHeight+15)&~15) / encConfig.partialHeight;
        // constraint
        if(partMaxIdx < encConfig.partialBufNum)
            encConfig.partialBufNum = partMaxIdx;

		JLOG(INFO, "Partial Mode Enable\n "); 
        JLOG(INFO, " - Num of line for partial: %d\n ", encConfig.partialHeight); 

    }
	
	if (framebufFormat == FORMAT_420 || framebufFormat == FORMAT_422)
		framebufWidth = (((encOP.picWidth+15)/16)*16);
	else
		framebufWidth = (((encOP.picWidth+7)/8)*8);

	if (framebufFormat == FORMAT_420 || framebufFormat == FORMAT_224)
		framebufHeight = (((encOP.picHeight+15)/16)*16);
	else
		framebufHeight = (((encOP.picHeight+7)/8)*8);
		
	framebufStride = framebufWidth;
		 
	if(encOP.packedFormat >= PACKED_FORMAT_422_YUYV && encOP.packedFormat <= PACKED_FORMAT_422_VYUY)
	{
		framebufStride = framebufStride*2;	
		framebufFormat = FORMAT_422;
		if (encConfig.rotAngle == 90 || encConfig.rotAngle == 270)
			framebufFormat = FORMAT_224;
	}
	else if (encOP.packedFormat == PACKED_FORMAT_444)
	{
		framebufStride = framebufStride*3;	
		framebufFormat = FORMAT_444;		
	}

	
	pYuv = malloc(framebufStride*framebufHeight*3);
	if (!pYuv) 
	{
		JLOG(ERR, "malloc() failed \n" );
		goto ERR_ENC_INIT;
	}


	
	// Open an instance and get initial information for encoding.
	ret = JPU_EncOpen(&handle, &encOP);
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_EncOpen failed Error code is 0x%x \n", ret );
		goto ERR_ENC_INIT;
	}

	//JPU_EncGiveCommand(handle, ENABLE_LOGGING, NULL);
	if (encConfig.useRot)
	{
		JPU_EncGiveCommand(handle, ENABLE_JPG_ROTATION, 0);
		JPU_EncGiveCommand(handle, ENABLE_JPG_MIRRORING, 0);
		JPU_EncGiveCommand(handle, SET_JPG_ROTATION_ANGLE, &encConfig.rotAngle);
		JPU_EncGiveCommand(handle, SET_JPG_MIRROR_DIRECTION, &encConfig.mirDir);
	}

    JPU_EncGiveCommand(handle, SET_JPG_USE_PARTIAL_MODE,  &encConfig.usePartialMode );
    JPU_EncGiveCommand(handle, SET_JPG_PARTIAL_FRAME_NUM, &encConfig.partialBufNum);
    JPU_EncGiveCommand(handle, SET_JPG_PARTIAL_LINE_NUM,  &encConfig.partialHeight);
    JPU_EncGiveCommand(handle, SET_JPG_USE_STUFFING_BYTE_FF, &encConfig.bEnStuffByte);

		
	ret = JPU_EncGetInitialInfo(handle, &initialInfo);
	if( ret != JPG_RET_SUCCESS )
	{
		JLOG(ERR, "JPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
		goto ERR_ENC_OPEN;
	}

	
    if(encConfig.usePartialMode)
    {
        needFrameBufCount  = encConfig.partialBufNum;
        partMaxIdx = ((encOP.picHeight +15)&~15) / encConfig.partialHeight;

		// Initialize frame buffers for encoding and source frame
		if (!AllocateFrameBuffer(instIdx, srcFrameFormat, framebufStride, encConfig.partialHeight, needFrameBufCount, encOP.packedFormat))
		{
			JpgLeaveLock();
			goto ERR_ENC_OPEN;
		}
    }       
    else
	{
		needFrameBufCount = ENC_SRC_BUF_NUM;

		// Initialize frame buffers for encoding and source frame
		if (!AllocateFrameBuffer(instIdx, srcFrameFormat, framebufStride, framebufHeight, needFrameBufCount , encOP.packedFormat))
		{
			JpgLeaveLock();
			goto ERR_ENC_OPEN;
		}
	}
	JpgEnterLock();	
	for( i = 0; i < needFrameBufCount; ++i ) 
	{
		pFrame[i] = GetFrameBuffer(instIdx, i);

		frameBuf[i].stride = pFrame[i]->strideY;
		frameBuf[i].bufY  = pFrame[i]->vbY.phys_addr;
		frameBuf[i].bufCb = pFrame[i]->vbCb.phys_addr;
		frameBuf[i].bufCr = pFrame[i]->vbCr.phys_addr;
		ClearFrameBuffer(instIdx, i);
	}

	JpgLeaveLock();

	JLOG(INFO, "framebuffer stride = %d, width = %d, height = %d\n", framebufStride, framebufWidth, framebufHeight);
	JLOG(INFO, "framebuffer format = %d, srcFrameFormat : %d, packed format = %d, Interleave = %d\n", framebufFormat, srcFrameFormat, encOP.packedFormat, encOP.chromaInterleave);
	
	    
	printf("Enc Start : Press enter key to show menu.\n" );
	printf("          : Press space key to stop.\n" );

	while( 1 ) 
	{
		srcFrameIdx = (frameIdx%ENC_SRC_BUF_NUM);

        // Write picture header
	    if (encConfig.encHeaderMode == ENC_HEADER_MODE_NORMAL)
	    {
		    encHeaderParam.size = STREAM_BUF_SIZE;
		    encHeaderParam.headerMode = ENC_HEADER_MODE_NORMAL;			//Encoder header disable/enable control. Annex:A 1.2.3 item 13
		    encHeaderParam.quantMode = JPG_TBL_NORMAL; //JPG_TBL_MERGE	// Merge quantization table. Annex:A 1.2.3 item 7
		    encHeaderParam.huffMode  = JPG_TBL_NORMAL; // JPG_TBL_MERGE	//Merge huffman table. Annex:A 1.2.3 item 6
		    encHeaderParam.disableAPPMarker = 0;						//Remove APPn. Annex:A item 11

		    if (encHeaderParam.headerMode == ENC_HEADER_MODE_NORMAL) {
                //make picture header
			    JPU_EncGiveCommand(handle, ENC_JPG_GET_HEADER, &encHeaderParam); // return exact header size int endHeaderparam.siz;

		    }
	    }


        // Partial Buffer Control
		if(encConfig.usePartialMode)
        {
			partBufIdx = 0;
            partPosIdx = 0;
			JPU_SWReset();
			// Get a yuv frame
		    if( !LoadYuvPartialImageHelperFormat( fpYuv, pYuv, 
			    frameBuf[partBufIdx].bufY, 
			    frameBuf[partBufIdx].bufCb,
			    frameBuf[partBufIdx].bufCr, 
			    framebufWidth, 
			    framebufHeight, 
                encConfig.partialHeight,
			    framebufStride, encOP.chromaInterleave, srcFrameFormat, encOP.frameEndian, partPosIdx, frameIdx, encConfig.packedFormat) )
			    break;	// must break to read last bitstream buffer

        }
        else 
		{
			if(tmbEn)
			{
				// Get a yuv frame
				if( !LoadYuvImageHelperFormat( fpYuvTmb, pYuv, 
					frameBuf[srcFrameIdx].bufY, 
					frameBuf[srcFrameIdx].bufCb,
					frameBuf[srcFrameIdx].bufCr, 
					framebufWidth, 
					framebufHeight, 
					framebufStride, encOP.chromaInterleave, srcFrameFormat, encOP.frameEndian, encConfig.packedFormat) )
					break;	// must break to read last bit-stream buffer
			}
			else
			{
			
				// Get a yuv frame
				if( !LoadYuvImageHelperFormat( fpYuv, pYuv, 
					frameBuf[srcFrameIdx].bufY, 
					frameBuf[srcFrameIdx].bufCb,
					frameBuf[srcFrameIdx].bufCr, 
					framebufWidth, 
					framebufHeight, 
					framebufStride, encOP.chromaInterleave, srcFrameFormat, encOP.frameEndian, encConfig.packedFormat) )
					break;	// must break to read last bit-stream buffer
			}

        }


    if(encConfig.usePartialMode)
    {
        // need more frame buffer when partial mode is enable
        encParam.sourceFrame = frameBuf;
    }
    else{
        encParam.sourceFrame = &frameBuf[srcFrameIdx];
    }
	
        // Start encoding a frame.		
	    ret = JPU_EncStartOneFrame(handle, &encParam, tmbEn);
		if( ret != JPG_RET_SUCCESS )
		{
			JLOG(ERR, "JPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
			goto ERR_ENC_OPEN;
		}

		while(1) 
		{
  		    if (kbhit()) {
			    key = getch();
			    fflush(stdout);
		    }

			if (key) {
				if (key == ' ')
				{
					ret = JPU_EncIssueStop(handle);
					if (ret != JPG_RET_SUCCESS)
					{
						JLOG(ERR, "JPU_EncIssueStop failed Error code is 0x%x \n", ret );
						goto ERR_ENC_OPEN;
					}                               
					//should break after interrupt

				}
			}

			int_reason = JPU_WaitInterrupt(JPU_INTERRUPT_TIMEOUT_MS);
			if (int_reason == -1)
			{
				JLOG(ERR, "Error : timeout happened\n");
				JPU_SWReset();
				break;
			}
			
            // goto loading partial buffer routine when partial buffer is used.
            if( encConfig.usePartialMode && (int_reason>>4))
            {
				printf("ENCODE : PARTIAL BUFFER IDX %d / POS %d / MAX POS %d / INT_REASON=0x%x\n", partBufIdx, partPosIdx+1, partMaxIdx, int_reason);

				if ((1<<partBufIdx) & ((int_reason & 0xf0)>>4))
				{
					JPU_ClrStatus((1<< (INT_JPU_PARIAL_BUF0_EMPTY + partBufIdx)));

					if (int_reason & (1<<INT_JPU_DONE))	// Must catch PIC_DONE interrupt before catching EMPTY interrupt
					{
						// Do no clear INT_JPU_DONE these will be cleared in JPU_EncGetOutputInfo.
						break;
					}
					partPosIdx ++;
					partBufIdx = (partPosIdx % encConfig.partialBufNum);
					// Get a yuv frame
					LoadYuvPartialImageHelperFormat( fpYuv, pYuv, 
						frameBuf[partBufIdx].bufY, 
						frameBuf[partBufIdx].bufCb,
						frameBuf[partBufIdx].bufCr, 
						framebufWidth, 
						framebufHeight, 
						encConfig.partialHeight,
						framebufStride, encOP.chromaInterleave, srcFrameFormat, encOP.frameEndian, partPosIdx, frameIdx, encConfig.packedFormat);						

					JPU_EncGiveCommand(handle, SET_JPG_ENCODE_NEXT_LINE, NULL);					
					
					continue;
				}
				else
				{
					JLOG(ERR, "Invalid partial interrupt : expected reason =0x%x, actual reason=0x%x \n", (1<<partBufIdx), ((int_reason & 0xf0)>>4));		
					goto ERR_ENC_OPEN;	
				}
            }

			if (int_reason & (1<<INT_JPU_DONE))	// Must catch PIC_DONE interrupt before catching EMPTY interrupt
			{
				// Do no clear INT_JPU_DONE these will be cleared in JPU_EncGetOutputInfo.
				break;			
			}

			if (int_reason & (1<<INT_JPU_BIT_BUF_EMPTY))
			{
				ret = ReadJpgBsBufHelper(handle, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBuffer + encOP.bitstreamBufferSize, 0, encOP.streamEndian);
				if( ret != JPG_RET_SUCCESS ) {
					JLOG(ERR, "ReadBsRingBufHelper failed Error code is 0x%x \n", ret );
					goto ERR_ENC_OPEN;
				}				
				JPU_ClrStatus((1<<INT_JPU_BIT_BUF_EMPTY));
			}

			 // expect this interrupt after stop is enabled.
			if (int_reason & (1<<INT_JPU_BIT_BUF_STOP)) 
			{
				ret = JPU_EncCompleteStop(handle);
				if( ret != JPG_RET_SUCCESS )
				{
					JLOG(ERR, "JPU_EncCompleteStop failed Error code is 0x%x \n", ret );
					goto ERR_ENC_OPEN;
				}
				JPU_ClrStatus((1<<INT_JPU_BIT_BUF_STOP));
				break;
			}

			if (int_reason & (1<<INT_JPU_PARIAL_OVERFLOW))
				JPU_ClrStatus((1<<INT_JPU_PARIAL_OVERFLOW));
		}	

		ret = JPU_EncGetOutputInfo(handle, &outputInfo);
		if (ret != JPG_RET_SUCCESS)
		{
			JLOG(ERR, "JPU_EncGetOutputInfo failed Error code is 0x%x \n", ret );
			goto ERR_ENC_OPEN;
		}
		
#ifdef ENC_SOURCE_FRAME_DISPLAY
		pDispFrame = FindFrameBuffer(instIdx, frameBuf[srcFrameIdx].bufY);
#ifdef CNM_FPGA_PLATFORM
		if( frameIdx )
			WaitMixerInt();
		SetMixerDecOutFrame(pDispFrame, encOP.picWidth, encOP.picHeight);
#endif
		//sw_mixer_open(instIdx, framebufStride, framebufHeight);

		SaveYuvImageHelperFormat(fpSrcYuv, pYuv, 
			pDispFrame->vbY.phys_addr, 
			pDispFrame->vbCb.phys_addr,
			pDispFrame->vbCr.phys_addr,
			encOP.picWidth, encOP.picHeight, framebufStride, encOP.chromaInterleave, srcFrameFormat, encOP.frameEndian, encConfig.packedFormat);
		//sw_mixer_draw(instIdx, 0, 0, framebufStride, framebufHeight, srcFrameFormat, encOP.packedFormat, encOP.chromaInterleave, pYuv);			
#endif

		ReadJpgBsBufHelper(handle, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBuffer + encOP.bitstreamBufferSize, encHeaderParam.size, encOP.streamEndian);

		JLOG(TRACE, "Enc: %d:%d, rdPtr=0x%x, wrPtr=0x%x\n", instIdx, frameIdx, outputInfo.bitstreamBuffer, outputInfo.bitstreamBuffer+outputInfo.bitstreamSize);
		
		if(tmbEn)
		{		

			//fseek(fpBitstrm, 0, SEEK_SET);
			//fseek(fpBitstrm, 0, SEEK_END);
			bsTmbSize = ftell(fpBitstrm);
			fseek(fpBitstrm, 0, SEEK_SET);

			pThumbnailFileBuf = malloc(bsTmbSize);
			if (!pThumbnailFileBuf)
			{
				JLOG(ERR, "Can't open pThumbnailFileBuf. \n");
				goto ERR_ENC_INIT;
			}
			fread(pThumbnailFileBuf, bsTmbSize, sizeof( Uint8 ), fpBitstrm);
			fclose(fpBitstrm);
			FreeFrameBuffer(instIdx);
			jdi_free_dma_memory(&vbStream);
			tmbEn = 0;
			goto THUMBNAIL_ENCODING;			
		}			

		frameIdx++;
		if (frameIdx > (encConfig.outNum-1))
			break;
		JLOG(TRACE, "Encoded instIdx=%d, frameIdx=%d\n", instIdx, frameIdx);		
	}

ERR_ENC_OPEN:
	// Now that we are done with encoding, close the open instance.
	ret = JPU_EncClose(handle);
	if( ret == JPG_RET_FRAME_NOT_COMPLETE )
	{
		JPU_EncGetOutputInfo( handle, &outputInfo );
		JPU_EncClose(handle);
	}

	JLOG(INFO, "\nEnc End. Tot Frame %d\n" , frameIdx );

ERR_ENC_INIT:

	FreeFrameBuffer(instIdx);

	jdi_free_dma_memory(&vbStream);

	if (pThumbnailFileBuf)
		free(pThumbnailFileBuf);
	if (pFileBuf)
		free(pFileBuf);
	if( pYuv )
		free( pYuv );
	if( fpBitstrm )
		fclose(fpBitstrm);
	if( fpYuv )
		fclose(fpYuv);
 	if (fpSrcYuv)
 		fclose(fpSrcYuv);
	//sw_mixer_close(instIdx);

	JPU_DeInit();
	return suc;
}

int MultiInstanceTest(MultiConfigParam	*param)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WINCE)
	int i;
	int ret;
	HANDLE hThread[MAX_NUM_INSTANCE];
	DecConfigParam *pDecConfig;
	EncConfigParam *pEncConfig;

		
	for(i=0; i<param->numMulti; i++)
	{
		if (param->multiMode[i]) //decoder case
		{
			pDecConfig = &param->decConfig[i];		
			pDecConfig->instNum = i;
			pDecConfig->FrameEndian = JPU_STREAM_ENDIAN;
			pDecConfig->StreamEndian = JPU_FRAME_ENDIAN;
			hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DecodeTest, (LPVOID)pDecConfig, 0, NULL);			
		}
		else
		{
			pEncConfig = &param->encConfig[i];	
			pEncConfig->instNum = i;
			pEncConfig->FrameEndian = JPU_STREAM_ENDIAN;
			pEncConfig->StreamEndian = JPU_FRAME_ENDIAN;
			hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EncodeTest, (LPVOID)pEncConfig, 0, NULL);
						
		}

		VPU_DELAY_MS(1000);
	}
	
	WaitForMultipleObjects(param->numMulti, hThread, TRUE, INFINITE);

	for(i=0; i<param->numMulti; i++)
	{
		GetExitCodeThread(hThread[i], &ret);
		if(ret == 0)	
			return 0;		
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
		if (param->multiMode[i]) //decoder case
		{
			pDecConfig = &param->decConfig[i];		
			pDecConfig->instNum = i;
			pDecConfig->FrameEndian = JPU_STREAM_ENDIAN;
			pDecConfig->StreamEndian = JPU_FRAME_ENDIAN;

			pthread_create(&thread_id[i], NULL, FnDecodeTest, pDecConfig);			
		}
		else
		{
			pEncConfig = &param->encConfig[i];	
			pEncConfig->instNum = i;
			pEncConfig->FrameEndian = JPU_STREAM_ENDIAN;
			pEncConfig->StreamEndian = JPU_FRAME_ENDIAN;
			pthread_create(&thread_id[i], NULL, FnEncodeTest, pEncConfig);
		}

		VPU_DELAY_MS(1000);	// it makes delay to adjust a gab among threads
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


