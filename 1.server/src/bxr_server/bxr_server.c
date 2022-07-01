/*---------------------------------------------------------------------------*/
/* Name : bxr_sertner.c                                                       */
/* Info : Blue X-ray Log Sertner                                              */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#include "bxrcommon.h"
#include "bxrutils.h"
#include "bxrs.h"

int Log_DBMS_Insert(char *, int );

/*---------------------------------------------------------------------------*/
/* Main()                                                                    */
/*---------------------------------------------------------------------------*/
void main()
{
	nng_socket	sock;
	char		SERV_URL[  64];
	char		PATH_CFG[1024];
	int			rtn;

	if( getenv("BXRG_HOME") == NULL )
	{
		syslog(LOG_WARNING, "[BxrG] $(BXRG_HOME) Not Found... " );
		return;
	}

	sprintf( PATH_CFG, "%s/conf", getenv("BXRG_HOME") );
	BXR_Init( PATH_CFG );
	B_Log( INF, "BXR_Init End...\n" );
	
	BXR_Daemonize();
	B_Log( INF, "BXR_Daemonize End...\n" );


	if ((rtn = nng_rep0_open(&sock)) != 0) 
	{
		B_Log( ERR, "nng_open : [%s]\n", nng_strerror(rtn));
		return;
	}

	sprintf( SERV_URL, "tcp://%s:%d", SERV_IP, SERV_PORT );
	if ((rtn = nng_listen(sock, SERV_URL, NULL, 0)) != 0) 
	{
		B_Log( ERR, "nng_listen : [%s][%s]\n", SERV_URL, nng_strerror(rtn));
		return;
	}

	B_Log( DBG, "SERVER: RECV start\n" );

	for (;;) 
	{
		char *   rbuf = NULL;
		size_t   sz;
		uint64_t val;
	
		B_Log( DBG, "SERVER: RECV Wait...\n" );
		if ((rtn = nng_recv(sock, &rbuf, &sz, NNG_FLAG_ALLOC)) != 0) 
		{
			B_Log( ERR, "nng_recv : [%s]\n", nng_strerror(rtn));
			nng_free(rbuf, sz);
			break;
		}

		B_Log( DBG, "SERVER: RECV DATA : sz[%d] [%.*s]\n", sz, sz, rbuf);

		if (sz != 0 ) 
		{
			/* Recv Log DBMS Insert */
			if( Log_DBMS_Insert(rbuf, sz) == FALSE )
				sprintf( rbuf, " \"etype\": \"%c\", \"ecode\": \"%03d\" ", 'E', 1 );
			else
				sprintf( rbuf, " \"etype\": \"%c\", \"ecode\": \"%03d\" ", 'S', 0 );

			B_Log( DBG, "SERVER: SEND DATA : [%d][%s]\n", strlen(rbuf), rbuf);
			
			rtn = nng_send(sock, rbuf, strlen(rbuf), NNG_FLAG_ALLOC);
			if (rtn != 0) 
			{
				B_Log( ERR, "nng_send : [%s]\n", nng_strerror(rtn));
				nng_free(rbuf, sz);
				break;
			}

			B_Log( DBG, "SERVER: SEND E n d : [%d]\n", rtn );
		
			continue;
		}
	
		// Unrecognized command, so toss the buffer.
		nng_free(rbuf, sz);
	}
	
	return;

}/* End of Main() */


/*---------------------------------------------------------------------------*/
/* Main()                                                                    */
/*---------------------------------------------------------------------------*/
int Log_DBMS_Insert(char *LogMsg, int LogMsgLen)
{
	MYSQL  *DBConn;
	char	SQL_Buf[20480];

	DBConn = mysql_init(NULL);
	if( mysql_real_connect( DBConn, DBMS_IP, DBMS_USER, DBMS_PASS, DBMS_NAME, DBMS_PORT, NULL, 0) == NULL)
	{
		B_Log( ERR, "The authentication failed with the following message:[%s]\n", mysql_error(DBConn));
		return( FALSE );
	}

	mysql_set_character_set( DBConn, "utf8");

	// Construct the SQL statement
	sprintf(SQL_Buf, 
			"INSERT INTO `bluexg_v1`.`tb_log_event_law` (`state`,`cDate`,`iData`) VALUES('N', now(3), '%.*s' )", 
			LogMsgLen, LogMsg );

	if( mysql_query( DBConn, SQL_Buf ) != 0 )
	{
		B_Log( ERR, "Query failed  with the following message:[%s]\n", mysql_error(DBConn));
		return( FALSE );
	}

	B_Log( DBG, "Log inserted into the database success...\n" );

	mysql_close(DBConn);

	return( TRUE );

}/* End of Log_DBMS_Insert() */

