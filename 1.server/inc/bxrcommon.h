/*---------------------------------------------------------------------------*/
/* Name : bxrcommon.h                                                        */
/* Info : Blue X-ray Common Header file                                      */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#ifndef BLUEXRAY_COMMON_H
#define BLUEXRAY_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>

#include "bxrerrno.h"

int LOGMODE;
/*- Log define ------------*/
#define	ERR	LOGFILE,1,__LINE__
#define	INF	LOGFILE,2,__LINE__
#define	WRN	LOGFILE,3,__LINE__
#define	DBG	LOGFILE,4,__LINE__

#endif

