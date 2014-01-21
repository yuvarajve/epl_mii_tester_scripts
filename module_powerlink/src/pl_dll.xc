#include <stdint.h>
#include "powerlink.h"
#include "pl_dll.h"
#include "pl_error_defines.h"
#include <stdio.h>
#define DLL
#include "thread_id.h"
#include "global_state.h"


#define PRES_BUFFERS 2

typedef enum {
  DLL_CS_NON_CYCLIC,
  DLL_CS_WAIT_SOC,
  DLL_CS_WAIT_PREQ,
  DLL_CS_WAIT_SOA
} t_dll_state;


typedef struct {
  unsigned pres_buf_active;
  unsigned pres_size[PRES_BUFFERS];
  uintptr_t pres_pointer[PRES_BUFFERS];

  unsigned cycle_number;
  unsigned rx_time_of_soc;
  t_dll_state state;

} t_dll;


/*
 * Return true if the powerlink frame is to be filtered
 */
int filter_powerlink_dst(uintptr_t pointer, unsigned node_id);
uint16_t get_ethertype(uintptr_t pointer);
int is_type_powerlink(uintptr_t pointer);
void prepare_PRes(uintptr_t tx_buffer_address, chanend c_can_open);

#define PDOVersion 0x20
uint8_t pres_buffer[PRES_BUFFERS][C_DLL_ISOCHR_MAX_PAYL] = {
{
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    PollResponse,
    C_ADR_MN_DEF_NODE_ID, //0xf0
    0x00, 0x00, //flags set by the nmt
    PDOVersion,
    0x00,     //reserved
    0x00, 0x00
    //payload
},
{
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    PollResponse,
    C_ADR_MN_DEF_NODE_ID, //0xf0
    0x00, 0x00, //flags set by the nmt
    PDOVersion,
    0x00,     //reserved
    0x00, 0x00
    //payload
}

};

static void report_lost_frame(t_error_codes code, chanend error_handler){
  error_handler <: code;
}
/*
static void report_start_of_cycle(chanend error_handler){
  error_handler <: NO_ERROR_SOC;
}

static int is_multiplexed(){
 // return can_open_read_UNSIGNED8(NMT_CycleTiming_TYPE, MultiplCycleCnt_U8) < 2;
}
*/

/*
 * This passes the current pres to the nmt and changes the active index
 */
static void pass_current_pres(chanend c, t_dll &dll_state){
  uintptr_t p = dll_state.pres_pointer[dll_state.pres_buf_active];
  unsigned size = dll_state.pres_size[dll_state.pres_buf_active];
  dll_state.pres_buf_active = (dll_state.pres_buf_active+1)%PRES_BUFFERS;
  c <: p;
  c <: size;
}

static void handle_wait_preq(
    uintptr_t rx_buffer_address,
    unsigned frame_presentation_time,
    t_dll &state,
    chanend c_nmt,
    chanend c_eh,
    chanend c_can_open){

  message_type_id_t msg_type = get_powerlink_type(rx_buffer_address);

  switch (msg_type) {
  case Start_of_Cycle : {
    /* DLL_CT7
    The reaction to a SoC frame is state independent. The state machine synchronises to the start of a new cycle.
    DLL_CE_SOC [ ] / synchronise to the cycle begin, report error E_DLL_LOSS_SOA_TH */

    //TODO
    //process_SoC(rx_buffer_address);

    //report_start_of_cycle(c_eh);
    //report_lost_frame(E_DLL_LOSS_SOA_TH, c_eh);
    state.cycle_number++;
    state.rx_time_of_soc = frame_presentation_time;
    break;
  }
  case PollRequest : {
    /* DLL_CT2
    The PReq event occurs within the isochronous phase of communication.
    DLL_CE_PREQ [ ] / Process the PReq frame and send a PRes frame */


    //only do this if
    // NMT_CS_PRE_OPERATIONAL_2 :
    // NMT_CS_READY_TO_OPERATE :
    // NMT_CS_OPERATIONAL
    //first thing to do is hand the previous pres over to the nmt
    c_nmt :> unsigned;
    pass_current_pres(c_nmt, state);

    //TODO if we needed more speed then we could pass this off to the can thread to do
    process_PReq(rx_buffer_address, c_can_open);
    prepare_PRes(state.pres_pointer[state.pres_buf_active], c_can_open);

    //last_poll_request_cycle = cycle_number;
    state.state = DLL_CS_WAIT_SOA;

    break;
  }
  case PollResponse : {
    /* DLL_CT7
    If a PRes frame of another CN was received (cross traffic), the Pres frame shall be processed (if
    configured to do so). The CN waits for either a PRes from another CN (cross traffic) or a PReq frame.
    DLL_CE_PRES [ ] / process PRes frames (cross traffic) */

    //TODO
    //process_PRes(rx_buffer_address, c_can_open, node_id);

    break;
  }
  case Start_of_Asynchronous : {
    /* DLL_CT8
    If the CN is in the NMT_CS_OPERATIONAL or NMT_CS_READY_TO_OPERATE the CN will
    assume a LOSS_OF_PREQ if the number of cycles since the last PReq is greater than that
    expected. (1 for non multiplexed CNs, n for multiplexed CNs where n is NMT_CycleTiming_REC.MultipleCycleCnt_U8)

    DLL_CE_SOA [ CN = multiplexed ] / process SoA; if invited, transmit a legal Ethernet frame
    DLL_CE_SOA [ CN != multiplexed ] / process SoA; if invited, transmit a legal Ethernet frame, additionally report error E_DLL_LOSS_PREQ_TH */

    //TODO

    break;
  }
  case Asynchronous_Send : {
    /* DLL_CT7
    ASnd frames and non POWERLINK frames shall be processed during the isochronous phase.
    DLL_CE_ASND [ ] / report error E_DLL_LOSS_SOA_TH */
    //TODO

    break;
  }
  }

  #if 0
  switch (msg_type) {
  case Start_of_Cycle : {
    /* DLL_CT7
    The reaction to a SoC frame is state independent. The state machine synchronises to the start of a new cycle.
    DLL_CE_SOC [ ] / synchronise to the cycle begin, report error E_DLL_LOSS_SOA_TH */
    rx_time_of_soc = frame_presentation_time;
    process_SoC(rx_buffer_address);
    report_start_of_cycle(c_eh);
    report_lost_frame(E_DLL_LOSS_SOA_TH, c_eh);
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

    nmt_state_t nmt_state;

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
#endif
}

void handle_wait_soc(
    uintptr_t rx_buffer_address,
    unsigned frame_presentation_time,
    t_dll &state,
    chanend c_nmt,
    chanend c_eh,
    chanend c_can_open){

  message_type_id_t msg_type = get_powerlink_type(rx_buffer_address);

  switch (msg_type) {
  case Start_of_Cycle : {
    /* DLL_CT1
    The occurrence of the SoC event indicates the beginning of a new POWERLINK cycle. The
    asynchronous phase of the previous cycle ends and the isochronous phase of the next cycle begins.
    DLL_CE_SOC [ ] / synchronise the start of cycle and generate a SoC trigger to the application */
    //rx_time_of_soc = frame_presentation_time;
   // process_SoC(rx_buffer_address, c_can_open, latest_SoC);
    state.state = DLL_CS_WAIT_PREQ;
   // report_start_of_cycle(error_handler);
   // cycle_number++;
    break;
  }
#if 0
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
#endif
  //default : __builtin_trap(); break;
  }
}

void handle_wait_soa(
    uintptr_t rx_buffer_address,
    unsigned frame_presentation_time,
    t_dll &state,
    chanend c_nmt,
    chanend c_eh,
    chanend c_can_open){

  message_type_id_t msg_type = get_powerlink_type(rx_buffer_address);

  switch (msg_type) {
#if 0
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
#endif
  case Start_of_Asynchronous : {
    //DLL_CT3
    /*
    The DLL_CE_SOA event denotes the end of the isochronous phase and the beginning of the
    asynchronous phase of the current cycle. If the SoA frame includes an invitation to the CN, the CN
    may respond with one valid frame.
    DLL_CE_SOA [ ] / process SoA, if allowed send an ASnd frame or a non POWERLINK frame
    */
    //this will be processed by the NMT
    state.state = DLL_CS_WAIT_SOC;
    break;
  }
#if 0
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
#endif
  default : __builtin_trap(); break;
  }
}

static void switch_to_cyclic(unsigned * pl_cycle_time, chanend c_can_open){
  //NMT_CNBasicEthernetTimeout_U32
  //NMT_CycleTiming_REC.MultiplCycleCnt_U8 and NMT_MultiplCycleAssign_AU8.
/*
  NMT_CycleTiming_REC.MultiplCycleCnt_U8 defines the length of the multiplexed cycle in
  POWERLINK cycle counts. If NMT_CycleTiming_REC.MultiplCycleCnt_U8 is zero, the multiplexed
  access method shall not be applied, e.g. all CNs shall be accessed continuously.
  The respective sub-index of NMT_MultiplCycleAssign_AU8 defines the cycle count inside the
  multiplexed cycle, when the respective CN shall be polled by the MN. If the sub-index is zero, the CN
  shall be accessed continuously.
  The order in which the CNs are polled by the MN may be set up by object
  NMT_MultiplCycleAssign_AU8.
*/
}



/*
 * Object 1101h: DIA_NMTTelegrCount_REC
 * NumberOfEntries
 * IsochrCyc_U32      This sub-index holds the number of transmitted (MN) or received (CN) SoC frames.
 * IsochrRx_U32       This sub-index holds the number of received PReq and PRes frames.
 * IsochrTx_U32       This sub-index holds the number of transmitted PReq and PRes frames.
 * AsyncRx_U32        This sub-index holds the number of received asynchronous frames (POWERLINK ASnd, IP frames etc., but not SoA).
 * AsyncTx_U32        This sub-index holds the number of transmitted asynchronous frames (POWERLINK ASnd, IP frames etc., but not SoA).
 * SdoRx_U32          This sub-index holds the number of received SDO telegrams via UDP/IP or POWERLINK ASnd.
 * SdoTx_U32          This sub-index holds the number of transmitted SDO telegrams via UDP/IP or POWERLINK ASnd.
 * Status_U32         This sub-index holds the number of received StatusRequest SoA telegrams.
 *
 *
 *
 * Object 1F98h: NMT_CycleTiming_REC
 * NumberOfEntries
 * IsochrTxMaxPayload_U16   Provides the device specific upper limit for payload data size in octets of isochronous messages to be transmitted by the device
 * IsochrRxMaxPayload_U16   Provides the device specific upper limit for payload data size in octets of isochronous messages to be received by the device.
 * PResMaxLatency_U32       Provides the maximum time in ns, that is required by the CN to respond to PReq.
 * PReqActPayloadLimit_U16  Provides the configured PReq payload data slot size in octets expected by the CN. Note: This results in a fixed frame size regardless of the size of PDO data used.
 * PResActPayloadLimit_U16  Provides the configured PReq payload data slot size in octets expected by the CN.
 * ASndMaxLatency_U32       Provides the configured PRes payload data slot size in octets sent by the CN.
 * MultiplCycleCnt_U8       This sub-index describes the length of the multiplexed cycle in multiples of the POWERLINK cycle.
 * AsyncMTU_U16             This sub-index describes the maximum asynchoronous frame size in octets.
 * Prescaler_U16            This sub-index configurates the toggle rate of the SoC PS flag.
 *
 *NMT_MultiplCycleAssign_AU8
 *NumberOfEntries
 *01h - FEh: CycleNo
 *
 *
 *
 *
 *
 *
 *
 */



static void init(t_dll &state){
  state.state = DLL_CS_NON_CYCLIC;
  state.state = DLL_CS_WAIT_PREQ;
  for(unsigned i=0;i<PRES_BUFFERS;i++){
    state.pres_size[i] = 60;
    asm("mov %0, %1": "=r"(state.pres_pointer[i]):"r"(pres_buffer[i]));
  }
  state.pres_buf_active = 0;
  state.cycle_number = 0;

}

/*
 * The DLL never transmits a frame directly. Data (PRes frames) are requested by the NMT to be sent)
 *
 */
void pl_dll(streaming chanend c_mii, chanend c_eh, chanend c_nmt, chanend c_can_open){

  t_dll dll_state;
  init(dll_state);

  timer t;
  unsigned rx_time_of_soc;
  unsigned pl_cycle_time = 0;// co_read_NMT_CycleLen_U32(c_can_open);//TODO check this

  while(1){

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
        nmt_state_t nmt_state = get_nmt_status();
        if(nmt_state != NMT_CS_PRE_OPERATIONAL_2){
         // report_lost_frame(E_DLL_LOSS_SOC_TH, error_handler);
         // if(dll_state != E_DLL_LOSS_SOC_TH){
           // report_lost_frame(E_DLL_LOSS_SOA_TH, error_handler);
          //}
        }
       // dll_state = DLL_CS_WAIT_SOC;
        break;
      }
      case c_nmt :> unsigned cmd :{
        pass_current_pres(c_nmt, dll_state);
        break;
      }
      case c_mii :> unsigned mii_resp : {
        uintptr_t rx_buffer_address = mii_resp;
        unsigned frame_presentation_time;
        t:> frame_presentation_time;

        //filter out
        /*
        if(reject_mac(rx_buffer_address)){
          printf("rejected - mac filtered\n");
          c_mii<:0;
          break;
        }
*/

        if(is_type_powerlink(rx_buffer_address)){

          //get the nmt state
          nmt_state_t nmt_state = get_nmt_status();


          switch (dll_state.state) {
          case DLL_CS_WAIT_PREQ:{
            handle_wait_preq( rx_buffer_address, frame_presentation_time, dll_state,
                c_nmt, c_eh, c_can_open);
            break;
          }
          case DLL_CS_WAIT_SOC:{
            handle_wait_soc(rx_buffer_address, frame_presentation_time, dll_state,
                c_nmt, c_eh, c_can_open);
            break;
          }
          case DLL_CS_NON_CYCLIC:{
            //don't do anything
            break;
          }
          case DLL_CS_WAIT_SOA:{
            handle_wait_soa(rx_buffer_address, frame_presentation_time, dll_state,
            c_nmt, c_eh, c_can_open);
            break;
          }
          }
        }


        c_mii<:0;
        break;
      }
    }


  }
  return;
}
