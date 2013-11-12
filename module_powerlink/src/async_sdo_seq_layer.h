#ifndef ASYNC_SDO_SEQ_LAYER_H_
#define ASYNC_SDO_SEQ_LAYER_H_
#include <stdint.h>
#include <xccompat.h>
#include "frame.h"
#include "async_sdo_cmd_layer.h"

typedef struct {
  unsigned rsnr, rcon, ssnr, scon, tx_request, invited;

} sdo_seq_state;

void sdo_seq_init_layer(REFERENCE_PARAM(sdo_seq_state,state));
#ifndef __XC__
void sdo_seq_recieved(
    REFERENCE_PARAM(Sequence_Layer_Protocol_t, s),
    REFERENCE_PARAM(sdo_seq_state, seq_state),
    REFERENCE_PARAM(Command_Layer_Protocol_t, c),
    REFERENCE_PARAM(sdo_cmd_state, cmd_state)
);
#endif
#endif /* ASYNC_SDO_SEQ_LAYER_H_ */
