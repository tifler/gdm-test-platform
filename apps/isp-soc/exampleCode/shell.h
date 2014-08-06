/*--------------------------------------------------------------------------
/	Project name	: 
/	Copyright		: Anapass, All rights reserved.
/	File name		: shell.h
/	Description		:
/
/	
/	
/	
/	Name		Date			Action
/	thlee		14.06.18		Create
---------------------------------------------------------------------------*/

#ifndef	__ISP_SHELL_H__
#define __ISP_SHELL_H__

/* --------------------------------------------------------------------------------------------------------------------
													CONSTANTS
 ------------------------------------------------------------------------------------------------------------------ */
#define SHELL_RETURN_OK		0
#define SHELL_RETURN_EXIT	1
#define SHELL_RETURN_ERROR	2

/* --------------------------------------------------------------------------------------------------------------------
													DATA TYPES
 ------------------------------------------------------------------------------------------------------------------ */

typedef struct SHELL_CMD {
	char *cmd;
	char *help;
	int (*pfunc)(char *);
}shell_cmd;

/* --------------------------------------------------------------------------------------------------------------------
													FUNCTION PROTOTYPES
 ------------------------------------------------------------------------------------------------------------------ */
int str2int(const char *str, int multi);
int findargument(int index);

static int shell_quit(char *data);
static int shell_help(char *data);
static int shell_dump(char *data);
static int shell_write(char *data);
static int shell_i2cm_write(char *data);
static int shell_drv_debug(char *data);

int shell_main(void);

#endif

/* EOF */

