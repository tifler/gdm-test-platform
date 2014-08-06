/*--------------------------------------------------------------------------
/	Project name	: 
/	Copyright		: Anapass, All rights reserved.
/	File name		: shell.c
/	Description		:
/
/	
/	
/	
/	Name		Date			Action
/	thlee		14.06.18		Create
---------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------------------------
													INCLUDE
 ------------------------------------------------------------------------------------------------------------------ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/ioctl.h>
#include "shell.h"
#include "isp.h"
#include "ioctl.h"

/* --------------------------------------------------------------------------------------------------------------------
													LOCAL CONSTANTS
 ------------------------------------------------------------------------------------------------------------------ */
#define	KEY_BUFFER_LENGTH	80

#define KEY_ENTER	0x0a
#define KEY_SPACE	0x20
#define KEY_BACKSP	0x08

/* --------------------------------------------------------------------------------------------------------------------
													LOCAL DATA TYPES
 ------------------------------------------------------------------------------------------------------------------ */
/* --------------------------------------------------------------------------------------------------------------------
													LOCAL FUNCTION PROTOTYPES
 ------------------------------------------------------------------------------------------------------------------ */
extern int shell_isp_init(char *data);
extern int shell_isp_geter(char *data);
extern int shell_isp_vsensorstart(char *data);

/* --------------------------------------------------------------------------------------------------------------------
													LOCAL VARIABLES
 ------------------------------------------------------------------------------------------------------------------ */
static shell_cmd gshell[] = 
{
	{"quit",	"quit",				&shell_quit},
	{"help",	"help",				&shell_help},
	{"dump",	"dump data",		&shell_dump},
	{"write",	"write data",		&shell_write},
	{"isp",		"isp init",			&shell_isp_init},
	{"ispdump",		"isp get dump",			&shell_isp_geter},
	{"i2cmw",	"i2cm write",		&shell_i2cm_write},
	{"drvdebug",	"isp driver debug on/off",	&shell_drv_debug},
	{"post",	"post event",	&shell_isp_vsensorstart},
	{NULL,		NULL,		NULL},
};

int		gKeyIndex = 0;
char	gKeyBuf[KEY_BUFFER_LENGTH];


/* --------------------------------------------------------------------------------------------------------------------
													FUNCTIONS
 ------------------------------------------------------------------------------------------------------------------ */
int str2int(const char *str, int multi)
{
	int	index =0;
	int result=0;
	int	digit = 1;

	// find end
	while(1)
	{
		if(*(str+index)==0)break;
		index++;
	}

	if(index<=0)return 0;
	index--;

	while(1)
	{
		if((*(str+index)>='0')&&(*(str+index)<='9'))
		{
			result += ((*(str+index)-'0')*digit);
			digit *= multi;
		}
		else if((*(str+index)>='a')&&(*(str+index)<='f')&&(multi==16))
		{
			result += ((*(str+index)-'a'+10)*digit);
			digit *= multi;
		}
		else if((*(str+index)>='A')&&(*(str+index)<='F')&&(multi==16))
		{
			result += ((*(str+index)-'A'+10)*digit);
			digit *= multi;
		}
		else
			return -1;	// error

		index--;
		if(index < 0)return result;
	}

	return 0;
}

int findargument(int index)
{
	int index_result;

	index_result = index;

	while(1)
	{
		if((gKeyBuf[index_result]==0)&&(gKeyBuf[index_result+1]!=0))
		{
			index_result++;
			return index_result;
		}
		index_result++;
		if(index_result >= KEY_BUFFER_LENGTH)return -1;
	}

	return 0;
}

static int shell_quit(char *data)
{
	return SHELL_RETURN_EXIT;
}

static int shell_help(char *data)
{
	int		loop = 0;

	printf("%16s, %16s\n", "Command", "Description");
	while(1)
	{
		if(gshell[loop].cmd==NULL)break;
		printf("%16s, %16s\n", gshell[loop].cmd, gshell[loop].help);
		loop++;
	}

	return SHELL_RETURN_OK;
}

static int shell_dump(char *data)
{
#if	0
	int		index;
	int		addr;
	int		loop;

	index = findargument(3);
	if(index < 0)return SHELL_RETURN_ERROR;

	if((gKeyBuf[index]=='0')&&(gKeyBuf[index+1]=='x'))
	{
		addr = str2int(&gKeyBuf[index+2], 16);
		printf("addr 0x%x\n", addr);

		// read part
		for(loop=0; loop<16; loop++)
		{
			index = isp_RegRead(addr+(loop*4));
			printf("0x%8x, ", index);
			if((loop!=0)&&((loop%4)==0))printf("\n");
		}
	}
	else
		return SHELL_RETURN_ERROR;
#endif
	return SHELL_RETURN_OK;
}

static int shell_write(char *data)
{
	int		index;
	int		addr, wdata;

	index = findargument(4);
	if(index < 0)return SHELL_RETURN_ERROR;

	if((gKeyBuf[index]=='0')&&(gKeyBuf[index+1]=='x'))
	{
		addr = str2int(&gKeyBuf[index+2], 16);
		
		index = findargument(index+2);
		if(index < 0)return SHELL_RETURN_ERROR;
		if((gKeyBuf[index]=='0')&&(gKeyBuf[index+1]=='x'))
		{
			wdata = str2int(&gKeyBuf[index+2], 16);
		}
		else
			return SHELL_RETURN_ERROR;

		printf("addr 0x%x, data 0x%x\n", addr, wdata);
		// write part
		isp_setRegister(addr, wdata);
	}
	else
		return SHELL_RETURN_ERROR;

	return SHELL_RETURN_OK;
}

static int shell_i2cm_write(char *data)
{
	char	buf[32];
	ioctl_i2cm_info	*pi2cm_info;
	int		result;

	pi2cm_info = (ioctl_i2cm_info *)buf;

	pi2cm_info->id		= 0x6c;
	pi2cm_info->addr	= 0x3103;
	pi2cm_info->length	= 1;

	buf[sizeof(ioctl_i2cm_info)]	= 0x93;	// data

	//ioctl(gIspDev, ISP_IOCTL_I2CM_WRITE, buf);
	isp_cmd_write(ISP_I2C_IOC_WRITE, (uint32_t)buf);

	//isp_cmd_write(ISP_IOCTL_INTGEN, 0);

	DRIVER_MSG_ON;

	result = DxOISP_getRegister(0xd700);
	printf("dxo 0x0d700 value 0x%x\n", result);

	return SHELL_RETURN_OK;
}

static int shell_drv_debug(char *data)
{
	int		flag;

	flag = str2int(data, 10);
	printf("flag is %d\n", flag);

	//ioctl(gIspDev, ISP_IOCTL_DEBUG, flag);
	if(flag==0)DRIVER_MSG_OFF;
	else DRIVER_MSG_ON;

	return SHELL_RETURN_OK;
}

int shell_main(void)
{
	int		ch;
	int		loop, index;

	ch = getchar();
	if(ch == KEY_SPACE)ch = 0;
	gKeyBuf[gKeyIndex] = (char)ch;
	gKeyIndex++;
	
	// check length
	if(gKeyIndex==KEY_BUFFER_LENGTH)
	{
		printf("\n Key input overwrite\n");
		gKeyIndex = 0;
		return SHELL_RETURN_ERROR;
	}

	// check enter
	if(ch == KEY_ENTER)
	{
		gKeyIndex--;
		gKeyBuf[gKeyIndex] = 0;	// remove 0x0a

		// check cmd
		loop = 0;
		while(1)
		{
			if(gshell[loop].cmd == NULL)break;

			if(strcmp(gKeyBuf, gshell[loop].cmd) == 0)
			{
				index = findargument(0);
				ch = (*gshell[loop].pfunc)(&gKeyBuf[index]);
				gKeyIndex = 0;
				return ch;
			}

			loop++;
		}

		if(gKeyIndex != 0)printf("wrong cmd input\n");
		gKeyIndex = 0;
	}
	else if(ch == KEY_BACKSP)
	{
		if(gKeyIndex != 0)
		{
			gKeyIndex--;
		}
	}

	return SHELL_RETURN_OK;
}

/* EOF */

