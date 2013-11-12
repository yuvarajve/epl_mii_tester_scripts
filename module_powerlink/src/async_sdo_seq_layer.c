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

#define NODE_ID 1
static uint8_t sdo_buf[1522] = {
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_MN_DEF_NODE_ID,
    NODE_ID,
    SDO,


};

static uint8_t * get_sdo_response_pointer(){
  return sdo_buf;
}

enum {
  PRIO_GENERIC_REQUEST = 0x3,
  PRIO_NMT_REQUEST = 0x7

};

static void make_reply(unsigned rsnr, unsigned rcon, unsigned ssnr, unsigned scon){
  sdo_buf[18] = (rsnr<<2) + rcon;
  sdo_buf[19] = (ssnr<<2) + scon;
}


void sdo_seq_init_layer(sdo_seq_state * state){

}

static void respond(unsigned rsnr, unsigned rcon, unsigned ssnr, unsigned scon){

}

void sdo_seq_recieved(
    Sequence_Layer_Protocol_t * s, sdo_seq_state * seq_state,
    Command_Layer_Protocol_t  * c, sdo_cmd_state * cmd_state){

   unsigned rxd_scon = s->SendCon;
   unsigned rxd_ssnr = s->SendSequenceNumber;
   unsigned rxd_rcon = s->ReceiveCon;
   unsigned rxd_rsnr = s->ReceiveSequenceNumber;

   switch(seq_state->scon){
   case 0:{
     switch(s->SendCon){
     case 0: {
       //do nothing
       break;
     }
     case 1:{
       seq_state->tx_request = 1;
       seq_state->ssnr = 0;
       seq_state->scon = 1;
       seq_state->rsnr = s->SendSequenceNumber;
       seq_state->rcon = s->SendCon;
       respond(s->SendSequenceNumber, s->SendCon, seq_state->ssnr, seq_state->scon);
     }
     case 2:
     case 3:{
       //FIXME This is invalid - report an error?
     }
     }

     break;
   }
   case 1:{
     //verify that the rcon and rsnr are correct
     if(seq_state->ssnr != s->ReceiveSequenceNumber || seq_state->scon != s->ReceiveCon){
       //respond with the last good reception
       respond(s->SendSequenceNumber, s->SendCon, seq_state->ssnr, seq_state->scon);
       break;
     }

     switch(s->SendCon){
     case 0: {
       //close the connection
       assert(s->ReceiveCon == 0);
       seq_state->scon = 0;
     }
     case 1:{
       //FIXME What do I do here?
     }
     case 3:
     case 2:{
       seq_state->tx_request = 1;
       seq_state->scon = 2;
       respond(s->SendSequenceNumber, s->SendCon, seq_state->ssnr, seq_state->scon);
     }
     }
     break;
   }
   case 2:
   case 3:{
     //verify that the rcon and rsnr are correct
     if(seq_state->ssnr != s->ReceiveSequenceNumber || seq_state->scon != s->ReceiveCon){
       //respond with the last good reception
       respond(s->SendSequenceNumber, s->SendCon, seq_state->ssnr, seq_state->scon);
       break;
     }
     switch(s->SendCon){
     case 0: {
       //close the connection
       assert(s->ReceiveCon == 0);
       seq_state->scon = 0;
     }
     case 1:{
       //FIXME What do I do here?
     }
     case 3:
     case 2:{
       unsigned t_index;
       unsigned cmd_tx_request = FALSE;
       for(t_index = 0;t_index<CONCURRENT_TID;t_index++){
         if(cmd_state->t[t_index].transaction_status == DOWNLOAD_COMPLETE ||
             cmd_state->t[t_index].transaction_status == UPLOAD_COMPLETE){
           cmd_tx_request = TRUE;
           break;
         }
       }

       if(cmd_tx_request == TRUE &&
           seq_state->tx_request == FALSE){
         seq_state->ssnr++;
         //fill in the data that i want to send
         //insert_cmd_in_pending_sdo_buffer(seq_state, t_index, cmd_state);
         cmd_state->t[t_index].transaction_status = NOT_IN_USE;
         seq_state->tx_request = TRUE;
       }
       //seq_state->tx_request = 1;
       if(s->SendSequenceNumber == seq_state->rsnr + 1){
         seq_state->rsnr++;
         //TODO pass the data to the next layer
         //sdo_cmd_recieved(c, seq_state);
       } else if(s->SendSequenceNumber == seq_state->rsnr){
         //drop this frame
       } else {
         //this is an error - a frame was lost
         respond(seq_state->rsnr, 3, seq_state->ssnr, seq_state->scon);
         break;
       }
       respond(seq_state->rsnr, 2, seq_state->ssnr, seq_state->scon);
     }
     }
     break;
   }
   }

   //SDO_CmdLayerTimeout_U32

  //D_SDO_SeqLayerTxHistorySize_U16
}
