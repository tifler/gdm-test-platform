#include "MpMmeIF.h"
#include "../MmpComm/MmpUtil.hpp"


#if (MMP_OS==MMP_OS_WIN32)

//#define MTEKMMEDLL_FILENAME TEXT("D:\\Project_MPlayer\\MPlayer_Project\\MPlayer_Win32Prj\\MPlayer_Win32Prj\\debug\\MtekMME.dll")
#define MTEKMMEDLL_FILENAME TEXT("D:\\Project_MPlayer\\MPlayer_Project\\MPlayer_Win32Prj\\MPlayer_Win32Prj\\debug\\MnsMME.dll")

#define PROCTEXT(x) (LPCSTR)(x)

#elif (MMP_OS==MMP_OS_WINCE60)

#if DEBUG
#define MTEKMMEDLL_FILENAME L"\\Program Files\\MtekMme\\MtekMME.dll"

#else

#define MTEKMMEDLL_FILENAME L"\\Windows\\MtekMME.dll"

#endif

#define PROCTEXT TEXT

#else
#error "ERROR : Select Filter Platform in fAVIMuxer.cpp"
#endif

//Mp Demuxer Api
MPDEMUXER_HDL (MPAPICALL *MpDemuxer_Create)( MPLAYER_DEMUXERTYPE demuxerType, WCHAR* destFileName )=0;
MP_RESULT (MPAPICALL *MpDemuxer_Destroy)(MPDEMUXER_HDL hdl)=0;
MP_RESULT (MPAPICALL *MpDemuxer_GetAudioProp)(MPDEMUXER_HDL hdl, void* prop)=0;
MP_RESULT (MPAPICALL *MpDemuxer_GetVideoProp)(MPDEMUXER_HDL hdl, void* prop)=0;
MP_RESULT (MPAPICALL *MpDemuxer_GetNextData)(MPDEMUXER_HDL hdl, unsigned char* data, int* dataSize, int maxInputSize, MPLAYER_MEDIA_TYPE* mediaType, unsigned int* timeStamp)=0;
MP_RESULT (MPAPICALL *MpDemuxer_GetNextAudioData)(MPDEMUXER_HDL hdl, unsigned char* data, int* dataSize, int maxInputSize)=0;
MP_RESULT (MPAPICALL *MpDemuxer_GetNextVideoData)(MPDEMUXER_HDL hdl, unsigned char* data, int* dataSize, int maxInputSize)=0;
int (MPAPICALL *MpDemuxer_GetMediaInfoSize)(MPDEMUXER_HDL hdl, MPLAYER_MEDIA_TYPE mediaType)=0;

MP_RESULT (MPAPICALL *MpDemuxer_RegisterEventWnd)(MPDEMUXER_HDL hdl, unsigned int notifymsg, void* handle)=0;
MP_RESULT (MPAPICALL *MpDemuxer_GetStopTimeStamp)(MPDEMUXER_HDL hdl, unsigned int* timeStamp)=0;
MP_RESULT (MPAPICALL *MpDemuxer_SetCurTimeStamp)(MPDEMUXER_HDL hdl, unsigned int timeStamp)=0;
MP_RESULT (MPAPICALL *MpDemuxer_GetCurTimeStamp)(MPDEMUXER_HDL hdl, unsigned int* timeStamp)=0;
int (MPAPICALL *MpDemuxer_IsSeekable)(MPDEMUXER_HDL hdl)=0;

MPDECODERV_HDL (MPAPICALL *MpDecoderV_Create)(CMpCodecCreateProp*)=0;
MP_RESULT (MPAPICALL *MpDecoderV_Destroy)(MPDECODERV_HDL hdl)=0;
MP_RESULT (MPAPICALL *MpDecoderV_DecodeAu)(MPDECODERV_HDL hdl, unsigned char* pAu, int auSize, int sampleNumber, int renderId, unsigned char* decodedBuffer, int* isImage, unsigned int* decTick)=0;

MPDECODERA_HDL (MPAPICALL *MpDecoderA_Create)(MPLAYER_AUDIO_INFO*)=0;
MP_RESULT (MPAPICALL *MpDecoderA_Destroy)(MPDECODERA_HDL)=0;
MP_RESULT (MPAPICALL *MpDecoderA_DecodeAu)(MPDECODERA_HDL hdl, unsigned char* pAu, int auSize, int sampleNumber, int renderId, unsigned char* decodedBuffer, int* decodedSize, int , unsigned int* decTick)=0;


static HINSTANCE s_hDllInst;
static int s_MpMme_RefCount=0;



MMP_RESULT MpMmeIF_Create()
{
    if(s_hDllInst)
    {
        s_MpMme_RefCount++;
        return MMP_SUCCESS;
    }

    s_hDllInst=LoadLibrary(MTEKMMEDLL_FILENAME);
    if(!s_hDllInst)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMediaFile_MPlayer::Open] FAIL: LoadLibrary(%s) \n\r"),MTEKMMEDLL_FILENAME)) ;
        return MMP_FAILURE;
    }

    MpDemuxer_Create=(MPDEMUXER_HDL  (MPAPICALL *)( MPLAYER_DEMUXERTYPE demuxerType, WCHAR* destFileName ))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_Create") );
    MpDemuxer_Destroy=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_Destroy") );
    MpDemuxer_GetAudioProp=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl, void* prop))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_GetAudioProp") );
    MpDemuxer_GetVideoProp=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl, void* prop))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_GetVideoProp") );
    MpDemuxer_GetMediaInfoSize=(int (MPAPICALL *)(MPDEMUXER_HDL hdl, MPLAYER_MEDIA_TYPE mediaType))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_GetMediaInfoSize"));
    MpDemuxer_GetNextData=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl, unsigned char* data, int* dataSize, int maxInputSize, MPLAYER_MEDIA_TYPE* mediaType, unsigned int* timeStamp))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_GetNextData") );
    
    MpDemuxer_RegisterEventWnd=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl, unsigned int notifymsg, void* handle))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_RegisterEventWnd") );
    MpDemuxer_GetStopTimeStamp=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl, unsigned int* timeStamp))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_GetStopTimeStamp") );
    MpDemuxer_SetCurTimeStamp=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl, unsigned int timeStamp))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_SetCurTimeStamp") );
    MpDemuxer_GetCurTimeStamp=(MP_RESULT  (MPAPICALL *)(MPDEMUXER_HDL hdl, unsigned int* timeStamp))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_GetCurTimeStamp") );
    MpDemuxer_IsSeekable=(int  (MPAPICALL *)(MPDEMUXER_HDL hdl))GetProcAddress(s_hDllInst, PROCTEXT("MpDemuxer_IsSeekable") );
    

    MpDecoderV_Create=(MPDECODERV_HDL (MPAPICALL *)(CMpCodecCreateProp*))GetProcAddress(s_hDllInst, PROCTEXT("MpDecoderV_Create") );
    MpDecoderV_Destroy=(MP_RESULT (MPAPICALL *)(MPDECODERV_HDL hdl))GetProcAddress(s_hDllInst, PROCTEXT("MpDecoderV_Destroy") );
    MpDecoderV_DecodeAu=(MP_RESULT (MPAPICALL *)(MPDECODERV_HDL hdl, unsigned char* pAu, int auSize, int sampleNumber, int renderId, unsigned char* decodedBuffer, int* isImage, unsigned int* decTick))GetProcAddress(s_hDllInst, PROCTEXT("MpDecoderV_DecodeAu") );

    MpDecoderA_Create=(MPDECODERA_HDL (MPAPICALL *)(MPLAYER_AUDIO_INFO*))GetProcAddress(s_hDllInst, PROCTEXT("MpDecoderA_Create") );
    MpDecoderA_Destroy=(MP_RESULT (MPAPICALL *)(MPDECODERA_HDL hdl))GetProcAddress(s_hDllInst, PROCTEXT("MpDecoderA_Destroy") );
    MpDecoderA_DecodeAu=(MP_RESULT (MPAPICALL *)(MPDECODERA_HDL hdl, unsigned char* pAu, int auSize, int sampleNumber, int renderId, unsigned char* decodedBuffer, int* decodedSize, int , unsigned int* decTick))GetProcAddress(s_hDllInst, PROCTEXT("MpDecoderA_DecodeAu") );

    if( !MpDemuxer_Create ||
        !MpDemuxer_Destroy ||
        !MpDemuxer_GetAudioProp ||
        !MpDemuxer_GetVideoProp ||
        !MpDemuxer_GetNextData ||
        !MpDemuxer_GetMediaInfoSize ||
        
        !MpDemuxer_RegisterEventWnd ||
        !MpDemuxer_GetStopTimeStamp ||
        !MpDemuxer_SetCurTimeStamp ||
        !MpDemuxer_GetCurTimeStamp ||
        !MpDemuxer_IsSeekable ||
        

        !MpDecoderV_Create ||
        !MpDecoderV_Destroy ||
        !MpDecoderV_DecodeAu ||
    
        !MpDecoderA_Create ||
        !MpDecoderA_Destroy ||
        !MpDecoderA_DecodeAu 
        )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CDemuxer_MPlayer::Open] FAIL:  MpDll Get Interface \n\r")));
        
        MpMmeIF_Destroy();

        return MMP_FAILURE;
    }
    
    s_MpMme_RefCount++;

    return MMP_SUCCESS;
}

MMP_RESULT MpMmeIF_Destroy()
{
    if(s_hDllInst)
    {
        s_MpMme_RefCount--;
        if(s_MpMme_RefCount>0)
        {
            return MMP_SUCCESS;
        }

        FreeLibrary(s_hDllInst);
        s_hDllInst=NULL;

        MpDemuxer_Create=NULL;
        MpDemuxer_Destroy=NULL;
        MpDemuxer_GetAudioProp=NULL;
        MpDemuxer_GetVideoProp=NULL;
        MpDemuxer_GetNextData=NULL;
        MpDemuxer_GetMediaInfoSize=NULL;
        MpDemuxer_GetStopTimeStamp=NULL;

        MpDemuxer_RegisterEventWnd=NULL;
        MpDemuxer_GetStopTimeStamp=NULL;
        MpDemuxer_SetCurTimeStamp=NULL;
        MpDemuxer_GetCurTimeStamp=NULL;
        

        MpDecoderV_Create=NULL;
        MpDecoderV_Destroy=NULL;
        MpDecoderV_DecodeAu=NULL;

        MpDecoderA_Create=NULL;
        MpDecoderA_Destroy=NULL;
        MpDecoderA_DecodeAu=NULL;
    }
    return MMP_SUCCESS;
}

#if 0
bool MpMmeIF_IsPlayThisFile(CString strFileName)
{
    CString strExt;
    bool flag;

    strExt=CMfcUtil::SplitExt(strFileName);
    strExt.MakeLower();

    if( strExt.Compare(TEXT("avi"))==0 ||
        strExt.Compare(TEXT("mp4"))==0 ||
        strExt.Compare(TEXT("3gp"))==0 ||
        strExt.Compare(TEXT("mov"))==0 ||
        strExt.Compare(TEXT("flv"))==0 ||
        strExt.Compare(TEXT("mkv"))==0 ||
        strExt.Compare(TEXT("rm"))==0 ||
        strExt.Compare(TEXT("rmvb"))==0 ||
        strExt.Compare(TEXT("264"))==0 
    )
    {
        flag=true;
    }
    else
        flag=false;

    return flag;
}
#endif