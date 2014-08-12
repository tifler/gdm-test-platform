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

#include "MmpDemuxerBuffer.hpp"

CMmpDemuxerBuffer::CMmpDemuxerBuffer() {

    MMP_S32 i;

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {

        this->m_p_queue_media[i] = new TCircular_Queue<struct queue_packet>(30*10);
    }

}

CMmpDemuxerBuffer::~CMmpDemuxerBuffer() {

    MMP_S32 i;
    struct queue_packet pack;

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {

        if(this->m_p_queue_media[i]!=NULL) {

            while(!this->m_p_queue_media[i]->IsEmpty()) {
                
                this->m_p_queue_media[i]->Delete(pack);
                if(pack.data!=NULL) {
                    free(pack.data);
                }
            }

            delete this->m_p_queue_media[i];
        }

        this->m_p_queue_media[i] = NULL;
    }
}

MMP_S32 CMmpDemuxerBuffer::queue_get_empty_streamindex(void) {

    MMP_S32 index = -1;
    MMP_S32 i;

    /* check if there are empty queue */
    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        
        if(this->m_p_queue_media[i] != NULL) {
        
            if(this->m_p_queue_media[i]->IsEmpty()) {
                index = i;
                break;
            }
        }
    }

    return index;
}

void CMmpDemuxerBuffer::queue_add(struct queue_packet* p_pack) {

    struct queue_packet pack;

    if( (p_pack->mediatype >= 0)
        && (p_pack->mediatype < MMP_MEDIATYPE_MAX) ){
    
        memcpy(&pack, p_pack, sizeof(struct queue_packet));

        pack.data = (MMP_U8*)malloc(pack.size);
        if(pack.data != NULL) {
            memcpy(pack.data, p_pack->data, pack.size);
            this->m_p_queue_media[pack.mediatype]->Add(pack);
        }
    
    }

}

MMP_RESULT CMmpDemuxerBuffer::queue_get(CMmpMediaSample* pMediaSample) {

    int i, media_index;
    struct queue_packet qpack;
    bool bret = false;
    MMP_S64 ts_min;
    MMP_RESULT mmpResult = MMP_FAILURE;

    static unsigned int s_nops_value_flag = 0;

    media_index = -1;
    ts_min = LLONG_MAX;
    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        
        if(this->m_p_queue_media[i] != NULL) {
        
            if(!this->m_p_queue_media[i]->IsEmpty()) {
            
                this->m_p_queue_media[i]->GetFirstItem(qpack);
                if(qpack.pts < ts_min) {
                    ts_min = qpack.pts;
                    media_index = i;
                }
            }
        }
    }

    if(media_index >= 0) {
        
        this->m_p_queue_media[media_index]->Delete(qpack);

        if(qpack.size <= pMediaSample->uiAuMaxSize) {
        
            pMediaSample->uiMediaType = qpack.mediatype;
            memcpy(pMediaSample->pAu, qpack.data, qpack.size);
            pMediaSample->uiAuSize = qpack.size;
            pMediaSample->uiTimeStamp = qpack.pts;
            pMediaSample->uiFlag = qpack.flags;
            mmpResult = MMP_SUCCESS;
        }
        
        if(qpack.data!=NULL) {
            free(qpack.data);
        }
    }
    
    return mmpResult;
}

MMP_RESULT CMmpDemuxerBuffer::queue_clear() {

     MMP_S32 i;
    struct queue_packet pack;

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {

        if(this->m_p_queue_media[i]!=NULL) {

            while(!this->m_p_queue_media[i]->IsEmpty()) {
                
                this->m_p_queue_media[i]->Delete(pack);
                if(pack.data!=NULL) {
                    free(pack.data);
                }
            }
        }
    }

    return MMP_SUCCESS;
}