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


#include "MmpRenderer_WaveOut.hpp"
#include "../MmpComm/MmpUtil.hpp"


/////////////////////////////////////////////////////////////
//CMmpRenderer_WaveOut Member Functions


void CALLBACK CMmpRenderer_WaveOut::waveOutProcStub(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    CMmpRenderer_WaveOut* pObj=(CMmpRenderer_WaveOut*)dwInstance;
    pObj->waveOutProc(WaveOutHandle, uMsg, dwInstance, dwParam1, dwParam2 );
}

CMmpRenderer_WaveOut::CMmpRenderer_WaveOut(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_WaveOutHandle(NULL)
,m_queue_cs(NULL)
,m_iRefWaveOutWrite(0)
,m_sample_queue(15)

{
    memset(&m_WaveHdr, 0x00, sizeof(WAVEHDR));
}

CMmpRenderer_WaveOut::~CMmpRenderer_WaveOut()
{

}

MMP_RESULT CMmpRenderer_WaveOut::Open()
{
    MMP_RESULT mmpResult;
    MMRESULT  mmresult;

    m_queue_cs=CMmpOAL::GetCsInstance()->Create();
    if(!m_queue_cs)
    {
        return MMP_FAILURE;
    }

    mmpResult=CMmpRenderer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    WAVEFORMATEX  waveFormat;	
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = m_pRendererProp->m_wf.nChannels;
    waveFormat.nSamplesPerSec = m_pRendererProp->m_wf.nSamplesPerSec;
    waveFormat.wBitsPerSample = m_pRendererProp->m_wf.wBitsPerSample;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);//1; // waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;//44100; // waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;	

    mmresult = waveOutOpen(NULL, WAVE_MAPPER, &waveFormat, NULL, NULL, WAVE_FORMAT_QUERY);
	if ( mmresult != MMSYSERR_NOERROR ) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_WaveOut::Open] FAIL: waveOutOpen(QUERY) \n\r")));
		m_WaveOutHandle = 0;
		return MMP_FAILURE;
	}

	mmresult = waveOutOpen(&m_WaveOutHandle, WAVE_MAPPER, &waveFormat, (DWORD)(VOID*)CMmpRenderer_WaveOut::waveOutProcStub, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if ( mmresult != MMSYSERR_NOERROR ) 
    {
		m_WaveOutHandle = 0;
		return MMP_FAILURE;
	}

    

#if 0
    //8) build WAVEHDR
    WAVEHDR * pWaveHeader = &m_WaveHdr;
	pWaveHeader->lpData = pWaveDataBlock;  // the DATA we finally got
	pWaveHeader->dwBufferLength = lDatasize;  // data size
	pWaveHeader->dwFlags = 0L;    // start position
	pWaveHeader->dwLoops = 0L;   // loop
	pWaveHeader->dwBytesRecorded = lDatasize; 
#endif

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_WaveOut::Close()
{
    MMP_RESULT mmpResult;

    if ( m_WaveOutHandle ) 
    {
		waveOutClose(m_WaveOutHandle);

        Sleep(1000);
		m_WaveOutHandle=NULL;
	}

    mmpResult=CMmpRenderer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    if(m_queue_cs)
    {
        CMmpOAL::GetCsInstance()->Destroy(m_queue_cs);
        m_queue_cs=NULL;
    }

    return MMP_SUCCESS;
}

void CMmpRenderer_WaveOut::waveOutProc(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    WAVEHDR * pWaveHeader; //= &m_WaveHdr;
    bool bIsValid;
    CMmpAudioBuffer* pMmpAudioBuffer;

    switch (uMsg) 
	{ 
	    case MM_WOM_DONE: 
		{ 			
			// same as OnWaveOutDone(dwParam1, dwParam2);
			//ASSERT( (UINT)wParam == (UINT)m_WaveOutHandle );
			//(LPWAVEHDR)lParam must be one of m_arrWaveHeader[3]

            MMPDEBUGMSG(1, (TEXT("[MM_WOM_DONE] \n\r")));
			MMRESULT  mmresult = waveOutUnprepareHeader((HWAVEOUT)WaveOutHandle, (LPWAVEHDR) dwParam1, sizeof(WAVEHDR) ); 
            pWaveHeader=(WAVEHDR*)dwParam1;
            pMmpAudioBuffer=(CMmpAudioBuffer*)CMmpAudioBuffer::GetInstance((unsigned char*)pWaveHeader->lpData,pWaveHeader->dwBufferLength);
            delete pMmpAudioBuffer;

			//ASSERT( mmresult == MMSYSERR_NOERROR);

            CMmpOAL::GetCsInstance()->Enter(m_queue_cs);
            
            m_iRefWaveOutWrite--;

            bIsValid=(m_sample_queue.GetSize() >  m_sample_queue.GetTotalSize()/2 )?true:false;
            if(bIsValid)
            {
                m_sample_queue.Delete(pMmpAudioBuffer);

                pWaveHeader=&m_WaveHdr;
                pWaveHeader->lpData = (char*)pMmpAudioBuffer->m_pBuffer;//pWaveDataBlock;  // the DATA we finally got
                pWaveHeader->dwBufferLength =pMmpAudioBuffer->m_iBufSize;// lDatasize;  // data size
	            pWaveHeader->dwFlags = 0L;    // start position
	            pWaveHeader->dwLoops = 0L;   // loop
	            pWaveHeader->dwBytesRecorded = pMmpAudioBuffer->m_iBufSize;//lDatasize; 
                pWaveHeader->reserved=(DWORD_PTR)pMmpAudioBuffer;

                MMPDEBUGMSG(1, (TEXT("[MM_WOM_START] \n\r")));

	            waveOutPrepareHeader( m_WaveOutHandle,  pWaveHeader,  sizeof(*pWaveHeader) );	
	            waveOutWrite( m_WaveOutHandle,  pWaveHeader,  sizeof(*pWaveHeader) );
                m_iRefWaveOutWrite++;
            }

            CMmpOAL::GetCsInstance()->Leave(m_queue_cs);


			break;
		}

    	case WIM_DATA: 
		{ 
            MMPDEBUGMSG(1, (TEXT("[WIM_DATA] \n\r")));
			break;
		}

        default:
            MMPDEBUGMSG(1, (TEXT("[MM_UNKNOWN MSG] %d 0x%x\n\r"), uMsg, uMsg));
            break;
	} // end of switch   

}


MMP_RESULT CMmpRenderer_WaveOut::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    bool bQueueFull;
    CMmpAudioBuffer* pMmpAudioBuffer;
    WAVEHDR * pWaveHeader; 

    while(1)
    {
        CMmpOAL::GetCsInstance()->Enter(m_queue_cs);
        bQueueFull=m_sample_queue.IsFull();
        CMmpOAL::GetCsInstance()->Leave(m_queue_cs);

        if(!bQueueFull) break;
        Sleep(1);
    }

    pMmpAudioBuffer=new CMmpAudioBuffer((unsigned char*)pDecResult->uiDecodedBufLogAddr, pDecResult->uiDecodedSize);
    if(pMmpAudioBuffer)
    {
        if( pMmpAudioBuffer->IsInit() )
        {
            CMmpOAL::GetCsInstance()->Enter(m_queue_cs);
            m_sample_queue.Add(pMmpAudioBuffer);
            CMmpOAL::GetCsInstance()->Leave(m_queue_cs);
        }
        else
        {
            delete pMmpAudioBuffer;
        }
    }

    CMmpOAL::GetCsInstance()->Enter(m_queue_cs);
    if(m_iRefWaveOutWrite==0)
    {
        bool bIsValid=(m_sample_queue.GetSize() >  m_sample_queue.GetTotalSize()/2 )?true:false;
        if(bIsValid)
        {
            m_sample_queue.Delete(pMmpAudioBuffer);

            pWaveHeader=&m_WaveHdr;
            pWaveHeader->lpData = (char*)pMmpAudioBuffer->m_pBuffer;//pWaveDataBlock;  // the DATA we finally got
            pWaveHeader->dwBufferLength =pMmpAudioBuffer->m_iBufSize;// lDatasize;  // data size
            pWaveHeader->dwFlags = 0L;    // start position
            pWaveHeader->dwLoops = 0L;   // loop
            pWaveHeader->dwBytesRecorded = pMmpAudioBuffer->m_iBufSize;//lDatasize; 
         
            MMPDEBUGMSG(1, (TEXT("[MM_WOM_START] \n\r")));

            waveOutPrepareHeader( m_WaveOutHandle,  pWaveHeader,  sizeof(*pWaveHeader) );	
            waveOutWrite( m_WaveOutHandle,  pWaveHeader,  sizeof(*pWaveHeader) );
            m_iRefWaveOutWrite++;
        }
    }
    CMmpOAL::GetCsInstance()->Leave(m_queue_cs);


#if 0
    WAVEHDR * pWaveHeader = &m_WaveHdr;

    pWaveHeader->lpData = (char*)pDecResult->m_uiDecodedBufLogAddr;//pWaveDataBlock;  // the DATA we finally got
    pWaveHeader->dwBufferLength = pDecResult->m_iDecodedSize;// lDatasize;  // data size
	pWaveHeader->dwFlags = 0L;    // start position
	pWaveHeader->dwLoops = 0L;   // loop
	pWaveHeader->dwBytesRecorded = pDecResult->m_iDecodedSize;//lDatasize; 

    MMPDEBUGMSG(1, (TEXT("[MM_WOM_START] \n\r")));

	waveOutPrepareHeader( m_WaveOutHandle,  pWaveHeader,  sizeof(*pWaveHeader) );	
	waveOutWrite( m_WaveOutHandle,  pWaveHeader,  sizeof(*pWaveHeader) );
#endif

    return MMP_SUCCESS;
}

