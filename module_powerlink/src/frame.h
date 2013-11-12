#ifndef FRAME_H_
#define FRAME_H_
#include <stdint.h>
#include "pl_general_purpose_constants.h"

#ifndef __XC__

typedef struct {
  uint8_t NMTStatus;
  uint8_t:1, ER:1, EA:1,:5;
  uint8_t res;
  uint8_t RequestedServiceID;
  uint8_t RequestedServiceTarget;
  uint8_t EPLVersion;
} SoA_t;

typedef struct {
  uint8_t NMTStatus;
  uint8_t RD:1, :3, EN:1, MS:1, :2;
  uint8_t RS:3, PR:3, :2;
  uint8_t PDOVersion;
  uint8_t res;
  uint8_t size;
  uint8_t payload[C_DLL_MAX_PAYL_OFFSET-10];
} PRes_t;

typedef struct {
  uint8_t NMTStatus;
  uint8_t RD:1,:1, EA:1, :2, MS:1, :2;
  uint8_t res0;
  uint8_t PDOVersion;
  uint8_t res1;
  uint8_t size;
  uint8_t payload[C_DLL_MAX_PAYL_OFFSET-10];
} PReq_t;

typedef struct {
  uint8_t res0;
  uint8_t :6, PS:1, MC:1;
  uint8_t res1;
  uint8_t NetTime[8];
  uint8_t RelativeTime[8];
} SoC_t;

typedef struct {
  uint8_t ReceiveCon:2, ReceiveSequenceNumber:6;
  uint8_t SendCon:2, SendSequenceNumber:6;
  uint8_t res_0;
  uint8_t res_1;
} Sequence_Layer_Protocol_t;

typedef struct {
  //TODO remove
  int x;
  uint8_t payload[];
} cmd_without_initiate;

typedef struct {
  uint32_t DataSize __attribute__((packed));
  //cmd_without_initiate cmd;
  uint8_t payload[];
} cmd_with_initiate;

typedef struct {
  uint8_t res0;
  uint8_t Transaction_ID;
  uint8_t reserved:4, Segmentation:2, Abort:1, Response:1;
  uint8_t Command_ID;
  uint16_t Segment_Size __attribute__((packed));
  uint16_t res1 __attribute__((packed));
  union {
    cmd_with_initiate cmd_init;
    cmd_without_initiate cmd_no_init;
  };
} Command_Layer_Protocol_t;

typedef struct {
  uint8_t NMTCommandID;
  uint8_t Reserved;
  uint8_t NMTCommandData[];
} nmt_command_t;

typedef struct {
  Sequence_Layer_Protocol_t slp;
  Command_Layer_Protocol_t clp;
} SDO_t;

typedef struct {
  uint8_t MessageType;
  uint8_t res0;
  uint8_t res1;
  uint8_t ServiceID;
  SDO_t sdo;
} SDO_via_UDP_t;

typedef struct {
  uint8_t ServiceID;
  union {
    SDO_t sdo;
    nmt_command_t nmt_cmd;
  } __attribute__((packed));
} ASnd_t;

typedef struct {
  uint8_t type:7, res:1;
  uint8_t dst;
  uint8_t src;
  union {
    PRes_t pres;
    PReq_t preq;
    SoA_t soa;
    SoC_t soc;
    ASnd_t asnd;
  } __attribute__((packed));
} EPL_t;

typedef struct {
 uint16_t Source_Port;
 uint16_t Destination_Port;
 uint8_t Length;
 uint8_t Checksum;
 union {
   SDO_via_UDP_t sdo ;
 } __attribute__((packed));
} UDP_t;

typedef struct {
  uint8_t IHL:4, Version:4;
  uint8_t ECN:2, DSCP:6;
  uint16_t Total_Length __attribute__((packed));
  uint16_t Identification;

  //uint16_t Fragment_Offset:13, Flags:3 __attribute__((packed));
  uint8_t Fragment_Offset_Flags[2];

  uint8_t Time_To_Live;
  uint8_t Protocol;
  uint16_t Header_Checksum __attribute__((packed));
  uint8_t Src_IP[4];
  uint8_t Dst_IP[4];
  union {
    UDP_t udp __attribute__((packed));
  } __attribute__((packed)) ;
} IPv4_t;

typedef struct {
  uint8_t dst[6];
  uint8_t src[6];
  uint16_t ethertype __attribute__((packed));
  union {
    EPL_t pl_frame;
    IPv4_t ipv4 __attribute__((packed));
  } __attribute__((packed));
} frame;
#endif
#endif /* FRAME_H_ */
