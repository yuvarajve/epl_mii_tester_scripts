/**
 * The copyrights, all other intellectual and industrial
 * property rights are retained by XMOS and/or its licensors.
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2012
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the
 * copyright notice above.
 *
 **/

#ifndef __common_h__
#define __common_h__

/*---------------------------------------------------------------------------
 constants
 ---------------------------------------------------------------------------*/

#define NODE_ID 0
#define NUMBER_OF_PDOS_SUPPORTED 4
#define MAX_DATA_BUFFER_LENGTH 100
#define BIT_TIME_TABLE_LENGTH 9

enum access
{
  RO = 0,
  WO = 1,
  RW = 2,
  RWR = 3,
  RWW = 4,
  CONST = 5
};

#endif
