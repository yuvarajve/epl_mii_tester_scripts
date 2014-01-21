/*
 * This exists to manage the "Asynchronous SDO Command Layer".
 *
 * Tasks of the POWERLINK Command Layer
 *  1. Addressing of the parameters, e.g. via index/sub-index or via name
 *  2. Provide additional services
 *  3. Distinguish between expedited and segmented transfer
 *
 * The POWERLINK Command Layer is embedded in the POWERLINK Sequence Layer.
 * If a large block is to be transferred the POWERLINK Command Layer has
 * to decide whether the transfer can be completed in one frame
 * (expedited transfer) or if it must be segmented in several frames (segmented
 * transfer). Further it has to know whether an Upload or a Download should
 * be initiated. For all transfer types it is the client that takes the
 * initiative for a transfer. The owner of the accessed object dictionary
 * is the server of the Service Data Object (SDO). Either the client or the
 * server can take the initiative to abort the transfer of a SDO. All commands
 * are confirmed. The remote result parameter indicates the success of the
 * request. In case of a failure, an Abort Transfer Request must be executed.
 *
 */

//I think that the CN(me) is the server

//Note: The Download Protocol is used for write commands and the
//Upload Protocol is used for read commands.

/*
 * Questions:
 * When is the data written to the object dictionary? i.e. in segmented
 * transfer does it wait until all segments have been received?
 *
 *
 */
#include "async_sdo_cmd_layer.h"
#include "pl_error_defines.h"
#include "can_open_interface.h"
#include "frame.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

void sdo_cmd_init_layer(sdo_cmd_state_t * state){
  for(unsigned i=0;i<CONCURRENT_TID;i++)
    state->t[i].transaction_status = NOT_IN_USE;
}

/*
 * This will select the correct response type depending on the ammount of
 * data you wish to transfer.
 */
static void make_transfer_response(transaction_t * t){
  //check that the transfer_bytes is valid
  if(t->transfer_bytes > MAX_SEGMENT_SIZE){
    t->resp_type = INITIAL_DOMAIN_TRANSFER;
  } else {
    t->resp_type = EXPEDIATED_TRANSFER_RESPONSE;
  }
  t->resp_bytes_issued = 0;
}

static unsigned convert_to_pl_error_code(co_error_code_t reason){
  return 0x6020000;
  //TODO
}

static void make_abort(transaction_t * t, co_error_code_t reason){
  unsigned error_code = convert_to_pl_error_code(reason);
  t->resp_type = ABORT;
  memcpy(t->data_buffer, &error_code, 4);
}

/*
 * The purpose of this function is to prepare the data. if it is a download command the data is in the
 * data_buffer. if it is an upload command the data is fetched into the data_buffer
 */
static void process_command(transaction_t * t, chanend c_can_open){

#if 0
  switch(t->Command_ID){
  case NIL :{
    break;
  }
  case Read_by_Index :{
    uint16_t index = ((uint16_t*)t->data_buffer)[0];
    uint8_t subindex = t->data_buffer[2];
    printf("Read by Index 0x%04x:%d\n", index, subindex);
    printf("Data size: %d bytes\n", t->Data_Size);
    printf("transfer bytes: %d bytes\n", t->transfer_bytes);
    for(unsigned i=4;i<t->transfer_bytes;i++)
      printf("0x%02x\n", (t->data_buffer)[i]);
    break;
  }
  case Write_by_Index :{
    uint16_t index = ((uint16_t*)t->data_buffer)[0];
    uint8_t subindex = t->data_buffer[2];
    printf("Write by Index 0x%04x:%d\n", index, subindex);
    printf("Data size: %d bytes\n", t->Data_Size);
    printf("transfer bytes: %d bytes\n", t->transfer_bytes);
    printf("number of bytes: %d bytes\n", t->transfer_bytes-4);
    for(unsigned i=4;i<t->transfer_bytes;i++)
      printf("0x%02x\n", (t->data_buffer)[i]);

    if((index == 0x1600 || index == 0x1a00) && subindex != 0){
      uint16_t rx_index = ((uint16_t*)(t->data_buffer))[2];
      uint8_t rx_subindex = ((uint8_t*)(t->data_buffer))[6];
      uint16_t rx_offset = ((uint16_t*)(t->data_buffer))[4];
      uint16_t rx_length = ((uint16_t*)(t->data_buffer))[5];
      printf("Index:0x%04x Subindex:0x%02x Offset:0x%04x Length0x%04x\n", rx_index, rx_subindex, rx_offset, rx_length);
    }

    break;
  }
  default:{
    printf("Unsupported CMD\n");
  }
  }
#endif

  t->transaction_status = CMD_RECEIVED;

  switch(t->Command_ID){
  case NIL :{
    break;
  }
  case Read_by_Index :{
    uint16_t index = ((uint16_t*)t->data_buffer)[0];
    uint8_t subindex = t->data_buffer[2];
    unsigned number_of_bytes = MAX_DATA_SIZE;
    co_error_code_t err = co_read(c_can_open, index, subindex,
        t->data_buffer, &number_of_bytes);
    if(err == e_success){
      t->transfer_bytes = number_of_bytes;
      make_transfer_response(t);
    } else {
      make_abort(t, err);
    }
    break;
  }
  case Write_by_Index :{
    uint16_t index = ((uint16_t*)t->data_buffer)[0];
    uint8_t subindex = t->data_buffer[2];
    unsigned number_of_bytes = t->transfer_bytes - 4; //cmd specific header
    co_error_code_t err = co_write(c_can_open, index, subindex,
        t->data_buffer+4, number_of_bytes);
    if(err == e_success){
      t->transfer_bytes = 0;
      make_transfer_response(t);
    } else {
      make_abort(t, err);
    }
    break;
  }
  default:{
    break;
  }
  }
}

/*
 * This copies the data from the frames into a buffer to be stored until the command is complete
 */
static void commit_data_to_memory(Command_Layer_Protocol_t * c, transaction_t * t){
  switch(c->Segmentation){
  case Segm_Transfer_Complete :
  case Segment :{
    memcpy(t->data_buffer + t->transfer_bytes, c->cmd_no_init.payload, c->Segment_Size);
    t->transfer_bytes += c->Segment_Size;
    break;
  }
  case Initiate_Segm_Transfer :{
    memcpy(t->data_buffer, c->cmd_init.payload, c->Segment_Size);
    t->transfer_bytes =c->Segment_Size;
    break;
  }
  case Expedited_Transfer :{
    memcpy(t->data_buffer, c->cmd_no_init.payload, c->Segment_Size);
    t->transfer_bytes = c->Segment_Size;
    break;
  }
  default:{
   //_builtin_unreachable();
  }
  }
}

void cmd_layer_recieve_data(chanend c_can_open, Command_Layer_Protocol_t * cmd, sdo_cmd_state_t * cmd_state){

  if(cmd->Abort){
    //aborts are unconfirmed
    unsigned t_index;
    for(t_index=0;t_index<CONCURRENT_TID;t_index++){
      if(cmd->Transaction_ID == cmd_state->t[t_index].TID){
        cmd_state->t[t_index].transaction_status = NOT_IN_USE;
        return;
      }
    }
    printf("ERROR: aborting a unknown cmd\n");
    return;
  }

  switch(cmd->Segmentation){
  case Expedited_Transfer:{
    //this is the start of a new command
    unsigned t_index;
    transaction_t * t;
    //this needs a new tid_index
    for(t_index=0;t_index<CONCURRENT_TID;t_index++){
     t = &(cmd_state->t[t_index]);
     if(t->transaction_status == NOT_IN_USE)
       break;
    }
    if(t_index == CONCURRENT_TID){
     //we have reached out limit and need to reject the command
     printf("ERROR: too many SDO TIDs\n");
     return;
    }
    assert(cmd->Response == 0);
    commit_data_to_memory(cmd, t);
    t->Command_ID = cmd->Command_ID;
    t->TID = cmd->Transaction_ID;
    t->Data_Size = cmd->Segment_Size;
    process_command(t, c_can_open);
    return;
  }
  case Initiate_Segm_Transfer:{
    //this is the start of a new command
    unsigned t_index;
    transaction_t * t;
    //this needs a new tid_index
    for(t_index=0;t_index<CONCURRENT_TID;t_index++){
     t = &(cmd_state->t[t_index]);
     if(t->transaction_status == NOT_IN_USE)
       break;
    }
    if(t_index == CONCURRENT_TID){
     //we have reached out limit and need to reject the command
     printf("ERROR: too many SDO TIDs\n");
     return;
    }
    t->Command_ID = cmd->Command_ID;
    t->TID = cmd->Transaction_ID;
    t->Data_Size = cmd->cmd_init.DataSize;
    t->transaction_status = IN_USE;
    commit_data_to_memory(cmd, t);
  }
  case Segment:
  case Segm_Transfer_Complete:{
    transaction_t * t = 0;
    for(unsigned t_index=0;t_index<CONCURRENT_TID;t_index++){
      t = &(cmd_state->t[t_index]);
      if(cmd->Transaction_ID == t->TID)
        break;
    }
    if(t == 0){
      printf("ERROR: invalid transaction cmd id\n");
      break;
    }
    //TODO we could check all the fields are correct
    commit_data_to_memory(cmd, t);
    if(cmd->Segmentation == Segm_Transfer_Complete){
      //TODO We could check that the data size is the transfered size
      process_command(t, c_can_open);
    }
    break;
  }
  default:{
    //builtin_unreachable();
    break;
  }
  }
  return;
}

int cmd_layer_data_waiting(sdo_cmd_state_t * cmd_state){
  for(unsigned i=0;i<CONCURRENT_TID;i++){
    if(cmd_state->t[i].transaction_status!= NOT_IN_USE)
      return 1;
  }
  return 0;
}

unsigned cmd_layer_append_data(uint8_t * sdo_buf, sdo_cmd_state_t * cmd_state){
  for(unsigned i=0;i<CONCURRENT_TID;i++){
    transaction_t * t = &(cmd_state->t[i]);

    frame * f = (frame*)sdo_buf ;
    Command_Layer_Protocol_t * c = &(f->pl_frame.asnd.sdo.clp);

    switch(t->resp_type){
      case ABORT :{
        c->Transaction_ID = t->TID;
        c->Segmentation = Expedited_Transfer;
        c->Abort = 1;
        c->Response = 1;
        c->Command_ID = t->Command_ID; // or 0
        c->Segment_Size = 4;
        for(unsigned i=0;i<4;i++)
          c->cmd_no_init.payload[i] = t->data_buffer[i];
        t->transaction_status = NOT_IN_USE;
        return PAYLOAD_OFFSET + 4; //the number of bytes to transfer
      }
      case EXPEDIATED_TRANSFER_RESPONSE: {
        c->Transaction_ID = t->TID;
        c->Segmentation = Expedited_Transfer;
        c->Abort = 0;
        c->Response = 1;
        c->Command_ID = t->Command_ID;
        c->Segment_Size = t->transfer_bytes;
        memcpy(c->cmd_no_init.payload, t->data_buffer, t->transfer_bytes);
        t->transaction_status = NOT_IN_USE;
        return PAYLOAD_OFFSET + t->transfer_bytes;
      }

      case INITIAL_DOMAIN_TRANSFER: {
        c->Transaction_ID = t->TID;
        c->Segmentation = Initiate_Segm_Transfer;
        c->Abort = 0;
        c->Response = 1;
        c->Command_ID = t->Command_ID;
        c->Segment_Size = MAX_SEGMENT_SIZE;

        memcpy(c->cmd_no_init.payload, &(t->transfer_bytes), 4);
        memcpy(c->cmd_no_init.payload+4, t->data_buffer, MAX_SEGMENT_SIZE-4);

        t->resp_bytes_issued = MAX_SEGMENT_SIZE - 4;
        if(t->transfer_bytes - t->resp_bytes_issued < MAX_SEGMENT_SIZE)
           t->resp_type = DOMAIN_TRANSFER_COMPLETE;
        else
          t->resp_type = SEGMENT_TRANSFER;

        return MAX_SEGMENT_SIZE + PAYLOAD_OFFSET;
      }

      case SEGMENT_TRANSFER: {
        c->Transaction_ID = t->TID;
        c->Segmentation = Segment;
        c->Abort = 0;
        c->Response = 1;
        c->Command_ID = t->Command_ID;
        c->Segment_Size = MAX_SEGMENT_SIZE;

        memcpy(c->cmd_no_init.payload, t->data_buffer, MAX_SEGMENT_SIZE);

        t->resp_bytes_issued += MAX_SEGMENT_SIZE;
        if(t->transfer_bytes - t->resp_bytes_issued < MAX_SEGMENT_SIZE)
          t->resp_type = DOMAIN_TRANSFER_COMPLETE;

        return MAX_SEGMENT_SIZE + PAYLOAD_OFFSET;
      }

      case DOMAIN_TRANSFER_COMPLETE: {
        c->Transaction_ID = t->TID;
        c->Segmentation = Segm_Transfer_Complete;
        c->Abort = 0;
        c->Response = 1;
        c->Command_ID = t->Command_ID;
        unsigned transfer_size = t->transfer_bytes - t->resp_bytes_issued;
        c->Segment_Size = transfer_size;
        memcpy(c->cmd_no_init.payload, t->data_buffer + t->resp_bytes_issued,
            transfer_size);
        t->resp_bytes_issued += transfer_size;
        t->transaction_status = NOT_IN_USE;
        return PAYLOAD_OFFSET + transfer_size;
      }
    }
  }
  return 0;
}
