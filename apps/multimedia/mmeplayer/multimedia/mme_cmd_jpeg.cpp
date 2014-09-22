#include "mme_shell.h"
#include "mmp_buffer_mgr.hpp"
#include "mmp_vpu_dev.hpp"


#if (MMP_OS == MMP_OS_WIN32)
#define CONTENTS_PATH "C:\\MediaSample\\JPEG\\"
#else
#define CONTENTS_PATH "/mnt/"
#endif


struct mme_support_extension {
    int id;
    char ext[8];
};

static struct mme_support_extension mme_exts[] = { 
    { 0, "jpg" },  
  
};
				  

#define MAX_PLAYER_COUNT 4
static CMmpPlayer* s_pMmpPlayer_Arr[MAX_PLAYER_COUNT] = {NULL, NULL, NULL, NULL};

int mme_command_jpegviewer_start(int argc, char* argv[]) {

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
    MMP_S32 i;
	int iret = 0;

    int contents_number;
    MMP_S32 player_index = 0;

    MMP_BOOL bForceSWCodec = MMP_FALSE;
    MMP_U32 player_type = CMmpPlayer::DEFAULT;
    
    /* Get Player Index */
    for(i = 0; i < MAX_PLAYER_COUNT; i++) {
        if(s_pMmpPlayer_Arr[i] == NULL) {
            player_index = i;
            break;
        }
    }
    if(i == MAX_PLAYER_COUNT) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("All Player is running. \n"));
        return -1;
    }

    /* Check Arg */
    if( (argc > 1) && (argc < 10) ) {
        bForceSWCodec = atoi(argv[1]);
    }
    if(argc == 2) {
        if(strcmp(argv[1], "a") == 0) {
            player_type = CMmpPlayer::AUDIO_ONLY;
            MMESHELL_PRINT(MMESHELL_ERROR, ("Player Typer is CMmpPlayer::AUDIO_ONLY \n"));
        }
    }

    /* Select Media File */
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

    /* Play Start */
    if(file_name != NULL) {
    
        memset(&player_create_config, 0x00, sizeof(player_create_config));
        strcpy(player_create_config.filename, file_name);
        player_create_config.video_config.m_hRenderWnd = NULL;//hwnd;
        player_create_config.video_config.m_hRenderDC = NULL; //hdc;
        player_create_config.bForceSWCodec = bForceSWCodec;
        
        pPlayer = CMmpPlayer::CreateObject(player_type, &player_create_config);
#if 1
        if(pPlayer != NULL) {
            pPlayer->PlayStart();
            s_pMmpPlayer_Arr[player_index] = pPlayer;
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

static MMP_S32 get_player_index(int argc, char* argv[]) {

    MMP_S32 player_index = 0;
    MMP_S32 i;

    for(i = 0; i < MAX_PLAYER_COUNT; i++) {
        if(s_pMmpPlayer_Arr[i] != NULL) {
            player_index = i;
            break;
        }
    }

    if(argc > 1) {
        player_index = atoi(argv[1]);
    }

    if( (player_index <0) || (player_index >= MAX_PLAYER_COUNT) ) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR: player index = %d \n", player_index));
        return -1;
    }

    if(s_pMmpPlayer_Arr[player_index] == NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR: player[%d] is not running \n", player_index));
        return -1;
    }
    
    return player_index;
}

int mme_command_jpegviewer_stop(int argc, char* argv[]) {

    MMP_S32 player_index = 0;
    CMmpPlayer* pPlayer;

    player_index = get_player_index(argc, argv);
    if(player_index < 0) {
        return -1;
    }
    
    pPlayer = s_pMmpPlayer_Arr[player_index];
    pPlayer->PlayStop();
    CMmpPlayer::DestroyObject(pPlayer);
    
    MMESHELL_PRINT(MMESHELL_ERROR, ("[Player %d] Stop, Bye!  \n", player_index));
    
    s_pMmpPlayer_Arr[player_index] = NULL;

    return 0;
}   

int mme_command_jpegviewer_stop_all(int argc, char* argv[]) {

    MMP_S32 player_index = 0;
    CMmpPlayer* pPlayer;


    for(player_index = 0; player_index < MAX_PLAYER_COUNT; player_index++) {
    
        pPlayer = s_pMmpPlayer_Arr[player_index];
        if(pPlayer != NULL) {
            pPlayer->PlayStop();
            CMmpPlayer::DestroyObject(pPlayer);
            MMESHELL_PRINT(MMESHELL_ERROR, ("[Player %d] Stop, Bye!  \n", player_index));
        }
        s_pMmpPlayer_Arr[player_index] = NULL;
    }

    return 0;
}

int mme_command_jpegviewer_seek(int argc, char* argv[]) {

#if 0
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
#endif        
    return 0;
}

int mme_command_jpegviewer_status(int argc, char* argv[]) {

    MMP_S32 dur_hour, dur_min, dur_sec, dur_msec;
    MMP_S32 pos_hour, pos_min, pos_sec, pos_msec;
    MMP_U32 vf;

    MMP_S32 player_index = 0, inst_idx;
    CMmpPlayer* pPlayer;
    struct mmp_video_hw_codec_instance_info hw_codec_inst_info;

    for(player_index = 0; player_index < MAX_PLAYER_COUNT; player_index++) {
    
        pPlayer = s_pMmpPlayer_Arr[player_index];

        if(pPlayer != NULL) {
            pPlayer->GetDuration(&dur_hour, &dur_min, &dur_sec, &dur_msec);
            pPlayer->GetPlayPosition(&pos_hour, &pos_min, &pos_sec, &pos_msec);

            vf = pPlayer->GetVideoFormat();

            MMESHELL_PRINT(MMESHELL_ERROR, ("[Player %d] Display(%s) %c%c%c%c(%s) %dx%d PlayerFPS=%d  DecodingDur=%d  TotalFrm=%d (%02d:%02d:%02d/%02d:%02d:%02d) \n",
                                                player_index, 
                                                pPlayer->IsFirstVideoRenderer()?"ON":"OFF",
                                                MMPGETFOURCC(vf,0),MMPGETFOURCC(vf,1),MMPGETFOURCC(vf,2),MMPGETFOURCC(vf,3),
                                                pPlayer->GetVideoDecoderClassName(),
                                                pPlayer->GetVideoWidth(), pPlayer->GetVideoHeight(),
                                                pPlayer->GetPlayFPS(), 
                                                pPlayer->GetVideoDecoderDur(),
                                                pPlayer->GetVideoDecoderTotalDecFrameCount(),
                                                pos_hour, pos_min, pos_sec, 
                                                dur_hour, dur_min, dur_sec 
                                                ));
        }
        else {
            MMESHELL_PRINT(MMESHELL_ERROR, ("[Player %d] Not Available     \n", player_index));
        }

    }

    for(inst_idx = 0; inst_idx < 4; inst_idx++) {
        mmp_vpu_dev::get_instance()->VPU_GetCodecInfo(inst_idx, &hw_codec_inst_info);
        MMESHELL_PRINT(MMESHELL_ERROR, ("Inst%d.  Use(%d)   \n", 
                       hw_codec_inst_info.instance_index, 
                       hw_codec_inst_info.is_use
                       ));
    }



    return 0;
}

int mme_command_jpegviewer_set_first_renderer(int argc, char* argv[]) {

    MMP_S32 player_index = 0;
    CMmpPlayer* pPlayer;

    if(argc != 2) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("FAIL:  argument err  e.g. disp [player_idx] \n"));
        return -1;
    }

    player_index = atoi(argv[1]);

    if( (player_index <0) || (player_index >= MAX_PLAYER_COUNT) ) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR: player index = %d (0~%d) \n", player_index, MAX_PLAYER_COUNT-1));
        return -1;
    }

    pPlayer = s_pMmpPlayer_Arr[player_index];
    if(pPlayer == NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR: Player %d  is stopped \n", player_index));
        return -1;
    }
    
    pPlayer->SetFirstVideoRenderer();

    return 0;
}

int mme_command_jpegviewer_loop(int argc, char* argv[]) {

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

    MMP_U32 play_start_tick;
    MMP_U32 play_dur, play_pos;
    MMP_U32 before_tick, cur_tick;
    char msg[1024];

    static CMmpPlayer* pMmpPlayer = NULL;


#if 0
    if(s_pMmpPlayer != NULL) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("player is running. now will stop \n"));
        s_pMmpPlayer->PlayStop();
        CMmpPlayer::DestroyObject(s_pMmpPlayer);
        s_pMmpPlayer = NULL;
    }
#else

    if( get_player_index(argc, argv) != -1) {
        MMESHELL_PRINT(MMESHELL_ERROR, ("ERROR: other player is running. cannot start loop test!! \n"));
        return -1;
    }

#endif

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

        pMmpPlayer = CMmpPlayer::CreateObject(CMmpPlayer::STAGEFRIGHT, &player_create_config);
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


int mme_command_jpegviewer_meminfo(int argc, char* argv[]) {

    /* display mmp_buffer list */  
    mmp_buffer_mgr::get_instance()->print_info();
    
    return 0;
}