#include "can_open_interface.h"

typedef enum {
  co_read_u32,
  co_read_pdo,
  co_read_noe,
  co_read_data,
  co_write_data
} cmds;

/*
 * TODO generate the header of known object names
 */

/*
 * These can only be used for know objects
 */
uint32_t co_read_UNSIGNED32(chanend c, const uint16_t index, const uint8_t subindex){
  uint32_t data;
  c <: co_read_u32;
  c <: index;
  c <: subindex;
  c :> data;
  return data;
}

co_error_code_t co_read_PDO_entry(
    chanend c,
    const uint16_t index,
    const uint8_t subindex,
    uint16_t &pdo_index,
    uint8_t &pdo_subindex,
    uint16_t &pdo_length,
    uint16_t &pdo_offset
){
  co_error_code_t err;
  c <: co_read_pdo;
  c <: index;
  c <: subindex;

  c :> pdo_index;
  c :> pdo_subindex;
  c :> pdo_length;
  c :> pdo_offset;
  c :> err;

  return err;
}

uint8_t co_read_NumberOfEntries(chanend c, const unsigned short index){
  unsigned data;
  c <: co_read_noe;
  c <: index;
  c :> data;
  return data;
}
co_error_code_t co_read(chanend c, const uint16_t index, const uint8_t subindex,
    uint8_t data[], unsigned &no_of_bytes){
  co_error_code_t err;
  uintptr_t p;
  asm("mov %0, %1": "=r"(p):"r"(data));
  c <: co_read_data;
  c <: index;
  c <: subindex;
  c <: p;
  c :> err;
  c :> no_of_bytes;
  return err;
}

co_error_code_t co_write(chanend c, const uint16_t index, const uint8_t subindex,
    uint8_t data[], unsigned no_of_bytes){

  if(index == 0x1011) return e_unhandled_exception;
  else if(index == 0x1020) return e_success;

  co_error_code_t err;
  uintptr_t p;
  asm("mov %0, %1": "=r"(p):"r"(data));
  c <: co_write_data;
  c <: index;
  c <: subindex;
  c <: p;
  c <: no_of_bytes;
  c :> err;
  return err;
}
#include <stdio.h>

static void handle_cmd(chanend c, unsigned cmd){
  unsafe {
    switch(cmd){
      case co_read_u32:{
        uint16_t index;
        unsigned char subindex;
        c :> index;
        c :> subindex;
        unsigned val;
        unsigned no_of_bytes;
        co_od_read(index, subindex, &val, &no_of_bytes);
        c <: val;
        break;
      }
      case co_read_noe:{
        uint16_t index;
        c :> index;
        uint8_t noe;
        co_od_read(index, 0, &noe, 1);
        c <: noe;
        break;
      }
      case co_read_pdo:{
        uint16_t index;
        uint8_t subindex;
        c :> index;
        c :> subindex;
        uint16_t data[8];
        unsigned n = 8;
        co_od_read(index, subindex, &data, &n);
        c <: data[0];
        c <: (data, uint8_t [])[2]; //FIXME or 3!
        c <: data[3];
        c <: data[2];
        c <: 0;     //fixme this shoudl be the error code
        break;
      }
      case co_read_data :{
        uint16_t index;
        uint8_t subindex;
        uintptr_t p;
        unsigned size;
        c :> index;
        c :> subindex;
        c :> p;
        c <: co_od_read(index, subindex, p, &size);
        c <: size;
        break;
      }
      case co_write_data :{
        uint16_t index;
        uint8_t subindex;
        uintptr_t p;
        unsigned size;
        c :> index;
        c :> subindex;
        c :> p;
        c :> size;
        c <: co_od_write(index, subindex, p, size);
        break;
      }
    }
  }
  return;
}

void can_open_interface(chanend c_dll, chanend c_nmt, chanend c_app){

  while(1){
    select {
      case c_dll :> unsigned cmd:{
        handle_cmd(c_dll, cmd);
        break;
      }
      case c_nmt :> unsigned cmd:{
        handle_cmd(c_nmt, cmd);
        break;
      }
      case c_app :> unsigned cmd:{
        handle_cmd(c_app, cmd);
        break;
      }
    }
  }
}
