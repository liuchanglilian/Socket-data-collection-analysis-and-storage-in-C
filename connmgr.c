#define _GNU_SOURCE
#include <pthread.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "datamgr.h"
#include "config.h"
#include "lib/tcpsock.h"
#include <unistd.h>
#include "lib/dplist.h"
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <errno.h>
#include "connmgr.h"
#include "errmacros.h"
#define MAXVALUE 3
#ifndef TIMEOUT 
#error "TIMEOUT NOT DEFINED"
#endif
#define FIFO_NAME 	"MYFIFO"

typedef struct Element Element_structure;
void* element_copy(void *element);
void element_free(void **element);
int element_compare(void *x,void *y);
tcpsock_t *server;
char *send_buf; 
FILE *fp;
FILE *fpfifo;
pthread_mutex_t fifo_mutex3 = PTHREAD_MUTEX_INITIALIZER;
//sbuffer_t* buffer;
static void dpl_print( dplist_t * list );
struct tcpsock {
  long cookie;		// if the socket is bound, cookie should be equal to MAGIC_COOKIE
			// remark: the use of magic cookies doesn't guarantee a 'bullet proof' test
  int sd;		// socket descriptor
  char * ip_addr;	// socket IP address
  int port;   		// socket port number
  } ;
struct Element{
 struct tcpsock  *tcpInAdcancedstructure;
 time_t timestamp;
 int flag;
 int sensorid;
};
void* element_copy(void* src_element){
  Element_structure* element;
  element = malloc(sizeof(Element_structure));
  *element = *((Element_structure*)src_element);
  return (void*)element;
}

void element_free(void** element){
  free(*element);
  *element = NULL;
}

int element_compare(void* x, void* y)
{
    int a = *((int*) x); //Dereferencing an int pointer
    int b = *((int*) y);

    if(a == b)
    {
        return 0; 
    }

    if(a<b)
    {
        return -1;     
    }

    else
    {
        return 1;     
    }     
}
static dplist_t *sockList=NULL;
extern void fifoconnection()
{
    int presult;
    presult = pthread_mutex_lock(&fifo_mutex3);
    PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
    presult = mkfifo(FIFO_NAME, 0666);
    CHECK_MKFIFO(presult); 
    fpfifo = fopen(FIFO_NAME, "w");
    printf("syncing with reader ok(conn)\n");
    FILE_OPEN_ERROR(fpfifo);
}
void connmgr_listen(int port_number, sbuffer_t ** xbuffer)
{
 tcpsock_t *client;
 struct timeval tv;
 int maxfd;
 sensor_data_t datareceived;
 int readID=-2;
 int readValue=-2;
 int readTimestamp=-2;
 time_t current;
 int serverSD;
 fd_set fdset;
 fp = fopen("sensor_data_recv", "w");
 sockList = dpl_create(element_copy,element_free,element_compare);
 printf("Start socketListen(connmgr)\n");
 if(tcp_passive_open(&server, port_number) != TCP_NO_ERROR) exit(EXIT_FAILURE);
   
 if(server == NULL || (tcp_get_sd(server, &serverSD) == TCP_SOCKET_ERROR))  
 exit(EXIT_FAILURE);
 maxfd=serverSD;
 printf("The sd of the server is(connmgr) %d\n", serverSD);
while(1)
{ tv.tv_sec=TIMEOUT;
  tv.tv_usec=0;
  time(&current);
  FD_ZERO(&fdset);
  FD_SET(serverSD,&fdset);
  int sockListLength=dpl_size( sockList );
  for(int i=0;i<sockListLength;i++)
     {
       Element_structure* tem=(Element_structure*)dpl_get_element_at_index(sockList,i);
     time_t lastChange=tem->timestamp;
       double timebetween=difftime(current,lastChange);
       if (timebetween>TIMEOUT)
	      {  printf("timeout\n");
			 printf("T\n");
			  printf("I\n");
			   printf("M\n");
			    printf("E\n");
			     printf("O\n");
			      printf("U\n");
			       printf("T\n");
			        printf("?\n");
	          sockList=dpl_remove_at_index( sockList, i, 1);
		  }

}

for(int i=0;i<sockListLength;i++)
{ 
   Element_structure* tem=(Element_structure*)dpl_get_element_at_index(sockList,i);
   int usedSD=(tem->tcpInAdcancedstructure->sd);
   FD_SET(usedSD,&fdset);
 
}



int tag=select(maxfd+1,&fdset,NULL,NULL,&tv);
 
 if(tag==-1)
{
 printf("error select(connmgr)\n");
 break;
}
else if(tag==0)
{printf("select timeout(connmgr)\n");
	 
 break;
}
if (FD_ISSET(serverSD, &fdset))
{ printf("MAKE A NEW CONNECTION fdset serverSD(connmgr)\n");
  int clientSD;
  if(tcp_wait_for_connection(server,&client)!=TCP_NO_ERROR)    exit(EXIT_FAILURE);
	  
  if(client==NULL||tcp_get_sd(client,&clientSD)==TCP_SOCKET_ERROR)   exit(EXIT_FAILURE);
  printf("client SD  %d\n",clientSD);
  FD_SET(clientSD,&fdset);
  if(clientSD>maxfd)
  maxfd=clientSD;
	
  int lengthOfCurrentList=dpl_size( sockList );
	
  Element_structure * newElement=NULL;
  newElement=malloc(sizeof(Element_structure*));
  ((newElement)->tcpInAdcancedstructure)=client;
  (newElement->timestamp)=time(NULL);
  (newElement->flag)=0;
  sockList=dpl_insert_at_index( sockList, newElement, lengthOfCurrentList, 1);
  free(newElement);
  newElement=NULL;
  printf(" free new element have no error(connmgr)\n");
}

else
{   printf("Receive and send data oldcall(connmgr)\n");
	dpl_print(sockList);
	int getLength=dpl_size( sockList );
	printf("LENGTH%d",getLength);
    for(int k=0;k<getLength;k++){
        Element_structure* tem=(Element_structure*)dpl_get_element_at_index(sockList,k);
        if(tem==NULL){printf("error\n");
		      break;}
        int usedSD=(tem->tcpInAdcancedstructure->sd);
        printf("used SD when k=(connmgr)%d, %d\n",k,usedSD);
        if(FD_ISSET(usedSD,&fdset)){
	                            printf("equal,the k=(connmgr)%d\n",k);
		                        client=tem->tcpInAdcancedstructure;
		                        int datasize;
		                        datasize=sizeof(datareceived.id);
		                        readID=tcp_receive((tem->tcpInAdcancedstructure),&datareceived.id,&datasize);
                                if(readID==TCP_CONNECTION_CLOSED){
                                                               printf("there is no data anymore(connmgr)\n");
                                                               sockList=dpl_remove_at_index( sockList, k, 1);  
                                                               tcp_close(&client);
                                                               fifoconnection(); 
                                                               asprintf( &(send_buf),"%ld %d closed\n",time(NULL),tem->sensorid);
		                                                       if ( fputs( send_buf, fpfifo ) == EOF ){
                                                                                                       fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                                                                                                        exit( EXIT_FAILURE );
                                                                                                       } 
                                                               FFLUSH_ERROR(fflush(fpfifo));
                                                               int presult=pthread_mutex_unlock(&fifo_mutex3);	  
                                                               PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
                                                               break;





                                                                  }
                               int firsttimeflag=tem->flag;
	                           printf("flag%d",firsttimeflag);
		                      if(firsttimeflag==0){
		                                            printf("entering first tim");
			                                        fifoconnection();
                                                    asprintf( & send_buf,"%ld the newconnection from sensor node %d\n",time(NULL),datareceived.id);
		                                            if ( fputs(  send_buf,  fpfifo ) == EOF ){
                                                                                              fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                                                                                              exit( EXIT_FAILURE );
                                                                                             } 
		                                            FFLUSH_ERROR(fflush(fpfifo));
		                                            int presult=pthread_mutex_unlock(&fifo_mutex3);	  
	                                                PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );	  
	                                              }
		                     datasize=sizeof(datareceived.value);
		                     readValue=tcp_receive((tem->tcpInAdcancedstructure),&datareceived.value,&datasize);
                             printf("readValue=(connmgr)%d",readValue);
                             datasize=sizeof(datareceived.ts);
                             readTimestamp=tcp_receive((tem->tcpInAdcancedstructure),&datareceived.ts,&datasize);
                             printf("readTimestamp=(connmgr)%d",readTimestamp);
                             if((readID==TCP_NO_ERROR)&&(readValue==TCP_NO_ERROR)&&(readTimestamp==TCP_NO_ERROR)){
		                        fprintf(fp,"%" PRIu16 " %g %ld\n", datareceived.id,datareceived.value,datareceived.ts); 
	                            fflush(fp);
	                            if(xbuffer!=NULL){
	                                              int presult=sbuffer_insert(*xbuffer, &datareceived);
	                                              printf("the current size of Sbuffer(connmgr)%d \n",sizeOfBuffer(xbuffer));
	                                              if(presult==SBUFFER_FAILURE)
	                                              printf("insert failed(connmgr)\n");
		                                          }
		                       if(xbuffer==NULL)
		                          printf("XBUFFER IS NULL\n");
	                          Element_structure* new;
		                      new=malloc(sizeof(Element_structure*));
		                      (new->tcpInAdcancedstructure)=(tem->tcpInAdcancedstructure);
		                       time_t timenow;
		                       time(&timenow);
		                       new->timestamp=timenow;
		                       new->flag=1;
		                       new->sensorid=datareceived.id;
		                       sockList=dpl_remove_at_index( sockList, k, 1);
		                       sockList=dpl_insert_at_index(sockList, new, k, 1); 
		                       free(new);
		                        new=NULL;
		                        printf("the signal received to indicate the result(connmgr) %d",readID);
		                        if((readID==TCP_NO_ERROR)&&(readValue==TCP_NO_ERROR)&&(readTimestamp==TCP_NO_ERROR)){
									printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", datareceived.id,datareceived.value, datareceived.ts);
                                   }
        
   
        
      
		}	


}
}
}
}

}

static void dpl_print( dplist_t * list )
{
  int i,length;
  length = dpl_size(list);
  //assert(dplist_errno == DPLIST_NO_ERROR);
  for ( i = 0; i < length; i++)    
  {
  	Element_structure* element2 = (Element_structure*)dpl_get_element_at_index(list, i);
  	if(element2 != NULL){
  		//assert(dplist_errno == DPLIST_NO_ERROR);
  		printf("printing ListARRAT(connmgr)\n");
  		printf("element at index %d SD= %d  timestamp=%ld(connmgr) \n", i, element2->tcpInAdcancedstructure->sd,element2->timestamp);
  		printf("printing finished(connmgr)\n");
       // sensor_value_t  arr[dpl_size((connmgr)->historyvalue)];
  	
  	   }
  	else{
  		 printf("The list is NULL!(connmgr)");
  	    }
  
  } 
   

}



void connmgr_free()
{


 if (tcp_close( &server )!=TCP_NO_ERROR) exit(EXIT_FAILURE);
        printf("Test server is shutting down(connmgr)\n");    
 fclose(fp);
 dpl_free(&sockList, 1);

}
