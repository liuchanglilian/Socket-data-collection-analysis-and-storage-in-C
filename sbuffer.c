#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "sbuffer.h"
#include "errmacros.h"
struct sbuffer_data {
  struct sbuffer_data * next;
  sensor_data_t data;
};
struct sbuffer {
  sbuffer_data_t * head;
  sbuffer_data_t * tail;
};
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
int presult;
int sbuffer_init(sbuffer_t ** buffer)
{ 
  *buffer = malloc(sizeof(sbuffer_t));
  if (*buffer == NULL) return SBUFFER_FAILURE;
  (*buffer)->head = NULL;
  (*buffer)->tail = NULL;  
  return SBUFFER_SUCCESS; 

}
int sizeOfBuffer(sbuffer_t ** buffer)
{ int i=0;
  sbuffer_data_t * dummy;
  dummy = malloc(sizeof(sbuffer_data_t));
  if ((buffer==NULL) || (*buffer==NULL)) return -1;
  if ((*buffer)->head == NULL) return 0;
  dummy = (*buffer)->head;
  while(dummy!=NULL)
  {
    dummy=dummy->next;
    i=i+1;
  }
  free(dummy);
  return i;
}


int sbuffer_free(sbuffer_t ** buffer)
{
  sbuffer_data_t * dummy;
  presult = pthread_mutex_destroy( &data_mutex );
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  if ((buffer==NULL) || (*buffer==NULL))  return SBUFFER_FAILURE;
  while ( (*buffer)->head )
  {
    dummy = (*buffer)->head;
    (*buffer)->head = (*buffer)->head->next;
    free(dummy);
  }
  free(*buffer);
  *buffer = NULL;
  return SBUFFER_SUCCESS;		
}


int sbuffer_remove(sbuffer_t * buffer,sensor_data_t * data, int timeout)
{ 
  presult = pthread_mutex_lock(&data_mutex);
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  printf("entering remove(sbuffer)\n");
  sbuffer_data_t * dummy;
  if (buffer == NULL) {
                       printf("buffer=NULL\n");
                       presult = pthread_mutex_unlock( &data_mutex );
                       return SBUFFER_FAILURE;
                       }
  if (buffer->head == NULL){
                            printf("sleeping(sbuffer)\n");
                            sleep(timeout);
	                    if (buffer->head == NULL){
	                                              printf("still no data(sbuffer)\n"); 
                                                      presult = pthread_mutex_unlock( &data_mutex );
                                                      PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ ); 
                                                      return SBUFFER_NO_DATA;
       
                                                     }
                           }
  *data = buffer->head->data;
  dummy = buffer->head;
  printf("got dummy(sbuffer)\n");
  if (buffer->head == buffer->tail)  buffer->head = buffer->tail = NULL;  
  else  buffer->head = buffer->head->next;
  presult = pthread_mutex_unlock( &data_mutex );
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  free(dummy);
  return SBUFFER_SUCCESS;
}


int sbuffer_insert(sbuffer_t * buffer, sensor_data_t * data)
{ presult = pthread_mutex_lock(&data_mutex);
  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  sbuffer_data_t * dummy;
  dummy = malloc(sizeof(sbuffer_data_t));
  if (dummy == NULL) return SBUFFER_FAILURE;
  dummy->data = *data;
  dummy->next = NULL;
  
  if (buffer == NULL){
                       presult = pthread_mutex_unlock( &data_mutex );
                       PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
                       return SBUFFER_FAILURE;
                     }
  if (buffer->tail == NULL){ // buffer empty (buffer->head should also be NULL
                            printf("buffer is empty now(sbuffer)\n");
                            buffer->head = buffer->tail = dummy;
                            } 
  else {// buffer not empty
        printf("buffer is not empty(sbuffer)\n");
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next; 
      }
 presult = pthread_mutex_unlock( &data_mutex );
 PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  return SBUFFER_SUCCESS;
}




