
#include <stdio.h>

#include "dispatcher.h"

#define SCHED_MAX_PRIORITY 2
#define STARVATION_THRESHOLD 10
LIST_ENTRY ReadyQueues[2];

PDISPATCHER_TASK Schedule(PDISPATCHER_TASK OldThread) {

	PDISPATCHER_TASK NextTask;
	PLIST_ENTRY NextTaskListEntry;
	PLIST_ENTRY ReadyQueue;
	static unsigned int HighPrioCounter = 0;

	if (
		!IsListEmpty(&ReadyQueues[SCHED_PRIORITY_HIGH])
		&& (HighPrioCounter < STARVATION_THRESHOLD || IsListEmpty(&ReadyQueues[SCHED_PRIORITY_LOW])) // low prio list is not yet starving or not ready
		)
	{
		ReadyQueue = &ReadyQueues[1];
		HighPrioCounter++;
	}
	else if (!IsListEmpty(&ReadyQueues[SCHED_PRIORITY_LOW]))
	{
		ReadyQueue = &ReadyQueues[0];
		HighPrioCounter = 0;
	}
	else
	{
		return OldThread;
	}

	printf("HighPrioCounter = %d\n", HighPrioCounter);

	NextTaskListEntry = RemoveHeadList(ReadyQueue);

	// get list entry for next task
	NextTask = CONTAINING_RECORD(NextTaskListEntry, DISPATCHER_TASK, Link);

	// UpdateStarvationCount();

	// re-insert task at list tail, but skip the "idle" thread
	if (OldThread)
	{
		InsertTailList(&ReadyQueues[OldThread->Priority], &OldThread->Link);
	}

	// return result
	return NextTask;
}

BOOLEAN InitializeScheduler()
{
	InitializeListHead(&ReadyQueues[0]);
	InitializeListHead(&ReadyQueues[1]);

	return TRUE;
}

BOOLEAN AddThread(PDISPATCHER_TASK Thread)
{
	if (Thread->Priority != SCHED_PRIORITY_HIGH && Thread->Priority != SCHED_PRIORITY_LOW)
	{
		return FALSE;
	}

	InsertTailList(&ReadyQueues[Thread->Priority], &Thread->Link);

	return TRUE;
}
