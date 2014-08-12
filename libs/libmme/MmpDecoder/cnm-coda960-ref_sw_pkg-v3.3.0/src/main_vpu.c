//------------------------------------------------------------------------------
// File: main.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#include "vpuapi.h"
#include "vpuapifunc.h"
#include "regdefine.h"
#include "vpurun.h"
#include "vpuhelper.h"
#include "../vdi/vdi.h"
#include "../vdi/vdi_osal.h"
#include <stdio.h>
#include <memory.h>


#ifdef CNM_FPGA_PLATFORM
void DumpSdram(Uint32 core_idx, char *strFile, int start, int byte_size, int endian, int read);
int SetDefaultClock(Uint32 core_idx);
#endif
#define DUMP_READ  1
#define DUMP_WRITE 0

int DumpConfig(char *fileName, void *data, int size, int rd);


#ifdef NDEBUG
/* To avoid link error on release configuration */
int _get_output_format(void)
{
    return 0;
}
#endif

#ifdef ANDROID
#define CHECK_CORE_INDEX(COREINDEX) if (COREINDEX > (MAX_NUM_VPU_CORE-1) ) {  break; }
#else
#define CHECK_CORE_INDEX(COREINDEX) if (COREINDEX > (MAX_NUM_VPU_CORE-1) ) { fprintf(stderr, "Invalid Core Index\n"); break; } 
#endif


int main_vpu(int argc, char **argv)
{
	char	cmdstr[265]={0};
	char	strFile[256]={0};
    int		storeImage;
	int		cmd;
	int		coreNum;
	Uint32 core_idx;
	Uint32	addr;
    Uint32	data;
	int		ret = 0;
	VpuReportConfig_t reportCfg;
	DecConfigParam	decConfig;
	MultiConfigParam multiConfig;
	int i;
	

	EncConfigParam	encConfig;	
	int Cache_set_flag = 0;

#ifdef CNM_FPGA_PLATFORM
	if (sizeof(CodecInst) > MAX_INST_HANDLE_SIZE) 
	{
		printf("MAX_INST_HANDLE_SIZE is too small than sizeof(CodecInst), sizeof(CodecInst)=%d, MAX_INST_HANDLE_SIZE=%d\n", sizeof(CodecInst), MAX_INST_HANDLE_SIZE);
		return 0;
	}	
#endif

	memset(&decConfig, 0x00, sizeof(DecConfigParam));
	memset(&multiConfig, 0x00, sizeof(MultiConfigParam));
	memset(&encConfig, 0x00, sizeof(EncConfigParam));	
		
	
	
	InitLog();

	coreNum = 0;


	while (1) 
	{
		puts("-------------------------------------------");
        puts("-           VPU debug function			-");
        puts("-------------------------------------------");
#ifdef CNM_FPGA_PLATFORM
		puts(" 1. Register Read from board");
        puts(" 2. Register Write to board");
		puts(" 3. SDRAM Read from board");
        puts(" 4. SDRAM Write to board");
		puts(" 5. SDRAM Clear");
        puts(" 6. BIT Status report");
		puts(" 7. Reset System");
		puts(" 8. Set FPGA board clock");
		puts(" 9. Set HPI REGs for optimized timing");
		puts("10. Reset All Core");
#endif 	
        puts("-------------------------------------------");
        puts("50. DECODER TEST");
        puts("55. LAST DECODER TEST");
		puts("-------------------------------------------");
		puts("60. ENCODER MPEG4/H.264");		
		puts("66. LAST ENCODER TEST");
		puts("70. MULTI INSTANCE TEST");
		puts("71. LAST MULTI INSTANCE TEST");
#ifdef SUPPORT_FFMPEG_DEMUX
		puts("-------------------------------------------");
		puts("90. TRANSCODING TEST");
		puts("91. LAST TRANSCODING TEST");
#endif
        // [96 - 99] : Cache configuration
        puts("-------------------------------------------");
        puts("96. Maverick Cache Setup");
        puts("97. Maverick Cache Setup - View");
        puts("-------------------------------------------");			
		puts("99. Exit");
		
		cmd = -1;

        puts("\nEnter option: ");
        scanf("%s", cmdstr);	
        cmd = atoi(cmdstr);

		switch (cmd) 
		{
		case 50:
		case 51:
		case 55:

			memset( &decConfig, 0x00, sizeof( decConfig) );
			decConfig.cmd = cmd;
			if (cmd == 55)
			{
				if (DumpConfig("dec_cmd", &decConfig, sizeof(decConfig), DUMP_READ))
				{
					printf("VIDEO ES or MEDIA file name : %s\n", decConfig.bitstreamFileName);
					printf("Bit stream Mode : %d\n", decConfig.bitFormat);
					printf("Rotation angle in degrees : %d, Mirror direction : %d\n", decConfig.rotAngle, decConfig.mirDir);
					printf("Enter Frame buffer Map Type : %d\n", decConfig.mapType); 
					printf("Bitstream Mode(0: Interrupt mode, 1: Rollback mode, 2: PicEnd mode) : %d\n", decConfig.bitstreamMode);
					printf("WTL mode(0: Not use, 1: Frame mode) : %d\n", decConfig.wtlEnable);
					if (decConfig.wtlEnable)
						decConfig.wtlEnable = 1;
					cmd = decConfig.cmd;
					goto RUN_LAST_DEC_CMD;
				}
			}
			printf("Enter VPU Core Index: ");
			scanf("%d", &decConfig.coreIdx);
			CHECK_CORE_INDEX(decConfig.coreIdx);
			
			printf("Enter VIDEO ES or MEDIA file name: ");
			scanf("%s", decConfig.bitstreamFileName );

			if (cmd != 51)
			{
				printf("Enter Bit stream Mode \n");
				printf("0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8) "); 
#ifdef SUPPORT_FFMPEG_DEMUX
				printf("/ 12(FilePlay, Theora, VP3) : ");
#endif
				scanf("%d", &decConfig.bitFormat);


				if (decConfig.bitFormat == STD_AVC) {
					printf("Enter AVC extension 0(No) / 1(MVC) : ");
					scanf("%d", &decConfig.avcExtension);				
				}
				else if (decConfig.bitFormat == STD_MPEG4) {
					printf("Enter MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) : ");
					scanf("%d", &decConfig.mpeg4Class);	
				}

				printf("Enter rotation angle in degrees(0, 90, 180, 270): ");
				scanf("%d", &decConfig.rotAngle);
				if (decConfig.rotAngle != 0 && decConfig.rotAngle != 90 && decConfig.rotAngle != 180 && decConfig.rotAngle != 270) {
					VLOG(ERR, "Invalid rotation angle.\n");
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
            if(!Cache_set_flag)
            {
                decConfig.frameCacheBypass   = 0;
                decConfig.frameCacheBurst    = 0;
                decConfig.frameCacheMerge    = 3;
                decConfig.frameCacheWayShape = 15;			
            }
            printf("Enter Frame buffer Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear): ");
            scanf("%d", &decConfig.mapType);		


			printf("Store image?(0: display, 1: store to a file) : ");
			scanf("%d", &storeImage);
			if (storeImage) 
			{
				printf("Enter output(YUV) file name: ");
				scanf("%s", decConfig.yuvFileName );
			}	
			else
			{

				decConfig.lowDelayInfo.lowDelayEn = 0;
				decConfig.lowDelayInfo.numRows      = 0;
				if (decConfig.bitFormat == STD_AVC && decConfig.avcExtension==0) 
				{
					printf("Do you want to display before one picture decoding done?(0:no, 1:yes) : ");
					scanf("%d", &decConfig.lowDelayInfo.lowDelayEn);
					if (decConfig.lowDelayInfo.lowDelayEn)
					{
						printf("Enter the number of mb rows : ");
						scanf("%d", &decConfig.lowDelayInfo.numRows);
						if (decConfig.lowDelayInfo.numRows==0)
							decConfig.lowDelayInfo.lowDelayEn = 0;
					}
				}
			}

	
            printf("Enter Number of Images that you want to decode(0: decode continue, -1: loop): ");
            scanf("%d", &decConfig.outNum);	
            if (decConfig.bitFormat >= 12)
                printf("Enter Bitstream Mode(0: Interrupt mode, 1: Rollback mode, 2: PicEnd mode): ");
            else
                printf("Enter Bitstream Mode(0: Interrupt mode, 1: Rollback mode): ");
            scanf("%d", &decConfig.bitstreamMode);
			if (cmd != 51)
			{
                printf("Enter WTL mode(0: Not use, 1: Frame mode): ");
                scanf("%d", &decConfig.wtlEnable);

                if (decConfig.wtlEnable)
                    decConfig.wtlEnable = 1;
			}

			decConfig.maxWidth = 0;
			decConfig.maxHeight = 0;
            decConfig.mp4DeblkEnable = 0;
			decConfig.iframeSearchEnable = 0; // 1:IFrameSearch & IDR only in AVC, VC1 field, 2:IFrameSearch enable 
			decConfig.skipframeMode = 0; // 1:PB skip, 2:B skip

            if( decConfig.outNum < 0 )
            {
                decConfig.checkeos = 0;
                decConfig.outNum = 0;
            }
            else
            {
                decConfig.checkeos = 1;
            }
            
				
RUN_LAST_DEC_CMD:
            do {
                memset(&reportCfg, 0x00, sizeof(reportCfg));
                reportCfg.userDataEnable = VPU_REPORT_USERDATA;
                reportCfg.userDataReportMode = 0;
                OpenDecReport(decConfig.coreIdx, &reportCfg);
            } while(0);

			DumpConfig("dec_cmd", &decConfig, sizeof(decConfig), DUMP_WRITE);

            osal_init_keyboard();

				
#ifdef SUPPORT_FFMPEG_DEMUX
            if (decConfig.bitFormat >= 12)
				ret = FilePlayTest(&decConfig);
			else
				ret = DecodeTest(&decConfig);
#else
			ret = DecodeTest(&decConfig);
#endif	//	SUPPORT_FFMPEG_DEMUX

			DumpConfig("dec_cmd", &decConfig, sizeof(decConfig), DUMP_WRITE);

			if (!ret)
				VLOG(ERR, "\nFailed to DecodeTest\n");
			CloseDecReport(decConfig.coreIdx);
			osal_close_keyboard();
			break;	
			
		case 60:        // MPEG4/H.264 ENCODER
		case 66:
			memset( &encConfig, 0x00, sizeof( encConfig ) );

			if (cmd == 66)
			{
				if (DumpConfig("enc_cmd", &encConfig, sizeof(encConfig), DUMP_READ))
					goto RUN_LAST_ENC_CMD;
			}

			if (cmd != 51)
			{
				printf("Enter VPU Core Index: ");
				scanf("%d", &encConfig.coreIdx);
				CHECK_CORE_INDEX(encConfig.coreIdx);
			}
			
			printf("Enter ENC CFG file name [0 if manual]: ");
			scanf("%s", encConfig.cfgFileName);
			do {
				// Check CFG file validity
				osal_file_t fpTmp;
				fpTmp = osal_fopen(encConfig.cfgFileName, "r");
				if (fpTmp == 0)
					encConfig.cfgFileName[0] = 0;
				else
					osal_fclose(fpTmp);
			} while(0);

			if (strlen(encConfig.cfgFileName) == 0 || 
				encConfig.cfgFileName[0] == '0' && strlen(encConfig.cfgFileName) == 1 )
			{
				// Manual Config Mode
				printf("Enter Image(YUV) file name: ");	
				scanf("%s", encConfig.yuvFileName );
				printf("Enter picture width: ");
				scanf("%d", &encConfig.picWidth );
				printf("Enter picture height: ");
				scanf("%d", &encConfig.picHeight );
				printf("Enter bitrate in kbps: ");
				scanf("%d", &encConfig.kbps );
				printf("Enter frame number: ");
				scanf("%d", &encConfig.outNum );

				strcpy(encConfig.cfgFileName, "");
			}
		
				
			printf("Enter bitstream file name: ");
			scanf("%s", encConfig.bitstreamFileName );
			
			printf("Enter Bit stream Mode 0(MPEG4) / 1(H.263) / 2(H.264): ");
			scanf("%d", &encConfig.stdMode );


#ifdef SUPPORT_FFMPEG_DEMUX
            //support only line buffer mode
            if ((strstr(encConfig.bitstreamFileName, ".avi") || 
                    strstr(encConfig.bitstreamFileName, ".mp4")))
            {
                printf("Do you want to use %s container?\n"
                       "Bit stream buffer mode will be automatically selected a line buffer mode\n" 
                       "(0:element stream, 1:use %s container) : ", 
                            strrchr(encConfig.bitstreamFileName, '.'), strrchr(encConfig.bitstreamFileName, '.'));
                scanf("%d", &encConfig.en_container);
                
                if (encConfig.en_container)
                {
                    printf("Enter frame rate?(ex:25, fps=25) : ");
                    scanf("%d", &encConfig.container_frame_rate);
                    if (encConfig.container_frame_rate < 1 )
                    {
                        VLOG(ERR, "Invalid frame rate\n");
                        break;
                    }
                }
                encConfig.ringBufferEnable = 0;
    			printf("Bit stream buffer Mode 0(line buffer) was selected\n");
                goto SKIP_BUFFER_MODE;
            }
#endif
			printf("Enter Bit stream buffer Mode 0(line buffer) / 1(ring buffer) : ");
			scanf("%d", &encConfig.ringBufferEnable);

#ifdef SUPPORT_FFMPEG_DEMUX
SKIP_BUFFER_MODE:
#endif

            if (!encConfig.ringBufferEnable)
            {
                printf("Line buffer interrupt mode enable? 0(disable) / 1(enable) : ");
                scanf("%d", &encConfig.lineBufIntEn);
            }
			if(!Cache_set_flag)
			{			
				encConfig.frameCacheBypass   = 0;
				encConfig.frameCacheBurst    = 0;
				encConfig.frameCacheMerge    = 3;
				encConfig.frameCacheWayShape = 15;			
			}
			printf("Enter Frame buffer Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Linear2Tiled): ");
			scanf("%d", &encConfig.mapType);
			// Rotation parameter
			printf("Enter rotation angle in degrees(0, 90, 180, 270): ");
			scanf("%d", &encConfig.rotAngle);
			if (encConfig.rotAngle != 0 && encConfig.rotAngle != 90 && encConfig.rotAngle != 180 && encConfig.rotAngle != 270) {
				VLOG(ERR, "Invalid rotation angle.\n");
				break;
			}
			// Flip parameter
			printf("Enter mirror direction(0-no mirror, 1-vertical, 2-horizontal, 3-both): ");
			scanf("%d", &encConfig.mirDir);
			if (encConfig.mirDir != 0 && encConfig.mirDir != 1 && encConfig.mirDir != 2 && encConfig.mirDir != 3) {
				printf("Invalid mirror direction.\n");
				break;
			}
			if( encConfig.rotAngle != 0 || encConfig.mirDir != 0 )
				encConfig.useRot = 1;


		

			
RUN_LAST_ENC_CMD:
			DumpConfig("enc_cmd", &encConfig, sizeof(encConfig), DUMP_WRITE);

			osal_init_keyboard();

			ret = EncodeTest(&encConfig);

			if (!ret)
				VLOG(ERR, "\nFailed to EncodeTest\n");
			osal_close_keyboard();
			break;
		case 70:        // MULTI INSTANCE TEST
		case 71:
			if (cmd == 71)
			{
				if (DumpConfig("mul_cmd", &multiConfig, sizeof(multiConfig), DUMP_READ))
					goto RUN_LAST_MULTI_CMD;
			}

			memset(&multiConfig, 0x00, sizeof(multiConfig));
			memset(&multiConfig.encConfig[0], 0x00, sizeof(EncConfigParam)*MAX_NUM_INSTANCE);
			memset(&multiConfig.decConfig[0], 0x00, sizeof(DecConfigParam)*MAX_NUM_INSTANCE);
			do 
			{
				osal_file_t fp;
				int i;
				int val = 0;
				char str[1024];

				fp = osal_fopen("multi.lst", "r");
				if (fp == NULL) {
					VLOG(ERR, "\nFailed to open list file\n");
					break;
				}
				fscanf(fp, "%d", &val);
				multiConfig.numMulti = val;
				VLOG(INFO, "\nLoading list [%d]\n", multiConfig.numMulti);
				for (i=0; i < multiConfig.numMulti; i++) 
				{
					multiConfig.decConfig[i].frameCacheBypass   = 0;
					multiConfig.decConfig[i].frameCacheBurst    = 0;
					multiConfig.decConfig[i].frameCacheMerge    = 3;
					multiConfig.decConfig[i].frameCacheWayShape = 15;
					multiConfig.encConfig[i].frameCacheBypass   = 0;
					multiConfig.encConfig[i].frameCacheBurst    = 0;
					multiConfig.encConfig[i].frameCacheMerge    = 3;
					multiConfig.encConfig[i].frameCacheWayShape = 15; 					
					fscanf(fp, "%s %d", str, &val);
					VLOG(INFO, "INSTANCE[%d] : %s, %d\n", i, str, val);
					strcpy(multiConfig.multiFileName[i], str);
					multiConfig.multiMode[i] = val;

					if (val >= STD_MP4_ENC && val <= (STD_AVC_ENC+1))
					{
						sprintf(multiConfig.encConfig[i].cfgFileName, "%s%s", "./", str);
						
						do{
							// Check CFG file validity
							osal_file_t fpTmp;
							fpTmp = osal_fopen(multiConfig.encConfig[i].cfgFileName, "r");
							if (fpTmp == 0)
								multiConfig.encConfig[i].cfgFileName[0] = 0;
							else
								osal_fclose(fpTmp);
						}while(0);

						if (val == STD_AVC_ENC)
							sprintf(str, "/tmp/out%d.264",i);
						else
							sprintf(str, "/tmp/out%d.mp4",i);
						multiConfig.encConfig[i].stdMode = multiConfig.multiMode[i] - STD_MP4_ENC;
						strcpy(multiConfig.encConfig[i].bitstreamFileName, str);
						multiConfig.encConfig[i].rotAngle = 0;
						multiConfig.encConfig[i].mirDir = 0;
						multiConfig.encConfig[i].useRot = 0;
						multiConfig.encConfig[i].ringBufferEnable = 0;
					}
					else
					{

						multiConfig.decConfig[i].bitFormat = multiConfig.multiMode[i];
						if (multiConfig.decConfig[i].bitFormat == 100)
                        {
							multiConfig.decConfig[i].runFilePlayTest = 1;
                            multiConfig.decConfig[i].bitstreamMode = 2;
                        }
						strcpy(multiConfig.decConfig[i].bitstreamFileName, multiConfig.multiFileName[i]);
						printf("%s :\n", multiConfig.decConfig[i].bitstreamFileName);
						printf("Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear): ");						
						
						scanf("%d", &multiConfig.decConfig[i].mapType);		
						//sprintf(multiConfig.decConfig[i].yuvFileName, "out%d.yuv", i);
                        multiConfig.decConfig[i].coreIdx = multiConfig.decConfig->coreIdx;

					}
				}
				osal_fclose(fp);

				
RUN_LAST_MULTI_CMD:
				DumpConfig("mul_cmd", &multiConfig, sizeof(multiConfig), DUMP_WRITE);

	 	 	 	osal_init_keyboard();

				ret = MultiInstanceTest(&multiConfig);

				DumpConfig("mul_cmd", &multiConfig, sizeof(multiConfig), DUMP_WRITE);

				if (!ret)
					VLOG(ERR, "\nFailed to MultiDecodeTest\n");
				osal_close_keyboard();
			} while(0);

		break;


#ifdef SUPPORT_FFMPEG_DEMUX
		case 90:            //TRANSCODING TEST
		case 91:

			if (cmd == 91)
			{
				if (DumpConfig("tra_cmd", &multiConfig, sizeof(multiConfig), DUMP_READ))
				{
					memcpy(&encConfig, &multiConfig.encConfig, sizeof(decConfig));
					memcpy(&decConfig, &multiConfig.decConfig, sizeof(decConfig));
					goto RUN_LAST_TRANS_CMD;
				}
			}


			memset( &decConfig, 0x00, sizeof( decConfig) );
			decConfig.cmd = cmd;

			printf("Enter VPU Core Index: ");
			scanf("%d", &decConfig.coreIdx);
			CHECK_CORE_INDEX(decConfig.coreIdx);
			printf("Enter VIDEO ES or MEDIA file name: ");
			scanf("%s", decConfig.bitstreamFileName);

			if(decConfig.bitstreamFileName == NULL)
				break;

			decConfig.bitFormat = 12;	//set to FilePlayTest function in default

			decConfig.maxWidth = 0;
			decConfig.maxHeight = 0;
			
			printf("Enter rotation angle in degrees(0, 90, 180, 270): ");
			scanf("%d", &decConfig.rotAngle);

			if (decConfig.rotAngle != 0 && decConfig.rotAngle != 90 && decConfig.rotAngle != 180 && decConfig.rotAngle != 270) 
			{
				VLOG(ERR, "Invalid rotation angle.\n");
				break;
			}

			printf("Enter mirror direction(0-no mirror, 1-vertical, 2-horizontal, 3-both): ");
			scanf("%d", &decConfig.mirDir);

			if (decConfig.mirDir != 0 && decConfig.mirDir != 1 && decConfig.mirDir != 2 && decConfig.mirDir != 3) 
			{
				printf("Invalid mirror direction.\n");
				break;
			}

			if( decConfig.rotAngle != 0 || decConfig.mirDir != 0 )
				decConfig.useRot = 1;
			if(!Cache_set_flag)
			{
				decConfig.frameCacheBypass   = 0;
				decConfig.frameCacheBurst    = 0;
				decConfig.frameCacheMerge    = 3;
				decConfig.frameCacheWayShape = 15;			
			}

			printf("Enter Frame buffer Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear): ");
			scanf("%d", &decConfig.mapType);		
			printf("Store image?(0: display, 1: store to a file) : ");
			scanf("%d", &storeImage);
			if (storeImage) 
			{
				printf("Enter output(YUV) file name: ");
				scanf("%s", decConfig.yuvFileName );
			}	
			else
			{

				decConfig.lowDelayInfo.lowDelayEn = 0;
				decConfig.lowDelayInfo.numRows      = 0;
			}
			// decoding continue
			decConfig.outNum = 0;

			if (decConfig.bitFormat >= 12)
				printf("Enter Bitstream Mode(0: Interrupt mode, 1: Rollback mode, 2: PicEnd mode): ");
			else
				printf("Enter Bitstream Mode(0: Interrupt mode, 1: Rollback mode): ");

			scanf("%d", &decConfig.bitstreamMode);

            printf("Enter WTL mode(0: Not use, 1: Frame mode): ");
            scanf("%d", &decConfig.wtlEnable);
            if (decConfig.wtlEnable)
                decConfig.wtlEnable = 1;

			decConfig.mp4DeblkEnable = 0;
			decConfig.iframeSearchEnable = 0;

			if( decConfig.outNum < 0 )
			{
				decConfig.checkeos = 0;
				decConfig.outNum = 0;
			}
			else
				decConfig.checkeos = 1;

			osal_init_keyboard();

			//////////////////////////////////////////////////////////////////////

			memset( &encConfig, 0x00, sizeof( encConfig ) );

			printf("Enter ENC CFG file name [0 if manual, 1 if decoder option]: ");
			scanf("%s", encConfig.cfgFileName);

			// Check CFG file validity
			{
				osal_file_t fpTmp;
				fpTmp = osal_fopen(encConfig.cfgFileName, "r");
				if (fpTmp == 0)
				{
					if(encConfig.cfgFileName[0] != '0' && encConfig.cfgFileName[0] != '1')
						encConfig.cfgFileName[0] = 0;
				}
				else
					osal_fclose(fpTmp);
			}

			if (strlen(encConfig.cfgFileName) == 0 || 
				encConfig.cfgFileName[0] == '0' && strlen(encConfig.cfgFileName) == 1 )
			{
				// Manual Config Mode
				printf("Enter picture width: (0: default)");
				scanf("%d", &encConfig.picWidth );

				printf("Enter picture height: (0: default)");
				scanf("%d", &encConfig.picHeight );

				printf("Enter bitrate in kbps: ");
				scanf("%d", &encConfig.kbps );

				strcpy(encConfig.cfgFileName, "");
			}
			else if (encConfig.cfgFileName[0] == '1' && strlen(encConfig.cfgFileName) == 1)
			{
				encConfig.picWidth = decConfig.maxWidth;
				encConfig.picHeight = decConfig.maxHeight;

				strcpy(encConfig.cfgFileName, "");
			}


			printf("Enter bitstream file name: ");
			scanf("%s", encConfig.bitstreamFileName );

			//line buffer mode only
			if ((strstr(encConfig.bitstreamFileName, ".avi") || 
				strstr(encConfig.bitstreamFileName, ".mp4")) && !encConfig.ringBufferEnable)
			{
				printf("Do you want to use %s container?(0:element stream, 1:use %s container) : ", 	strrchr(encConfig.bitstreamFileName, '.'), strrchr(encConfig.bitstreamFileName, '.'));
				scanf("%d", &encConfig.en_container);
				if (encConfig.en_container)
				{
					encConfig.ringBufferEnable = 0;
					printf("Bit stream buffer Mode 0(line buffer) was selected\n");
					goto SKIP_BUFFER_MODE_TRANS;
				}
			}
			printf("Enter Bit stream buffer Mode 0(line buffer) / 1(ring buffer) : ");
			scanf("%d", &encConfig.ringBufferEnable);
SKIP_BUFFER_MODE_TRANS:


			printf("Enter Bit stream Mode 0(MPEG4) / 1(H.263) / 2(H.264) / 3(MJPG) : ");
			scanf("%d", &encConfig.stdMode );



			if (!encConfig.ringBufferEnable)
			{
				printf("Line buffer interrupt mode enable? 0(disable) / 1(enable) : ");
				scanf("%d", &encConfig.lineBufIntEn);
			}
			if(!Cache_set_flag)
			{			
				encConfig.frameCacheBypass   = 0;
				encConfig.frameCacheBurst    = 0;
				encConfig.frameCacheMerge    = 3;
				encConfig.frameCacheWayShape = 15;			
			}
			// Rotation parameter
			encConfig.rotAngle = 0;

			// Flip parameter
			encConfig.mirDir = 0;
			encConfig.coreIdx = decConfig.coreIdx;

RUN_LAST_TRANS_CMD:
			do {
				memset(&reportCfg, 0x00, sizeof(reportCfg));
				reportCfg.userDataEnable = VPU_REPORT_USERDATA;
				reportCfg.userDataReportMode = 0;
				OpenDecReport(decConfig.coreIdx, &reportCfg);
			} while(0);
            
			memcpy(&multiConfig.encConfig, &encConfig, sizeof(encConfig));
			memcpy(&multiConfig.decConfig, &decConfig, sizeof(decConfig));
			DumpConfig("tra_cmd", &multiConfig, sizeof(multiConfig), DUMP_WRITE);

			//////////////////////////////////////////////////////////////////////
			ret = FileTranscodingTest(&decConfig, &encConfig);
			memcpy(&multiConfig.encConfig, &encConfig, sizeof(encConfig));
			memcpy(&multiConfig.decConfig, &decConfig, sizeof(decConfig));
			DumpConfig("tra_cmd", &multiConfig, sizeof(multiConfig), DUMP_WRITE);
			if (!ret)
				VLOG(ERR, "\nFailed to DecodeTest\n");
			CloseDecReport(decConfig.coreIdx);
			osal_close_keyboard();
			break;
#endif

			//------------------------------------------------------------------
			// Maverick Frame Buffer Cache
			//------------------------------------------------------------------
		case 96:
			{
				int  userInput;
				int  frameCacheBypass   ;
				int  frameCacheBurst    ;
				int  frameCacheMerge    ;
				int  frameCacheWayShape ; 		     			

				frameCacheBypass   = 0;
				frameCacheBurst    = 0;
				frameCacheMerge    = 3;
				frameCacheWayShape = 15;

				printf("Cache Bypass?[0:ME/MC CACHE, 1:ME BYPASS/MC CACHE, 2:MC BYPASS/ME CACHE, 3:ME/MC BYPASS]: ");
				scanf("%d", &userInput);
				if (userInput>=4) break;
				frameCacheBypass = userInput; // Enable Frame Buffer Cache

				printf("Cache Burst setting?[0:Burst 4, 1:Burst 8]: ");
				scanf("%d", &userInput);
				if (userInput>=2) break;
				frameCacheBurst = userInput;

				printf("Cache Merge?[0:Hor NoMerge, 1:Hor Merge, 2:Ver NoMerge, 3:Ver Merge]: ");
				scanf("%d", &userInput);
				if (userInput>=4) break;
				frameCacheMerge = userInput;

				printf("Cache WayShape? (0 - 15) {chroma[1:0], luma[1:0]} \n");
				printf("Luma   [0/1 : 64x64, 2/3 : 128x32]\n");
				printf("Chroma Separated   [0/1: 32x32, 2/3: 64x16]\n");
				printf("Chroma Interleaved [0/1: 32x64, 2: 64x32, 3: 128x16]: ");
				scanf("%d", &userInput);
				if (userInput>=16) break;
				frameCacheWayShape = userInput;
				encConfig.frameCacheBypass     = frameCacheBypass;
				encConfig.frameCacheBurst        = frameCacheBurst;
				encConfig.frameCacheMerge       = frameCacheMerge;
				encConfig.frameCacheWayShape = frameCacheWayShape;		
				decConfig.frameCacheBypass     = frameCacheBypass;
				decConfig.frameCacheBurst        = frameCacheBurst;
				decConfig.frameCacheMerge       = frameCacheMerge;
				decConfig.frameCacheWayShape = frameCacheWayShape;
				Cache_set_flag = 1;		   
			}
			break;

		case 97:
			{

				printf("CACHE Bypass %d, Burst %d, Merge %d, Way %d\n", 
					decConfig.frameCacheBypass,
					decConfig.frameCacheBurst,
					decConfig.frameCacheMerge,
					decConfig.frameCacheWayShape);
			}
			break;				
		case 99:
			ret = 1;
			goto ERROR_MAIN;
			break;
		}
	}

ERROR_MAIN:	

	for (i=0; i<coreNum; i++)
		vdi_release(i);

	DeInitLog();
	return ret;
}

#ifdef CNM_FPGA_PLATFORM
int SetDefaultClock(Uint32 core_idx)
{
	int aclk_freq, cclk_freq;

	// Clock Default
	aclk_freq		= 30;
	cclk_freq		= 30;

	printf("Set default ACLK to %d\n", aclk_freq);
	if(vdi_set_clock_freg(core_idx, 0, aclk_freq, 10) == -1)	// ACLK	
		return -1;
	printf("Set default CCLK to %d\n", cclk_freq);
	if(vdi_set_clock_freg(core_idx, 1, cclk_freq, 10) == -1)	// CCLK
		return -1;

	return 1;
}


void DumpSdram(Uint32 core_idx, char *strFile, int start, int byte_size, int endian, int read)
{
	osal_file_t fp = NULL;
	BYTE *pBuf;
	vpu_buffer_t buffer;
	int nread;

	if (strlen(strFile) > 0)
	{
		if (read)
			fp = osal_fopen(strFile, "wb");		
		else
			fp = osal_fopen(strFile, "rb");		

		if (fp == NULL) {
			return;
		}
	}
	

	buffer.phys_addr = start;
	buffer.size      = byte_size;
    if (vdi_attach_dma_memory(core_idx, &buffer) != 0)
	{
		VLOG(ERR, "fail to allocation dma memory addr=0x%x, size=%d\n", (int)buffer.phys_addr, (int)buffer.size);
		return; 
	}

	pBuf = (BYTE *)malloc(byte_size);
	if (!pBuf)
		return;

	if (read)
	{
		vdi_read_memory(core_idx, start, pBuf, byte_size, endian);
		osal_fwrite(pBuf, 1, byte_size, fp);
	}
	else
	{
		if (fp)
		{
			nread = osal_fread(pBuf, 1, byte_size, fp);
			vdi_write_memory(core_idx, start, pBuf, nread, endian);
		}
		else	// clear case
		{
			memset(pBuf, 0x00, byte_size);
			vdi_write_memory(core_idx, start, pBuf, byte_size, endian);
		}
		
	}

    vdi_dettach_dma_memory(core_idx, &buffer);

	free(pBuf);
	if (fp)
		osal_fclose(fp);
}

#endif
int DumpConfig(char *fileName, void *data, int size, int rd)
{
	osal_file_t fp = NULL;

	if (rd)
	{
		fp = osal_fopen(fileName, "rb");		
		if (!fp)
			return 0;

		osal_fread(data, 1, size, fp);
	}
	else
	{
		fp = osal_fopen(fileName, "wb");
		if (!fp)
			return 0;

		osal_fwrite(data, 1, size, fp);
	}
	
	
	osal_fclose(fp);

	return 1;
}

