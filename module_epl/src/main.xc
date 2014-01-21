#include <stdio.h>
#include <xscope.h>
#include <stdint.h>
#include "main.h"

#define SHOW_STATE_TRANSITIONS 1


#define MII_ACK 0

typedef enum {
  //NMT_GS_OFF = 0x00, (Impossible)
  //NMT_GS_POWERED   (Super state)
    //NMT_GS_INITIALISATION   (Super state)
      NMT_GS_INITIALISING = 0x09,
      NMT_GS_RESET_APPLICATION = 0x19,
      NMT_GS_RESET_COMMUNICATION = 0x29,
      NMT_GS_RESET_CONFIGURATION = 0x39,
    //NMT_GS_COMMUNICATING   (Super state)
      NMT_CS_NOT_ACTIVE = 0x1c,
      //NMT_CS_EPL_MODE  (Super state)
        NMT_CS_PRE_OPERATIONAL_1 = 0x1d,
        NMT_CS_PRE_OPERATIONAL_2 = 0x5d,
        NMT_CS_READY_TO_OPERATE = 0x6d,
        NMT_CS_OPERATIONAL =0xfd,
        NMT_CS_STOPPED = 0x4d,
      NMT_CS_BASIC_ETHERNET = 0x1e
} nmt_state_t;

typedef enum {
  DLL_CS_NON_CYCLIC,
  DLL_CS_WAIT_SOC,
  DLL_CS_WAIT_PREQ,
  DLL_CS_WAIT_SOA
} t_dll_state;

typedef enum {
  Start_of_Cycle = 0x01,
  PollRequest = 0x03,
  PollResponse = 0x04,
  Start_of_Asynchronous = 0x05,
  Asynchronous_Send = 0x06
  //Active_Managing_Node_Indication  =0x07, //used by EPSG DS302-A [1]
  //Asynchronous_Invite  =0x0d, // used by EPSG DS302-B [2]
} message_type_id_t;


static void handle_nmt_commands(){
  //TODO the below function should probably call something like this to do their functionality
}

static void handle_NMT_GS_INITIALISING(){
  /*
  This is the first sub-state the POWERLINK node shall enter after Power On (NMT_GT1),
  hardware resp. software Reset (NMT_GT2) or the reception of an NMTSwReset (NMT_GT8)
  command. After finishing the basic node initialisation, the POWERLINK node shall
  autonomously enter the sub-state NMT_GS_RESET_APPLICATION (NMT_GT10).
  */
  //TODO
  //send a reset to the error signalling thread

  //At startup (NMT_GT1, NMT_GT2 or NMT_GT8) the CN shall reset the Error Signaling and set EC=1.
  //async_state.EC = 1;

  nmt_state = NMT_GS_RESET_APPLICATION;
  if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_APPLICATION");
}
static void handle_NMT_GS_RESET_APPLICATION(){
  /*
  In this sub-state, the parameters of the manufacturer-specific profile area and of the
  standardised device profile area shall be set to their PowerOn values. After setting the PowerOn
  values, the sub-state NMT_GS_RESET_COMMUNICATION shall be entered autonomously
  (NMT_GT11).
  NMT_GS_RESET_APPLICATION shall be entered upon the reception of an NMTResetNode
  command from all sub-states of NMT_GS_COMMUNICATING, e.g. the NMT MN resp. CN state
  machine.
  */
  //TODO
  //init all the power on values
/*
 *
 //TODO
  for(everything in CO OD)
    if it had a default then set the value to the default
  */
  nmt_state = NMT_GS_RESET_COMMUNICATION;
  if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_COMMUNICATION");

}
static void handle_NMT_GS_RESET_COMMUNICATION(){
  /*
  In this sub-state the parameters of the communication profile area shall be set to their PowerOn
  values.
  NMT_GS_RESET_COMMUNICATION shall be entered upon the recognition of an internal
  communication error or the reception of an NMTResetCommunication command from all sub-
  states of NMT_GS_COMMUNICATING, e.g. the MN resp. CN NMT state machine.
  PowerOn values are the last stored parameters. If no stored configuration is available or if the
  Reset was preceded by a restore default command (object NMT_RestoreDefParam_REC), the
  PowerOn values shall be set to the default values according to the communication and device
  profile specifications.

  If parameters of the object dictionary concerning the devices cycle configuration (node assigment
  and timing) are changed, the modification shall take effect after the NMTResetConfiguration
  command is received.
  */

  //TODO
  //init all the power on values
  //sdo_seq_init_layer();

  nmt_state = NMT_GS_RESET_CONFIGURATION;
  if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_CONFIGURATION");

}
static void handle_NMT_GS_RESET_CONFIGURATION(){
  /*
  In this sub-state the configuration parameters set in the object dictionary are used to generate
  the active device configuration. The node shall examine its Node ID in order to decide if its
  configured to be an MN or a CN. If the node is equal to C_ADR_MN_DEF_NODE_ID , the node
  shall enter the MN NMT state machine (NMT_MT1), otherwise the CN NMT state machine shall
  be entered (NMT_CT1).
  NMT_GS_RESET_CONFIGURATION shall be entered upon the reception of an
  NMTResetConfiguration command from all substates of NMT_GS_COMMUNICATING.
  This sub-state is used to re-configure devices which do not support storing of communication
  parameters.
  */

  nmt_state = NMT_CS_NOT_ACTIVE;
  if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_NOT_ACTIVE");

  //set the time that NMT_CS_NOT_ACTIVE began
  //t :> frame_rx_time;

}

//This should copy the MC and PS flags into this node, i.e.
static void update_flags_MC_PS(uint8_t rx_buffer[]){
  uint8_t rx_flags = rx_buffer[14 + 4];
  rx_flags &= 0b11000000;
  uint8_t flags_lo;
  flags_lo &= 0b00111111;
  flags_lo |= rx_flags;
}

static void handle_NMT_CS_NOT_ACTIVE(){

  if(filter_on_dst_and_mac(rx_frame))
    return;

  //DLL is still noncyclic
  message_type_id_t mt_id = get_powerlink_type(rx_frame);
  switch(mt_id){
  case Start_of_Asynchronous:
  case Start_of_Cycle:{
    nmt_state = NMT_CS_PRE_OPERATIONAL_1;
    if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> (NMT_CT2) -> NMT_CS_PRE_OPERATIONAL_1");
    break;
  }
  case Asynchronous_Send:{
   /*frame accepted
    *
    * SDO reception - no frame handling
    * NMT Command   - only selected NMT commands accepted, shall cause state transition,
    *                 reception requires previous loss of SoA
    * other protocols - no frame handling
    *
    * if(NMT Command == NMTResetNode)          state => NMT_GS_RESET_APPLICATION
    * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
    * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
    * if(NMT Command == NMTSwReset)            state => NMT_GS_INITIALISING
    * if(NMT anything else) log an error
    */
    nmt_command_id_t valid_commands[4] = {NMTResetNode, NMTResetCommunication,
            NMTResetConfiguration, NMTSwReset};
    process_asnd(
        rx_buffer_address,  // pointer
        valid_commands, 4,  // nmt commands accepted at the time
        false               // SDO cmds accepted
    );
    break;
  }
  case PollRequest:
  case PollResponse:{
    //no frame handling
    break;
  }
  default: _builtin_unreachable();break;
  }
}

static void handle_soa(){

}

static void handle_NMT_CS_PRE_OPERATIONAL_1(){

  if(filter_on_dst_and_mac(rx_frame))
    return;

  //DLL is still noncyclic
  message_type_id_t mt_id = get_powerlink_type(rx_buffer_address);
  switch(mt_id){
  case Start_of_Asynchronous:{
    handle_soa();
    break;
  }
  case Start_of_Cycle:{
    //frame accepted, triggers state transition
    dll_sm_state = DLL_CS_WAIT_PREQ;
    update_flags_MC_PS();
    soc_rx_time = time;
    nmt_state = NMT_CS_PRE_OPERATIONAL_2;
    if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> (NMT_CT4) -> NMT_CS_PRE_OPERATIONAL_2");
    break;
  }
  case Asynchronous_Send:{
    /*frame accepted
     * SDO reception - frame data interpreted resp. transmitted
     * NMT Command - may cause state transition
     * other protocols - frame data interpreted resp. transmitted
     *
     * if(NMT Command == NMTResetNode) state => NMT_GS_RESET_APPLICATION
     * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
     * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
     * if(NMT Command == NMTSwReset) state => NMT_GS_INITIALISING
     * if(NMT anything else) log an error
     *
     */
    nmt_command_id_t valid_commands[4] = {NMTResetNode, NMTResetCommunication,
            NMTResetConfiguration, NMTSwReset};
    process_asnd(
        rx_buffer_address,  // pointer
        valid_commands, 4,  // nmt commands accepted at the time
        true                // SDO cmds accepted
    );
    break;
  }
  case PollRequest:
  case PollResponse:{
    //no frame handling
    break;
  }
  default: _builtin_unreachable();break;
  }

}

handle_soc(rx_buffer, flags, dll_state_t dll_sm_state){
  dll_state_t dll_sm_state;
  if(dll_sm_state == DLL_CS_WAIT_PREQ || dll_sm_state == DLL_CS_WAIT_SOA){
    report_error(c_eh, E_DLL_LOSS_SOA_TH);
  }
  //TODO report start of cycle to error handler - why?
  dll_sm_state = DLL_CS_WAIT_PREQ;
  update_flags_MC_PS();
  soc_rx_time = time;
  //TODO net time
  //TODO relative time
  //TODO cycle number
  //TODO dia counters DIA_NMTTelegrCount_REC:IsochrCyc_U32 ++
}

handle_preq(rx_buffer, flags, dll_state_t dll_sm_state){

  //TODO flags
  //MS - Shall be set in PReq frames to CNs that are served by a multiplexed timeslot
  //EA - Error signaling, refer 6.5.2
  //RD - if not set then ignore data

  switch(dll_sm_state){
  case DLL_CS_WAIT_PREQ:{
    //TODO process the request
    send_pres();

    process_preq();
    prrepare_pres();

    dll_sm_state = DLL_CS_WAIT_SOA;
    break;
  }
  case DLL_CS_WAIT_SOA:{
    //TODO find out it we process this frame or not

    report_error(c_eh, E_DLL_LOSS_SOA_TH);
    report_error(c_eh, E_DLL_LOSS_SOC_TH);

    dll_sm_state = DLL_CS_WAIT_SOC;
    break;
  }
  case DLL_CS_WAIT_SOC:{
    //DLL_CE_PREQ [ ] / respond with PRes frame, report error DLL_CEV_LOSS_SOC
    //If a SoA, PReq or PRes frame is received, there may be a loss of a SoC frame in between. The DLL
    //Error Handling shall be notified with the error DLL_CEV_LOSS_SOC

    send_pres();
    process_preq();
    prrepare_pres();
    report_error(c_eh, E_DLL_LOSS_SOC_TH);
    //stay in this state
    break;
  }
  default: _builtin_unreachable();break;
  }

}

handle_pres(rx_buffer, flags, dll_state_t dll_sm_state){

  //TODO flags

  switch(dll_sm_state){
  case DLL_CS_WAIT_SOA:
  case DLL_CS_WAIT_PREQ:{
    //(DLL_CT7)
    //if configured to do so:
    process_pres(); //this is not for us but it might have cross traffic in it
    dll_sm_state = DLL_CS_WAIT_PREQ;
    break;
  }
  case DLL_CS_WAIT_SOC:{
    //TODO I assume there is no processing in this case?
    report_error(c_eh, E_DLL_LOSS_SOC_TH);
    //no state change this time
    break;
  }
  default: _builtin_unreachable();break;
  }

}

handle_asnd(rx_buffer, flags, dll_state_t dll_sm_state){

  //TODO flags

  switch(dll_sm_state){
  case DLL_CS_WAIT_SOA:{
    report_error(c_eh, E_DLL_LOSS_SOA_TH);
    //DLL_CT10 ASnd frames and non POWERLINK frames shall be acceptedduring the isochronous phase.
    //TODO does this mean process it or not?
    break;
  }
  case DLL_CS_WAIT_PREQ:{
    report_error(c_eh, E_DLL_LOSS_SOA_TH);
    break;
  }
  case DLL_CS_WAIT_SOC:{
    process_asnd();
    /*frame accepted
     * SDO reception - frame data interpreted resp. transmitted
     * NMT Command - may cause state transition
     * other protocols - frame data interpreted resp. transmitted
     *
     * if(NMT Command == NMTStopNode) state => NMT_CS_STOPPED
     * if(NMT Command == NMTEnableReadyToOperate) state => NMT_CS_PRE_OPERATIONAL_2
     * if(NMT Command == NMTResetNode) state => NMT_GS_RESET_APPLICATION
     * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
     * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
     * if(NMT Command == NMTSwReset) state => NMT_GS_INITIALISING
     * if(NMT anything else) log an error
     *
     */
    nmt_command_id_t valid_commands[6] = {NMTStopNode, NMTEnableReadyToOperate,
        NMTResetNode, NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
    process_async_frame(rx_buffer_address, valid_commands, 6, TRUE, c_eh, async_state, c_can_open)

    break;
  }
  default: _builtin_unreachable();break;
  }
}

static void handle_NMT_CS_EPL_MODE(){
  if(filter_on_dst_and_mac(rx_frame))
    return;
  message_type_id_t mt_id = get_powerlink_type(rx_buffer_address);
  switch(mt_id){
  case Start_of_Asynchronous:{handle_soa();break;}
  case Start_of_Cycle:{ handle_soc();  break;}
  case Asynchronous_Send:{ handle_asnd();;break;}
  case PollRequest:{ handle_preq();;break;}
  case PollResponse:{ handle_pres();;break;}
  default: _builtin_unreachable();break;
  }
}

static void handle_NMT_CS_PRE_OPERATIONAL_2(){
  handle_NMT_CS_EPL_MODE();
}

static void handle_READY_TO_OPERATE(){
  handle_NMT_CS_EPL_MODE();
}

static void handle_CS_OPERATIONAL(){
  handle_NMT_CS_EPL_MODE();
}


static void handle_nmt_state(uintptr_t rx_frame){
  nmt_state_t nmt_sm_state;
  switch(nmt_sm_state){
  case NMT_GS_INITIALISING:{
    handle_NMT_GS_INITIALISING();
    break;
  }
  case NMT_GS_RESET_APPLICATION:{
    handle_NMT_GS_RESET_APPLICATION();
    break;
  }
  case NMT_GS_RESET_COMMUNICATION:{
    handle_NMT_GS_RESET_COMMUNICATION();
    break;
  }
  case NMT_GS_RESET_CONFIGURATION:{
    handle_NMT_GS_RESET_CONFIGURATION();
    break;
  }
  //NMT_GS_COMMUNICATING
  case NMT_CS_NOT_ACTIVE:{
    handle_NMT_CS_NOT_ACTIVE();
    break;
  }
  case NMT_CS_PRE_OPERATIONAL_1:{
    handle_NMT_CS_PRE_OPERATIONAL_1();
    break;
  }
  case NMT_CS_PRE_OPERATIONAL_2:{
    handle_NMT_CS_PRE_OPERATIONAL_2();
    break;
  }
  case NMT_CS_READY_TO_OPERATE:{
    handle_NMT_CS_READY_TO_OPERATE();
    break;
  }
  case NMT_CS_OPERATIONAL:{
    handle_NMT_CS_OPERATIONAL();
    break;
  }
  case NMT_CS_STOPPED:{
    handle_NMT_CS_STOPPED();
    break;
  }
  case NMT_CS_BASIC_ETHERNET:{
    handle_NMT_CS_BASIC_ETHERNET();
    break;
  }
  default:{
    _builtin_unreachable();
  }
  }
}

void epl(streaming chanend c_mii, chanend c_eh, chanend c_app, chanend c_pdo_manager){
  timer basic_ethernet_timer;
  unsigned basic_ethernet_timeout = 0;

  timer soc_timer;
  unsigned soc_timeout = 0;


  while(1){
#pragma ordered
    select{
      case c_mii :> unsigned data :{
        if(data == MII_ACK){

        } else {
          uintptr_t rx_frame = (uintptr_t)data;
          handle_nmt_state(rx_frame);
        }
        break;
      }
      case basic_ethernet_timeout => basic_ethernet_timer when
        timerafter(basic_ethernet_timeout):> basic_ethernet_timeout:{
        break;
      }
      case soc_timeout => soc_timer when timerafter(soc_timeout) :> soc_timeout :{
        if(nmt_state != NMT_CS_PRE_OPERATIONAL_2){
          report_error(c_eh, DLL_CEV_LOSS_SOC);
          report_error(c_eh, DLL_CEV_LOSS_SOA);
        }
        soc_timeout = 0;
        break;
      }
      case c_app :> unsigned cmd:{
        break;
      }

    }
  }
}
