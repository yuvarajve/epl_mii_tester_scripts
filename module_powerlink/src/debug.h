#ifndef DEBUG_H_
#define DEBUG_H_

#define PRINT
#define ASSERT

#ifdef ASSERT
#include <assert.h>
#endif

#ifdef PRINT
#include <stdint.h>
#include <stdio.h>
#include <print.h>
#include "pl_defines.h"
void print(intptr_t pointer);
void print_nmt_state(nmt_state_t state);
#else
void print(intptr_t pointer){};
#endif

#define ADD_DEBUG(x) debug_reg = (debug_reg<<4)+x


typedef enum {
  SoC,//0
  SoA,//1
  ASnd,//2
  PReq,//3
  PRes//4
} debug_pl_type;

typedef enum {
  ID_REQed,
  ID_REQsent,
  MII_TX_REQ,
  SR_REQed,
  SR_REQsent,
  FrameRXed,
} debug_event;


#endif /* DEBUG_H_ */
