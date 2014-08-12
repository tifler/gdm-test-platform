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


#include "MmpRenderer_WaveOutEx2.hpp"
#include "../MmpComm/MmpUtil.hpp"


/////////////////////////////////////////////////////////////
//CMmpRenderer_WaveOutEx2 Member Functions


void CALLBACK CMmpRenderer_WaveOutEx2::waveOutProcStub(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    CMmpRenderer_WaveOutEx2* pObj=(CMmpRenderer_WaveOutEx2*)dwInstance;
    pObj->waveOutProc(WaveOutHandle, uMsg, dwInstance, dwParam1, dwParam2 );
}

CMmpRenderer_WaveOutEx2::CMmpRenderer_WaveOutEx2(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(pRendererProp)
,m_WaveOutHandle(NULL)
,m_sync_cs(NULL)
,m_hEvent(NULL)
,m_bPrepareHeader(false)
,m_bWaitForEvent(false)
,m_waveBuffer(NULL)
,m_prepare_queue(WAVE_HDR_MAX_QUEUE_COUNT+2)
,m_unprepare_queue(WAVE_HDR_MAX_QUEUE_COUNT+2)

,m_storage_buffer(NULL)
,m_storage_buffer_size(0)
{
    m_iWaveBufMaxSize=0;
    
}

CMmpRenderer_WaveOutEx2::~CMmpRenderer_WaveOutEx2()
{

}

MMP_RESULT CMmpRenderer_WaveOutEx2::Open()
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
    if(waveFormat.nChannels==0 || waveFormat.nChannels>2 )
        waveFormat.nChannels=2;
    waveFormat.nSamplesPerSec = m_pRendererProp->m_wf.nSamplesPerSec;
    waveFormat.wBitsPerSample = m_pRendererProp->m_wf.wBitsPerSample;
    if(waveFormat.wBitsPerSample==0)
        waveFormat.wBitsPerSample=16;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);//1; // waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;//44100; // waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;	

    mmresult = waveOutOpen(NULL, WAVE_MAPPER, &waveFormat, NULL, NULL, WAVE_FORMAT_QUERY);
	if ( mmresult != MMSYSERR_NOERROR ) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_WaveOutEx2::Open] FAIL: waveOutOpen(QUERY) \n\r")));
		m_WaveOutHandle = 0;
		return MMP_FAILURE;
	}

	mmresult = waveOutOpen(&m_WaveOutHandle, WAVE_MAPPER, &waveFormat, (DWORD)(VOID*)CMmpRenderer_WaveOutEx2::waveOutProcStub, (DWORD_PTR)this, CALLBACK_FUNCTION);
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
    m_iWaveBufCount=10;//WAVE_BUFFERING_TIME/WAVE_FRAME_TIME;

    m_storage_buffer = new unsigned char[m_iWaveBufMaxSize*10];

    m_waveBuffer=new CMmpWaveBuffer[m_iWaveBufCount];
    if(m_waveBuffer==NULL)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_WaveOutEx2::Open] FAIL: new CMmpWaveBuffer[m_iWaveBufCount] \n\r")));
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
        pWaveBuffer->m_WaveHdr.dwUser=(DWORD)pWaveBuffer;

        m_prepare_queue.Add(pWaveBuffer);
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_WaveOutEx2::Close()
{
    MMP_RESULT mmpResult;
    int i;
    CMmpWaveBuffer* pWaveBuffer;
    WAVEHDR* pWaveHdr;
    unsigned int t1, t2;

    if(m_WaveOutHandle) 
    {
        t1=CMmpOAL_GetTickCount();
        while(m_prepare_queue.GetSize()!=m_iWaveBufCount)
        {
            //Unprepare WaveHeader and Add Prepare Queue
            CMmpOAL::GetCsInstance()->Enter(m_sync_cs);
            while(!m_unprepare_queue.IsEmpty())
            {
                m_unprepare_queue.Delete(pWaveBuffer);
                pWaveHdr=&pWaveBuffer->m_WaveHdr;
                waveOutUnprepareHeader(m_WaveOutHandle, (LPWAVEHDR)pWaveHdr, sizeof(WAVEHDR) ); 

                m_prepare_queue.Add(pWaveBuffer);
            }
            CMmpOAL::GetCsInstance()->Leave(m_sync_cs);

            t2=CMmpOAL_GetTickCount();
            if(t2-t1>3000)
            {
                MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_WaveOutEx2::Close] Wait Callback TimeOut!! \n\r")));
                break;
            }

            Sleep(100);
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

    if(m_storage_buffer)
    {
        delete [] m_storage_buffer;
        m_storage_buffer = NULL;
    }

    return MMP_SUCCESS;
}

void CMmpRenderer_WaveOutEx2::waveOutProc(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    WAVEHDR* pWaveHdr;

    switch (uMsg) 
	{ 
	    case MM_WOM_DONE: 
		{ 			
			// same as OnWaveOutDone(dwParam1, dwParam2);
			//ASSERT( (UINT)wParam == (UINT)m_WaveOutHandle );
			//(LPWAVEHDR)lParam must be one of m_arrWaveHeader[3]

            CMmpOAL::GetCsInstance()->Enter(m_sync_cs);
            pWaveHdr=(WAVEHDR*)dwParam1;
            m_unprepare_queue.Add((CMmpWaveBuffer*)pWaveHdr->dwUser);
            CMmpOAL::GetCsInstance()->Leave(m_sync_cs);
            if(m_bWaitForEvent)
            {
                SetEvent(m_hEvent);
                m_bWaitForEvent=false;
            }
            MMPDEBUGMSG(0, (TEXT("[WIM_DONE] UnpreQ:%d preQ:%d \n\r"), m_unprepare_queue.GetSize(), m_prepare_queue.GetSize() ));
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

MMP_RESULT CMmpRenderer_WaveOutEx2::RenderInternal(CMmpMediaSampleDecodeResult* pDecResult)
{
    WAVEHDR* pWaveHdr;
    CMmpWaveBuffer* pWaveBuffer;
    MMRESULT mmrResult;
    MMP_RESULT mmpResult=MMP_FAILURE;
    
    //Check if WaveDevcie is busy
    if(m_prepare_queue.GetSize()<m_iWaveBufCount/2)
    {
         m_bWaitForEvent=true;
         WaitForSingleObject(m_hEvent, INFINITE);
         ResetEvent(m_hEvent);
         
    }

    if(m_prepare_queue.GetSize()<m_iWaveBufCount/2) {
        
        return MMP_FAILURE;
    }
    
    //Unprepare WaveHeader and Add Prepare Queue
    CMmpOAL::GetCsInstance()->Enter(m_sync_cs);
    while(!m_unprepare_queue.IsEmpty())
    {
        m_unprepare_queue.Delete(pWaveBuffer);
        pWaveHdr=&pWaveBuffer->m_WaveHdr;
        waveOutUnprepareHeader(m_WaveOutHandle, (LPWAVEHDR)pWaveHdr, sizeof(WAVEHDR) ); 

        m_prepare_queue.Add(pWaveBuffer);
    }
    CMmpOAL::GetCsInstance()->Leave(m_sync_cs);

    //Get WaveHeader
    if(m_prepare_queue.IsEmpty())
    {
        return mmpResult;
    }
    m_prepare_queue.Delete(pWaveBuffer);

    memcpy(pWaveBuffer->m_pBuffer, (unsigned char*)pDecResult->uiDecodedBufferLogAddr[0], pDecResult->uiDecodedSize );
    pWaveBuffer->m_iSize=pDecResult->uiDecodedSize;
    
    pWaveBuffer->m_WaveHdr.lpData = (LPSTR)pWaveBuffer->m_pBuffer;
    pWaveBuffer->m_WaveHdr.dwBufferLength=pWaveBuffer->m_iSize;
    pWaveBuffer->m_WaveHdr.dwFlags = 0;
    pWaveBuffer->m_WaveHdr.dwLoops = 0L;
    pWaveBuffer->m_WaveHdr.dwBytesRecorded=pWaveBuffer->m_iSize;
   
    mmrResult=waveOutPrepareHeader(m_WaveOutHandle, &pWaveBuffer->m_WaveHdr, sizeof(WAVEHDR));	
    if(mmrResult!=MMSYSERR_NOERROR)
    {
        m_prepare_queue.Add(pWaveBuffer); //return object to prepare queue
        return mmpResult;
    }
    
    mmrResult=waveOutWrite(m_WaveOutHandle,  &pWaveBuffer->m_WaveHdr, sizeof(WAVEHDR) );         
    if(mmrResult==MMSYSERR_NOERROR)
    {
        mmpResult=MMP_SUCCESS;
    }
    else
    {
        m_prepare_queue.Add(pWaveBuffer); //return object to prepare queue
    }

    return mmpResult;
}


MMP_RESULT CMmpRenderer_WaveOutEx2::RenderInternal(unsigned char* buffer, int bufsize)
{
    WAVEHDR* pWaveHdr;
    CMmpWaveBuffer* pWaveBuffer;
    MMRESULT mmrResult;
    MMP_RESULT mmpResult=MMP_FAILURE;
    
#if 1
    //Check if WaveDevcie is busy
    if(m_prepare_queue.GetSize()<m_iWaveBufCount/2)
    {
#if 1
         m_bWaitForEvent=true;
         WaitForSingleObject(m_hEvent, INFINITE);
         ResetEvent(m_hEvent);
#else   
         Sleep(10);
         return MMP_FAILURE;
#endif
    }
#endif
    
    
    //Unprepare WaveHeader and Add Prepare Queue
    CMmpOAL::GetCsInstance()->Enter(m_sync_cs);
    while(!m_unprepare_queue.IsEmpty())
    {
        m_unprepare_queue.Delete(pWaveBuffer);
        pWaveHdr=&pWaveBuffer->m_WaveHdr;
        waveOutUnprepareHeader(m_WaveOutHandle, (LPWAVEHDR)pWaveHdr, sizeof(WAVEHDR) ); 

        m_prepare_queue.Add(pWaveBuffer);
    }
    CMmpOAL::GetCsInstance()->Leave(m_sync_cs);

    //Get WaveHeader
    if(m_prepare_queue.IsEmpty())
    {
        return mmpResult;
    }
    m_prepare_queue.Delete(pWaveBuffer);

    memcpy(pWaveBuffer->m_pBuffer, (unsigned char*)buffer, bufsize);
    pWaveBuffer->m_iSize=bufsize;
    
    pWaveBuffer->m_WaveHdr.lpData = (LPSTR)pWaveBuffer->m_pBuffer;
    pWaveBuffer->m_WaveHdr.dwBufferLength=pWaveBuffer->m_iSize;
    pWaveBuffer->m_WaveHdr.dwFlags = 0;
    pWaveBuffer->m_WaveHdr.dwLoops = 0L;
    pWaveBuffer->m_WaveHdr.dwBytesRecorded=pWaveBuffer->m_iSize;
   
    mmrResult=waveOutPrepareHeader(m_WaveOutHandle, &pWaveBuffer->m_WaveHdr, sizeof(WAVEHDR));	
    if(mmrResult!=MMSYSERR_NOERROR)
    {
        m_prepare_queue.Add(pWaveBuffer); //return object to prepare queue
        return mmpResult;
    }
    
    mmrResult=waveOutWrite(m_WaveOutHandle,  &pWaveBuffer->m_WaveHdr, sizeof(WAVEHDR) );         
    if(mmrResult==MMSYSERR_NOERROR)
    {
        mmpResult=MMP_SUCCESS;
    }
    else
    {
        m_prepare_queue.Add(pWaveBuffer); //return object to prepare queue
    }

    return mmpResult;
}

#if 0

MMP_RESULT CMmpRenderer_WaveOutEx2::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    int bufIndex,decodedSize;
    unsigned int decodedBufAddr;
    MMP_RESULT mmpResult;

    if(pDecResult->uiDecodedSize>=m_iWaveBufMaxSize)
    {
        decodedSize=pDecResult->uiDecodedSize;
        decodedBufAddr=pDecResult->uiDecodedBufferLogAddr[0];
        bufIndex=0;
        while(1) 
        {
            pDecResult->uiDecodedBufferLogAddr[0]=decodedBufAddr;
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
                pDecResult->uiDecodedBufferLogAddr[0]=decodedBufAddr;
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

#else


MMP_RESULT CMmpRenderer_WaveOutEx2::Render(CMmpMediaSampleDecodeResult* pDecResult)
{
    int bufIndex,decodedSize;
    unsigned int decodedBufAddr;
    MMP_RESULT mmpResult = MMP_SUCCESS;

#if 0
    static unsigned int  before_tick = 0;
    static int count = 0;
    unsigned int cur_tick;

    cur_tick = GetTickCount();
    
    MMPDEBUGMSG(0, (TEXT("^^^^^^^^^^^^^  Wave Gap : %d  cnt=%d playtime=%d.%d s UnpreQ:%d preQ:%d \n\r"), 
                 
                 cur_tick - before_tick, 
                 count, 
                 (count*48)/1000, (count*48)%1000 ,
                 m_unprepare_queue.GetSize(), m_prepare_queue.GetSize() 
                 ));
    
    before_tick = cur_tick;
    count++;
#endif

    memcpy(&m_storage_buffer[m_storage_buffer_size], (void*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM], pDecResult->uiDecodedSize);
    m_storage_buffer_size+=pDecResult->uiDecodedSize;
    
    while(1) {

        if(m_storage_buffer_size >= m_iWaveBufMaxSize)
        {
            
            mmpResult=this->RenderInternal(m_storage_buffer, m_iWaveBufMaxSize);

            m_storage_buffer_size-=m_iWaveBufMaxSize;
            if(m_storage_buffer_size > 0)
            {
                memcpy(&m_storage_buffer[0], &m_storage_buffer[m_iWaveBufMaxSize], m_storage_buffer_size);
            }

        }
        else {
            break;
        }

        Sleep(1);
    }

    return mmpResult;
}

#endif