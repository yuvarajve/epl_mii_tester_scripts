#include <stdlib.h>

#include "pl_nmt.h"
#include "pl_defines.h"
#include "pl_general_purpose_constants.h"
#include "frame.h"
#include "debug.h"
#include "device_description.h"
#include <xscope.h>
#define NODE_ID 1

#include <assert.h>
static uint16_t htons(uint16_t s){
  return (__builtin_bswap32(s)>>16)&0xffff;
}
#define IDENT_RESPONSE_BUFFER_SIZE_BYTES (176)
/*
 * The IdentResponse service may be initiated by a CN via the NMT
 * Request mechanism (see 7.3.6). The NMTRequestedCommandID field
 * of the NMT requesting ASnd frame shall be set to IDENT_REQUEST.
 */
#define MTU (C_DLL_MAX_ASYNC_MTU - C_DLL_MIN_ASYNC_MTU)
#define MTU_hi ((MTU>>8)&0xff)
#define MTU_low (MTU&0xff)

#define PIS C_DLL_ISOCHR_MAX_PAYL
#define PIS_hi ((PIS>>8)&0xff)
#define PIS_low (PIS&0xff)

#define POS C_DLL_ISOCHR_MAX_PAYL
#define POS_hi ((POS>>8)&0xff)
#define POS_low (POS&0xff)


extern t_nmt_state g_nmt_status;

static uint8_t ident_response_buf[IDENT_RESPONSE_BUFFER_SIZE_BYTES] = {
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
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
    FEATURE_FLAG_OCTET_0,FEATURE_FLAG_OCTET_1,0x00,0x00,//FeatureFlags
    MTU_low, MTU_hi,//MTU
    PIS_low,PIS_hi,//PollInSize
    POS_low,POS_hi,//PollOutSize
    0x00,0x00,0x00,0x00,//ResponseTime
    0x00,0x00,//Reserved
    0x91,0x01,0x00,0x00,//DeviceType      //FIXME this comes from canopen headers
    0x00,0x00,0x00,0x00,//VendorID
    0x01,0x00,0x00,0x00,//ProductCode
    0x00,0x00,0x00,0x00,//RevisionNumber
    0x01,0x00,0x00,0x00,//SerialNumber
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//VendorSpecificExtension1
    0x00,0x00,0x00,0x00,//VerifyConfigurationDate
    0x00,0x00,0x00,0x00,//VerifyConfigurationTime
    0x01,0x00,0x00,0x00,//ApplicationSwDate
    0x01,0x00,0x00,0x00,//ApplicationSwTime
    IP_0,IP_1,IP_2,IP_3,//IPAddress
    SN_0,SN_1,SN_2,SN_3,//SubnetMask
    GW_0,GW_1,GW_2,GW_3,//DefaultGateway

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

static void build_ident_response(ASync_state * async_state){
  //fill in the details PR and RS are filled in later
  ident_response_buf[20] = g_nmt_status;
  //ident_response_buf[87] = async_state->node_id;  //check this
  ident_response_buf[87] = 1;  //check this
}

static uint8_t * get_ident_response_pointer(){
  return ident_response_buf;
}

extern int error_signalling_data_present;


void process_PReq_error_flags(ASync_state * async_state, uintptr_t rx_buffer_address){
  frame * frm = (frame*)rx_buffer_address;

    SoA_t * soa = &(frm->pl_frame.soa);

}

void process_SoA(uintptr_t rx_buffer_address, ASync_state * async_state, chanend c_eh){
  frame * frm = (frame*)rx_buffer_address;

  SoA_t * soa = &(frm->pl_frame.soa);

  //if(soa->RequestedServiceTarget == async_state->node_id){
  if(soa->RequestedServiceTarget == 1){

    //first we must deal with the EA and ER flags
    /*
     * Initialisation of the Error Signaling
     * ER: When a CN receives the value 1b it shall reset its EN bit to 0b and clear the Emergency Queue.
     *
     */
    if(soa->ER){
      printf("seen_a_valid_frame_with_ER_true\n");
      async_state->seen_a_valid_frame_with_ER_true = TRUE;
      async_state->EN = 0;
      //TODO clear the emergency queue
    }

    if(async_state->seen_a_valid_frame_with_ER_true){
      async_state->EC = soa->ER;
    }

    /*
     * Isochronous CNs shall evaluate the EA bit of the SoA(...) only in NMT_CS_PRE_OPERATIONAL_1.
     */

    if(async_state->EA == async_state->EN && error_signalling_data_present){
      async_state->EN = 1 - async_state->EN;
    }

    //then its going to be my turn to send
    switch(soa->RequestedServiceID){
      case NO_SERVICE: {
        /*
         * Shall be used if the asynchronous slot is not assigned to any node.
         * RequestedServiceTarget shall be C_ADR_INVALID.
         */
        return;
      }
      case IDENT_REQUEST: {
        /*
         * Shall be used to identify inactive CNs and/or to query the identification
         * data of a CN. The addressed CN shall answer immediately after the reception
         * of the SoA with the node specific IdentRequest frame.The IdentRequest frame
         * is based on the ASnd frame.
         */
        build_ident_response(async_state); //make sure this is like lightnening
        async_state->SoA_response_p = (uintptr_t)get_ident_response_pointer();
        async_state->SoA_response_size = IDENT_RESPONSE_BUFFER_SIZE_BYTES;
        async_state->asnd_invite_response = IDENT_RESPONSE;
        return;
      }
      case STATUS_REQUEST: {
        /*
         * Shall be used to request the current status and detailed error information
         * of a node. Async-only CNs shall be cyclically queried by StatusRequest to
         * supervise their status and to query their requests for the asynchronous slot.
         * The addressed node shall answer immediately after the reception of the SoA,
         * with the node specific StatusRequest frame. The StatusRequest frame is based
         * on the ASnd frame.
         */
        //TODO the error handling thread needs to set a flag to say that there is data waiting

        //if(soa->EA == async_state->EN){
          //printf("ea:%d en:%d ec:%d er%d\n", async_state->EA, async_state->EN, async_state->EC, async_state->ER);
           //When the rxd EA is equal to my EN then we will not send a status response
          //return;
         //}

        //if it is not equal then we will send a status response

        request_status_response_from_eh(c_eh, async_state);
        async_state->asnd_invite_response = STATUS_RESPONSE;
        return;
      }
      case NMT_REQUEST_INVITE: {
        /*
         * Shall be used to assign the asynchronous slot to a node that has indicated
         * a pending NMTCommand / NMTRequest by a
         *
         *  Request to Send (RS bit of PRes, StatusResponse or IdentResponse)
         *
         * with the priority level PRIO_NMT_REQUEST. The addressed node shall answer
         * immediately after the reception of the SoA with the NMTCommand / NMTRequest
         * frame. The NMTCommand and NMTRequest frames are based on the ASnd frame.
         */
        async_state->SoA_response_size = async_state->PR_queues_size[7][0];
        async_state->SoA_response_p = async_state->PR_queues_p[7][0];
        for(unsigned i=0;i< async_state->PR_queues[7]-1;i++){
          async_state->PR_queues_p[7][i] = async_state->PR_queues_p[7][i+1];
          async_state->PR_queues_size[7][i] = async_state->PR_queues_size[7][i+1];
        }
        async_state->PR_queues[7]--;
        return;
      }
      case UNSPECIFIED_INVITE: {
        /*
         * Shall be used to assign the asynchronous slot to a node that has indicated
         * a pending transmit request by a
         *
         *  Request to Send (RS bit of PRes, StatusResponse or IdentResponse)
         *
         * with the priority level PRIO_GENERIC_REQUEST. The addressed node shall answer
         * immediately after the reception of the SoA, with any kind of a POWERLINK ASnd
         * or a Legacy Ethernet frame.
         */
        const unsigned queue = 3;
        /*
         * QUESTION: does an unspecified invite always return a reply from queue 3?
         */
        async_state->SoA_response_size = async_state->PR_queues_size[queue][0];
        async_state->SoA_response_p = async_state->PR_queues_p[queue][0];
        for(unsigned i=0;i< async_state->PR_queues[queue]-1;i++){
          async_state->PR_queues_p[queue][i] = async_state->PR_queues_p[queue][i+1];
          async_state->PR_queues_size[queue][i] = async_state->PR_queues_size[queue][i+1];
        }
        async_state->PR_queues[queue]--;
        return;
      }
    }

    if(soa->RequestedServiceID > NMT_REQUEST_INVITE && soa->RequestedServiceID < MANUF_SVC_IDS){
      //reserved
      assert(0 && "Got a reserved RequestedServiceID");
    } else {
      //manufacture specific
      assert(0 && "Got a manufacture specific RequestedServiceID");
    }
  } else {
    async_state->asnd_invite_response = 0;
  }
}
/*
 * Note, a cn that wants an UNSPECIFIED_INVITE sets RS = 1 and PR to be PRIO_GENERIC_REQUEST.
 * A cn that wants a NMT_REQUEST_INVITE sets RS = 1 and PR to be PRIO_NMT_REQUEST
 */


//FIXME The NMTRequest ASnd frame is unicast to the MN.
static uint8_t nmt_req_buf[1522] = {
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_MN_DEF_NODE_ID,
    NODE_ID,
    NMT_REQUEST,
    //NMTRequestedCommandID
    //NMTRequestedCommandTarget
    //NMTRequestedCommandData
};


static void build_nmt_request_frame(uint8_t CommandID, uint8_t CommandTarget){

}
#include "async_sdo_seq_layer.h"
#include "async_sdo_cmd_layer.h"


sdo_seq_state seq_state;
sdo_cmd_state cmd_state;

static void handle_SDO(SDO_t * frm,  ASync_state * async_state, chanend c_eh){
  /*
   * The sdo is made up of two fields:
   *  Sequence Layer
   *  Command Layer
   */
  Sequence_Layer_Protocol_t *s = &(frm->slp);
  Command_Layer_Protocol_t *c = &(frm->clp);
  //sdo_seq_state * seq_state = &(async_state->seq_state);
  //sdo_cmd_state * cmd_state = &(async_state->cmd_state);

  sdo_seq_recieved(s, &seq_state, c, &cmd_state);
}

static void handle_SDO_via_UDP(SDO_via_UDP_t * frm,  ASync_state * async_state, chanend c_eh){
  if(frm->MessageType != Asynchronous_Send){
    assert(0 && "SDO via UDP not Asynchronous_Send!");
    return;
  }
  if(frm->ServiceID != SDO){
    assert(0 && "SDO via UDP not SDO!");
    return;
  }
  handle_SDO(&(frm->sdo), async_state, c_eh);
}

#define UDP 0x11

static void handle_IPv4(IPv4_t * frm,  ASync_state * async_state, chanend c_eh){

  //TODO any IPv4 stuff!
  if(frm->Dst_IP[0] != IP_0 ||  //192
      frm->Dst_IP[1] != IP_1 || //168
      frm->Dst_IP[2] != IP_2 || //100
      frm->Dst_IP[3] != async_state->node_id){
    return;
  }

  switch(frm->Protocol){
    case UDP:{
      //TODO test the checksum
      uint16_t crc = 0xffff;

      if(crc == 0xffff){
        if(htons(frm->udp.Destination_Port) == C_SDO_EPL_PORT){
          handle_SDO_via_UDP(&(frm->udp.sdo), async_state, c_eh);
        } else {
          assert(0 && "not the C_SDO_EPL_PORT for udp");
        }
      }
    }
    default:{

      assert(0 && "we don't support this ip protocol");
    }
  }
}



/*
 * We make the assumption that extended commands are valid and have been converted to
 * plain commands, i.e. if not valid for this node then this isn't called.
 */
static void handle_plain_nmt_command(uint8_t cmd,  ASync_state * async_state, chanend c_eh){
  int nmt_cs_communicating = async_state->NMTStatus >= NMT_CS_NOT_ACTIVE;

  switch (cmd){
  case NMTSwReset:{
    if(nmt_cs_communicating){
      async_state->NMTStatus = NMT_GS_INITIALISING;
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTResetNode :{
    if(nmt_cs_communicating){
      async_state->NMTStatus = NMT_GS_RESET_APPLICATION;
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTResetCommunication :{
    if(nmt_cs_communicating){
      async_state->NMTStatus = NMT_GS_RESET_COMMUNICATION;
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTResetConfiguration :{
    if(nmt_cs_communicating){
      async_state->NMTStatus = NMT_GS_RESET_CONFIGURATION;
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTEnableReadyToOperate :{
    //According to Tab. 125 this is only effecitve in NMT_CS_PRE_OPERATIONAL_2
    if(async_state->NMTStatus == NMT_CS_PRE_OPERATIONAL_2 ){
      //TODO
      //if config is complete - what's this???
      int config_is_complete = 0;
      if(config_is_complete){
        async_state->NMTStatus = NMT_CS_READY_TO_OPERATE;
        async_state->RD = 0;
      }
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTStartNode :{
   //According to Tab. 125 this is only effecitve in NMT_CS_READY_TO_OPERATE
    if(async_state->NMTStatus == NMT_CS_READY_TO_OPERATE){
      //TODO
      //if config is valid - what's this???
      int config_is_valid = 0;
      if(config_is_valid)
        async_state->NMTStatus = NMT_CS_OPERATIONAL;
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTStopNode : {
    //According to Tab. 125 this is only effecitve in NMT_CS_PRE_OPERATIONAL_2,
    //NMT_CS_READY_TO_OPERATE or NMT_CS_OPERATIONAL
    if(async_state->NMTStatus == NMT_CS_PRE_OPERATIONAL_2 ||
        async_state->NMTStatus == NMT_CS_READY_TO_OPERATE ||
        async_state->NMTStatus == NMT_CS_OPERATIONAL){
      async_state->NMTStatus = NMT_CS_STOPPED;
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTEnterPreOperational2 : {
    //According to Tab. 125 this is only effecitve in NMT_CS_PRE_OPERATIONAL_2,
    //NMT_CS_READY_TO_OPERATE or NMT_CS_OPERATIONAL
    if(async_state->NMTStatus == NMT_CS_STOPPED ||
        async_state->NMTStatus == NMT_CS_OPERATIONAL){
      async_state->NMTStatus = NMT_CS_PRE_OPERATIONAL_2;
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  default :
    //__builtin_unreachable();
    return;
  }
}

// 7.3.4 NMT Info Services
static void handle_nmt_command(nmt_command_t * cmd, ASync_state * async_state, chanend c_eh){
  switch(cmd->NMTCommandID){
    case IdentResponse :{
      //TODO - make this into a function
      if(async_state->mii_pointer_in_use != (uintptr_t)get_ident_response_pointer()){
        build_ident_response(async_state); //make sure this is like lightnening
        async_state->SoA_response_p = (uintptr_t)get_ident_response_pointer();
        async_state->SoA_response_size = IDENT_RESPONSE_BUFFER_SIZE_BYTES;
      } else {
        assert(FALSE && "mii is using a pointer when I want it!");
      }
      break;
    }
    case StatusResponse :{
      /*
      //TODO - make this into a function
      if(async_state->mii_pointer_in_use != (uintptr_t)get_status_response_pointer()){
        build_status_response(async_state); //make sure this is like lightnening
        async_state->SoA_response_p = (uintptr_t)get_status_response_pointer();
        async_state->SoA_response_size = STATUS_RESPONSE_BUFFER_SIZE_BYTES;
      } else {
        assert(FALSE && "mii is using a pointer when I want it!");
      }
      */
      break;
    }
    case NMTStartNode:
    case NMTStopNode:
    case NMTEnterPreOperational2:
    case NMTEnableReadyToOperate:
    case NMTResetNode:
    case NMTResetCommunication:
    case NMTSwReset:
    case NMTResetConfiguration: {
      handle_plain_nmt_command(cmd->NMTCommandID, async_state, c_eh);
      return;
    }

    case NMTStartNodeEx:
    case NMTStopNodeEx:
    case NMTEnterPreOperational2Ex:
    case NMTEnableReadyToOperateEx:
    case NMTResetNodeEx:
    case NMTResetCommunicationEx:
    case NMTSwResetEx:
    case NMTResetConfigurationEx: {
#if FEATURE_Ext_NMT_State_Cmds
      if(!(cmd->NMTCommandData[(NODE_ID>>3)] & (1<<(NODE_ID&7))))
        return;
      cmd = cmd & !EXTENDED_CMD_DIFF; //convert to a plain command
      handle_plain_nmt_command(cmd->NMTCommandID,
          async_state->NMTStatus, nmt_state->RD_flag);
#endif
      return;
    }

    case NMTPublishConfiguredNodes:{
      /*
       * Using the NMTPublishConfiguredNodes service, the MN or a CN may publish a list
       * of nodes configured in its configuration.
       * The NMTPublishConfiguredNodes service uses the POWERLINK Node List format (see 7.3.1.2.3).
       *
       * Node IDs that correspond to configured CNs are indicated by 1b.
       *
       * Information to be published is obtained from NMT_NodeAssignment_AU32 sub-index Bit 1.
       *
       * Support shall be indicated by D_NMT_PublishConfigNodes_BOOL.
       *
       */

      //TODO - no idea what to do with the data yet

      return;
    }

    case NMTPublishActiveNodes:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishPreOperational1:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishPreOperational2:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishReadyToOperate:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishOperational:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishStopped:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishNodeStates:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishEmergencyNew:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTPublishTime:{
      //TODO - no idea what to do with the data yet
      return;
    }
    case NMTInvalidService:{
      //TODO - no idea what to do with the data yet
      return;
    }

#if D_NMT_NetHostNameSet_BOOL
#error D_NMT_NetHostNameSet_BOOL is unsupported yet
    case NMTNetHostNameSet: {
      assert(0 && "not implemented");
      break;
    }
#endif
#if D_NMT_FlushArpEntry_BOOL
#error D_NMT_FlushArpEntry_BOOL is unsupported yet
    case NMTFlushArpEntry: {
      assert(0 && "not implemented");
      break;
    }
#endif
    default:{
      __builtin_trap();
    }
  }
}

void handle_nmt_command_p(uintptr_t nmt_cmd, ASync_state * async_state, chanend c_eh){
  handle_nmt_command((nmt_command_t*)nmt_cmd, async_state, c_eh);
}

static void handle_asnd(ASnd_t * frm, ASync_state * async_state, chanend c_eh){
  switch(frm->ServiceID){
  case IDENT_RESPONSE:{
    assert(0 && "IDENT_RESPONSE should not be for us");
    break;
  }
  case STATUS_RESPONSE:{
    assert(0 && "STATUS_RESPONSE should not be for us");
    break;
  }
  case NMT_REQUEST:{
    assert(0 && "NMT_REQUEST should not be for us");
    break;
  }
  case NMT_COMMAND:{

    handle_nmt_command(&(frm->nmt_cmd), async_state, c_eh);
    break;
  }
  case SDO:{
    printf("ooh an sdo for me\n");
    handle_SDO(&(frm->sdo), async_state, c_eh);
    break;
  }
  default:{
    assert(0); //no support for any extensions yet
    break;
  }
  }
}

static void handle_powerlink_frame(EPL_t * frm, ASync_state * async_state, chanend c_eh){
  switch(frm->type){
  case Asynchronous_Send:{
    handle_asnd(&(frm->asnd), async_state, c_eh);
    break;
  }
  default:{
    //should have been handled elsewhere
    assert(0);
    break;
  }
  }
}

void process_ASnd_or_nonpowerlink_frame(uintptr_t rx_buffer_address, chanend c_eh, ASync_state * async_state){

  frame * frm = (frame*)rx_buffer_address;

  switch(frm->ethertype){
    case IPv4:{
      handle_IPv4(&(frm->ipv4), async_state, c_eh);
      break;
    }
    case Powerlink:{
      //This is a powerlink frame
      handle_powerlink_frame(&(frm->pl_frame), async_state, c_eh);
      break;

    }
    case ARP:{
      //I don't care about arps
      break;
    }
    default : {
      break;
     // _builtin_trap();
    }
  }
}

int is_valid_nmt_command(uintptr_t nmt_cmd_p, t_nmt_command valid_commands[], unsigned valid_cmd_count){
  nmt_command_t * nmt_cmd = (nmt_command_t *)nmt_cmd_p;
  for(unsigned i=0;i<valid_cmd_count;i++)
    if(nmt_cmd->NMTCommandID == valid_commands[i]) return 1;
#if FEATURE_Ext_NMT_State_Cmds
  //check the extended cmds as well
  for(unsigned i=0;i<valid_cmd_count;i++)
    if(nmt_cmd->NMTCommandID == (valid_commands[i]+EXTENDED_CMD_DIFF)) return 1;
#endif
  return 0;
}

int is_nmt_command(uintptr_t rx_buffer_address, uintptr_t * nmt_cmd){

  frame * frm = (frame*)rx_buffer_address;
  if(frm->pl_frame.asnd.ServiceID == NMT_COMMAND){
    *nmt_cmd = (uintptr_t)&(frm->pl_frame.asnd);
    return 1;
  } else {
    return 0;
  }
}

int reject_pl_dst(uintptr_t rx_buffer_address, unsigned node_id){
  frame * frm = (frame*)rx_buffer_address;
  if(frm->pl_frame.dst == node_id) return 0;
  return 1;
}

int reject_mac(uintptr_t rx_buffer_address){
  frame * frm = (frame*)rx_buffer_address;
  if(frm->dst[0]&1) return 0;
  if(frm->dst[0] == MAC_0 && frm->dst[1] == MAC_1 &&
      frm->dst[2] == MAC_2 && frm->dst[3] == MAC_3 &&
      frm->dst[4] == MAC_4 && frm->dst[5] == MAC_5 ) return 0;
  return 1;
}
