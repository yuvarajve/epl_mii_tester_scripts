
#ifndef __XC_UTILS_H_
#define __XC_UTILS_H_

#include <xccompat.h>
#include "common.h"

void wait(unsigned delay);
uintptr_t get_buffer(CHANEND_PARAM(chanend, c_chl));
unsigned get_buffer_int(CHANEND_PARAM(chanend, c_chl));
void put_buffer(CHANEND_PARAM(chanend, c_chl), uintptr_t dptr);
void put_buffer_int(CHANEND_PARAM(chanend, c_chl), unsigned val);

#endif /* __XC_UTILS_H_ */
