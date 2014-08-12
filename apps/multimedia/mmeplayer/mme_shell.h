#ifndef MMESHELL_H__
#define MMESHELL_H__

#include "MmpDefine.h"
#include "MmpDemuxer.hpp"
#include "MmpMuxer.hpp"
#include "MmpDecoderVideo.hpp"
#include "MmpEncoderVideo.hpp"
#include "MmpRenderer.hpp"
#include "MmpUtil.hpp"
#include "MmpOAL.hpp"
#include "MmpPlayer.hpp"


#define MMESHELL_INFO  1
#define MMESHELL_ERROR 1
#if (MMP_OS == MMP_OS_WIN32)
//#define MMESHELL_PRINT(cond,printf_exp) do { if(cond) CMmpUtil::Printf printf_exp; }while(0);
#define MMESHELL_PRINT(cond,printf_exp) do { if(cond) printf printf_exp; }while(0);
#else
#define MMESHELL_PRINT(cond,printf_exp) do { if(cond) printf printf_exp; }while(0);
#endif

#ifdef __cplusplus
extern "C" {
#endif

void mme_shell_main(void);
void mme_shell_deinit(void);

int  mme_console_get_number(void);

/* player */
int mme_command_player_start(int argc, char* argv[]);
int mme_command_player_stop(int argc, char* argv[]);
int mme_command_player_seek(int argc, char* argv[]);
int mme_command_player_info(int argc, char* argv[]);

int mme_command_player_loop(int argc, char* argv[]);

/* encoder */
int mme_command_encoder_test01(int argc, char* argv[]);
int mme_command_player_enc_start(int argc, char* argv[]);
int mme_command_player_enc_stop(int argc, char* argv[]);

/* vpu */
int mme_command_vpu_load(int argc, char* argv[]);
int mme_command_vpu_unload(int argc, char* argv[]);
int mme_command_vpu_test1(int argc, char* argv[]);
int mme_command_vpu_test2(int argc, char* argv[]);
int mme_command_vpu_test3(int argc, char* argv[]);

/* system */
int mme_command_system_init(int argc, char* argv[]);
int mme_command_system_deinit(int argc, char* argv[]);
int mme_command_system_reset(int argc, char* argv[]);
int mme_command_system_memalloc(int argc, char* argv[]);
int mme_command_system_hardwork(int argc, char* argv[]);
int mme_command_system_meminfo(int argc, char* argv[]);
int mme_command_system_checktick(int argc, char* argv[]);
int mme_command_system_struct_align(int argc, char* argv[]);


int mme_command_socket_connect_to_server(int argc, char* argv[]);


#ifdef __cplusplus
}
#endif


#endif
