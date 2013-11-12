#include <stdint.h>
#include "powerlink.h"
#include "pl_dll.h"
#include "pl_error_defines.h"
#include "can_open.h"
#define DLL
#include "thread_id.h"


typedef enum {
  MII_PACKET_RX,
  MII_CRC_FAIL,
} mii_cmd;

typedef enum {
  DLL_CS_NON_CYCLIC,
  DLL_CS_WAIT_SOC,
  DLL_CS_WAIT_PREQ,
  DLL_CS_WAIT_SOA
} t_dll_state;

static void report_start_of_cycle(chanend error_handler){
  error_handler <: NO_ERROR_SOC;
}

static void report_lost_frame(t_error_codes code, chanend error_handler){
  error_handler <: code;
}

static int is_multiplexed(){
 // return can_open_read_UNSIGNED8(NMT_CycleTiming_TYPE, MultiplCycleCnt_U8) < 2;
}

static void send_PRes(streaming chanend c_mii, uintptr_t tx_pointer, unsigned size){
  unsigned packed_word = make_tx_req_p(tx_pointer, size, 0);
  c_mii <: packed_word;
}
uint16_t get_ethertype(uintptr_t pointer);
int is_type_powerlink(uintptr_t pointer);

/*
 * Return true if the powerlink frame is to be filtered
 */
int filter_powerlink_dst(uintptr_t pointer, unsigned node_id);
#if 0
void handle_wait_preq(Message_Type_ID msg_type, uintptr_t rx_buffer_address){
  switch (msg_type) {
  case Start_of_Cycle : {
    /* DLL_CT7
    The reaction to a SoC frame is state independent. The state machine synchronises to the start of a new cycle.
    DLL_CE_SOC [ ] / synchronise to the cycle begin, report error E_DLL_LOSS_SOA_TH */
    rx_time_of_soc = frame_presentation_time;
    process_SoC(rx_buffer_address, c_can_open, latest_SoC);
    report_start_of_cycle(error_handler);
    report_lost_frame(E_DLL_LOSS_SOA_TH, error_handler);
    cycle_number++;
    break;
  }
  case PollRequest : {
    /* DLL_CT2
    The PReq event occurs within the isochronous phase of communication.
    DLL_CE_PREQ [ ] / Process the PReq frame and send a PRes frame */

    if(waiting_on_tx_ack){
      process_PReq(rx_buffer_address, c_can_open, node_id);
    } else {
      send_PRes(c_mii, next_PRes_tx_pointer, next_PRes_size); //This has to happen before the end of the interframe gap.
      waiting_on_tx_ack = TRUE;
      process_PReq(rx_buffer_address, c_can_open, node_id);
      build_next_PRes(next_PRes_tx_pointer, next_PRes_size);
    }

    last_poll_request_cycle = cycle_number;
    dll_state = DLL_CS_WAIT_SOA;
    break;
  }
  case PollResponse : {



    /* DLL_CT7
    If a PRes frame of another CN was received (cross traffic), the Pres frame shall be processed (if
    configured to do so). The CN waits for either a PRes from another CN (cross traffic) or a PReq frame.
    DLL_CE_PRES [ ] / process PRes frames (cross traffic) */
    process_PRes(rx_buffer_address, c_can_open, node_id);
    break;
  }
  case Start_of_Asynchronous : {
    /* DLL_CT8
    If the CN is in the NMT_CS_OPERATIONAL or NMT_CS_READY_TO_OPERATE the CN will
    assume a LOSS_OF_PREQ if the number of cycles since the last PReq is greater than that
    expected. (1 for non multiplexed CNs, n for multiplexed CNs where n is NMT_CycleTiming_REC.MultipleCycleCnt_U8)

    DLL_CE_SOA [ CN = multiplexed ] / process SoA; if invited, transmit a legal Ethernet frame
    DLL_CE_SOA [ CN != multiplexed ] / process SoA; if invited, transmit a legal Ethernet frame, additionally report error E_DLL_LOSS_PREQ_TH */

    t_nmt_state nmt_state;

    //process_SoA(rx_buffer_address, latest_SoA); //now in NMT
    c_nmt <: REQUEST_STATE;
    c_nmt :> nmt_state;  //TODO check for deadlock and worst case latency -> maybe make comms hi-priority
    if(nmt_state != NMT_CS_PRE_OPERATIONAL_2){
      unsigned number_of_cycles_since_last_PReq = last_poll_request_cycle - cycle_number;
      //if(number_of_cycles_since_last_PReq > CANopen(NMT_CycleTiming_REC,MultipleCycleCnt_U8))
        report_lost_frame(E_DLL_LOSS_PREQ_TH, error_handler);
    }

    //FIXME look at this
    //if(latest_SoA.RequestedServiceTarget == node_id)
    //  send_asynchronous(c_mii);
    dll_state = DLL_CS_WAIT_SOC;
    break;
  }
  case Asynchronous_Send : {
    /* DLL_CT7
    ASnd frames and non POWERLINK frames shall be processed during the isochronous phase.
    DLL_CE_ASND [ ] / report error E_DLL_LOSS_SOA_TH */
    //process_asnd_or_nonpowerlink_frame(rx_buffer_address, c_nmt);//now in NMT
    report_lost_frame(E_DLL_LOSS_SOA_TH, error_handler);
    break;
  }
  default : __builtin_trap(); break;
  }

  break;
}

void handle_wait_soc(){
  switch (msg_type) {
  case Start_of_Cycle : {
    /* DLL_CT1
    The occurrence of the SoC event indicates the beginning of a new POWERLINK cycle. The
    asynchronous phase of the previous cycle ends and the isochronous phase of the next cycle begins.
    DLL_CE_SOC [ ] / synchronise the start of cycle and generate a SoC trigger to the application */
    rx_time_of_soc = frame_presentation_time;
    process_SoC(rx_buffer_address, c_can_open, latest_SoC);
    dll_state = DLL_CS_WAIT_PREQ;
    report_start_of_cycle(error_handler);
    cycle_number++;
    break;
  }
  //////////////////////////////////////////
  case PollRequest : {
    /* DLL_CT4
    If a SoA, PReq or PRes frame is received, there may be a loss of a SoC frame in between. The DLL
    Error Handling shall be notified with the error E_DLL_LOSS_SOC_TH.
    If a PReq frame was received, the incoming data may be ignored and a PRes frame shall be sent.
    DLL_CE_PREQ [ ] / respond with PRes frame, report error E_DLL_LOSS_SOC_TH */

    if(waiting_on_tx_ack){
      process_PReq(rx_buffer_address, c_can_open, node_id);
    } else {
      send_PRes(c_mii, next_PRes_tx_pointer, next_PRes_size);
      waiting_on_tx_ack = TRUE;
      process_PReq(rx_buffer_address, c_can_open, node_id);
      build_next_PRes(next_PRes_tx_pointer, next_PRes_size);
    }
    report_lost_frame(E_DLL_LOSS_SOC_TH, error_handler);
    last_poll_request_cycle = cycle_number;
    break;
  }
  case PollResponse : {
    //DLL_CT4
    /*
    If a SoA, PReq or PRes frame is received, there may be a loss of a SoC frame in between. The DLL
    Error Handling shall be notified with the error E_DLL_LOSS_SOC_TH.
    DLL_CE_PRES [ ] / report error E_DLL_LOSS_SOC_TH
     */
    report_lost_frame(E_DLL_LOSS_SOC_TH, error_handler);
    break;
  }
  case Start_of_Asynchronous : {
    //DLL_CT4
    /*
    If a SoA, PReq or PRes frame is received, there may be a loss of a SoC frame in between. The DLL
    Error Handling shall be notified with the error E_DLL_LOSS_SOC_TH.
    DLL_CE_SOA [ ] / report error E_DLL_LOSS_SOC_TH
    */
    // FIXME is it ok to process this?
    report_lost_frame(E_DLL_LOSS_SOC_TH, error_handler);
    break;
  }
  case Asynchronous_Send : {
    //DLL_CT4
    /*
    If an ASnd frame has been received it shall be processed. The state shall not be changed. Although
    only one asynchronous frame per cycle is allowed, the state machine of the CN does not limit the
    amount of received frames within the asynchronous phase of the cycle.
    DLL_CE_ASND [ ] / process frame
    */
    //FIXME what if there is more than one?
    break;
  }
  default : __builtin_trap(); break;
  }
  break;
}

void handle_wait_soa(){
  switch (msg_type) {
  case Start_of_Cycle : {
    //DLL_CT9
    /*
    The reaction on reception of a SoC is independent of the NMT state, the state machine assumes that
    an expected frame was lost and (re-)synchronises on the SoC.
    DLL_CE_SOC [ ] / synchronise on the SoC, report error E_DLL_LOSS_SOA_TH
    */
    //TODO
    cycle_number++;
    report_start_of_cycle(error_handler);
    report_lost_frame(E_DLL_LOSS_SOA_TH, error_handler);
    dll_state = DLL_CS_WAIT_PREQ;
    break;
  }
  case PollRequest : {
    //DLL_CT3
    /*
    The occurrence of a DLL_CE_PREQ signifies that an expected SoA and SoC frame were lost. The
    DLL_CS will attend to synchronise the new cycle. The DLL Error Handling shall be notified.
    DLL_CE_PREQ[ ] / accept the PReq frame and send a PRes frame, report error E_DLL_LOSS_SOC_TH and E_DLL_LOSS_SOA_TH

    */
    if(waiting_on_tx_ack){
      process_PReq(rx_buffer_address, c_can_open, node_id);
    } else {
      send_PRes(c_mii, next_PRes_tx_pointer, next_PRes_size);
      waiting_on_tx_ack = TRUE;
      process_PReq(rx_buffer_address, c_can_open, node_id);
      build_next_PRes(next_PRes_tx_pointer, next_PRes_size);
    }
    dll_state = DLL_CS_WAIT_SOC;
    report_lost_frame(E_DLL_LOSS_SOC_TH, error_handler);
    report_lost_frame(E_DLL_LOSS_SOA_TH, error_handler);
    last_poll_request_cycle = cycle_number;
    break;
  }
  case PollResponse : {
    //DLL_CT10
    /*
     The CN may process PRes of other CNs.
     DLL_CE_PRES [ ] / process PRes frames (cross traffic)
     */
    process_PRes(rx_buffer_address, c_can_open, node_id);
    dll_state = DLL_CS_WAIT_PREQ;
    break;
  }
  case Start_of_Asynchronous : {
    //DLL_CT3
    /*
    The DLL_CE_SOA event denotes the end of the isochronous phase and the beginning of the
    asynchronous phase of the current cycle. If the SoA frame includes an invitation to the CN, the CN
    may respond with one valid frame.
    DLL_CE_SOA [ ] / process SoA, if allowed send an ASnd frame or a non POWERLINK frame
    */
    //this will be processed by the NMT
    dll_state = DLL_CS_WAIT_SOC;
    break;
  }
  case Asynchronous_Send : {
    //DLL_CT10
    /*
     ASnd frames and non POWERLINK frames shall be accepted during the isochronous phase.
     DLL_CE_ASND [ ] / report error E_DLL_LOSS_SOA_TH
     */
    //this will be processed by the NMT
    report_lost_frame(E_DLL_LOSS_SOA_TH, error_handler);
    break;
  }
  default : __builtin_trap(); break;
  }
  break;
}
#endif

void pl_dll(streaming chanend c_mii, chanend error_handler, chanend c_nmt, chanend c_can_open){

  t_nmt_cmd cmd_nmt;
  unsigned cmd_mii;

  unsigned rx_time_of_soc;
  timer t;
  unsigned pl_cycle_time;

  t_dll_state dll_state = DLL_CS_NON_CYCLIC;

  unsigned multiplexed = FALSE;
  unsigned char node_id;

  t_frame_SoC latest_SoC;

  uintptr_t rx_buffer_base; //this is the base address of the rx buffer
  uintptr_t tx_buffer_base; //this is the base address of the tx buffer

  unsigned cycle_number = 0;
  unsigned last_poll_request_cycle;

  unsigned next_PRes_size = 0; //set to zero to disable
  uintptr_t next_PRes_tx_pointer;
  unsigned waiting_on_tx_ack = FALSE;

  while(1){

    //fetch the NMTState

    select{

      case pl_cycle_time => t when timerafter (rx_time_of_soc + pl_cycle_time) :> unsigned : {

        //DLL_CT4
        /*
        In case of a DLL_CE_SOC_TIMEOUT event happened in NMT_CS_READY_TO_OPERATE or
        NMT_CS_OPERATIONAL, SoA and SoC frames may have been lost. The DLL Error Handling shall
        be notified.
        DLL_CE_SOC_TIMEOUT [CN NMT state != NMT_CS_PRE_OPERATIONAL_2] / report error E_DLL_LOSS_SOC_TH
        */

        //DLL_CT3
        /*
        In case of a DLL_CE_SOC_TIMEOUT event happened in NMT_CS_READY_TO_OPERATE or
        NMT_CS_OPERATIONAL, SoA and SoC frames may have been lost. The DLL Error Handling shall
        be notified.
        DLL_CE_SOC_TIMEOUT [CN NMT state != NMT_CS_PRE_OPERATIONAL_2] / Synchronise to the
        next SoC, report error E_DLL_LOSS_SOC_TH and E_DLL_LOSS_SOA_TH
        */

        //DLL_CT8
        /*
        In case of a DLL_CE_SOC_TIMEOUT event happened in NMT_CS_READY_TO_OPERATE or
        NMT_CS_OPERATIONAL, SoA and SoC frames may have been lost. On non-multiplexed nodes or if
        a multiplexed node should have been requested this cycle, the PRes frame was additionally lost. The
        DLL Error Handling shall be notified.

        //TODO does the loss of PRes matter?

        DLL_CE_SOC_TIMEOUT [CN NMT state != NMT_CS_PRE_OPERATIONAL_2] / Synchronise on
        the next SoC, report error E_DLL_LOSS_SOC_TH and E_DLL_LOSS_SOA_TH
         */
        //the timeout has happend
        t_nmt_state nmt_state;
        c_nmt <: REQUEST_STATE;
        c_nmt :> nmt_state;  //TODO check for worst case latency
        if(nmt_state != NMT_CS_PRE_OPERATIONAL_2){
          report_lost_frame(E_DLL_LOSS_SOC_TH, error_handler);
          if(dll_state != E_DLL_LOSS_SOC_TH){
            report_lost_frame(E_DLL_LOSS_SOA_TH, error_handler);
          }
        }
        dll_state = DLL_CS_WAIT_SOC;
        break;
      }

      case c_mii :> unsigned mii_resp : {
        if(!mii_resp) {
          //assert(waiting_on_tx_ack);
          waiting_on_tx_ack = FALSE;
        } else {
          uintptr_t rx_buffer_address = mii_resp;
          //TODO find out if I need this
          c_mii :> unsigned frame_presentation_time;
/*
          //filter out
          if(reject_mac(rx_buffer_address)){
            c_mii<:0;
            break;
          }
          */
          if(is_type_powerlink(rx_buffer_address)){
            Message_Type_ID mt_id = get_powerlink_type(rx_buffer_address);
 /*
            switch (dll_state) {
            case DLL_CS_WAIT_PREQ:{
              handle_wait_preq(mt_id, rx_buffer_address);
              break;
            }
            case DLL_CS_WAIT_SOC:{
              handle_wait_soc(mt_id, rx_buffer_address);
              break;
            }
            case DLL_CS_NON_CYCLIC:{
              handle_non_cyclic(mt_id, rx_buffer_address);
              break;
            }
            case DLL_CS_WAIT_SOA:{
              handle_wait_soa(mt_id, rx_buffer_address);
              break;
            }
            }
   */
          }


          c_mii<:0;
        }
        break;
      }
    }


  }

}
