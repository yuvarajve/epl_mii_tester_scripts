#ifndef PL_NMT_H_
#define PL_NMT_H_

#include <xccompat.h>
#include <stdint.h>
#include <pl_defines.h>

#include "device_description.h"

typedef struct t_frame_SoA {
  unsigned char NMTStatus;
  unsigned char RequestedServiceID;
  unsigned char RequestedServiceTarget;
  unsigned char EPLVersion;
  unsigned char EA;
  unsigned char ER;
} t_frame_SoA;

/*
 * The purpose of this struct is to store the state of the Network Management Services
                    IDENT_REQUEST
                    STATUS_REQUEST
                    NMT_REQUEST_INVITE
                    MANUF_SVC_IDS
                    UNSPECIFIED_INVITE
 */
#define ASYNC_QUEUE_SIZE 4

enum {INIT = 0, CLOSED = 1, OPEN = 2};

#include "async_sdo_seq_layer.h"
#include "async_sdo_cmd_layer.h"
typedef struct {




/*  maybe pack these
 * |MC|PS|MS|EN|EC|EA|ER|RD|
 * |x |x |  PR    |   RS   |
 */

  unsigned RD;
  unsigned ER;
  unsigned EA;
  unsigned EN;
  unsigned EC;
  unsigned MS;
  unsigned PS;
  unsigned MC;

  int seen_a_valid_frame_with_ER_true;


  /*
   * With the PRes, IdentResponse or StatusResponse RS flag
   * the CN shall indicate the number of send-ready packages in its queues.
   *
   * An RS value of 0 (000b) shall indicate that the queues are empty and an
   * RS value of 7 (111b) shall indicate that 7 or more packages are queued.
   *
   */
  unsigned RS;
  unsigned PR;

  unsigned PR_queues[8];         //the count of pointer in the PR queue
  uintptr_t PR_queues_p[8][8];   //the pointers
  unsigned PR_queues_size[8][8]; //the size of each pointer

  uintptr_t mii_pointer_in_use;


  //This is stuff for the async slot
  unsigned ASync_waiting_size;
  uintptr_t ASync_waiting_p;
  unsigned ASync_slot_in_use;

  unsigned SoA_response_size;
  uintptr_t SoA_response_p;

  unsigned seq_no_tx_by_me;
  unsigned seq_no_rx_by_me;


  unsigned rx_waiting;

  //NMT stuff
  t_nmt_state NMTStatus;
  uint8_t node_id;


  //want to send a NMT_REQUEST_INVITE
  unsigned nmt_req_invite;
  //want to send an UNSPECIFIED_INVITE
  unsigned unspec_invite;

  unsigned asnd_invite_response;

  unsigned seq_con;
} ASync_state;

/*
 * This assumes rx buffer points to a valid frame and that the frame is of the form: frame->powerlink->asnd.
 * From here all that is left to do is verify that the ServiceID is NMTCommand.
 */

void report_error(chanend c_eh, unsigned error);

void request_status_response_from_eh(chanend c_eh, REFERENCE_PARAM(ASync_state, async_state));

int is_nmt_command(uintptr_t rx_buffer_address, REFERENCE_PARAM(uintptr_t,  nmt_cmd));
int is_valid_nmt_command(uintptr_t nmt_cmd_p, t_nmt_command valid_commands[], unsigned valid_cmd_count);
int reject_mac(uintptr_t rx_buffer_address);
int reject_pl_dst(uintptr_t rx_buffer_address, unsigned node_id);

void handle_nmt_command_p(uintptr_t nmt_cmd, REFERENCE_PARAM(ASync_state, async_state), chanend c_eh);


void process_PReq_error_flags( REFERENCE_PARAM(ASync_state, async_state), uintptr_t rx_buffer_address);
void process_ASnd_or_nonpowerlink_frame(uintptr_t rx_buffer_address, chanend c_eh, REFERENCE_PARAM(ASync_state, async_state));
void process_SoA(uintptr_t rx_buffer_address, REFERENCE_PARAM(ASync_state, async_state), chanend c_eh);

#endif /* PL_NMT_H_ */
