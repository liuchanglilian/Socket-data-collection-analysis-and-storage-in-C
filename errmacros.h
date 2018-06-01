#ifndef __errmacros_h__
#define __errmacros_h__

#include <errno.h>

#define SYSCALL_ERROR(err) 									\
		do {												\
			if ( (err) == -1 )								\
			{												\
				perror("Error executing syscall");			\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)
		
#define CHECK_MKFIFO(err) 									\
		do {												\
			if ( (err) == -1 )								\
			{												\
				if ( errno != EEXIST )						\
				{											\
					perror("Error executing mkfifo");		\
					exit( EXIT_FAILURE );					\
				}											\
			}												\
		} while(0)
		
#define FILE_OPEN_ERROR(fp) 								\
		do {												\
			if ( (fp) == NULL )								\
			{												\
				perror("File open failed");					\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define FILE_CLOSE_ERROR(err) 								\
		do {												\
			if ( (err) == -1 )								\
			{												\
				perror("File close failed");				\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define ASPRINTF_ERROR(err) 								\
		do {												\
			if ( (err) == -1 )								\
			{												\
				perror("asprintf failed");					\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define FFLUSH_ERROR(err) 								\
		do {												\
			if ( (err) == EOF )								\
			{												\
				perror("fflush failed");					\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)
#define PTHREAD_ERR_HANDLER( err_code,msg, file_name, line_nr )  \
                         do {                                                  \
                             if ( 0 != err_code )                           \
                               fprintf( stderr, "\n%s failed with error code %d in file %s at line %d\n", msg,   err_code, file_name, line_nr );             \
                                                       \
               }while(0)

#endif
