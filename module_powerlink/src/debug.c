#include "debug.h"
#include "frame.h"
#include "pl_defines.h"
#include <stdio.h>


static uint16_t htons(uint16_t s){
  return (__builtin_bswap32(s)>>16)&0xffff;
}

static void print_SDO(SDO_t * f);

static void print_soc(SoC_t * f){
  printf("MC:%d PS:%d NetTime: 0x%02x%02x%02x%02x%02x%02x RelativeTime 0x%02x%02x%02x%02x%02x%02x\n",
      f->MC, f->PS, f->NetTime[0],f->NetTime[1],f->NetTime[2],f->NetTime[3],f->NetTime[4],f->NetTime[5],
      f->RelativeTime[0], f->RelativeTime[1], f->RelativeTime[2], f->RelativeTime[3], f->RelativeTime[4], f->RelativeTime[5]);
}
static void print_preq(PReq_t * f){
  printf("RD:%d EA:%d MS:%d PDOVersion:%02x Size %04d\n",
       f->RD, f->EA, f->MS, f->PDOVersion, f->size);
  /*
  for(unsigned i=0;i<f->size;i++){
    printf("%02x ", f->payload[i]);
    if(i%8 == 7)
      printf("\n");
  }
*/
}

static void print_pres(PRes_t * f){
  printf("NMTStatus: 0x%02x RD:%d EN:%d MS:%d PR:%02d RS:%02d PDOVersion:%02x Size %04d\n",
      f->NMTStatus, f->RD, f->EN, f->MS, f->PR, f->RS, f->PDOVersion, f->size);

  for(unsigned i=0;i<f->size;i++){
    printf("%02x ", f->payload[i]);
    if(i%8 == 7)
      printf("\n");
  }


}
static void print_soa(SoA_t * f){
  printf("ER:%d EA:%d RequestedServiceID:%02x RequestedServiceTarget:%02x EPLVersion 0x%02x\n", f->ER, f->EA, f->RequestedServiceID, f->RequestedServiceTarget, f->EPLVersion);

}

static void print_nmt_command(nmt_command_t * frm){
  switch(frm->NMTCommandID){
  case IdentResponse: printf("IdentResponse\n");break;
  case StatusResponse: printf("StatusResponse\n");break;
  case NMTStartNode: printf("NMTStartNode\n");break;
  case NMTStopNode: printf("NMTStopNode\n");break;
  case NMTEnterPreOperational2: printf("NMTEnterPreOperational2\n");break;
  case NMTEnableReadyToOperate: printf("NMTEnableReadyToOperate\n");break;
  case NMTResetNode: printf("NMTResetNode\n");break;
  case NMTResetCommunication: printf("NMTResetCommunication\n");break;
  case NMTResetConfiguration: printf("NMTResetConfiguration\n");break;
  case NMTSwReset: printf("NMTSwReset\n");break;
  case NMTStartNodeEx: printf("NMTStartNodeEx\n");break;
  case NMTStopNodeEx: printf("NMTStopNodeEx\n");break;
  case NMTEnterPreOperational2Ex: printf("NMTEnterPreOperational2Ex\n");break;
  case NMTEnableReadyToOperateEx: printf("NMTEnableReadyToOperateEx\n");break;
  case NMTResetNodeEx: printf("NMTResetNodeEx\n");break;
  case NMTResetCommunicationEx: printf("NMTResetCommunicationEx\n");break;
  case NMTResetConfigurationEx: printf("NMTResetConfigurationEx\n");break;
  case NMTSwResetEx: printf("NMTSwResetEx\n");break;
  case NMTNetHostNameSet: printf("NMTNetHostNameSet\n");break;
  case NMTFlushArpEntry: printf("NMTFlushArpEntry\n");break;
  case NMTPublishConfiguredNodes: printf("NMTPublishConfiguredNodes\n");break;
  case NMTPublishActiveNodes: printf("NMTPublishActiveNodes\n");break;
  case NMTPublishPreOperational1: printf("NMTPublishPreOperational1\n");break;
  case NMTPublishPreOperational2: printf("NMTPublishPreOperational2\n");break;
  case NMTPublishReadyToOperate: printf("NMTPublishReadyToOperate\n");break;
  case NMTPublishOperational: printf("NMTPublishOperational\n");break;
  case NMTPublishStopped: printf("NMTPublishStopped\n");break;
  case NMTPublishNodeStates: printf("NMTPublishNodeStates\n");break;
  case NMTPublishEmergencyNew: printf("NMTPublishEmergencyNew\n");break;
  case NMTPublishTime: printf("NMTPublishTime\n");break;
  case NMTInvalidService: printf("NMTInvalidService\n");break;
  }
}

static void print_asnd(ASnd_t * frm){
switch(frm->ServiceID){
  case IDENT_RESPONSE:{
    printf("IDENT_RESPONSE ");
    break;
  }
  case STATUS_RESPONSE:{
    printf("STATUS_RESPONSE ");
    break;
  }
  case NMT_REQUEST:{
    printf("NMT_REQUEST ");
    break;
  }
  case NMT_COMMAND:{
    printf("NMT_COMMAND \n");

    nmt_command_t * n = &(frm->nmt_cmd);
    print_nmt_command(n);
    break;
  }
  case SDO:{
    printf("SDO ");
    print_SDO(&(frm->sdo));
    break;
  }
  default:{
    assert(0); //no support for any extensions yet
    break;
  }
}
}

static void print_powerlink(EPL_t *f) {
  switch (f->type){
  case Start_of_Cycle        : {printf("SoC  ");break;}
  case PollRequest           : {printf("PReq ");break;}
  case PollResponse          : {printf("PRes ");break;}
  case Start_of_Asynchronous : {printf("SoA  ");break;}
  case Asynchronous_Send     : {printf("ASnd ");break;}
  }

  printf("dst: %03d src: %03d ", f->dst, f->src);

  switch (f->type){
  case Start_of_Cycle        : {print_soc(&(f->soc));break;}
  case PollRequest           : {print_preq(&(f->preq));break;}
  case PollResponse          : {print_pres(&(f->pres));break;}
  case Start_of_Asynchronous : {print_soa(&(f->soa));break;}
  case Asynchronous_Send     : {print_asnd(&(f->asnd));break;}
  printf("\n");
  }
}
const char * ServiceIDs[6] = {
"reserved", "IDENT_RESPONSE", "STATUS_RESPONSE", "NMT_REQUEST",
"NMT_COMMAND", "SDO"
};
const char * MessageTypes[7] = {
"reserved", "Start of Cycle","reserved"
"PollRequest",
"PollResponse",
"Start of Asynchronous",
"Asynchronous Send",
};


static void print_Sequence_Layer_Protocol(Sequence_Layer_Protocol_t * f){
  printf("ReceiveSequenceNumber:%d ReceiveCon:%d SendSequenceNumber:%d SendCon:%d ::: ", f->ReceiveSequenceNumber, f->ReceiveCon,
      f->SendSequenceNumber, f->SendCon);
}

const char * Response_txt[2] = {"Request", "Response"};
const char * Abort_txt[2] = {"transfer ok", "abort transfer"};

const char * Segmentation_txt[4] = {"Expedited Transfer",
                                    "Initiate Segm. Transfer",
                                    "Segment",
                                    "Segm. Transfer Complete"};

void print_sdo_cmd(uint8_t Command_ID, cmd_without_initiate * c){
  switch(Command_ID){
  case NIL:{
    printf("NIL\n");
    break;
  }
  case Write_by_Index:{
    printf("Write_by_Index\n");
    break;
  }
  case Read_by_Index:{
    printf("Read_by_Index\n");
    break;
  }
  case Write_All_by_Index:{
    printf("Write_All_by_Index\n");
    break;
  }
  case Read_All_by_Index:{
    printf("Read_All_by_Index\n");
    break;
  }
  case Write_by_Name:{
    printf("Write_by_Name\n");
    break;
  }
  case Read_by_Name:{
    printf("Read_by_Name\n");
    break;
  }
  case File_Write:{
    printf("File_Write\n");
    break;
  }
  case File_Read:{
    printf("File_Read\n");
    break;
  }
  case Write_Multiple_Parameter_by_Index:{
    printf("Write_Multiple_Parameter_by_Index\n");
    break;
  }
  case Read_Multiple_Parameter_by_Index:{
    printf("Read_Multiple_Parameter_by_Index\n");
    break;
  }
  case Maximum_Segment_Size:{
    printf("Maximum_Segment_Size\n");
    break;
  }
  }
}

static void print_Command_Layer_Protocol(Command_Layer_Protocol_t * f){
  printf("Transaction_ID: %d Segmentation: %s(%d) Abort: %s(%d) Response: %s(%d) ",
      f->Transaction_ID, Segmentation_txt[f->Segmentation],f->Segmentation,
      Abort_txt[f->Abort], f->Abort, Response_txt[f->Response], f->Response);
  printf("Command_ID: %d Segment_Size: %d ", f->Command_ID, f->Segment_Size);
  cmd_without_initiate * c;
  if(f->Segmentation == 1){
    printf("DataSize: %u ", f->cmd_init.DataSize);
   // c = &(f->cmd_init.cmd);
  } else {
    c = &(f->cmd_no_init);
  }
  print_sdo_cmd(f->Command_ID, c);
}

static void print_SDO(SDO_t * f){
  print_Sequence_Layer_Protocol(&(f->slp));
  print_Command_Layer_Protocol(&(f->clp));
}

static void print_SDO_via_UDP(SDO_via_UDP_t * f){
  printf("MessageType: %s(%d) ServiceID %s(%d)\n",
      MessageTypes[f->MessageType], f->MessageType,
      ServiceIDs[f->ServiceID], f->ServiceID);
}

static void print_UDP(UDP_t * f){
  printf("Source_Port:%d Destination_Port:%d Length:%d Checksum:%04x (epl port:%d)\n",
      htons(f->Source_Port), htons(f->Destination_Port),
          htons(f->Length), htons(f->Checksum), C_SDO_EPL_PORT);

  if(htons(f->Destination_Port) == C_SDO_EPL_PORT){
    print_SDO_via_UDP(&(f->sdo));
  } else {
    printf("UDP packet is not for me\n");
  }

}

const char
    * IPv4_Types[141] =
        {
            "IPv6 Hop-by-Hop Option",
            "Internet Control Message Protocol",
            "Internet Group Management Protocol",
            "Gateway-to-Gateway Protocol",
            "IPv4 (encapsulation)",
            "Internet Stream Protocol",
            "Transmission Control Protocol",
            "Core-based trees",
            "Exterior Gateway Protocol",
            "Interior Gateway Protocol (any private interior gateway (used by Cisco for their IGRP))",
            "BBN RCC Monitoring",
            "Network Voice Protocol",
            "Xerox PUP",
            "ARGUS",
            "EMCON",
            "Cross Net Debugger",
            "Chaos",
            "User Datagram Protocol",
            "Multiplexing",
            "DCN Measurement Subsystems",
            "Host Monitoring Protocol",
            "Packet Radio Measurement",
            "XEROX NS IDP",
            "Trunk-1",
            "Trunk-2",
            "Leaf-1",
            "Leaf-2",
            "Reliable Datagram Protocol",
            "Internet Reliable Transaction Protocol",
            "ISO Transport Protocol Class 4",
            "Bulk Data Transfer Protocol",
            "MFE Network Services Protocol",
            "MERIT Internodal Protocol",
            "Datagram Congestion Control Protocol",
            "Third Party Connect Protocol",
            "Inter-Domain Policy Routing Protocol",
            "Xpress Transport Protocol",
            "Datagram Delivery Protocol",
            "IDPR Control Message Transport Protocol",
            "TP++ Transport Protocol",
            "IL Transport Protocol",
            "6in4 (encapsulation)",
            "Source Demand Routing Protocol",
            "Routing Header for IPv6",
            "Fragment Header for IPv6",
            "Inter-Domain Routing Protocol",
            "Resource Reservation Protocol",
            "Generic Routing Encapsulation",
            "Mobile Host Routing Protocol",
            "BNA",
            "Encapsulating Security Payload",
            "Authentication Header",
            "Integrated Net Layer Security Protocol",
            "SwIPe",
            "NBMA Address Resolution Protocol",
            "IP Mobility (Min Encap)",
            "Transport Layer Security Protocol (using Kryptonet key management)",
            "Simple Key-Management for Internet Protocol",
            "ICMP for IPv6",
            "No Next Header for IPv6",
            "Destination Options for IPv6",
            "Any host internal protocol",
            "CFTP",
            "Any local network",
            "SATNET and Backroom EXPAK",
            "Kryptolan",
            "MIT Remote Virtual Disk Protocol",
            "Internet Pluribus Packet Core",
            "Any distributed file system",
            "SATNET Monitoring",
            "VISA Protocol",
            "Internet Packet Core Utility",
            "Computer Protocol Network Executive",
            "Computer Protocol Heart Beat",
            "Wang Span Network",
            "Packet Video Protocol",
            "Backroom SATNET Monitoring",
            "SUN ND PROTOCOL-Temporary",
            "WIDEBAND Monitoring",
            "WIDEBAND EXPAK",
            "International Organization for Standardization Internet Protocol",
            "Versatile Message Transaction Protocol",
            "Secure Versatile Message Transaction Protocol",
            "VINES",
            "Internet Protocol Traffic Manager",
            "NSFNET-IGP",
            "Dissimilar Gateway Protocol",
            "TCF",
            "EIGRP",
            "Open Shortest Path First",
            "Sprite RPC Protocol",
            "Locus Address Resolution Protocol",
            "Multicast Transport Protocol",
            "AX.25",
            "IP-within-IP Encapsulation Protocol",
            "Mobile Internetworking Control Protocol",
            "Semaphore Communications Sec. Pro",
            "Ethernet-within-IP Encapsulation",
            "Encapsulation Header",
            "Any private encryption scheme",
            "GMTP",
            "Ipsilon Flow Management Protocol",
            "PNNI over IP",
            "Protocol Independent Multicast",
            "IBM's ARIS (Aggregate Route IP Switching) Protocol",
            "SCPS (Space Communications Protocol Standards)",
            "QNX",
            "Active Networks",
            "IP Payload Compression Protocol",
            "Sitara Networks Protocol",
            "Compaq Peer Protocol",
            "IPX in IP",
            "Virtual Router Redundancy Protocol, Common Address Redundancy Protocol (not IANA assigned)",
            "PGM Reliable Transport Protocol", "Any 0-hop protocol",
            "Layer Two Tunneling Protocol Version 3",
            "D-II Data Exchange (DDX)", "Interactive Agent Transfer Protocol",
            "Schedule Transfer Protocol", "SpectraLink Radio Protocol", "UTI",
            "Simple Message Protocol", "SM",
            "Performance Transparency Protocol", "IS-IS over IPv4", "",
            "Combat Radio Transport Protocol", "Combat Radio User Datagram",
            "", "", "Secure Packet Shield",
            "Private IP Encapsulation within IP",
            "Stream Control Transmission Protocol", "Fibre Channel",
            "RSVP-E2E-IGNORE", "Mobility Header", "UDP Lite", "MPLS-in-IP",
            "MANET Protocols", "Host Identity Protocol",
            "Site Multihoming by IPv6 Intermediation"};

static void print_ipv4(IPv4_t * f) {
#define VERBOSE_IPv4

#ifdef VERBOSE_IPv4
  printf("src: %d.%d.%0d.%d dst: %d.%d.%d.%d\n",
      f->Src_IP[0], f->Src_IP[1], f->Src_IP[2], f->Src_IP[3],
      f->Dst_IP[0], f->Dst_IP[1], f->Dst_IP[2], f->Dst_IP[3]);
  printf("Version: %d IHL:%d DSCP:%d ECN:%d Total_Length:%d Identification:%d\n",
      f->Version, f->IHL, f->DSCP, f->ECN, htons(f->Total_Length),
       f->Identification);
#endif
  printf("Protocol: %s\n", IPv4_Types[f->Protocol]);

  switch(f->Protocol){
  case 17:{
    print_UDP(&(f->udp));
    break;
  }
  default:{
    break;
  }
  }
}
#define VERBOSE_ETHERNET_II

static void print_frame(frame * f){
  unsigned ethertype = htons(f->ethertype);
#ifdef VERBOSE_ETHERNET_II
  printf("dst: 0x%02x%02x%02x%02x%02x%02x src: 0x%02x%02x%02x%02x%02x%02x ",
      f->dst[0], f->dst[1], f->dst[2], f->dst[3], f->dst[4], f->dst[5],
      f->src[0], f->src[1], f->src[2], f->src[3], f->src[4], f->src[5]);



  switch(ethertype){
      case 0x0800:printf("IPv4        ");break;
      case 0x0806:printf("ARP         ");break;
      case 0x86DD:printf("IPv6        ");break;
      case 0x8870:printf("Jumbo Frame ");break;
      case 0x8892:printf("PROFINET    ");break;
      case 0x88A4:printf("EtherCAT    ");break;
      case C_DLL_ETHERTYPE_EPL:printf("Powerlink   ");break;
      default:
        printf(" 0x%04x      ", ethertype);
        break;
    }
#endif

  if(ethertype == C_DLL_ETHERTYPE_EPL){
    printf(" ");
    print_powerlink(&(f->pl_frame));
  } else if(f->ethertype == 0x0008){
    printf(" ");
    print_ipv4(&(f->ipv4));
    printf("\n");
  } else {
    printf("\n");
  }
}

void print(intptr_t pointer){
  frame * frm = (frame *)pointer;
  print_frame(frm);
}
#include <print.h>
void print_nmt_state(nmt_state_t state){
  switch(state){
    case NMT_GS_OFF: printstrln("NMT_GS_OFF");break;
    case NMT_GS_INITIALISING: printstrln("NMT_GS_INITIALISING");break;
    case NMT_GS_RESET_APPLICATION: printstrln("NMT_GS_RESET_APPLICATION");break;
    case NMT_GS_RESET_COMMUNICATION: printstrln("NMT_GS_RESET_COMMUNICATION");break;
    case NMT_GS_RESET_CONFIGURATION: printstrln("NMT_GS_RESET_CONFIGURATION");break;
    case NMT_CS_NOT_ACTIVE: printstrln("NMT_CS_NOT_ACTIVE");break;
    case NMT_CS_PRE_OPERATIONAL_1: printstrln("NMT_CS_PRE_OPERATIONAL_1");break;
    case NMT_CS_PRE_OPERATIONAL_2: printstrln("NMT_CS_PRE_OPERATIONAL_2");break;
    case NMT_CS_READY_TO_OPERATE: printstrln("NMT_CS_READY_TO_OPERATE");break;
    case NMT_CS_OPERATIONAL: printstrln("NMT_CS_OPERATIONAL");break;
    case NMT_CS_STOPPED: printstrln("NMT_CS_STOPPED");break;
    case NMT_CS_BASIC_ETHERNET: printstrln("NMT_CS_BASIC_ETHERNET");break;
  }

}

