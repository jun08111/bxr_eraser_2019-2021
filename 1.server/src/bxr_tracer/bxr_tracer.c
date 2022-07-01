/*---------------------------------------------------------------------------*/
/* Name : bxr_tracer.c                                                       */
/* Info : Blue X-ray Log Tracer Client                                       */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#include "bxrcommon.h"
#include "bxrutils.h"
#include "bxrc.h"

int	BXR_NNG_REQ( char *, char * );
char *bus_recv_handler(const char *);
/*---------------------------------------------------------------------------*/
/* Main()                                                                    */
/*---------------------------------------------------------------------------*/
int main()
{
    int auth_result;
    int run_result;
	int rtn;

    user_data_t app_data;
   
	memset(&app_data, 0x00, sizeof(user_data_t));

	BXR_Init("/etc");
	B_Log( INF, "BXR_Init End...\n" );
	
	BXR_Daemonize();
	B_Log( INF, "BXR_Daemonize End...\n" );

	B_Log( INF, "bxr_tracer Start...\n" );
   
	rtn  = auth(&app_data, TRACER);
    if( rtn != AUTH_STAT_OK ) 
	{
		B_Log( ERR, "auth Error : [%d]\n", rtn );
		return( 0 );
	}

	rtn = recv_run(recv_handler, &app_data,
									HANDLER_OPT_RETURN_STRING_FREE);
    if( rtn != AUTH_STAT_OK ) 
	{
		B_Log( ERR, "recv_run error : [%d]\n", rtn );
		return( 0 );
	}
 
    while(1) 
	{
        sleep(30);
		// 추후 프로세서 종료 시그널을 받아 종료 되도록 수정 필요
		B_Log( DBG, "bxr_tracer sleep...\n" );
    }

    return( 0 );

}/* End of Main() */

/*---------------------------------------------------------------------------*/
/* Main()                                                                    */
/*---------------------------------------------------------------------------*/
char *bus_recv_handler(const char *Req_Msg)
{ 
    char *Rep_Msg = malloc(2048);

	// json request message parsing
	json_object *Req_Obj, *Rep_Obj, *Val_Obj, *Rtn_Obj;
    const char *Req_From_Val, *Req_To_Val, *Req_Func_Val;
	
	Req_Obj = json_tokener_parse(Req_Msg);
	B_Log( DBG, "Req_Obj from str:\n---\n%s\n---\n", 
			json_object_to_json_string_ext( Req_Obj, 
											JSON_C_TO_STRING_SPACED | 
											JSON_C_TO_STRING_PRETTY ) );

    json_object_object_get_ex( Req_Obj, "from",     &Val_Obj );
	Req_From_Val = json_object_get_string(Val_Obj);
    
	json_object_object_get_ex( Req_Obj, "to",       &Val_Obj );
	Req_To_Val = json_object_get_string(Val_Obj);
    
	json_object_object_get_ex( Req_Obj, "function", &Val_Obj );
	Req_Func_Val = json_object_get_string(Val_Obj);
    
	// Blue X-ray G Sertner Req & Rep 
	BXR_NNG_REQ( "BXR_TRACER", (char *)json_object_to_json_string_ext( Req_Obj, 
											JSON_C_TO_STRING_SPACED | 
											JSON_C_TO_STRING_PRETTY ));

    Rep_Obj = json_object_new_object();
    Rtn_Obj = json_object_new_object();

    /* Header { ..., } 영역 */
    json_object_object_add(Rep_Obj, "from",		json_object_new_string(Req_To_Val));
    json_object_object_add(Rep_Obj, "to",		json_object_new_string(Req_From_Val));
    json_object_object_add(Rep_Obj, "function", json_object_new_string(Req_Func_Val));

    /* Data { ..., } 영역 */
    json_object_object_add(Rtn_Obj, "etype",	json_object_new_string("S"));
    json_object_object_add(Rtn_Obj, "ecode",	json_object_new_string("000"));
    json_object_object_add(Rtn_Obj, "emesg",	json_object_new_string("Success"));

    /* Data 영역 Header 등록 */
	json_object_object_add(Rep_Obj, "return",	Rtn_Obj);
 
    memset( Rep_Msg, 0x00, sizeof(Rep_Msg) );

	B_Log( DBG, "Rep_Obj from str:\n---\n%s\n---\n", 
			json_object_to_json_string_ext( Rep_Obj, 
											JSON_C_TO_STRING_SPACED | 
											JSON_C_TO_STRING_PRETTY ) );
    sprintf(Rep_Msg, "%s",
			json_object_to_json_string_ext( Rep_Obj, 
											JSON_C_TO_STRING_SPACED | 
											JSON_C_TO_STRING_PRETTY ) );

    return( Rep_Msg );

}/* End of bus_recv_handler() */


/*---------------------------------------------------------------------------*/
/* BXR_NNG_REQ() :                                                           */
/*---------------------------------------------------------------------------*/
int	BXR_NNG_REQ( char *TRNM, char *Req_Msg)
{
	nng_socket sock;
	int        rtn;
	char *     Rep_Msg = NULL;
	size_t     Rep_Len;
	uint8_t    sbuf[1024];

	if ((rtn = nng_req0_open(&sock)) != 0) {
		B_Log( ERR, "nng_socket :[%s]\n", nng_strerror(rtn));
	}

	B_Log( DBG, "Server Info : [%s:%d]\n", SERV_IP, SERV_PORT );
	sprintf( sbuf, "tcp://%s:%d", SERV_IP, SERV_PORT );
	B_Log( DBG, "Server Info : [%s]\n", sbuf );
	if ((rtn = nng_dial(sock, sbuf, NULL, 0)) != 0) {
//	if ((rtn = nng_dial(sock, "tcp://172.16.195.101:9500", NULL, 0)) != 0) {
		B_Log( ERR, "nng_dial :[%s]\n", nng_strerror(rtn));
	}

	B_Log( DBG, "CLIENT: SENDING To Server Start : [%d]\n---\n%.*s\n---\n", 
			strlen(Req_Msg), strlen(Req_Msg), Req_Msg );

	if ((rtn = nng_send(sock, Req_Msg, strlen(Req_Msg), 0)) != 0) {
		B_Log( ERR, "nng_send :[%s]\n", nng_strerror(rtn));
	}
	
	B_Log( DBG, "CLIENT: SENDING To Server E n d\n" );

	if ((rtn = nng_recv(sock, &Rep_Msg, &Rep_Len, NNG_FLAG_ALLOC)) != 0) {
		B_Log( ERR, "nng_recv :[%s]\n", nng_strerror(rtn));
	}

	B_Log( DBG, "CLIENT: RECEIVED DATE:[%s]\n", Rep_Msg);

	// This assumes that buf is ASCIIZ (zero terminated).
	nng_free(Rep_Msg, Rep_Len);
	nng_close(sock);
	
	return (0);

}/* End of BXR_NNG_REQ() */

