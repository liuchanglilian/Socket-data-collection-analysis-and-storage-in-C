#define _GNU_SOURCE
#include <inttypes.h>
#include <assert.h>
#include <unistd.h>
#include "connmgr.h"
#include "sensor_db.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include "datamgr.h"
#include <unistd.h>
#include "lib/dplist.h"
#include <sys/types.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <stdio.h> 
#include "errmacros.h"
#ifndef TIMEOUT 
#error "TIMEOUT NOT DEFINED"
#endif
#define CHILD_POS	"\t\t\t"
#define FIFO_NAME 	"MYFIFO"
#define MAX 300
int portNumber=5678;
sbuffer_t * Sbuffer;
sbuffer_t * Sbuffersecond;
int presult;
pthread_t conmanager;
pthread_t datamanager;
pthread_t sqlmanager;
pthread_mutex_t fifo_mutex2 = PTHREAD_MUTEX_INITIALIZER;
void *connection(void *id)
{ printf("entering connection thread(main)\n");
  connmgr_listen(portNumber,&Sbuffer);
  connmgr_free();
  return NULL;
}

void *datamanagement(void *id)
{  printf("entering datamanagement thread(main)\n");
   FILE * fp= NULL;
   fp=fopen("room_sensor.map","r");
   if(fp==NULL)
   printf("fp=null\n");
   printf("opened fp\n");
   datamgr_parse_sensor_data(fp,&Sbuffer,&Sbuffersecond);
   printf("the size of buffer2=(main) %d",sizeOfBuffer(&Sbuffersecond));
   return NULL;
}

void *sqlmanagement(void *id)
{  printf("entering sqlmanagement thread(main)\n");
	while(1)
	{
		DBCONN* connection=init_connection('1');
		int i=0; //counting for turns trying to connect
		while((connection==NULL)&&(i<3))
		{
			//printf("the i is %d(in the thread of sql)",i);
			connection=init_connection('1');
			i=i+1;
				usleep(TIMEOUT*1000000);
			
		}
		if((i=3)&&(connection==NULL))
		{ 
			break;
			
		}
		storagemgr_parse_sensor_data(connection, &Sbuffersecond);
		
		
	 }
	 return NULL;
   
   

}

void run_child(){
	 FILE * fp_log;
	 fp_log = fopen("gateway.log", "w");
	 printf(CHILD_POS"entering child\n");
	 pid_t my_pid, parent_pid; 
         my_pid = getpid();
	 parent_pid = getppid();
	 printf(CHILD_POS "Child process (pid = %d) of parent (pid = %d) ...\n", my_pid, parent_pid);
	 FILE *fp=NULL; 
         int result;
 // char *send_buf; 
        printf(CHILD_POS"making fifo\n");
        result = mkfifo(FIFO_NAME, 0666);
        CHECK_MKFIFO(result); 
        printf(CHILD_POS"checked passed\n");
        char *str_result;
        char recv_buf[MAX]; 
        fp = fopen(FIFO_NAME, "r");
        printf(CHILD_POS"syncing with writer ok\n");
        if(fp==NULL){
                   printf(CHILD_POS"fp is null\n");}
                   FILE_OPEN_ERROR(fp);
                   int sequenceNumber=0;
		  do {
                      presult = pthread_mutex_lock(&fifo_mutex2);
                      PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
                      str_result = fgets(recv_buf, MAX, fp);
                      presult = pthread_mutex_unlock(&fifo_mutex2);
                      PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
                      if ( str_result != NULL )
                          { 
                           printf(CHILD_POS"GOT INFORMATION: %s", recv_buf); 
                           fprintf(fp_log, "%d  %s\n", sequenceNumber, recv_buf);
                           fflush(fp_log);
                           sequenceNumber++;
      
      
                          }
                     } while ( str_result != NULL ); 
	
	
	        result = fclose( fp );
                FILE_CLOSE_ERROR(result);
  
                exit(EXIT_SUCCESS);
}
int main(void)
{   
  pid_t my_pid, child_pid;
  my_pid = getpid();
  printf("Parent process (pid = %d) is started ...\n", my_pid);
  printf("making child\n");
  child_pid = fork();
  SYSCALL_ERROR(child_pid);
  if ( child_pid == 0  )
  {  
    run_child();
  }

  else
  {  
    printf("Parent process (pid = %d) has created child process (pid = %d)...\n", my_pid, child_pid);
    FILE *fp; 
    int result;
 // char *send_buf; 
    printf("making fifo\n");
    result = mkfifo(FIFO_NAME, 0666);
    CHECK_MKFIFO(result); 
    printf("checked passed\n");
    fp = fopen(FIFO_NAME, "w"); 
    if(fp==NULL)
    printf("fp is null\n");
    else printf("syncing with reader ok\n");
    }
    int bufferinit=sbuffer_init(&Sbuffer);
    printf("the return of init is(main) %d",bufferinit);
    printf("the initial size of Sbuffer(main)%d \n",sizeOfBuffer(&Sbuffer));
    if (bufferinit==SBUFFER_FAILURE)
     {   printf("buffer init failed(main)\n");
     }
    if (bufferinit==SBUFFER_SUCCESS)
       printf("buffer init succed(main)\n");
       printf("the initial size of Sbuffer(main)%d \n",sizeOfBuffer(&Sbuffer));
       bufferinit=sbuffer_init(&Sbuffersecond);
       printf("the return of init is(main) %d",bufferinit);
       printf("the initial size of Sbuffer(main)%d \n",sizeOfBuffer(&Sbuffer));
   if (bufferinit==SBUFFER_FAILURE)
      printf("buffer init failed(main)\n");
     
   if (bufferinit==SBUFFER_SUCCESS)
       printf("buffer init succed(main)\n");
   printf("the initial size of Sbuffersecond(main)%d \n",sizeOfBuffer(&Sbuffersecond));
   presult=pthread_create(&conmanager,NULL,&connection,NULL);
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  if (presult != 0) {
                     printf("create thread tcp recv_thread failed(main)/n");
                  }

  printf("Creating  next thread(main)\n");
  if (presult == 0){
                    printf("succeed\n");
                   }
  presult=pthread_create(&datamanager,NULL,&datamanagement,NULL);
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  if (presult!= 0) {
               printf("create thread tcp recv_thread failed(main)/n");
               return 1;
                    }

  if (presult == 0) 
  printf("succeed(main)\n");
  presult=pthread_create(&sqlmanager,NULL,&sqlmanagement,NULL);
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  if (presult!= 0) {
               printf("create thread sqlmanagement failed(main)/n");
               return 1;
        }
 if (presult == 0) 
 printf("succeed(main)\n");
//Dealing with FiFo stuffs
presult= pthread_join( conmanager, NULL );
PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
 if (presult == 0) 
 printf("joined connmanager succeed(main)\n");
 presult= pthread_join( datamanager, NULL );
 PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
 if (presult == 0) printf("joined datamanager succeed(main)\n");
 presult= pthread_join( sqlmanager, NULL );
 PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
 if (presult == 0) printf("joined sqlmanager succeed(main)\n");
 sbuffer_free(&Sbuffer);
 sbuffer_free(&Sbuffersecond);
 pthread_exit( NULL );
 PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );


}
