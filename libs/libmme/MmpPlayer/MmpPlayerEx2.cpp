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


#include "MmpPlayerEx2.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerEx2 Member Functions

CMmpPlayerEx2::CMmpPlayerEx2(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)
{
    

}

CMmpPlayerEx2::~CMmpPlayerEx2()
{
    
}

MMP_RESULT CMmpPlayerEx2::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpPlayer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    //Create Audio Decoder
    m_pMmpDecoder[MMP_MEDIATYPE_AUDIO]=CMmpDecoder::CreateObject(m_pMmpSource->GetMediaInfo(MMP_MEDIATYPE_AUDIO));
    if(!m_pMmpDecoder[MMP_MEDIATYPE_AUDIO])
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerAudio::Open] FAIL: AUDIO CMmpDecoder::CreateObject() \n\r")));
        return MMP_FAILURE;
    }
    
    //Create Video Decoder
    m_pMmpDecoder[MMP_MEDIATYPE_VIDEO]=CMmpDecoder::CreateObject(m_pMmpSource->GetMediaInfo(MMP_MEDIATYPE_VIDEO));
    if(!m_pMmpDecoder[MMP_MEDIATYPE_VIDEO])
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerEx2::Open] FAIL: VIDEO CMmpDecoder::CreateObject() \n\r")));
        return MMP_FAILURE;
    }
    
    //Create Audio Renderer
    m_pMmpRenderer[MMP_MEDIATYPE_AUDIO]=CMmpRenderer::CreateObject(m_pMmpSource->GetMediaInfo(MMP_MEDIATYPE_AUDIO));
    if(!m_pMmpRenderer[MMP_MEDIATYPE_AUDIO])
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerAudio::Open] FAIL: AUDIO CMmpRenderer::CreateObject() \n\r")));
        return MMP_FAILURE;
    }
    
    //Create Video Renderer
    m_playerCreateProp.m_videoRendererProp.m_iPicWidth=m_pMmpDecoder[MMP_MEDIATYPE_VIDEO]->GetPicWidth();
    m_playerCreateProp.m_videoRendererProp.m_iPicHeight=m_pMmpDecoder[MMP_MEDIATYPE_VIDEO]->GetPicHeight();
    m_pMmpRenderer[MMP_MEDIATYPE_VIDEO]=CMmpRenderer::CreateObject(&m_playerCreateProp.m_videoRendererProp);
    if(!m_pMmpRenderer[MMP_MEDIATYPE_VIDEO])
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerEx2::Open] FAIL: VIDEO CMmpRenderer::CreateObject() \n\r")));
        return MMP_FAILURE;
    }
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpPlayerEx2::Close()
{
    MMP_RESULT mmpResult;


    mmpResult=CMmpPlayer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

void CMmpPlayerEx2::Service()
{
    unsigned char* pAu=NULL;
    unsigned char* pDecodedBuffer=NULL;
    const int MAXBUFSIZE=1920*1088*3/2;
    int auSize;
    MMP_MEDIATYPE mediaType;
    unsigned int timeStamp;
    MMP_RESULT mmpResult;
    CMmpMediaSample mediaSample;
    CMmpMediaSampleDecodeResult decResult;
    int iSampleNumber;
    CMmpDecoder* pMmpDecoder;
    CMmpRenderer* pMmpRenderer;
    unsigned int t1, t2;
    unsigned int curTick, beforeTick, startTick;
    unsigned int decTick[MMP_MEDIATYPE_MAX], renTick[MMP_MEDIATYPE_MAX];
    unsigned int decCount[MMP_MEDIATYPE_MAX], renCount[MMP_MEDIATYPE_MAX];
    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerEx2::Service] Start Service.. \n\r")));

    pAu=new unsigned char[MAXBUFSIZE];
    pDecodedBuffer=new unsigned char[MAXBUFSIZE];

    startTick=CMmpOAL_GetTickCount();
    beforeTick=startTick;
    iSampleNumber=0;
    memset(decTick, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));
    memset(renTick, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));
    memset(decCount, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));
    memset(renCount, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));

    while(m_bServiceRun)
    {
        curTick=CMmpOAL_GetTickCount();
        mmpResult=m_pMmpSource->GetNextData(pAu, &auSize, MAXBUFSIZE, &mediaType, &timeStamp);
        if(mmpResult!=MMP_SUCCESS)
        {
            continue;
        }

        pMmpDecoder=m_pMmpDecoder[mediaType];
        pMmpRenderer=m_pMmpRenderer[mediaType];


        MMPDEBUGMSG(1, (TEXT("[Player] Type:%d Sz:%d (%x %x %x %x %x %x %x %x ) ts(%d) \n\r"),
                    mediaType,
                    auSize,
                    pAu[0],pAu[1],pAu[2],pAu[3],pAu[4],pAu[5],pAu[6],pAu[7],
                    timeStamp ));

        memset(&mediaSample, 0x00, sizeof(mediaSample));
        memset(&decResult, 0x00, sizeof(decResult));
        mediaSample.m_pAu=pAu;
        mediaSample.m_iAuSize=auSize;
        mediaSample.m_iSampleNumber=iSampleNumber;
        mediaSample.m_iDestLogAddr=(unsigned int)pDecodedBuffer;
        mediaSample.m_iResultBufMaxSize=MAXBUFSIZE;

        t1=CMmpOAL_GetTickCount();
        mmpResult=pMmpDecoder->DecodeAu(&mediaSample, &decResult);
        t2=CMmpOAL_GetTickCount();

        decTick[mediaType]+=t2-t1;
        decCount[mediaType]++;

        if(mmpResult!=MMP_SUCCESS)
        {
            MMPDEBUGMSG(1, (TEXT("[Player] FAIL: DecodeAu \n\r")));
        }
        else
        {
            MMPDEBUGMSG(0, (TEXT("[Player] SUCCESS: DecodeAu (IsImage:%d  ) \n\r"), decResult.m_IsImage));
             if(decResult.m_IsImage)
             {
                  t1=CMmpOAL_GetTickCount();
                  pMmpRenderer->Render(&decResult);
                  t2=CMmpOAL_GetTickCount();

                  renTick[mediaType]+=t2-t1;
                  renCount[mediaType]++;
             }
        }
    
        iSampleNumber++;

        if(curTick-beforeTick>1000)
        {
            MMPDEBUGMSG(1, (TEXT("[PlayerEx2] %d. AD(%d %d) VD(%d %d) AR(%d %d) VR(%d %d) \n"), 
                (curTick-startTick)/1000,
                decCount[MMP_MEDIATYPE_AUDIO],  (decCount[MMP_MEDIATYPE_AUDIO]==0)?0:decTick[MMP_MEDIATYPE_AUDIO]/decCount[MMP_MEDIATYPE_AUDIO],
                decCount[MMP_MEDIATYPE_VIDEO],  (decCount[MMP_MEDIATYPE_VIDEO]==0)?0:decTick[MMP_MEDIATYPE_VIDEO]/decCount[MMP_MEDIATYPE_VIDEO],

                renCount[MMP_MEDIATYPE_AUDIO],  (renCount[MMP_MEDIATYPE_AUDIO]==0)?0:renTick[MMP_MEDIATYPE_AUDIO]/renCount[MMP_MEDIATYPE_AUDIO],
                renCount[MMP_MEDIATYPE_VIDEO],  (renCount[MMP_MEDIATYPE_VIDEO]==0)?0:renTick[MMP_MEDIATYPE_VIDEO]/renCount[MMP_MEDIATYPE_VIDEO]
                
                ));
            
            memset(decTick, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));
            memset(renTick, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));
            memset(decCount, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));
            memset(renCount, 0x00, MMP_MEDIATYPE_MAX*sizeof(unsigned int));


            beforeTick=curTick;
        }
    }

    if(pAu)
    {
        delete [] pAu;
    }

    delete [] pDecodedBuffer;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerEx2::Service] Service Ended!! \n\r")));
}

