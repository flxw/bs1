# include <windows.h>
# include <winternl.h>
# include <stdio.h>
# include <conio.h>

__declspec(noinline)
__declspec(naked)
NTSTATUS CallNtQueryInformationProcess(IN HANDLE ProcessHandle,
            IN PROCESS_INFORMATION_CLASS ProcessInformationClass,
            OUT PVOID ProcessInformation,
            IN ULONG ProcessInformationLength,
            OUT PULONG ReturnLength) {
        __asm {
            mov eax, 0x00ea
            mov edx, 0x7FFE0300
            call dword ptr [edx]

            ret
    }
}

int main(int argc, char** argv) {

	PROCESS_BASIC_INFORMATION output;
	unsigned long buffersize = sizeof(output);
	unsigned long outputsize = 0;

    DWORD userspaceId = GetCurrentProcessId();
	NTSTATUS kernelspaceStatus = CallNtQueryInformationProcess(GetCurrentProcess(), 0, &output, buffersize, &outputsize);

	DWORD kernelspaceId = output.UniqueProcessId;

	printf("syscall: %i usermode-API: %i\n", kernelspaceId, userspaceId);
    _getch();
}
