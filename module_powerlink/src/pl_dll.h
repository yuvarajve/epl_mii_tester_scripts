#ifndef PL_DLL_H_
#define PL_DLL_H_

#include <xccompat.h>
#include <stdint.h>
#include "pl_defines.h"

#include "device_description.h"

typedef struct t_frame_SoC {
  unsigned char Multiplexed_Cycle_Completed;
  unsigned char Prescaled_Slot;
#if D_NMT_NetTime_BOOL
  unsigned long long NetTime;
#endif
#if D_NMT_RelativeTime_BOOL
  unsigned long long RelativeTime;
#endif
} t_frame_SoC;

void process_SoC(uintptr_t rx_buffer_address, chanend c_can_open,
    REFERENCE_PARAM(t_frame_SoC, latest_SoC));
void process_PReq(uintptr_t rx_buffer_address, chanend c_can_open,
    unsigned char node_id);
void process_PRes(uintptr_t rx_buffer_address, chanend c_can_open,
    unsigned char node_id);

void build_next_PRes(REFERENCE_PARAM(uintptr_t, next_PRes_tx_pointer),
    REFERENCE_PARAM(unsigned, next_PRes_size));

Message_Type_ID get_powerlink_type(uintptr_t pointer);

#endif /* PL_DLL_H_ */
