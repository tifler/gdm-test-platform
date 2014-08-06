/*--------------------------------------------------------------------------
/	Project name	: 
/	Copyright		: Anapass, All rights reserved.
/	File name		: debug.c
/	Description		:
/
/	
/	
/	
/	Name		Date			Action
/	thlee		14.07.16		Create
---------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------------------------
													INCLUDE
 ------------------------------------------------------------------------------------------------------------------ */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>

#include <pthread.h>
#include "isp.h"
#include "define.h"

/* --------------------------------------------------------------------------------------------------------------------
													LOCAL CONSTANTS
 ------------------------------------------------------------------------------------------------------------------ */
#define	DEBUG_BLOCK		1
#define	DEBUG_NONBLOCK	0

#define	DEBUG_BUF_LENGTH	(1024*12)
/* --------------------------------------------------------------------------------------------------------------------
													LOCAL DATA TYPES
 ------------------------------------------------------------------------------------------------------------------ */
/* --------------------------------------------------------------------------------------------------------------------
													LOCAL FUNCTION PROTOTYPES
 ------------------------------------------------------------------------------------------------------------------ */
/* --------------------------------------------------------------------------------------------------------------------
													LOCAL VARIABLES
 ------------------------------------------------------------------------------------------------------------------ */
extern int 	gThreadState;

static	int	volatile debug_block;
static	int	volatile debug_buf_head, debug_buf_tail;
static	int volatile debug_output = 0;
static	char debug_buf[DEBUG_BUF_LENGTH];
static	char temp_str[512];
pthread_t	debugThreadHandle;
/* --------------------------------------------------------------------------------------------------------------------
													FUNCTIONS
 ------------------------------------------------------------------------------------------------------------------ */
void *DebugThread(void *data)
{
	int	head, tail, loop;

	while(gThreadState==THREAD_RUN)
	{
		if((debug_block==DEBUG_NONBLOCK)&&(debug_buf_head!=debug_buf_tail))
		{
			debug_output = 1;
			head = debug_buf_head;
			tail = debug_buf_tail;
			
			if(head >= tail)
			{
				while(head!=tail)
				{
					putchar(debug_buf[tail]);
					tail++;
				}
			}
			else
			{
				head = DEBUG_BUF_LENGTH-1;
				while(head!=tail)
				{
					putchar(debug_buf[tail]);
					tail++;
				}
				tail = 0;
			}

			debug_buf_tail = tail;
			debug_output = 0;
		}
		else
			usleep(50000);
	}
}



int debug_init(void)
{
	int 	result;

	debug_block	= DEBUG_NONBLOCK;
	debug_buf_head	= 0;
	debug_buf_tail	= 0;

	result = pthread_create(&debugThreadHandle, NULL, DebugThread, NULL);
	if(result < 0)
	{
		printf("Debug Thread Create Error\n");
		return -1;
	}
	pthread_detach(debugThreadHandle);
	
	return 0;
}

void isp_printf(char *fmt, ...)
{
	char	*s;
	float	f;
	int		i,j,lenfmt;
	va_list	ap;
	char	buf[64];
	int		length = 0, itemp;
	
	va_start(ap, fmt);
	lenfmt = strlen(fmt);

	for(j=0; j<lenfmt; j++)
	{
		if(fmt[j] != '%')
		{
			//putchar(fmt[j]);
			temp_str[length] = fmt[j];
			length++;
		}
		else
		{
			j++;
			if(fmt[j]=='d')
			{
				i = va_arg(ap, int);
				itemp = sprintf(&temp_str[length], "%d", i);
				if(itemp > 0)length += itemp;
			}
			else if(fmt[j]=='x')
			{
				i = va_arg(ap, int);
				itemp = sprintf(&temp_str[length], "%x", i);
				if(itemp > 0)length += itemp;

			}
			else if(fmt[j]=='s')
			{
				s = va_arg(ap, char*);
				while(*s)
				{
					//putchar(*s++);
					temp_str[length] = *s++;
					length++;
				}
			}
			else
			{
				//putchar(fmt[j]);
				temp_str[length] = fmt[j];
				length++;
			}
		}
	}

	// printf
	//for(i=0; i<length; i++)putchar(temp_str[i]);
	debug_block = DEBUG_BLOCK;
	i = debug_buf_head;
	j = debug_buf_tail;
	if((i == j)&&(debug_output==0))
	{
		debug_buf_head = 0;
		debug_buf_tail = 0;
		i = 0;
		j = 0;
	}
	// remain size calc
	if(i >= j)lenfmt = DEBUG_BUF_LENGTH - (i-j)-1;
	else lenfmt = j - i;

	if(length > lenfmt)return;	// loss message(buf full)
	
	if((i+length) >= DEBUG_BUF_LENGTH)
	{
		j = DEBUG_BUF_LENGTH - (i+length) - 1;
		memcpy(&debug_buf[debug_buf_head], temp_str, j);
		memcpy(debug_buf, &temp_str[j], length-j);
		debug_buf_head = length-j;
	}
	else
	{
		memcpy(&debug_buf[debug_buf_head], temp_str, length);
		debug_buf_head += length;
	}

	debug_block = DEBUG_NONBLOCK;
}

/* EOF */

