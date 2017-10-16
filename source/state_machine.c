#include "state_machine.h"

#define FLAG   0x7e

int next_State(State_Machine* sm, unsigned char* read_char){
    (sm->package_received)[sm->state_id] = *read_char;
    if(*read_char == FLAG){
        sm->state_id = 0;
        return 0;
    }

    if(sm->state_id == 2){
        (sm->expect_flag)[3] = (sm->package_received)[1] ^ (sm->package_received)[2];
    }
    if((sm->package_received)[sm->state_id] == (sm->expect_flag)[sm->state_id]){
        sm->state_id++;
    }
    return 0;
}

int init_State_Machine(State_Machine* sm, unsigned char* expect_flag_default){
    sm = malloc(sizeof(State_Machine));
    sm->state_id = 0;
    memcpy(sm->expect_flag, expect_flag_default, 4);
    return 0;
}

int endOfStateMachine(State_Machine* sm){
    if(sm->state_id == 5) {
        return 1;
    }
    return 0;
}