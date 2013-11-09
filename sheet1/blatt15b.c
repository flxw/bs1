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
    DWORD userspaceId = GetCurrentProcessId();
    /*DWORD kernelspaceId = CallNtQueryInformationProcess(GetCurrentProcess(),*/

    printf("syscall: XX usermode-API: %i\n", userspaceId);
    getch();
}
