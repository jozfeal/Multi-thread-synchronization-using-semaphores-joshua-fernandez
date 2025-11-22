//
// Example from: http://www.amparo.netce155/sem-ex.c
//
// Adapted using some code from Downey's book on semaphores
//
// Compilation:
//
//       g++ main.cpp -lpthread -o main -lm
// or 
//      make
//

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */
#include <iostream>
using namespace std;

/*
 This wrapper class for semaphore.h functions is from:
 http://stackoverflow.com/questions/2899604/using-sem-t-in-a-qt-project
 */
class Semaphore {
public:
    // Constructor
    Semaphore(int initialValue)
    {
        sem_init(&mSemaphore, 0, initialValue);
    }
    // Destructor
    ~Semaphore()
    {
        sem_destroy(&mSemaphore); /* destroy semaphore */
    }
    
    // wait
    void wait()
    {
        sem_wait(&mSemaphore);
    }
    // signal
    void signal()
    {
        sem_post(&mSemaphore);
    }
    
    
private:
    sem_t mSemaphore;
};

//  Lightswitch class provided in the book
class Lightswitch {
int counter;
Semaphore Mutex;
public:
	Lightswitch() : counter(0), Mutex(1) {}

	void lock(Semaphore &semaphore) {
		Mutex.wait();
		counter += 1;
		if (counter == 1)
			semaphore.wait();
		Mutex.signal();
	}

	void unlock(Semaphore &semaphore) {
		Mutex.wait();
		counter -= 1;
		if (counter == 0)
			semaphore.signal();
		Mutex.signal();
	}
};


/* global vars */
const int bufferSize = 5;
const int numPeopleType = 5; 
int problemNumber = 1;

/* semaphores are declared global so they can be accessed
 in main() and in thread routine. */

// no-starve readers writers
Lightswitch readSwitch;
Semaphore roomEmpty(1);
Semaphore turnstile(1);

// writer priority readers writers
Lightswitch writeSwitch;
Semaphore noReaders(1);
Semaphore noWriters(1);


/*
    Writer function 
*/
void *Writer ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;

    if (problemNumber == 1) {
    while( 1 )
    {
        sleep(3); // Slow the thread down a bit so we can see what is going on
        turnstile.wait();
	     roomEmpty.wait();
       	     printf("Writer %d: writing\n", x);
             fflush(stdout);
	turnstile.signal();
        roomEmpty.signal();
    }
    }

    else {
    while( 1 )
    {
        sleep(3);
        writeSwitch.lock(noReaders);
             noWriters.wait();
                  printf("Writer %d: writing\n", x);
                  fflush(stdout);
             noWriters.signal();
        writeSwitch.unlock(noReaders);
    }
    }

}

/*
    Reader function 
*/
void *Reader ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;
    
    if (problemNumber == 1) {
    while( 1 )
    {
        turnstile.wait();
        turnstile.signal();
	readSwitch.lock(roomEmpty);
            printf("Reader %d: reading \n", x);
            fflush(stdout);
        readSwitch.unlock(roomEmpty);
        sleep(5);   // Slow the thread down a bit so we can see what is going on
    }
    }

    else {
    while ( 1 )
    {
         noReaders.wait();
             readSwitch.lock(noWriters);
         noReaders.signal();
             printf("Reader %d: reading \n", x);
             fflush(stdout);
	 readSwitch.unlock(noWriters);
	 sleep(5);
    }
    }

}


int main(int argc, char **argv )
{
    // get the problem number from the command line argument
    problemNumber = stol(argv[1]);

    pthread_t writerThread[ numPeopleType ];
    pthread_t readerThread[ numPeopleType ];

    // for the readers writers problems
    if (problemNumber == 1 or problemNumber == 2) {
    // Create the writers
    for( long w = 0; w < numPeopleType; w++ )
    {
        int rc = pthread_create ( &writerThread[ w ], NULL, 
                                  Writer, (void *) (w+1) );
        if (rc) {
            printf("ERROR creating writer thread # %ld; \
                    return code from pthread_create() is %d\n", w, rc);
            exit(-1);
        }
    }

    // Create the readers 
    for( long r = 0; r < numPeopleType; r++ )
    {
        int rc = pthread_create ( &readerThread[ r ], NULL, 
                                  Reader, (void *) (r+1) );
        if (rc) {
            printf("ERROR creating reader thread # %ld; \
                    return code from pthread_create() is %d\n", r, rc);
            exit(-1);
        }
    }
    }

    // for the philosophers problems
    else if (problemNumber == 3 or problemNumber == 4) {
	// philosophers
    }

    printf("Main: program completed. Exiting.\n");


    // To allow other threads to continue execution, the main thread 
    // should terminate by calling pthread_exit() rather than exit(3). 
    pthread_exit(NULL); 


} /* main() */


// no-starve readers writers solution

