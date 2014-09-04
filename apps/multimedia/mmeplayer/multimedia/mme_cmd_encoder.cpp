#include "mme_shell.h"
#include "MmpEncoder.hpp"

static CMmpPlayer* s_pMmpPlayerEnc = NULL;

#if (MMP_OS == MMP_OS_WIN32)
#define CONTENTS_PATH "C:\\MediaSample\\Mp4\\"
#else
#define CONTENTS_PATH "/mnt/"
#endif
//#define CONTENTS_PATH "/storage/sdcard1/"

struct mme_support_extension {
    int id;
    char ext[8];
};

#if 1
static struct mme_support_extension mme_exts[] = { 
    { 0, "wmv" },  
    { 0, "asf" },  
    { 0, "avi" },  
    //{ 0, "divx" },  
    { 0, "rm" },  
    { 0, "rmvb" },  
    { 0, "mp4" },  
    { 0, "mov" },  
    //{ 0, "k3g" },  
    //{ 0, "ogv" },  
    //{ 0, "ogg" },  
    //{ 0, "3gp" },  
    //{ 0, "m4a" },  
  
    { 0, "mkv" },  
    { 0, "webm" },  
  
    //{ 0, "flv" },  
    //{ 0, "tp" },  
    //{ 0, "ts" },  
  
    //{ 0, "skm" },  
    //{ 0, "mpeg" },  
    //{ 0, "mpg" },  
  
    //{ 0, "mp3" },  
    //{ 0, "mid" },  //ERROR : Stagefright Player
    //{ 0, "aac" },  
    //{ 0, "wma" },  
    //{ 0, "flac" },  
    //{ 0, "wav" },  

};
#else

struct mme_support_extension mme_exts[] = { 
    //{ 0, "wmv" },  
    //{ 0, "asf" },  
    //{ 0, "avi" },  
    //{ 0, "divx" },  
    { 0, "rm" },  
    { 0, "rmvb" },  
  					  
    //{ 0, "mp4" },  
    //{ 0, "mov" },  
    //{ 0, "k3g" },  
    //{ 0, "ogv" },  
    //{ 0, "ogg" },  
    //{ 0, "3gp" },  
    //{ 0, "m4a" },  
  
    //{ 0, "mkv" },  
    //{ 0, "webm" },  
  
    //{ 0, "flv" },  
    //{ 0, "tp" },  
    //{ 0, "ts" },  
  
    //{ 0, "skm" },  
    //{ 0, "mpeg" },  
    //{ 0, "mpg" },  
  
    //{ 0, "mp3" },  
    //{ 0, "mid" },  
    //{ 0, "aac" },  
    //{ 0, "wma" },  
    //{ 0, "flac" },  
    //{ 0, "wav" },  

};
#endif
				  

static int s_player_enc_file_count = 0;
int mme_command_player_enc_start(int argc, char* argv[]) {

    CMmpPlayerCreateProp player_create_config;
    int file_array_max = 256;
    int file_size_max = 512;
    char* file_array = NULL;
    char* file_name;

    int ext_array_max = sizeof(mme_exts)/sizeof(mme_exts[0]);
    int ext_size_max = 32;
    int ext_count = 0;
    char* ext_array = NULL;
    char*  ext;
	
#ifdef WIN32	
    MMP_U32 encFormat = MMP_FOURCC_VIDEO_MPEG4;
#else
    MMP_U32 encFormat = MMP_FOURCC_VIDEO_H264;
#endif

    char contents_path[256] = CONTENTS_PATH;

    int file_cnt;
    int i;
	
    int contents_number;
#ifdef WIN32	
    MMP_BOOL bForceSWDecoder = MMP_TRUE;
#else
    MMP_BOOL bForceSWDecoder = MMP_FALSE;
#endif

    MMP_BOOL bForceSWEncoder = MMP_FALSE;

    if(argc > 1) {
        if(strcmp(argv[1], "h264") == 0) {
            encFormat = MMP_FOURCC_VIDEO_H264;
        }
        else if(strcmp(argv[1], "mpeg4") == 0) {
            encFormat = MMP_FOURCC_VIDEO_MPEG4;
        }
        else {
            encFormat = MMP_FOURCC_VIDEO_MPEG4;
        }
    }
    if(argc > 2) {
        bForceSWEncoder = atoi(argv[2]);
        if( (bForceSWEncoder==1) || (bForceSWEncoder==0) ) {
            
        }
        else {
            bForceSWEncoder = 0;
        }
    }
    if(argc > 3) {
        bForceSWDecoder = atoi(argv[3]);
        if( (bForceSWDecoder==1) || (bForceSWDecoder==0) ) {
            
        }
        else {
            bForceSWDecoder = 1;
        }
    }

    //MMP_U32 player_type = MMP_PLAYER_STAGEFRIGHT;
    MMP_U32 player_type = MMP_PLAYER_VIDEO_ONLY;

    if(s_pMmpPlayerEnc != NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is running. now will stop \n"));
        s_pMmpPlayerEnc->PlayStop();
        CMmpPlayer::DestroyObject(s_pMmpPlayerEnc);
        s_pMmpPlayerEnc = NULL;
    }

    //if(argc == 2) {
    //    if(strcmp(argv[1], "a") == 0) {
   //         player_type = MMP_PLAYER_AUDIO_ONLY;
    //        MMESHELL_PRINT(MMESHELL_ERROR, ("Player Typer is MMP_PLAYER_AUDIO_ONLY \n"));
    //    }
    //}

    file_array = (char*)malloc(file_array_max*file_size_max);
    ext_array = (char*)malloc(ext_array_max*ext_size_max);
    
    for(i = 0; i < ext_array_max; i++) {
        ext=&ext_array[ext_count*ext_size_max];  
        strcpy(ext, mme_exts[i].ext); 
        ext_count++;
    }
    
    file_cnt = CMmpUtil::GetFileList(contents_path, 
                                     ext_array, ext_count, ext_size_max, 
                                     file_array, file_array_max, file_size_max);

    while(1) {

        MMESHELL_PRINT(MMESHELL_INFO, ("********* filecnt : %d ************* \n", file_cnt));
        for(i = 0; i < file_cnt; i++) {
            
            file_name = &file_array[i*file_size_max];
            MMESHELL_PRINT(MMESHELL_INFO, ("%d. %s \n", i+1, file_name));
        }
        MMESHELL_PRINT(MMESHELL_INFO, ("\n\n Select Media Number : "));

        file_name = NULL;
        contents_number = mme_console_get_number() - 1;

        if(contents_number == -1) {
        
            break;
        }
        if(contents_number < file_cnt ) {

            file_name = &file_array[contents_number*file_size_max];

            MMESHELL_PRINT(MMESHELL_INFO, ("Selected File : \n"));
            MMESHELL_PRINT(MMESHELL_INFO, ("\t%s \n", file_name ));

            break;
        }
        else {
        
            MMESHELL_PRINT(MMESHELL_INFO, ("Invalid Number \n"));
        }
    }

    if(file_name != NULL) {
    
        strcpy(player_create_config.filename, file_name);
        player_create_config.video_config.m_hRenderWnd = NULL;//hwnd;
        player_create_config.video_config.m_hRenderDC = NULL; //hdc;
        player_create_config.bForceSWCodec = bForceSWDecoder;

#if (MMP_OS == MMP_OS_WIN32)
        player_create_config.video_config.m_renderer_type = MMP_RENDERER_TYPE_DUMMY;
#else
        player_create_config.video_config.m_renderer_type = MMP_RENDERER_TYPE_NORMAL;    
#endif
        player_create_config.video_config.m_bVideoEncoder = MMP_TRUE;
        player_create_config.video_config.m_bVideoEncoderForceSWCodec = bForceSWEncoder;
        
#if (MMP_OS == MMP_OS_WIN32)
        strcpy(player_create_config.video_config.m_VideoEncFileName, "d:\\work\\player_enc.ammf");
#elif (MMP_OS == MMP_OS_LINUX)
        //strcpy(player_create_config.video_config.m_VideoEncFileName, "/root/player_enc.ammf");
        sprintf(player_create_config.video_config.m_VideoEncFileName, "/mnt/player_enc%d_%c%c%c%c.ammf", 
                                             s_player_enc_file_count,

                                              MMPGETFOURCC(encFormat, 0), 
                                               MMPGETFOURCC(encFormat, 1),
                                               MMPGETFOURCC(encFormat, 2), 
                                               MMPGETFOURCC(encFormat, 3) 
                                             );
#else
        strcpy(player_create_config.video_config.m_VideoEncFileName, file_name);
        strcat(player_create_config.video_config.m_VideoEncFileName, ".ammf");
#endif

        
        CMmpEncoder::CreateVideoDefaultConfig(encFormat, //MMP_U32 mmp_video_fourcc,
                                              0, 0, //MMP_U32 pic_width, MMP_U32 pic_height,
                                              30, 10, 1024*1024, //MMP_U32 framerate, MMP_U32 idr_period, MMP_U32 bitrate,
                                              &player_create_config.video_config.m_VideoEncoderCreateConfig);

        
        MMESHELL_PRINT(MMESHELL_INFO, ("SrcName: %s \n", player_create_config.filename));
        MMESHELL_PRINT(MMESHELL_INFO, ("EncName: %s \n", player_create_config.video_config.m_VideoEncFileName));
        MMESHELL_PRINT(MMESHELL_INFO, ("SWDecoder: %d  SWEncoder: %d \n", bForceSWDecoder, bForceSWEncoder));
        MMESHELL_PRINT(MMESHELL_INFO, ("EncFormat : %c%c%c%c \n", 
                           MMPGETFOURCC(player_create_config.video_config.m_VideoEncoderCreateConfig.nFormat, 0), 
                           MMPGETFOURCC(player_create_config.video_config.m_VideoEncoderCreateConfig.nFormat, 1),
                           MMPGETFOURCC(player_create_config.video_config.m_VideoEncoderCreateConfig.nFormat, 2), 
                           MMPGETFOURCC(player_create_config.video_config.m_VideoEncoderCreateConfig.nFormat, 3) 
                           ));


        s_pMmpPlayerEnc = CMmpPlayer::CreateObject(player_type, &player_create_config);
        //s_pMmpPlayerEnc = CMmpPlayer::CreateObject(MMP_PLAYER_STAGEFRIGHT, &player_create_config);
        //pPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_AUDIO_VIDEO, &player_create_config);
        //pPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_VIDEO_ONLY, &player_create_config);
        //pPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_AUDIO_ONLY, &player_create_config);
#if 1
        if(s_pMmpPlayerEnc != NULL) {
            s_pMmpPlayerEnc->PlayStart();
        }
#endif
    }

    if(file_array) free(file_array);
    if(ext_array) free(ext_array);

    s_player_enc_file_count++;

    return 0;
}

int mme_command_player_enc_stop(int argc, char* argv[]) {

    if(s_pMmpPlayerEnc != NULL) {
        
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is running. now will stop s_pMmpPlayerEnc=0x%08x \n", s_pMmpPlayerEnc));

        s_pMmpPlayerEnc->PlayStop();
        
        MMESHELL_PRINT(MMESHELL_ERROR, ("player release111.  s_pMmpPlayerEnc=0x%08x \n", s_pMmpPlayerEnc));

        CMmpPlayer::DestroyObject(s_pMmpPlayerEnc);
        s_pMmpPlayerEnc = NULL;

    }

    return 0;
}   

