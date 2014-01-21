#ifndef PL_DEFINES_H_
#define PL_DEFINES_H_

#include "pl_general_purpose_constants.h"

#define FALSE 0
#define TRUE 1

typedef enum {
  NMT_CT1,  // The occurrence of the SoC event indicates the beginning of a new
            // POWERLINK cycle. The asynchronous phase of the previous cycle ends
            // and the isochronous phase of the next cycle begins.

  NMT_CT2,  // If a SoA or SoC frame is received in NMT_CS_NOT_ACTIVE,
            // the node shall change over to thestate NMT_CS_PRE_OPERATIONAL_1.

  NMT_CT3,  //

  NMT_CT4,  // If the node receives a SoC frame in NMT_CS_PRE_OPERATIONAL_1,
            // the node shall change over to NMT_CS_PRE_OPERATIONAL_2.

  NMT_CT5,  //
  NMT_CT6,  //
  NMT_CT7,  //
  NMT_CT8,  //
  NMT_CT9,  //
  NMT_CT10, //
  NMT_CT11, // If the node recognizes an error condition in NMT_CS_PRE_OPERATIONAL_2,
            // NMT_CS_READY_TO_OPERATE, NMT_CS_OPERATIONAL or NMT_CS_STOPPED,
            // the node shall change over to NMT_CS_PRE_OPERATIONAL_1
  NMT_CT12, //

} nmt_transition_t;

typedef enum {
  //requestable ASnd ServiceIDs
  IdentResponse = 0x01,
  StatusResponse = 0x02,

  //Plain NMT State Commands
  NMTStartNode = 0x21,
  NMTStopNode = 0x22,
  NMTEnterPreOperational2 = 0x23,
  NMTEnableReadyToOperate = 0x24,
  NMTResetNode = 0x28,
  NMTResetCommunication = 0x29,
  NMTResetConfiguration = 0x2a,
  NMTSwReset = 0x2b,
  //NMTGoToStandby = 0x2c, //EPSG DS302-A [1]

  //Extended NMT State Commands
  NMTStartNodeEx = 0x41,
  NMTStopNodeEx = 0x42,
  NMTEnterPreOperational2Ex = 0x43,
  NMTEnableReadyToOperateEx = 0x44,
  NMTResetNodeEx = 0x48,
  NMTResetCommunicationEx = 0x49,
  NMTResetConfigurationEx = 0x4a,
  NMTSwResetEx = 0x4b,

  //NMT Managing Commands
  NMTNetHostNameSet = 0x62,
  NMTFlushArpEntry = 0x63,

  //NMT Info services
  NMTPublishConfiguredNodes = 0x80,
  NMTPublishActiveNodes = 0x90,
  NMTPublishPreOperational1 = 0x91,
  NMTPublishPreOperational2 = 0x92,
  NMTPublishReadyToOperate = 0x93,
  NMTPublishOperational = 0x94,
  NMTPublishStopped = 0x95,
  NMTPublishNodeStates = 0x96,
  NMTPublishEmergencyNew = 0xa0,
  NMTPublishTime = 0xb0,
  NMTInvalidService = 0xc0
} nmt_command_id_t;

#define EXTENDED_CMD_DIFF (NMTStartNodeEx - NMTStartNode)

typedef enum {
  NMT_GS_OFF = 0x00,
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

//POWERLINK Message Type IDs
typedef enum {
  Start_of_Cycle = 0x01,
  PollRequest = 0x03,
  PollResponse = 0x04,
  Start_of_Asynchronous = 0x05,
  Asynchronous_Send = 0x06
  //Active_Managing_Node_Indication  =0x07, //used by EPSG DS302-A [1]
  //Asynchronous_Invite  =0x0d, // used by EPSG DS302-B [2]
} message_type_id_t;

//ASnd ServiceIDs
typedef enum {
  //reserved                0x00
  IDENT_RESPONSE = 0x01,
  STATUS_RESPONSE = 0x02,
  NMT_REQUEST = 0x03,
  NMT_COMMAND = 0x04,
  SDO = 0x05
  //reserved                0x06 .. 0x9F
  //Manufacturer_specific   0xA0 .. 0xFE
  //reserved                0xFF
} asnd_serviceids_t;

//SoA RequestedServiceIDs
typedef enum {
  NO_SERVICE = 0x00,
  IDENT_REQUEST = 0x01,
  STATUS_REQUEST = 0x02,
  NMT_REQUEST_INVITE = 0x03,
  //reserved                0x04 ... 0x9F
  MANUF_SVC_IDS =  0xA0, // ... 0xFE
  UNSPECIFIED_INVITE = 0xFF,
} SoA_RequestedServiceIDs_t;


typedef enum {
  DLL_CE_SOC,
  DLL_CE_PREQ,
  DLL_CE_PRES,
  DLL_CE_SOA,
  DLL_CE_ASND,
  DLL_CE_SOC_TIMEOUT
} dll_events_t;

//6.3.2.4.2 Commands
typedef enum {
  NIL=0x0,
  Write_by_Index=0x1,
  Read_by_Index=0x2,
  Write_All_by_Index=0x3,
  Read_All_by_Index=0x4,
  Write_by_Name=0x5,
  Read_by_Name=0x6,
  File_Write=0x20,
  File_Read=0x21,
  Write_Multiple_Parameter_by_Index=0x31,
  Read_Multiple_Parameter_by_Index=0x32,
  Maximum_Segment_Size=0x70,
  //Manufacturer specific 0x80h - 0xFF
} Commands_t;

typedef enum {
  Expedited_Transfer = 0x0,
  Initiate_Segm_Transfer = 0x1,
  Segment = 0x2,
  Segm_Transfer_Complete = 0x3,
} Segmentation_t;

typedef enum {
  IPv4 = 0x0008,
  ARP = 0x0608,
  IPv6 = 0xDD86,
  Powerlink = 0xab88
} Ethertype_t;

#endif /* PL_DEFINES_H_ */
