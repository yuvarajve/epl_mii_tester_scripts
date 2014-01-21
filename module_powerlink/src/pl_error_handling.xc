#include "pl_error_defines.h"
#include "device_description.h"
#include "pl_defines.h"
#include <stdint.h>
#include <assert.h>
/*
 * Error Signalling
 */


#define NODE_ID 1

typedef struct {
  uint16_t type;

  uint16_t STATUS_BITS;
  //uint16_t status:1, send:1, mode:2, profile:12;

  uint16_t code;
  uint64_t time_stamp;
  uint8_t additional_data[8];
} ErrorEntry_DOM;

#define GET_STATUS(x) ((x.STATUS_BITS)&1)
#define GET_SEND(x) (((x.STATUS_BITS)>>1)&1)
#define GET_MODE(x) (((x.STATUS_BITS)>>2)&3)
#define GET_PROFILE(x) (((x.STATUS_BITS)>>4))


/*
After the CN has indicated new StatusResponse data to the MN by toggling the EN flag, the
StatusResponse frame on the CN may not be changed anymore as it can be requested by the
MN at any time. Only if the reception of the response is acknowledged by the EA flag of the MN,
the CN may change the StatusResponse frame data again.
*/

void make_error_entry(ErrorEntry_DOM &error_entry){
  //status
  //send
  //mode
  //profile


  //Static Error Bit Field
  //Content of ERR_ErrorRegister_U8
  //Reserved
  //Device profile or vendor specific errors



  //The object ERR_ErrorRegister_U8 is compatible to the object 'error register' of the standard
  //communication profile CiA DS 301.

}

#define ERROR_ENTRY_SIZE_IN_BYTES 20

void tx_error_entry(chanend c_eh){
  ErrorEntry_DOM error_entry;
 // make_error_entry(error_entry);
  master {
    for(unsigned i=0;i<ERROR_ENTRY_SIZE_IN_BYTES/4;i++)
      c_eh <:(error_entry, unsigned[])[i];
  }
}


static void rx_error_entry(chanend c_eh, ErrorEntry_DOM &error_entry){
  slave {
    for(unsigned i=0;i<ERROR_ENTRY_SIZE_IN_BYTES/4;i++)
      c_eh :> (error_entry, unsigned[])[i];
  }
}

//D_NMT_EmergencyQueueSize_U32

int error_signalling_data_present;


typedef struct {
  unsigned current_srb_index;
  unsigned current_srb_entries;
  uint8_t static_error_bit_field; //this is a mirror of Object 1001h
} eh_status;

#define STATUS_RESPONSE_BUFFER_SIZE_BYTES (32 + D_NMT_ErrorEntries_U32*20)

static uint8_t status_response_buf[2][STATUS_RESPONSE_BUFFER_SIZE_BYTES] = {
    {
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_BROADCAST,
    NODE_ID,
    STATUS_RESPONSE,
    0x00,//EN + EC
    0x00,//PR + RS
    0x00,//NMTStatus
    0x00,0x00,0x00,//Reserved
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//StaticErrorBitField
    //Then there are D_NMT_ErrorEntries_U32 20 byte error entries
    },
    {
    0x01, 0x11, 0x1E, 0x00, 0x00, 0x04, //dst_mac
    MAC_0, MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, //src_mac
    0x88, 0xAB, //ethertype
    Asynchronous_Send,
    C_ADR_BROADCAST,
    NODE_ID,
    STATUS_RESPONSE,
    0x00,//EN + EC
    0x00,//PR + RS
    0x00,//NMTStatus
    0x00,0x00,0x00,//Reserved
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//StaticErrorBitField
    //Then there are D_NMT_ErrorEntries_U32 20 byte error entries
    }
};

static uintptr_t get_status_response_pointer(eh_status &s){
  uintptr_t p;
  asm("mov %0, %1": "=r"(p):"r"(status_response_buf[s.current_srb_index]));
  return p;
}
static void insert_static_error_bit_field(eh_status &s){
  //Object 1001h : ERR_ErrorRegister_U8
  status_response_buf[s.current_srb_index][22] = s.static_error_bit_field;
}

static void terminate_status_response_buffer(eh_status &s){
  if(s.current_srb_index == D_NMT_ErrorEntries_U32)
    return;
  for(unsigned i=0;i<20;i++)
    status_response_buf[s.current_srb_index][31+20*s.current_srb_entries] = 0;
}

static void switch_status_response_buffer(eh_status &s){
  s.current_srb_index = 1 - s.current_srb_index;
}

static unsigned get_status_response_size(eh_status &s){
  return s.current_srb_entries*20 + 32;
}

static void insert_in_status_response_frame(ErrorEntry_DOM latest_error_entry,
    eh_status s){
  for(unsigned i=0;i<20;i++)
    status_response_buf[s.current_srb_index][i+32] = (latest_error_entry, uint8_t[])[i];
}

void pl_error_handling(chanend c_mii, chanend c_dll, chanend c_nmt){
  ErrorEntry_DOM latest_error_entry;
  error_signalling_data_present = 0;
  eh_status s;
  s.current_srb_index = 0;
  s.current_srb_entries = 2;

  while(1){
    select {
      case c_nmt :> unsigned cmd: {

        switch(cmd){
#if 0
        case signal_error : {
          rx_error_entry(c_mii, latest_error_entry);
          //process the entry
          //FIXME what do I do with mode and profile?

          if(GET_STATUS(latest_error_entry)){
            //Status Entry in StatusResponse frame (Bit 14 shall be set to 0b)
            assert(GET_SEND(latest_error_entry)==0);

            /*
             * the frame being inserted has to have a mode of 1
             */
            //insert_in_status_response_frame(latest_error_entry);
          } else {
            //ERR_History_ADOM Entry
           // insert_in_ERR_History(latest_error_entry); //uses can open
            if(GET_SEND(latest_error_entry)){
              //Additional to the ERR_History_ADOM the entry shall also be entered in to
              //the Emergency Queue of the Error Signaling.
              //insert_in_Emergency_Queue(latest_error_entry); //uses can open
            }
          }
          break;
        }
#endif
        case request_status_response: {
          /*
           * This sends a pointer and size to the requester. It points to
           * a STATUS_RESPONSE the requires the flags to be filled in before issue.
           */
       //   assert(error_signalling_data_present);
          //fill in Static Error Bit Field

         // insert_static_error_bit_field();

          /*
           * This returns the current status response buffer pointer, after this no
           * more can be written to it.
           */
          c_nmt <: get_status_response_pointer(s);

          /*
           * This sets the mode of the latest error entry to 0
           */
         // terminate_status_response_buffer(s);
          /*
           * This returns the size of the current status response buffer.
           */
          c_nmt <: get_status_response_size(s);
          switch_status_response_buffer(s);
          error_signalling_data_present = FALSE;
          break;
        }
      }
      break;
    }
    }
  }
}
