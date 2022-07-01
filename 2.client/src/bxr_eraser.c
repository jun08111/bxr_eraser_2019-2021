/******************************************************************************/
/*  Source        : eraser.c                                          																						*/
/*  Description :                                                           																				  */
/*  Rev. History: Ver      Date    	 Description                              							    									 */
/*  --------------   -----  -----------   -----------------------------------------------------------------------------------------*/
/*                		  1.2   2020-04  Initial version                              															      */
/******************************************************************************/

#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <sqlite3.h>
#include <errno.h>

//#include "bxrcommon.h"
//#include "bxrutils.h"



#define	PROGRESS_SIZE		102400	//100k
#define	ERASER_SIZE					  512 //1k
#define	ERASER_ENC_SIZE			 896 //1k

/*- Log define ------------*/
char	*LogName;
#define LOGFILE LogName
#define ERR LOGFILE,	   1,__LINE__
#define INF LOGFILE,		2,__LINE__
#define WRN LOGFILE,	 3,__LINE__
#define DBG LOGFILE,	  4,__LINE__

#define DEF "./bxre.log",  1,__LINE__

/*- Glade data----------------*/
#define GLADE "/usr/share/icons/eraser/eraser.glade"

/*- DB data----------------*/
//#define DBFILE "/home/joeun/.bxrG/bxre.data"		// "DB File 위치"
#define DBFILE "/.bxrG/bxre.data"		// "DB File 위치"
#define CHKTRIM "/.bxrG/trim.txt"

#define MAX_CNT           10000					 //  LogViewer의 최대 행 개수
#define MAX_PATH_SIZE   4096				 // 경로 최대 길이

#pragma pack (push, 1)
typedef struct _SQLite_Data
{
    char	db_num		 [3];
    char	db_name [255];
    char	db_work    [10];
	char	db_ecode     [5];
    char	db_stat	   	 [20];
    char	db_time	  	[35];
    char	db_size		 [97];
    char	db_loca	   [255];
	char	db_lcode      [7];
	char    db		    [45];
}SQLite_Data;
SQLite_Data dBl[MAX_CNT];
#pragma pack (pop)

user_data_t app_data;
int 	chk_method;							// 삭제 종류 0: 임시파일
int     auth_cnt;								// 재 인증 시도 횟수
int     auth_result;							// 인증 결과
int		temptotal;								// 임시파일 전체크기
char *data_path;
static int	colcnt  = 1;						// 컬럼수 count

GtkBuilder	 					 *builder; 

GtkWidget						*window,
										 *window_about,
					 					 *window_logviewer,
										 *msdialog,
										 *window_start;

GtkWidget 						*g_lbl_status,
										 *g_lbl_file,
										 *g_bar_progress;

GtkWidget 						*start_lbl,
										 *msd_lbl;

GtkWidget						*view;

GtkScrolledWindow		 *logviewer_scrolledwindow;

GtkTreeStore					*treestore;

GtkTreeIter							logviewer_iter;
GtkTreeModel			  	  *model;

void func_refresh_progressbar();
void func_refresh_scrolledwindow();

void on_window_main_destroy();
void on_smn_open_activate 			  (GtkMenuItem *smn_open,			  gpointer *data);
void on_smn_quit_activate 			 	(GtkMenuItem *smn_quit,   			  gpointer *data);
void on_smn_log_activate 				(GtkMenuItem *smn_log, 				   gpointer *data);
void on_smn_about_activate 			  (GtkMenuItem *smn_about, 			  gpointer *data); 

void on_btn_fileopen_clicked 			(GtkButton *btn_fileopen, 	 			gpointer *data);
void on_btn_setting_clicked				 (GtkButton *btn_setting, 	 			  gpointer *data);
void on_btn_about_ok_clicked 		  (GtkButton *btn_about_ok,  		    gpointer *data); 
void on_logviewer_clear_clicked 	   (GtkButton *btn_logviewer_clear,	 gpointer *data); 
void on_logviewer_close_clicked		   (GtkButton *btn_logviewer_close,  gpointer *data); 
static GtkTreeModel *create_and_fill_model (void);

void func_get_num_work    (int colcnt, char *stat);
void func_get_ecode_stat_time    (char *ecode, char *stat);
void func_get_fname_fsize_floca (char filename[]);
int func_gtk_dialog_modal           (int type, GtkWidget *window, char *message);
int func_file_eraser 			            (int type, char *filename);

int BXLog (const char *, int , int , const char *, ...);
int callback(void *, int, char **, char **);


/*---------------------------------------------------------------------------*/
/* Check Home Path    														         */
/*---------------------------------------------------------------------------*/
int func_get_homepath()
{
	data_path = getenv("HOME");
	if (data_path == NULL)
	{
		BXLog (ERR, "%-30s	\n", "GET HOMEPATH ERROR");
	}
	return 0;
}/* func_get_homepath() */


/*---------------------------------------------------------------------------*/
/* Check Client ID    														         */
/*---------------------------------------------------------------------------*/
int func_chk_clientid()
{
	int cidleng = 0;

	if ((cidleng = strlen (app_data.client_id)) == 36)
	{
		BXLog (DBG, "%-30s	[%s]\n", "CHECK CLIENT_ID", app_data.client_id);
	}
	else
	{
		BXLog (ERR, "%-30s	[%s][%d]\n", "CLIENT_ID LENGTH ERROR", app_data.client_id, cidleng);

	}

	return 0;
}/* func_chk_clientid() */


/*---------------------------------------------------------------------------*/
/* Check PID     														           	        */
/*---------------------------------------------------------------------------*/
int func_chk_psname()
{
	FILE *fp;
	char buff[256] = {0, };
	char *buff_pid = {"/usr/bin/bxr_eraser"};
	int cnt = 0;
	
	fp = popen ("ps -ef | grep /usr/bin/bxr_eraser | awk '{print $8}'", "r");    // 프로세스 이름 test인거 확인

	if (fp == NULL)
	{
		perror ("erro : ");
		exit (0);
	}
	
	while (fgets (buff, 256, fp) != NULL)
	{
		if (memcmp (buff, buff_pid, 19) == 0)
		{
			cnt++;
		}
		
	}
	pclose (fp);

	if (cnt > 1)
	{
		BXLog (DBG, "%-30s	\n", "CLIENT ALREADY RUNNING");
		func_gtk_dialog_modal (0, window, "\n    프로그램이 이미 실행중입니다.    \n");
		gtk_widget_destroy (window_start);
		gtk_main_quit();
	}
	else if (cnt == 1)
	{
		gtk_widget_destroy (window_start);
		gtk_widget_show (window);
		func_get_num_work (colcnt, "start");
		func_get_ecode_stat_time ("J000", "complete");
		func_get_lcode ("000000", "success");
		func_auth_api_gooroom();
		func_chk_clientid();
		func_api_gooroom ("u");
		gtk_main();
	}
	else
	{
		BXLog (DBG, "%-30s	[%s]\n", "CLIENT RUNNING ON CMD", buff);
		func_gtk_dialog_modal (0, window, "\n    프로그램이 잘못된 경로에서 실행되었습니다.    \n");
		gtk_widget_destroy (window_start);
		gtk_main_quit();
	}

	return cnt;
}/* func_chk_psname() */


/*---------------------------------------------------------------------------*/
/* Scanning Folder and File                                      				   */
/*---------------------------------------------------------------------------*/
int func_Detect (gchar *path)
{
	DIR *dp = NULL;
	struct dirent *file = NULL;
	struct stat buf;
	char filepath[300];
	long lSize = 0;

	if (lstat (path, &buf) == -1)
	{
		BXLog (ERR, "%-30s	[%s][%d]\n", "STAT ERROR", strerror (errno), errno);

		return -1;
	}
	// 폴더 //
	if (S_ISDIR (buf.st_mode))
	{
		if ((dp = opendir(path)) == NULL)
		{
			BXLog (ERR, "%-30s	[%s][%d]\n",  "BXRC_FOLDER_OPEN_ERROR", path, strerror (errno), errno);
			return -1;
		}
		while ((file = readdir(dp)) != NULL)
		{
			// filepath에 현재 path넣기
			sprintf (filepath, "%s/%s", path, file->d_name);

			// .이거하고 ..이거 제외
			if ((!strcmp (file->d_name, ".")) || (!strcmp (file->d_name, "..")))
			{
				continue;
			}
			// 안에 폴더로 재귀함수
			func_Detect(filepath);
		}
		closedir (dp);
		BXLog (DBG, "%-30s	\n", "CLOSE DIR");
	}
	else if (S_ISREG (buf.st_mode))
	{
		temptotal += buf.st_size;
		func_file_eraser (3, path);
	}

	return  0;
}
/* end of func_Detect(); */


/*---------------------------------------------------------------------------*/
/* Refresh ProgressBar                                      					      */
/*---------------------------------------------------------------------------*/
void func_refresh_progressbar()
{
	char	message[1024] = {0, };

	memset (message,   0x00, strlen (message));
	sprintf    (message, "0%%");
	gtk_progress_bar_set_text        (GTK_PROGRESS_BAR (g_bar_progress), message);

	gtk_label_set_text (GTK_LABEL (g_lbl_status), "Ready...");
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 0 );

	return;
}/* End of func_refresh_progressbar() */




/*---------------------------------------------------------------------------*/
/*	BXDB Push Data														           	  */
/*---------------------------------------------------------------------------*/
int  BXDB_Push (char *flag, char *type) // N: nomal, D: delete
{
	sqlite3	*db;
	int			 rc 		    		= 0;
	char 	 *err_msg  		    = 0;
	char	 *sql			 		 = 0;
	char 	   sql_buf[1024] = {0, };
	char	 *db_path[1024] = {0,};

	sprintf (db_path, "%s%s", data_path, DBFILE);

	rc = sqlite3_open (db_path, &db);

    if (rc != SQLITE_OK)
    {
		BXLog (ERR, "%-30s	[%s]\n", "CAN NOT OPEN DB", sqlite3_errmsg (db));
		sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    }

	if (strcmp (flag, "D") == 0)
	{
		colcnt = 1;
	}
	else
	{
		func_api_gooroom (type);
					
		sprintf (sql_buf, "INSERT INTO Client_Log VALUES ('%s','%s','%s','%s','%s','%s','%s', '%s', '%s', '%s', '%s');" ,  dBl[colcnt].db_num,
																																																	dBl[colcnt].db_name, 
																																																	dBl[colcnt].db_work,
																																																	dBl[colcnt].db_ecode,
																																																	dBl[colcnt].db_stat, 
																																																	dBl[colcnt].db_time, 
																																																	dBl[colcnt].db_size, 
																																																	dBl[colcnt].db_loca,
																																																	dBl[colcnt].db_lcode,
																																																	dBl[colcnt].db,
																																																	flag
																																																	);

		BXLog (DBG, "%-30s	[%03s] | [%-20.20s] | [%-5.5s] | [%s] | [%s] | [%s] | [%10.10s] | [%-50.50s][%s] | [%s] | [%s]\n",   "DB PUSH DATA CHECK",
																																																					dBl[colcnt].db_num,
																																																					dBl[colcnt].db_name, 
																																																					dBl[colcnt].db_work,
																																																					dBl[colcnt].db_ecode,
																																																					dBl[colcnt].db_stat, 
																																																					dBl[colcnt].db_time, 
																																																					dBl[colcnt].db_size, 
																																																					dBl[colcnt].db_loca,
																																																					dBl[colcnt].db_lcode,
																																																					dBl[colcnt].db,
																																																					flag
																																																					);

		sql = sql_buf;

		colcnt++;

		rc = sqlite3_exec (db, sql, 0, 0, &err_msg);
	}

	
    if (rc != SQLITE_OK)
    {
		BXLog (ERR, "%-30s	[%s][%d]\n", "SQL ERROR", err_msg, rc);
        sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    } 

	sqlite3_close (db);

	return 0;
}/* End of BXDB_Push () */


/*---------------------------------------------------------------------------*/
/*	BXDB Pop Data														           	   */
/*---------------------------------------------------------------------------*/
int BXDB_Pop()
{
    sqlite3 *db;
	int 		rc = 0;
    char     *err_msg = 0;
	char	 *db_path[1024] = {0,};

	sprintf (db_path, "%s%s", data_path, DBFILE);

	rc = sqlite3_open (db_path, &db);
    
    if (rc != SQLITE_OK)
    {
		BXLog (ERR, "%-30s	[%s]\n", "CAN NOT OPEN DB", sqlite3_errmsg (db));
        sqlite3_close    (db);
        
        return 1;
    }
    
    char *sql = "SELECT * FROM Client_Log";
    rc = sqlite3_exec (db, sql, callback, 0, &err_msg);
    
    if (rc != SQLITE_OK )
    {
		BXLog (ERR, "%-30s\n", "FAILED SELECT DATA");
        BXLog (ERR, "%-30s	[%s][%d]\n", "SQL ERROR", err_msg, rc);

        sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    } 

    sqlite3_close (db);

    return 0;
}/* End of BXDB_Pop() */


/*---------------------------------------------------------------------------*/
/*	BXDB CallBack Pop Data														 */
/*---------------------------------------------------------------------------*/
int callback (void *NotUsed, int argc, char **argv, char **azColName)
{
    NotUsed = 0;

	if (strcmp (argv[10], "D") != 0)
	{
		sprintf (dBl[colcnt].db_num,   "%s", argv[0]);
		sprintf (dBl[colcnt].db_name, "%s", argv[1]);
		sprintf (dBl[colcnt].db_work,  "%s", argv[2]);
		sprintf (dBl[colcnt].db_ecode, "%s", argv[3]);
		sprintf (dBl[colcnt].db_stat, 	 "%s", argv[4]);
		sprintf (dBl[colcnt].db_time,   "%s", argv[5]);
		sprintf (dBl[colcnt].db_size,     "%s", argv[6]);
		sprintf (dBl[colcnt].db_loca,    "%s", argv[7]);

		model = create_and_fill_model();

		colcnt++;
		sprintf (dBl[colcnt].db_num , "%d", colcnt);
	}

    return 0;
}/* BXDB CallBack Pop Data() */


/*---------------------------------------------------------------------------*/
/*	BXDB Update Data														        */
/*---------------------------------------------------------------------------*/
int  BXDB_Update()
{
	sqlite3	*db;
	int			 rc 		    		= 0;
	char 	 *err_msg  		    = 0;
	char	 *sql			 		 = 0;
	char 	   sql_buf[1024] = {0, };
	char	 *db_path[1024] = {0,};

	sprintf (db_path, "%s%s", data_path, DBFILE);

	rc = sqlite3_open (db_path, &db);

    if (rc != SQLITE_OK)
    {
		BXLog (ERR, "%-30s	[%s]\n", "CAN NOT OPEN DB", sqlite3_errmsg (db));
		sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    }

	sprintf (sql_buf, "UPDATE Client_Log SET flag = 'D' where flag = 'N';");		// Client_Log table의 flag컬럼 중 값이 N인것을 D로 변경
	colcnt = 1;																											  // 컬럼 초기화 ==> LogViewer 화면 Clear
	sql = sql_buf;

	rc = sqlite3_exec (db, sql, 0, 0, &err_msg);

	
    if (rc != SQLITE_OK)
    {
		BXLog (ERR, "%-30s	[%s][%d]\n", "SQL ERROR", err_msg, rc);
        sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    }
	
	sqlite3_close (db);

	return 0;
}/* End of BXDB_Update () */


/*---------------------------------------------------------------------------*/
/*	menu bar => 도움말  => 정보 => clicked					              */
/*---------------------------------------------------------------------------*/
void on_smn_about_activate (GtkMenuItem *smn_log, gpointer *data)
{
    gtk_widget_show (GTK_WIDGET (data));

	return;
}/* End of on_smn_about_activate() */


/*---------------------------------------------------------------------------*/
/*	menu bar => 보기 => 로그 => clicked					                   */
/*---------------------------------------------------------------------------*/
void on_smn_log_activate (GtkMenuItem *smn_log, gpointer *data)
{
	colcnt = 1;
	BXDB_Pop();
	func_refresh_scrolledwindow();
    gtk_widget_show (GTK_WIDGET (data));

	return;
}/* End of on_smn_log_activate() */


/*---------------------------------------------------------------------------*/
/*	menu bar => 파일 => 열기 => clicked					                   */
/*---------------------------------------------------------------------------*/
void on_smn_open_activate (GtkMenuItem *smn_open, gpointer *data)
{
	on_btn_fileopen_clicked (GTK_BUTTON (data), (gpointer) window);

	return;
}/* End of on_smn_open_activate() */


/*---------------------------------------------------------------------------*/
/*	Target File clicked															          */
/*---------------------------------------------------------------------------*/
void on_btn_fileopen_clicked (GtkButton *btn_fileopen, gpointer *data)
{
    GtkWidget *dialog;
	char filename[MAX_PATH_SIZE] = {0, };

    dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(data), GTK_FILE_CHOOSER_ACTION_OPEN, 
												("_Cancel"), GTK_RESPONSE_CANCEL, ("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (dialog);

    gint resp = gtk_dialog_run (GTK_DIALOG (dialog));

    if (resp == GTK_RESPONSE_ACCEPT)
	{
		gtk_label_set_text (GTK_LABEL (g_lbl_file), gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)));
		sprintf (filename, "%s", gtk_label_get_text (g_lbl_file));
		gtk_widget_set_tooltip_text (btn_fileopen, filename);
   	} 

	func_refresh_progressbar();
    	
	gtk_widget_destroy (dialog);

	return;
}/* End of on_btn_fileopen_clicked() */


/*---------------------------------------------------------------------------*/
/*	menu bar => 파일 => 닫기 => clicked					                   */
/*---------------------------------------------------------------------------*/
void on_smn_quit_activate (GtkMenuItem *smn_quit, gpointer *data)
{
	if (func_gtk_dialog_modal (1, GTK_WIDGET (window), "\n    정말 종료 하시겠습니까?    \n") == GTK_RESPONSE_ACCEPT)
	{
		on_window_main_destroy(data);
	}

	return;
}/* End of on_smn_quit_activate( ) */


/*---------------------------------------------------------------------------*/
/*	menu bar => 도움말  => 정보 => clicked	=>	확인	             */
/*---------------------------------------------------------------------------*/
void on_btn_about_ok_clicked (GtkButton *btn_about_ok, gpointer *data)
{
    gtk_widget_hide (GTK_WIDGET (data));

	return;         
}/* End of on_btn_about_ok_clicked() */


/*---------------------------------------------------------------------------*/
/*	Log Viewer Clear			                   									   */
/*---------------------------------------------------------------------------*/
void on_logviewer_clear_clicked (GtkButton *btn_logviewer_clear, gpointer *data)
{
	char	message[1024] = {0, };

	sprintf (message, 
			"\n    삭제 후에 로그뷰어에서 로그 확인을 할 수 없습니다.\n    로그를 삭제하시겠습니까?       \n");
    
	if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
	{
		BXDB_Update();
		memset (dBl, 0x00, sizeof(dBl));		//로그 삭제후 구조체 초기화
		func_refresh_scrolledwindow();
		gtk_widget_show (window_logviewer);
	}
	else
	{
		func_refresh_progressbar();
		gtk_label_set_text (GTK_LABEL (g_lbl_status), "Cancelled...");
	}

	return;
}/* End of on_logviewer_clear_clicked() */


void on_logviewer_close_clicked (GtkButton *btn_logviewer_close, gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));

	return;
}/* End of on_logviewer_close_clicked() */


/*---------------------------------------------------------------------------*/
/*	### Log Viewer Scrolled Window Architecture	###				*/
/*---------------------------------------------------------------------------*/
enum
{
	treeview_num = 0,
	treeview_name,
	treeview_work,
	treeview_ecode,
	treeview_stat,
	treeview_time,
	treeview_size,
	treeview_loca,
	NUM_COLS
} ;

/*---------------------------------------------------------------------------*/
/*	Log Viewer Select			                   									   */
/*---------------------------------------------------------------------------*/
gboolean
view_selection_func 	 (GtkTreeSelection	  *selection,
										 GtkTreeModel		 *model,
										 GtkTreePath			*path,
										 gboolean				    path_currently_selected,
										 gpointer				 	 userdata)
{
	/* GtkTreeIter iter;
	gchar *stat, *fpath; */
	
	/* if (gtk_tree_model_get_iter (model, &iter, path))
	{
		if (!path_currently_selected)
		{
			// set select data
			gtk_tree_model_get (model, &iter, treeview_name, 	 &fname, -1);
			gtk_tree_model_get (model, &iter, treeview_size,		&fsize,		-1);
			gtk_tree_model_get (model, &iter, treeview_stat,		&stat,		-1);
			gtk_tree_model_get (model, &iter, treeview_loca,	   &fpath,	  -1);

			// input data in structure
			strcpy (sfDs.fname, fname);
			strcpy (sfDs.stat, stat);
			strcpy (sfDs.fpath, fpath);

			//g_print ("파일위치: [%s], 파일크기: [%d] 선택.\n", sfDs.fpath, sfDs.fsize);

		}
		else
		{
			//g_print ("파일위치: [%s] 선택 해제.\n", sfDs.fpath);
		}
	} */

	return TRUE; /* allow selection state to change */
}/* End of view_selection_func() */


/*---------------------------------------------------------------------------*/
/*	Log Viewer Create and Fill Model			                   			*/
/*---------------------------------------------------------------------------*/
static GtkTreeModel *
create_and_fill_model (void)
{
	treestore = gtk_tree_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	for (int i = 1; i  < colcnt; i++)
	{
		gtk_tree_store_append (treestore, &logviewer_iter, NULL);
		gtk_tree_store_set 		   (treestore, &logviewer_iter,
												 treeview_num,		dBl[i].db_num,
												 treeview_name,	   dBl[colcnt - i].db_name,
												 treeview_work,		dBl[colcnt - i].db_work,
												 treeview_ecode,	dBl[colcnt - i].db_ecode,
												 treeview_stat,		  dBl[colcnt - i].db_stat,
												 treeview_time,		 dBl[colcnt - i].db_time,
												 treeview_size,		  dBl[colcnt - i].db_size,
												 treeview_loca,		  dBl[colcnt - i].db_loca,
												 -1);
	}

	return GTK_TREE_MODEL (treestore);
}/* End of create_and_fill_model() */

/*---------------------------------------------------------------------------*/
/*	Log Viewer Create View and Model			                   		*/
/*---------------------------------------------------------------------------*/
static GtkWidget *
create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer			 *renderer;
	GtkWidget					*view;
	//GtkTreeModel			  *model;
	GtkTreeSelection		  *selection;
	
	view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (col, "번호");
	
	// pack tree view column into tree view //
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_num);

	// --- Column #파일이름 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일이름");
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_name);

	// --- Column #작업종류--- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업종류");
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_work);

	// --- Column #작업코드 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업코드");
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_ecode);

	// --- Column #작업상태 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업상태");
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_stat);
	
	// --- Column #작업시간 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업시간");
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_time);

	// --- Column #파일크기 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일크기");
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_size);
	
	// --- Column #파일위치 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일위치");
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", treeview_loca);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
	
	gtk_tree_selection_set_select_function (selection, view_selection_func, NULL, NULL);


	model = create_and_fill_model();
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

	g_object_unref (model); // destroy model automatically with view //

	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (view)),
							  						  GTK_SELECTION_SINGLE);

	return view;
}/* End of create_view_and_model() */


/*---------------------------------------------------------------------------*/
/* Refresh ScrollWindow                                      					   */
/*---------------------------------------------------------------------------*/
void func_refresh_scrolledwindow()
{
	gtk_container_remove (GTK_CONTAINER (logviewer_scrolledwindow), view);	// 다 지우기
	view = create_view_and_model();
	gtk_container_add (GTK_CONTAINER (logviewer_scrolledwindow), view);
	gtk_widget_show_all ((GtkWidget *) logviewer_scrolledwindow);

	return;
}/* End of func_refresh_scrolledwindow() */
/* ### End of Log Viewer Scrolled Window Architecture ### */


/*---------------------------------------------------------------------------*/
/*	3Pass 삭제 button clicked								    	               */
/*---------------------------------------------------------------------------*/
void on_btn_3pass_clicked (GtkButton *btn_3pass, gpointer data )
{
	char	filename[MAX_PATH_SIZE] = {0, };
	char	message[1024] = {0, };

	chk_method = 1;
	func_refresh_progressbar();

	if (gtk_label_get_text (g_lbl_file) != NULL)
	{
		sprintf (filename, "%s", gtk_label_get_text (g_lbl_file));		// filename = 파일위치
		func_get_fname_fsize_floca (filename);
	}
	else
	{
		filename[0] = 0x00;
	}

	if (filename[0] == 0x00)
	{
		func_gtk_dialog_modal (0, window, "\n    대상파일이 선택되지 않았습니다.    \n");
	}
	else
	{
		sprintf (message, 
			"\n    삭제 후에 복구가 불가능 합니다.\n    아래 파일을 삭제하시겠습니까?\n    [ %s ]    \n", filename);
    
		if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			func_get_num_work (colcnt, "3pass");
			func_get_ecode_stat_time ("J100", "start");
			func_get_fname_fsize_floca (filename);	
			func_get_lcode ("000000", "success");
			BXDB_Push ("N", "f");
			
			func_get_num_work (colcnt, "3pass");
			func_file_eraser (3, filename);
		}
		else
		{
			func_refresh_progressbar();
			gtk_label_set_text (GTK_LABEL (g_lbl_status), "Cancelled...");
		}
	}
	
	return ;
}/* End of on_btn_3pass_clicked() */


/*---------------------------------------------------------------------------*/
/*	7Pass 삭제 button clicked								    	               */
/*---------------------------------------------------------------------------*/
void on_btn_7pass_clicked (GtkButton *btn_7pass, gpointer data )
{
	
	char	filename[MAX_PATH_SIZE] = {0, };
	char	message[1024] = {0, };

	chk_method = 1;
	
	func_refresh_progressbar();

	if (gtk_label_get_text (g_lbl_file) != NULL)
	{
		sprintf (filename, "%s", gtk_label_get_text (g_lbl_file));		// filename = 파일위치
		func_get_fname_fsize_floca (filename);
	}
	else
		filename[0] = 0x00;

	if (filename[0] == 0x00)
	{
		func_gtk_dialog_modal (0, window, "\n    대상파일이 선택되지 않았습니다.    \n");
	}
	else
	{
		sprintf (message, 
			"\n    삭제 후에 복구가 불가능 합니다.\n    아래 파일을 삭제하시겠습니까?\n    [ %s ]    \n", filename);
    
		if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			func_get_num_work (colcnt, "7pass");
			func_get_ecode_stat_time ("J100", "start");
			func_get_fname_fsize_floca (filename);
			func_get_lcode ("000000", "success");
			BXDB_Push ("N", "f");

			func_get_num_work (colcnt, "7pass");
			func_file_eraser (7, filename);
		}
		else
		{
			func_refresh_progressbar();
			gtk_label_set_text (GTK_LABEL (g_lbl_status), "Cancelled...");
		}
	}
	
	return ;
}/* End of on_btn_7pass_clicked() */


/*---------------------------------------------------------------------------*/
/*	Trim button clicked								    	               			   */
/*---------------------------------------------------------------------------*/
void on_btn_trim_clicked()
{
	FILE *fp;
	int ii = 0;
	char del[] = " ";
	char *token;
	char buff[256] = { 0, };
	char *trim_path[512] = {0,};
	char   msg[1024] = {0,};
	char   message[1024] = {0, };
	
	memset (dBl[colcnt].db_name,  0x00, strlen (dBl[colcnt].db_name));
	memset (dBl[colcnt].db_size,     0x00, strlen (dBl[colcnt].db_size));
	memset (dBl[colcnt].db_loca,    0x00, strlen (dBl[colcnt].db_loca));	
	memset (buff, 0, 1);
	memset (msg, 0, strlen(msg));

	gtk_label_set_text (GTK_LABEL (g_lbl_file), NULL);

	BXLog (DBG, "%-30s\n", "TRIM CLICK");
	func_refresh_progressbar();

	memset (message,   0x00, strlen (message));
	sprintf    (message, "Trim 기능 실행중...");
	gtk_label_set_text (GTK_LABEL (g_lbl_status), message);

	sprintf (message, 
			"\n    SSD완전삭제 후에 복구가 불가능 합니다.\n    Trim기능을 실행하시겠습니까?\n");

	if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
	{
		sprintf (trim_path, "bash %s/.bxrG/trimchk", data_path);
		func_get_num_work (colcnt, "trim");
		func_get_ecode_stat_time ("J100", "start");
		func_get_lcode ("000000", "success");
		BXDB_Push ("N", "f");

		system(trim_path);

		memset (trim_path, 0, 1);
		sprintf (trim_path, "%s%s", data_path, CHKTRIM);
		fp = fopen (trim_path, "r");

		func_get_num_work (colcnt, "trim");

		while (fgets (buff, 256, fp) != NULL)
		{
			while (gtk_events_pending()) 
					gtk_main_iteration (); 

			memset (message,   0x00, strlen (message));
			sprintf    (message, "\n\n    SSD 완전소거를 진행중 입니다...    \n\n");
			gtk_label_set_text (GTK_LABEL (msd_lbl), message);
			gtk_widget_show (msdialog);

			if ((strncmp (buff, "fstrim", 6) == 0) || (strncmp (buff, "Error", 5) == 0))
			{
				gtk_widget_destroy (msdialog);
				BXLog (DBG, "%-30s [%s]\n", "TRIM RESULT", buff);
				func_get_ecode_stat_time ("J003", "fail");
				func_get_lcode ("000000", "success");
				BXDB_Push ("N", "f");

				memset (message,   0x00, strlen (message));
				sprintf    (message, "trim size [0 byte/0 byte]\n");
				gtk_label_set_text (GTK_LABEL (g_lbl_status), message);
				

				memset (message,   0x00, strlen (message));
				sprintf (message, "%.0f%% Complete", 0.0);
				gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 0.0);
				gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

				func_gtk_dialog_modal (0, window, "\n   해당 하드디스크에서는 Trim기능을 사용할 수 없습니다.    \n");
			}
			else
			{
				gtk_widget_destroy (msdialog);
				BXLog (DBG, "%-30s [%s]\n", "TRIM RESULT", buff);
				func_get_ecode_stat_time ("J000", "complete");
				func_get_lcode ("000000", "success");
				BXDB_Push ("N", "f");
				
				memset (message,   0x00, strlen (message));
				
				token = strtok (buff, del);

				while (token != NULL)
				{
					ii++;
					token = strtok (NULL, " ");
					if (ii == 3)
					{
						sprintf (message, "trim size [%s byte/%s byte]\n", &token[1], &token[1]);
						break;
					}
				}
				gtk_label_set_text (GTK_LABEL (g_lbl_status), message);

				memset (message,   0x00, strlen (message));
				sprintf (message, "%.0f%% Complete", 100.0);
				gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 100.0 / 100.0);
				gtk_progress_bar_set_text        (GTK_PROGRESS_BAR (g_bar_progress), message);

				sprintf (msg, "\n    SSD완전삭제 완료.    \n");
				func_gtk_dialog_modal (0, window, msg);
			}
		}

		fclose (fp);
	}
	else
	{
		func_refresh_progressbar();
		gtk_label_set_text (GTK_LABEL (g_lbl_status), "Cancelled...");
	}
	
	remove (trim_path);

	return ;
}/* End of on_btn_trim_clicked() */


/*---------------------------------------------------------------------------*/
/*	Temp button clicked								    	               			 */
/*---------------------------------------------------------------------------*/
void on_btn_temp_clicked()
{
	char *tempapth[MAX_PATH_SIZE] = {0,};
	char message[1024] = {0, };
	//char *uname[256] = {0,};
	chk_method = 0;
	temptotal = 0;

	memset (dBl[colcnt].db_name,  0x00, strlen (dBl[colcnt].db_name));

	memset (dBl[colcnt].db_size,     0x00, strlen (dBl[colcnt].db_size));

	memset (dBl[colcnt].db_loca,    0x00, strlen (dBl[colcnt].db_loca));

	gtk_label_set_text (GTK_LABEL (g_lbl_file), NULL);

	BXLog (DBG, "%-30s\n", "TEMP CLICK");
	func_refresh_progressbar();

	memset (message,   0x00, strlen (message));
	sprintf    (message, "임시파일 삭제중...");
	gtk_label_set_text (GTK_LABEL (g_lbl_status), message);

	sprintf (message, 
			"\n    삭제 후에 복구가 불가능 합니다.\n    임시 파일을 삭제하시겠습니까?\n");
    
	if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
	{
		func_get_num_work (colcnt, "temp");
		func_get_ecode_stat_time ("J100", "start");
		func_get_lcode ("000000", "success");
		memset	(dBl[colcnt].db_size,  0x00, strlen (dBl[colcnt].db_size));	
		sprintf		(dBl[colcnt].db_size, "%ld", temptotal);
		BXDB_Push ("N", "f");

		sprintf (tempapth, "%s/.local/share/hwp", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 15.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 15.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.local/share/hwpviewer", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 25.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 25.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.local/share/hcl", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 35.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 35.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.local/share/hclviewer", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 45.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 45.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.local/share/hsl", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 55.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 55.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.local/share/hslviewer", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 65.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 65.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.local/share/hword", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 75.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 75.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.local/share/hwordviewer", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 85.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 85.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.hnc", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 90.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 90.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);

		sprintf (tempapth, "%s/.config/hnc", data_path);
		func_Detect (tempapth);

		sprintf (message, "%.0f%% Complete", 100.0);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 100.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);
	
		/*root 권한 여부 결정해야함*/
		/* cuserid (uname);
		sprintf (tempapth, "/tmp/hnc-%s", uname);
		func_Detect (tempapth); */
		func_get_num_work (colcnt, "temp");
		func_get_ecode_stat_time ("J000", "complete");
		func_get_lcode ("000000", "success");
		memset	(dBl[colcnt].db_size,  0x00, strlen (dBl[colcnt].db_size));
		sprintf		(dBl[colcnt].db_size, "%ld", temptotal);
		BXDB_Push ("N", "f");

		memset (message, 0x00, strlen (message));
		sprintf    (message, "file size [%.0f byte/%ld byte]\n", temptotal, temptotal);
		gtk_label_set_text (GTK_LABEL (g_lbl_status), message);

		func_gtk_dialog_modal (0, window, "\n    임시파일 삭제가 완료되었습니다.    \n");
	}
	else
	{
		func_refresh_progressbar();
		gtk_label_set_text (GTK_LABEL (g_lbl_status), "Cancelled...");
	}

	return ;
}/* End of on_btn_temp_clicked() */


/*---------------------------------------------------------------------------*/
/*	Setting button clicked								    	               		 */
/*---------------------------------------------------------------------------*/
void on_btn_setting_clicked(GtkButton *btn_setting, gpointer *data)
{	
	BXLog (DBG, "%-30s\n", "SETTING CLICK");
	func_gtk_dialog_modal (0, window, "\n    현재는 해당 기능을 지원하지 않습니다.    \n");

	return ;
}/* End of on_btn_setting_clicked() */


/*---------------------------------------------------------------------------*/
/*  Get DB Num and Work Data												*/
/*---------------------------------------------------------------------------*/
void func_get_num_work (int colcnt, char *work)
{
	memset (dBl[colcnt].db_num,   0x00, strlen (dBl[colcnt].db_num));
	sprintf    (dBl[colcnt].db_num , "%d", colcnt);

	memset (dBl[colcnt].db_work,   0x00, strlen (dBl[colcnt].db_work));
	strcpy     (dBl[colcnt].db_work, work);

	return ;
}/* End of func_get_num_work() */


/*---------------------------------------------------------------------------*/
/*  Get DB Ecode, Stat and Time Data										*/
/*---------------------------------------------------------------------------*/
void func_get_ecode_stat_time (char *ecode, char *stat)
{
	struct  timeval t;
    struct  tm *tm;

	gettimeofday   (&t, NULL);
    tm = localtime (&t.tv_sec);

	memset (dBl[colcnt].db_ecode,   0x00, strlen (dBl[colcnt].db_ecode));
	strcpy     (dBl[colcnt].db_ecode, ecode);

	memset (dBl[colcnt].db_stat,   0x00, strlen (dBl[colcnt].db_stat));
	strcpy     (dBl[colcnt].db_stat, stat);
	
	memset (dBl[colcnt].db_time,   0x00, strlen (dBl[colcnt].db_time));
	sprintf    (dBl[colcnt].db_time, "%04d/%02d/%02d/%02d:%02d:%02d.%03ld",
														tm->tm_year + 1900, 
														tm->tm_mon + 1, 
														tm->tm_mday, 
														tm->tm_hour, 
														tm->tm_min, 
														tm->tm_sec,
														t.tv_usec/1000
														);
														
	return ;
}/* End of func_get_ecode_stat_time() */


/*---------------------------------------------------------------------------*/
/*  Get DB File Name, File Size, File Location Data					   */
/*---------------------------------------------------------------------------*/
void func_get_fname_fsize_floca (char filename[])
{
	struct	stat  file_info;
	char    *pos = 0;
	char tempfpath[MAX_PATH_SIZE] = {0,};
	
	stat (filename, &file_info);

	if (filename[0] != 0)
	{
		pos = strrchr (filename,    '/');
		memset  	   (dBl[colcnt].db_name, 0x00, strlen (dBl[colcnt].db_name));		// 파일이름
		strcpy 			   (dBl[colcnt].db_name, pos+1);
	}

	memset	(dBl[colcnt].db_size,  0x00, strlen (dBl[colcnt].db_size));						// 파일크기
	sprintf		(dBl[colcnt].db_size, "%ld", file_info.st_size);

	memset  (dBl[colcnt].db_loca, 0x00, strlen (dBl[colcnt].db_loca));						// 파일위치
	strcpy 	    (tempfpath, filename);
	strcpy 	    (dBl[colcnt].db_loca, dirname (tempfpath));

	return ;
}/* End of func_get_fname_fsize_floca() */


/*---------------------------------------------------------------------------*/
/*  Get LSF, LSF Code																	*/
/*---------------------------------------------------------------------------*/
void func_get_lcode (char *lcode, char *lstat)
{
	memset (dBl[colcnt].db_lcode,   0x00, strlen (dBl[colcnt].db_lcode));
	strcpy     (dBl[colcnt].db_lcode, lcode);

	memset (dBl[colcnt].db,   0x00, strlen (dBl[colcnt].db));
	strcpy     (dBl[colcnt].db, lstat);

	return ;
}/* End of func_get_lcode() */


/*---------------------------------------------------------------------------*/
/*  func_auth_api gooroom											    		*/
/*---------------------------------------------------------------------------*/
int func_auth_api_gooroom()
{
	char message[1024] = {0, };

	memset (&message, 0x00, sizeof (message));
    memset (&app_data, 0x00, sizeof (user_data_t));

    auth_result = auth (&app_data, ERASER_PASSPHRASE);



	while (gtk_events_pending()) 
					gtk_main_iteration (); 

	switch (auth_result)
	{
		case 0:
			BXLog (DBG, "%-30s	[%d]\n", "AUTH SUCCESS", auth_result);
			
			break;

		case 1:		
			if (auth_cnt < 3)
			{
				sprintf (message, "    서버가 불안정한 상태입니다.    \n    다시 연결을 시도합니다. (%d/3)    \n", auth_cnt + 1);
				sleep(2);
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "AUTH ERROR", auth_result, auth_cnt + 1);
				gtk_label_set_text (GTK_LABEL (start_lbl), message);

				auth_cnt++;
				func_auth_api_gooroom();
			}
			else
			{
				sprintf (message, "    서버가 불안정하여 연결에 실패했습니다.    \n    계속 실행하시겠습니까?");
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "RETRY AUTH ERROR", auth_result, auth_cnt);
				
				if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
				{
					auth_result = 0;
				}
				else
				{
					return -1;
				}
			}

			break;

		case 2:
			if (auth_cnt < 3)
			{
				sprintf (message, "    서버가 불안정한 상태입니다.    \n    다시 연결을 시도합니다. (%d/3)    \n", auth_cnt + 1);
				sleep(2);
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "AUTH ERROR", auth_result, auth_cnt + 1);
				gtk_label_set_text (GTK_LABEL (start_lbl), message);

				auth_cnt++;
				func_auth_api_gooroom();
			}
			else
			{
				sprintf (message, "    서버가 불안정하여 연결에 실패했습니다.    \n    계속 실행하시겠습니까?");
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "RETRY AUTH ERROR", auth_result, auth_cnt);
				
				if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
				{
					auth_result = 0;
				}
				else
				{
					return -1;
				}
			}
			
			break;

		case 3:
			if (auth_cnt < 3)
			{
				sprintf (message, "    서버가 불안정한 상태입니다.    \n    다시 연결을 시도합니다. (%d/3)    \n", auth_cnt + 1);
				sleep(2);
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "AUTH ERROR", auth_result, auth_cnt + 1);
				gtk_label_set_text (GTK_LABEL (start_lbl), message);

				auth_cnt++;
				func_auth_api_gooroom();
			}
			else
			{
				sprintf (message, "    서버가 불안정하여 연결에 실패했습니다.    \n    계속 실행하시겠습니까?");
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "RETRY AUTH ERROR", auth_result, auth_cnt);
				
				if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
				{
					auth_result = 0;
				}
				else
				{
					return -1;
				}
			}
			
			break;

		case 4:
			if (auth_cnt < 3)
			{
				sprintf (message, "    서버가 불안정한 상태입니다.    \n    다시 연결을 시도합니다. (%d/3)    \n", auth_cnt + 1);
				sleep(2);
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "AUTH ERROR", auth_result, auth_cnt + 1);
				gtk_label_set_text (GTK_LABEL (start_lbl), message);

				auth_cnt++;
				func_auth_api_gooroom();
			}
			else
			{
				sprintf (message, "    서버가 불안정하여 연결에 실패했습니다.    \n    계속 실행하시겠습니까?");
				BXLog (ERR, "%-30s	[%d][%d/3]\n", "RETRY AUTH ERROR", auth_result, auth_cnt);
				
				if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
				{
					auth_result = 0;
				}
				else
				{
					return -1;
				}
			}
			
			break;
		
	}
	auth_cnt = 0;

	return auth_result;
}/* End of func_auth_api_gooroom() */


/*---------------------------------------------------------------------------*/
/*  func_api gooroom											    			    */
/*---------------------------------------------------------------------------*/
int func_api_gooroom (char *type)
{
	struct  timeval t;
    struct  tm *tm;
	json_object *send_obj;
    char *response = NULL;
    char   message[1024 * 8] = {0,};
	char *msg_fmt = NULL;
    int message_result = 0;
	int dbfsize = 0;
	json_object *req_obj, *req_return_obj, *req_etype_obj;
	 const char *req_return_val, *req_etype_val;

	if (auth_result !=0)
	{
		return 0;
	}

	gettimeofday   (&t, NULL);
    tm = localtime (&t.tv_sec);

	if (type == "u")
	{
		msg_fmt = "{		}";

		snprintf (message, 1024 * 8, msg_fmt, app_data.access_token,	app_data.client_id,
																													dBl[colcnt].db_work,
																													dBl[colcnt].db_ecode,
																													dBl[colcnt].db_stat,
																													tm->tm_year + 1900, 
																													tm->tm_mon + 1, 
																													tm->tm_mday, 
																													tm->tm_hour, 
																													tm->tm_min, 
																													tm->tm_sec,
																													t.tv_usec/1000,
																													dBl[colcnt].db_lcode,
																													dBl[colcnt].db);
	}
	else if (type == "f")
	{
		msg_fmt = "{ }";

		dbfsize =atoi (dBl[colcnt].db_size);
		snprintf (message, 1024 * 8, msg_fmt, app_data.access_token, 	app_data.client_id,
																													dBl[colcnt].db_name, 
																													dbfsize, 
																													dBl[colcnt].db_loca, 
																													dBl[colcnt].db_work,
																													dBl[colcnt].db_ecode,
																													dBl[colcnt].db_stat,
																													tm->tm_year + 1900, 
																													tm->tm_mon + 1, 
																													tm->tm_mday, 
																													tm->tm_hour, 
																													tm->tm_min, 
																													tm->tm_sec,
																													t.tv_usec/1000,
																													dBl[colcnt].db_lcode,
																													dBl[colcnt].db);
	}
	else
	{
		BXLog (ERR, "%-30s	[%c]\n", "LOG TYPE ERROR", type);
		
		return -1;
	}

    message_result = send_message (app_data.symm_key, message, &response);
	send_obj = json_tokener_parse (message);
	BXLog (DBG, "%-30s	[%d][%s]\n", "MESSAGE", strlen(message), json_object_to_json_string_ext (send_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

	switch (message_result)
	{
		case 0:
				BXLog (DBG, "%-30s	[%s][%d]\n", "SEND_MESSAGE SUCCESS", response, message_result);
			
			break;

		case 1:		
				BXLog (ERR, "%-30s	[%s][%d]\n", "SEND_MESSAGE ERROR", response, message_result);

			break;

		case 2:
				BXLog (ERR, "%-30s	[%s][%d]\n", "SEND_MESSAGE ERROR", response, message_result);

			break;

		case 3:
				BXLog (ERR, "%-30s	[%s][%d]\n", "SEND_MESSAGE ERROR", response, message_result);
			
			break;

		case 4:
				BXLog (ERR, "%-30s	[%s][%d]\n", "SEND_MESSAGE ERROR", response, message_result);
				func_auth_api_gooroom();
				func_api_gooroom (type);
			
			break;
		
	}

	req_obj = json_tokener_parse (response);

	json_object_object_get_ex (req_obj, 			"return", &req_return_obj);
	json_object_object_get_ex (req_return_obj, "etype",  &req_etype_obj);
	req_return_val  = json_object_get_string (req_return_obj);
	req_etype_val  	= json_object_get_string (req_etype_obj);

	if (req_etype_val != 'S')
	{
		BXLog (ERR, "%-30s	[%s]\n", "G SERVER ERROR", req_etype_val);
	}

    return 0;
}/* End of func_api_gooroom() */


/*---------------------------------------------------------------------------*/
/*  M A I N																		              */
/*---------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
	char *flag;
	char *glade_path[MAX_PATH_SIZE] = {0,};
	
   	LogName = basename (argv[0]); 
	gtk_init (&argc, &argv);

    builder = gtk_builder_new();

	func_get_homepath();

	sprintf (glade_path, "%s", GLADE);

    gtk_builder_add_from_file (builder, glade_path, NULL);
	
	window = 					  				GTK_WIDGET (gtk_builder_get_object (builder, "window_main"));
	window_logviewer = 	  				GTK_WIDGET (gtk_builder_get_object (builder, "window_logviewer"));
	window_about =						  GTK_WIDGET (gtk_builder_get_object (builder, "window_about"));
	window_start = 							GTK_WIDGET (gtk_builder_get_object (builder, "window_start"));
	msdialog		 =							GTK_WIDGET (gtk_builder_get_object (builder, "msdialog"));

	start_lbl	= 				 			   	   GTK_WIDGET (gtk_builder_get_object (builder, "start_lbl")); 
	msd_lbl	= 				 			   	   	 GTK_WIDGET (gtk_builder_get_object (builder, "msd_lbl"));
	g_lbl_file	= 								  GTK_WIDGET (gtk_builder_get_object (builder, "lbl_file"));
    g_lbl_status = 				 				GTK_WIDGET (gtk_builder_get_object (builder, "lbl_status")); 
    g_bar_progress = 		 				GTK_WIDGET (gtk_builder_get_object (builder, "bar_progress"));

	logviewer_scrolledwindow	=	GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "logviewer_scrolledwindow"));

	gtk_window_set_position (GTK_WINDOW (window), 					GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (window_start), 		 GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (window_logviewer), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (window_about), 	   GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (msdialog), 	   			 GTK_WIN_POS_CENTER);

	g_signal_connect (window_about,		   "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	g_signal_connect (window_logviewer,	 "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	g_signal_connect (msdialog,	 				 "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	

	gtk_builder_connect_signals (builder, NULL);
    g_object_unref (builder);

	BXDB_Pop();

	BXLog (DBG, "%-30s	[%d]\n", "DB COLCNT CHECK", colcnt);

	func_chk_psname();

	gtk_widget_show (window_start);

    return 0;
}/* End of main() */


/*---------------------------------------------------------------------------*/
/*	Destroy window_main          											     */
/*---------------------------------------------------------------------------*/
void on_window_main_destroy()
{
	func_get_num_work (colcnt, "end");
	func_get_ecode_stat_time ("J000", "complete");
	func_get_lcode ("000000", "success");
	func_api_gooroom ("u");
	//BXDB_Push ("U", "u");
    gtk_main_quit();
	
	return;
}/* End of on_window_main_destroy() */


/*---------------------------------------------------------------------------*/
/*	Call dialog_modal												                   */
/*---------------------------------------------------------------------------*/
int func_gtk_dialog_modal (int type, GtkWidget *widget, char *message)
{
	GtkWidget *dialog, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;
	int	rtn = GTK_RESPONSE_REJECT;

	switch(type)
	{
		case 0 :
			dialog = gtk_dialog_new_with_buttons("Blue X-ray Eraser", GTK_WINDOW(widget), flags, 
																			 ("_OK"), GTK_RESPONSE_ACCEPT, NULL );
			break;

		case 1 :
			dialog = gtk_dialog_new_with_buttons("Blue X-ray Eraser", GTK_WINDOW(widget), flags,
																			 ("_OK"), GTK_RESPONSE_ACCEPT, 
																			 ("_Cancel"), GTK_RESPONSE_REJECT, NULL );
			break;

		default :
			break;
	}

	label = gtk_label_new (message);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_widget_show_all (dialog);

	rtn = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return (rtn);	
}/* End of func_gtk_dialog_modal() */


/*---------------------------------------------------------------------------*/
/*	Func_file_eraser (3Pass, 7Pass)								                */
/*---------------------------------------------------------------------------*/
int func_file_eraser (int type, char *filename)
{
	FILE *fp;
	struct	stat  file_info;
	char message[1024 * 4] = {0, };
	char MsgTmp[5];
	char *msize;
	int mode = R_OK | W_OK;
	gdouble percent = 0.0;
	gdouble size = 0.0;
	char newfpath[MAX_PATH_SIZE] = {0,};
	char tempfpath[MAX_PATH_SIZE] = {0,};
	char randstr[24] = {0,};
	int fl = 0;
	
	gtk_label_set_text (GTK_LABEL (g_lbl_status), "Start Eraser...");

	strcpy (dBl[colcnt].db_name,  dBl[colcnt - 1].db_name);
	strcpy (dBl[colcnt].db_work,   dBl[colcnt - 1].db_work);
	strcpy (dBl[colcnt].db_size,      dBl[colcnt - 1].db_size);
	strcpy (dBl[colcnt].db_loca,     dBl[colcnt - 1].db_loca);

	if (access (filename, mode) != 0)
	{
		BXLog (ERR, "%-30s [%s][%d]\n", "FILE/FOLDER ERR", strerror (errno), errno);
		gtk_label_set_text (GTK_LABEL (g_lbl_status), "File Errors...");
		func_get_ecode_stat_time ("J001", "fail");
		func_get_lcode ("000000", "success");
		BXDB_Push ("N", "f");
		memset (message,   0x00, strlen (message));
		sprintf    (message, "\n    파일이 삭제 가능한 상태가 아닙니다.    \n    (%s)    \n", strerror (errno)); // errno 별로 따로 처리해줘야 할수도..
		func_gtk_dialog_modal (0, window, message);
	}
	else
	{
		if (stat (filename, &file_info) > 0)
		{
			BXLog (ERR, "%-30s [%s][%d]\n", "FILE/FOLDER ERR", strerror (errno), errno);
			gtk_label_set_text (GTK_LABEL (g_lbl_status), "Checking File Errors...");		
			func_get_ecode_stat_time ("J002", "fail");
			func_get_lcode ("000000", "success");
			BXDB_Push ("N", "f");
		}
		else
		{
			msize = malloc (ERASER_SIZE);
			fp = fopen (filename, "w");
			sprintf (message, "file size [%ld]", file_info.st_size);
			gtk_label_set_text (GTK_LABEL (g_lbl_status), message);
			
			memset (message,   0x00, strlen (message));
			sprintf    (message, "%.0f%% Complete", percent);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), percent / 100.0);
			gtk_progress_bar_set_text        (GTK_PROGRESS_BAR (g_bar_progress), message);

			size = 0;

			while (size < file_info.st_size)
			{   
				switch (type)
				{
					case 3 :
						MsgTmp[0] = 0x00;
						memset (msize, MsgTmp[0], ERASER_SIZE);
						fseek (fp, 0L, SEEK_SET);
						
						MsgTmp[0] = 0xFF;
						memset (msize, MsgTmp[0], ERASER_SIZE);
						fseek (fp, 0L, SEEK_SET);
						
						srand (time (NULL));
						if (size < ERASER_SIZE)
							for (int j = 0 ;j < ERASER_SIZE; j++)
								msize[j] = (random() % 26);

						break;
						
					case 7 :
					
						for (int i = 0; i < 2; i++)
						{
							MsgTmp[0] = 0x00;
							memset (msize, MsgTmp[0], ERASER_SIZE);
							fseek (fp, 0L, SEEK_SET);
							
							MsgTmp[0] = 0xFF;
							memset (msize, MsgTmp[0], ERASER_SIZE);
							fseek (fp, 0L, SEEK_SET);
							
							MsgTmp[0] = 0x96;
							memset (msize, MsgTmp[0], ERASER_SIZE);
							fseek (fp, 0L, SEEK_SET);

						}
						
						srand (time (NULL));
						if (size < ERASER_SIZE )
							for (int j = 0; j < ERASER_SIZE; j++)
								msize[j] = (random() % 26);
						fseek (fp, 0L, SEEK_SET);
						break;

				}

				while (gtk_events_pending()) 
					gtk_main_iteration (); 

				fwrite (msize, 1, ( (file_info.st_size / size)) > 0 ? ERASER_SIZE : (file_info.st_size % ERASER_SIZE), fp);
				size += ERASER_SIZE;
				percent = size / file_info.st_size * 100.0;
				if ((int) size % PROGRESS_SIZE == 0)
				{
					memset (message, 0x00, strlen (message));
					sprintf (message, "%.0f%% Complete", percent);
				//				gchar *message = g_strdup_printf ("%.0f%% Complete", percent);
					gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), percent / 100.0);
					gtk_progress_bar_set_text        (GTK_PROGRESS_BAR (g_bar_progress), message);


					memset (message, 0x00, strlen (message));
					sprintf    (message, "file size [%.0f byte/%ld byte]\n", size, file_info.st_size);
					gtk_label_set_text (GTK_LABEL (g_lbl_status), message);
				}
			}

			sprintf (message, "%.0f%% Complete", 100.0);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (g_bar_progress), 100.0);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR (g_bar_progress), message);
	
			fclose (fp);
			free (msize);
			memset (message,   0x00, strlen (message));
			sprintf    (message, "file size [%ld byte/%ld byte]\n", file_info.st_size, file_info.st_size);
			gtk_label_set_text (GTK_LABEL (g_lbl_status), message);

			for (int i = 0; i < 24; i++ )
			{
				randstr[i] = 'a' + rand() % 26;
			}

			sprintf (tempfpath, "%s", filename);
			sprintf (newfpath, "%s/%s", dirname (tempfpath), randstr);
			//BXLog (DBG, "%-30s\nnewfpath: [%s]\nfilename: [%s]\n", "PATH CHECK", newfpath, filename);
			fl = rename (filename, newfpath);

			if (fl == -1)
			{
				BXLog (ERR, "%-30s [%s][%d]\n", "FILE/FOLDER ERR", strerror (errno), errno);
			}
			else
			{
				BXLog (DBG, "%-30s\n", "RENAME SUCCESS BEFORE REMOVE");
			}

			if (chk_method != 0)
			{
				if (remove (newfpath) == 0)
				{
					func_get_ecode_stat_time ("J000", "complete");
					func_get_fname_fsize_floca (filename);
					sprintf		(dBl[colcnt].db_size, "%ld", file_info.st_size);
					func_get_lcode ("000000", "success");
					BXDB_Push ("N", "f");

					func_gtk_dialog_modal (0, window, "\n    삭제가 완료되었습니다.    \n");
				}
				else
				{
					func_get_ecode_stat_time ("J004", "fail");
					func_get_fname_fsize_floca (filename);
					func_get_lcode ("000000", "success");
					BXDB_Push ("N", "f");
				}
			}
			else
			{
				remove (newfpath);
			}

		}
	}

	return (TRUE);
}/* End of int func_file_eraser() */


/*---------------------------------------------------------------------------*/
/*	BXLog			                                                                          */
/*---------------------------------------------------------------------------*/
int BXLog (const char *logfile, int logflag, int logline, const char *fmt, ...)
{
    int fd, len;
    struct  timeval t;
    struct tm *tm;
    static char fname[128];
    static char sTmp[1024*2], sFlg[5];

    va_list ap;

    switch (logflag)
    {
        case    1 :
            sprintf (sFlg, "E");
            break;
        case    2 :
            sprintf (sFlg, "I");
            break;
        case    3 :
            sprintf (sFlg, "W");
            break;
        case    4 :
        default   :
#ifndef _BXDBG
            return 0;
#endif
            sprintf (sFlg, "D");

            break;
    }

    memset (sTmp, 0x00, sizeof (sTmp));
    gettimeofday (&t, NULL);
    tm = localtime (&t.tv_sec);

    /* [HHMMSS ssssss flag __LINE__] */
    len = sprintf (sTmp, "[%5d:%08x/%02d:%02d:%02d.%03ld/%s:%4d] ",
            getpid(), (unsigned int) pthread_self(),
            tm->tm_hour, tm->tm_min, tm->tm_sec, t.tv_usec/1000,
            sFlg, logline );

    va_start (ap, fmt);
    vsprintf ((char *) &sTmp[len], fmt, ap);
    va_end (ap);

	sprintf (fname, "%s/.bxrG/%s.%02d%02d", data_path, logfile, tm->tm_mon+1, tm->tm_mday);
    if (access (fname, 0) != 0)
	{
        fd = open (fname, O_WRONLY | O_CREAT,	  0660);
	}
    else
	{
        fd = open (fname, O_WRONLY | O_APPEND,	0660);
	}
	
    if (fd >= 0)
    {
        write (fd, sTmp, strlen (sTmp));
        close (fd);
    }

    return 0;

}/* End of BXLog() */

