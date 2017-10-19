#include "state_machine.h"
#include <stdio.h>

#define FLAG   0x7e

/**
* Returns 0 in case of sucess, -1 if not succesful.
*/
int next_State(State_Machine* sm, unsigned char* read_char, unsigned char* expected_bytes){
  printf("next_state called\n");
  for (int i = 0; i < 5; i++){
    printf("sm->expect_bytes[%d] = 0x%02X\n", i, expected_bytes[i]);
  }

    (sm->package_received)[sm->state_id] = *read_char;
    printf("expected char: 0x%02X, got: 0x%02X, current state=%d\n", (expected_bytes)[sm->state_id], *read_char,sm->state_id);
    if (sm->state_id == 2){
      sm->state_id++;
      return 0;
    }
    if(sm->state_id == 3){
        (expected_bytes)[3] = (sm->package_received)[1] ^ (sm->package_received)[2];
    }

    if((sm->package_received)[sm->state_id] == (expected_bytes)[sm->state_id]){
        sm->state_id++;
        return 0;
    }
    if((sm->package_received)[sm->state_id] == FLAG){
        sm->state_id = 1;
    }
    return -1;
}

int init_State_Machine(State_Machine* sm, unsigned char* expected_bytes_default){
    sm = malloc(sizeof(State_Machine));
    sm->state_id = 0;
    /*
    memcpy(expected_bytes, expected_bytes_default, 5);
    for (int i = 0; i < 5; i++){
      printf("sm->expect_flag[%d] = 0x%02X\n", i, expected_bytes[i]);
    }*/
    return 0;
}

int endOfStateMachine(State_Machine* sm){
    if(sm->state_id == 5) {
        return 1;
    }
    return 0;
}
