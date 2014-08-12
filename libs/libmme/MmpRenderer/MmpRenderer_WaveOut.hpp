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

#ifndef _MMPRENDERER_WAVEOUT_HPP__
#define _MMPRENDERER_WAVEOUT_HPP__

#include "MmpRenderer.hpp"
#include "../MmpComm/MmpOAL.hpp"
#include "../MmpGlobal/TemplateList.hpp"

class CMmpRenderer_WaveOut;
class CMmpAudioBuffer
{
friend class CMmpRenderer_WaveOut;
private:
    unsigned char* m_pBuffer;
    int m_iBufSize;

private:
    CMmpAudioBuffer(unsigned char* pBuffer, int bufSize)
    {
        m_iBufSize=bufSize;
        m_pBuffer=NULL;

        if(m_iBufSize>0)
        {
            m_pBuffer=new unsigned char[bufSize+32];
            if(m_pBuffer)
            {
                memcpy(m_pBuffer, pBuffer, m_iBufSize);

                int offset=((bufSize+8-1)>>3)<<3;
                unsigned int addr=(unsigned int)this;
                memcpy(&m_pBuffer[offset], &addr, 4);
            }
        }

    }

    ~CMmpAudioBuffer()
    {
        if(m_pBuffer)
        {
            delete [] m_pBuffer;
        }
    }

    bool IsInit() { return (m_pBuffer?true:false); }

    static CMmpAudioBuffer* GetInstance(unsigned char* pBuffer, int bufSize) 
    {
        int offset=((bufSize+8-1)>>3)<<3;
        unsigned int addr;

        memcpy(&addr, &pBuffer[offset], 4);
        return (CMmpAudioBuffer*)addr;
    }

};

class CMmpRenderer_WaveOut : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    HWAVEOUT m_WaveOutHandle;
    WAVEHDR m_WaveHdr;
    MMPOALCS_HANDLE m_queue_cs;
    int m_iRefWaveOutWrite;

    TCircular_Queue<CMmpAudioBuffer*> m_sample_queue;

protected:
    CMmpRenderer_WaveOut(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_WaveOut();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static void CALLBACK  waveOutProcStub(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 );
    void waveOutProc(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 );

public:
    virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);

};

#endif

