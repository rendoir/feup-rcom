#ifndef ALARM_H
#define ALARM_H

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


int setNewAlarmHandler(void (*signal_hanlder)(int));

int resetAlarmHandler();

#endif
