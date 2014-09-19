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

#ifndef _MMPRENDERER_WAVEOUTEX2_HPP__
#define _MMPRENDERER_WAVEOUTEX2_HPP__

#include "MmpRenderer.hpp"
#include "../MmpComm/MmpOAL.hpp"
#include "../MmpGlobal/TemplateList.hpp"


typedef struct CMmpWaveBuffer_st
{
    WAVEHDR m_WaveHdr;
    unsigned char* m_pBuffer;
    int m_iSize;
    int m_iIndex;
}CMmpWaveBuffer;

#define WAVE_FRAME_TIME      300  //unit: milesecodn
#define WAVE_BUFFERING_TIME  5000 //unit: milesecodn

#define WAVE_HDR_MAX_QUEUE_COUNT 100

class CMmpRenderer_WaveOutEx2 : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    HWAVEOUT m_WaveOutHandle;
    MMPOALCS_HANDLE m_sync_cs;
    HANDLE m_hEvent;

    int m_iWaveBufMaxSize;
    int m_iWaveBufCount;
    CMmpWaveBuffer* m_waveBuffer;

    bool m_bPrepareHeader;
    bool m_bWaitForEvent;

    TCircular_Queue<CMmpWaveBuffer*> m_prepare_queue;
    TCircular_Queue<CMmpWaveBuffer*> m_unprepare_queue;

    unsigned char *m_storage_buffer;
    int m_storage_buffer_size;

protected:
    CMmpRenderer_WaveOutEx2(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_WaveOutEx2();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static void CALLBACK  waveOutProcStub(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 );
    void waveOutProc(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 );

    virtual MMP_RESULT RenderInternal(CMmpMediaSampleDecodeResult* pDecResult);
    virtual MMP_RESULT RenderInternal(unsigned char* buffer, int bufsize);

public:
    virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);
    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe) { return MMP_FAILURE; }
    virtual MMP_RESULT Render(class mmp_buffer_imageframe* p_buf_imageframe) { return MMP_FAILURE; }


};

#endif

