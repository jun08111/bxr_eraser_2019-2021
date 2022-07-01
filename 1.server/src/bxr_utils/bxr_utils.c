/*---------------------------------------------------------------------------*/
/* Name : bxr_utils.c                                                        */
/* Info : Blue X-ray G Utils Func                                            */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#include "bxrcommon.h"
#include "bxrutils.h"
#include "iniparser.h"

#define BXR_PGM		"Blue X-ray G Utils"                    /* Program Name	*/
#define BXR_VER		"1.01 (Thu Mar 19 09:47:54 KST 2020)"   /* Program Ver	*/

/*---------------------------------------------------------------------------*/
/* B_Log() - Blue X-ray Log Func                                             */
/*---------------------------------------------------------------------------*/
int B_Log(const char *logfile, int logflag, int logline, const char *fmt, ...)
{
	int fd, len;
	struct	timeval	t;
	struct tm *tm;
	static char	fname[128];
	static char sTmp[1024*2], sFlg[5];

	va_list ap;

//	syslog(LOG_INFO, "[BxrG] %s(), [%s/%d/%d]", __FUNCTION__, logfile, logflag, logline);
	if( LOGMODE < logflag ) return 0;

	switch( logflag )
	{
		case	0 :
		case	1 :
			sprintf( sFlg, "E" );
			break;
		case	2 :
			sprintf( sFlg, "I" );
			break;
		case	3 :
			sprintf( sFlg, "W" );
			break;
		case	4 :
		default   :
#ifndef _BXDBG
			return( BXR_RESULT_OK );
#endif
			sprintf( sFlg, "D" );

			break;
	}

	memset( sTmp, 0x00, sizeof(sTmp) );
	gettimeofday(&t, NULL);
	tm = localtime(&t.tv_sec);

	/* [HHMMSS ssssss flag __LINE__] */
	len = sprintf(sTmp, "[%5d:%08x/%02d%02d%02d %06d/%s:%4d:%4d]",
			getpid(), (unsigned int)pthread_self(),
			tm->tm_hour, tm->tm_min, tm->tm_sec, t.tv_usec, 
			sFlg, errno, logline );

	va_start(ap, fmt);
	vsprintf((char *)&sTmp[len], fmt, ap);
	va_end(ap);

	sprintf(fname, "%s/%s.%02d%02d", LOGPATH, logfile, tm->tm_mon+1, tm->tm_mday);
//	syslog(LOG_INFO, "[BxrG] %s(), logfile[%s]", __FUNCTION__, fname );
	if (access(fname, 0) != 0)
		fd = open(fname, O_WRONLY|O_CREAT, 0660);
	else
		fd = open(fname, O_WRONLY|O_APPEND, 0660);

	if (fd >= 0)
	{
//		syslog(LOG_INFO, "[BxrG] %s(), [%.*s]", __FUNCTION__, sTmp, strlen(sTmp));
		write(fd, sTmp, strlen(sTmp));
		close(fd);
	}

	return( BXR_RESULT_OK );

}/* End of B_Log() */

/*---------------------------------------------------------------------------*/
/* Blue X-ray G Init Function                                                */
/*---------------------------------------------------------------------------*/
int BXR_Init(char *cfgpath)
{
    struct stat sb;
	char	cfgfile[512];
	int		rtn, len;

	/* Blue X-ray G Profile Path Check */
	len = sprintf( cfgfile, "%s", cfgpath );
	len = sprintf( cfgfile+len, "/%s", "BlueXrayG.conf" );
	
	if((rtn=access(cfgfile, R_OK)) < 0 )
	{
		/* Blue X-ray G Profile Defautl Data Create */
		syslog(LOG_WARNING, "[BxrG] %s(), Conf File Not Found... '%s'", __FUNCTION__, cfgfile );
		return( BXR_RESULT_ERROR );
	}

    if( stat( cfgfile, &sb ) == FALSE )
	{
		syslog(LOG_ERR, "[BxrG] %s(), Conf File Check Error... '%s':(%d)", __FUNCTION__, cfgfile, errno);
		return( BXR_RESULT_ERROR );
	}

	/* 환경 파일 읽어 적용 */
	if( CONFTIME != sb.st_mtime )
	{
		syslog(LOG_INFO, "[BxrG] %s(), Load Conf File Start...", __FUNCTION__);

		if( (rtn = BXR_LoadConf( cfgfile )) == FALSE ) 
			return( BXR_RESULT_ERROR );
	}
	
	CONFTIME = sb.st_mtime;
		
	syslog(LOG_INFO, "[BxrG] %s(), Load Conf File End...", __FUNCTION__);

	/* Module Information */
	B_Log( INF, "INF >> Program : [%s]\n", BXR_program() );
	B_Log( INF, "INF >> Version : [%s]\n", BXR_version() );
	
    return( BXR_RESULT_OK );

}/* End of BXR_Init() */

/*---------------------------------------------------------------------------*/
/* load configure values form 'BlueXrayG.conf' file.                         */
/*---------------------------------------------------------------------------*/
int BXR_LoadConf( char *cfgfile )
{
	dictionary  *ini;
	const char	*s;

	ini = iniparser_load(cfgfile);
	if( ini == NULL )
	{
		syslog(LOG_ERR, "[BxrG] %s(), Conf File Load Error... '%s'", __FUNCTION__, cfgfile );
       	return( BXR_RESULT_ERROR );
	}

//#ifdef _BXDBG_INI
	iniparser_dump(ini, stdout);
//#endif

	/* Log File Path */
	s = iniparser_getstring(ini, "debug:logpath", NULL);
	sprintf( LOGPATH, "%s", s ? s : getenv("HOME") );
	//sprintf( LOGPATH, "%s", s ? s : "/var/log/bluexg" );

	/* Log printf mode */	
	LOGMODE = iniparser_getint(ini, "debug:logmode", 0);

	/* Blue X-ray G Server IP, PORT */
	s = iniparser_getstring(ini, "config:bxrgip", NULL);
	sprintf( SERV_IP, "%s", s ? s : "0.0.0.0" );
	SERV_PORT = iniparser_getint(ini, "config:bxrgport", 9500);

	/* Blue X-ray G DBMS IP, PORT */
	s = iniparser_getstring(ini, "config:bxrgip", NULL);
	sprintf( DBMS_IP, "%s", s ? s : "127.0.0.1" );
	DBMS_PORT = iniparser_getint(ini, "config:dbmsport", 3306);
	
	/* Log File Name */
	sprintf( LOGFILE, "%s.log", BXR_GetProcName() );
	syslog(LOG_INFO, "[BxrG] %s(), Log file... [%s]", __FUNCTION__, LOGFILE );

	iniparser_freedict(ini);

    return( BXR_RESULT_OK );

}/* End of BXR_LoadConf() */

/*---------------------------------------------------------------------------*/
/* Get Process name Function                                                 */
/*---------------------------------------------------------------------------*/
char *BXR_GetProcName()
{
    FILE *fp;
    static char pbuff[256];  
    char fname[128];  
    static char pname[128]; 
    int i;

    sprintf( pname, "BlueXrayG" );
    sprintf( fname, "/proc/%d/status", getpid() );  
    
    fp = fopen(fname, "r");  
    if(fp == NULL) 
    {   
		syslog(LOG_ERR, "[BxrG] %s(), Process Name Not Found... '%s'", 
				__FUNCTION__, fname );
		return( (char *)pname );  
    }   

    fgets(pbuff, sizeof(pbuff), fp);
    pbuff[strlen(pbuff)-1] = 0x00;
    for( i=5 ; i < strlen(pbuff) ; i++ )
    {   
        if( pbuff[i] >= 'a' && pbuff[i] <= 'z' ) break;
        if( pbuff[i] >= 'A' && pbuff[i] <= 'Z' ) break;
    }

    fclose(fp);  
        
    if( i < strlen(pbuff) )
		return( (char *)&pbuff[i] );
    else
		return( (char *)pname );  
    
}/* End of BXR_GetProcName() */

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
char *BXR_program()
{
    return( (char *)BXR_PGM );

}/* End of BXR_program() */

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
char *BXR_version()
{
    return( (char *)BXR_VER );

}/* End of BXR_version() */

/*---------------------------------------------------------------------------*/
/* Create Daemon Process Function                                            */
/*---------------------------------------------------------------------------*/
int	BXR_Daemonize()
{
	int pid;
 
	B_Log( INF, "Process Daemonize Start...\n" );
	
	// fork 생성
	pid = fork();
	B_Log( DBG, "pid = [%d] \n", pid);
 
	// fork 에러 발생 시 로그 남긴 후 종료
	if(pid < 0){
		printf("fork Error... : return is [%d] \n", pid );
		perror("fork error : ");
		exit(0);
	// 부모 프로세스 로그 남긴 후 종료
	}else if (pid > 0){
		printf("child process : [%d] - parent process : [%d] \n", pid, getpid());
		exit(0);
	// 정상 작동시 로그
	}else if(pid == 0){
		B_Log( INF, "process : [%d]\n", getpid());
	}
 
	// 터미널 종료 시 signal의 영향을 받지 않도록 처리
	signal(SIGHUP, SIG_IGN);
	close(0);
	close(1);
	close(2);
 
	// 실행위치를 Root로 변경
	chdir("/");
 
	// 새로운 세션 부여
	setsid();

	B_Log( INF, "Process Daemonize End...\n" );

    return( BXR_RESULT_OK );
	
}/* End of BXR_Daemonize() */

