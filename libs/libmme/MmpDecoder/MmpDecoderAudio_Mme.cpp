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

#include "MmpDecoderAudio_Mme.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderAudio_Mme Member Functions

CMmpDecoderAudio_Mme::CMmpDecoderAudio_Mme(CMmpMediaInfo* pMediaInfo) : CMmpDecoderAudio(pMediaInfo)
,m_decoder_hdl(NULL)
{

}

CMmpDecoderAudio_Mme::~CMmpDecoderAudio_Mme()
{

}

MMP_RESULT CMmpDecoderAudio_Mme::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderAudio::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    MpMmeIF_Create();

    memcpy(&m_mpAudioInfo.wf, m_pwf, sizeof(MMPWAVEFORMATEX));
    m_decoder_hdl=(*MpDecoderA_Create)(&m_mpAudioInfo);
    if(m_decoder_hdl==NULL)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_Mme::Open] MpDecoderA_Create(%d %d %d %d) \n\r"), m_pwf->wFormatTag, m_pwf->nSamplesPerSec,  m_pwf->nChannels, m_pwf->wBitsPerSample));
        return MMP_FAILURE;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderAudio_Mme::Close()
{
    MMP_RESULT mmpResult;

    if(m_decoder_hdl)
    {
        mmpResult=(MMP_RESULT)(*MpDecoderA_Destroy)(m_decoder_hdl);
        m_decoder_hdl=NULL;
        MpMmeIF_Destroy();
    }

    mmpResult=CMmpDecoderAudio::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_Mme::Close] CMmpDecoderAudio::Close() \n\r")));
        return mmpResult;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio_Mme::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
#if 0
    MMP_RESULT mmpResult;
    
    pDecResult->m_IsImage=0;
    mmpResult=(MMP_RESULT)(*MpDecoderA_DecodeAu)(m_decoder_hdl, 
                                                 pMediaSample->m_pAu, pMediaSample->m_iAuSize,
                                                 pMediaSample->m_iSampleNumber,
                                                 0, //renderId
                                                 (unsigned char*)pMediaSample->m_iDestLogAddr, //decodedBuffer
                                                 &pDecResult->m_iDecodedSize, pMediaSample->m_iResultBufMaxSize,
                                                 (unsigned int*)&pDecResult->m_uiDecodedDuration );
    pDecResult->m_uiDecodedBufPhyAddr=0;
    pDecResult->m_uiDecodedBufLogAddr=(unsigned int)pMediaSample->m_iDestLogAddr;
    if(pDecResult->m_iDecodedSize>0)
    {
        pDecResult->m_IsImage=1;
    }
  
    return mmpResult;
#else
	return MMP_FAILURE;
#endif
}