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


#include "MmpPlayerEx1.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerEx1 Member Functions

CMmpPlayerEx1::CMmpPlayerEx1(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp)
,m_pMmpDecoderVideo(NULL)
,m_pMmpRendererVideo(NULL)
{
    

}

CMmpPlayerEx1::~CMmpPlayerEx1()
{
    
}

MMP_RESULT CMmpPlayerEx1::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpPlayer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    m_pMmpDecoder[MMP_MEDIATYPE_VIDEO]=CMmpDecoder::CreateObject(m_pMmpSource->GetMediaInfo(MMP_MEDIATYPE_VIDEO));
    if(!m_pMmpDecoder[MMP_MEDIATYPE_VIDEO])
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerEx1::Open] FAIL: CMmpDecoder::CreateObject() \n\r")));
        return MMP_FAILURE;
    }
    m_pMmpDecoderVideo=m_pMmpDecoder[MMP_MEDIATYPE_VIDEO];

#if 0
    m_playerCreateProp.m_videoRendererProp.m_iPicWidth=m_pMmpDecoderVideo->GetPicWidth();
    m_playerCreateProp.m_videoRendererProp.m_iPicHeight=m_pMmpDecoderVideo->GetPicHeight();
    m_pMmpRenderer[MMP_MEDIATYPE_VIDEO]=CMmpRenderer::CreateObject(&m_playerCreateProp.m_videoRendererProp);
    if(!m_pMmpRenderer[MMP_MEDIATYPE_VIDEO])
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerEx1::Open] FAIL: CMmpRenderer::CreateObject() \n\r")));
        return MMP_FAILURE;
    }
    m_pMmpRendererVideo=m_pMmpRenderer[MMP_MEDIATYPE_VIDEO];
#endif

    return MMP_SUCCESS;
}


MMP_RESULT CMmpPlayerEx1::Close()
{
    MMP_RESULT mmpResult;


    mmpResult=CMmpPlayer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    m_pMmpDecoderVideo=NULL;
    m_pMmpRendererVideo=NULL;

    return MMP_SUCCESS;
}

void CMmpPlayerEx1::Service()
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
    unsigned int t1, t2, fps;
    unsigned int decTick, renTick;
    unsigned int startTick, beforeTick, curTick;

#if 1
    m_playerCreateProp.m_videoRendererProp.m_iPicWidth=m_pMmpDecoderVideo->GetPicWidth();
    m_playerCreateProp.m_videoRendererProp.m_iPicHeight=m_pMmpDecoderVideo->GetPicHeight();
    m_pMmpRenderer[MMP_MEDIATYPE_VIDEO]=CMmpRenderer::CreateObject(&m_playerCreateProp.m_videoRendererProp);
    if(!m_pMmpRenderer[MMP_MEDIATYPE_VIDEO])
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerEx1::Open] FAIL: CMmpRenderer::CreateObject() \n\r")));
        return;
    }
    m_pMmpRendererVideo=m_pMmpRenderer[MMP_MEDIATYPE_VIDEO];
#endif

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerEx1::Service] Start Service.. \n\r")));

    pAu=new unsigned char[MAXBUFSIZE];
    pDecodedBuffer=new unsigned char[MAXBUFSIZE];

    iSampleNumber=0;
    startTick=CMmpOAL_GetTickCount();
    beforeTick=startTick;
    decTick=0;
    renTick=0;
    fps=0;
    while(m_bServiceRun)
    {
        //m_pMmpDemuxer->GetNextData()
        //m_pMmpDecoder->DecodeAu();
        //m_pMmpRenderer->Render();
        curTick=CMmpOAL_GetTickCount();
        mmpResult=m_pMmpSource->GetNextData(pAu, &auSize, MAXBUFSIZE, &mediaType, &timeStamp);
        if(mmpResult!=MMP_SUCCESS)
        {
            continue;
        }

        if(mediaType==MMP_MEDIATYPE_AUDIO)
        {
            continue;
        }

        MMPDEBUGMSG(0, (TEXT("[Player] %d (%x %x %x %x %x %x %x %x ) ts(%d) \n\r"),
                    auSize,
                    pAu[0],pAu[1],pAu[2],pAu[3],pAu[4],pAu[5],pAu[6],pAu[7],
                    timeStamp ));

        memset(&mediaSample, 0x00, sizeof(mediaSample));
        memset(&decResult, 0x00, sizeof(decResult));
        mediaSample.m_pAu=pAu;
        mediaSample.m_iAuSize=auSize;
        mediaSample.m_iSampleNumber=iSampleNumber;
        mediaSample.m_iDestLogAddr=(unsigned int)pDecodedBuffer;
        
        if(m_pMmpDecoderVideo)
        {
            t1=CMmpOAL_GetTickCount();
            mmpResult=m_pMmpDecoderVideo->DecodeAu(&mediaSample, &decResult);
            t2=CMmpOAL_GetTickCount();
            decTick+=t2-t1;
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
                      m_pMmpRendererVideo->Render(&decResult);
                      t2=CMmpOAL_GetTickCount();
                      renTick+=t2-t1;
                 }
                 }

            fps++;
            }

        if(curTick-beforeTick>1000 && fps>0 )
        {
            MMPDEBUGMSG(1, (TEXT("%d. DT(%d %d)  RT(%d %d)\n"), (curTick-startTick)/1000, decTick/fps, fps, renTick/fps, fps));
         
            beforeTick=curTick;
            decTick=0;
            renTick=0;
            fps=0;
        }
    
        iSampleNumber++;
        CMmpOAL_Sleep(1);
    }

    if(pAu)
    {
        delete [] pAu;
    }

    delete [] pDecodedBuffer;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerEx1::Service] Service Ended!! \n\r")));
}

