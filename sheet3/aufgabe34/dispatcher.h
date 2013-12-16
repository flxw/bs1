
#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <windows.h>

#include "list.h"

#define DBG_OUTPUT

#define SCHED_PRIORITY_HIGH 1
#define SCHED_PRIORITY_LOW  0

//
// internal structures and functions
//

typedef struct _DISPATCHER_TASK {

	HANDLE ThreadHandle;
	LIST_ENTRY Link;
	UCHAR Priority;
	
	//
	// Add necessary information here 
	//
 
} DISPATCHER_TASK, *PDISPATCHER_TASK;

//
// algorithm callback definition
//

typedef PDISPATCHER_TASK(*SchedulingAlgorithm)(PDISPATCHER_TASK);

//
// scheduler implementation
//

PDISPATCHER_TASK Schedule(PDISPATCHER_TASK OldThread);
BOOLEAN AddThread(PDISPATCHER_TASK Thread);
BOOLEAN InitializeScheduler();

//
// dispatcher configuration structure
//

typedef struct _DISPATCHER_CONFIG {

	INT Quantum;
	SchedulingAlgorithm Algorithm;

	LIST_ENTRY TaskList;

} DISPATCHER_CONFIG, *PDISPATCHER_CONFIG;

//
// dispatcher interface
//

BOOL InitializeDispatcher(PDISPATCHER_CONFIG Config, INT QuantumLength, SchedulingAlgorithm Algorithm);

BOOL RegisterTask(HANDLE Thread, UCHAR Priority, PDISPATCHER_CONFIG Config);

BOOL DispatcherStart(PDISPATCHER_CONFIG Config);
BOOL DispatcherStop();

//
// utility functions
//

HANDLE CreateDummyThread();
VOID DisplayError(char* msg, DWORD NTStatusMessage);

#endif // _DISPATCHER_H_
