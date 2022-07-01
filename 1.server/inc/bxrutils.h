/******************************************************************************/
/*  Header      : bxrerrno.h                                                  */
/*  Description : Common Error Code Header                                    */
/*  Rev. History: Ver   Date    Description                                   */
/*                ----  ------- ----------------------------------------------*/
/*                0.5   2020-06 Initial version                               */
/******************************************************************************/
#ifndef BLUEXRAY_UTILS_H
#define BLUEXRAY_UTILS_H

#include <pthread.h>
#include <syslog.h>

/* Config Ini Define */
int		LOGMODE; 						/* 0:Not, 1:Debug, 2:WRN, 3:ERR */
char 	LOGPATH		[2048];
char 	LOGFILE		[2048];
char 	SERV_IP		[  32];
int	 	SERV_PORT;
char 	DBMS_IP		[  32];
int	 	DBMS_PORT;
time_t	CONFTIME;

int		B_Log(const char *, int , int , const char *, ...);
int		BXR_Init(char *);
int		BXR_LoadConf();
char	*BXR_program();
char	*BXR_version();
int		BXR_Daemonize();
char	*BXR_GetProcName();
int		BXR_REQ( char *, char *, int );
#endif

