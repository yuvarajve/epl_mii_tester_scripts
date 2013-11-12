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
#include "frame.h"
#include <assert.h>


void sdo_cmd_init_layer(sdo_cmd_state * state){
  state->active_transactions = FALSE;

}



static void process_command(sdo_cmd_state * cls, unsigned tid_index){
  t_transaction * t = &(cls->t[tid_index]);

  switch(t->Command_ID){
  case NIL :{
    break;
  }
  case Read_by_Index :{
    //upload protocol

    //read data from can open into data_buffer
    //TODO

    t->transaction_status = UPLOAD_COMPLETE;
    break;
  }
  case Write_by_Index :{
    //download protocol

    //write data from data_buffer in to canopen
    //TODO

    t->transaction_status = DOWNLOAD_COMPLETE;
  }
  case Write_All_by_Index :{
    break;
  }
  case Read_All_by_Index: {
    break;
  }
  default :{
    //unreachable();
  }
}
}


static unsigned cmd_specific_header_size(Command_Layer_Protocol_t *c){
  unsigned payload_bytes = 0;
  switch(c->Command_ID){
    case NIL :{
      break;
    }
    case Read_by_Index :
    case Write_by_Index :
    case Write_All_by_Index :
    case Read_All_by_Index: {
      payload_bytes -= 4;
      break;
    }
    default :{
      //assert( 0 & "unsupported sdo cmd");
    }
  }
  return payload_bytes;
}

#define DATA_SIZE_BYTES 4

static void commit_data_to_memory(Command_Layer_Protocol_t *c, sdo_cmd_state * cls, unsigned tid_index){

  switch(c->Segmentation){
  case Segm_Transfer_Complete :
  case Segment :{
    unsigned payload_bytes = c->Segment_Size;
    unsigned byte_offset = cls->t[tid_index].written_bytes;
    for(unsigned i=0;i<payload_bytes;i++)
      cls->t[tid_index].data_buffer[i + byte_offset] = c->cmd_init.payload[i];
    cls->t[tid_index].written_bytes += payload_bytes;
    break;
  }
  case Initiate_Segm_Transfer :{
    unsigned payload_bytes = c->Segment_Size-DATA_SIZE_BYTES;
    for(unsigned i=0;i<payload_bytes;i++)
      cls->t[tid_index].data_buffer[i] = c->cmd_init.payload[i];
    cls->t[tid_index].written_bytes = payload_bytes;
    break;
  }
  case Expedited_Transfer :{
    unsigned payload_bytes = c->Segment_Size - cmd_specific_header_size(c);
    for(unsigned i=0;i<payload_bytes;i++)
      cls->t[tid_index].data_buffer[i] = c->cmd_no_init.payload[i];
    cls->t[tid_index].written_bytes = payload_bytes;
    break;
  }
  default:{
    //unreachable();
  }
  }
}


void sdo_cmd_recieved(Command_Layer_Protocol_t * cmd, sdo_cmd_state * cls){

  if(cmd->Abort){
    unsigned t_index;
     //this needs a new tid_index
     for(t_index=0;t_index<CONCURRENT_TID;t_index++){
       if(cls->t[t_index].transaction_status == NOT_IN_USE)
         break;
     }
     if(t_index == CONCURRENT_TID){
       //we have reached out limit and need to reject the command
       return;
     }
     cls->t[t_index].transaction_status = NOT_IN_USE;

     //TODO
     //abort code is in the payload
  }


   if(cmd->Segmentation == Expedited_Transfer){
     unsigned t_index;
     //this needs a new tid_index
     for(t_index=0;t_index<CONCURRENT_TID;t_index++){
       if(cls->t[t_index].transaction_status == NOT_IN_USE)
         break;
     }
     if(t_index == CONCURRENT_TID){
       //we have reached out limit and need to reject the command
       return;
     }

     process_command(cls, t_index);
     return;
   }

   if(cmd->Segmentation == Initiate_Segm_Transfer){
     unsigned t_index;
     //this needs a new tid_index
     for(t_index=0;t_index<CONCURRENT_TID;t_index++){
       if(cls->t[t_index].transaction_status == NOT_IN_USE)
         break;
     }
     if(t_index == CONCURRENT_TID){
       //we have reached out limit and need to reject the command
       return;
     }
     cls->t[t_index].transaction_status = IN_USE;
     cls->active_transactions++;
     commit_data_to_memory(cmd, cls, t_index);
   } else {

     if(cls->active_transactions){
       unsigned t_index;
       for(t_index = 0;t_index<CONCURRENT_TID;t_index++){
         if(cls->t[t_index].TID == cmd->Transaction_ID)
           break;
       }
       assert(t_index != CONCURRENT_TID);

       //this is a segmented transfer
       if(cmd->Segmentation == Segment){
         commit_data_to_memory(cmd, cls, t_index);

       } else if(cmd->Segmentation == Segm_Transfer_Complete){
         commit_data_to_memory(cmd, cls, t_index);
         process_command(cls, t_index);
       } else {
         //unreachable()
       }
     } else {
       //what is going on!!?
       //FIXME - check into how we react to a broken cmd
     }
   }
}
