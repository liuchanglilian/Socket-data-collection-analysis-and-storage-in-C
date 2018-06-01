#define _GNU_SOURCE
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
#define FIFO_NAME 	"MYFIFO"
#ifndef RUN_AVG_LENGTH

    #define  RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP 
    #error "undefined MAX_TEMP"
#endif
#ifndef SET_MIN_TEMP 
   #error "undefined MIN_TEMP"
   
#endif
#ifndef TIMEOUT 
#error "TIMEOUT NOT DEFINED"
#endif

typedef int element_t;
typedef struct Element Element_structure;
//extern fifo_mutex3
static dplist_t *datalist;

char *send_buf;
FILE* fpfifo;
extern pthread_mutex_t fifo_mutex3;
struct Element{
   sensor_id_t sensorID;
   room_id_t roomID;
  sensor_value_t runningAve;
  sensor_ts_t  lastModify;
 dplist_t* historyvalue;
  };
static Element_structure* search_sensor_at_id(sensor_id_t sensor_id);
static void* element_copy(void* src_element){
  Element_structure* element;
  element = malloc(sizeof(Element_structure));
  *element = *((Element_structure*)src_element);
  return (void*)element;
}

static void element_free(void** element){
  free(*element);
  *element = NULL;
}

static int element_compare(void* x,void* y){
	if(fabs((((Element_structure*)x)->runningAve)-(((Element_structure*)y)->runningAve))>0.0001){
		return 1;
	}else if(fabs((((Element_structure*)y)->runningAve)-(((Element_structure*)x)->runningAve))>0.0001){
		return -1;
	}else{
		return 0;
	}
}

void datamgr_parse_sensor_files(FILE * fp_sensor_map, FILE * fp_sensor_data)
{  printf("entering parse sensor files(datamgr)\n");
  sensor_id_t SI=0;
  room_id_t iRI=0;
  sensor_id_t iSI=0;
  room_id_t RI=0;
  sensor_value_t SV=0;
  sensor_ts_t  ST=0;
  sensor_id_t dataSI=0;
  int i=0;
  //dplist_t *datalist;
  datalist=dpl_create(&element_copy,&element_free,&element_compare);
  printf("created finished(datamgr)\n");
  ERROR_HANDLER(fp_sensor_map==NULL,"NO MAP");
 fseek(fp_sensor_map,0,0);

while(1)

{ if(fscanf(fp_sensor_map,"%hd %hd",&iRI,&iSI )==EOF) break;
  
    {  printf("aa(datamgr)\n");
       Element_structure* ei;
      ei=malloc(sizeof(Element_structure));
      RI=(room_id_t)iRI;
      printf(" RI=%d(datamgr)",RI);
      SI=(sensor_id_t)iSI;
      printf(" SI=%d(datamgr)",SI); 
      ei->sensorID=SI;
      printf("give value o ei(datamgr) \n"); 
      ei->roomID=RI;
      printf("give value o ei (datamgr)\n");
      ei->runningAve=0;
      ei->lastModify=0;
      dplist_t * history=NULL;
      history=dpl_create (&element_copy,&element_free,&element_compare);
      ei->historyvalue=history;
      datalist = dpl_insert_at_index(datalist,ei,i,true);
      printf("datalist:(datamgr) \n");
    //dpl_print(datalist);
   // fclose(fp_sensor_map);
     i++;
     free(ei);
    }
}
fseek(fp_sensor_data,0,0);
  while(fread(&dataSI,sizeof(sensor_id_t),1,fp_sensor_data) == 1 )
 {
	fread(&SV,sizeof(sensor_value_t),1,fp_sensor_data);
	fread(&ST,sizeof(sensor_ts_t),1,fp_sensor_data);
        printf("dataSI =(datamgr)%hd\n",dataSI);      
        printf("SV =(datamgr)%lf\n",SV);
   printf("ST =(datamgr)%ld\n",ST); 
  Element_structure* elementwithcorrectsensorid=search_sensor_at_id(dataSI);
  
    //ERROR_HANDLER(elementwithcorrectsensorid==NULL,"we don't have that sensorID");
    if(elementwithcorrectsensorid==NULL) printf("we don't have that sensorID(datamgr) %hd\n",dataSI);
		                             
	else
{									    
  elementwithcorrectsensorid->lastModify=ST;
elementwithcorrectsensorid->historyvalue=dpl_insert_at_index((elementwithcorrectsensorid->historyvalue),&SV,0,1);
printf("SV:%lf\n",SV);
//printf("historyvalue0=%lf\n",*((sensor_value_t*)dpl_get_element_at_index(elementwithcorrectsensorid->historyvalue, 0)));
//printf("historyvalue1=%lf\n",*((sensor_value_t*)dpl_get_element_at_index(elementwithcorrectsensorid->historyvalue, 1)));
//printf("historyvalue2=%lf\n",*((sensor_value_t*)dpl_get_element_at_index(elementwithcorrectsensorid->historyvalue, 2)));
if(dpl_size(elementwithcorrectsensorid->historyvalue)>=RUN_AVG_LENGTH)
{   
  sensor_value_t totaltillnow=0;
  for(int k=0;k<RUN_AVG_LENGTH;k++){
                                   sensor_value_t currentvalue;
	                            currentvalue=*((sensor_value_t*)dpl_get_element_at_index( elementwithcorrectsensorid->historyvalue, k));
	                             totaltillnow=totaltillnow+currentvalue;
	                            }
  elementwithcorrectsensorid->runningAve=totaltillnow/RUN_AVG_LENGTH;
  if((elementwithcorrectsensorid->runningAve) > SET_MAX_TEMP) 
		{fprintf(stderr,"it is too hot in (datamgr)%hd\n",elementwithcorrectsensorid->roomID); 
		 fifoconnection();
		 asprintf( &(send_buf),"%ld %d it is too hot\n",time(NULL),elementwithcorrectsensorid->roomID);
		 if ( fputs( send_buf, fpfifo ) == EOF )
                             {
                              fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                              exit( EXIT_FAILURE );
                              } 
		 FFLUSH_ERROR(fflush(fpfifo));
                 int presult=pthread_mutex_unlock(&fifo_mutex3);
                 PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
                }
	        if((elementwithcorrectsensorid->runningAve) < SET_MIN_TEMP) 
		fprintf(stderr,"it is too cold in (datamgr)%hd\n",elementwithcorrectsensorid->roomID); 
}           
           
 }          
           
           
           
           
       
 }  
   



 //dpl_print(datalist);
}
void datamgr_parse_sensor_data(FILE * fp_sensor_map, sbuffer_t ** buffer, sbuffer_t ** buffer2)
{
	 printf("entering parse sensor files(datamgr)\n");
 sensor_id_t SI=0;
 room_id_t iRI=0;
 sensor_id_t iSI=0;
 room_id_t RI=0;
 int i=0;
 datalist=dpl_create(&element_copy,&element_free,&element_compare);
 printf("created finished(datamgr)\n");
  ERROR_HANDLER(fp_sensor_map==NULL,"NO MAP");
 fseek(fp_sensor_map,0,0);

while(1)

{ if(fscanf(fp_sensor_map,"%hd %hd",&iRI,&iSI )==EOF) break;
  
    {  printf("aa\n");
       Element_structure* ei;
      ei=malloc(sizeof(Element_structure)); //ei is the inserting element
      RI=(room_id_t)iRI;
      //printf(" RI=%d",RI);
      //SI=(sensor_id_t)iSI;
      // printf(" SI=%d",SI); 
      ei->sensorID=SI;
      // printf("give value o ei (datamgr)\n"); 
      ei->roomID=RI;
       //  printf("give value o ei(datamgr) \n");
      ei->runningAve=0;
      ei->lastModify=0;
      dplist_t * history=NULL;   //store history value
      history=dpl_create (&element_copy,&element_free,&element_compare);
      ei->historyvalue=history;
      datalist = dpl_insert_at_index(datalist,ei,i,true);
      //printf("datalist:(datamgr) \n");
    //dpl_print(datalist);
   // fclose(fp_sensor_map);
       i++;
       free(ei);
    }
}
while(1)
{	printf("entering insert sbuffer(datamgr)\n");
	sensor_data_t * data=NULL;
	data=malloc(sizeof(sensor_data_t));
	int SBremovereturn=0;
	SBremovereturn=sbuffer_remove( *buffer,data,TIMEOUT);
	if(SBremovereturn==SBUFFER_FAILURE)
	{   printf("Sbuffer failure(datamgr)\n");
	    break;
	}
	if(SBremovereturn==SBUFFER_NO_DATA)
	{  printf("failure(datamgr)\n");
	   sleep(TIMEOUT);
	   SBremovereturn=sbuffer_remove( *buffer, data,TIMEOUT);
	   if(SBremovereturn==SBUFFER_NO_DATA)
	   {
		   printf("the sbuffer doesn't exit(datamgr)");
		   break;
		  
	   }	
	}
	if(SBremovereturn==SBUFFER_SUCCESS)
	{  printf("succed(datamgr)\n");
           int insertSucced=sbuffer_insert(*buffer2,data);
           if(insertSucced==SBUFFER_SUCCESS)
           printf("insert succeed(datamgr)\n");
           
           else   printf("insert not succeed(datamgr)\n");
           printf("the size of buffer2=(datamgr) %d",sizeOfBuffer(buffer2));
        
    }
    printf("the sensor id=(datamgr)%d\n",data->id);
	printf("the value=(datamgr)%g \n",data->value);
	printf("the ts=(datamgr)%ld \n",data->ts);
	int dataSI=data->id;
    sensor_value_t SV=data->value;
	sensor_ts_t  ST=data->ts;
	Element_structure* elementwithcorrectsensorid=search_sensor_at_id(dataSI);
	if(elementwithcorrectsensorid==NULL)
		 { printf("we don't have that sensorID (datamgr)%hd\n",data->id);
		   fifoconnection();
		   asprintf( &send_buf,"%ld we don't have that sensorID %hd\n",time(NULL),dataSI);
		   if ( fputs( send_buf, fpfifo ) == EOF )
                       {
                        fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                         exit( EXIT_FAILURE );
                      } 
		  FFLUSH_ERROR(fflush(fpfifo));
          int  presult = pthread_mutex_unlock(&fifo_mutex3);	
          PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
	     }
	      else{
                   elementwithcorrectsensorid->lastModify=ST;
                   elementwithcorrectsensorid->historyvalue=dpl_insert_at_index((elementwithcorrectsensorid->historyvalue),&SV,0,1);
                   printf("SV:%lf\n",SV);
//printf("historyvalue0=%lf\n",*((sensor_value_t*)dpl_get_element_at_index(elementwithcorrectsensorid->historyvalue, 0)));
//printf("historyvalue1=%lf\n",*((sensor_value_t*)dpl_get_element_at_index(elementwithcorrectsensorid->historyvalue, 1)));
//printf("historyvalue2=%lf\n",*((sensor_value_t*)dpl_get_element_at_index(elementwithcorrectsensorid->historyvalue, 2)));
                  if(dpl_size(elementwithcorrectsensorid->historyvalue)>=RUN_AVG_LENGTH){
                    sensor_value_t totaltillnow=0;
                    for(int k=0;k<RUN_AVG_LENGTH;k++){
                       sensor_value_t currentvalue;
	                   currentvalue=*((sensor_value_t*)dpl_get_element_at_index( elementwithcorrectsensorid->historyvalue, k));
	                   totaltillnow=totaltillnow+currentvalue;
	                   }
	               elementwithcorrectsensorid->runningAve=totaltillnow/RUN_AVG_LENGTH;
	               if((elementwithcorrectsensorid->runningAve) > SET_MAX_TEMP) {
		                fprintf(stderr,"it is too hot in (datamgr)%hd\n",elementwithcorrectsensorid->roomID); 
				        fifoconnection();
		                asprintf( &send_buf,"%ld it is too hot in %hd\n",time(NULL),elementwithcorrectsensorid->roomID);
		               if ( fputs( send_buf, fpfifo ) == EOF )
                          {
                           fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                           exit( EXIT_FAILURE );
                          } 
		          FFLUSH_ERROR(fflush(fpfifo));
		          int presult=pthread_mutex_unlock(&fifo_mutex3);	
                  PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
		    
		       }
		    
	        if((elementwithcorrectsensorid->runningAve) < SET_MIN_TEMP) {
		          fprintf(stderr,"it is too cold in (datamgr)%hd\n",elementwithcorrectsensorid->roomID); 
		          asprintf( &send_buf,"%ld it is too cold in %hd\n",time(NULL),elementwithcorrectsensorid->roomID);
		    if ( fputs( send_buf, fpfifo ) == EOF ){
                  fprintf( stderr, "Error writing data to fifo(connmgr)\n");
                  exit( EXIT_FAILURE );
            } 
		    FFLUSH_ERROR(fflush(fpfifo));
            int presult=pthread_mutex_unlock(&fifo_mutex3);
            PTHREAD_ERR_HANDLER( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
			   
			   
			}
		   
}
		
		}
		
	
free(data);	
}

//dpl_print(datalist);
}

Element_structure * search_sensor_at_id(sensor_id_t sensor_id){
	for(int i = 0; i<dpl_size(datalist);i++){ 
		if((sensor_id_t)(((Element_structure *)dpl_get_element_at_index(datalist,i))->sensorID)==sensor_id)
		{  printf("find sensorID(datamgr)\n");
		   printf("looking for sensorID(datamgr) %d\n",sensor_id);
           return dpl_get_element_at_index(datalist,i);
		}
	}
	
    return NULL;
}
void datamgr_free()
{   for(int i = 0; i<dpl_size(datalist);i++){
	  
	   Element_structure * a=dpl_get_element_at_index( datalist, i);
	   dpl_free(&(a->historyvalue),1);
	}
	dpl_free(&datalist,1);

}
uint16_t datamgr_get_room_id(sensor_id_t sensor_id)
{
  Element_structure * element=search_sensor_at_id(sensor_id);
  ERROR_HANDLER(element==NULL,"NO such sensor for get room id(datamgr)");
  room_id_t currentroom=element->roomID;
  return currentroom;

}
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id)
{
 Element_structure * element=search_sensor_at_id(sensor_id);
 ERROR_HANDLER(element==NULL,"NO such sensor for get avg(datamgr)");
 sensor_value_t ave=element->runningAve;
 return ave;


}

time_t datamgr_get_last_modified(sensor_id_t sensor_id)
{
  Element_structure * element=search_sensor_at_id(sensor_id);
  ERROR_HANDLER(element==NULL,"NO such sensor for get last modified(datamgr)");
  sensor_ts_t lastchange=element->lastModify;
  return lastchange;
}
int datamgr_get_total_sensors()
{
   
   int length = dpl_size(datalist);
   return length;
  
  
}

