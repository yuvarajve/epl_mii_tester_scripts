#ifndef CAN_OPEN_INTERFACE_H_
#define CAN_OPEN_INTERFACE_H_

#include <stdint.h>
#include "can_open_od.h"
#include <xccompat.h>

typedef enum {
  //fixed size
  Boolean = 0x0001,
  Integer8 = 0x0002,
  Integer16 = 0x0003,
  Integer32 = 0x0004,
  Unsigned8 = 0x0005,
  Unsigned16 = 0x0006,
  Unsigned32 = 0x0007,
  Real32 = 0x0008,
  Integer24 = 0x0010,
  Real64 = 0x0011,
  Integer40 = 0x0012,
  Integer48 = 0x0013,
  Integer56 = 0x0014,
  Integer64 = 0x0015,
  Time_of_Day = 0x000C,
  Time_Diff = 0x000D,
  Domain = 0x000F,
  Unsigned24 = 0x0016,
  Unsigned40 = 0x0018,
  Unsigned48 = 0x0019,
  Unsigned56 = 0x001A,
  Unsigned64 = 0x001B,
  MAC_ADDRESS = 0x0401,
  IP_ADDRESS = 0x0402,
  NETTIME = 0x0403,

  //variable size
  Visible_String = 0x0009,
  Octet_String = 0x000A,
  Unicode_String = 0x000B,
} data_type_name_t;

uint32_t co_read_UNSIGNED32(chanend c, const uint16_t index, const uint8_t subindex);

co_error_code_t co_read_PDO_entry(chanend c,const uint16_t index,const uint8_t subindex,
    REFERENCE_PARAM(uint16_t, pdo_index),
    REFERENCE_PARAM(uint8_t, pdo_subindex),
    REFERENCE_PARAM(uint16_t, pdo_length),
    REFERENCE_PARAM(uint16_t, pdo_offset));

uint8_t co_read_NumberOfEntries(chanend c, const uint16_t index);

co_error_code_t co_read(chanend c, const uint16_t index, const uint8_t subindex,
    uint8_t data[], REFERENCE_PARAM(unsigned, no_of_bytes));


/*
 * co_read_UNSIGNED32
 * co_increment_UNSIGNED32
 */
//co_error_code_t co_read(chanend c, const uint16_t index, const uint8_t subindex,
 //   REFERENCE_PARAM(uint8_t, data), REFERENCE_PARAM(unsigned, no_of_bytes));

co_error_code_t co_write(chanend c,const uint16_t index, const uint8_t subindex,
    uint8_t data[], unsigned no_of_bytes);


void can_open_interface(chanend c_dll, chanend c_nmt, chanend c_app);


#endif /* CAN_OPEN_INTERFACE_H_ */
