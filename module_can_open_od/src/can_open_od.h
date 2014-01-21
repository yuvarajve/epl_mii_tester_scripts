#ifndef CAN_OPEN_OD_H_
#define CAN_OPEN_OD_H_

#if defined(__XC__)
extern "C" {
#endif

typedef enum {
  e_success = 0,
  e_object_does_not_exist_in_od,
  e_sub_index_does_not_exist_in_object,
  e_number_of_bytes_does_not_match_data_type,
  e_invalid_access,
  e_unhandled_exception,
  e_object_code_not_supported,
  e_object_data_missing
} co_error_code_t;

/*
 * no_of_bytes should be set to the maximum number of bytes that the data buffer may hold, on return
 *             will be the number of bytes written to data.
 */
co_error_code_t  co_od_read(unsigned short address, unsigned char sub_index, void* data, unsigned* no_of_bytes);
co_error_code_t co_od_write(unsigned short address, unsigned char sub_index, void* data, unsigned no_of_bytes);

#if defined(__XC__)
} // Extern "C"
#endif

#endif /* CAN_OPEN_OD_H_ */
