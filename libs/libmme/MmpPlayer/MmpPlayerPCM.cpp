/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "MmpPlayerPCM.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerPCM Member Functions

CMmpPlayerPCM::CMmpPlayerPCM(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)

,m_fp_pcm(NULL)
,m_pRendererAudio(NULL)
,m_pcm_buffer(NULL)
,m_pcm_buffer_max_size(1024*1024)
{
    

}

CMmpPlayerPCM::~CMmpPlayerPCM()
{
    
    
}

MMP_RESULT CMmpPlayerPCM::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S64 file_pos;

    //CMmpRendererCreateProp RendererProp;
    //CMmpRendererCreateProp* pRendererProp=&RendererProp; 

    /* create pcm file */
    if(mmpResult == MMP_SUCCESS ) {
        m_fp_pcm = fopen(this->m_create_config.filename, "rb");
        if(m_fp_pcm == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fseek(m_fp_pcm, 0, SEEK_END);
            file_pos = (MMP_S64)ftell(m_fp_pcm);
            fseek(m_fp_pcm, 0, SEEK_SET);

            m_play_duration = (file_pos*1000000L) / (MMP_S64)this->m_create_config.audio_wf.nAvgBytesPerSec;
        }
    }
    
    /* create pcm buffer */
    if(mmpResult == MMP_SUCCESS ) {
        
        m_pcm_buffer = (MMP_U8*)malloc(m_pcm_buffer_max_size);
        if(m_pcm_buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }


    /* create audio render */
    if(mmpResult == MMP_SUCCESS ) {
        m_pRendererAudio = CMmpRenderer::CreateAudioObject(&this->m_create_config.audio_wf);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }


    return mmpResult;
}


MMP_RESULT CMmpPlayerPCM::Close()
{
    CMmpPlayer::Close();

    if(m_pRendererAudio != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererAudio);  
        m_pRendererAudio = NULL;

        CMmpUtil::Sleep(1000);
    }
    
    if(m_pcm_buffer!=NULL) 
    { 
        free(m_pcm_buffer); 
        m_pcm_buffer = NULL; 
    }
    
    if(m_fp_pcm!=NULL) { 
        fclose(m_fp_pcm); 
        m_fp_pcm=NULL; 
    }
     
    return MMP_SUCCESS;

}

void CMmpPlayerPCM::Service()
{
    MMP_S64 start_tick, before_tick, cur_tick;
    
    CMmpRenderer* pRendererAudio = m_pRendererAudio;
        
    float fv;
    float* p_pcm_float;
    MMP_S32 i, j;
    MMP_U16 *p_pcm_16bit, u16, u16_1;
    MMP_S32 pcm_size;
    MMP_S64 pcm_dur;

    MMP_S64 packet_pts=0, last_render_pts=0;
    MMP_S64 timestamp_weight = 0;

    
    start_tick = CMmpUtil::GetTickCountUS();
    before_tick = start_tick;
    while(m_bServiceRun == MMP_TRUE) {
    
        //pcm_size = fread(m_pcm_buffer, 1, 7680, m_fp_pcm);
        pcm_size = fread(m_pcm_buffer, 1, 1920, m_fp_pcm);
        if(pcm_size > 0) {
            //pcm_size = 1920;//1440*4;
            //pcm_size = 1920;//1440*4;
            memset(&m_pcm_buffer[pcm_size], 0xFF, 1024*400);

            p_pcm_16bit = (MMP_U16*)m_pcm_buffer;
            p_pcm_float = (float*)m_pcm_buffer;

#if 0
            for(i = 0; i < pcm_size/4; i++) {

                fv = p_pcm_float[i];
                fv*=32768.0f;
                
                //p_pcm_float[i] = fv;
            }
#endif


#if 1
            for(i = 0; i < pcm_size/2; i++) {
                u16 = p_pcm_16bit[i];
                
                
                
                //u16 += (65536/2);
#if 0
                //for(j = 0, u16_1=0; j < 16; j++) {
                //    u16_1 |= ((u16>>j)&0x01)<<(15-j);
                //}
                u16_1 = u16;
                //u16_1<<=8;//&=0xFFF0;
#else
                u16_1 = MMP_SWAP_U16(u16);
                //u16_1<<=12;//&=0xFFF0;

                //u16_1 = u16;
                //memcpy(&fv, 
#endif
                

                //if(u16_1 == 0x0002) {
                //    u16_1 = 0x0000;
               // }
                //u16_1+=0x7FFF;
                //u16_1>>=1;

                p_pcm_16bit[i] = u16_1;
            }
#endif

            pRendererAudio->RenderPCM(m_pcm_buffer, pcm_size);

            pcm_dur = ((MMP_S64)pcm_size*1000000L) / (MMP_S64)this->m_create_config.audio_wf.nAvgBytesPerSec;
            packet_pts += pcm_dur;
            CMmpUtil::Sleep(pcm_dur/1000);
        }
        
        cur_tick = CMmpUtil::GetTickCountUS();
        if( (cur_tick - before_tick) > (1000LL*1000LL) ) {
        
            if(this->m_create_config.callback != NULL) {

                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                
                msg = MMP_PLAYER_CALLBACK_PLAYTIME;
                playtime_st.media_duration  = this->m_play_duration;
                playtime_st.media_pts = packet_pts;

                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }

            before_tick = cur_tick;
        }

        //CMmpUtil::Sleep(30);
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */
 
}

