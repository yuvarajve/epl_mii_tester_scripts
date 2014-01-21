#include <string.h>
#include "can_open_od.h"
#include "object_dictionary.h"
#include "object_dictionary_defines.h"


#define DISABLE_ACCESS
//#define DISABLE_BOUNDS_CHECK

typedef enum {
  Boolean = 0x0001,
  Integer8 = 0x0002,
  Integer16 = 0x0003,
  Integer32 = 0x0004,
  Unsigned8 = 0x0005,
  Unsigned16 = 0x0006,
  Unsigned32 = 0x0007,
  Real32 = 0x0008,
  Visible_String = 0x0009,
  Integer24 = 0x0010,
  Real64 = 0x0011,
  Integer40 = 0x0012,
  Integer48 = 0x0013,
  Integer56 = 0x0014,
  Integer64 = 0x0015,
  Octet_String = 0x000A,
  Unicode_String = 0x000B,
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
} data_type_name_t;



unsigned data_size_lu[28][2] = {
  {0,0x0009}, //Visible_String
  {0,0x000A}, //Octet_String
  {0,0x000B}, //Unicode_String
  {0,0x000F}, //Domain

  {1,0x0001}, //Boolean
  {1,0x0002}, //Integer8
  {1,0x0005}, //Unsigned8

  {2,0x0003}, //Integer16
  {2,0x0006}, //Unsigned16

  {3,0x0016}, //Unsigned24
  {3,0x0010}, //Integer24

  {4,0x0004}, //Integer32
  {4,0x0007}, //Unsigned32
  {4,0x0008}, //Real32
  {4,0x0402}, //IP_ADDRESS

  {5,0x0012}, //Integer40
  {5,0x0018}, //Unsigned40

  {6,0x0013}, //Integer48
  {6,0x000C}, //Time_of_Day
  {6,0x000D}, //Time_Diff
  {6,0x0019}, //Unsigned48
  {6,0x0401}, //MAC_ADDRESS

  {7,0x001A}, //Unsigned56
  {7,0x0014}, //Integer56

  {8,0x0011}, //Real64
  {8,0x0015}, //Integer64
  {8,0x001B}, //Unsigned64
  {8,0x0403}, //NETTIME
};





static object_t * get_object(unsigned short address){
  //do a binary search to find the object
#if 0
  unsigned index_above = 0;
  unsigned index_below = NUM_OBJECTS-1;
  while(1){
    unsigned index = index_above + (index_below - index_above)/2;
    unsigned object_address = objects[index].index;
    if(object_address == address){
      return &(objects[index]);
    } else if(object_address > address){
      index_below = index;
    } else {
      index+=index_above == index;
      index_above = index;
    }
  }
#else
  for(unsigned i=0;i<NUM_OBJECTS;i++)
    if(objects[i].index == address) return &(objects[i]);
#endif
  return 0;
}

static var_t * get_var(object_t * o){return (var_t*)(o->data);}
static array_t * get_array(object_t * o){return (array_t*)(o->data);}
static record_t * get_record(object_t * o){return (record_t*)(o->data);}

static object_code_t get_object_code(object_t * o){return o->object_code;}

static unsigned char get_var_data_size(var_t * v){
  return v->data_num_bytes;
}

static unsigned char get_record_number_of_entries(record_t * r){
  return *((unsigned char*)(r->sub_indicies[0].var.value));
}
static unsigned char get_array_number_of_entries(array_t * a){
  return *((unsigned char*)(a->number_of_entries->value));
}

static co_error_code_t read_var(var_t * v, void * data, unsigned * no_of_bytes){
#ifndef DISABLE_ACCESS
  if(get_var_access_type(v) == access_wo)
    return e_invalid_access;
#endif
  *no_of_bytes = get_var_data_size(v);
  if(v->value)
    memcpy(data, v->value, *no_of_bytes);
  else
    return e_object_data_missing;
  return e_success;
}

static co_error_code_t write_var(var_t * v, void * data, unsigned no_of_bytes){
#ifndef DISABLE_ACCESS
  if(get_var_access_type(v) == access_ro || get_var_access_type(v) == access_const)
    return e_invalid_access;
#endif

  unsigned expected_no_of_bytes =  get_var_data_size(v);

  if(expected_no_of_bytes != no_of_bytes)
      return e_number_of_bytes_does_not_match_data_type;

//TODO value range
  if(v->value)
    memcpy(v->value, data, no_of_bytes);
  else
    return e_object_data_missing;
  return e_success;
}

void * get_array_subindex_data(array_t * a, unsigned short si){
  return a->value + a->data_num_bytes * si;
}


co_error_code_t co_od_read(unsigned short address, unsigned char sub_index, void * data,
    unsigned * no_of_bytes){

  object_t * o = get_object(address);
  if(o){
    object_code_t oc = get_object_code(o);

    switch(oc) {
    case DEFSTRUCT:
    case DEFTYPE: {
      return e_object_code_not_supported;
    }
    case VAR : {
      var_t * v = get_var(o);
      return read_var(v, data, no_of_bytes);
    }
    case RECORD:{
      record_t * r = get_record(o);

      unsigned char number_of_entries = get_record_number_of_entries(r);
      if(number_of_entries < sub_index)
        return e_sub_index_does_not_exist_in_object;

      var_t * v = &(r->sub_indicies[sub_index].var);
      return read_var(v, data, no_of_bytes);
    }
    case ARRAY:{
      array_t * a = get_array(o);

      if(sub_index == 0){
        *no_of_bytes = 1;
        memcpy(data, a->number_of_entries->value, 1);
        return e_success;
      }

      unsigned char number_of_entries = get_array_number_of_entries(a);

#ifndef DISABLE_BOUNDS_CHECK
      if(number_of_entries < sub_index)
        return e_sub_index_does_not_exist_in_object;
#endif
      void * v = get_array_subindex_data(a, sub_index);
      memcpy(data, v, a->data_num_bytes);
      *no_of_bytes = a->data_num_bytes;
      return e_success;

    }
    }
  } else {
    return e_object_does_not_exist_in_od;
  }
  return e_unhandled_exception;
}

co_error_code_t co_od_write(unsigned short address, unsigned char sub_index, void * data,
    unsigned no_of_bytes){

  object_t * o = get_object(address);
  if(o){
    object_code_t oc = get_object_code(o);

    switch(oc) {
    case DEFSTRUCT:
    case DEFTYPE: {
      return e_object_code_not_supported;
    }
    case VAR : {
      var_t * v = get_var(o);
      return write_var(v, data, no_of_bytes);
    }
    case RECORD:{
      record_t * r = get_record(o);

      unsigned char number_of_entries = get_record_number_of_entries(r);
      if(number_of_entries < sub_index)
        return e_sub_index_does_not_exist_in_object;

      var_t * v = &(r->sub_indicies[sub_index].var);
      return write_var(v, data, no_of_bytes);
    }
    case ARRAY:{
      array_t * a = get_array(o);

      if(sub_index == 0){
        //printf("trying to write %d bytes\n", no_of_bytes);
        if(no_of_bytes != 1)
          return e_number_of_bytes_does_not_match_data_type;
        else {
          memcpy(a->number_of_entries->value, data, 1);
          return e_success;
        }
      }

      unsigned char number_of_entries = get_array_number_of_entries(a);
#ifndef DISABLE_BOUNDS_CHECK
      if(sub_index > a->si_max_number_of_entries)
        return e_sub_index_does_not_exist_in_object;
#endif
      if(a->data_num_bytes == no_of_bytes){
        void * v = get_array_subindex_data(a, sub_index);
        memcpy(v, data, no_of_bytes);
        return e_success;
      } else {
        return e_number_of_bytes_does_not_match_data_type;
      }
    }
    }
  } else {
    return e_object_does_not_exist_in_od;
  }
  return e_unhandled_exception;
}




