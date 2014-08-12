/*
 *
 *  Copyright (C) 2010-2011 TokiPlayer Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MMPRENDERER_WAVEOUTEX1_HPP__
#define _MMPRENDERER_WAVEOUTEX1_HPP__

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

class CMmpRenderer_WaveOutEx1 : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    HWAVEOUT m_WaveOutHandle;
    MMPOALCS_HANDLE m_sync_cs;
    HANDLE m_hEvent;

    int m_iWaveHdrIndex;
    int m_iWaveBufMaxSize;
    int m_iWaveBufCount;
    CMmpWaveBuffer* m_waveBuffer;

    unsigned int m_WaveHdrCount;
    bool m_bPrepareHeader;
    bool m_bWaitForEvent;

protected:
    CMmpRenderer_WaveOutEx1(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_WaveOutEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static void CALLBACK  waveOutProcStub(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 );
    void waveOutProc(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 );

    virtual MMP_RESULT RenderInternal(CMmpMediaSampleDecodeResult* pDecResult);

public:
    virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);

};

#endif

