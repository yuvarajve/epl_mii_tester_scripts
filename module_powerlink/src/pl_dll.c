#include "pl_dll.h"
#include "frame.h"
#include "can_open_interface.h"
#include <stdio.h>
//Should we check that frame
#define CHECK_CONSTANTS 0

#if CHECK_CONSTANTS
#include <assert.h>
#endif

void prepare_PRes(uintptr_t tx_buffer_address, chanend c_can_open){
  unsigned char number_of_entries;
  unsigned no_of_bytes;
  co_error_code_t err = co_read(c_can_open, 0x1a00, 0, &number_of_entries, &no_of_bytes);
  //TODO check the err
  frame * frm = (frame *)tx_buffer_address;

  PRes_t * p = &(frm->pl_frame.pres);

  for(unsigned short subindex=1;subindex<=number_of_entries;subindex++){
    uint16_t rx_index;
    uint8_t rx_subindex;
    uint16_t rx_offset;
    uint16_t rx_length;
    co_read_PDO_entry(c_can_open, 0x1a00, subindex, &rx_index, &rx_subindex, &rx_length, &rx_offset);
    uint8_t data[C_DLL_ISOCHR_MAX_PAYL];
    unsigned number_of_bytes;
    co_error_code_t e = co_read(c_can_open, rx_index, rx_subindex, p->payload, &number_of_bytes);
  }

  p->size = 1;
}


//Note, the preq needs to be able to set the EN flag
void process_PReq(uintptr_t rx_buffer_address, chanend c_can_open){
  frame * frm = (frame *)rx_buffer_address;

  PReq_t * p = &(frm->pl_frame.preq);

  if(p->RD==0)
    return;

  //read from 0x1600:0
  unsigned char number_of_entries;
  unsigned no_of_bytes;
  co_error_code_t err = co_read(c_can_open, 0x1600, 0, &number_of_entries, &no_of_bytes);
  //TODO check the err

  for(unsigned short subindex=1;subindex<=number_of_entries;subindex++){
    uint16_t rx_index;
    uint8_t rx_subindex;
    uint16_t rx_offset;
    uint16_t rx_length;
    co_read_PDO_entry(c_can_open, 0x1600, subindex, &rx_index, &rx_subindex, &rx_length, &rx_offset);
    uint8_t data[C_DLL_ISOCHR_MAX_PAYL];
    unsigned number_of_bytes = rx_length>>3;
    co_error_code_t e = co_write(c_can_open, rx_index, rx_subindex, p->payload, number_of_bytes);
  }
  //TODO deal with the error flags in much the same way as soa
  //if(error_signalling_data_present)
  //    async_state->EN = 1 - p->EA;
}

void process_PRes(uintptr_t rx_buffer_address, chanend c_can_open,
    unsigned char node_id){
  //frame * frm = (frame *)rx_buffer_address;

}

void process_SoC(uintptr_t rx_buffer_address){
  frame * frm = (frame*)rx_buffer_address;
#if CHECK_CONSTANTS
  assert(frm->pl_frame.type == Start_of_Cycle);
  assert(frm->pl_frame.dst == C_ADR_BROADCAST);
  assert(frm->pl_frame.src == C_ADR_MN_DEF_NODE_ID);
#endif
  //SoC_t * soc = &(frm->pl_frame.soc);

 // latest_SoC->Multiplexed_Cycle_Completed = soc->MC;
  //latest_SoC->Prescaled_Slot = soc->PS;
#if D_NMT_NetTime_BOOL
  //latest_SoC->NetTime = soc->NetTime;
#endif
#if D_NMT_RelativeTime_BOOL
  //latest_SoC->RelativeTime = soc->RelativeTime;
#endif

  //send a broadcast to the apps
}

message_type_id_t get_powerlink_type(uintptr_t pointer){
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
