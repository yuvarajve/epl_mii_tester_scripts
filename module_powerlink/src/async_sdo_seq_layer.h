#ifndef ASYNC_SDO_SEQ_LAYER_H_
#define ASYNC_SDO_SEQ_LAYER_H_
#include "frame.h"
#include "pl_nmt.h"



#ifndef __XC__
void sdo_seq_recieved(
    Sequence_Layer_Protocol_t * s,
    Command_Layer_Protocol_t * c,
    ASync_state * async_state,
    chanend c_can_open
);
#endif

void sdo_seq_init_layer();

#endif /* ASYNC_SDO_SEQ_LAYER_H_ */
