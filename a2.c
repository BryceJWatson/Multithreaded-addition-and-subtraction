/* a2.c
Multi-threaded program that solves the addition and subtraction problem which is
a variation on the readers and writers synchronisation problem.

Author: Bryce Watson
Student ID: 220199390

Parameters: none

Returns 0 on success, 1 on failure

To build it use: make

To run:
  make run (./a2)
  OR
  ./a2

To clean:
  make clean

Run example:
  ./a2
*/

/***** Included libraries *****/
#include "sem_ops.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/***** Global variables *****/

/* Constants */
#define MAX_READERS 10
#define MAX_WRITERS 5

/* Variables to hold semaphore values */
int reader_sem;
int sum_sem;

/* Global thread counters */
int readers = 0;
int incrementers = 0;
int decrementers = 0;

/* definition of the struct that will contain the shared data that is to be
 * printed to the console as the result- contains all fields specified by the
 * assignment criteria
 */
struct SharedData
{
  int sum;
  int last_incrementer_id;
  int last_decrementer_id;
  int num_readers;
  int num_incrementers;
  int num_decrementers;
};

/* Global struct containing the shared data*/
struct SharedData data;

/**
 * @brief Reader thread function that each reader thread will execute upon being
 * created.
 *
 * The first reader acquires the semaphore for the data (the sum) on behalf of
 * all the readers that follow. Each reader reads the data one at a time. The
 * last reader will unlock the sum semaphore to allow other threads to access
 * it.
 *
 * @param arg A pointer to the thread's identifier.
 * @return void* This function does not return any explicit value
 */
void *
reader (void *arg)
{

  /* This is what number I am, e.g the first reader will be *me = 0 */
  int *me = (int *) arg;

  /* Only 1 Reader thread at a time */
  P (reader_sem);		/* Lock the reader semaphore */
  readers++;			/* Increment the reader count */
  if (readers == 1)		/* If I'm the first reader */
    P (sum_sem);		/* Lock the data semaphore */
  V (reader_sem);		/* Unlock the reader semaphore */

  /* Print my identity and my operation to stdout */
  printf ("Reader %d got %d\n", *me, data.sum);

  /* Only 1 reader at a time */
  P (reader_sem);		/* Lock the reader semaphore */
  readers--;			/* Decrement the reader count */
  if (readers == 0)		/* If I'm the last reader */
    V (sum_sem);		/* Unlock the data semaphore */
  V (reader_sem);		/* Unlock the reader semaphroe */

  /* After I have done my job i exit */
  pthread_exit (NULL);
}

/**
 * @brief Incrementer thread function that each intermenter thread will execute
 * upon being created.
 *
 * Each incrementer thread increments the shared data value (the sum) in a
 * synchronized manner using semaphores. Each incrementer thread acquires
 * exclusive access to the shared data before incrementing it by one and
 * releasing the lock. The last Incrementer sets the last incrementer id to its
 * own id.
 *
 * @param arg A pointer to the thread's identifier.
 * @return void* This function does not return any explicit value
 */
void *
incrementer (void *arg)
{

  /* This is what number I am, e.g the first incrementer will be *me = 0 */
  int *me = (int *) arg;

  /* Only 1 Incrementer thread at a time */
  P (sum_sem);			/* Lock the data semaphore */
  data.sum++;			/* Increment the sum by 1 */

  /* Print my identity and operation to stdout */
  printf ("Incrementer %d set sum = %d\n", *me, data.sum);

  V (sum_sem);			/* Unlock the data semaphore */

  /* If I'm the last Incrementer thread set the last incrementer id to mine */
  if (*me == data.num_incrementers - 1)
    data.last_incrementer_id = *me;

  /* After I have done my job i exit */
  pthread_exit (NULL);
}

/**
 * @brief Decrementer thread function that each decrementer thread will execute
 * upon being created.
 *
 * Each decrementer thread decrements the shared data value (the sum) in a
 * synchronized manner using semaphores. Each decrementer thread acquires
 * exclusive access to the shared data before decrementing it by one and
 * releasing the lock. The last decrementer sets the last decrementer id to its
 * own id.
 *
 * @param arg A pointer to the thread's identifier.
 * @return void* This function does not return any explicit value
 */
void *
decrementer (void *arg)
{

  /* This is what number I am, e.g the first decrementer will be *me = 0 */
  int *me = (int *) arg;

  /*Only 1 Decrementer thread at a time */
  P (sum_sem);			/* Lock the data semaphore */
  data.sum--;			/* Decrement the sum by 1 */

  /* Print my identity and operation to stdout */
  printf ("Decrementer %d set sum = %d\n", *me, data.sum);

  V (sum_sem);			/* Unlock the data semaphore */

  /* If I'm the last Decrementer thread set the last Decrementer id to mine */
  if (*me == data.num_decrementers - 1)
    data.last_decrementer_id = *me;

  /* After I have done my job i exit */
  pthread_exit (NULL);
}

/**
 * @brief Simple function that prints each field in the shared data to stdout as
the results.
 *
 * This function was designed so it exactly mimics the example assignment output
 */
void
display_results ()
{
  printf ("There were %d readers, %d incrementers, and %d decrementers\n",
	  data.num_readers, data.num_incrementers, data.num_decrementers);
  printf ("The final state of the data is:\n");
  printf ("last incrementer: %d\n", data.last_incrementer_id);
  printf ("last decrementer %d\n", data.last_decrementer_id);
  printf ("total writers %d\n",
	  (data.num_incrementers + data.num_decrementers));
  printf ("sum %d\n", data.sum);
}

/**
 * @brief Exit handler function that removes all semaphores created when the
 * program exits.
 *
 */
void
cleanup ()
{
  rm_sem (reader_sem);
  rm_sem (sum_sem);
}

/**
 * @brief Main function that runs the program.
 *
 * @return int This function returns 0 on success and 1 on failure.
 */
int
main ()
{

  /* Register exit handler function */
  atexit (cleanup);

  /* Declare variables */
  int i;

  /* Initialise number of threads */
  srand (time (NULL));
  data.num_readers = rand () % MAX_READERS + 1;
  data.num_incrementers = rand () % MAX_WRITERS + 1;
  data.num_decrementers = rand () % MAX_WRITERS + 1;

  pthread_t rtid[data.num_readers];	/* Reader thread identifier array */
  int readerid[data.num_readers];	/* Reader id array */
  pthread_t incrementer_tid[data.num_incrementers];	/* Incrementer writer thread
							   identifier array */
  int incrementerid[data.num_incrementers];	/* Incrementer writer id array */
  pthread_t decrementer_tid[data.num_decrementers];	/* Decrementer writer thread
							   identifier array */
  int decrementerid[data.num_decrementers];	/* decrementer writer id array */

  /* Create and initialise semaphores to 1 */
  reader_sem = semtran (IPC_PRIVATE);
  sum_sem = semtran (IPC_PRIVATE);
  V (reader_sem);
  V (sum_sem);

  /* Set up thread ids */
  for (i = 0; i < data.num_incrementers; i++)
    {
      incrementerid[i] = i;
    }
  for (i = 0; i < data.num_decrementers; i++)
    {
      decrementerid[i] = i;
    }
  for (i = 0; i < data.num_readers; i++)
    {
      readerid[i] = i;
    }

  /* Run the threads */
  for (i = 0; i < data.num_incrementers; i++)
    {
      if (pthread_create (&incrementer_tid[i], NULL, incrementer,
			  (void *) &incrementerid[i]))
	{
	  perror ("Could not create thread");
	  exit (EXIT_FAILURE);
	}
    }
  for (i = 0; i < data.num_decrementers; i++)
    {
      if (pthread_create (&decrementer_tid[i], NULL, decrementer,
			  (void *) &decrementerid[i]))
	{
	  perror ("Could not create thread");
	  exit (EXIT_FAILURE);
	}
    }
  for (i = 0; i < data.num_readers; i++)
    {
      if (pthread_create (&rtid[i], NULL, reader, (void *) &readerid[i]))
	{
	  perror ("Could not create thread");
	  exit (EXIT_FAILURE);
	}
    }

  /* Wait for threads to finish */
  for (i = 0; i < data.num_incrementers; i++)
    {
      if (pthread_join (incrementer_tid[i], NULL))
	{
	  perror ("Thread join failed");
	  exit (EXIT_FAILURE);
	}
    }
  for (i = 0; i < data.num_decrementers; i++)
    {
      if (pthread_join (decrementer_tid[i], NULL))
	{
	  perror ("Thread join failed");
	  exit (EXIT_FAILURE);
	}
    }
  for (i = 0; i < data.num_readers; i++)
    {
      if (pthread_join (rtid[i], NULL))
	{
	  perror ("Thread join failed");
	  exit (EXIT_FAILURE);
	}
    }

  /* Display results */
  display_results ();

  /* Cleanup and exit */
  exit (EXIT_SUCCESS);
}
