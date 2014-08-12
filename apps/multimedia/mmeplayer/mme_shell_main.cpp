#include "mme_shell.h"
#include <stdio.h>
#include <string.h>


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
        { (char*)"seek"    , mme_command_player_seek,  (char*)"player seek ex)seek [hour] [min] [sec] "},
        { (char*)"playinfo"    , mme_command_player_info,  (char*)"display play infomation  "},
        { (char*)"play_loop"    , mme_command_player_loop,  (char*)"player loop "},

        /*encoder */
        { (char*)"play_enc"    , mme_command_player_enc_start,  (char*)"player enc start"},
        { (char*)"stop_enc"    , mme_command_player_enc_stop,  (char*)"player enc stop"},

        
        /* vpu */
        { (char*)"vpu_load"    , mme_command_vpu_load,  (char*)"vpu driver load"},
        { (char*)"vpu_unload"    , mme_command_vpu_unload,  (char*)"vpu driver unload"},
        { (char*)"vpu_test1"    , mme_command_vpu_test1,  (char*)"vpu driver test1"},
        { (char*)"vpu_test2"    , mme_command_vpu_test2,  (char*)"vpu driver test2"},
        
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

    mme_command_player_stop(0, NULL);
    mme_command_system_deinit(0, NULL);
    //mme_command_ril_deinit(0, NULL);
}

void mme_shell_main() {

    int cmdlp, cmdsz;
    int maxbufsize = 4096;
    char* readbuffer = (char*)malloc(maxbufsize);
    int  argc;
    char *argv[32];

    mme_shell_init();

    cmdsz=sizeof(mme_cmds)/sizeof(mme_cmds[0]);

    printf("------------------------------------ \n");
    printf("  mmeshell 2014-05-02 \n");
    printf("------------------------------------ \n");

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

void foo(char *fmt, ...) {

    va_list ap;
    int d;
    char c, *p, *s;
    char buf[512];

    va_start(ap, fmt);

#if 0
    while(*fmt) {

        if(*fmt == '%') {

            fmt++;

            switch(*fmt) {

                case 's':                       /* string */
                        s = va_arg(ap, char *);
                        printf("string %s\n", s);
                        break;
                case 'd':                       /* int */
                        d = va_arg(ap, int);
                        printf("int %d\n", d);
                        break;
                case 'c':                       /* char */
                        /* need a cast here since va_arg only
                           takes fully promoted types */
                        c = (char) va_arg(ap, int);
                        printf("char %c\n", c);
                        break;
            }

        }

        fmt++;
    }
#else

    vsprintf(buf, fmt, ap);
    printf("%s", buf);

#endif

    va_end(ap);

}

static int command_test1(int argc, char **argv) {

    foo("TTTT %d %08x %s \n", 1, 10, "TTTEEESSSTTTT");

    return 0;
}

static int command_test2(int argc, char **argv) {
    return 0;
}






