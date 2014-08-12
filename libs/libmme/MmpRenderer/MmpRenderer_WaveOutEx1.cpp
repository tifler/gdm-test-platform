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


#include "MmpRenderer_WaveOutEx1.hpp"
#include "../MmpComm/MmpUtil.hpp"


/////////////////////////////////////////////////////////////
//CMmpRenderer_WaveOutEx1 Member Functions


void CALLBACK CMmpRenderer_WaveOutEx1::waveOutProcStub(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    CMmpRenderer_WaveOutEx1* pObj=(CMmpRenderer_WaveOutEx1*)dwInstance;
    pObj->waveOutProc(WaveOutHandle, uMsg, dwInstance, dwParam1, dwParam2 );
}

CMmpRenderer_WaveOutEx1::CMmpRenderer_WaveOutEx1(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_WaveOutHandle(NULL)
,m_iWaveHdrIndex(0)
,m_sync_cs(NULL)
,m_WaveHdrCount(0)
,m_hEvent(NULL)
,m_bPrepareHeader(false)
,m_bWaitForEvent(false)
,m_waveBuffer(NULL)
{
    m_iWaveBufMaxSize=0;
    
}

CMmpRenderer_WaveOutEx1::~CMmpRenderer_WaveOutEx1()
{

}

MMP_RESULT CMmpRenderer_WaveOutEx1::Open()
{
    MMP_RESULT mmpResult;
    MMRESULT  mmresult;
    int i;

    m_sync_cs=CMmpOAL::GetCsInstance()->Create();
    if(m_sync_cs==NULL)
    {
        return MMP_FAILURE;
    }

    m_hEvent=CreateEvent(NULL, TRUE, FALSE, TEXT(""));
    
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
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_WaveOutEx1::Open] FAIL: waveOutOpen(QUERY) \n\r")));
		m_WaveOutHandle = 0;
		return MMP_FAILURE;
	}

	mmresult = waveOutOpen(&m_WaveOutHandle, WAVE_MAPPER, &waveFormat, (DWORD)(VOID*)CMmpRenderer_WaveOutEx1::waveOutProcStub, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if ( mmresult != MMSYSERR_NOERROR ) 
    {
		m_WaveOutHandle = 0;
		return MMP_FAILURE;
	}

    CMmpWaveBuffer* pWaveBuffer;
    
    //   m_iWaveBufMaxSize : waveFormat.nAvgBytesPerSec = WAVE_FRAME_TIME : 1000
    //m_iWaveBufMaxSize=WAVE_FRAME_TIME*waveFormat.nAvgBytesPerSec/1000;  
    //m_iWaveBufMaxSize=((m_iWaveBufMaxSize+4-1)>>2)<<2;
    m_iWaveBufMaxSize=4096*10;
    m_iWaveBufCount=30;//WAVE_BUFFERING_TIME/WAVE_FRAME_TIME;

    m_waveBuffer=new CMmpWaveBuffer[m_iWaveBufCount];
    if(m_waveBuffer==NULL)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_WaveOutEx1::Open] FAIL: new CMmpWaveBuffer[m_iWaveBufCount] \n\r")));
		return MMP_FAILURE;
	}

    for(i=0;i<m_iWaveBufCount;i++)
    {
        pWaveBuffer=(CMmpWaveBuffer*)&m_waveBuffer[i];
        pWaveBuffer->m_pBuffer=new unsigned char[m_iWaveBufMaxSize];
        pWaveBuffer->m_iIndex=0;
        pWaveBuffer->m_iSize=m_iWaveBufMaxSize;

        pWaveBuffer->m_WaveHdr.lpData =(LPSTR)pWaveBuffer->m_pBuffer;
	    pWaveBuffer->m_WaveHdr.dwBufferLength = m_iWaveBufMaxSize;
	    pWaveBuffer->m_WaveHdr.dwFlags = 0L;    // start position
	    pWaveBuffer->m_WaveHdr.dwLoops = 0L;   // loop
	    pWaveBuffer->m_WaveHdr.dwBytesRecorded = m_iWaveBufMaxSize;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_WaveOutEx1::Close()
{
    MMP_RESULT mmpResult;
    int i;
    CMmpWaveBuffer* pWaveBuffer;

    if(m_WaveOutHandle) 
    {
        while(m_WaveHdrCount>0)
        {
            m_bWaitForEvent=true;
            WaitForSingleObject(m_hEvent, INFINITE);
            ResetEvent(m_hEvent);
        }
        
		waveOutClose(m_WaveOutHandle);
		m_WaveOutHandle=NULL;
        

        for(i=0;i<m_iWaveBufCount;i++)
        {
            pWaveBuffer=(CMmpWaveBuffer*)&m_waveBuffer[i];
            delete [] pWaveBuffer->m_pBuffer;
        }
	}

    mmpResult=CMmpRenderer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    if(m_sync_cs)
    {
        CMmpOAL::GetCsInstance()->Destroy(m_sync_cs);
        m_sync_cs=NULL;
    }

    if(m_hEvent)
    {
        CloseHandle(m_hEvent);
        m_hEvent=NULL;
    }

    if(m_waveBuffer)
    {
        delete [] m_waveBuffer;
        m_waveBuffer=NULL;
    }

    return MMP_SUCCESS;
}

void CMmpRenderer_WaveOutEx1::waveOutProc(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    WAVEHDR* pWaveHdr;

    switch (uMsg) 
	{ 
	    case MM_WOM_DONE: 
		{ 			
			// same as OnWaveOutDone(dwParam1, dwParam2);
			//ASSERT( (UINT)wParam == (UINT)m_WaveOutHandle );
			//(LPWAVEHDR)lParam must be one of m_arrWaveHeader[3]

            m_WaveHdrCount--;
             
            pWaveHdr=(LPWAVEHDR)dwParam1;
            MMPDEBUGMSG(1, (TEXT("[MM_WOM_DONE] WaveHdrCnt(%d) dwUser %d dwBuffer %d dwRec %d WaitEvent(%d) \n\r"), 
                m_WaveHdrCount,
                pWaveHdr->dwUser, pWaveHdr->dwBufferLength, pWaveHdr->dwBytesRecorded, m_bWaitForEvent));

//MMPDEBUGMSG(1, (TEXT("D")));
			 waveOutUnprepareHeader((HWAVEOUT)WaveOutHandle, (LPWAVEHDR) dwParam1, sizeof(WAVEHDR) ); 

             //CMmpOAL::GetCsInstance()->Leave(m_sync_cs);
             if(m_bWaitForEvent)
             {
MMPDEBUGMSG(1, (TEXT("E")));
                SetEvent(m_hEvent);
                m_bWaitForEvent=false;
MMPDEBUGMSG(1, (TEXT("V")));
             }
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

MMP_RESULT CMmpRenderer_WaveOutEx1::RenderInternal(CMmpMediaSampleDecodeResult* pDecResult)
{
    CMmpWaveBuffer* pWaveBuffer;
    MMRESULT mmrResult;

    pWaveBuffer=&m_waveBuffer[m_iWaveHdrIndex];

    memcpy(&pWaveBuffer->m_pBuffer[0], (unsigned char*)pDecResult->uiDecodedBufLogAddr, pDecResult->uiDecodedSize );
    pWaveBuffer->m_iIndex=pDecResult->uiDecodedSize;


//MMPDEBUGMSG(1, (TEXT("1")));
    if(1)//pWaveBuffer->m_iIndex+pDecResult->uiDecodedSize>=m_iWaveBufMaxSize)
    {
//MMPDEBUGMSG(1, (TEXT("0")));
        pWaveBuffer->m_WaveHdr.lpData = (LPSTR)pWaveBuffer->m_pBuffer;
	    pWaveBuffer->m_WaveHdr.dwBufferLength = pWaveBuffer->m_iIndex;//m_iWaveBufMaxSize;
	    pWaveBuffer->m_WaveHdr.dwFlags = 0L;    // start position
	    pWaveBuffer->m_WaveHdr.dwLoops = 0L;   // loop
	    pWaveBuffer->m_WaveHdr.dwBytesRecorded = pWaveBuffer->m_iIndex;//m_iWaveBufMaxSize;

        //pWaveBuffer->m_WaveHdr.dwBufferLength = pWaveBuffer->m_iIndex;
        pWaveBuffer->m_iIndex=0;


       // CMmpOAL::GetCsInstance()->Enter(m_sync_cs);
//MMPDEBUGMSG(1, (TEXT("A")));

        if( m_WaveHdrCount>m_iWaveBufCount/2)
        {
MMPDEBUGMSG(1, (TEXT("W")));
            m_bWaitForEvent=true;
            WaitForSingleObject(m_hEvent, INFINITE);
            ResetEvent(m_hEvent);
MMPDEBUGMSG(1, (TEXT("A")));
        }
        

        pWaveBuffer->m_WaveHdr.dwUser=m_iWaveHdrIndex;
        mmrResult=waveOutPrepareHeader(m_WaveOutHandle, &pWaveBuffer->m_WaveHdr, sizeof(WAVEHDR));	
        if(mmrResult==MMSYSERR_NOERROR)
        {
            //m_bPrepareHeader=true;
//MMPDEBUGMSG(1, (TEXT("B")));
            //pWaveBuffer->m_WaveHdr.dwBufferLength = pWaveBuffer->m_iIndex;
            //pWaveBuffer->m_iIndex=0;

           // MMPDEBUGMSG(1, (TEXT("[MM_WOM_START] Write %d HdrIdx %d \n\r"), m_WaveHdrCount, m_iWaveHdrIndex));

            MMPDEBUGMSG(1, (TEXT("[MM_WOM_START] waveHdrCnt(%d) dwUser %d dwBuffer %d dwRec %d \n\r"), 
                m_WaveHdrCount,
                pWaveBuffer->m_WaveHdr.dwUser, pWaveBuffer->m_WaveHdr.dwBufferLength, pWaveBuffer->m_WaveHdr.dwBytesRecorded));

MMPDEBUGMSG(1, (TEXT("0")));
            mmrResult=waveOutWrite(m_WaveOutHandle,  &pWaveBuffer->m_WaveHdr, sizeof(WAVEHDR) );         
MMPDEBUGMSG(1, (TEXT("1(%d) "), mmrResult));
            m_WaveHdrCount++;
        }

        
        m_iWaveHdrIndex=(m_iWaveHdrIndex+1)%m_iWaveBufCount;
        pWaveBuffer=&m_waveBuffer[m_iWaveHdrIndex];
    }

//MMPDEBUGMSG(1, (TEXT("2")));
  //  memcpy(&pWaveBuffer->m_pBuffer[pWaveBuffer->m_iIndex], (unsigned char*)pDecResult->m_uiDecodedBufLogAddr, pDecResult->m_iDecodedSize );
//    pWaveBuffer->m_iIndex+=pDecResult->m_iDecodedSize;

//MMPDEBUGMSG(1, (TEXT("3")));
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_WaveOutEx1::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    CMmpWaveBuffer* pWaveBuffer;
    int bufIndex,decodedSize;
    unsigned int decodedBufAddr;
    MMP_RESULT mmpResult;

    if(pDecResult->uiDecodedSize>=m_iWaveBufMaxSize)
    {
        decodedSize=pDecResult->uiDecodedSize;
        decodedBufAddr=pDecResult->uiDecodedBufLogAddr;
        bufIndex=0;
        while(1) 
        {
            pDecResult->uiDecodedBufLogAddr=decodedBufAddr;
            pDecResult->uiDecodedSize=m_iWaveBufMaxSize;
            mmpResult=this->RenderInternal(pDecResult);
            if(mmpResult!=MMP_SUCCESS)
            {
                break;
            }

            bufIndex+=m_iWaveBufMaxSize;
            decodedBufAddr+=m_iWaveBufMaxSize;

            if(bufIndex+m_iWaveBufMaxSize>=decodedSize)
            {
                pDecResult->uiDecodedBufLogAddr=decodedBufAddr;
                pDecResult->uiDecodedSize=decodedSize-bufIndex;
                mmpResult=this->RenderInternal(pDecResult);
                break;
            }
        }
        
    }
    else
    {
        mmpResult=this->RenderInternal(pDecResult);
    }

    return mmpResult;
}