#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include "stats.h"
#include "bbuff.h"

typedef struct {
	int factory_number;
	double time_stamp_in_ms;
} candy_t;

double current_time_in_ms(void)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}

_Bool stop_thread = false;


void* fac_runner(void* arg)
{
	int id = *(int*)arg;

	while(!stop_thread){
		int wait_time = rand() % 4;
		printf("\tFactory %d ships candy & waits %ds\n", id, wait_time);
		candy_t* candy = (candy_t*) malloc(sizeof(candy_t));
		candy->factory_number = id;
		candy->time_stamp_in_ms = current_time_in_ms();
		bbuff_blocking_insert(candy);
		stats_record_produced(id);
		sleep(wait_time);
	}
	printf("candy-factory %d done\n", id);
	pthread_exit(0);
}

void* kid_runner(void* arg)
{
	while(1){
		candy_t* candy = bbuff_blocking_extract();
		stats_record_consumed(candy->factory_number, current_time_in_ms() - candy->time_stamp_in_ms);
		free(candy);
		int wait_time = rand() % 2;
		if(wait_time)
			sleep(1);
	}

}

int main(int argc, char const *argv[])
{
// 1. Extract arguments
	int factories = atoi(argv[1]);
	int kids = atoi(argv[2]);
	int seconds = atoi(argv[3]);

	if (factories <= 0 || kids <= 0 || seconds <= 0){
		fprintf(stderr, "All arguments must be greater than 0!\n");
		exit(-1);
	}
// 2. Initialize modules
	bbuff_init();
	stats_init(factories);

// 3. Launch candy-factory threads
	pthread_t *facIDs;
	int *arg;
	facIDs = (pthread_t*) malloc(factories * sizeof(pthread_t));
	arg = (int*) malloc(factories * sizeof(int));
	for (int i = 0; i < factories; ++i)
	{
		arg[i] = i;
		pthread_t tid;
		pthread_create(&tid, NULL, fac_runner, &arg[i]);
		facIDs[i] = tid;
	}
// 4. Launch kid threads
	pthread_t *kidIDs;
	kidIDs = (pthread_t*) malloc(kids * sizeof(pthread_t));

	for (int i = 0; i < kids; ++i)
	{
		pthread_t tid;
		pthread_create(&tid, NULL, kid_runner, NULL);
		kidIDs[i] = tid;
	}
// 5. Wait for requested time
	for (int i = 0; i < seconds; ++i)
	{
		sleep(1);
		printf("Time %ds\n", i + 1);
	}

// 6. Stop candy-factory threads

	stop_thread = true;

	for (int i = 0; i < factories; ++i)
	{
		pthread_join(facIDs[i], NULL);
	}

// 7. Wait until no more candy
	while (!bbuff_is_empty())
	{
		printf("Waiting for all candy to be consumed\n");
		sleep(1);
	}
// 8. Stop kid threads
	for (int i = 0; i < kids; ++i)
	{
		pthread_cancel(kidIDs[i]);
		pthread_join(kidIDs[i], NULL);
	}
// 9. Print statistics
	stats_display();
// 10. Cleanup any allocated memory
	free(facIDs);
	free(kidIDs);
	free(arg);
	stats_cleanup();

	return 0;
}