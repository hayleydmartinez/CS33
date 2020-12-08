#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

/*  =====================================================================
	In this lab, you will implement several versions of a code which
	computes a histogram of a set of random numbers.  While seemingly
	simple, there are many possible optimizations.  Also, this is a key
	kernel in a variety of algorithms (eg. radix sort).

	A skeleton of the lab is provided which already spawns NTHREADS threads
	for you, and calls the various kernels which you are to implement.
	Each for each kernel (histo_1,histo_2, etc...), we describe an idea
	of how to parallelize that you should follow, along with some hint.
	Please answer the questions along with each implementation, and turn
	in the completed thread.c file to CCLE.
	
	Task 1, 2 and 3 are mandatory. For task 4 and 5, you can choose to 
	finish either one of them to get full credits. If you choose to 
	finish both task 4 and 5, you can get 5 extra points if your 
	implemenation and explaination are correct.
    =====================================================================
*/

#include "thread.h"

#define DATA_SIZE 100000000
#define NTHREADS 8
#define STEP (DATA_SIZE/NTHREADS)
#define DATA_MAX 255
#define BUCKET_SIZE 8

typedef void* (*f)(void* );
f thread_routine[6] = {&histo_0, &histo_1, &histo_2, &histo_3, &histo_4, &histo_5};   

/*  =====================================================================
	YOUR CODE HERE:
	
	Add corresponding locks/semaphores and any other global variables here

	Sample declarations:
	sem_t mutex; //don't for get to initialize in main

        pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_barrier_t barrier; //don't forget to initialize in main
        pthread_mutex_t locks[BUCKET_SIZE]; //don't forget to initialize in main


    =====================================================================
*/

sem_t hist1;
pthread_mutex_t hist2;

int bucket[BUCKET_SIZE] = {0};  // record correct bucket result
int global_histogram[BUCKET_SIZE] = {0};
unsigned char data[DATA_SIZE];

/*  =====================================================================
	END YOUR CODE HERE
    =====================================================================
*/

int main() {
	// time variables
	struct timeval start, end;
	long mtime, secs, usecs; 

        // semaphores & mutexes
        sem_init(&hist1, 0, 1);
        if (pthread_mutex_init(&hist2, NULL) != 0) 
    	{ 
            printf("\n Initializing hist2 has failed\n"); 
            return 1;
        }

	// thread id
	pthread_t thread_id[NTHREADS];

	long int i;

	// generate data
	for (i = 0; i < DATA_SIZE; i++) { 
		int datum = rand() % DATA_MAX; 
		data[i] = datum;
		bucket[datum%BUCKET_SIZE]++;
	}

	// run through each thread routine
	int thread_rt_id;
	for (thread_rt_id = 0; thread_rt_id < 6; thread_rt_id++){

		int buc_id;
		for (buc_id=0; buc_id<BUCKET_SIZE; buc_id++){
			global_histogram[buc_id] = 0;
		}

		printf("\nRunning thread_%d: \n", thread_rt_id);
		// get start time
		gettimeofday(&start, NULL);

		for(i=0; i<NTHREADS; i++){
			pthread_create(&thread_id[i], NULL, thread_routine[thread_rt_id], (void*)i);
		}		

		for(i=0; i < NTHREADS; i++)
		{
			pthread_join( thread_id[i], NULL); 
		}

		// visualize the histogram
		printHistogram(global_histogram, BUCKET_SIZE);

		// print the total time
		gettimeofday(&end, NULL);
		secs  = end.tv_sec  - start.tv_sec;
		usecs = end.tv_usec - start.tv_usec;
		mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;
		printf("Elapsed time: %ld millisecs\n", mtime);
		
	}

        // destroy mutexes
        pthread_mutex_destroy(&hist2);

	return 0;
}


/*  =====================================================================
	Kernel to Parallelize
    =====================================================================
    =====================================================================

	This is the code we would like to parallelize, it computes
	a histogram of our randomly generated dataset.  This particular
	histogram counts the number of data items with a given remainder
	when divided by BUCKET_SIZE; we use the modulo (%) operator 
	for that.

	Note that even though this function is called with multiple threads,
	only one thread performs the computation.  It is sequential.
	
    =====================================================================
*/
void *histo_0(void *vargp){
 	int thread_id = (long int)vargp;
	if(thread_id==0) { //only run on thread 0
		int j;
		for (j=0; j<DATA_SIZE; j++){
			global_histogram[data[j]%BUCKET_SIZE]++;
		}
	}       
}


/*  =====================================================================
	BEGIN TASK 1 (mandatory)
    =====================================================================
    =====================================================================

	The given code divides the data into blocks, one for each
	thread.  However, there is a race! (where?)

	Try to add a global lock and modify this thread routine.

	HINT: You might want to apply the lock each time the associated
	counter changes

    =====================================================================
*/

void *histo_1(void *vargp){
	int ind = (long int)vargp;
	ind = ind*STEP;
	int j;
        sem_wait(&hist1);
	for (j=ind; j<ind+STEP; j++){
		global_histogram[data[j]%BUCKET_SIZE]++;
	}
        sem_post(&hist1);
}

/*  =====================================================================
	QUESTION 1:
		After adding the global lock, do you observe any 
		performance changes (i.e. correctness and time) compared
		to histo_0 and histo_1 without the lock? If so, 
		please briefly explain the reasons behind the changes, 
		in one or two sentences.
    
	YOUR ANSWER:
                The global lock solves the problem of the race! This is
                because it forces each thread to complete its computation
                and iterate in sequence before allowing the other thread
                to perform its computation on the memory. Putting the
                semaphores inside the for loop greatly increases the
                runtime of both programs, but putting them outside grants
                the program accuracy and decent runtime to to lack of
                extra overhead.
	
    =====================================================================   
    =====================================================================
	END TASK 1
    =====================================================================
*/



/*  =====================================================================
	BEGIN TASK 2 (mandatory)
    =====================================================================
    =====================================================================

	One problem with the previous implementation is that many threads
	access the lock at the same time.  One possible solution is to
	have one lock per histogram bucket.

	Try to add a bucket-specific lock and modify this thread routine.

	HINT: You might want to use pthread_mutex_init to initialize each
	bucket-specific lock before using threads.

    =====================================================================
*/

void *histo_2(void *vargp){
	int ind = (long int)vargp;
	ind = ind*STEP;
	int j;
	for (j=ind; j<ind+STEP; j++){
                pthread_mutex_lock(&hist2);
		global_histogram[data[j]%BUCKET_SIZE]++;
                pthread_mutex_unlock(&hist2);
	}
}

/*  =====================================================================
	QUESTION 2:
		After adding the bucket-specific lock, do you observe any 
		performance changes (i.e. correctness and time) compared 
		to histo_1 (with global lock)? If so, please briefly 
		explain the reasons behind the changes, in one or two 
		sentences.
    
	YOUR ANSWER:
                Compared to histo_1, histo_2 is much slower, even though
                it has comprable correctness. This is because there is
                a lot of overhead that happens every time the mutex is
                locked and unlocked, and since it does this for each
                bucket, there is a lot of behind-the-scenes calls being
                made by this function.
	
    =====================================================================
    =====================================================================
	END TASK 2
    =====================================================================
*/



/*  =====================================================================
	BEGIN TASK 3 (mandatory)
    =====================================================================
    =====================================================================
    	
	Locks enable us to put anything within a critical section, but
	notice that here we are performing only a single increment within
	the critical section.  The X86 ISA (and others) have a special
	instruction to perform this operation -- this might be faster.

	Try to use atomic increment and modify this thread routine.
	
	HINT: You might want to check the Atomic Builtin function called 
	__sync_fetch_and_add

    =====================================================================
*/

void *histo_3(void *vargp){
	int ind = (long int)vargp;
	ind = ind*STEP;
	int j;
	for (j=ind; j<ind+STEP; j++){
	    __sync_fetch_and_add(&(global_histogram[data[j]%BUCKET_SIZE]),1);
	}
}

/*  =====================================================================
	QUESTION 3:
		After using atomic increment, do you observe any 
		performance changes (i.e. correctness and time) compared 
		to histo_1 (with global lock)? If so, please briefly 
		explain the reasons behind the changes, in one or two 
		sentences.
    
	YOUR ANSWER:
                The performance decreases, because even though the load
                of 1 is atomic and adding it to &(global_hist...) is also
                atomic, two atomic operations are not necessarily atomic
                when composed together. This function has a similar issue
                to the previous function, bcause it locks every time in
                the for loop, contributing to unecessary overhead.
	
    =====================================================================
    =====================================================================
	END TASK 3
    =====================================================================
*/



/*  =====================================================================
	BEGIN TASK 4 (optional if you have finished Task 5)
    =====================================================================
    =====================================================================
	
	It seems that in order to get speedup, we need to eliminate
        fine-grain synchronization.  One simple idea is for each thread
        to perform the histogram operation in its own local array, 
        and only synchronize to accumulate the final sums.

	Try to implement the local histogram-based method.  

	HINT: You might want to use a pthread_barrier to synchronize the
	threads before you perform the reduction on one thread.
	
	ATTENTION: this task is the most difficult one

    =====================================================================
*/

void *histo_4(void *vargp){
	int ind = (long int)vargp;
	ind = ind*STEP;
	int j;
	for (j=ind; j<ind+STEP; j++){
		global_histogram[data[j]%BUCKET_SIZE]++;
	}
}

/*  =====================================================================
	QUESTION 4:
		After implementing local histograms, do you observe any 
		performance changes (i.e. correctness and time) compared 
		to implementation of a global histogram? If so, please 
		briefly explain the reasons behind the changes, in one 
		or two sentences.
    
	YOUR ANSWER:
	
    =====================================================================
    =====================================================================
	END TASK 4
    =====================================================================
*/


/*  =====================================================================
	BEGIN TASK 5 (optional if you have finished task 4)
    =====================================================================
    =====================================================================
        
	The previous method requires some overhead in performing the final
        sum of local histograms.  But perhaps this is not necessary if 
        we split up the work different, and exploit the idea 
        of redundant computation.  Specifically, we could assign each
        thread to compute a particular bucket, rather than sum a portion
        of the array.

	Try to implement a version where each thread reads all the data,
	but sums into a unique bucket.

	HINTS: no synchronization is required, also note that 
	NTHREADS==BUCKET_SIZE

    =====================================================================
*/

void *histo_5(void *vargp){
	//int ind = (long int)vargp;
	//ind = ind*STEP;
	//int j;
	//for (j=ind; j<ind+STEP; j++){
	//	global_histogram[data[j]%BUCKET_SIZE]++;
	//}
	int ind = (long int)vargp;
        int j;
        for (j = 0; j < DATA_SIZE; j++) {
                if ((data[j] % NTHREADS) == ind)
                        global_histogram[ind]++;
        }
}

/*  =====================================================================
	QUESTION 5:
		For these 4 ways of parallelization, or 5 ways if you
		choose to do all of them, which one do you
		think is the best and why? Please briefly explain your
		choice in one or two sentences.
    
	YOUR ANSWER:
                I think the best way to parallelize is the first one,
                because it requires the least amount of overhead in
                terms of synchronization while also reducing the amount
                of commands to execute. Histo_5 is the second best, but
                it requires an extra check (the if statement) that slows
                down the program.
	
    =====================================================================
    =====================================================================
	END TASK 5
    =====================================================================
*/


void printHistogram(int *hist, int n) {

	int i, sum=0;
	for (i = 0; i < n; i++) {
		printf("Bucket [%d] ", i);
		printf("%d", hist[i]);
		printf("\n");
		sum += hist[i];
	}
	printf("Calculated sum: %d, correct sum: %d\n", sum, DATA_SIZE);
}
