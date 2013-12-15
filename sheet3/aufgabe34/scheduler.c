
#include <stdio.h>

#include "dispatcher.h"

#define SCHED_MAX_PRIORITY 2
#define STARVATION_THRESHOLD 10
LIST_ENTRY ReadyQueues[2];

void UpdateStarvationCount(PDISPATCHER_TASK NextRunningTask) {
	PLIST_ENTRY ThisListEntry;
	PLIST_ENTRY NextListEntry;
	PDISPATCHER_TASK ThisListEntryTask;

	// Iterate low priority list
	// - high priority processes won't starve
	// - remember that therefore, high priority processes won't have correct waiting counts!
	ThisListEntry = &ReadyQueues[SCHED_PRIORITY_LOW];
	while(ThisListEntry->Blink != ThisListEntry) {
		NextListEntry = ThisListEntry->Blink;

		ThisListEntryTask = CONTAINING_RECORD(ThisListEntry, DISPATCHER_TASK, Link);

		// Update and boost if starving - but ignore "idle" thread
		if((ThisListEntryTask) && (ThisListEntryTask->WaitCount++) > STARVATION_THRESHOLD) {
			
			RemoveEntryList(ThisListEntry);
			InsertHeadList(&ReadyQueues[SCHED_PRIORITY_HIGH], ThisListEntry);

#ifdef DBG_OUTPUT
			printf("[DEBUG] boosting task = %p\n", ThisListEntryTask->ThreadHandle);
#endif
		}
		
		ThisListEntry = NextListEntry;
	}

	// lower priority if next task was boosted - again, ignore "idle" thread
	if ((NextRunningTask) && NextRunningTask->Priority > NextRunningTask->BasePriority) {
		NextRunningTask->Priority = NextRunningTask->BasePriority;
	}
}

PDISPATCHER_TASK Schedule(PDISPATCHER_TASK OldThread) {

	PDISPATCHER_TASK NextTask;
	PLIST_ENTRY NextTaskListEntry;
	PLIST_ENTRY ReadyQueue;

	if (!IsListEmpty(&ReadyQueues[SCHED_PRIORITY_HIGH]))
	{
		ReadyQueue = &ReadyQueues[1];
	}
	else if (!IsListEmpty(&ReadyQueues[SCHED_PRIORITY_LOW]))
	{
		ReadyQueue = &ReadyQueues[0];
	}
	else
		return OldThread;

	NextTaskListEntry = RemoveHeadList(ReadyQueue);

	// get list entry for next task
	NextTask = CONTAINING_RECORD(NextTaskListEntry, DISPATCHER_TASK, Link);

	UpdateStarvationCount(NextTask);

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
