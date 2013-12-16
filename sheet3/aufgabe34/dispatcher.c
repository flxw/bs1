
#include <stdio.h>

#include "dispatcher.h"

HANDLE dispatcherThreadHandle = NULL;

BOOL InitializeDispatcher(PDISPATCHER_CONFIG Config, INT QuantumLength, SchedulingAlgorithm Algorithm) {

	if (QuantumLength <= 0) {
		printf("[ERROR] quantum length should be > 0\n");
		return FALSE;
	}
	if (Algorithm == NULL) {
		printf("[ERROR] no scheduling algorithm specified\n");
		return FALSE;
	}

	if (!InitializeScheduler())
	{
		printf("[ERROR] initialization of scheduler failed.\n");
		return FALSE;
	}

	Config->Quantum = QuantumLength;
	Config->Algorithm = Algorithm;

	//InitializeListHead(&Config->TaskList);

	if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)) {
		printf("[ERROR] could not set HIGH_PRIORITY_CLASS\n");
		return FALSE;
	}

	return TRUE;
}

BOOL RegisterTask(HANDLE Thread, UCHAR Priority, PDISPATCHER_CONFIG Config) {

	PDISPATCHER_TASK NewTask;

	if (Thread == NULL) {
		printf("[ERROR] invalid task specified\n");
		return FALSE;
	}

	if (!SetThreadPriority(Thread, THREAD_PRIORITY_IDLE)) {
		DisplayError("[ERROR] SetThreadPriority - ", GetLastError());
		return FALSE;
	}

	//start with zeroed structure
	NewTask = (PDISPATCHER_TASK)calloc(1,sizeof(DISPATCHER_TASK)); 
	if( NewTask == NULL) 
	{
		perror("[ERROR] calloc "); 
		return FALSE;
	}

	NewTask->ThreadHandle = Thread;
	NewTask->Priority = Priority;

	return AddThread(NewTask);
}

DWORD WINAPI DispatcherThreadFunc(LPVOID arg) {

	PDISPATCHER_CONFIG Config = (PDISPATCHER_CONFIG)arg;
	PDISPATCHER_TASK CurrentTask = NULL;

#ifdef DBG_OUTPUT
	printf("[DEBUG] dispatcher thread starts\n");
#endif

	while(TRUE) {

		CurrentTask = Config->Algorithm(CurrentTask);

#ifdef DBG_OUTPUT
	printf("[DEBUG] resume next task = %p\n", CurrentTask->ThreadHandle);
#endif

		if (CurrentTask->ThreadHandle != NULL) {
			if (!SetThreadPriority(CurrentTask->ThreadHandle, THREAD_PRIORITY_HIGHEST)) {
				DisplayError("[ERROR] SetThreadPriority - ", GetLastError());
			}
			//if (!ResumeThread(CurrentTask->ThreadHandle)) {
			//	DisplayError("[ERROR] ResumeThread - ", GetLastError());
			//}
		}

#ifdef DBG_OUTPUT
		printf("[DEBUG] dispatcher sleeps\n", CurrentTask->ThreadHandle);
#endif

		Sleep(Config->Quantum);

#ifdef DBG_OUTPUT
	printf("[DEBUG] suspend current task\n", CurrentTask->ThreadHandle);
#endif

		if (CurrentTask->ThreadHandle != NULL) {
			//if (SuspendThread(CurrentTask->ThreadHandle) != (DWORD)-1) {
			//	DisplayError("[ERROR] SuspendThread - ", GetLastError());
			//}
			if (!SetThreadPriority(CurrentTask->ThreadHandle, THREAD_PRIORITY_IDLE)) {
				DisplayError("[ERROR] SetThreadPriority - ", GetLastError());
			}
		}

#ifdef DBG_OUTPUT
	printf("[DEBUG] dispatcher waits\n", CurrentTask->ThreadHandle);
#endif

		Sleep(100);
	}

	return 0;
}

BOOL DispatcherStart(PDISPATCHER_CONFIG Config) {

	if (dispatcherThreadHandle != NULL) {
		printf("[ERROR] dispatcher thread already running\n");
		return FALSE;
	}

	if (Config->Algorithm == NULL) {
		printf("[ERROR] no algorithm defined\n");
		return FALSE;
	}

	dispatcherThreadHandle = CreateThread(NULL, 0, DispatcherThreadFunc, (LPVOID)Config, 0, NULL);

	if (dispatcherThreadHandle == NULL) {
		DisplayError("[ERROR] CreateThread - ", GetLastError());
		return FALSE;
	}

#ifdef DBG_OUTPUT
	printf("[DEBUG] dispatcher started\n");
#endif

	return TRUE;
}

BOOL DispatcherStop() {

	if (!TerminateThread(dispatcherThreadHandle, 0)) {
		DisplayError("[ERROR] TerminateThread - ", GetLastError());
		return FALSE;
	}

	dispatcherThreadHandle = NULL;

#ifdef DBG_OUTPUT
	printf("[DEBUG] dispatcher stopped\n");
#endif

	return TRUE;
}

DWORD WINAPI DummyThreadFunc(LPVOID arg) {

	DWORD i = 0;

#ifdef DBG_OUTPUT
	printf("[DEBUG] DummyThreadFunc starts\n");
#endif

	while(TRUE) {
		// loop
		Sleep(1000);
	}

#ifdef DBG_OUTPUT
	printf("[DEBUG] DummyThreadFunc finished\n");
#endif

	return 0;
}

HANDLE CreateDummyThread() {

	return CreateThread(NULL, 0, DummyThreadFunc, NULL, CREATE_SUSPENDED, NULL);
}

VOID DisplayError(char* msg, DWORD NTStatusMessage) {

	LPVOID lpMessageBuffer;
	HMODULE Hand = LoadLibrary("NTDLL.DLL");

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_FROM_HMODULE,
		Hand,
		NTStatusMessage,  
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMessageBuffer,  
		0,  
		NULL );

	printf("%s%s", msg, lpMessageBuffer);

	LocalFree( lpMessageBuffer ); 
	FreeLibrary(Hand);
}
