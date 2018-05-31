#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <sys/ucontext.h>
#include <ucontext.h>
#include <string.h>
#include <execinfo.h>

void handler_SIGSEGV(int signal, siginfo_t *data, void *extra_data) {
    ucontext_t *p = (ucontext_t *) extra_data;

    //int registers[9] = {REG_RAX, REG_RBP, REG_RBX, REG_RCX, REG_RDI, REG_RDX, REG_RIP, REG_RSI, REG_RSP};

    printf("Value of general purpose registrs:\n");
    printf("REG_RAX = %d\n", (int) p->uc_mcontext.gregs[REG_RAX]);
    printf("REG_RBP = %d\n", (int) p->uc_mcontext.gregs[REG_RBP]);
    printf("REG_RBX = %d\n", (int) p->uc_mcontext.gregs[REG_RBX]);
    printf("REG_RCX = %d\n", (int) p->uc_mcontext.gregs[REG_RCX]);
    printf("REG_RDI = %d\n", (int) p->uc_mcontext.gregs[REG_RDI]);
    printf("REG_RDX = %d\n", (int) p->uc_mcontext.gregs[REG_RDX]);
    printf("REG_RIP = %d\n", (int) p->uc_mcontext.gregs[REG_RIP]);
    printf("REG_RSI = %d\n", (int) p->uc_mcontext.gregs[REG_RSI]);
    printf("REG_RSP = %d\n", (int) p->uc_mcontext.gregs[REG_RSP]);

    printf("\nDump of memory near crash:\n");

    void *dump[100];
    char **line;
    int size = backtrace(dump, 50);

    dump[0] = (void *) p->uc_mcontext.fpregs->rip;

    line = backtrace_symbols(dump, size);

    for (int i = 1; i < size && line != NULL; ++i) {
        printf("[#%d]: %s\n", i, line[i]);
    }

    free(line);

    exit(1);
}


int main() {

    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(struct sigaction));
    sig_act.sa_flags = SA_SIGINFO;
    sig_act.sa_sigaction = handler_SIGSEGV;
    sigaction(SIGSEGV, &sig_act, NULL);

    int a[1];
    int b = a[-10000000000];
    printf("%d", b);

    return 0;
}
