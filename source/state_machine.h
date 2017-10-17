#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdlib.h>
#include <string.h>

typedef struct State_Machine
{
    unsigned state_id;
    unsigned char expect_flag[4];
    unsigned char package_received[4];
} State_Machine;

int next_State(State_Machine *sm, unsigned char *read_char);

int init_State_Machine(State_Machine *sm, unsigned char *expect_flag_default);

int endOfStateMachine(State_Machine *sm);

#endif