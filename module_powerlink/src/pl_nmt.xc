#include <xs1.h>
#include "pl_nmt.h"
#include "pl_defines.h"
#include "thread_id.h"
#include "pl_error_defines.h"
#include <print.h>
#include "debug.h"
#include <xscope.h>
#include "async_sdo_seq_layer.h"
#include "global_state.h"


int inform_seq_layer(uintptr_t tx_frame);

#define SHOW_STATE_TRANSITIONS 0
#define FRAME_PROCESS_ID 0


void request_status_response_from_eh(chanend c_eh, ASync_state &async_state){
  c_eh <: request_status_response;
  c_eh :> async_state.SoA_response_p;
  c_eh :> async_state.SoA_response_size;
}

static void init_nmt_state(ASync_state &async_state, unsigned &frame_rx_time, timer t){

  switch(get_nmt_status()) {
  case NMT_GS_INITIALISING: {
    /*
    This is the first sub-state the POWERLINK node shall enter after Power On (NMT_GT1),
    hardware resp. software Reset (NMT_GT2) or the reception of an NMTSwReset (NMT_GT8)
    command. After finishing the basic node initialisation, the POWERLINK node shall
    autonomously enter the sub-state NMT_GS_RESET_APPLICATION (NMT_GT10).
    */
    //TODO
    //send a reset to the error signalling thread

    //At startup (NMT_GT1, NMT_GT2 or NMT_GT8) the CN shall reset the Error Signaling and set EC=1.
    async_state.EC = 1;
    set_nmt_status(NMT_GS_RESET_APPLICATION);
    if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_APPLICATION");
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
    //init all the power on values

    set_nmt_status(NMT_GS_RESET_COMMUNICATION);
    if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_COMMUNICATION");

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

    //TODO
    //init all the power on values
    sdo_seq_init_layer();

    set_nmt_status(NMT_GS_RESET_CONFIGURATION);
    if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_CONFIGURATION");
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

    set_nmt_status(NMT_CS_NOT_ACTIVE);
    if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_NOT_ACTIVE");

    //set the time that NMT_CS_NOT_ACTIVE began
    t :> frame_rx_time;
    break;
  }
  case NMT_CS_PRE_OPERATIONAL_2: {


    set_nmt_status(NMT_CS_READY_TO_OPERATE);
    if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_READY_TO_OPERATE");
    break;
  }
  }
}

void process_soa(uintptr_t rx_buffer_address, t_frame_SoA &latest_SoA);
void process_ASnd_or_nonpowerlink_frame(uintptr_t rx_buffer_address,
    chanend c_eh, ASync_state &async_state, chanend c_can_open);
uint16_t get_ethertype(uintptr_t pointer);
message_type_id_t get_powerlink_type(uintptr_t pointer);
int is_type_powerlink(uintptr_t pointer);
int reject_pl_dst(uintptr_t rx_buffer_address, unsigned node_id);
int preq_is_for_me(uintptr_t);

static void init_async_state(ASync_state &async_state){

  set_nmt_status(NMT_GS_INITIALISING);
  if(SHOW_STATE_TRANSITIONS) printstrln("NMT = NMT_GS_INITIALISING (init)");
  /*
   * At startup (NMT_GT1, NMT_GT2 or NMT_GT8) the CN shall reset the Error Signaling and set EC=1.
   * The CN shall not change the EC flag before at least 1 valid frame with ER=1 was received.
   */
  //TODO move this into initialisation
  async_state.EC = 1;
}

void report_error(chanend c_eh, unsigned error){
  c_eh <: error; //FIXME send the correct error to the eh
}

static void process_async_frame(
    uintptr_t rx_buffer_address,
    nmt_command_id_t valid_commands[],
    unsigned valid_cmd_count,
    int frame_handling,
    chanend c_eh,
    ASync_state &async_state, chanend c_can_open){

  uintptr_t nmt_cmd;
  if(is_nmt_command(rx_buffer_address, nmt_cmd)){
    if(is_valid_nmt_command(nmt_cmd, valid_commands, valid_cmd_count)){
      handle_nmt_command_p(nmt_cmd, async_state, c_eh);

    } else {
     // report_bad_nmt_command(c_eh);
    }
  } else {
    if(frame_handling)
      process_ASnd_or_nonpowerlink_frame(rx_buffer_address, c_eh, async_state, c_can_open);
      //handle_known_asnd(rx_buffer_address);
  }
}

void handle_SoC(ASync_state &async_state){
  xscope_int(FRAME_PROCESS_ID, SoC);
  switch(get_nmt_status()){
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
      set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_1");

      break;
    }
    case NMT_CS_PRE_OPERATIONAL_1 : {
      //frame accepted, triggers state transition

      /* (NMT_CT4)
       * If the node receives a SoC frame in NMT_CS_PRE_OPERATIONAL_1,
       * the node shall change over to NMT_CS_PRE_OPERATIONAL_2.
       */
      set_nmt_status(NMT_CS_PRE_OPERATIONAL_2);
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_2");
      //async_state.RD = 1;
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
      set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_1");
      break;
    }
  }
}

/*
 * This updated the PR and RS flags. It only needs to be run after something might have changed them, i.e.
 *  - SDO
 *  -
 */
static void update_PR_and_RS(ASync_state &a){
  for(int i=7;i>=0;i--){
    if(a.PR_queue_fill_level[i]>0){
      a.PR = i;
      if(a.PR_queue_fill_level[i]>6){
        a.RS = 7;
        return;
      } else {
        a.RS = a.PR_queue_fill_level[i];
        return;
      }
    }
  }
  a.PR = 0;
  a.RS = 0;
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
static void send_asnd(uintptr_t SoA_response_p, unsigned SoA_response_size, streaming chanend c_mii){
   //DIA_NMTTelegrCount_REC.AsyncTx_U32 ++;

  c_mii <: make_tx_req_p(SoA_response_p, SoA_response_size, 1);

}

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
void set_PR_and_RS_and_NMT_status(ASync_state &a, uintptr_t SoA_response_p){
  uint8_t flags = (a.RS) + (a.PR << 3);
  asm("st8 %0, %1[%2]"::"r"(flags),"r"(SoA_response_p), "r"(19));
  asm("st8 %0, %1[%2]"::"r"(get_nmt_status()),"r"(SoA_response_p), "r"(20));
}

static void set_EN_and_EC(ASync_state &a, uintptr_t SoA_response_p){
  uint8_t flags = (a.EN << 4) + (a.EC << 3);
  asm("st8 %0, %1[%2]"::"r"(flags),"r"(SoA_response_p), "r"(18));
}

#define IDENT_RESPONSE_BUFFER_SIZE_BYTES (176)

#if 1

#define NODE_ID 1

#define MTU (C_DLL_MAX_ASYNC_MTU - C_DLL_MIN_ASYNC_MTU)
#define MTU_hi ((MTU>>8)&0xff)
#define MTU_low (MTU&0xff)

#define PIS C_DLL_ISOCHR_MAX_PAYL
#define PIS_hi ((PIS>>8)&0xff)
#define PIS_low (PIS&0xff)

#define POS C_DLL_ISOCHR_MAX_PAYL
#define POS_hi ((POS>>8)&0xff)
#define POS_low (POS&0xff)

static uint8_t ident_response_buf[IDENT_RESPONSE_BUFFER_SIZE_BYTES] = {
    0x01,   0x11,   0x1E,   0x00,   0x00,   0x04, //dst_mac
    MAC_0,  MAC_1,  MAC_2,  MAC_3,  MAC_4,  MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_BROADCAST,
    NODE_ID,
    IDENT_RESPONSE,
    0x00,//flags(reserved)
    0x00,//PR + RS
    0x00,//NMTStatus
    0x00,//Reserved
    0x20,//EPLVersion
    0x00,//Reserved

    0x45, 0x03, 0x00, 0x00,//FEATURE_FLAG_OCTET_0,FEATURE_FLAG_OCTET_1,0x00,0x00,//FeatureFlags

    0x2c, 0x01,//MTU_low, MTU_hi,//MTU
    PIS_low,PIS_hi,//PollInSize
    POS_low,POS_hi,//PollOutSize
    0x50,0xc3,0x00,0x00,//ResponseTime

    0x00,0x00,//Reserved

    0x91,0x01,0x00,0x00,//DeviceType
    0x00,0x00,0x00,0x00,//VendorID
    0x01,0x00,0x00,0x00,//ProductCode
    0x00,0x00,0x00,0x00,//RevisionNumber
    0x01,0x00,0x00,0x00,//SerialNumber
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//VendorSpecificExtension1
    0x00,0x00,0x00,0x00,//VerifyConfigurationDate
    0x00,0x00,0x00,0x00,//VerifyConfigurationTime
    0x01,0x00,0x00,0x00,//ApplicationSwDate
    0x01,0x00,0x00,0x00,//ApplicationSwTime
    IP_3,IP_2,IP_1,IP_0,//IPAddress
    SN_3,SN_2,SN_1,SN_0,//SubnetMask
    GW_3,GW_2,GW_1,GW_0,//DefaultGateway

    0x6D,0x61,0x6B,0x65,0x20,0x61,0x6E,0x64,
    0x72,0x65,0x77,0x20,0x61,0x20,0x63,0x68,
    0x65,0x65,0x73,0x65,0x20,0x73,0x61,0x6E,
    0x64,0x77,0x69,0x63,0x68,0x00,0x00,0x00, //HostName

    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//VendorSpecificExtension2
};
#else
static uint8_t ident_response_buf[IDENT_RESPONSE_BUFFER_SIZE_BYTES] = {
    0x01,   0x11,   0x1E,   0x00,   0x00,   0x04, //dst_mac
    MAC_0,  MAC_1,  MAC_2,  MAC_3,  MAC_4,  MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_BROADCAST,
    0x00,//NodeID
    IDENT_RESPONSE,
    0x00,//flags(reserved)
    0x00,//PR + RS
    0x00,//NMTStatus
    0x00,//Reserved
    0x20,//EPLVersion
    0x00,//Reserved
};
#endif
/*
 * This should be called when any of the fields that make up and IdentResponse
 * change.
 */

static void load_const_into_ident_response(chanend c_co){
 /*
  * Object 1F82h: NMT_FeatureFlags_U32           const
  * C_DLL_MIN_ASYNC_MTU - C_DLL_MAX_ASYNC_MTU    const
  * NMT_CycleTiming_REC.PResMaxLatency_U32       const
  * NMT_DeviceType_U32                           const
  * NMT_IdentityObject_REC.VendorId_U32          const
  * NMT_IdentityObject_REC.RevisionNo_U32        const
  * NMT_IdentityObject_REC.SerialNo_U32          const
  */
}
static void load_ro_into_ident_response(chanend c_co){
  /*
   * NWL_IpAddrTable_Xh_REC.Addr_IPAD             ro
   * NWL_IpAddrTable_Xh_REC.NetMask_IPAD          ro
   * NWL_IpAddrTable_Xh_REC.DefGateway_IPAD       ro
   * NMT_EPLNodeID_REC.NodeID_U8                  ro
   *
   */
}
static void load_rw_into_ident_response(chanend c_co){
  /*
   * NMT_CycleTiming_REC.PReqActPayloadLimit_U16  rw
   * NMT_CycleTiming_REC.PResActPayloadLimit_U16  rw
   * CFM_VerifyConfiguration_REC.ConfDate_U32     rw
   * CFM_VerifyConfiguration_REC.ConfTime_U32     rw
   * PDL_LocVerApplSw_REC.ApplSwDate_U32          rw
   * PDL_LocVerApplSw_REC.ApplSwTime_U32          rw
   * NMT_HostName_VSTR                            rw
   */
}


void handle_SoA(uintptr_t rx_buffer, int &waiting_on_tx_ack,
    ASync_state &async_state, streaming chanend c_mii, chanend c_eh){

  switch(get_nmt_status()){
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
      set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_1");
        break;
    }
    case NMT_CS_PRE_OPERATIONAL_1 :
    case NMT_CS_PRE_OPERATIONAL_2 :
    case NMT_CS_READY_TO_OPERATE :
    case NMT_CS_OPERATIONAL :
    case NMT_CS_STOPPED :
    {
      unsigned target;
      asm("ld8u %0, %1[%2]":"=r"(target):"r"(rx_buffer), "r"(21));

      if(target == 1){

        asnd_serviceids_t type;
        asm("ld8u %0, %1[%2]":"=r"(type):"r"(rx_buffer), "r"(20));

        switch(type){
        case NO_SERVICE:{
          //do nothing
          break;
        }
        case IDENT_REQUEST:{
          uintptr_t SoA_response_p;
          asm("mov %0, %1":"=r"(SoA_response_p):"r"(ident_response_buf));
          set_PR_and_RS_and_NMT_status(async_state, SoA_response_p);
          send_asnd(SoA_response_p, IDENT_RESPONSE_BUFFER_SIZE_BYTES, c_mii);
          assert(waiting_on_tx_ack == FALSE);
          waiting_on_tx_ack = TRUE;
          //TODO error flags
          break;
        }
        case STATUS_REQUEST:{
          //fill in
          unsigned SoA_response_size;
          uintptr_t SoA_response_p;

          //with the status response
          c_eh <: request_status_response;
          c_eh :> SoA_response_p;
          c_eh :> SoA_response_size;

          //set EN and EC
          set_EN_and_EC(async_state, SoA_response_p);

          set_PR_and_RS_and_NMT_status(async_state, SoA_response_p);
          send_asnd(SoA_response_p, SoA_response_size, c_mii);
          assert(waiting_on_tx_ack == FALSE);
          waiting_on_tx_ack = TRUE;
          //TODO error flags
          break;
        }
        case NMT_REQUEST_INVITE:{
          //TODO
          break;
        }
        case UNSPECIFIED_INVITE:{
          //TODO we must ba able to deal with an unspecified invite that came in error, i.e. we have nothing to send

          //this means that we asked for an invite which should be in the PR register
          unsigned PR = async_state.PR;


          if(async_state.RS){

            send_asnd(async_state.PR_queues_p[PR][0], async_state.PR_queues_size[PR][0], c_mii);

            if(inform_seq_layer(async_state.PR_queues_p[PR][0])){
              //TODO set a timeout
            }

            //TODO handle sdo history pointers
            assert(waiting_on_tx_ack == FALSE);
            waiting_on_tx_ack = TRUE;

            assert(PR<8);
            assert(async_state.PR_queue_fill_level[PR]<8);

            for(unsigned i=0;i< async_state.PR_queue_fill_level[PR]-1;i++){
              async_state.PR_queues_p[PR][i] = async_state.PR_queues_p[PR][i+1];
              async_state.PR_queues_size[PR][i] = async_state.PR_queues_size[PR][i+1];
            }

            async_state.PR_queue_fill_level[PR]--;
          } else {
            //UnspecifiedInvite when I have nothing to send!
            printstrln("UnspecifiedInvite when I have nothing to send!");
          }
         // printf("before RS: %d\n", async_state.RS);
          //now update the PR and RS
          update_PR_and_RS(async_state);
          //TODO error flags

          break;
        }
        case MANUF_SVC_IDS:{
          break;
        }
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
      set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_1");
    //  process_SoA(rx_buffer_address, async_state);
      break;
    }
  }
}

void handle_PRes(){
  xscope_int(0, PRes);
  switch(get_nmt_status()){
    //NMT_GS_INITIALISATION
    case NMT_GS_INITIALISING:
    case NMT_GS_RESET_APPLICATION:
    case NMT_GS_RESET_COMMUNICATION:
    case NMT_GS_RESET_CONFIGURATION:
    case NMT_CS_NOT_ACTIVE:
    case NMT_CS_PRE_OPERATIONAL_1 :
    case NMT_CS_STOPPED : {
      //frame not accepted
      break;
    }
    case NMT_CS_PRE_OPERATIONAL_2 :
    case NMT_CS_READY_TO_OPERATE :
    case NMT_CS_OPERATIONAL :  {
      //frame accepted
      break;
    }
    case NMT_CS_BASIC_ETHERNET : {
      //frame accepted, triggers state transition
      set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_1");
      break;
    }
  }
}

{uintptr_t, unsigned} static fetch_pres_from_dll(chanend c_dll){
  unsigned size;
  uintptr_t p;
  c_dll <: 0;
  c_dll :> p;
  c_dll :> size;
  return {p, size};
}

static void send_pres(uintptr_t pres_pointer, unsigned pres_size, streaming chanend c_mii){
  c_mii <: make_tx_req_p(pres_pointer, pres_size, 1);
}

static void set_pres_flags(uintptr_t pres, ASync_state &a){
  uint8_t node_id = 1; //FIXME get this from can open

  uint8_t signals = (a.EN << 4) + (a.RD << 0) + (a.MS << 5);
  uint8_t flags = (a.RS<<0) + (a.PR << 3);
  unsigned data = 0;
  data += node_id;
  data += (get_nmt_status()<<8);
  data += (signals<<16);
  data += (flags<<24);
  asm("stw %0, %1[%2]"::"r"(data),"r"(pres), "r"(4));
}

void handle_PReq(ASync_state &async_state, uintptr_t rx_buffer_address,
    streaming chanend c_mii, int &waiting_on_tx_ack, chanend c_dll){
  switch(get_nmt_status()){
    //NMT_GS_INITIALISATION
    case NMT_GS_INITIALISING:
    case NMT_GS_RESET_APPLICATION:
    case NMT_GS_RESET_COMMUNICATION:
    case NMT_GS_RESET_CONFIGURATION:
    case NMT_CS_NOT_ACTIVE :
    case NMT_CS_PRE_OPERATIONAL_1 :
    case NMT_CS_STOPPED :{
      //frame not accepted
      break;
    }
    case NMT_CS_PRE_OPERATIONAL_2 :
    case NMT_CS_READY_TO_OPERATE :
    case NMT_CS_OPERATIONAL :{
      //frame accepted
      if(preq_is_for_me(rx_buffer_address)){

        //we could send a request on a timeout

        uintptr_t pres_pointer;
        unsigned pres_size;

        {pres_pointer, pres_size} = fetch_pres_from_dll(c_dll);

        //TODO this can be moved to any code that changes the PR and RS flags
        set_pres_flags(pres_pointer, async_state);

        //now begin updating the next response (powerlink is ok with this
        //TODO move this likewise
        process_PReq_error_flags(async_state, rx_buffer_address);

        //do this as soon as possible to reduce latency
        send_pres(pres_pointer, pres_size, c_mii);

        waiting_on_tx_ack = TRUE;
      }
      break;
    }
    case NMT_CS_BASIC_ETHERNET : {
      //frame accepted, triggers state transition
      set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_1");
      break;
    }
  }
}

void handle_ASnd(ASync_state &async_state, chanend c_eh, uintptr_t rx_buffer_address, chanend c_can_open){
  xscope_int(0, ASnd);

  //counter = DIA_NMTTelegrCount_REC.AsyncRx_U32

  switch(get_nmt_status()){

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
      nmt_command_id_t valid_commands[4] = {NMTResetNode, NMTResetCommunication,
          NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 4, FALSE, c_eh, async_state, c_can_open);
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
      nmt_command_id_t valid_commands[4] = {NMTResetNode, NMTResetCommunication,
          NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 4, TRUE, c_eh, async_state, c_can_open);

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
      nmt_command_id_t valid_commands[6] = {NMTStopNode, NMTEnableReadyToOperate,
          NMTResetNode, NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 6, TRUE, c_eh, async_state, c_can_open);
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
      nmt_command_id_t valid_commands[7] = {NMTStartNode, NMTStopNode, NMTEnableReadyToOperate,
          NMTResetNode, NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 7, TRUE, c_eh, async_state, c_can_open);
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
      nmt_command_id_t valid_commands[6] = {NMTStopNode, NMTEnableReadyToOperate,
          NMTResetNode, NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 6, TRUE, c_eh, async_state, c_can_open);
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
      nmt_command_id_t valid_commands[5] = {NMTEnterPreOperational2, NMTResetNode,
          NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 5, FALSE, c_eh, async_state, c_can_open);
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
      nmt_command_id_t valid_commands[4] = {NMTResetNode,
          NMTResetCommunication, NMTResetConfiguration, NMTSwReset};
      process_async_frame(rx_buffer_address, valid_commands, 4, FALSE, c_eh, async_state, c_can_open);

      set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
      break;
    }
  }
  update_PR_and_RS(async_state);
}



#include "can_open_interface.h"
#include "object_dictionary_defines.h"

//NMT main thread loop
void pl_nmt(streaming chanend c_mii, chanend c_eh, chanend c_dll, chanend c_can_open){

  ASync_state async_state = {0};
  int waiting_on_tx_ack = FALSE;

  timer sdo_ack_timer;
  unsigned sdo_ack_timeout = co_read_UNSIGNED32(c_can_open, SDO_SequLayerTimeout_U32, 0) * 100000;
  int sdo_ack_waiting = FALSE;

  timer basic_ethernet_timer;
  unsigned basic_ethernet_timeout = co_read_UNSIGNED32(c_can_open, NMT_CNBasicEthernetTimeout_U32, 0) * 100;
  unsigned frame_rx_time;

  init_async_state(async_state);

  //co_write_UNSIGNED32(NMT_CurrNMTState_U8

  while(1) {

    int nmt_cs_communicating = (get_nmt_status() & 12) == 12;
    select {
      case c_eh :> int error_code : {
        switch (get_nmt_status()) {
          case NMT_CS_PRE_OPERATIONAL_2:
          case NMT_CS_READY_TO_OPERATE:
          case NMT_CS_OPERATIONAL:
          case NMT_CS_STOPPED: {
            set_nmt_status(NMT_CS_PRE_OPERATIONAL_1);
            if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_PRE_OPERATIONAL_1");
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

          if(is_type_powerlink(rx_buffer_address)){

             //filter out
            /*
            if(reject_mac(rx_buffer_address)){
              c_mii<:0;
              break;
            }
            */
            message_type_id_t mt_id = get_powerlink_type(rx_buffer_address);

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
              handle_ASnd(async_state, c_eh, rx_buffer_address, c_can_open);
              break;
            }
            case PollRequest:{
             handle_PReq(async_state, rx_buffer_address, c_mii, waiting_on_tx_ack, c_dll);
              break;
            }
            case PollResponse:{
              //This is handled by the DLL
              break;
            }
            }
            //this is after the handling to improve performance
            if(mt_id != Asynchronous_Send)
              basic_ethernet_timer :> frame_rx_time;
          } else {
            //TODO
          }
          c_mii<:0;
        }
        break;
      }

      case sdo_ack_waiting => sdo_ack_timer when timerafter(sdo_ack_timeout):> int :{
        sdo_ack_waiting = FALSE;
        break;
      }
      case basic_ethernet_timeout & nmt_cs_communicating & (get_nmt_status() != NMT_CS_BASIC_ETHERNET) =>
          basic_ethernet_timer when timerafter (frame_rx_time + basic_ethernet_timeout) :> unsigned : {
        set_nmt_status(NMT_CS_BASIC_ETHERNET);
        if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_BASIC_ETHERNET");
        break;
      }
    !nmt_cs_communicating => default:
      init_nmt_state(async_state, frame_rx_time, basic_ethernet_timer);
      break;
    }

    if(get_nmt_status() == NMT_CS_OPERATIONAL)
      async_state.RD = 1; //FIXME
  }
}
