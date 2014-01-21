#ifndef ASYNC_SDO_CMD_LAYER_H_
#define ASYNC_SDO_CMD_LAYER_H_

#include "pl_defines.h"
#include "frame.h"
#include <xccompat.h>

#define CONCURRENT_TID 1

typedef enum {
  //Not in use - transaction free to be assigned
  NOT_IN_USE,

  //Currently in use by a transaction
  IN_USE,

  //Command received from the client
  CMD_RECEIVED

} transaction_status_t;

typedef enum {
  EXPEDIATED_TRANSFER_RESPONSE,
  INITIAL_DOMAIN_TRANSFER,
  SEGMENT_TRANSFER,
  DOMAIN_TRANSFER_COMPLETE,
  ABORT
} response_type_t;

#define PAYLOAD_OFFSET 30
#define MAX_DATA_SIZE 2000
#define MAX_SEGMENT_SIZE 1458

typedef struct {
  transaction_status_t transaction_status;
  unsigned TID;
  unsigned Segmented_Transfer;
  unsigned Command_ID ;
  unsigned Data_Size; //this is the size of the whole transfer

  uint8_t data_buffer[MAX_DATA_SIZE];
  unsigned transfer_bytes;

  response_type_t resp_type;
  unsigned resp_bytes_issued;

} transaction_t;

typedef struct {
  transaction_t t[CONCURRENT_TID];
} sdo_cmd_state_t;

void sdo_cmd_init_layer(sdo_cmd_state_t * state);
void cmd_layer_recieve_data(chanend  c_can_open, Command_Layer_Protocol_t * c, sdo_cmd_state_t * cmd_state);
int cmd_layer_data_waiting(sdo_cmd_state_t * cmd_state);
unsigned  cmd_layer_append_data(uint8_t * buffer, sdo_cmd_state_t * cmd_state);

#endif /* ASYNC_SDO_CMD_LAYER_H_ */
