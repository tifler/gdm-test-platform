#include "mme_shell.h"

static CMmpPlayer* s_pMmpPlayer = NULL;
#define CONTENTS_PATH "/mnt/"
//#define CONTENTS_PATH "/storage/sdcard1/"

struct mme_support_extension {
    int id;
    char ext[8];
};

#if 1
struct mme_support_extension mme_exts[] = { 
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

    { 0, "ammf" },  
  
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
				  


int mme_command_player_start(int argc, char* argv[]) {

    CMmpPlayer* pPlayer;
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

    char contents_path[256] = CONTENTS_PATH;

    int file_cnt;
    int i;
	int iret = 0;

    int contents_number;
    MMP_BOOL bForceSWCodec = MMP_FALSE;

    if( (argc > 1) && (argc < 10) ) {
        bForceSWCodec = atoi(argv[1]);
    }

    //MMP_U32 player_type = MMP_PLAYER_STAGEFRIGHT;
    MMP_U32 player_type = MMP_PLAYER_VIDEO_ONLY;

    if(s_pMmpPlayer != NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is running. now will stop \n"));
        s_pMmpPlayer->PlayStop();
        CMmpPlayer::DestroyObject(s_pMmpPlayer);
        s_pMmpPlayer = NULL;
    }

    if(argc == 2) {
    
        if(strcmp(argv[1], "a") == 0) {
            player_type = MMP_PLAYER_AUDIO_ONLY;

            MMESHELL_PRINT(MMESHELL_ERROR, ("Player Typer is MMP_PLAYER_AUDIO_ONLY \n"));
        }

    }

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

	if(argc == 0xFFFF) {
	
		contents_number = 0;
		file_name = &file_array[contents_number*file_size_max];
		strcpy(file_name, argv[0]);
	}
	else {

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
	}

    if(file_name != NULL) {
    
        strcpy(player_create_config.filename, file_name);
        player_create_config.video_config.m_hRenderWnd = NULL;//hwnd;
        player_create_config.video_config.m_hRenderDC = NULL; //hdc;
        player_create_config.bForceSWCodec = bForceSWCodec;
        
        s_pMmpPlayer = CMmpPlayer::CreateObject(player_type, &player_create_config);
        //s_pMmpPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_STAGEFRIGHT, &player_create_config);
        //pPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_AUDIO_VIDEO, &player_create_config);
        //pPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_VIDEO_ONLY, &player_create_config);
        //pPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_AUDIO_ONLY, &player_create_config);
#if 1
        if(s_pMmpPlayer != NULL) {
            s_pMmpPlayer->PlayStart();
        }
		else {
			iret = -1;
		}
		
#endif
    }

    if(file_array) free(file_array);
    if(ext_array) free(ext_array);

    return iret;
}


int mme_command_player_stop(int argc, char* argv[]) {

    if(s_pMmpPlayer != NULL) {
        
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is running. now will stop s_pMmpPlayer=0x%08x \n", s_pMmpPlayer));

        s_pMmpPlayer->PlayStop();
        
        MMESHELL_PRINT(MMESHELL_ERROR, ("player release111.  s_pMmpPlayer=0x%08x \n", s_pMmpPlayer));

        CMmpPlayer::DestroyObject(s_pMmpPlayer);
        s_pMmpPlayer = NULL;

    }

    return 0;
}   

int mme_command_player_seek(int argc, char* argv[]) {

    MMP_S32 hour, min, sec;
    MMP_S64 pts;

    if(s_pMmpPlayer == NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is not running.  \n"));
        return -1;
    }
        
    if(argc != 4) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR:  usage  is  'seek [hour] [min] [sec]  \n"));
        return -1; 
    }

    hour = atoi(argv[1]);
    min = atoi(argv[2]);
    sec = atoi(argv[3]);

    pts = (MMP_S64)(hour*3600 + min*60 + sec) * 1000000L;
    
    s_pMmpPlayer->Seek(pts);
        
    return 0;
}

int mme_command_player_info(int argc, char* argv[]) {

    MMP_S32 dur_hour, dur_min, dur_sec, dur_msec;
    MMP_S32 pos_hour, pos_min, pos_sec, pos_msec;
    

    if(s_pMmpPlayer == NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is not running.  \n"));
        return -1;
    }
    
    s_pMmpPlayer->GetDuration(&dur_hour, &dur_min, &dur_sec, &dur_msec);
    s_pMmpPlayer->GetPlayPosition(&pos_hour, &pos_min, &pos_sec, &pos_msec);

    MMESHELL_PRINT(MMESHELL_ERROR, ("PlayPos :  %02d:%02d:%02d:%02d  / %02d:%02d:%02d:%02d  \n",
                                        pos_hour, pos_min, pos_sec, pos_msec,
                                        dur_hour, dur_min, dur_sec, dur_msec
                                        ));

        
    return 0;
}

int mme_command_player_loop(int argc, char* argv[]) {


    CMmpPlayer* pPlayer;
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

    char contents_path[256] = CONTENTS_PATH;

    int file_index, file_cnt;
    int i;
    int play_contents_count;

    int contents_number;

    MMP_U32 play_start_tick;
    MMP_U32 play_dur, play_pos;
    MMP_U32 before_tick, cur_tick;
    char msg[1024];

    static CMmpPlayer* pMmpPlayer = NULL;

    if(s_pMmpPlayer != NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is running. now will stop \n"));
        s_pMmpPlayer->PlayStop();
        CMmpPlayer::DestroyObject(s_pMmpPlayer);
        s_pMmpPlayer = NULL;
    }

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

/*
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
*/

    MMESHELL_PRINT(MMESHELL_INFO, ("********* filecnt : %d ************* \n", file_cnt));
    for(i = 0; i < file_cnt; i++) {
        
        file_name = &file_array[i*file_size_max];
        MMESHELL_PRINT(MMESHELL_INFO, ("%d. %s \n", i+1, file_name));
    }
    MMESHELL_PRINT(MMESHELL_INFO, ("************************************* \n\n"));


    for(play_contents_count = 0, file_index = 0; file_cnt > 0 ;file_index = (file_index+1)%file_cnt) {
    
        file_name = &file_array[file_index*file_size_max];

        strcpy(player_create_config.filename, file_name);
        player_create_config.video_config.m_hRenderWnd = NULL;//hwnd;
        player_create_config.video_config.m_hRenderDC = NULL; //hdc;

        MMESHELL_PRINT(MMESHELL_ERROR, ("[LongRun Test] %d. %s  \n", file_index, file_name));

        pMmpPlayer = CMmpPlayer::CreateObject(MMP_PLAYER_STAGEFRIGHT, &player_create_config);
        if(pMmpPlayer != NULL) {
            pMmpPlayer->PlayStart();

            play_contents_count++;

            before_tick = CMmpUtil::GetTickCount();
            play_dur = pMmpPlayer->GetDurationMS();
            play_start_tick = CMmpUtil::GetTickCount();
            while(1) {
                
                cur_tick = CMmpUtil::GetTickCount();
                play_pos = pMmpPlayer->GetPlayPositionMS();

                if( (cur_tick-before_tick) > 1000*60 ) {

                    sprintf(msg, "[LongRun Test] %d. %02d:%02d:%02d /  %02d:%02d:%02d  fileIdx:%d/%d %d filename: %s",
                        (cur_tick-play_start_tick)/1000, 
                        CMmpUtil::Time_GetHour(play_pos), CMmpUtil::Time_GetMin(play_pos), CMmpUtil::Time_GetSec(play_pos),
                        CMmpUtil::Time_GetHour(play_dur), CMmpUtil::Time_GetMin(play_dur), CMmpUtil::Time_GetSec(play_dur),
                        file_index, file_cnt, play_contents_count,
                        file_name 
                        );

                    MMESHELL_PRINT(MMESHELL_ERROR, (" %s \n", msg));

                    before_tick = cur_tick;

                }

                if(    ((play_pos + 100) > play_dur)  
                    || (((cur_tick-play_start_tick)+3000) > play_dur) ) 
                {
                    break;
                }

                
                CMmpUtil::Sleep(100);
            }

            sprintf(msg, "[LongRun PlayStop] %d. %02d:%02d:%02d /  %02d:%02d:%02d  fileIdx:%d/%d %d filename: %s",
                        (cur_tick-play_start_tick)/1000, 
                        CMmpUtil::Time_GetHour(play_pos), CMmpUtil::Time_GetMin(play_pos), CMmpUtil::Time_GetSec(play_pos),
                        CMmpUtil::Time_GetHour(play_dur), CMmpUtil::Time_GetMin(play_dur), CMmpUtil::Time_GetSec(play_dur),
                        file_index, file_cnt, play_contents_count,
                        file_name 
                        );
            MMESHELL_PRINT(MMESHELL_ERROR, (" %s \n", msg));

            pMmpPlayer->PlayStop();
            CMmpPlayer::DestroyObject(pMmpPlayer);
            pMmpPlayer = NULL;
        }
    }


    if(file_array) free(file_array);
    if(ext_array) free(ext_array);

    return 0;
}
