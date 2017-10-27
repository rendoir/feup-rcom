#include "alarm.h"

struct sigaction oldAction;

int setNewAlarmHandler(void (*signal_handler)(int)){
	struct sigaction action;
	action.sa_handler = signal_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction(SIGALRM,&action, &oldAction) < 0)
	{
		fprintf(stderr,"Unable to install SIGALARM handler\n");
		return -1;
	}
	return 0;
}

int resetAlarmHandler(){
	if (sigaction(SIGALRM,&oldAction, NULL) < 0)
	{
		fprintf(stderr,"Unable to reset SIGALARM handler\n");
		return -1;
	}
	return 0;
}
