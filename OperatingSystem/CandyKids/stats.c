#include "stats.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

FacStats *facStats;
int producers;

void stats_init(int num_producers)
{
	producers = num_producers;
	facStats = (FacStats*) malloc(sizeof(FacStats) * num_producers);
	for (int i = 0; i < num_producers; ++i)
	{
		facStats[i].create_num = facStats[i].consume_num = 0;
		facStats[i].max_delay = facStats[i].min_delay = facStats[i].avg_delay = 0;
	}
}

void stats_cleanup(void)
{
	free(facStats);
}


void stats_record_produced(int factory_number)
{
	facStats[factory_number].create_num++;
}

void stats_record_consumed(int factory_number, double delay_in_ms)
{
	facStats[factory_number].consume_num++;
	if (facStats[factory_number].consume_num == 1 ){
		facStats[factory_number].max_delay = delay_in_ms;
		facStats[factory_number].min_delay = delay_in_ms;
		facStats[factory_number].avg_delay = delay_in_ms;
	}
	else {
		if(delay_in_ms > facStats[factory_number].max_delay )
			facStats[factory_number].max_delay = delay_in_ms;
		if(delay_in_ms < facStats[factory_number].min_delay )
			facStats[factory_number].min_delay = delay_in_ms;
		facStats[factory_number].avg_delay = (facStats[factory_number].avg_delay * 
			(facStats[factory_number].consume_num - 1) + delay_in_ms) / facStats[factory_number].consume_num;
	}
}

void stats_display(void)
{
	printf("%8s%8s%8s%14s%14s%14s\n", "Factory#", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");
	for (int i = 0; i < producers; ++i)
	{ 
		printf("%8d%8d%8d   %10.5f    %10.5f    %10.5f\n", i, facStats[i].create_num, facStats[i].consume_num, 
			facStats[i].min_delay, facStats[i].avg_delay, facStats[i].max_delay);
	}
}