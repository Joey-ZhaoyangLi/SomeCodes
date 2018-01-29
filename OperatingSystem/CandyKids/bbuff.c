#include "bbuff.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

void* buffer[BUFFER_SIZE];
sem_t mutex;
sem_t full;
sem_t empty;

void bbuff_init(void)
{
	for (int i = 0; i < BUFFER_SIZE; ++i)
	{
		buffer[i] = NULL;
	}
	sem_init(&mutex, 0, 1);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, BUFFER_SIZE);
}

void bbuff_blocking_insert(void* item)
{
	sem_wait(&empty);
	sem_wait(&mutex);
	for (int i = 0; i < BUFFER_SIZE; ++i)
	{
		if (buffer[i] == NULL){
			buffer[i] = item;
			break;
		}
	}

	sem_post(&mutex);
	sem_post(&full);
}

void* bbuff_blocking_extract(void)
{
	void *res;
	sem_wait(&full);
	sem_wait(&mutex);
	for (int i = 0; i < BUFFER_SIZE; ++i)
	{
		if (buffer[i] != NULL){
			res = buffer[i];
			buffer[i] = NULL;
			break;
		}
	}

	sem_post(&mutex);
	sem_post(&empty);
	return res;
}

_Bool bbuff_is_empty(void)
{
	for (int i = 0; i < BUFFER_SIZE; ++i)
	{
		if (buffer[i] != NULL)
		{
			return false;
		}
	}
	return true;
}