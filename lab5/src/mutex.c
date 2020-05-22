/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *  Modified by A.Kostin
 ********************************************************
 * mutex.c
 *
 * Simple multi-threaded example with a mutex lock.
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void do_one_thing(int *);
void do_two_thing(int *);
void do_three_thing(int *);
void do_four_thing(int *);
void do_wrap_up(int);
int common = 0; /* A shared variable for two threads */
int r1 = 0, r2 = 0, r3 = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int main() {
	pthread_t thread1, thread2, thread3, thread4;

	if (pthread_create(&thread1, NULL, (void *)do_one_thing,
		(void *)&common) != 0) {
		perror("pthread_create");
		exit(1);
	}

	if (pthread_create(&thread2, NULL, (void *)do_two_thing,
		(void *)&common) != 0) {
		perror("pthread_create");
		exit(1);
	}

	if (pthread_create(&thread3, NULL, (void *)do_three_thing,
		(void *)&common) != 0) {
		perror("pthread_create");
		exit(1);
	}

	if (pthread_create(&thread4, NULL, (void *)do_four_thing,
		(void *)&common) != 0) {
		perror("pthread_create");
		exit(1);
	}
	if (pthread_join(thread1, NULL) != 0) {
		perror("pthread_join");
		exit(1);
	}

	if (pthread_join(thread2, NULL) != 0) {
		perror("pthread_join");
		exit(1);
	}

	if (pthread_join(thread3, NULL) != 0) {
		perror("pthread_join");
		exit(1);
	}

	if (pthread_join(thread4, NULL) != 0) {
		perror("pthread_join");
		exit(1);
	}

	do_wrap_up(common);

	return 0;
}

void do_one_thing(int *pnum_times) {
	int i, j, x;
	unsigned long k;
	int work;
	for (i = 0; i < 25; i++) {
		pthread_mutex_lock(&mut);
		printf("doing one thing\n");
		work = *pnum_times;
		printf("counter = %d\n", work);
		work++; /* increment, but not write */
		for (k = 0; k < 500000; k++)
			;                 /* long cycle */
		*pnum_times = work; /* write back */
		pthread_mutex_unlock(&mut);
		usleep(1);
	}
}

void do_two_thing(int *pnum_times) {
	int i, j, x;
	unsigned long k;
	int work;
	for (i = 0; i < 25; i++) {
		pthread_mutex_lock(&mut);
		printf("doing two thing\n");
		work = *pnum_times;
		printf("counter = %d\n", work);
		work++; /* increment, but not write */
		for (k = 0; k < 500000; k++)
			;                 /* long cycle */
		*pnum_times = work; /* write back */
		pthread_mutex_unlock(&mut);
		usleep(1);
	}
}

void do_three_thing(int *pnum_times) {
	int i, j, x;
	unsigned long k;
	int work;
	for (i = 0; i < 25; i++) {
		pthread_mutex_lock(&mut);
		printf("doing three thing\n");
		work = *pnum_times;
		printf("counter = %d\n", work);
		work++; /* increment, but not write */
		for (k = 0; k < 500000; k++)
			;                 /* long cycle */
		*pnum_times = work; /* write back */
		pthread_mutex_unlock(&mut);
		usleep(1);
	}
}

void do_four_thing(int *pnum_times) {
	int i, j, x;
	unsigned long k;
	int work;
	for (i = 0; i < 25; i++) {
		pthread_mutex_lock(&mut);
		printf("doing four thing\n");
		work = *pnum_times;
		printf("counter = %d\n", work);
		work++; /* increment, but not write */
		for (k = 0; k < 500000; k++)
			;                 /* long cycle */
		*pnum_times = work; /* write back */
		pthread_mutex_unlock(&mut);
		usleep(1);
	}
}
void do_wrap_up(int counter) {
	int total;
	printf("All done, counter = %d\n", counter);
}
