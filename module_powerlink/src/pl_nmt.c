#include <stdlib.h>

#include "pl_nmt.h"
#include "pl_defines.h"
#include "pl_general_purpose_constants.h"
#include "frame.h"
#include "debug.h"
#include "device_description.h"
#include <xscope.h>
#include "async_sdo_seq_layer.h"

#include "global_state.h"


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


extern nmt_state_t g_nmt_status;


extern int error_signalling_data_present;

int preq_is_for_me(uintptr_t p){
  frame * frm = (frame*)p;
  //return frm->pl_frame.dst == get_node_id()
      return frm->pl_frame.dst == 1;
}
void process_PReq_error_flags(ASync_state * async_state, uintptr_t rx_buffer_address){
  frame * frm = (frame*)rx_buffer_address;

   // SoA_t * soa = &(frm->pl_frame.soa);

}

static void handle_SDO(SDO_t * frm,  ASync_state * async_state, chanend c_eh, chanend c_can_open){
  /*
   * The sdo is made up of two fields:
   *  Sequence Layer
   *  Command Layer
   */
  Sequence_Layer_Protocol_t *s = &(frm->slp);
  Command_Layer_Protocol_t *c = &(frm->clp);

  sdo_seq_recieved(s, c, async_state, c_can_open);
}

static void handle_SDO_via_UDP(SDO_via_UDP_t * frm,  ASync_state * async_state, chanend c_eh, chanend c_can_open){
  if(frm->MessageType != Asynchronous_Send){
    assert(0 && "SDO via UDP not Asynchronous_Send!");
    return;
  }
  if(frm->ServiceID != SDO){
    assert(0 && "SDO via UDP not SDO!");
    return;
  }
  handle_SDO(&(frm->sdo), async_state, c_eh, c_can_open);
}

#define UDP 0x11

static void handle_IPv4(IPv4_t * frm,  ASync_state * async_state, chanend c_eh, chanend c_can_open){

  //TODO any IPv4 stuff!
  if(frm->Dst_IP[0] != IP_0 ||  //192
      frm->Dst_IP[1] != IP_1 || //168
      frm->Dst_IP[2] != IP_2 || //100
      frm->Dst_IP[3] != get_node_id()){
    return;
  }

  switch(frm->Protocol){
    case UDP:{
      //TODO test the checksum
      uint16_t crc = 0xffff;

      if(crc == 0xffff){
        if(htons(frm->udp.Destination_Port) == C_SDO_EPL_PORT){
          handle_SDO_via_UDP(&(frm->udp.sdo), async_state, c_eh, c_can_open);
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


#define SHOW_STATE_TRANSITIONS 0

/*
 * We make the assumption that extended commands are valid and have been converted to
 * plain commands, i.e. if not valid for this node then this isn't called.
 */
static void handle_plain_nmt_command(uint8_t cmd,  ASync_state * async_state, chanend c_eh){
  int nmt_cs_communicating = g_nmt_status >= NMT_CS_NOT_ACTIVE;

  switch (cmd){
  case NMTSwReset:{
    if(nmt_cs_communicating){
      g_nmt_status = NMT_GS_INITIALISING;
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_INITIALISING");
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTResetNode :{
    if(nmt_cs_communicating){
      g_nmt_status = NMT_GS_RESET_APPLICATION;
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_APPLICATION");
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTResetCommunication :{
    if(nmt_cs_communicating){
      g_nmt_status = NMT_GS_RESET_COMMUNICATION;
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_COMMUNICATION");
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTResetConfiguration :{
    if(nmt_cs_communicating){
      g_nmt_status = NMT_GS_RESET_CONFIGURATION;
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_GS_RESET_CONFIGURATION");
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTEnableReadyToOperate :{


    //According to Tab. 125 this is only effecitve in NMT_CS_PRE_OPERATIONAL_2
    if(g_nmt_status == NMT_CS_PRE_OPERATIONAL_2 ){
      //TODO
      //if config is complete - what's this???
      int config_is_complete =1;
      if(config_is_complete){
        g_nmt_status = NMT_CS_READY_TO_OPERATE;
        if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_READY_TO_OPERATE");
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
    if(g_nmt_status == NMT_CS_READY_TO_OPERATE){
      //TODO
      //if config is valid - what's this???
      int config_is_valid = 1;
      if(config_is_valid)
        g_nmt_status = NMT_CS_OPERATIONAL;
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_OPERATIONAL");
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTStopNode : {
    //According to Tab. 125 this is only effecitve in NMT_CS_PRE_OPERATIONAL_2,
    //NMT_CS_READY_TO_OPERATE or NMT_CS_OPERATIONAL
    if(g_nmt_status == NMT_CS_PRE_OPERATIONAL_2 ||
        g_nmt_status == NMT_CS_READY_TO_OPERATE ||
        g_nmt_status == NMT_CS_OPERATIONAL){
      g_nmt_status = NMT_CS_STOPPED;
      if(SHOW_STATE_TRANSITIONS) printstrln("NMT -> NMT_CS_STOPPED");
    } else {
      //TODO report error in log
      report_error(c_eh, 0);
    }
    return;
  }
  case NMTEnterPreOperational2 : {
    //According to Tab. 125 this is only effecitve in NMT_CS_PRE_OPERATIONAL_2,
    //NMT_CS_READY_TO_OPERATE or NMT_CS_OPERATIONAL
    if(g_nmt_status == NMT_CS_STOPPED ||
        g_nmt_status == NMT_CS_OPERATIONAL){
      g_nmt_status = NMT_CS_PRE_OPERATIONAL_2;
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
      //build_ident_response(async_state); //make sure this is like lightnening
    // async_state->SoA_response_p = (uintptr_t)get_ident_response_pointer();
     // async_state->SoA_response_size = IDENT_RESPONSE_BUFFER_SIZE_BYTES;
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
          g_nmt_status, nmt_state->RD_flag);
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

static void handle_asnd(ASnd_t * frm, ASync_state * async_state, chanend c_eh, chanend c_can_open){
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
    handle_SDO(&(frm->sdo), async_state, c_eh, c_can_open);
    break;
  }
  default:{
    assert(0); //no support for any extensions yet
    break;
  }
  }
}

static void handle_powerlink_frame(EPL_t * frm, ASync_state * async_state, chanend c_eh, chanend c_can_open){
  switch(frm->type){
  case Asynchronous_Send:{
    handle_asnd(&(frm->asnd), async_state, c_eh, c_can_open);
    break;
  }
  default:{
    //should have been handled elsewhere
    assert(0);
    break;
  }
  }
}

void process_ASnd_or_nonpowerlink_frame(uintptr_t rx_buffer_address, chanend c_eh, ASync_state * async_state, chanend c_can_open){

  frame * frm = (frame*)rx_buffer_address;

  switch(frm->ethertype){
    case IPv4:{
      handle_IPv4(&(frm->ipv4), async_state, c_eh, c_can_open);
      break;
    }
    case Powerlink:{
      //This is a powerlink frame
      handle_powerlink_frame(&(frm->pl_frame), async_state, c_eh, c_can_open);
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

int is_valid_nmt_command(uintptr_t nmt_cmd_p, nmt_command_id_t valid_commands[], unsigned valid_cmd_count){
  nmt_command_t * nmt_cmd = (nmt_command_t *)nmt_cmd_p;
  for(unsigned i=0;i<valid_cmd_count;i++){
    if(nmt_cmd->NMTCommandID == valid_commands[i]) return 1;
#if FEATURE_Ext_NMT_State_Cmds
  //check the extended cmds as well
  for(unsigned i=0;i<valid_cmd_count;i++)
    if(nmt_cmd->NMTCommandID == (valid_commands[i]+EXTENDED_CMD_DIFF)) return 1;
#endif
  }
  return 0;
}

int is_nmt_command(uintptr_t rx_buffer_address, uintptr_t * nmt_cmd){

  frame * frm = (frame*)rx_buffer_address;
  if(frm->pl_frame.asnd.ServiceID == NMT_COMMAND){
    *nmt_cmd = (uintptr_t)&(frm->pl_frame.asnd.nmt_cmd);
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
