#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
	#define DEBUG_PRINTF(...) 									         \
		do {											         \
			fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	 \
			fprintf(stderr,__VA_ARGS__);								 \
			fflush(stderr);                                                                          \
                } while(0)
#else
	#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition,err_code)\
	do {						            \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");    \
            assert(!(condition));                                    \
        } while(0)

        
/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
  dplist_node_t * prev, * next;
  void * element;
};

struct dplist {
  dplist_node_t * head;
  void * (*element_copy)(void * src_element);			  
  void (*element_free)(void ** element);
  int (*element_compare)(void * x, void * y);
};


dplist_t * dpl_create (// callback functions
			  void * (*element_copy)(void * src_element),
			  void (*element_free)(void ** element),
			  int (*element_compare)(void * x, void * y)
			  )
{
  dplist_t * list;
  list = malloc(sizeof(struct dplist));
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);
  list->head = NULL;  
  list->element_copy = element_copy;
  list->element_free = element_free;
  list->element_compare = element_compare; 
  return list;
}

void dpl_free(dplist_t ** list, bool free_element)
{if(free_element)
 {
  dplist_node_t * dummy;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  if (((*list)->head)!= NULL) {
                                dplist_node_t * dummy_next;
                                for(dummy=(*list)->head;(dummy!=NULL);)
                                {printf("free!!!!\n");
				                 dummy_next=dummy->next;
				                (((*list) -> element_free))(&(dummy->element));
				                free(dummy);
			                    dummy=dummy_next;
                              }
                             }
  free(*list);
  *list=NULL;
 }
 else
 {
  dplist_node_t * dummy;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  if (((*list)->head)!= NULL) 
  {
	  dplist_node_t * dummy_next;
      for(dummy=(*list)->head;(dummy!=NULL);)
         {printf("free!!!!\n");
          dummy_next=dummy->next;
          free(dummy);
          dummy=dummy_next;
         }
  }
  free(*list);
  *list=NULL;

 }
}

dplist_t * dpl_insert_at_index(dplist_t * list, void * element, int index, bool insert_copy)
{ 
   if(insert_copy)
 {
  dplist_node_t * ref_at_index, * list_node;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  list_node = malloc(sizeof(dplist_node_t));
  DPLIST_ERR_HANDLER(list_node==NULL,DPLIST_MEMORY_ERROR);
  (list_node->element)=(*(list -> element_copy))(element);
  if (list->head == NULL)  
  { 
    list_node->prev = NULL;
    list_node->next = NULL;
    list->head = list_node;   //header is empty and add the node directly after it
    // pointer drawing breakpoint
  } else if (index <= 0)  
  { // covers case 2 
    list_node->prev = NULL;
    list_node->next = list->head;
    list->head->prev = list_node;
    list->head = list_node; 
  } else 
  {
    ref_at_index = dpl_get_reference_at_index(list, index);  
    assert( ref_at_index != NULL);
    // pointer drawing breakpoint
    if (index < dpl_size(list))
    { // covers case 4
	  list_node->prev = ref_at_index->prev;
      list_node->next = ref_at_index;
      ref_at_index->prev->next = list_node;
      ref_at_index->prev = list_node;   //header is not empty and the node need to be inserted into the lis
    } else
    { // covers case 3 
      assert(ref_at_index->next == NULL);
      list_node->next = NULL;
      list_node->prev = ref_at_index;
      ref_at_index->next = list_node;    //the node is added as the last one of the list
      // pointer drawing breakpoint
    }
  }
  return list;
   }

  else
 {
  dplist_node_t * ref_at_index, * list_node;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  list_node = malloc(sizeof(dplist_node_t));
  DPLIST_ERR_HANDLER(list_node==NULL,DPLIST_MEMORY_ERROR);
  list_node->element = element;
  // pointer drawing breakpoint
  if (list->head == NULL)  
  { // covers case 1 
    list_node->prev = NULL;
    list_node->next = NULL;
    list->head = list_node;   //header is empty and add the node directly after it
    // pointer drawing breakpoint
  } else if (index <= 0)  
  { // covers case 2 
    list_node->prev = NULL;
    list_node->next = list->head;
    list->head->prev = list_node;
    list->head = list_node;   //header is not empty, but not need to be add before the header
    // pointer drawing breakpoint
  } else 
  {
    ref_at_index = dpl_get_reference_at_index(list, index);  
    assert( ref_at_index != NULL);
    // pointer drawing breakpoint
    if (index < dpl_size(list))
    { // covers case 4
      list_node->prev = ref_at_index->prev;
      list_node->next = ref_at_index;
      ref_at_index->prev->next = list_node;
      ref_at_index->prev = list_node;   //header is not empty and the node need to be inserted into the list
      // pointer drawing breakpoint
    } else
    { // covers case 3 
      assert(ref_at_index->next == NULL);
      list_node->next = NULL;
      list_node->prev = ref_at_index;
      ref_at_index->next = list_node;    //the node is added as the last one of the list
      // pointer drawing breakpoint
    }
  }
  return list;
  }
}

dplist_t * dpl_remove_at_index( dplist_t * list, int index, bool free_element)
{ 
  if(free_element)
  { 
  dplist_node_t * ref_at_index;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  // pointer drawing breakpoint
  if (list->head == NULL)  
   return list;
   else if (index <= 0)  
  { 
    ref_at_index=list->head;
    list->head=ref_at_index->next;
     if((ref_at_index->next)!=NULL)
     ref_at_index->next->prev=NULL;
     (list->element_free)(&(ref_at_index->element));
     free(ref_at_index);
  } else 
     {  
       ref_at_index = dpl_get_reference_at_index(list, index);    //ruoyaocharu de list bushi kongde, er qie ye bushi yao charudao list de zuiqianmian
       assert( ref_at_index != NULL);
    // pointer drawing breakpoint
       if (index < (dpl_size(list)-1))
          { // covers case 4
           ref_at_index->prev->next = ref_at_index->next;
           if((ref_at_index->next)!=NULL)
              ref_at_index->next->prev = ref_at_index->prev;
           (list->element_free)(&(ref_at_index->element));
           free(ref_at_index);
      // pointer drawing breakpoint
         } else
                { // covers case 3 ,yao charu de difang zai zui mowei
		           if((ref_at_index->prev)!=NULL)  ref_at_index->prev->next = NULL; 
                     else  list->head=NULL;
	              (list->element_free)(&(ref_at_index->element));
                  free(ref_at_index);  
                }
  }
  return list;
   }
  else
  {   dplist_node_t * ref_at_index;
      DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  // pointer drawing breakpoint
     if (list->head == NULL)  
          return list;
     else if (index <= 0)  
            { // covers case 2 
               ref_at_index=list->head;
              if((ref_at_index->next)!=NULL)
                list->head=ref_at_index->next;
              else
                 list->head=NULL;
            if((ref_at_index->next)!=NULL)
              ref_at_index->next->prev=NULL;
              free(ref_at_index);
              } else 
                   {
                    ref_at_index = dpl_get_reference_at_index(list, index);    //ruoyaocharu de list bushi kongde, er qie ye bushi yao charudao list de zuiqianmian
                    assert( ref_at_index != NULL);
                    // pointer drawing breakpoint
                   if (index < (dpl_size(list)-1))
                      { // covers case 4
                       ref_at_index->prev->next = ref_at_index->next;
                       ref_at_index->next->prev = ref_at_index->prev;
                        free(ref_at_index);
      // pointer drawing breakpoint
                      } else
                           { // covers case 3 ,yao charu de difang zai zui mowei
	                        if(ref_at_index->prev!=NULL)
                                ref_at_index->prev->next = NULL;  
                            else
                                list->head=NULL;
                          free(ref_at_index);  
      // pointer drawing breakpoint
                          }
                     }
  return list;




   
   }
}


int dpl_size( dplist_t * list )
{
  
  dplist_node_t * dummy;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  if (list->head == NULL ) return 0;
  else{
	  int count = 1;
  for( dummy = list->head; dummy->next != NULL ; dummy = dummy->next) 
  {count++;
  }
  return count;
}
}

dplist_node_t * dpl_get_reference_at_index( dplist_t * list, int index )
{
 
 
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  if (list->head == NULL ) return NULL;
    else if (index<=0) return list->head;
    else{    int count=0;
		      dplist_node_t * dummy;
           for ( dummy = list->head; dummy->next != NULL ; dummy = dummy->next, count++) 
           { 
             if (count >= index) return dummy;
           }  
           return dummy;  
         }
}

void * dpl_get_element_at_index( dplist_t * list, int index )
{
  dplist_node_t * dummy;
  //DPLIST_ERR_HANDLER(list,DPLIST_INVALID_ERROR);
  if (list->head == NULL ) return 0;
  else if (index<=0) return (list->head->element);
       else{int count=0;
             for ( dummy = list->head; dummy->next != NULL ; dummy = dummy->next, count++) 
                 { 
                   if (count >= index) return (dummy->element);
                  }  
                  return (dummy->element);
}
    
}

int dpl_get_index_of_element( dplist_t * list, void * element )
{
   
  dplist_node_t * dummy;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  if ((list->head )== NULL ) return -1;
  else{
	  int count = 0;
  for( dummy = list->head; dummy!= NULL ; dummy = dummy->next,count++) 
  {
   if ((*(list->element_compare))(element,dummy->element)==0) 
       {
       return count;
       
                                    }
   }
 
   return -1;
  }

}

// HERE STARTS THE EXTRA SET OF OPERATORS //

// ---- list navigation operators ----//
  
dplist_node_t * dpl_get_first_reference( dplist_t * list )
{  if(list->head==NULL)
	{return NULL;}
	else{
    dplist_node_t *a= dpl_get_reference_at_index(list, 0);
    return a;
}
}

dplist_node_t * dpl_get_last_reference( dplist_t * list )
{    if((list->head)==NULL)
	  {return NULL;
		  }
	  else{//printf("not empty");
     dplist_node_t * dummy;
     int i=0;
     
      for ( dummy = list->head; dummy->next!= NULL ; dummy = dummy->next)  
      {i++;}
      return dummy; 
  } 
}

dplist_node_t * dpl_get_next_reference( dplist_t * list, dplist_node_t * reference )
{   if(list->head==NULL)
	{return NULL;}
	else
	{if(reference==NULL)
		{return NULL;}
	 else {
		if(dpl_get_index_of_reference(list,reference)==-1)
			{return NULL;}
		else
			{dplist_node_t * next;
			next= reference->next;
			return next;}
	 }
    }
}

dplist_node_t * dpl_get_previous_reference( dplist_t * list, dplist_node_t * reference )
{ if(list->head==NULL)
	{return NULL;}
	
	else
 {   if(reference==NULL)
	  {dplist_node_t * last=dpl_get_last_reference(list);
		  return last;}
	   else {
		    if(dpl_get_index_of_reference(list,reference)==-1)
	        {return NULL;}
	 else
     {dplist_node_t * prev;
    prev= reference->prev;
    return prev;}
    
}
}
}

// ---- search & find operators ----//  
  
void * dpl_get_element_at_reference( dplist_t * list, dplist_node_t * reference )
{   
	if((list->head)==NULL)
	{   //printf("list is empty\n");
		return NULL;}
	else{ void * element;
		if(reference==NULL)
		{  // printf("reference is NULL\n");
			dplist_node_t * last=dpl_get_last_reference(list);
		 element=last->element;
		 return element;
	    }
    else if((dpl_get_index_of_reference(list,reference))==-1)
          {  // printf("can not find the reference\n");
			  return NULL;}
          else
             {element=reference->element;
               return element;}
               }
}

dplist_node_t * dpl_get_reference_of_element( dplist_t * list, void * element )
{
    dplist_node_t * dummy;
    
     DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
    if ((list->head )== NULL ) return NULL;
    else{
		 int count=0;
      for (dummy = list->head; dummy!= NULL ; dummy = dummy->next,count++)  
      { 
         if ((*(list->element_compare))(element,dummy->element)==0) 
             { //printf("find the reference,%d\n",count);
				 return dummy;
               
              }
       }
     
      return NULL;  
}
}

int dpl_get_index_of_reference( dplist_t * list, dplist_node_t * reference )
{
 
  dplist_node_t * dummy;
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  if ((list->head )== NULL ) return -1;
  else{ if(reference==NULL)
	    {return dpl_size(list)-1;}
	    else
	    {
	 int count = 0;
  for( dummy = list->head; dummy!= NULL ; dummy = dummy->next,count++) 
  {//printf("searching %d\n",count);
   if (dummy==reference) 
       {
        printf("count is %d\n",count);
        return count;
                                    }
   }

   return -1;
}
  }
}
  
// ---- extra insert & remove operators ----//

dplist_t * dpl_insert_at_reference( dplist_t * list, void * element, dplist_node_t * reference, bool insert_copy )
{
    if(insert_copy)
 
 {  //printf("inserting copy???element \n");
  
  
    dplist_node_t * list_node,*dummy;
   

    DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
 
    list_node = malloc(sizeof(dplist_node_t));

    DPLIST_ERR_HANDLER(list_node==NULL,DPLIST_MEMORY_ERROR);

   (list_node->element)=(*(list -> element_copy))(element);
    if( reference==NULL)
      {dplist_node_t * last=dpl_get_last_reference(list);
	   if(last!=NULL)
     { list_node->next=NULL;
      list_node->prev=last;
      last->next=list_node;
      //printf("inserted\n");
      return list;}
      else
      {list->head=list_node;
	   list_node->prev=NULL;
	   list_node->next=NULL;
	   return list;
      }
  }  
    else
        {   int count = 0;
			for( dummy = list->head; dummy!= NULL ; dummy = dummy->next,count++) 
            {//printf("searching %d\n",count);
             if (dummy==reference) 
                                  {
                                  // printf("count is %d\n",count);
                                   list_node->prev = dummy->prev;
                                   list_node->next = dummy;
                                   if(dummy->prev!=0)
                                   {dummy->prev->next = list_node;}
                                   else
                                   {list->head=list_node;}
                                    dummy->prev = list_node;
                                   // printf("find referenceand intered");
                                    return list;
                                   }
             }
             
        // printf("not find reference");
         free(list_node);
         return list;
        }

}
  else
 { //printf("inserting no copy???? index is /n");
 
  dplist_node_t * dummy, * list_node;
  
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
  list_node = malloc(sizeof(dplist_node_t));
  DPLIST_ERR_HANDLER(list_node==NULL,DPLIST_MEMORY_ERROR);
  list_node->element = element;
  if( reference==NULL)
      {dplist_node_t * last=dpl_get_last_reference(list);
		   if(last!=NULL)
      {list_node->next=NULL;
      list_node->prev=last;
      last->next=list_node;
      return list;
      }
       else
      {list->head=list_node;
	   list_node->prev=NULL;
	   list_node->next=NULL;
	   return list;
      }
      }  
    else
        {   int count = 0;
			for( dummy = list->head; dummy!= NULL ;dummy = dummy->next, count++) 
            {//printf("searching %d\n",count);
             if (dummy==reference) 
                                  {
                                   //printf("count is %d\n",count);
                                   list_node->prev = dummy->prev;
                                   list_node->next = dummy;
                                    dummy->prev->next = list_node;
                                    dummy->prev = list_node;
                                    return list;
                                   }
             }
        // printf("not find reference");
         free(list_node);
         return list;
        }
 }
}

dplist_t * dpl_insert_sorted( dplist_t * list, void * element, bool insert_copy )
{  
       dplist_node_t * dummy;
       dplist_t * list_after;
    	DPLIST_ERR_HANDLER((list==NULL),DPLIST_INVALID_ERROR);
	if(list->head == NULL) { list_after=dpl_insert_at_index(list, element, 0, insert_copy);
		                     return list_after;
						 }
   else{int count;
	for(dummy = list->head, count = 0; dummy!= NULL; dummy = dummy->next, count++)
	{
		int compare = (*(list->element_compare))(element,dummy->element);
	  // if( compare == 0   ) return list;
	 if(( compare == -1)||(compare==0)  ) {list_after=dpl_insert_at_index(list, element, count, insert_copy);
		                                  return list_after;}
	   
	}
	
	   
	return dpl_insert_at_index(list, element, count, insert_copy);

      
}
}







dplist_t * dpl_remove_at_reference( dplist_t * list, dplist_node_t * reference, bool free_element )
{ if(list->head==NULL)
	{return list;
	}
  else{
	
	     dplist_node_t * dummy;
	     int count;
        if(free_element)
          { 
         DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
            if(reference==NULL){
		
		   dplist_node_t *  ref_at_index=dpl_get_last_reference( list );
		    if(ref_at_index->prev!=NULL)
           { ref_at_index->prev->next = NULL;  }
           else
           {list->head=NULL;
		   }
           (list->element_free)(&(ref_at_index->element));
          free(ref_at_index);  
          return list;
             } 
   
            else  //REFERENCE!=NULL
                 {for( dummy = list->head, count = 0; dummy!= NULL ; dummy = dummy->next,count++) 
                     {
                           if (dummy==reference) 
                                  {  if((dummy->prev!=NULL)&&(dummy->next!=NULL))
									  {dummy->prev->next = dummy->next;
                                       dummy->next->prev = dummy->prev;
									  }
									  if((dummy->prev=NULL)&&(dummy->next!=NULL))
									  {list->head=dummy->next;
										   dummy->next->prev = NULL;
										  
									  }
									  if((dummy->prev!=NULL)&&(dummy->next=NULL))
									  {dummy->prev->next=NULL;
									  }
  
                                    (list->element_free)(&(dummy->element));
                                    free(dummy);
                                    return list;
                                   }
             }
        
          return list;
        }
    
   }
  else
      {  
         DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
         if(reference==NULL)
          { dplist_node_t *  ref_at_index=dpl_get_last_reference( list );
             if(ref_at_index->prev!=NULL)
           { ref_at_index->prev->next = NULL;  }
           else
           {list->head=NULL;
		   }
            free(ref_at_index);  
            return list;
          }
         else
        {for( dummy = list->head, count = 0; dummy!= NULL ; dummy = dummy->next,count++) 
            {
             if (dummy==reference) 
                                  {if((dummy->prev!=NULL)&&(dummy->next!=NULL))
									  {dummy->prev->next = dummy->next;
                                       dummy->next->prev = dummy->prev;
									  }
									  if((dummy->prev=NULL)&&(dummy->next!=NULL))
									  {list->head=dummy->next;
										   dummy->next->prev = NULL;
										  
									  }
									  if((dummy->prev!=NULL)&&(dummy->next=NULL))
									  {dummy->prev->next=NULL;
									  }
                                    free(dummy);
                                    return list;;
                                   }
             }
          return list;
        }
        
    }
}

}
dplist_t * dpl_remove_element( dplist_t * list, void * element, bool free_element )
{ 
   dplist_node_t * reference=dpl_get_reference_of_element(list,element );
   dplist_node_t * dummy;
   int count=0;
  if(free_element)
  {
    if(reference==NULL){
      return list;
      } 
   
  else
        {   
			for( dummy = list->head; dummy!= NULL ; dummy = dummy->next,count++) 
            {
             if (dummy==reference) 
                                  {  
									  if((dummy->next)!=NULL)// NEXT NOT NULL
                                   {   if((dummy->prev)!=NULL)//NEXT NOT NULL PRE NOT NULL
									   {dummy->prev->next = dummy->next;
										dummy->next->prev = dummy->prev;}
									   else
									   {list->head=dummy->next;
										 dummy->next->prev = NULL;}//NEXT NOT NULL PRE IS NULL
										   }
                                    else                 //NEXT IS NULL
                                    {
										
										if((dummy->prev)!=NULL ) //NEXT IS  NULL PRE IS NOT NULL
                                          {dummy->prev->next=NULL;
								     
										   }
                                        else                     //NEXT IS NULL PRE IS NULL
                                            {list->head=NULL;}
										}

                                    (list->element_free)(&(dummy->element));
                                    free(dummy);
                                    return list;
                                   }
             }
         return list;
        }

   }
  else
      {
         if(reference==NULL)
          {
            return list;
          }
         else
        {for( dummy = list->head, count = 0; dummy!= NULL ; dummy = dummy->next,count++) 
            {
             if (dummy==reference) 
                                  { 
									  if((dummy->next)!=NULL)// NEXT NOT NULL
                                   {   if((dummy->prev)!=NULL)//NEXT NOT NULL PRE NOT NULL
									   {dummy->prev->next = dummy->next;
										dummy->next->prev = dummy->prev;}
									   else
									   {list->head=dummy->next;
										 dummy->next->prev = NULL;}//NEXT NOT NULL PRE IS NULL
										   }
                                    else                 //NEXT IS NULL
                                    {
										
										if((dummy->prev)!=NULL ) //NEXT IS  NULL PRE IS NOT NULL
                                          {dummy->prev->next=NULL;
								     
										   }
                                        else                     //NEXT IS NULL PRE IS NULL
                                            {list->head=NULL;}
										}
                  
                                    
 
                            
                                    free(dummy);
                                    return list;
                                   }
             }
         return list;
        }








}
}

// ---- you can add your extra operators here ----//



