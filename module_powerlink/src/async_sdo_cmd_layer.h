#ifndef ASYNC_SDO_CMD_LAYER_H_
#define ASYNC_SDO_CMD_LAYER_H_

#include "pl_defines.h"
#include "frame.h"
#include <xccompat.h>

#define CONCURRENT_TID 1

typedef enum {
  NOT_IN_USE,
  IN_USE,
  DOWNLOAD_COMPLETE,
  UPLOAD_COMPLETE,
} t_transaction_status;

typedef struct {
  t_transaction_status transaction_status;
  unsigned TID;
  unsigned Segmented_Transfer;
  unsigned Command_ID ;
  unsigned Segment_Size;
  uint8_t data_buffer[2000];
  unsigned written_bytes;
} t_transaction;

typedef struct {
  unsigned active_transactions;
  t_transaction t[CONCURRENT_TID];
} sdo_cmd_state;

void sdo_cmd_init_layer(REFERENCE_PARAM(sdo_cmd_state, state));

#ifndef __XC__
void sdo_cmd_recieved(REFERENCE_PARAM(Command_Layer_Protocol_t, cmd), REFERENCE_PARAM(sdo_cmd_state, state));
#endif
#endif /* ASYNC_SDO_CMD_LAYER_H_ */
