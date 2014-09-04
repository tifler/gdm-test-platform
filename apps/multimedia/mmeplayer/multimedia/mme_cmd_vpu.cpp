#include "mme_shell.h"
#include "MmpDecoder.hpp"
#include "vpuapi.h"
#include "vpurun.h"

#define STREAM_BUF_SIZE		 0x300000  // max bitstream size


int mme_command_vpu_load(int argc, char* argv[]) {
    
    system("/mnt/load_mme_vpu.sh");
    return 0;
}

int mme_command_vpu_unload(int argc, char* argv[]) {

    system("/mnt/unload_mme_vpu.sh");
    return 0;
}



int mme_command_vpu_test1(int argc, char* argv[]) {

    int coreIdx = 0;
    RetCode vpu_ret;
    Uint32 version;
    Uint32 revision;
    Uint32 productId;
    DecHandle		handle = NULL;//		= {0};
	DecOpenParam	decOP;//		= {(Cod0};
    vpu_buffer_t	vbStream	= {0};
	int err_cnt = 0;


    vpu_ret = VPU_Init(coreIdx);
    if( (vpu_ret == RETCODE_SUCCESS) || (vpu_ret == RETCODE_CALLED_BEFORE) ) {
        MMESHELL_PRINT(MMESHELL_INFO, ("SUCCESS :  VPU_Init \n"));
#if 0
        VPU_GetVersionInfo(coreIdx, &version, &revision, &productId);	

        MMESHELL_PRINT(MMESHELL_INFO, ("VPU coreNum : [%d]\n", coreIdx));
        MMESHELL_PRINT(MMESHELL_INFO, ("Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n", 
        (Uint32)(version>>16), (Uint32)((version>>(12))&0x0f), (Uint32)((version>>(8))&0x0f), (Uint32)((version)&0xff), revision));
        MMESHELL_PRINT(MMESHELL_INFO, ("Hardware Version => %04x\n", productId));
        MMESHELL_PRINT(MMESHELL_INFO, ("API Version => %04x\n\n", API_VERSION));
#endif

    }
    else {
        MMESHELL_PRINT(MMESHELL_INFO, ("FAIL :  VPU_Init \n"));
        err_cnt++;
    }

    if(err_cnt == 0) {
        VPU_DeInit(coreIdx);
    }

    return 0;
}


int mme_command_vpu_test2(int argc, char* argv[]) {

    int coreIdx = 0;
    RetCode vpu_ret;
    Uint32 version;
    Uint32 revision;
    Uint32 productId;
    DecHandle		handle = NULL;//		= {0};
	DecOpenParam	decOP;//		= {(Cod0};
    vpu_buffer_t	vbStream	= {0};
	int err_cnt = 0;
    int mapType;


    vpu_ret = VPU_Init(coreIdx);
    if( (vpu_ret == RETCODE_SUCCESS) || (vpu_ret == RETCODE_CALLED_BEFORE) ) {
        MMESHELL_PRINT(MMESHELL_INFO, ("SUCCESS :  VPU_Init \n"));

        VPU_GetVersionInfo(coreIdx, &version, &revision, &productId);	

        MMESHELL_PRINT(MMESHELL_INFO, ("VPU coreNum : [%d]\n", coreIdx));
        MMESHELL_PRINT(MMESHELL_INFO, ("Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n", 
        (Uint32)(version>>16), (Uint32)((version>>(12))&0x0f), (Uint32)((version>>(8))&0x0f), (Uint32)((version)&0xff), revision));
        MMESHELL_PRINT(MMESHELL_INFO, ("Hardware Version => %04x\n", productId));
        MMESHELL_PRINT(MMESHELL_INFO, ("API Version => %04x\n\n", API_VERSION));


    }
    else {
        MMESHELL_PRINT(MMESHELL_INFO, ("FAIL :  VPU_Init \n"));
        err_cnt++;
    }

#if 1
    /* Alloc Video Buffer */
    if(err_cnt == 0) {

        memset(&vbStream, 0x00, sizeof(vbStream));
        vbStream.size	 = STREAM_BUF_SIZE;
	    if(vdi_allocate_dma_memory(coreIdx, &vbStream) < 0)
	    {
		    MMESHELL_PRINT(MMESHELL_INFO, ("FAIL :  vdi_allocate_dma_memory \n"));
            err_cnt++;
	    }
    }
#endif

#if 1
    /* VPU Decoder Open */
    if(err_cnt == 0) {

       
        memset(&decOP, 0x00, sizeof(decOP));

        decOP.bitstreamFormat = STD_AVC;
	    decOP.avcExtension = 0;//decConfig.avcExtension&0x1;

        decOP.coreIdx = coreIdx;
	    decOP.bitstreamBuffer = vbStream.phys_addr;
	    decOP.bitstreamBufferSize = vbStream.size;
	    decOP.mp4DeblkEnable = 0;//decConfig.mp4DeblkEnable;
	    decOP.mp4Class = 0;

        mapType = 0; //Linear
	    decOP.tiled2LinearEnable = (mapType>>4)&0x1;; // printf("Enter Frame buffer Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear): ");
           
        decOP.bitstreamMode = 0;
        decOP.cbcrInterleave = CBCR_INTERLEAVE;
	    decOP.bwbEnable	  = VPU_ENABLE_BWB;
	    decOP.frameEndian	= VPU_FRAME_ENDIAN;
    	decOP.streamEndian   = VPU_STREAM_ENDIAN;

        vpu_ret = VPU_DecOpen(&handle, &decOP);
	    if( vpu_ret != RETCODE_SUCCESS )
	    {
		    MMESHELL_PRINT(MMESHELL_INFO, ("FAIL :  VPU_DecOpen ret=0x%x \n", vpu_ret));
            err_cnt ++;
	    }
    }
#endif

/*  
    //VPU_DecGiveCommand(handle, ENABLE_LOGGING, 0);

	ret = WriteBsBufHelper(coreIdx, handle, &bufInfo, &vbStream, STREAM_FILL_SIZE, decConfig.checkeos, &streameos, decOP.streamEndian);
	if (ret != RETCODE_SUCCESS)
	{
		VLOG(ERR, "WriteBsBufHelper failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}
*/

    /* Decoder Close */
#if 1
    if(err_cnt == 0) {
        if(handle!=NULL) VPU_DecClose(handle);
    }

    VPU_DeInit(coreIdx);
#endif

    return 0;
}



int mme_command_vpu_test3(int argc, char* argv[]) {

    int coreIdx = 0;
    RetCode vpu_ret;
    Uint32 version;
    Uint32 revision;
    Uint32 productId;
    DecHandle		handle = NULL;//		= {0};
	DecOpenParam	decOP;//		= {(Cod0};
    vpu_buffer_t	vbStream	= {0};
	int err_cnt = 0;


    vpu_ret = VPU_Init(coreIdx);
    if( (vpu_ret == RETCODE_SUCCESS) || (vpu_ret == RETCODE_CALLED_BEFORE) ) {
        MMESHELL_PRINT(MMESHELL_INFO, ("SUCCESS :  VPU_Init \n"));

        VPU_GetVersionInfo(coreIdx, &version, &revision, &productId);	

        MMESHELL_PRINT(MMESHELL_INFO, ("VPU coreNum : [%d]\n", coreIdx));
        MMESHELL_PRINT(MMESHELL_INFO, ("Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n", 
        (Uint32)(version>>16), (Uint32)((version>>(12))&0x0f), (Uint32)((version>>(8))&0x0f), (Uint32)((version)&0xff), revision));
        MMESHELL_PRINT(MMESHELL_INFO, ("Hardware Version => %04x\n", productId));
        MMESHELL_PRINT(MMESHELL_INFO, ("API Version => %04x\n\n", API_VERSION));


    }
    else {
        MMESHELL_PRINT(MMESHELL_INFO, ("FAIL :  VPU_Init \n"));
        err_cnt++;
    }

#if 1
    /* Alloc Video Buffer */
    if(err_cnt == 0) {

        memset(&vbStream, 0x00, sizeof(vbStream));
        vbStream.size	 = STREAM_BUF_SIZE;
	    if(vdi_allocate_dma_memory(coreIdx, &vbStream) < 0)
	    {
		    MMESHELL_PRINT(MMESHELL_INFO, ("FAIL :  vdi_allocate_dma_memory \n"));
            err_cnt++;
	    }
    }
#endif

#if 1
    /* VPU Decoder Open */
    if(err_cnt == 0) {

       
        memset(&decOP, 0x00, sizeof(decOP));

        decOP.bitstreamFormat =(CodStd) STD_AVC_DEC; //printf("0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8) "); 
	    decOP.avcExtension = 0;// if (decConfig.bitFormat == STD_AVC)  printf("Enter AVC extension 0(No) / 1(MVC) : ");

        decOP.coreIdx = coreIdx;
	    decOP.bitstreamBuffer = vbStream.phys_addr;
	    decOP.bitstreamBufferSize = 1024*32;
	    decOP.mp4DeblkEnable = 0;//decConfig.mp4DeblkEnable;
	    decOP.mp4Class = 0;
	    decOP.tiled2LinearEnable = 0;
        decOP.bitstreamMode = 0;
        decOP.cbcrInterleave = CBCR_INTERLEAVE;
	    decOP.bwbEnable	  = 0;//VPU_ENABLE_BWB;
	    decOP.frameEndian	= VPU_FRAME_ENDIAN;
    	decOP.streamEndian   = VPU_STREAM_ENDIAN;

        vpu_ret = VPU_DecOpen(&handle, &decOP);
	    if( vpu_ret != RETCODE_SUCCESS )
	    {
		    MMESHELL_PRINT(MMESHELL_INFO, ("FAIL :  VPU_DecOpen ret=0x%x \n", vpu_ret));
            err_cnt ++;
	    }
    }
#endif

#if 1
    if(err_cnt == 0) {
        if(handle!=NULL) VPU_DecClose(handle);
    }

    VPU_DeInit(coreIdx);
#endif

    return 0;
}

