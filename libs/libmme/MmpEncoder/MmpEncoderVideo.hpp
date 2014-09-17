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

#ifndef MMPENCODERVIDEO_HPP__
#define MMPENCODERVIDEO_HPP__

#include "MmpEncoder.hpp"
#include "TemplateList.hpp"

struct mmp_enc_video_frame {
    MMP_U8* pData;
    MMP_U32 nDataSize;
    MMP_U32 nFlag;
};

class CMmpEncoderVideo : public CMmpEncoder
{
friend class CMmpEncoder;

protected:
    MMP_U8 m_szCodecName[32];
    MMPBITMAPINFOHEADER* m_pbih;;
	MMPBITMAPINFOHEADER m_bih_in;
	MMPBITMAPINFOHEADER m_bih_out;

private:
    MMP_BOOL m_bNeedPictureBufPhyAddr;
    
    TCircular_Queue<struct mmp_enc_video_frame> m_queue_ecnframe;
    
protected:
    CMmpEncoderVideo(struct MmpEncoderCreateConfig *pCreateConfig, MMP_BOOL bbNeedPictureBufPhyAddr);
    virtual ~CMmpEncoderVideo();

    virtual MMP_RESULT Open();
	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize) {return MMP_FAILURE;}
    virtual MMP_RESULT Close();

    void EncodeMonitor(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult);
    void EncodeMonitor(class mmp_buffer_videostream* p_buf_videostream);
    virtual const MMP_U8* GetClassName() = 0;
    
public:
    virtual MMP_RESULT EncodeAu(class mmp_buffer_videoframe* p_buf_videoframe, class mmp_buffer_videostream* p_buf_videostream) = 0;
    
	inline const MMPBITMAPINFOHEADER& GetBIH_Out() { return m_bih_out; }
	inline const MMPBITMAPINFOHEADER& GetBIH_In() { return m_bih_in; }
    inline MMP_BOOL IsNeedPictureBufPhyAddr() { return m_bNeedPictureBufPhyAddr; }

    void SetVideoSize(MMP_U32 w, MMP_U32 h);


    MMP_BOOL EncodedFrameQueue_IsEmpty();
    MMP_RESULT EncodedFrameQueue_GetFrame(MMP_U8* pBuffer, MMP_U32 nBufMaxSize, MMP_U32* nRetBufSize, MMP_U32* nRetFlag);

    //virtual MMP_RESULT Encode_YUV420Planar_Vir(MMP_U8* Y, MMP_U8* U, MMP_U8* V, 
    //                                           MMP_U8* pEncStreamBuf, MMP_U32 nBufMaxSize, MMP_U32* nBufSize, MMP_U32* nFlag);


    inline MMP_S32 GetVideoPicWidth() { return m_bih_out.biWidth; }
    inline MMP_S32 GetVideoPicHeight() { return m_bih_out.biHeight; }

protected:
    MMP_RESULT EncodedFrameQueue_AddFrame(MMP_U8* pBuffer, MMP_U32 nBufSize, MMP_U32 nFlag);
    MMP_RESULT EncodedFrameQueue_AddFrameWithConfig_Mpeg4(MMP_U8* pBuffer, MMP_U32 nBufSize, MMP_U32 nFlag);
};

#endif

