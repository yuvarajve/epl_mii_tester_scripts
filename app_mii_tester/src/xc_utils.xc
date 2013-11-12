#include <xs1.h>
#include <stdint.h>
#include "xc_utils.h"

void wait(unsigned delay)
{
    timer t;
    int time;

    t :> time;
    t when timerafter(time + delay) :> void;
}

uintptr_t get_buffer(CHANEND_PARAM(chanend, c_chl))
{
	uintptr_t dptr;
	c_chl :> dptr;
	return dptr;
}
unsigned get_buffer_int(CHANEND_PARAM(chanend, c_chl))
{
	unsigned val;
	c_chl :> val;
	return val;
}
void put_buffer(CHANEND_PARAM(chanend, c_chl), uintptr_t dptr)
{
	c_chl <: dptr;
}

void put_buffer_int(CHANEND_PARAM(chanend, c_chl), unsigned val)
{
	c_chl <: val;
}

