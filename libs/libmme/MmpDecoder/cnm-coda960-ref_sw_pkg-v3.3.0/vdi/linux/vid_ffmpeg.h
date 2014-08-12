#ifndef WIN32_VID_FFMPEG_H__
#define WIN32_VID_FFMPEG_H__

#include "MmpDefine.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libavresample/audio_convert.h"

#include "regdefine.h"
#include "vpuapifunc.h"


struct ffmpeg_obj {
    
    int init_reg;
    unsigned int base_reg_addr;
    unsigned int parameter_buffer_phyaddr;
    
    enum AVCodecID m_AVCodecID;
    AVCodec *m_pAVCodec;
    AVCodecContext *m_pAVCodecContext;
    AVFrame *m_pAVFrame_Decoded;
    //MMP_U8* m_extra_data;

    AVFrame *m_pAVFrame_Input; //Encoded Input FrameBuffer

    //unsigned char* decoded_buffer;
    int display_buffer_index;
    unsigned int framebuffer_count;
	unsigned int framebuffer_stride;

    unsigned int stream_buffer_statr_phyaddr;
    unsigned int stream_buffer_size;

};

extern struct ffmpeg_obj s_ffmpeg_obj;

#define WRITE_VPU_REG(ADDR, VALUE) *((unsigned long *)(ADDR + this->base_reg_addr)) = VALUE
#define READ_VPU_REG(ADDR) (*((unsigned long *)(ADDR + this->base_reg_addr))) 

MMP_RESULT do_BIT_RUN_COMMAND_SEQ_INIT_Enc(unsigned long base_reg_addr);
MMP_RESULT do_BIT_RUN_COMMAND_PIC_RUN_Enc(unsigned long base_reg_addr);

#endif
