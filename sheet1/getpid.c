# include <unistd.h>
# include <syscall.h>
# include <stdio.h>

int main(int argc, char** argv) {
    pid_t syscallPid   = syscall(__NR_getpid);
    pid_t userspacePid = getpid();

    printf("syscall: %i usermode-API: %i\n", syscallPid, userspacePid);
    return 0;
}
