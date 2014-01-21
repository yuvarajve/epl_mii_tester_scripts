#ifndef GLOBAL_STATE_H_
#define GLOBAL_STATE_H_

#include "pl_general_purpose_constants.h"


nmt_state_t get_nmt_status();
void set_nmt_status(nmt_state_t s);
uint8_t get_node_id();

#endif
