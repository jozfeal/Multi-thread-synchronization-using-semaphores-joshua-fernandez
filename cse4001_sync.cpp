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



// dining philosophers helper functions
Semaphore forks[5] = { Semaphore (1), Semaphore (1), Semaphore (1),
	                    Semaphore (1), Semaphore (1) };
Semaphore footman(4);
int left(int i) { return i; }
int right(int i ) { return (i + 1) % 5; }

void get_forks(int i) {
     // footman solution version
     if (problemNumber == 3) {
          footman.wait();
          forks[right(i)].wait();
          forks[left(i)].wait();
     }

     else {
          // makes only the first philosopher left handed
	  if (i == 0) {
	       forks[left(i)].wait();
	       forks[right(i)].wait();
	  }
	  else {
               forks[right(i)].wait();
               forks[left(i)].wait();
	  }
     }
}

void put_forks(int i) {
     // footman solution version
     if (problemNumber == 3) {
          forks[right(i)].signal();
          forks[left(i)].signal();
          footman.signal();
     }

     else {
          // makes only the first philosopher left handed
          if (i == 0) {
               forks[left(i)].signal();
               forks[right(i)].signal();
	  }
          else {
               forks[right(i)].signal();
               forks[left(i)].signal();
          }
     }
}

/*
    Philosopher function
*/
void *Philosopher ( void *threadID )
{
    // Thread numbenr
    int x = (long)threadID;

    while( 1 )
    {
	printf("Philosopher %d: thinking \n", x);
        fflush(stdout);
	
	get_forks(x - 1);

        printf("Philosopher %d: eating \n", x);
        fflush(stdout);

	put_forks(x - 1);
        sleep(2);   // Slow the thread down a bit so we can see what is going on
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
    // Create the philosophers
    for( long p = 0; p < numPeopleType; p++ )
    {
        int rc = pthread_create ( &writerThread[ p ], NULL,
                                  Philosopher, (void *) (p+1) );
        if (rc) {
            printf("ERROR creating philosopher thread # %ld; \
                    return code from pthread_create() is %d\n", p, rc);
            exit(-1);
        }
    }
    }

    printf("Main: program completed. Exiting.\n");


    // To allow other threads to continue execution, the main thread 
    // should terminate by calling pthread_exit() rather than exit(3). 
    pthread_exit(NULL); 


} /* main() */


// no-starve readers writers solution

