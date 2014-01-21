#include "pl_general_purpose_constants.h"
#include <stdint.h>
#include "pl_defines.h"

nmt_state_t g_nmt_status;
uint8_t g_node_id;

nmt_state_t get_nmt_status(){
  return g_nmt_status;
}

void set_nmt_status(nmt_state_t s){
  g_nmt_status = s;
}
uint8_t get_node_id(){
  return g_node_id;
}
