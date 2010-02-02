// adapted from
// http://en.wikipedia.org/wiki/POSIX_Threads#Example

// Test case for bnc#565918:
// The container variable for y2debug messages was not protected from
// concurrent thread access.
// Simply invoke y2debug from several threads many times to make it crash.
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS     5

#include <y2util/y2log.h>

int ticks;

void *PrintHello(void *threadid)
{
   long tid;
 
 
   tid = (long)threadid;
   printf("Hello World! It's me, thread %p!\n", threadid);
   for (int i = 0; i < ticks; ++i)
     y2debug("Thread %p tick %d", threadid, i);
   pthread_exit(NULL);
}
 
int main (int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];

   // enable collecting y2debug messages
   setenv("Y2DEBUGONCRASH", "1", 1 /*overwrite*/);
   ticks = argc>1? atoi(argv[1]): 1000;
   int rc;
   intptr_t t;
   for(t=1; t<=NUM_THREADS; t++){
      printf("In main: creating thread %p\n", (void*)t);
      rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
 
 
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
	 return 1;
      }
   }
   pthread_exit(NULL);
   // if the bug exists, the program crashes before it can exit succesfully
   return 0;
}

