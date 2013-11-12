#include <xs1.h>
#include "pl_nmt.h"
#include "pl_defines.h"
#include "can_open.h"
#include "thread_id.h"
#include "pl_error_defines.h"
#include <print.h>
#include "debug.h"
#include <xscope.h>

/*
 * This is used for broadcasting the NMT state. All interested parties have to poll this variable.
 */
t_nmt_state g_nmt_status;






void request_status_response_from_eh(chanend c_eh, ASync_state &async_state){
  c_eh <: request_status_response;
  c_eh :> async_state.SoA_response_p;
  c_eh :> async_state.SoA_response_size;
}

static void init_nmt_state(ASync_state &async_state, unsigned &frame_rx_time, timer t, int &nmt_cs_communicating){

  switch(g_nmt_status) {
  case NMT_GS_INITIALISING: {
    /*
    This is the first sub-state the POWERLINK node shall enter after Power On (NMT_GT1),
    hardware resp. software Reset (NMT_GT2) or the reception of an NMTSwReset (NMT_GT8)
    command. After finishing the basic node initialisation, the POWERLINK node shall
    autonomously enter the sub-state NMT_GS_RESET_APPLICATION (NMT_GT10).
    */
    //TODO


    //At startup (NMT_GT1, NMT_GT2 or NMT_GT8) the CN shall reset the Error Signaling and set EC=1.
    async_state.EC = 1;
    g_nmt_status = NMT_GS_RESET_APPLICATION;
    break;
  }
  case NMT_GS_RESET_APPLICATION: {
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
    g_nmt_status = NMT_GS_RESET_COMMUNICATION;
    break;
  }
  case NMT_GS_RESET_COMMUNICATION: {
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
    g_nmt_status = NMT_GS_RESET_CONFIGURATION;
    break;
  }
  case NMT_GS_RESET_CONFIGURATION: {
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
    g_nmt_status = NMT_CS_NOT_ACTIVE;
    //set the time that NMT_CS_NOT_ACTIVE began
    t :> frame_rx_time;
    nmt_cs_communicating = TRUE;
    break;
  }
  }
}

void process_soa(uintptr_t rx_buffer_address, t_frame_SoA &latest_SoA);
void process_ASnd_or_nonpowerlink_frame(uintptr_t rx_buffer_address,
    chanend c_eh, ASync_state &async_state);
uint16_t get_ethertype(uintptr_t pointer);
Message_Type_ID get_powerlink_type(uintptr_t pointer);
int is_type_powerlink(uintptr_t pointer);

static void init_async_state(ASync_state &async_state){
  //g_nmt_status = NMT_GS_INITIALISING;
  g_nmt_status = NMT_CS_NOT_ACTIVE;


  /*
   * At startup (NMT_GT1, NMT_GT2 or NMT_GT8) the CN shall reset the Error Signaling and set EC=1.
   * The CN shall not change the EC flag before at least 1 valid frame with ER=1 was received.
   */
  //TODO move this into initialisation
  async_state.EC = 1;

}

void report_error(chanend c_eh, unsigned error){
  c_eh <: error;//FIXME send the correct error to the eh
}


static void process_async_frame(
    uintptr_t rx_buffer_address,
    t_nmt_command valid_commands[],
    unsigned valid_cmd_count,
    int frame_handling,
    chanend c_eh,
    ASync_state &async_state){

  uintptr_t nmt_cmd;
  if(is_nmt_command(rx_buffer_address, nmt_cmd)){
    if(is_valid_nmt_command(nmt_cmd, valid_commands, valid_cmd_count)){
     // handle_nmt_command_p(nmt_cmd, nmt_state);
    } else {
     // report_bad_nmt_command(c_eh);
    }
  } else {
    if(frame_handling)
      process_ASnd_or_nonpowerlink_frame(rx_buffer_address, c_eh, async_state);
      //handle_known_asnd(rx_buffer_address);
  }
}



int reject_pl_dst(uintptr_t rx_buffer_address, unsigned node_id);

/*
 * This is used to fill in the single byte that holds the PR and RS flags. It
 * is applicable for:
 *  - PRes
 *  - ASnd: IdentResponse via an IdentRequest SoA frame.
 *  - ASnd: StatusResponse via an StatusRequest SoA frame.
 *
 *  not applicable for:
 *   - ASnd: NMTRequest via an NMTRequestInvite SoA frame.
 *
 */
static void set_PR_and_RS(ASync_state a){
  uint8_t flags = (a.RS) + (a.PR << 3);
  asm("st8 %0, %1[%2]"::"r"(flags),"r"(a.SoA_response_p), "r"(19));
}

static void set_NMT_status(ASync_state a){
  asm("st8 %0, %1[%2]"::"r"(g_nmt_status),"r"(a.SoA_response_p), "r"(20));
}

static void set_EN_and_EC(ASync_state a){
  uint8_t flags = (a.EN << 4) + (a.EC << 3);
  asm("st8 %0, %1[%2]"::"r"(flags),"r"(a.SoA_response_p), "r"(18));
}

/*
 * ASnd responses are for:
 *  - The POWERLINK ASnd frame shall use the POWERLINK addressing scheme and shall be
 *    sent via unicast or broadcast to any other node.
 *  - A Legacy Ethernet message may be sent.
 *   - ASnd: IdentResponse
 *   - ASnd: StatusResponse
 *   - ASnd: NMTRequest
 */
static void send_asnd(ASync_state a, streaming chanend c_mii){
   //DIA_NMTTelegrCount_REC.AsyncTx_U32 ++;
  c_mii <: make_tx_req_p(a.SoA_response_p, a.SoA_response_size, 1);
  a.asnd_invite_response = 0;
}

#define FRAME_PROCESS_ID 0

void handle_SoC(ASync_state async_state){
  xscope_int(FRAME_PROCESS_ID, SoC);
  switch(g_nmt_status){
    //NMT_GS_INITIALISATION
    case NMT_GS_INITIALISING:
    case NMT_GS_RESET_APPLICATION:
    case NMT_GS_RESET_COMMUNICATION:
    case NMT_GS_RESET_CONFIGURATION:{
      //frame not accepted
      break;
    }
    case NMT_CS_NOT_ACTIVE : {
      //frame not accepted

      /*  (NMT_CT2)
       *  If a SoA or SoC frame is received in NMT_CS_NOT_ACTIVE,
       *  the node shall change over to the state NMT_CS_PRE_OPERATIONAL_1.
       */
      g_nmt_status = NMT_CS_PRE_OPERATIONAL_1;

      break;
    }
    case NMT_CS_PRE_OPERATIONAL_1 : {
      //frame accepted, triggers state transition

      /* (NMT_CT4)
       * If the node receives a SoC frame in NMT_CS_PRE_OPERATIONAL_1,
       * the node shall change over to NMT_CS_PRE_OPERATIONAL_2.
       */
      g_nmt_status = NMT_CS_PRE_OPERATIONAL_2;
      async_state.RD = 0;
      break;
    }
    case NMT_CS_PRE_OPERATIONAL_2 :
    case NMT_CS_READY_TO_OPERATE :
    case NMT_CS_OPERATIONAL :
    case NMT_CS_STOPPED :  {
      //frame accepted
      break;
    }
    case NMT_CS_BASIC_ETHERNET : {
      //frame accepted, triggers state transition
      /* (NMT_CT12)
       * If a SoC, PReq, PRes or SoA frame is received in
       * NMT_CS_BASIC_ETHERNET, the node shall change over to
       * NMT_CS_PRE_OPERATIONAL_1.
       */
      g_nmt_status = NMT_CS_PRE_OPERATIONAL_1;
      break;
    }
  }
}

void handle_SoA(uintptr_t rx_buffer, int &waiting_on_tx_ack,
    ASync_state async_state, streaming chanend c_mii, chanend c_eh){
  //xscope_int(FRAME_PROCESS_ID, SoA);

  switch(g_nmt_status){
    //NMT_GS_INITIALISATION
    case NMT_GS_INITIALISING:
    case NMT_GS_RESET_APPLICATION:
    case NMT_GS_RESET_COMMUNICATION:
    case NMT_GS_RESET_CONFIGURATION:{
      //frame not accepted
      break;
    }
    case NMT_CS_NOT_ACTIVE : {
      //frame accepted, triggers state transition

      /*  (NMT_CT2)
       *  If a SoA or SoC frame is received in NMT_CS_NOT_ACTIVE,
       *  the node shall change over to the state NMT_CS_PRE_OPERATIONAL_1.
       */
      g_nmt_status = NMT_CS_PRE_OPERATIONAL_1;
      break;
    }
    case NMT_CS_PRE_OPERATIONAL_1 :
    case NMT_CS_PRE_OPERATIONAL_2 :
    case NMT_CS_READY_TO_OPERATE :
    case NMT_CS_OPERATIONAL :
    case NMT_CS_STOPPED :
    {
      //frame accepted

      process_SoA(rx_buffer, async_state, c_eh);

      //if invited I must respond
   //   assert(waiting_on_tx_ack == FALSE);

      switch(async_state.asnd_invite_response){
        case 0 : break;
        case IDENT_RESPONSE:{
          //fill in the nmt status
          set_NMT_status(async_state);

          //fill in the PR and RS flags
          set_PR_and_RS(async_state);
          assert(waiting_on_tx_ack == FALSE);
          send_asnd(async_state, c_mii);
          waiting_on_tx_ack = TRUE;
          break;
        }
        case STATUS_RESPONSE:{
          //fill in the nmt status
          set_NMT_status(async_state);

          //set EN and EC
          set_EN_and_EC(async_state);

          //fill in the PR and RS flags
          set_PR_and_RS(async_state);
          assert(waiting_on_tx_ack == FALSE);
          send_asnd(async_state, c_mii);
          waiting_on_tx_ack = TRUE;
          break;
        }
        case SDO:
        case NMT_COMMAND:
        case NMT_REQUEST:{
          //no flags
          assert(waiting_on_tx_ack == FALSE);
          send_asnd(async_state, c_mii);
          waiting_on_tx_ack = TRUE;
          break;
        }
      }
      break;
    }
    case NMT_CS_BASIC_ETHERNET : {
      //frame accepted, triggers state transition
      /* (NMT_CT12)
       * If a SoC, PReq, PRes or SoA frame is received in
       * NMT_CS_BASIC_ETHERNET, the node shall change over to
       * NMT_CS_PRE_OPERATIONAL_1.
       */
      g_nmt_status = NMT_CS_PRE_OPERATIONAL_1;
    //  process_SoA(rx_buffer_address, async_state);
      break;
    }
  }
}

void handle_PRes(){
  xscope_int(0, PRes);
  switch(g_nmt_status){
    //NMT_GS_INITIALISATION
    case NMT_GS_INITIALISING:
    case NMT_GS_RESET_APPLICATION:
    case NMT_GS_RESET_COMMUNICATION:
    case NMT_GS_RESET_CONFIGURATION:
    case NMT_CS_NOT_ACTIVE:
    case NMT_CS_PRE_OPERATIONAL_1 :
    case NMT_CS_PRE_OPERATIONAL_2 :
    case NMT_CS_STOPPED : {
      //frame not accepted
      break;
    }
    case NMT_CS_READY_TO_OPERATE :
    case NMT_CS_OPERATIONAL :  {
      //frame accepted
      break;
    }
    case NMT_CS_BASIC_ETHERNET : {
      //frame accepted, triggers state transition
      break;
    }
  }
}

void handle_PReq(ASync_state &async_state, uintptr_t rx_buffer_address){
//  xscope_int(0, PReq);
  switch(g_nmt_status){
    //NMT_GS_INITIALISATION
    case NMT_GS_INITIALISING:
    case NMT_GS_RESET_APPLICATION:
    case NMT_GS_RESET_COMMUNICATION:
    case NMT_GS_RESET_CONFIGURATION:{
      //frame not accepted
      break;
    }
    case NMT_CS_NOT_ACTIVE :
    case NMT_CS_PRE_OPERATIONAL_1 :{
      //frame not accepted
      break;
    }
    case NMT_CS_PRE_OPERATIONAL_2 :
    case NMT_CS_READY_TO_OPERATE :
    case NMT_CS_OPERATIONAL :{
      //frame accepted
      process_PReq_error_flags(async_state, rx_buffer_address);
      break;
    }
    case NMT_CS_STOPPED : {
      //frame not accepted
      break;
    }
    case NMT_CS_BASIC_ETHERNET : {
      //frame accepted, triggers state transition
      break;
    }
  }
}

void handle_ASnd(ASync_state &async_state, chanend c_eh, uintptr_t rx_buffer_address){
  xscope_int(0, ASnd);

  //counter = DIA_NMTTelegrCount_REC.AsyncRx_U32

  switch(g_nmt_status){

    //NMT_GS_INITIALISATION
    case NMT_GS_INITIALISING:
    case NMT_GS_RESET_APPLICATION:
    case NMT_GS_RESET_COMMUNICATION:
    case NMT_GS_RESET_CONFIGURATION:{
      //frame not accepted

      //i bet we generate a error log entry!
      break;
    }
    case NMT_CS_NOT_ACTIVE : {
      /*frame accepted
       *
       * SDO reception - no frame handling
       * NMT Command - only selected NMT commands accepted, shall cause state transition,
       *                reception requires previous loss of SoA
       * other protocols - no frame handling
       *
       * if(NMT Command == NMTResetNode)          state => NMT_GS_RESET_APPLICATION
       * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
       * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
       * if(NMT Command == NMTSwReset)            state => NMT_GS_INITIALISING
       * if(NMT anything else) log an error
       *
       */
      t_nmt_command valid_commands[4] = {NMTResetNode, NMTResetCommunication,
          NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 4, FALSE, c_eh, async_state);
      break;
    }
    case NMT_CS_PRE_OPERATIONAL_1 : {
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
      t_nmt_command valid_commands[4] = {NMTResetNode, NMTResetCommunication,
          NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 4, TRUE, c_eh, async_state);

      break;
    }
    case NMT_CS_PRE_OPERATIONAL_2 : {
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
      t_nmt_command valid_commands[6] = {NMTStopNode, NMTEnableReadyToOperate,
          NMTResetNode, NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 6, TRUE, c_eh, async_state);
      break;
    }
    case NMT_CS_READY_TO_OPERATE : {
      /*frame accepted
       * SDO reception - frame data interpreted resp. transmitted
       * NMT Command - may cause state transition
       * other protocols - frame data interpreted resp. transmitted
       *
       * if(NMT Command == NMTStartNode) state => NMT_CS_OPERATIONAL
       * if(NMT Command == NMTStopNode) state => NMT_CS_STOPPED
       * if(NMT Command == NMTResetNode) state => NMT_GS_RESET_APPLICATION
       * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
       * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
       * if(NMT Command == NMTSwReset) state => NMT_GS_INITIALISING
       * if(NMT anything else) log an error
       *
       */
      t_nmt_command valid_commands[6] = {NMTStopNode, NMTEnableReadyToOperate,
          NMTResetNode, NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 6, TRUE, c_eh, async_state);
      break;
    }
    case NMT_CS_OPERATIONAL : {
      /*frame accepted
       * SDO reception - frame data interpreted resp. transmitted
       * NMT Command - may cause state transition
       * other protocols - frame data interpreted resp. transmitted
       *
       * if(NMT Command == NMTStopNode) state => NMT_CS_STOPPED
       * if(NMT Command == NMTEnterPreOperational2) state => NMT_CS_PRE_OPERATIONAL_2
       * if(NMT Command == NMTResetNode) state => NMT_GS_RESET_APPLICATION
       * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
       * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
       * if(NMT Command == NMTSwReset) state => NMT_GS_INITIALISING
       * if(NMT anything else) log an error
       *
       */
      t_nmt_command valid_commands[6] = {NMTStopNode, NMTEnableReadyToOperate,
          NMTResetNode, NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 6, TRUE, c_eh, async_state);
      break;
    }
    case NMT_CS_STOPPED : {
      /*frame accepted
       *
       * SDO reception - no frame handling
       * NMT Command - may cause state transition
       * other protocols - no frame handling
       *
       * if(NMT Command == NMTEnterPreOperational2) state => NMT_CS_PRE_OPERATIONAL_2
       * if(NMT Command == NMTResetNode) state => NMT_GS_RESET_APPLICATION
       * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
       * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
       * if(NMT Command == NMTSwReset) state => NMT_GS_INITIALISING
       * if(NMT anything else) log an error
       *
       */
      t_nmt_command valid_commands[5] = {NMTEnterPreOperational2, NMTResetNode,
          NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 5, FALSE, c_eh, async_state);
      break;
    }
    case NMT_CS_BASIC_ETHERNET : {
      /*frame accepted
       *
       * SDO reception - no frame handling
       * NMT Command - only selected NMT commands accepted, shall cause state transition, reception requires previous loss of SoA
       * other protocols - no frame handling
       *
       * if(NMT Command == NMTResetNode) state => NMT_GS_RESET_APPLICATION
       * if(NMT Command == NMTResetCommunication) state => NMT_GS_RESET_COMMUNICATION
       * if(NMT Command == NMTResetConfiguration) state => NMT_GS_RESET_CONFIGURATION
       * if(NMT Command == NMTSwReset) state => NMT_GS_INITIALISING
       * if(NMT anything else) log an error
       *
       */
      t_nmt_command valid_commands[4] = {NMTResetNode,
          NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 4, FALSE, c_eh, async_state);

      g_nmt_status = NMT_CS_PRE_OPERATIONAL_1;
      break;
    }
  }
}

void pl_nmt(streaming chanend c_mii, chanend c_eh, chanend c_dll, chanend c_can_open){

  ASync_state async_state = {0};
  int waiting_on_tx_ack = FALSE;

  timer t;
  unsigned frame_rx_time;
  unsigned basic_ethernet_timeout = 100000000;//CANopen(NMT_CNBasicEthernetTimeout_U32);

  init_async_state(async_state);

  while(1) {

    int nmt_cs_communicating = g_nmt_status >= NMT_CS_NOT_ACTIVE;
    select {
      case c_eh :> int error_code : {
        switch (g_nmt_status) {
          case NMT_CS_PRE_OPERATIONAL_2:
          case NMT_CS_READY_TO_OPERATE:
          case NMT_CS_OPERATIONAL:
          case NMT_CS_STOPPED: {
            g_nmt_status = NMT_CS_PRE_OPERATIONAL_1;
            printstrln("Error: NMT -> NMT_CS_PRE_OPERATIONAL_1");
            break;
          }
          default : {
            break;
          }
        }
        break;
      }
      case c_mii :> unsigned mii_resp : {

        if(!mii_resp) {
          assert(waiting_on_tx_ack);
          waiting_on_tx_ack = FALSE;
        } else {
          uintptr_t rx_buffer_address = mii_resp;

          //TODO find out if I need this
          c_mii :> unsigned frame_presentation_time;

          //filter out
          if(reject_mac(rx_buffer_address)){
            c_mii<:0;
            break;
          }

          if(is_type_powerlink(rx_buffer_address)){
            Message_Type_ID mt_id = get_powerlink_type(rx_buffer_address);

            if(mt_id != Asynchronous_Send)
              t :> frame_rx_time;

            if(g_nmt_status == NMT_CS_BASIC_ETHERNET) {
             // printstrln("POWERLINK frame detected: NMT -> NMT_CS_PRE_OPERATIONAL_1");
              g_nmt_status = NMT_CS_PRE_OPERATIONAL_1;
            }

           // print(rx_buffer_address);

            switch (mt_id) {
            case Start_of_Cycle:{
              handle_SoC(async_state);
              break;
            }
            case Start_of_Asynchronous:{
              handle_SoA(rx_buffer_address, waiting_on_tx_ack, async_state, c_mii, c_eh);
              break;
            }
            case Asynchronous_Send:{
           //   handle_ASnd(async_state, c_eh, rx_buffer_address);
              break;
            }
            case PollRequest:{
             // handle_PReq(async_state, rx_buffer_address);
              break;
            }
            case PollResponse:{
              //handle_PRes();
              break;
            }
            }
          }
          c_mii<:0;
        }
        break;
      }

      case nmt_cs_communicating & (g_nmt_status != NMT_CS_BASIC_ETHERNET) =>
          t when timerafter (frame_rx_time + basic_ethernet_timeout) :> unsigned : {
        //no frames recieved for a while
        g_nmt_status = NMT_CS_BASIC_ETHERNET;
        printstrln("Timeout: NMT -> NMT_CS_BASIC_ETHERNET");
        break;
      }
    !nmt_cs_communicating => default:
     // init_nmt_state(async_state, frame_rx_time, t, nmt_cs_communicating);
      break;
    }


  }
}
