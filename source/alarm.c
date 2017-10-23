#include "alarm.h"

struct sigaction oldAction;

int main(){
	return 0;
}

int setNewSIGHandler(unsigned sig, void (*signal_hanlder)(int)){
	struct sigaction action;
	action.sa_handler = *signal_hanlder;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction(sig,&action, &oldAction) < 0)
	{
		fprintf(stderr,"Unable to install SIGALARM handler\n");
		return -1;
	}
	return 0;
}

int resetAlarmHandler(unsigned sig){
	if (sigaction(sig,&oldAction, NULL) < 0)
	{
		fprintf(stderr,"Unable to reset SIGALARM handler\n");
		return -1;
	}
	return 0;
}
