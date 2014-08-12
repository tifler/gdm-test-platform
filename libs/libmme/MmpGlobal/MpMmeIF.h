#ifndef _MPMMEIF_H__
#define _MPMMEIF_H__

#include "../MmpGlobal/MmpDefine.h"
#include <mmsystem.h>
#include "D:\\Project_MPlayer\\MPlayer_Project\\MPlayerDll\\MpDefine.h"

MMP_RESULT MpMmeIF_Create();
MMP_RESULT MpMmeIF_Destroy();
//bool MtekMmeIF_IsPlayThisFile(CString strFileName);

extern MPDEMUXER_HDL (MPAPICALL *MpDemuxer_Create)( MPLAYER_DEMUXERTYPE demuxerType, WCHAR* destFileName );
extern MP_RESULT (MPAPICALL *MpDemuxer_Destroy)(MPDEMUXER_HDL hdl);
extern MP_RESULT (MPAPICALL *MpDemuxer_GetAudioProp)(MPDEMUXER_HDL hdl, void* prop);
extern MP_RESULT (MPAPICALL *MpDemuxer_GetVideoProp)(MPDEMUXER_HDL hdl, void* prop);
extern MP_RESULT (MPAPICALL *MpDemuxer_GetNextData)(MPDEMUXER_HDL hdl, unsigned char* data, int* dataSize, int maxInputSize, MPLAYER_MEDIA_TYPE* mediaType, unsigned int* timeStamp);
extern MP_RESULT (MPAPICALL *MpDemuxer_GetNextAudioData)(MPDEMUXER_HDL hdl, unsigned char* data, int* dataSize, int maxInputSize);
extern MP_RESULT (MPAPICALL *MpDemuxer_GetNextVideoData)(MPDEMUXER_HDL hdl, unsigned char* data, int* dataSize, int maxInputSize);
extern int (MPAPICALL *MpDemuxer_GetMediaInfoSize)(MPDEMUXER_HDL hdl, MPLAYER_MEDIA_TYPE mediaType);

extern MP_RESULT (MPAPICALL *MpDemuxer_RegisterEventWnd)(MPDEMUXER_HDL hdl, unsigned int notifymsg, void* handle);
extern MP_RESULT (MPAPICALL *MpDemuxer_GetStopTimeStamp)(MPDEMUXER_HDL hdl, unsigned int* timeStamp);
extern MP_RESULT (MPAPICALL *MpDemuxer_SetCurTimeStamp)(MPDEMUXER_HDL hdl, unsigned int timeStamp);
extern MP_RESULT (MPAPICALL *MpDemuxer_GetCurTimeStamp)(MPDEMUXER_HDL hdl, unsigned int* timeStamp);
extern int (MPAPICALL *MpDemuxer_IsSeekable)(MPDEMUXER_HDL hdl);

extern MPDECODERV_HDL (MPAPICALL *MpDecoderV_Create)(CMpCodecCreateProp*);
extern MP_RESULT (MPAPICALL *MpDecoderV_Destroy)(MPDECODERV_HDL hdl);
extern MP_RESULT (MPAPICALL *MpDecoderV_DecodeAu)(MPDECODERV_HDL hdl, unsigned char* pAu, int auSize, int sampleNumber, int renderId, unsigned char* decodedBuffer, int* isImage, unsigned int* decTick);

extern MPDECODERA_HDL (MPAPICALL *MpDecoderA_Create)(MPLAYER_AUDIO_INFO*);
extern MP_RESULT (MPAPICALL *MpDecoderA_Destroy)(MPDECODERA_HDL);
extern MP_RESULT (MPAPICALL *MpDecoderA_DecodeAu)(MPDECODERA_HDL hdl, unsigned char* pAu, int auSize, int sampleNumber, int renderId, unsigned char* decodedBuffer, int* decodedSize, int maxDecodedBufSize, unsigned int* decTick);

#endif

