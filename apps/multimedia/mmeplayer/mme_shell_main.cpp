#include "mme_shell.h"
#include <stdio.h>
#include <string.h>
#include "MmpAudioTool.hpp"


static int mme_parse_args(char *cmdline, char **argv);
static int command_help(int argc, char **argv);
static int command_test1(int argc, char **argv);
static int command_test2(int argc, char **argv);

struct mme_command {
    char *cmdstr;                     	// command string
    int (*func)(int argc, char **argv);	// execute function
    char *desc;    			            // descriptor
};


struct mme_command mme_cmds[] = { 
        { (char*)"help" , command_help,  (char*)"display command description"},  
        { (char*)"?"    , command_help,  (char*)"display command description"},
        { (char*)"test1"    , command_test1,  (char*)"test1 "},
        { (char*)"test2"    , command_test2,  (char*)"test2 "},

        /* system */
        { (char*)"syst_reset"   , mme_command_system_reset,  (char*)"system test reset"},
        { (char*)"syst_malloc"  , mme_command_system_memalloc,  (char*)"system test malloc"},
        { (char*)"syst_hardwork"  , mme_command_system_hardwork,  (char*)"system test hardwork"},
#if (MMP_OS == MMP_OS_LINUX)
        //{ (char*)"syst_clisock"  , mme_command_socket_connect_to_server,  (char*)"connect to server via TCP/IP socket comm"},
#endif
        { (char*)"syst_meminfo"  , mme_command_system_meminfo,  (char*)"/proc/meminfo"},
        { (char*)"syst_tick"  , mme_command_system_checktick,  (char*)"check systick"},
        { (char*)"syst_align"  , mme_command_system_struct_align,  (char*)"check struct align"},
        
        /* player */
        { (char*)"play"    , mme_command_player_start,  (char*)"player start"},
        { (char*)"stop"    , mme_command_player_stop,  (char*)"player stop"},
        { (char*)"stop_all"    , mme_command_player_stop_all,  (char*)"all player stop"},
        { (char*)"seek"    , mme_command_player_seek,  (char*)"player seek ex)seek [hour] [min] [sec] "},
        { (char*)"stat"    , mme_command_player_status,  (char*)"display play status infomation  "},
        { (char*)"play_loop"    , mme_command_player_loop,  (char*)"player loop "},
        { (char*)"disp"    , mme_command_player_set_first_renderer,  (char*)"set first renderer to display  "},

        /*encoder */
        { (char*)"play_enc"    , mme_command_player_enc_start,  (char*)"player enc start"},
        { (char*)"stop_enc"    , mme_command_player_enc_stop,  (char*)"player enc stop"},


        /* ion */
        { (char*)"ion_t1"  , mme_command_ion_test1,  (char*)"ion test1"},

#if (MMESHELL_RIL==1)
        /*ril*/
        { (char*)"ril_init"   , mme_command_ril_init,  (char*)"ril init"},
        { (char*)"ril_deinit"   , mme_command_ril_deinit,  (char*)"ril deinit"},
        { (char*)"ril_radiopower"   , mme_command_ril_set_radio_power,  (char*)"ril radio power on/off (airplane mode)  ex)ril_radiopower on/off"},
        { (char*)"ril_modeminit"   , mme_command_ril_modem_init,  (char*)"ril modem init"},
        { (char*)"ril_sig_stren"   , mme_command_ril_signal_strength,  (char*)"display ril signal strength"},
        { (char*)"ril_mute"   , mme_command_ril_mute,  (char*)"ri_mute  0/1"},
        { (char*)"ril_call"   , mme_command_ril_call,  (char*)"ri_call 01077369829"},
        { (char*)"ril_testmem"   , mme_command_ril_test_request_mem,  (char*)"test req mem"},
#endif
};

int command_help(int argc, char **argv) {
  
   int i,j,k,sz;

  sz=sizeof(mme_cmds)/sizeof(mme_cmds[0]);
  for(i=0;i<sz;i++)
  {
        fputc('\r', stdout);
        fputc('\t', stdout);
	 
        k=(int)strlen(mme_cmds[i].cmdstr);
        for(j=0;j<k;j++)
            fputc(mme_cmds[i].cmdstr[j], stdout);
        for(;j<15;j++)
            fputc(' ', stdout); //MTVDEBUG_putchar(' ');
	 
        fputs(mme_cmds[i].desc, stdout);
        fputc('\n', stdout);
        fputc('\r', stdout);
    }
     
    return 0;
}

static void mme_shell_init() {

    mme_command_system_init(0, NULL);
}

void mme_shell_deinit() {

    mme_command_player_stop_all(0, NULL);
    mme_command_system_deinit(0, NULL);

#if (MMESHELL_RIL==1)
    mme_command_ril_deinit(0, NULL);
#endif

}

void mme_shell_main(int argc_app, char* argv_app[]) {

    int cmdlp, cmdsz;
    int maxbufsize = 4096;
    char* readbuffer = (char*)malloc(maxbufsize);
    int  argc;
    char *argv[32];
	FILE* fp;
	int i;
	int diriect_file_play_mode = 0;

    mme_shell_init();

    cmdsz=sizeof(mme_cmds)/sizeof(mme_cmds[0]);

    printf("------------------------------------ \n");
	printf("  App   : mmeplayer  \n");
    printf("  Build : %s , hthwang@anapass.com \n", __DATE__);
    printf("  \targc:%d  argv[0]:%s  argv[1]:%s \n", argc_app, argv_app[0], argv_app[1]);
	printf("------------------------------------ \n");
	
	if(argc_app == 2) {
		fp = fopen(argv_app[1], "rb");
		if(fp!=NULL) {
			fseek(fp, 0, SEEK_END);
			i = ftell(fp);
			if(i > 1024*100) {
				diriect_file_play_mode = 1;
			}
			fclose(fp);
		}
	}

	if(diriect_file_play_mode == 1) {
	
		argv[0] = argv_app[1];
		mme_command_player_start(0xFFFF, argv);
		while(1) {
			CMmpUtil::Sleep(1000*10);	
		}

	}
	else {

		while(1)
		{
			printf("mmeshell>> ");
			fgets(readbuffer, maxbufsize, stdin); 
			argc = mme_parse_args(readbuffer, argv);

			if(argc >= 1)
			{
				cmdlp = 0;
				while( cmdlp<cmdsz )
				{
					if( strcmp( argv[0], mme_cmds[cmdlp].cmdstr ) == 0 )
					{
						if(mme_cmds[cmdlp].func)
						{
							mme_cmds[cmdlp].func( argc, argv );
							printf("\n");
							break;
						}
					}
					cmdlp++;
				}
	         
				if(cmdlp==cmdsz) 
				{
					if( strcmp(argv[0], "exit") == 0 )
					{
						break;
					}
					else
					{
						printf("\tunknown command: %s\n\r", argv[0] );
					}
				}
			}
		}

	}


    free(readbuffer);

    mme_shell_deinit();

    printf("\n\rmme_shell_main bye bye! \n\n\r");
}


char * ___mme_strtok;
char * mme_strpbrk( char * cs,char * ct)
{
	const char *sc1,*sc2;

	for( sc1 = cs; *sc1 != '\0'; ++sc1) {
		for( sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2)
				return (char *) sc1;
		}
	}
	return 0;
}

char * mme_strtok(char * s,const char * ct)
{
	char *sbegin, *send;

	sbegin  = s ? s : ___mme_strtok;
	if (!sbegin) {
		return 0;
	}
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') {
		___mme_strtok = 0;
		return( 0 );
	}
	send = mme_strpbrk( sbegin, (char*)ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___mme_strtok = send;
	return (sbegin);
}

static const char *mme_delim = " \f\n\r\t\v";
static int mme_parse_args(char *cmdline, char **argv)
{
	char *tok;
	int argc = 0;

	argv[argc] = 0;
                   
	for (tok = mme_strtok(cmdline, mme_delim); tok; tok = mme_strtok(0, mme_delim)) 
    {
		argv[argc++] = tok;
	}

	return argc;
}

int  mme_console_get_number(void)
{
    char buffer[16];
    fgets(buffer, 16, stdin);
    return atoi(buffer);
}


static int command_test1(int argc, char **argv) {

    return 0;
}

static int command_test2(int argc, char **argv) {
    return 0;
}






