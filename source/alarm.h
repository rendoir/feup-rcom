#include <unistd.h>
#include <signal.h>
#include <stdio.h>

//struct sigaction {
//    void     (*sa_handler)(int);
//    void     (*sa_sigaction)(int, siginfo_t *, void *);
//    sigset_t   sa_mask;
//    int        sa_flags;
//    void     (*sa_restorer)(void);
//};


int setNewSIGHandler(unsigned sig, void (*alarm_hanlder)(int));

int resetAlarmHandler(unsigned sig);
