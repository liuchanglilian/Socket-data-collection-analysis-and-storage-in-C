#define     _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include "config.h"
#include "sensor_db.h"
#include <pthread.h>
#include "connmgr.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include "lib/dplist.h"
#include "config.h"
#include "datamgr.h"
#include <pthread.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/tcpsock.h"
#include <unistd.h>
#include "lib/dplist.h"
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "errmacros.h"
extern pthread_mutex_t fifo_mutex3;
char *send_buf;
FILE* fpfifo;
void storagemgr_parse_sensor_data(DBCONN * conn, sbuffer_t ** buffer)
{double SV=0;
 time_t ST=0;
 int dataSI=0;

while(1)
{   printf("$$$$$$$$$$$$$$$$$$$entering the while(1) in storageparse "); 
    sensor_data_t * data=NULL;
    data=malloc(sizeof(sensor_data_t));
    int SBremovereturn=0;
    printf("1\n");
    SBremovereturn=sbuffer_remove( *buffer,data,TIMEOUT);
    printf("2\n");
    if(SBremovereturn==SBUFFER_FAILURE)
	{   printf("Sbuffer failure(sensordb)\n");
	    break;
	}
	if(SBremovereturn==SBUFFER_NO_DATA)
	{  printf("failure\n");
	   usleep(TIMEOUT*1000000);
	   SBremovereturn=sbuffer_remove( *buffer, data,TIMEOUT);
	   if(SBremovereturn==SBUFFER_NO_DATA)
	   {
		   printf("the sbuffer doesn't exit(sensordb)");
		   pthread_exit(NULL);
		  
	   }	
	}
	   if(SBremovereturn==SBUFFER_SUCCESS)
	     {      
              printf("succed(sensordb)\n");
             }
           SV=data->value;
           ST=data->ts;
           dataSI=data->id;
           char * sql=NULL;
           char *err_msg = 0; 
           asprintf(&sql,"INSERT INTO %s VALUES(NULL,%f, %f,%f)", TO_STRING(TABLE_NAME), (double)dataSI, (double)SV,(double) ST);
           int rc = sqlite3_exec(conn, sql, 0, 0, &err_msg);
           free(sql);
          if(rc!=SQLITE_OK)
             {fprintf(stderr,"SQL error: %s\n", err_msg);
	      sqlite3_free(err_msg);
      
             }
         free(data);
    }

}
DBCONN * init_connection(char clear_up_flag)
{ /* Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * If the table existed, clear up the existing data if clear_up_flag is set to 1
 * Return the connection for success, NULL if an error occurs*/
 char * sql=NULL;
 sqlite3 *db;
 char *err_msg = 0;
 int rc = sqlite3_open(TO_STRING(DB_NAME), &db);
 	if(rc != SQLITE_OK) 
  	{
	  fprintf(stderr,"Cannot open database: %s\n", sqlite3_errmsg(db));
	  sqlite3_close(db);
	  fifoconnection();
	  asprintf( &send_buf,"%ldCannot open database\n",time(NULL));
          if ( fputs( send_buf, fpfifo ) == EOF )
             {
               fprintf( stderr, "Error writing data to fifo(connmgr)\n");
               exit( EXIT_FAILURE );
             } 
	 FFLUSH_ERROR(fflush(fpfifo));
         int presult=pthread_mutex_unlock(&fifo_mutex3);	
         PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );	    

  	}
      else{
	   fifoconnection();
	    asprintf( &send_buf,"%ldConnect to database\n",time(NULL));
		  if ( fputs( send_buf, fpfifo ) == EOF )
                     {
                       fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                        exit( EXIT_FAILURE );
                     } 
		FFLUSH_ERROR(fflush(fpfifo));
                int presult=pthread_mutex_unlock(&fifo_mutex3);
                PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );	
	 
            } 	
   if(clear_up_flag=='1')
     { asprintf(&sql,"DROP TABLE IF EXISTS %s;", TO_STRING(TABLE_NAME));
       rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
       printf("aborded\n");
       if(rc != SQLITE_OK) 
  	  {
	   fprintf(stderr,"Cannot open database: %s\n", sqlite3_errmsg(db)); 
            //  sqlite3_free(err_msg);  
            sqlite3_close(db);
	   return NULL;
  	  }
      free(sql);
   
    } 
asprintf(&sql, "CREATE TABLE IF NOT EXISTS %s(id INTEGER PRIMARY KEY AUTOINCREMENT not null, sensor_id INT, sensor_value DECIMAL(4,2),timestamp TIMESTAMP);", TO_STRING(TABLE_NAME));
 rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
 if(rc != SQLITE_OK) 
  	{
	fprintf(stderr,"Cannot open database: %s\n", sqlite3_errmsg(db)); 
          // sqlite3_free(err_msg); 
        sqlite3_close(db);
	return NULL;
  	}
  	else
  	{
	 fifoconnection();
	 asprintf( &send_buf,"%ld Create database%s\n",time(NULL), TO_STRING(TABLE_NAME));
	 if ( fputs( send_buf, fpfifo ) == EOF )
             {
                fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                exit( EXIT_FAILURE );
             } 
	 FFLUSH_ERROR(fflush(fpfifo));
         int presult=pthread_mutex_unlock(&fifo_mutex3);
         PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
		
		
		
      }
    free(sql);
    fifoconnection();
    asprintf( &send_buf,"%ld suceed in open database\n",time(NULL));
    if ( fputs( send_buf, fpfifo ) == EOF )
     {
        fprintf( stderr, "Error writing data to fifo(connmgr)\n");
        exit( EXIT_FAILURE );
     } 
   FFLUSH_ERROR(fflush(fpfifo));
  int presult=pthread_mutex_unlock(&fifo_mutex3);	
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  return db;
 }
void disconnect(DBCONN *conn)
{
sqlite3_close(conn);
}
int insert_sensor(DBCONN * conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts)
{  
 char * sql=NULL;
 char *err_msg = 0;
 asprintf(&sql,"INSERT INTO %s VALUES(NULL,%f, %f,%f)", TO_STRING(TABLE_NAME), (double)id, (double)value, (double)ts);
 int rc = sqlite3_exec(conn, sql, 0, 0, &err_msg);
 free(sql);
 if(rc == SQLITE_OK) 
   return 0;
 else
{       fprintf(stderr,"SQL error: %s\n", err_msg);
	sqlite3_free(err_msg);
        return 1;
}

}
int find_sensor_all(DBCONN * conn, callback_t f)
{
  char * sql=NULL;
  char *err_msg = 0;
  asprintf(&sql,"SELECT * from %s ", TO_STRING(TABLE_NAME));
  int rc = sqlite3_exec(conn, sql, f, 0, &err_msg);
  free(sql);
  if(rc == SQLITE_OK) 
     return 0;
  else
       {fprintf(stderr,"SQL error: %s\n", err_msg);
	sqlite3_free(err_msg);
        return 1;
       }
int find_sensor_by_value(DBCONN * conn, sensor_value_t value, callback_t f)
{
 char * sql=NULL;
 char *err_msg = 0;
// printf("finding sensor by value");
 asprintf(&sql,"SELECT * from %s WHERE sensor_value =%f  ", TO_STRING(TABLE_NAME),(double)value);
 int rc = sqlite3_exec(conn, sql,f, 0, &err_msg);
 free(sql);
 if(rc == SQLITE_OK) 
 return 0;
 else
 {     fprintf(stderr,"SQL error: %s\n", err_msg);
       sqlite3_free(err_msg);
       return 1;
 }

}
int find_sensor_exceed_value(DBCONN * conn, sensor_value_t value, callback_t f)
{char * sql=NULL;
 char *err_msg = 0;
asprintf(&sql,"SELECT * from %s WHERE sensor_value >%f  ", TO_STRING(TABLE_NAME),(double)value);
int rc = sqlite3_exec(conn, sql,f, 0, &err_msg);
free(sql);
if(rc == SQLITE_OK) 
{return 0;}
else
{      fprintf(stderr,"SQL error: %s\n", err_msg);
       sqlite3_free(err_msg);
       return 1;
}
int find_sensor_by_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f)
{
 char * sql=NULL;
 char *err_msg = 0;
 //printf("finding sensor by timestamp");
 asprintf(&sql,"SELECT * from %s WHERE timestamp =%f  ", TO_STRING(TABLE_NAME),(double)ts);
 int rc = sqlite3_exec(conn, sql,f, 0, &err_msg);
 free(sql);
 if(rc == SQLITE_OK) 
    return 0;
 else
{    fprintf(stderr,"SQL error: %s\n", err_msg);
     sqlite3_free(err_msg);
     return 1;
}


}
int find_sensor_after_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f)
{
  char * sql=NULL;
  char *err_msg = 0;
 //printf("finding sensor > timestamp");
  asprintf(&sql,"SELECT * from %s WHERE timestamp >%f  ", TO_STRING(TABLE_NAME),(double)ts);
  int rc = sqlite3_exec(conn, sql,f, 0, &err_msg);
  free(sql);
  if(rc == SQLITE_OK) 
    return 0;
  else
    {fprintf(stderr,"SQL error: %s\n", err_msg);
     sqlite3_free(err_msg);
     return 1;
}


}
}
}




























