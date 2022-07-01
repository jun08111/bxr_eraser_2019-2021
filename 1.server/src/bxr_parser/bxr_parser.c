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
	MYSQL *		DBConn;
    MYSQL_ROW   Sql_Row;
    MYSQL_RES *	Sql_Rtn; 
	char		SERV_URL[   64];
	char		PATH_CFG[ 1024];
	char		SQL_Buf	[20480];
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

	DBConn = mysql_init(NULL);
	if( mysql_real_connect( DBConn, DBMS_IP, DBMS_USER, DBMS_PASS, DBMS_NAME, DBMS_PORT, NULL, 0) == NULL)
	{
		B_Log( ERR, "The authentication failed with the following message:[%s]\n", mysql_error(DBConn));
		return;
	}
	
	mysql_set_character_set( DBConn, "utf8");

	for (;;) 
	{
		if( mysql_query(DBConn, "SELECT idx, state, cDate, iData FROM bluexg_v1.tb_log_event_law \
									WHERE state = 'N' AND IFNULL(rCount,0) < 5 ORDER BY cDate;") != 0)
		{
			B_Log( ERR, "Mysql query error : [%s]\n", mysql_error(DBConn));
			return;
		}
		
		Sql_Rtn = mysql_store_result(DBConn);
		
		while ( (Sql_Row = mysql_fetch_row(Sql_Rtn)) != NULL )
		{
			B_Log( DBG, "idx[%3d] flag[%s] cdate[%s]\n", atoi(Sql_Row[0]), Sql_Row[1], Sql_Row[2] );

			/* Recv Log DBMS Insert */
			if( Log_Task_Trans(DBConn, Sql_Row) == FALSE )
				B_Log( ERR, "Log_DBMS_Insert() Error...\n" );
			else
				B_Log( DBG, "Log_DBMS_Insert() End...\n" );

			usleep(1000);
		}
			
		B_Log( DBG, "Log Paser Complet...\n");

		mysql_free_result(Sql_Rtn);

		sleep(3);
		continue;
	}
	
	return;

}/* End of Main() */


/*---------------------------------------------------------------------------*/
/* Log_Task_Trans()                                                         */
/*---------------------------------------------------------------------------*/
int Log_Task_Trans(MYSQL *DBConn, MYSQL_ROW Sql_Row)
{
    MYSQL_RES *	Sql_Rtn; 
	char		Sql_Buf[1024*10];
	int			Sql_Len;
    int       query_stat; 

	json_object *Req_Obj, *Req_Obj_Data, *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;

    Req_Obj = json_tokener_parse(Sql_Row[3]);
	B_Log( DBG, "Req_Obj from str:\n---\n%s\n---\n", 
            json_object_to_json_string_ext( Req_Obj, 
                                            JSON_C_TO_STRING_SPACED | 
                                            JSON_C_TO_STRING_PRETTY ) );

	Sql_Len = sprintf( Sql_Buf, "INSERT INTO `bluexg_v1`.`tb_log_event_detail` ( `cDate`, `pIdx`, \
		 `appTo`,           \
		 `trFunction`,      \
		 `accessToken`,     \
		 `appFrom`,         \
		 `clientId`,        \
		 `logType`,         \
		 `fileName`,        \
		 `fileSize`,        \
		 `filePath`,        \
		 `logWork`,         \
		 `eCode`,           \
		 `logStat`,         \
		 `pDate`,           \
		 `Code`,         \
		 `Mesg`) ");
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "VALUES ( now(3), %d, ", atoi(Sql_Row[0]) );

	/* Req Head */
	json_object_object_get_ex( Req_Obj, "to",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex( Req_Obj, "function",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex( Req_Obj, "access_token",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex( Req_Obj, "from",     &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", Val_Str );

	/* Req Data */
	json_object_object_get_ex( Req_Obj, "params",     &Req_Obj_Data );
	
	if( strcmp( Val_Str, "kr.gooroom.joeunins.tracer" ) == 0 )
	{
		if( Log_Parser_Eraser( Sql_Buf, &Sql_Len, Req_Obj_Data ) == FALSE )
			B_Log( ERR, "Log_Parser_Tracer error... \n" );
	}
	else
	if( strcmp( Val_Str, "kr.gooroom.joeunins.eraser" ) == 0 )
	{
		if( Log_Parser_Eraser( Sql_Buf, &Sql_Len, Req_Obj_Data ) == FALSE )
			B_Log( ERR, "Log_Parser_Eraser error... \n" );
	}
	else
	if( strcmp( Val_Str, "kr.gooroom.softcamp.mv" ) == 0 )
	{
		if( Log_Parser_Antivirus( Sql_Buf, &Sql_Len, Req_Obj_Data ) == FALSE )
			B_Log( ERR, "Log_Parser_Antivirus error... \n" );
	}
	else
	{
		if( Log_Parser_DRM( Sql_Buf, &Sql_Len, Req_Obj_Data ) == FALSE )
			B_Log( ERR, "Log_Parser_DRM error... \n" );
	}

	Sql_Len += sprintf( Sql_Buf+Sql_Len, " )");

	/* Event Log Insert */
	B_Log( DBG, "Sql:[%d][%s]\n", Sql_Len, Sql_Buf );
    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }

	/* Raw Event Log Update */
    Sql_Len = sprintf(Sql_Buf, "UPDATE `bluexg_v1`.`tb_log_event_law` \
						SET		`state` = 'C', \
								`uDate` = now(3) \
						WHERE	`idx` = %s AND `state` = 'N' AND `cDate` = '%s'", 
								Sql_Row[0], Sql_Row[2] );

	B_Log( DBG, "Sql:[%d][%s]\n", Sql_Len, Sql_Buf );
    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }

	return( TRUE );

}/* End of Log_Task_Trans() */


/*---------------------------------------------------------------------------*/
/* Log_Parser_Eraser()                                                       */
/*---------------------------------------------------------------------------*/
int Log_Parser_Eraser( char *Sql_Buf, int *Sql_Len, json_object *Req_Obj_Data )
{
	// json request message parsing
	json_object *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;
	char	   *Def_Str = "-";
	
	json_object_object_get_ex( Req_Obj_Data, "clientId", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );
	
	B_Log( DBG, "Val_Str:[%s]\n", Val_Str );

	json_object_object_get_ex (Req_Obj_Data, "logType", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "fileName", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );

	json_object_object_get_ex (Req_Obj_Data, "fileSize", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "%d, ", json_object_get_int(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "filePath", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );
	
	json_object_object_get_ex (Req_Obj_Data, "logWork", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "eCode", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "logInfo", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "pDate", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "Code", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "%d, ", json_object_get_int(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "Mesg", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s' ", Val_Str != NULL ? Val_Str : Def_Str );

	return( TRUE );

}/* End of Log_Parser_Eraser() */

/*---------------------------------------------------------------------------*/
/* Log_Parser_Antivirus()                                                    */
/*---------------------------------------------------------------------------*/
int Log_Parser_Antivirus( char *Sql_Buf, int *Sql_Len, json_object *Req_Obj_Data )
{
	// json request message parsing
	json_object *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;
	char	   *Def_Str = "-";
	
	json_object_object_get_ex( Req_Obj_Data, "clientId", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );
	
	B_Log( DBG, "Val_Str:[%s]\n", Val_Str );

	json_object_object_get_ex (Req_Obj_Data, "logType", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "fileName", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );

	json_object_object_get_ex (Req_Obj_Data, "fileSize", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "%d, ", json_object_get_int(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "filePath", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );
	
	json_object_object_get_ex (Req_Obj_Data, "logWork", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "eCode", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "logInfo", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "pDate", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "Code", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "%d, ", json_object_get_int(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "Mesg", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s' ", Val_Str != NULL ? Val_Str : Def_Str );

	return( TRUE );

}/* End of Log_Parser_Antivirus() */

/*---------------------------------------------------------------------------*/
/* Log_Parser_DRM()                                                          */
/*---------------------------------------------------------------------------*/
int Log_Parser_DRM( char *Sql_Buf, int *Sql_Len, json_object *Req_Obj_Data )
{
	// json request message parsing
	json_object *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;
	char	   *Def_Str = "-";
	
	json_object_object_get_ex( Req_Obj_Data, "clientId", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );
	
	B_Log( DBG, "Val_Str:[%s]\n", Val_Str );

	json_object_object_get_ex (Req_Obj_Data, "logType", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "fileName", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );

	json_object_object_get_ex (Req_Obj_Data, "fileSize", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "%d, ", json_object_get_int(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "filePath", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", Val_Str != NULL ? Val_Str : Def_Str );
	
	json_object_object_get_ex (Req_Obj_Data, "logWork", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "eCode", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex (Req_Obj_Data, "logInfo", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "pDate", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "Code", &Val_Obj );
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "%d, ", json_object_get_int(Val_Obj));

	json_object_object_get_ex (Req_Obj_Data, "Mesg", &Val_Obj );
	Val_Str = json_object_get_string(Val_Obj);
	*Sql_Len += sprintf( Sql_Buf+*Sql_Len, "'%s' ", Val_Str != NULL ? Val_Str : Def_Str );

	return( TRUE );

}/* End of Log_Parser_DRM() */

