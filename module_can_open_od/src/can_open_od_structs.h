#ifndef CAN_OPEN_OD_STRUCTS_H_
#define CAN_OPEN_OD_STRUCTS_H_

typedef enum {
   access_rw,
   access_ro,
   access_wo,
   access_const,
} access_t;

typedef enum {
  pdo_no,
  pdo_default,
  pdo_optional,
  pdo_tpdo,
  pdo_rpdo
} pdo_mapping_t;

typedef struct {
  void * value;
  unsigned char data_num_bytes;
  unsigned char mask:3, access:2, pdo_mapable:3;
  unsigned short data_type;
} var_t;

typedef struct {
  var_t var;
  char * name;
} record_subindex_t;

typedef struct {
  var_t * number_of_entries;
  void * value;

  unsigned char si_max_number_of_entries;
  unsigned char mask;
  unsigned char access;
  unsigned char pdo_mapable;

  unsigned short data_type;
  unsigned short data_num_bytes;
} array_t;

typedef struct {
  record_subindex_t * sub_indicies;
  unsigned char si_max_number_of_entries;
  char padding[3];
} record_t;

typedef enum {
  DEFTYPE = 5,
  DEFSTRUCT = 6,
  VAR = 7,
  ARRAY = 8,
  RECORD = 9
} object_code_t;

typedef struct {
  void * data;  //must point to an s_var, s_array or s_record
  char * name;
  object_code_t object_code;
  unsigned short index;
  short padding;
} object_t;


#endif /* CAN_OPEN_OD_STRUCTS_H_ */
