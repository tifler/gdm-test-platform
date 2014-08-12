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

#include "MmpDecoderVideo_Dvc.hpp"

#if (MMP_OS==MMP_OS_WINCE60)

#include "../MmpComm/MmpUtil.hpp"

#define Dvc_CreateFile() CreateFile(L"DVC1:", (GENERIC_READ | GENERIC_WRITE), 0, NULL, OPEN_EXISTING, 0,	 NULL)
#define Dvc_CloseHandle(x) CloseHandle(x)
#define Dvc_DeviceIoControl(handle, msg, dwInBuf, dwInLen, dwOutBuf, dwOutLen) DeviceIoControl(handle, msg, dwInBuf, dwInLen, dwOutBuf, dwOutLen, NULL, NULL)


/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Dvc Member Functions

CMmpDecoderVideo_Dvc::CMmpDecoderVideo_Dvc(CMmpMediaInfo* pMediaInfo) : CMmpDecoderVideo(pMediaInfo)
,m_davinci_hdl(INVALID_HANDLE_VALUE)
{

}

CMmpDecoderVideo_Dvc::~CMmpDecoderVideo_Dvc()
{

}

MMP_RESULT CMmpDecoderVideo_Dvc::Open()
{
    MMP_RESULT mmpResult;
    SMmpDriverIOArg dvcIoArg;
    ABSTRACT_CODEC_TYPE codecID;
    BOOL bRet;

    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    codecID=this->GetDvcCodecID(m_pbih->biCompression);

    m_davinci_hdl=Dvc_CreateFile();
	if( m_davinci_hdl==INVALID_HANDLE_VALUE )
    {
        MMPDEBUGMSG( MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Dvc::Open] FAIL: Dvc_CreateFile() \n\r")));
        return MMP_FAILURE;
	}

    //Driver Open
    MMPDEBUGMSG( MMPZONE_MONITOR, (TEXT("[CDavinciDecoder::Open] TRY: Create Davinci Decoder \n\r")));
    dvcIoArg.inbuf[0]=m_pbih->biWidth;
    dvcIoArg.inbuf[1]=m_pbih->biHeight;
    dvcIoArg.inbuf[2]=(unsigned int)codecID;
    dvcIoArg.inbufcount=16;
    dvcIoArg.outbufcount=16;

    bRet=Dvc_DeviceIoControl(
          m_davinci_hdl, //  HANDLE hDevice, 
          DVCDRVMSG_DECODER_CREATE, //DWORD dwIoControlCode, 
          dvcIoArg.inbuf, //NULL, //LPVOID lpInBuffer, 
          dvcIoArg.inbufcount*sizeof(DWORD), //NULL, //DWORD nInBufferSize, 
          dvcIoArg.outbuf, //LPVOID lpOutBuffer, 
          dvcIoArg.outbufcount*sizeof(DWORD) //DWORD nOutBufferSize, 
         ); 
    if( bRet==FALSE )
    {
        MMPDEBUGMSG( MMPZONE_ERROR, (TEXT("[CDavinciDecoder::Open] FAIL: Create Davinci Decoder \n\r")));
        mmpResult=MMP_FAILURE;
    }
    else
    {
        MMPDEBUGMSG( MMPZONE_MONITOR, (TEXT("[CDavinciDecoder::Open] SUCCESS: Create Davinci Decoder \n\r")));
        mmpResult=MMP_SUCCESS;
    }

    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_Dvc::Close()
{
    MMP_RESULT mmpResult;
    
    if(m_davinci_hdl!=INVALID_HANDLE_VALUE )
    {
        Dvc_DeviceIoControl(
              m_davinci_hdl, //  HANDLE hDevice, 
              DVCDRVMSG_DECODER_DESTROY, //DWORD dwIoControlCode, 
              NULL,//dwInBuf, //NULL, //LPVOID lpInBuffer, 
              NULL, //2*sizeof(DWORD), //NULL, //DWORD nInBufferSize, 
              NULL, //LPVOID lpOutBuffer, 
              NULL //DWORD nOutBufferSize, 
             ); 
        
        Dvc_CloseHandle(m_davinci_hdl);
        m_davinci_hdl=INVALID_HANDLE_VALUE;
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


ABSTRACT_CODEC_TYPE CMmpDecoderVideo_Dvc::GetDvcCodecID(unsigned int fourcc)
{
    ABSTRACT_CODEC_TYPE codecId=CODEC_UNKNOWN;

    fourcc=CMmpUtil::MakeUpperFourcc(fourcc);

    switch(fourcc)
    {
        case MMPMAKEFOURCC('M','P','4','V'):
        case MMPMAKEFOURCC('X','V','I','D'):
        case MMPMAKEFOURCC('D','I','V','X'):
        case MMPMAKEFOURCC('D','X','5','0'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Mpeg4 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=CODEC_DECODER_MPEG4;
            break;
            
        //case MMPMAKEFOURCC('D','I','V','3'):
        //case MMPMAKEFOURCC('d','i','v','3'):
        //    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Divx3 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
        //    codecId=CODEC_DECODER_MPEG4;
        //    break;
    
        case MMPMAKEFOURCC('H','2','6','4'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] H264 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=CODEC_DECODER_H264;
            break;
    
        case MMPMAKEFOURCC('A','V','C','1'):
        case MMPMAKEFOURCC('X','2','6','4'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] AVC1 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=CODEC_DECODER_AVC1;
            break;
    
        case MMPMAKEFOURCC('H','2','6','3'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] H263 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=CODEC_DECODER_H263;
            break;
    
        case MMPMAKEFOURCC('R','V','4','0'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] RV40 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=CODEC_DECODER_RV40;
            break;
        
        case MMPMAKEFOURCC('F','L','V','1'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] FLV1 Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=CODEC_DECODER_FLV1_H263;
            break;
        
        case MMPMAKEFOURCC('W','M','V','3'):
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] WMV Decoder Select(%c%c%c%c) \n\r"), fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            codecId=CODEC_DECODER_VC1;
            break;

         
        default:
            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Mme::GetMtekMmeCodecID] Unknown codec fourccc(0x%08x %c%c%c%c) \n\r"), 
                fourcc,
                fourcc>>0, fourcc>>8, fourcc>>16, fourcc>>24));
            break;
    }

    return codecId;
}


MMP_RESULT CMmpDecoderVideo_Dvc::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult)
{
    SMmpDriverIOArg dvcIoArg;
    MMP_RESULT mmpResult=MMP_FAILURE;
    BOOL bRet;

    dvcIoArg.inbuf[0]=(unsigned int)0;//pMmpStreamFile->GetCodecType(pMmpStreamFile); //Abstract codec type
    dvcIoArg.inbuf[1]=0;
    dvcIoArg.inbuf[2]=(unsigned int)pMediaSample->m_pAu;
    dvcIoArg.inbuf[3]=(unsigned int)pMediaSample->m_iAuSize;
    dvcIoArg.inbuf[4]=(unsigned int)pMediaSample->m_iSampleNumber; 
    dvcIoArg.inbuf[5]=(unsigned int)MMP_PIXELFORMAT_YUV420_PHYADDR;  
    dvcIoArg.inbuf[6]=(unsigned int)0;//pDecodedBuffer;
    dvcIoArg.inbuf[7]=(unsigned int)0; //Dest PhyAddr
    dvcIoArg.inbuf[8]=(unsigned int)0;//pMmpStreamFile->GetPicWidth(pMmpStreamFile);  //Pic Width
    dvcIoArg.inbuf[9]=(unsigned int)0;//pMmpStreamFile->GetPicHeight(pMmpStreamFile);  //Pic Height

    dvcIoArg.inbufcount=16;
    dvcIoArg.outbufcount=16;

    pDecResult->m_IsImage=0;
    
    bRet=Dvc_DeviceIoControl(
                m_davinci_hdl, //  HANDLE hDevice, 
                DVCDRVMSG_DECODER_DECODE_AU, //DWORD dwIoControlCode, 
                dvcIoArg.inbuf, //NULL, //LPVOID lpInBuffer, 
                dvcIoArg.inbufcount*sizeof(DWORD), //NULL, //DWORD nInBufferSize, 
                dvcIoArg.outbuf, //LPVOID lpOutBuffer, 
                dvcIoArg.outbufcount*sizeof(DWORD) //DWORD nOutBufferSize, 
            );

    if(bRet)
    {
        mmpResult=MMP_SUCCESS;
        if(dvcIoArg.outbuf[0]==1)
        {
            pDecResult->m_IsImage=1;
            pDecResult->m_uiDecodedBufPhyAddr=dvcIoArg.outbuf[5];
        }
    }

    return mmpResult;
}

#endif//#if (MMP_OS==MMP_OS_WINCE60)
