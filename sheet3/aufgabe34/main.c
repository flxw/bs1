
#include <stdio.h>

#include "dispatcher.h"

int main(int argc, char* argv[]) {

	PDISPATCHER_CONFIG dispatcherConfig;

	dispatcherConfig = (PDISPATCHER_CONFIG)malloc(sizeof(DISPATCHER_CONFIG));
	if (dispatcherConfig == NULL) {
		printf("[ERROR] could not allocate memory for DISPATCHER_CONFIG\n");
		return 1;
	}

	// initialize dispatcher
	if (!InitializeDispatcher(dispatcherConfig, 2000, Schedule)) {
		printf("[ERROR] could not initialize dispatcher\n");
		return 1;
	}

	// register tasks at dispatcher
	RegisterTask(CreateDummyThread(), SCHED_PRIORITY_HIGH, dispatcherConfig);
	RegisterTask(CreateDummyThread(), SCHED_PRIORITY_HIGH, dispatcherConfig);
	RegisterTask(CreateDummyThread(), SCHED_PRIORITY_LOW, dispatcherConfig);

	// ... wait ...
	printf("Please start perfmon and add thread performance counters.\n");
	printf("Hit <enter> to start the dispatcher once you are ready.\n");
	printf("Hitting enter again will stop the dispatcher.\n");
	getchar();

	// start dispatching
	printf("Running dispatcher ...\n");
	DispatcherStart(dispatcherConfig);

	// ... wait ...
	getchar();

	// stop dispatching
	printf("Stopping dispatcher ...\n");
	DispatcherStop();

	return 0;
}
