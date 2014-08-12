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

#include "MmpDecoderVideo_Mme.hpp"
#include "../MmpComm/MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Mme Member Functions

CMmpDecoderVideo_Mme::CMmpDecoderVideo_Mme(CMmpMediaInfo* pMediaInfo) : CMmpDecoderVideo(pMediaInfo)
,m_decoder_hdl(NULL)
{

}

CMmpDecoderVideo_Mme::~CMmpDecoderVideo_Mme()
{

}

MMP_RESULT CMmpDecoderVideo_Mme::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    MpMmeIF_Create();

    m_codecProp.m_CodecID=this->GetMtekMmeCodecID(m_pbih->biCompression);
    m_codecProp.m_iPicWidth=m_pbih->biWidth;
    m_codecProp.m_iPicHeight=m_pbih->biHeight;
    m_codecProp.m_iDSISize=m_pMediaInfo->GetDSISize();
    if(m_codecProp.m_iDSISize>0)
    {
        memcpy(m_codecProp.m_DSI, m_pMediaInfo->GetDSI(), m_codecProp.m_iDSISize);
    }
    m_decoder_hdl=(*MpDecoderV_Create)(&m_codecProp);
    if(m_decoder_hdl==NULL)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Mme::Open] MpDecoderV_Create(%d %d %d) \n\r"), m_codecProp.m_CodecID, m_pbih->biWidth, m_pbih->biHeight));
        return MMP_FAILURE;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Mme::Close()
{
    MMP_RESULT mmpResult;

    if(m_decoder_hdl)
    {
        mmpResult=(MMP_RESULT)(*MpDecoderV_Destroy)(m_decoder_hdl);
        m_decoder_hdl=NULL;
        MpMmeIF_Destroy();
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Mme::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

    return MMP_SUCCESS;
}

MPLAYER_CODEC CMmpDecoderVideo_Mme::GetMtekMmeCodecID(unsigned int fourcc)
{
    MPLAYER_CODEC codecId=MPLAYER_CODEC_UNKNOWN;

    fourcc=CMmpUtil::MakeUpperFourcc(fourcc);

    switch(fourcc)
    {
        case MMPMAKEFOURCC('D','I','V','3'):
        
        case MMPMAKEFOURCC('M','P','4','2'): //Mpeg4 v2
        case MMPMAKEFOURCC('M','P','4','V'):
        case MMPMAKEFOURCC('X','V','I','D'):
        case MMPMAKEFOURCC('D','I','V','X'):
        case MMPMAKEFOURCC('D','X','5','0'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Mpeg4 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_MPEG4;
            break;
            
        case MMPMAKEFOURCC('M','P','G','1'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Mpeg1 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_MPEG1;
            break;
        
        case MMPMAKEFOURCC('M','P','G','2'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Mpeg2 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_MPEG2;
            break;
        
        //case MMPMAKEFOURCC('D','I','V','3'):
        //case MMPMAKEFOURCC('d','i','v','3'):
        //    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Divx3 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
        //    codecId=CODEC_DECODER_MPEG4;
        //    break;
    
        case MMPMAKEFOURCC('H','2','6','4'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] H264 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_H264;
            break;
    
        case MMPMAKEFOURCC('A','V','C','1'):
        case MMPMAKEFOURCC('X','2','6','4'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] AVC1 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_AVC1;
            break;
    
        case MMPMAKEFOURCC('H','2','6','3'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] H263 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_H263;
            break;
    
        case MMPMAKEFOURCC('R','V','4','0'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] RV40 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_RV40;
            break;
        
        case MMPMAKEFOURCC('F','L','V','1'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] FLV1 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_FLV1_H263;
            break;
        
        case MMPMAKEFOURCC('W','M','V','3'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] WMV Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=MPLAYER_CODEC_DECODER_VC1;
            break;

         
        default:
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Unknown codec fourccc(0x%08x %c%c%c%c) \n\r"), 
                fourcc,
                fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            break;
    }

    return codecId;
}

MMP_RESULT CMmpDecoderVideo_Mme::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
#if 0
    MMP_RESULT mmpResult;
    
    mmpResult=(MMP_RESULT)(*MpDecoderV_DecodeAu)(m_decoder_hdl, 
                                                 pMediaSample->m_pAu, pMediaSample->m_iAuSize,
                                                 pMediaSample->m_iSampleNumber,
                                                 0, //renderId
                                                 (unsigned char*)pMediaSample->m_iDestLogAddr, //decodedBuffer
                                                 &pDecResult->m_IsImage, 
												 (unsigned int*)&pDecResult->m_uiDecodedDuration );
    pDecResult->m_uiDecodedBufPhyAddr=0;
    pDecResult->m_uiDecodedBufLogAddr=(unsigned int)pMediaSample->m_iDestLogAddr;
  
    return mmpResult;
#else
	return MMP_FAILURE;
#endif
}