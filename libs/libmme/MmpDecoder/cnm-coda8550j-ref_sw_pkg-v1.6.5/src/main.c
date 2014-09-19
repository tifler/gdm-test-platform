//------------------------------------------------------------------------------
// File: main.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
//#include "../vld-10/vld.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jpuconfig.h"
#include "regdefine.h"
#include "jpulog.h"
#include "jpurun.h"
#include "jpuhelper.h"

#ifdef CNM_FPGA_PLATFORM
#ifdef PLATFORM_WIN32
#include "../jdi/windows/hpi.h"
#endif
#ifdef PLATFORM_LINUX
#include "../jdi/socket/hpi_client.h"
#endif
#endif



void DumpSdram(char *strFile, int start, int byte_size, int endian, int read);
void SetDefaultClock();
#define DUMP_READ  1
#define DUMP_WRITE 0
int DumpConfig(char *fileName, void *data, int size, int rd);




int main(int argc, char **argv)
{
	char	cmdstr[265]={0};
	char	strFile[256]={0};
	int		cmd;
#ifdef CNM_FPGA_PLATFORM
	int		min_clk ; 
	int		max_clk ;
#endif
	unsigned int	addr;
	unsigned int	data;
	int		endian;
	unsigned int  conf_menu = 0;
	int		ret = 0;
	int     storeImage;
	DecConfigParam	decConfig;
	MultiConfigParam multiConfig;
	EncConfigParam	encConfig;	

	memset(&decConfig, 0x00, sizeof(DecConfigParam));
	memset(&multiConfig, 0x00, sizeof(MultiConfigParam));
	memset(&encConfig, 0x00, sizeof(EncConfigParam));	


	InitLog();
#ifdef CNM_FPGA_PLATFORM
	if (jdi_probe() < 0)
		goto ERROR_MAIN;
	jdi_hw_reset();	
#if defined(_MSC_VER)
	Sleep(500);
#else
	usleep(500*1000);
#endif
	SetDefaultClock();	
#endif	


	while (1) 
	{
		puts("-------------------------------------------");
		puts("-           JPU debug function    -");
		puts("-------------------------------------------");
		puts(" 1. Register Read from board");
		puts(" 2. Register Write to board");
		puts(" 3. SDRAM Read from board");
		puts(" 4. SDRAM Write to board");
		puts(" 7. Reset System");
#ifdef CNM_FPGA_PLATFORM
		puts(" 8. Set FPGA board clock");
		puts(" 9. Set HPI REGs for optimized timing");
#endif 	
		puts("-------------------------------------------");
		puts("50. DECODER TEST");   
		puts("52. JPEG test stream(1.jpg)decoding ");
		puts("55. LAST DECODER TEST");
		puts("-------------------------------------------");
		puts("60. ENCODER TEST");		
		puts("66. LAST ENCODER TEST");
		puts("-------------------------------------------");
		puts("70. MULTI INSTANCE TEST");
		puts("77. LAST MULTI INSTANCE TEST");
		puts("99. Exit");
		
		cmd = -1;
		puts("\nEnter option: ");
		scanf("%s", cmdstr);	
		cmd = atoi(cmdstr);


		switch (cmd) 
		{
		case 1:         // register read
			printf("Enter address (hex): ");
			scanf("%x", &addr);
			data = JpuReadReg(addr);
			printf("Data read from %08x: %08x\n", addr, data);
			break;

		case 2:         // register write
			printf("Enter address (hex): ");
			scanf("%x", &addr);
			printf("Enter data (hex): ");
			scanf("%x", &data);
			JpuWriteReg(addr, data);
			break;

		case 3:         // SDRAM read
			printf ("Enter byte address (hex): ");
			scanf("%x", &addr);
			printf ("Enter byte size (dec): ");
			scanf("%d", &data);
			printf ("Enter file name: ");
			scanf("%s", strFile);
			printf ("Enter Endian Mode [0]little [1]Big :");			
			scanf("%d", &endian);
			DumpSdram(strFile, addr, data, endian, 1);
			break;

		case 4:         // SDRAM write
			printf ("Enter byte address (hex): ");
			scanf("%x", &addr);
			printf ("Enter byte size (dec): ");
			scanf("%d", &data);
			printf ("Enter file name: ");
			scanf("%s", strFile);
			printf ("Enter Endian Mode [0]little [1]Big :");									
			scanf("%d", &endian);
			DumpSdram(strFile, addr, data, endian, 0);
			break;

		case 7: 
			jdi_hw_reset();
			jdi_release();
			jdi_init();			
			break;

		case 50:
		case 52:			
			memset(&decConfig, 0x00, sizeof(decConfig));
		case 55:
			if (cmd == 55)
			{
                if (DumpConfig("dec_cmd", &decConfig, sizeof(decConfig), DUMP_READ))
				{
					goto RUN_LAST_DEC_CMD;
				}
			}

            if(cmd == 52)
            {				
                strcpy(decConfig.bitstreamFileName,"1.jpg");
    			printf("Enter VIDEO ES or MEDIA file name: %s\n" , decConfig.bitstreamFileName);
                decConfig.roiEnable = 0;
    			printf("Enter ROI Enable [0](OFF) [1](ON) : %d\n", decConfig.roiEnable);
    			decConfig.packedFormat = 0;
    			printf("Packed stream format output [0](PLANAR) [1](YUYV) [2](UYVY) [3](YVYU) [4](VYUY) [5](YUV_444 PACKED): %d \n",decConfig.packedFormat);   
                decConfig.chromaInterleave = 0;
				printf("Chroma format type [0](SEPARATED CHROMA) [1](CBCR INTERLEAVED) [2](CRCB INTERLEAVED): %d\n",decConfig.chromaInterleave);
    
    			decConfig.StreamEndian = JPU_STREAM_ENDIAN;
    			decConfig.FrameEndian = JPU_FRAME_ENDIAN;
    
    			if(!decConfig.roiEnable && !decConfig.packedFormat)
    			{
    			    decConfig.usePartialMode = 0;
    				printf("Enter partial Mode(0: OFF 1: ON) %d\n",decConfig.usePartialMode);    				
    			}
    			if(!decConfig.usePartialMode)
    			{
    				decConfig.rotAngle = 0;    			    
    				printf("Enter rotation angle in degrees(0, 90, 180, 270): %d \n",decConfig.rotAngle);
					decConfig.mirDir = 0;
    				printf("Enter mirror direction(0-no mirror, 1-vertical, 2-horizontal, 3-both): %d \n",decConfig.mirDir);
    			}
				storeImage = 0;
    			printf("Store image?(0: display, 1: store to a file) : %d \n",storeImage);


    			if(!decConfig.roiEnable && !decConfig.packedFormat)
    			{
    			    decConfig.ThumbNailEn = 0;
    				printf(" ThumbNailEnable (0: disable, 1: enable) %d\n",decConfig.ThumbNailEn);

    			}
				decConfig.outNum = 0;
    			printf("Enter Number of Images that you want to decode(0: decode continue, -1: loop): %d \n",decConfig.outNum);
            }
			else	
            {				
    			printf("Enter VIDEO ES or MEDIA file name: ");
    			scanf("%s", decConfig.bitstreamFileName );
    			printf("Enter ROI Enable [0](OFF) [1](ON) :");
    			scanf("%x", &decConfig.roiEnable);
    			if(decConfig.roiEnable)
    			{
    				printf("Enter ROI offsetX:");
    				scanf("%d",&decConfig.roiOffsetX);
    				printf("Enter ROI offsetY:");
    				scanf("%d", &decConfig.roiOffsetY);
    				printf("Enter ROI Width:");
    				scanf("%d", &decConfig.roiWidth);
    				printf("Enter ROI Height:");
    				scanf("%d", &decConfig.roiHeight);
    			}
    			printf("Packed stream format output [0](PLANAR) [1](YUYV) [2](UYVY) [3](YVYU) [4](VYUY) [5](YUV_444 PACKED): ");
    			scanf("%x", &decConfig.packedFormat);
    			//if(decConfig.packedFormat)
    			//	decConfig.chromaInterleave = 0;
    			//else
    			//	decConfig.chromaInterleave = JPU_CHROMA_INTERLEAVE;
    
    			printf("Chroma format type [0](SEPARATED CHROMA) [1](CBCR INTERLEAVED) [2](CRCB INTERLEAVED): ");
    			scanf("%x", &decConfig.chromaInterleave);
    
    			decConfig.StreamEndian = JPU_STREAM_ENDIAN;
    			decConfig.FrameEndian = JPU_FRAME_ENDIAN;
    
    			if(!decConfig.roiEnable && !decConfig.packedFormat)
    			{
    				printf("Enter partial Mode(0: OFF 1: ON) ");
    				scanf("%d", &decConfig.usePartialMode);
    
    				if(decConfig.usePartialMode)
    				{
    					printf("Enter Num of Frame Buffer[ 2 ~ 4 ] : ");
    					scanf("%d", &decConfig.partialBufNum);
    				}
    			}
    			if(!decConfig.usePartialMode)
    			{
    				printf("Enter rotation angle in degrees(0, 90, 180, 270): ");
    				scanf("%d", &decConfig.rotAngle);
    				if (decConfig.rotAngle != 0 && decConfig.rotAngle != 90 && decConfig.rotAngle != 180 && decConfig.rotAngle != 270) {
    					JLOG(ERR, "Invalid rotation angle.\n");
    					break;
    				}
    				printf("Enter mirror direction(0-no mirror, 1-vertical, 2-horizontal, 3-both): ");
    				scanf("%d", &decConfig.mirDir);
    				if (decConfig.mirDir != 0 && decConfig.mirDir != 1 && decConfig.mirDir != 2 && decConfig.mirDir != 3) {
    					printf("Invalid mirror direction.\n");
    					break;
    				}
    				if( decConfig.rotAngle != 0 || decConfig.mirDir != 0 )
    					decConfig.useRot = 1;
    			}
    			printf("Store image?(0: display, 1: store to a file) : ");
    			scanf("%d", &storeImage);
    			if (storeImage) 
    			{
    				printf("Enter output(YUV) file name: ");
    				scanf("%s", decConfig.yuvFileName );
    			}	
    			if(!decConfig.roiEnable && !decConfig.packedFormat)
    			{
    				printf(" ThumbNailEnable (0: disable, 1: enable)");
    				scanf("%d", &decConfig.ThumbNailEn);
    			}
    			printf("Enter Number of Images that you want to decode(0: decode continue, -1: loop): ");
    			scanf("%d", &decConfig.outNum);	
            }
RUN_LAST_DEC_CMD:
#ifdef PLATFORM_LINUX
			init_keyboard();
#endif
			DumpConfig("dec_cmd", &decConfig, sizeof(decConfig), DUMP_WRITE);

			ret = DecodeTest(&decConfig);			

			if (!ret)
				JLOG(ERR, "\nFailed to DecodeTest\n");			
#ifdef PLATFORM_LINUX
			close_keyboard();
#endif
			break;	
		case 60:
			memset( &encConfig, 0x00, sizeof( encConfig ) );
		case 66:
			if (cmd == 66)
			{
				if (DumpConfig("enc_cmd", &encConfig, sizeof(encConfig), DUMP_READ))
					goto RUN_LAST_ENC_CMD;
			}
			encConfig.StreamEndian = JPU_STREAM_ENDIAN;
			encConfig.FrameEndian = JPU_FRAME_ENDIAN;
			encConfig.chromaInterleave = JPU_CHROMA_INTERLEAVE;

			encConfig.bEnStuffByte = JPU_STUFFING_BYTE_FF;
			printf("Enter ENC CFG file name [0 if manual]: ");
			scanf("%s", encConfig.cfgFileName);
			do {
				// Check CFG file validity
				FILE *fpTmp;
				fpTmp = fopen(encConfig.cfgFileName, "r");
				if (fpTmp == 0)
					encConfig.cfgFileName[0] = 0;
				else
					fclose(fpTmp);
			} while(0);

			if (strlen(encConfig.cfgFileName) == 0)
			{
				// Manual Config Mode
				printf("Enter Image(YUV) file name: ");	
				scanf("%s", encConfig.yuvFileName );
				printf("Enter picture width: ");
				scanf("%d", &encConfig.picWidth );
				printf("Enter picture height: ");
				scanf("%d", &encConfig.picHeight );
				// Thumbnail image
				printf("Enter thumbnail encoding : [0] NO [1] YES : ");
				scanf("%d", &encConfig.thumbEn);
				if(encConfig.thumbEn){
					printf("Enter Thumbnail Image(YUV) file name: ");
					scanf("%s", encConfig.yuvTmbFileName);
					printf("Enter Thumbnail picture width: ");
					scanf("%d", &encConfig.picTmbWidth);
					printf("Enter Thumbnail picture height: ");
					scanf("%d", &encConfig.picTmbHeight);
				}				
				strcpy(encConfig.cfgFileName, "");
			}		

			printf("Enter bitstream file name: ");
			scanf("%s", encConfig.bitstreamFileName );

			if(strlen(encConfig.cfgFileName) == 0) 
			{		// MJPG
				printf("Huffman Table file name(0 : use pre-defined table in Ref-S/W) : ");	
				scanf("%s", encConfig.huffFileName );
				printf("Q Matrix Table file name(0 : use pre-dfined table in Ref-S/W) : ");
				scanf("%s", encConfig.qMatFileName );				
				printf("Enter Frame Format [0](PLANAR) [1](YUYV) [2](UYVY) [3](YVYU) [4](VYUY) [5](YUV_444 PACKED): ");
				scanf("%d", &encConfig.packedFormat);
				
				printf("Enter Source Chroma Format 0 (4:2:0) / 1 (4:2:2) / 2 (2:2:4 4:2:2 rotated) / 3 (4:4:4) / 4 (4:0:0)  : ");
				scanf("%d", &encConfig.mjpgChromaFormat);				
			}

			printf("Enter partial Mode(0: OFF 1: ON) ");
			scanf("%d", &encConfig.usePartialMode);

			if(encConfig.usePartialMode)
			{
				printf("Enter Num of Frame Buffer[ 2 ~ 4 ] : ");
				scanf("%d", &encConfig.partialBufNum);
			}


			if(encConfig.usePartialMode == 0)
			{
				// Rotation parameter
				printf("Enter rotation angle in degrees(0, 90, 180, 270): ");
				scanf("%d", &encConfig.rotAngle);
				if (encConfig.rotAngle != 0 && encConfig.rotAngle != 90 && encConfig.rotAngle != 180 && encConfig.rotAngle != 270) {
					JLOG(ERR, "Invalid rotation angle.\n");
					break;
				}
				// Flip parameter
				printf("Enter mirror direction(0-no mirror, 1-vertical, 2-horizontal, 3-both): ");
				scanf("%d", &encConfig.mirDir);
				if (encConfig.mirDir != 0 && encConfig.mirDir != 1 && encConfig.mirDir != 2 && encConfig.mirDir != 3) {
					printf("Invalid mirror direction.\n");
					break;
				}
			}

			if( encConfig.rotAngle != 0 || encConfig.mirDir != 0 )
				encConfig.useRot = 1;
	
RUN_LAST_ENC_CMD:
#ifdef PLATFORM_LINUX
			init_keyboard();
#endif		
            DumpConfig("enc_cmd", &encConfig, sizeof(encConfig), DUMP_WRITE);
			ret = EncodeTest(&encConfig);
			if (!ret)
				JLOG(ERR, "\nFailed to EncodeTest\n");

#ifdef PLATFORM_LINUX
			close_keyboard();
#endif
			break;
		case 70:        // MULTI INSTANCE TEST
			memset(&multiConfig, 0x00, sizeof(multiConfig));
			memset(&multiConfig.encConfig[0], 0x00, sizeof(EncConfigParam)*MAX_NUM_INSTANCE);
			memset(&multiConfig.decConfig[0], 0x00, sizeof(DecConfigParam)*MAX_NUM_INSTANCE);
        case 77:
			if (cmd == 77)
			{
				if (DumpConfig("mul_cmd", &multiConfig, sizeof(multiConfig), DUMP_READ))
					goto RUN_LAST_MULTI_CMD;
			}
			do 
			{
				FILE *fp;
				int i;
				int val = 0;
				char str[1024];

				fp = fopen("multi.lst", "r");
				if (fp == NULL) {
					JLOG(ERR, "\nFailed to open list file\n");
					break;
				}
				fscanf(fp, "%d", &val);
				multiConfig.numMulti = val;
				JLOG(INFO, "\nLoading list [%d]\n", multiConfig.numMulti);
				for (i=0; i < multiConfig.numMulti; i++) 
				{
					fscanf(fp, "%s %d", str, &val);
					JLOG(INFO, "INSTANCE[%d] : %s, %d\n", i, str, val);
					strcpy(multiConfig.multiFileName[i], str);
					multiConfig.multiMode[i] = val;

					if (multiConfig.multiMode[i]) //decoder 
					{
						memset(&multiConfig.decConfig[i], 0x00, sizeof(DecConfigParam));
						strcpy(multiConfig.decConfig[i].bitstreamFileName, str);
#ifdef CNM_FPGA_PLATFORM
						sprintf(str, "out%d.yuv",i);
						strcpy(multiConfig.decConfig[i].yuvFileName, str);
#endif
					}
					else
					{
						memset(&multiConfig.encConfig[i], 0x00, sizeof(EncConfigParam));
						strcpy(multiConfig.encConfig[i].cfgFileName, str);

						do{
							// Check CFG file validity
							FILE *fpTmp;
							fpTmp = fopen(multiConfig.encConfig[i].cfgFileName, "r");
							if (fpTmp == 0)
								multiConfig.encConfig[i].cfgFileName[0] = 0;
							else
								fclose(fpTmp);
						}while(0);
						sprintf(str, "out%d.jpg",i);
						strcpy(multiConfig.encConfig[i].bitstreamFileName, str);
					}
				}
				fclose(fp);

#ifdef PLATFORM_LINUX
				init_keyboard();
#endif

RUN_LAST_MULTI_CMD:
				DumpConfig("mul_cmd", &multiConfig, sizeof(multiConfig), DUMP_WRITE);
				ret = MultiInstanceTest(&multiConfig);
				if (!ret)
					JLOG(ERR, "\nFailed to MultiDecodeTest\n");
#ifdef PLATFORM_LINUX
				close_keyboard();
#endif
			} while(0);

			break;
		case 99:
			ret = 1;
			goto ERROR_MAIN;
			break;
		}
	}

ERROR_MAIN:	
	jdi_release();
	DeInitLog();
 	return ret;
}

void SetDefaultClock()
{
#ifdef CNM_FPGA_PLATFORM
	int aclk_freq, cclk_freq;

	// Clock Default
	aclk_freq		= 20;
	cclk_freq		= 20;

	printf("Set default ACLK to %d\n", aclk_freq);
	jdi_set_clock_freg(0, aclk_freq, 10);	// ACLK
	printf("Set default CCLK to %d\n", cclk_freq);
	jdi_set_clock_freg(1, cclk_freq, 10);	// CCLK

#endif
}

void DumpSdram(char *strFile, int start, int byte_size, int endian, int read)
{
	FILE *fp;
	BYTE *pBuf;
	jpu_buffer_t buffer;

	if (read)
		fp = fopen(strFile, "wb");		
	else
		fp = fopen(strFile, "rb");		

	if (fp == NULL) {
		fprintf(stderr, "file not created\n");
		return;
	}

	buffer.phys_addr = start;
	buffer.size = byte_size;
	jdi_allocate_dma_memory(&buffer);


	pBuf = malloc(byte_size);
	if (!pBuf)
		return;

	if (read)
	{
		JpuReadMem(start, pBuf, byte_size, endian);
		fwrite(pBuf, 1, byte_size, fp);
	}
	else
	{
		fread(pBuf, 1, byte_size, fp);
		JpuWriteMem(start, pBuf, byte_size, endian);

	}
	jdi_free_dma_memory(&buffer);

	free(pBuf);
	fclose(fp);
}

int DumpConfig(char *fileName, void *data, int size, int rd)
{
	FILE *fp = NULL;

	if (rd)
	{
		fp = fopen(fileName, "rb");		
		if (!fp)
			return 0;

		fread(data, 1, size, fp);
	}
	else
	{
		fp = fopen(fileName, "wb");
		if (!fp)
			return 0;

		fwrite(data, 1, size, fp);
	}

	fclose(fp);

	return 1;
}

