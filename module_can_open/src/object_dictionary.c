
#include "types.h"

//keep this in for testing
#include <assert.h>


typedef struct subindex_size {
  uintptr_t address;
  unsigned size;
} subindex_size;

typedef struct object {
  unsigned short addr;
  unsigned short subindex_count;
  void ** subindex_data;
} object;

#include "object_dictionary_gen.h"

/*
 * Note, there is no protection against searching for an invalid address or
 * subindex.
 * Solution 1:Set all leav node higher and lower pointers to 0xffff. A trap
 *            handler can take care of problems.
 * Solution 2:Add subindex_count and is_leaf fields to the struct(need more
 *            memory).
 */


/*
 * This returns a pointer to the start of the requested data.
 */
void * get_data_pointer(unsigned addr, unsigned subindex){

  unsigned upper_bound = NUMBER_OF_OBJECTS;
  unsigned lower_bound = 0;

  unsigned index_guess = (upper_bound + lower_bound)/2;

  unsigned p_addr = object_dict[index_guess].addr;

  while(1){
    if(p_addr < addr) {
      lower_bound = index_guess;
    } else if(p_addr < addr) {
      upper_bound = index_guess;
    } else {
      return object_dict[index_guess].subindex_data[subindex];
    }
    unsigned next_index_guess = (upper_bound + lower_bound)/2;
    if(index_guess == next_index_guess){
      //cry
    }
    index_guess = next_index_guess;
    p_addr = object_dict[index_guess].addr;
  }
}
/*
 * This has no method for dealing with invalid addresses
 */
UNSIGNED16 can_open_read_UNSIGNED16(unsigned addr, unsigned subindex){
  object * p = get_data_pointer(addr, subindex);
  return *((UNSIGNED16*)(p->subindex_data[subindex]));
}
