#include "pl_dll.h"
#include "frame.h"

//Should we check that frame
#define CHECK_CONSTANTS 0

#if CHECK_CONSTANTS
#include <assert.h>
#endif

//Note, the preq needs to be able to set the EN flag
void process_PReq(uintptr_t rx_buffer_address, chanend c_can_open,
    unsigned char node_id){
  frame * frm = (frame *)rx_buffer_address;

  //preq * p = &(frm->pl_frame.preq);

  //TODO deal with the error flags in much the same way as soa
  //if(error_signalling_data_present)
  //    async_state->EN = 1 - p->EA;



}

void process_PRes(uintptr_t rx_buffer_address, chanend c_can_open,
    unsigned char node_id){
  frame * frm = (frame *)rx_buffer_address;

}

void process_SoC(uintptr_t rx_buffer_address, chanend c_can_open,
    t_frame_SoC * latest_SoC){
  frame * frm = (frame*)rx_buffer_address;
#if CHECK_CONSTANTS
  assert(frm->pl_frame.type == Start_of_Cycle);
  assert(frm->pl_frame.dst == C_ADR_BROADCAST);
  assert(frm->pl_frame.src == C_ADR_MN_DEF_NODE_ID);
#endif
  SoC_t * soc = &(frm->pl_frame.soc);
  latest_SoC;
  latest_SoC->Multiplexed_Cycle_Completed = soc->MC;
  latest_SoC->Prescaled_Slot = soc->PS;
#if D_NMT_NetTime_BOOL
  //latest_SoC->NetTime = soc->NetTime;
#endif
#if D_NMT_RelativeTime_BOOL
  //latest_SoC->RelativeTime = soc->RelativeTime;
#endif

  //send a broadcast to the apps
}

Message_Type_ID get_powerlink_type(uintptr_t pointer){
  frame * frm = (frame *)pointer;
  return frm->pl_frame.type;
}

static uint16_t htons(uint16_t s){
  return (__builtin_bswap32(s)>>16)&0xffff;
}
uint16_t get_ethertype(uintptr_t pointer){
  frame * frm = (frame *)pointer;
  return htons(frm->ethertype);
}

int is_type_powerlink(uintptr_t pointer){
  return get_ethertype(pointer) == C_DLL_ETHERTYPE_EPL;
}

int filter_powerlink_dst(uintptr_t pointer, unsigned node_id){
  frame * frm = (frame *)pointer;
  return !(frm->pl_frame.dst == node_id || frm->pl_frame.dst == 0xff);
}

/*
 * This fills the tx buffer with next_PRes_size bytes of data past next_PRes_tx_pointer.
 * If anything goes wrong then it sets next_PRes_tx_pointer to 0, which disbales transmitting.
 */
void build_next_PRes(uintptr_t * next_PRes_tx_pointer, unsigned * next_PRes_size){

}

/*
 * Send a single ethernet frame
 */
void build_asynchronous(uintptr_t tx_buffer_address){

}
