/*
 * This exists to manage the "Asynchronous SDO Sequence Layer".
 *
 * The POWERLINK Sequence Layer provides the service of a reliable
 * bidirectional connection that guarantees that no messages are lost
 * or duplicated and that all messages arrive in the correct order.
 * Fragmentation is handled by the SDO Command Layer (6.3.2.4). The
 * POWERLINK Sequence Layer Header for asynchronous transfer shall
 * consist of 2 bytes. There shall be a sequence number for each sent
 * frame, and an acknowledgement for the sequence number of the opposite
 * node, as well a connection state and a connection acknowledge.
 *
 */

#include "async_sdo_seq_layer.h"
#include "async_sdo_cmd_layer.h"
#include "device_description.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

//#define PRINT_DEBUG

#define NODE_ID 1

#define TX_HISTORY_SIZE 4

/*
A minimum size of 256 bytes must be supported by every device. A
maximum of 1458 bytes may be supported.
*/

static uint8_t sdo_buf[TX_HISTORY_SIZE][1460] = {{
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_MN_DEF_NODE_ID,
    NODE_ID,
    SDO,
},{
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_MN_DEF_NODE_ID,
    NODE_ID,
    SDO,
},{
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_MN_DEF_NODE_ID,
    NODE_ID,
    SDO,
},{
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_MN_DEF_NODE_ID,
    NODE_ID,
    SDO,
},
};

typedef struct {
  unsigned rcon, scon, rsnr, ssnr;

  //this is the highest ssrn that has been confirmed to have been sent by the mii
  unsigned actually_ssnr;

  uint8_t * history_buffer[TX_HISTORY_SIZE];
  unsigned history_buffer_size[TX_HISTORY_SIZE];
  unsigned history_buffer_ssnr[TX_HISTORY_SIZE];
  unsigned history_buffer_tail;
  unsigned history_buffer_fill;

} sdo_seq_state;

sdo_seq_state seq_state;
sdo_cmd_state_t cmd_state;

void sdo_seq_init_layer(){
  memset(&seq_state, 0, sizeof(seq_state));
  memset(&cmd_state, 0, sizeof(cmd_state));
  for(unsigned i=0;i<TX_HISTORY_SIZE;i++)
    seq_state.history_buffer[i] = sdo_buf[i];
}

static void update_receive_sequence_numbers(){

// printf("update_receive_sequence_numbers\n");
//  printf("tx: %d %d %d %d\n", seq_state.rsnr, seq_state.rcon, seq_state.ssnr, seq_state.scon);
  assert(seq_state.history_buffer_fill != 0);
  unsigned index = seq_state.history_buffer_tail%TX_HISTORY_SIZE;
  uint8_t* next_sdo_to_be_issued = seq_state.history_buffer[index];
  next_sdo_to_be_issued[18] = (seq_state.rsnr<<2) + seq_state.rcon;
}

static void add_to_pres_queue(ASync_state * a, uintptr_t next_sdo_to_be_issued,
    unsigned num_bytes){
  unsigned pr_fill = a->PR_queue_fill_level[3];
  assert(pr_fill < 8);
  a->PR_queues_p[3][pr_fill] = next_sdo_to_be_issued;
  a->PR_queues_size[3][pr_fill] = num_bytes;
  a->PR_queue_fill_level[3]++;
}



static void issue_an_ack(ASync_state * a){
  //assert(seq_state.history_buffer_fill == 0);
  unsigned index = seq_state.history_buffer_tail%TX_HISTORY_SIZE;
  uint8_t* next_sdo_to_be_issued = seq_state.history_buffer[index];
  next_sdo_to_be_issued[18] = (seq_state.rsnr<<2) + seq_state.rcon;
  next_sdo_to_be_issued[19] = (seq_state.ssnr<<2) + seq_state.scon;

  seq_state.history_buffer_fill++;
  add_to_pres_queue(a, (uintptr_t)next_sdo_to_be_issued, 21);

  /*
  unsigned pr_fill = a->PR_queue_fill_level[3];
  assert(pr_fill < 8);


  a->PR_queues_p[3][pr_fill] = (uintptr_t)next_sdo_to_be_issued;
  a->PR_queues_size[3][pr_fill] = 21;//TODO check this
  a->PR_queue_fill_level[3]++;
*/
}

static void issue_sdo(ASync_state * a){

  unsigned fill = seq_state.history_buffer_fill;
  unsigned index = (seq_state.history_buffer_tail + fill)%TX_HISTORY_SIZE;

  uint8_t* next_sdo_to_be_issued = seq_state.history_buffer[index];

  seq_state.ssnr++;

  next_sdo_to_be_issued[18] = (seq_state.rsnr<<2) + seq_state.rcon;
  next_sdo_to_be_issued[19] = (seq_state.ssnr<<2) + seq_state.scon;

  unsigned pr_fill = a->PR_queue_fill_level[3];
  assert(pr_fill < 8);
  a->PR_queues_p[3][pr_fill] = (uintptr_t)next_sdo_to_be_issued;
  a->PR_queues_size[3][pr_fill] = seq_state.history_buffer_size[index];
  a->PR_queue_fill_level[3]++;

  seq_state.history_buffer_ssnr[index] = seq_state.ssnr;
  seq_state.history_buffer_fill++;
}

static void reissue(ASync_state * a, unsigned index){

  uint8_t* next_sdo_to_be_issued = seq_state.history_buffer[index];

  unsigned pr_fill = a->PR_queue_fill_level[3];
  assert(pr_fill < 8);


  a->PR_queues_p[3][pr_fill] = (uintptr_t)next_sdo_to_be_issued;
  a->PR_queues_size[3][pr_fill] = 21;//TODO check this
  a->PR_queue_fill_level[3]++;
}


//SDO_CmdLayerTimeout_U32
//D_SDO_SeqLayerTxHistorySize_U16
void sdo_timeout(){
  //the response we are waiting for didn't happen
//TODO ask for a new one
}
/*
 * We use this to inform the seq layer that a frame has been sent
 */
int inform_seq_layer(uintptr_t tx_frame){
  int timeout_active = FALSE;

  unsigned fill = seq_state.history_buffer_fill;
  unsigned tail = seq_state.history_buffer_tail;
  unsigned head = seq_state.history_buffer_tail + fill;


  for(unsigned i=tail; i<head; i++){
    unsigned index = i%TX_HISTORY_SIZE;
    //FIXME dont let the history buffer fill with too many acks!
    if(seq_state.history_buffer[index] == (uint8_t*)tx_frame){
      seq_state.actually_ssnr = seq_state.history_buffer_ssnr[index];

      //TODO if we just sent data then we are expecting an ack before the timeout
      break;
    }
  }
  return timeout_active;
}

int process_cmd_data(sdo_cmd_state_t * cls, unsigned tid_index, chanend c_can_open);
void get_response(sdo_cmd_state_t * cls, unsigned tid_index, unsigned response_number,
    uint8_t * sdo_buf, unsigned * sdo_size);

static int sdo_waiting_to_be_txd_in_nmt(){
  unsigned tail = seq_state.history_buffer_tail;
  unsigned index = tail%TX_HISTORY_SIZE;
  unsigned tail_ssnr = seq_state.history_buffer_ssnr[index];
  return tail_ssnr > seq_state.actually_ssnr;
}


void sdo_seq_recieved(
    Sequence_Layer_Protocol_t * s,
    Command_Layer_Protocol_t  * c,
    ASync_state * a, chanend c_can_open){
#ifdef PRINT_DEBUG
  printf("\n\nrx: %d %d %d %d\n", s->ReceiveSequenceNumber, s->ReceiveCon, s->SendSequenceNumber, s->SendCon);
#endif


  if(seq_state.scon == 0){
    if(s->SendCon == 0){
      //do nothing
      return;
    } else if(s->SendCon == 1){
      //we may not send data by must acknoledge the status change

      //set my status(send connection number) to 1
      //make up my send sequence number, set it to 0
      //set my recieved sequence number to s->SendSequenceNumber
      //set my recieved connection number to 1

      seq_state.scon = 1;
      seq_state.ssnr = 0;
      seq_state.rsnr = s->SendSequenceNumber;
      seq_state.rcon = 1;

      issue_an_ack(a);
      return;
    } else {
      // ignore it
#ifdef PRINT_DEBUG
      printf("ERROR: seq status from 0 -> 2 or 3\n");
#endif
    }
  } else if(seq_state.scon == 1){
    if(s->SendCon == 0){
      seq_state.scon = 0;
      return;
    } else if(s->SendCon == 1){
      seq_state.scon = 1;
      seq_state.ssnr = 0;
      seq_state.rsnr = s->SendSequenceNumber;
      seq_state.rcon = 1;

      issue_an_ack(a);
      return;
    } else if (s->SendCon == 2){
      seq_state.scon = 2;
      seq_state.ssnr = 0;
      seq_state.rcon = 2;
      if(seq_state.rsnr != s->SendSequenceNumber){
#ifdef PRINT_DEBUG
        printf("ERROR: rsnr may nor increase during init\n");
#endif
      }
      issue_an_ack(a);
      return;
    } else if (s->SendCon == 3){
#ifdef PRINT_DEBUG
        printf("ERROR: seq status from 1 -> 3\n");
#endif
        return;
    }
  }

  //we are in state 2

  if(s->SendCon == 0){
      seq_state.scon = 0;
      return;
    } else if(s->SendCon == 1){
#ifdef PRINT_DEBUG
        printf("ERROR: seq status from 2 -> 1\n");
#endif
      return;
    } else if(s->SendCon == 2){

      if(s->SendSequenceNumber == seq_state.rsnr){
        //this is an ack
        //nothing to do
      } else if(s->SendSequenceNumber == seq_state.rsnr + 1){
        //this is data that i want


        //pass the data to the cmd layer
        cmd_layer_recieve_data(c_can_open, c, &cmd_state);

        seq_state.rsnr = seq_state.rsnr + 1;

        //if there is an sdo waiting to be issued
        if(sdo_waiting_to_be_txd_in_nmt() > 0){
          //update it
          update_receive_sequence_numbers();
        } else {
          //issue data if it is waiting else just and ack
          if(seq_state.history_buffer_fill < TX_HISTORY_SIZE){
            if(cmd_layer_data_waiting(&cmd_state)){
              unsigned index = (seq_state.history_buffer_tail + seq_state.history_buffer_fill)%TX_HISTORY_SIZE;
              unsigned size = cmd_layer_append_data(seq_state.history_buffer[index], &cmd_state);
              seq_state.history_buffer_size[index] = size; //TODO can probably eliminate this
              issue_sdo(a);
            } else {
              issue_an_ack(a);
            }
          }
        }
      } else {
        seq_state.rcon = 3;
        //report that data is in the wrong order
        //if there is an sdo waiting to be issued
        if(sdo_waiting_to_be_txd_in_nmt() > 0){
          //update it
          update_receive_sequence_numbers();
        } else {
          //issue data if it is waiting else just and ack
          if(seq_state.history_buffer_fill < TX_HISTORY_SIZE){
            if(cmd_layer_data_waiting(&cmd_state)){
              unsigned index = (seq_state.history_buffer_tail + seq_state.history_buffer_fill)%TX_HISTORY_SIZE;
              unsigned size = cmd_layer_append_data(seq_state.history_buffer[index], &cmd_state);
              seq_state.history_buffer_size[index] = size; //TODO can probably eliminate this
              issue_sdo(a);
            } else {
              issue_an_ack(a);
            }
          }
        }
        seq_state.rcon = 2; //for safety
      }

      while(cmd_layer_data_waiting(&cmd_state) && seq_state.history_buffer_fill < TX_HISTORY_SIZE){
        unsigned index = (seq_state.history_buffer_tail + seq_state.history_buffer_fill)%TX_HISTORY_SIZE;
        unsigned size = cmd_layer_append_data(seq_state.history_buffer[index], &cmd_state);
        seq_state.history_buffer_size[index] = size; //TODO can probably eliminate this

        issue_sdo(a);
      }
    } else if(s->SendCon == 3){
      //check for data
    }

    unsigned ssnr_rxd_by_master = s->ReceiveSequenceNumber;

    if(ssnr_rxd_by_master <=  seq_state.actually_ssnr ){

      //release the ones that the master definatly has received
      unsigned fill = seq_state.history_buffer_fill;
      unsigned tail = seq_state.history_buffer_tail;
      unsigned head = seq_state.history_buffer_tail + fill;

      for(unsigned i=tail; i<head; i++){
        unsigned index = i%TX_HISTORY_SIZE;
        if(seq_state.history_buffer_ssnr[index] <= ssnr_rxd_by_master){
          tail++;
          fill--;
        }
      }

      for(unsigned i=tail; i< head; i++){
        unsigned index = i%TX_HISTORY_SIZE;
        //TODO maybe we should revoke any pending sdo packets?

        if(seq_state.history_buffer_ssnr[index] <= seq_state.actually_ssnr){
          //only reissue frames that have been sent
           reissue(a, index);
        }
      }

      seq_state.history_buffer_fill = fill;
      seq_state.history_buffer_tail = tail;
    }
}



