#include "debugger.h"
#include <sys/ptrace.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: trace <program> [args...]\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execv(argv[1], argv + 1);
        perror("execv");
        return 1;
    }

    printf("trace: pid %d\n", pid);
    Debugger dbg{argv[1], pid};
    dbg.run();
    return 0;
}
