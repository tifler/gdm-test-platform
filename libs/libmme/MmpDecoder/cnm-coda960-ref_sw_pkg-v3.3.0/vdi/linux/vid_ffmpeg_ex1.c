#include "vid_ffmpeg.h"

struct ffmpeg_obj s_ffmpeg_obj;


void  vdi_ffmpeg_init() {
    
    struct ffmpeg_obj *this = &s_ffmpeg_obj;
    memset(this, 0x00, sizeof(struct ffmpeg_obj));

    avcodec_register_all();


}

void  vdi_ffmpeg_deinit() {

    struct ffmpeg_obj *this = &s_ffmpeg_obj;

    if(this->m_pAVCodecContext != NULL) {
        avcodec_close(this->m_pAVCodecContext);
        av_free(this->m_pAVCodecContext);
        this->m_pAVCodecContext = NULL;
    }
    
    if(this->m_pAVFrame_Decoded != NULL) {
        avcodec_free_frame(&this->m_pAVFrame_Decoded);
        this->m_pAVFrame_Decoded = NULL;
    }

    //if(this->m_extra_data != NULL) {
     //   free(this->m_extra_data);
     //   this->m_extra_data = NULL;
   // }

    
}

static void  vdi_ffmpeg_init_reg() {

    struct ffmpeg_obj *this = &s_ffmpeg_obj;

    if(this->init_reg == 0) {
        
        //Set PC Value
        WRITE_VPU_REG(BIT_CUR_PC, 0x1000);
        WRITE_VPU_REG(BIT_BUSY_FLAG, 0);
        
        
        WRITE_VPU_REG(RET_FW_VER_NUM, 10);
        WRITE_VPU_REG(RET_FW_CODE_REV, 20);

        this->init_reg = 1;
    }
    
}


static MMP_RESULT do_BIT_RUN_COMMAND_SEQ_INIT_Dec(unsigned long base_reg_addr) {

    struct ffmpeg_obj *this = &s_ffmpeg_obj;
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    MMP_U8* pStream;
    MMP_U32 nStreamSize;

    int instIdx;
	int cdcMode;
	int auxMode;
    unsigned int regvalue;
    unsigned int signature;

    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;
        
    unsigned int read_ptr, write_ptr;
	
    write_ptr = READ_VPU_REG(BIT_WR_PTR);
    read_ptr = READ_VPU_REG(BIT_RD_PTR);
    pStream = (unsigned char*)(read_ptr + base_reg_addr); //READ_VPU_REG(BIT_WR_PTR)
    nStreamSize = READ_VPU_REG(CMD_DEC_SEQ_BB_SIZE) *1024; //MaxSize

    this->stream_buffer_statr_phyaddr = READ_VPU_REG(CMD_DEC_SEQ_BB_START);
    this->stream_buffer_size = READ_VPU_REG(CMD_DEC_SEQ_BB_SIZE)*1024;

    pStream = (unsigned char*)(this->stream_buffer_statr_phyaddr + base_reg_addr);
    nStreamSize = this->stream_buffer_size;

    instIdx = READ_VPU_REG(BIT_RUN_INDEX); 
    cdcMode = READ_VPU_REG(BIT_RUN_COD_STD); 
    auxMode = READ_VPU_REG(BIT_RUN_AUX_STD); 
    
    
    switch(cdcMode) {
    
        case AVC_DEC:
            this->m_AVCodecID = AV_CODEC_ID_H264;
            break;

        case MP4_DEC:
            signature = *((unsigned int*)pStream);
            if(signature == MKTAG('C', 'N', 'M', 'V'))  /* Divx3 */
            { 
                this->m_AVCodecID = AV_CODEC_ID_MSMPEG4V3;
            }
            else {
                this->m_AVCodecID = AV_CODEC_ID_MPEG4;
            }
            break;

        case VC1_DEC:
        case RV_DEC:
            cc1 = (AVCodecContext *)pStream;
            this->m_AVCodecID = cc1->codec_id;
            cc1->extradata = &pStream[sizeof(AVCodecContext)];
            break;

        case VPX_DEC:
            cc1 = (AVCodecContext *)&pStream[32];
            this->m_AVCodecID = cc1->codec_id;
            cc1->extradata = &pStream[32+sizeof(AVCodecContext)];
            break;
    }

    codec = avcodec_find_decoder(this->m_AVCodecID);
    if(codec == NULL) {
        return MMP_FAILURE;
    }
    cc= avcodec_alloc_context();

    switch(cdcMode) {
    
        case MP4_DEC:
            signature = *((unsigned int*)pStream);
            if(signature == MKTAG('C', 'N', 'M', 'V'))  /* Divx3 */
            { 
                memcpy(cc, &pStream[36], sizeof(AVCodecContext));
                pStream = NULL;
                nStreamSize = 0;

                cc->extradata = NULL;
                cc->extradata_size = 0;
            }
            else {
                cc->extradata = pStream;//this->m_extra_data;
                cc->extradata_size = nStreamSize;
            }
            break;

        case VC1_DEC:
        case RV_DEC:
        case VPX_DEC:
            memcpy(cc, cc1, sizeof(AVCodecContext));
            nStreamSize = 0;
            break;

        default:
            cc->extradata = pStream;//this->m_extra_data;
            cc->extradata_size = nStreamSize;
    }

    //this->m_extra_data = pStream;//(MMP_U8*)malloc(nStreamSize+1024);
    //memcpy(this->m_extra_data, pStream, nStreamSize);
    
    /* open it */
    if(avcodec_open(cc, codec) < 0) 
    {
        //MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderFfmpeg::DecodeDSI] FAIL: could not open codec\n\r")));
        return MMP_FAILURE;
    }

    this->m_pAVCodec = codec;
    this->m_pAVCodecContext = cc;
    this->m_pAVFrame_Decoded = avcodec_alloc_frame();

    if(nStreamSize > 0) {
        av_init_packet (&avpkt);
        avpkt.data = pStream;;
        avpkt.size = (int)nStreamSize;
        avpkt.pts = 0;
        
        usebyte = avcodec_decode_video2(this->m_pAVCodecContext, this->m_pAVFrame_Decoded, &frameFinished, &avpkt);
        if(usebyte > 0) {
            //pDecResult->uiAuUsedByte = usebyte;
            //WRITE_VPU_REG(BIT_RD_PTR,  read_ptr+usebyte); 
        }
        else {
            //pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
        }
    }

    regvalue = ((this->m_pAVCodecContext->width<<16) | (this->m_pAVCodecContext->height<<0));
    WRITE_VPU_REG(RET_DEC_SEQ_SRC_SIZE,  regvalue); 
   
    WRITE_VPU_REG(RET_DEC_SEQ_FRAME_NEED,  3); //framebuffer count 
    WRITE_VPU_REG(RET_DEC_SEQ_FRAME_DELAY,  0); 
    WRITE_VPU_REG(RET_DEC_SEQ_HEADER_REPORT,  0); 
        
    WRITE_VPU_REG(RET_DEC_SEQ_ASPECT,  0); 
    WRITE_VPU_REG(RET_DEC_SEQ_BIT_RATE,  1024*1024); 


    WRITE_VPU_REG(BIT_RD_PTR,  write_ptr); 

    return MMP_SUCCESS;
}

static MMP_RESULT do_BIT_RUN_COMMAND_PIC_RUN_Dec(unsigned long base_reg_addr) {

    struct ffmpeg_obj *this = &s_ffmpeg_obj;
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    MMP_U8* pStream = NULL;
    MMP_U32 nStreamSize;
    
    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;
    
    unsigned int i,j,k;

    unsigned int read_ptr, write_ptr;
    unsigned int regvalue;

    
    unsigned int framebuffer_addr_y[32];
    unsigned int framebuffer_addr_u[32];
    unsigned int framebuffer_addr_v[32];
    unsigned int framebuffer_addr_swap[32];
    unsigned char* framebuf_addr;
    unsigned char b3, b2, b1, b0;
	
    int cdcMode;
    
    WRITE_VPU_REG(BIT_INT_REASON,  0); 
    WRITE_VPU_REG(RET_DEC_PIC_SUCCESS, 0); 
    WRITE_VPU_REG(RET_DEC_PIC_DECODED_IDX, 0xFFFFFFFF); 
    WRITE_VPU_REG(RET_DEC_PIC_DISPLAY_IDX, 0xFFFFFFFF); 

    // Tell the decoder how much frame buffers were allocated.
    
    // *** Don't get framebuffercount hear because that value was cleared (comment by ht.hwang)
	    //..this->framebuffer_count = READ_VPU_REG(CMD_SET_FRAME_BUF_NUM); //VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_NUM, num);
	    //framebuffer_stride = READ_VPU_REG(CMD_SET_FRAME_BUF_STRIDE); //VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_STRIDE, stride);

	//VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_BIT_ADDR, pDecInfo->secAxiInfo.bufBitUse);
	//VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_IPACDC_ADDR, pDecInfo->secAxiInfo.bufIpAcDcUse);
	//VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_DBKY_ADDR, pDecInfo->secAxiInfo.bufDbkYUse);
	//VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_DBKC_ADDR, pDecInfo->secAxiInfo.bufDbkCUse);
	//VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_OVL_ADDR, pDecInfo->secAxiInfo.bufOvlUse);
	//VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_AXI_BTP_ADDR, pDecInfo->secAxiInfo.bufBtpUse);
	//VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_DELAY, pDecInfo->frameDelay);
	
    framebuf_addr = (unsigned char*)(this->parameter_buffer_phyaddr + this->base_reg_addr);
    j = 0;
    for(i = 0; i < (this->framebuffer_count*3); i++) {
        
           b0 = *framebuf_addr; framebuf_addr++;
           b1 = *framebuf_addr; framebuf_addr++;
           b2 = *framebuf_addr; framebuf_addr++;
           b3 = *framebuf_addr; framebuf_addr++;
           framebuffer_addr_swap[i] = MMPMAKEFOURCC(b0,b1,b2,b3);

           j++;
    }
    if( ((this->framebuffer_count*3)%2) != 0) {
           b0 = *framebuf_addr; framebuf_addr++;
           b1 = *framebuf_addr; framebuf_addr++;
           b2 = *framebuf_addr; framebuf_addr++;
           b3 = *framebuf_addr; framebuf_addr++;
           framebuffer_addr_swap[i] = MMPMAKEFOURCC(b0,b1,b2,b3);
           j++;
    }
    for(i = 0; i < j; i+=2) {
        k = framebuffer_addr_swap[i];
        framebuffer_addr_swap[i] = framebuffer_addr_swap[i+1];
        framebuffer_addr_swap[i+1] = k;
    }

    
    for(i = 0, j=0; i < this->framebuffer_count; i++) {
        
           framebuffer_addr_y[i] = framebuffer_addr_swap[j] + this->base_reg_addr; j++;
           framebuffer_addr_u[i] = framebuffer_addr_swap[j] + this->base_reg_addr; j++;
           framebuffer_addr_v[i] = framebuffer_addr_swap[j] + this->base_reg_addr; j++;
    }
    this->display_buffer_index = (this->display_buffer_index+1)%this->framebuffer_count;
    
    write_ptr = READ_VPU_REG(BIT_WR_PTR);
    read_ptr = READ_VPU_REG(BIT_RD_PTR);
    cdcMode = READ_VPU_REG(BIT_RUN_COD_STD); 

    if(read_ptr < write_ptr) {
        nStreamSize = write_ptr - read_ptr;
        pStream = malloc(nStreamSize);
        memcpy(pStream, (unsigned char*)(read_ptr + base_reg_addr), nStreamSize);
    }
    else {
        
        i = this->stream_buffer_size - (read_ptr - this->stream_buffer_statr_phyaddr);
        j = (write_ptr- this->stream_buffer_statr_phyaddr);
        nStreamSize = i + j; 
        pStream = malloc(nStreamSize);
        memcpy(pStream, (unsigned char*)(read_ptr + base_reg_addr), i);
        memcpy(&pStream[i], (unsigned char*)(this->stream_buffer_statr_phyaddr + base_reg_addr), j );
    }
    
    av_init_packet (&avpkt);

    switch(cdcMode) {

        case VC1_DEC:
            avpkt.data = &pStream[8];
            avpkt.size = (int)nStreamSize-8;
            avpkt.pts = 0;
            break;

        case RV_DEC:
            {
                unsigned int* rvpp;
                int cSlice, offset;
                rvpp = (unsigned int*)pStream;
                cSlice = rvpp[4];
                cSlice = MMP_SWAP_U32(cSlice);
                offset = 20 + cSlice*8;
                avpkt.data = &pStream[offset];
                avpkt.size = (int)nStreamSize-offset;
                avpkt.pts = 0;    
            }
            break;

        case VPX_DEC:
            avpkt.data = &pStream[12];
            avpkt.size = (int)nStreamSize-12;
            avpkt.pts = 0;
            break;

        default:
            avpkt.data = pStream;
            avpkt.size = (int)nStreamSize;
            avpkt.pts = 0;
    }
    
    usebyte = avcodec_decode_video2(this->m_pAVCodecContext, this->m_pAVFrame_Decoded, &frameFinished, &avpkt);
    if(usebyte > 0) {
        //pDecResult->uiAuUsedByte = usebyte;

        
    }
    else {
        //pDecResult->uiAuUsedByte = pMediaSample->uiAuSize;
    }

    if(frameFinished != 0) {

        AVPicture pic;
        AVPicture *pFrameOut = &pic;
        int luma_size, chroma_size;
        memset(pFrameOut, 0x00, sizeof(AVPicture));
        
        luma_size = this->m_pAVCodecContext->width*this->m_pAVCodecContext->height;
        chroma_size = luma_size/4;

        pFrameOut->data[0] = (uint8_t*)framebuffer_addr_y[this->display_buffer_index];
        pFrameOut->data[1] = (uint8_t*)framebuffer_addr_u[this->display_buffer_index];
        pFrameOut->data[2] = (uint8_t*)framebuffer_addr_v[this->display_buffer_index];
        pFrameOut->linesize[0] = this->m_pAVCodecContext->width; //pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y];//em_pAVCodecContext->width;
        pFrameOut->linesize[1] = this->m_pAVCodecContext->width/2; //pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_U];//m_pAVCodecContext->width >> 1;
        pFrameOut->linesize[2] = this->m_pAVCodecContext->width/2; //pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_V];//m_pAVCodecContext->width >> 1;

        if(this->m_pAVFrame_Decoded->format == AV_PIX_FMT_YUV420P) {

            av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)this->m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, this->m_pAVCodecContext->width, this->m_pAVCodecContext->height);
     
            WRITE_VPU_REG(BIT_INT_REASON,  (1<<INT_BIT_PIC_RUN) ); 
            
            regvalue = 1;
            WRITE_VPU_REG(RET_DEC_PIC_SUCCESS, regvalue); 
           	//info->decodingSuccess = val;
            //info->sequenceChanged = ((val>>20) & 0x1);

           	regvalue = this->display_buffer_index;
            WRITE_VPU_REG(RET_DEC_PIC_DECODED_IDX, regvalue); 
           	WRITE_VPU_REG(RET_DEC_PIC_DISPLAY_IDX, regvalue); 
           	//info->indexFrameDecoded	= VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_DECODED_IDX);
        	//info->indexFrameDisplay	= VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_DISPLAY_IDX);

            regvalue = ((this->m_pAVCodecContext->width<<16) | (this->m_pAVCodecContext->height<<0));
            WRITE_VPU_REG(RET_DEC_PIC_SIZE, regvalue); 
            //val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_SIZE); // decoding picture size
        	//info->decPicWidth  = (val>>16) & 0xFFFF;
	        //info->decPicHeight = (val) & 0xFFFF;


            regvalue = 0;
            WRITE_VPU_REG(RET_DEC_PIC_TYPE, regvalue); 
            //val = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_TYPE);
	        //info->interlacedFrame	= (val >> 18) & 0x1;
            //info->topFieldFirst     = (val >> 21) & 0x0001;	// TopFieldFirst[21]
	        //if (info->interlacedFrame) {
		    //    info->picTypeFirst      = (val & 0x38) >> 3;	  // pic_type of 1st field
		    //    info->picType           = val & 7;              // pic_type of 2nd field
	        //}
	        //else {
		    //    info->picTypeFirst   = PIC_TYPE_MAX;	// no meaning
		    //    info->picType = val & 7;
	        //}
            //info->pictureStructure  = (val >> 19) & 0x0003;	// MbAffFlag[17], FieldPicFlag[16]
	        //info->repeatFirstField  = (val >> 22) & 0x0001;
	        //info->progressiveFrame  = (val >> 23) & 0x0003;

            WRITE_VPU_REG(RET_DEC_SEQ_FRATE_NR, 0); 
            WRITE_VPU_REG(RET_DEC_SEQ_FRATE_DR, 0); 
            //info->fRateNumerator    = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_FRATE_NR);
	        //info->fRateDenominator  = VpuReadReg(pCodecInst->coreIdx, RET_DEC_SEQ_FRATE_DR);
	        //if (pCodecInst->codecMode == AVC_DEC && info->fRateDenominator > 0)
		    //    info->fRateDenominator  *= 2;

	        WRITE_VPU_REG(RET_DEC_PIC_MODULO_TIME_BASE, 0); 
            WRITE_VPU_REG(RET_DEC_PIC_VOP_TIME_INCREMENT, 0); 
            //if (pCodecInst->codecMode == MP4_DEC)
	        //{
		    //    info->mp4ModuloTimeBase = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_MODULO_TIME_BASE);
		    //    info->mp4TimeIncrement  = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_VOP_TIME_INCREMENT);
	        //}

            WRITE_VPU_REG(RET_DEC_PIC_ASPECT, 0); 
            WRITE_VPU_REG(RET_DEC_PIC_ERR_MB, 0); 
            //if( pCodecInst->codecMode == VPX_DEC )
        	//	info->aspectRateInfo = 0;
	        //else
		    //    info->aspectRateInfo = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_ASPECT);
	        //info->numOfErrMBs = VpuReadReg(pCodecInst->coreIdx, RET_DEC_PIC_ERR_MB);
	        //info->streamEndFlag = ((pDecInfo->streamEndflag>>2) & 0x01);

            WRITE_VPU_REG(BIT_BYTE_POS_FRAME_START, 0); 
            WRITE_VPU_REG(BIT_BYTE_POS_FRAME_END, 0); 
            //info->bytePosFrameStart = VpuReadReg(pCodecInst->coreIdx, BIT_BYTE_POS_FRAME_START);
	        //info->bytePosFrameEnd   = VpuReadReg(pCodecInst->coreIdx, BIT_BYTE_POS_FRAME_END);
        

        }
            
    }

    WRITE_VPU_REG(BIT_INT_REASON,  (1<<INT_BIT_PIC_RUN) ); 
    //unsigned int stream_buffer_addr; 

    //regvalue = READ_VPU_REG(CMD_DEC_SEQ_BB_START);
    regvalue = READ_VPU_REG(BIT_WR_PTR);

    WRITE_VPU_REG(BIT_RD_PTR,  regvalue); 
    //WRITE_VPU_REG(BIT_WR_PTR,  regvalue); 
    
    if(pStream != NULL) {
        free(pStream);
    }
    return MMP_SUCCESS;
}



static void do_BIT_RUN_COMMAND(unsigned long base_reg_addr, unsigned int data) {

    struct ffmpeg_obj *this = &s_ffmpeg_obj;
    int instIdx, cdcMode, auxMode;

    instIdx = READ_VPU_REG(BIT_RUN_INDEX); 
    cdcMode = READ_VPU_REG(BIT_RUN_COD_STD); 
    auxMode = READ_VPU_REG(BIT_RUN_AUX_STD); 

    if( (cdcMode == AVC_ENC) || (cdcMode == MP4_ENC)) {
    
        switch(data) {
            
            case FIRMWARE_GET: 
                WRITE_VPU_REG(BIT_CUR_PC, 0x1000);
                WRITE_VPU_REG(BIT_BUSY_FLAG, 0);
                    
                WRITE_VPU_REG(RET_FW_VER_NUM, 10);
                WRITE_VPU_REG(RET_FW_CODE_REV, 20);
                break;

            case SET_FRAME_BUF:
                this->framebuffer_count = READ_VPU_REG(CMD_SET_FRAME_BUF_NUM); //VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_NUM, num);
	            this->framebuffer_stride = READ_VPU_REG(CMD_SET_FRAME_BUF_STRIDE); //VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_STRIDE, stride);
	            break;

            case SEQ_INIT: do_BIT_RUN_COMMAND_SEQ_INIT_Enc(base_reg_addr); break;
            case PIC_RUN: do_BIT_RUN_COMMAND_PIC_RUN_Enc(base_reg_addr); break;
        }
    }
    else {
    
        switch(data) {
            
            case FIRMWARE_GET: 
                WRITE_VPU_REG(BIT_CUR_PC, 0x1000);
                WRITE_VPU_REG(BIT_BUSY_FLAG, 0);
                    
                WRITE_VPU_REG(RET_FW_VER_NUM, 10);
                WRITE_VPU_REG(RET_FW_CODE_REV, 20);
                break;

            case SET_FRAME_BUF:
                this->framebuffer_count = READ_VPU_REG(CMD_SET_FRAME_BUF_NUM); //VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_NUM, num);
	            this->framebuffer_stride = READ_VPU_REG(CMD_SET_FRAME_BUF_STRIDE); //VpuWriteReg(pCodecInst->coreIdx, CMD_SET_FRAME_BUF_STRIDE, stride);
	            break;

            case SEQ_INIT: do_BIT_RUN_COMMAND_SEQ_INIT_Dec(base_reg_addr); break;
            case PIC_RUN: do_BIT_RUN_COMMAND_PIC_RUN_Dec(base_reg_addr); break;
        }
    }
}

unsigned int vdi_read_register_ffmpeg(unsigned long base_reg_addr, unsigned int addr) {

    struct ffmpeg_obj *this = &s_ffmpeg_obj;
    unsigned int reg_value;

    this->base_reg_addr = base_reg_addr;

    vdi_ffmpeg_init_reg();
        
    reg_value = READ_VPU_REG(addr);

    return reg_value;
}

int vdi_write_register_ffmpeg(unsigned long base_reg_addr, unsigned int addr, unsigned int data) {
    
    struct ffmpeg_obj *this = &s_ffmpeg_obj;
    
    this->base_reg_addr = base_reg_addr;

    vdi_ffmpeg_init_reg();

    
    WRITE_VPU_REG(addr, data);
    
    switch(addr) {
    
        case BIT_RUN_COMMAND:  do_BIT_RUN_COMMAND(base_reg_addr, data); break;


        case CMD_DEC_SEQ_BB_START:
            //this->p_stream_buffer = (unsigned char*)(base_reg_addr + data);
            break;

        case CMD_DEC_SEQ_BB_SIZE:
            //this->stream_buffer_size = (int)data;
            break;
            
    }

    return 0;
}

int vdi_write_memory_ffmpeg_framebuffer(unsigned long coreIdx, unsigned int addr, unsigned char *data, int len, int endian) {

    struct ffmpeg_obj *this = &s_ffmpeg_obj;
    BYTE frameAddr[MAX_GDI_IDX][3][4];

    this->parameter_buffer_phyaddr = addr;

/*
    for (i=0; i<pDecInfo->numFrameBuffers; i++) {
		frameAddr[i][0][0] = (pDecInfo->frameBufPool[i].bufY  >> 24) & 0xFF;
		frameAddr[i][0][1] = (pDecInfo->frameBufPool[i].bufY  >> 16) & 0xFF;
		frameAddr[i][0][2] = (pDecInfo->frameBufPool[i].bufY  >>  8) & 0xFF;
		frameAddr[i][0][3] = (pDecInfo->frameBufPool[i].bufY  >>  0) & 0xFF;
		if (pDecInfo->openParam.cbcrOrder == CBCR_ORDER_NORMAL) {
			frameAddr[i][1][0] = (pDecInfo->frameBufPool[i].bufCb >> 24) & 0xFF;
			frameAddr[i][1][1] = (pDecInfo->frameBufPool[i].bufCb >> 16) & 0xFF;
			frameAddr[i][1][2] = (pDecInfo->frameBufPool[i].bufCb >>  8) & 0xFF;
			frameAddr[i][1][3] = (pDecInfo->frameBufPool[i].bufCb >>  0) & 0xFF;
			frameAddr[i][2][0] = (pDecInfo->frameBufPool[i].bufCr >> 24) & 0xFF;
			frameAddr[i][2][1] = (pDecInfo->frameBufPool[i].bufCr >> 16) & 0xFF;
			frameAddr[i][2][2] = (pDecInfo->frameBufPool[i].bufCr >>  8) & 0xFF;
			frameAddr[i][2][3] = (pDecInfo->frameBufPool[i].bufCr >>  0) & 0xFF;
		}
		else {
			frameAddr[i][2][0] = (pDecInfo->frameBufPool[i].bufCb >> 24) & 0xFF;
			frameAddr[i][2][1] = (pDecInfo->frameBufPool[i].bufCb >> 16) & 0xFF;
			frameAddr[i][2][2] = (pDecInfo->frameBufPool[i].bufCb >>  8) & 0xFF;
			frameAddr[i][2][3] = (pDecInfo->frameBufPool[i].bufCb >>  0) & 0xFF;
			frameAddr[i][1][0] = (pDecInfo->frameBufPool[i].bufCr >> 24) & 0xFF;
			frameAddr[i][1][1] = (pDecInfo->frameBufPool[i].bufCr >> 16) & 0xFF;
			frameAddr[i][1][2] = (pDecInfo->frameBufPool[i].bufCr >>  8) & 0xFF;
			frameAddr[i][1][3] = (pDecInfo->frameBufPool[i].bufCr >>  0) & 0xFF;
		}		
	}
*/
    return 0;
}
