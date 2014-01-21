#ifndef THREAD_ID_H_
#define THREAD_ID_H_

#define NMT_ID 0
#define DLL_ID 1

/*
 * A transmit request looks like:
 *
 * | Thread ID | Size in bytes | pointer |
 * |    31     |    30 - 19    | 18 - 0  |
 *
 */
#include <print.h>
#ifdef __XC__
static unsigned make_tx_req_p(uintptr_t pointer, unsigned size_in_bytes, unsigned thread_id) {
  unsigned t = pointer | (size_in_bytes<<19) | (thread_id << 31);
if(t == 0x82000040){
  while(1);
}
	return t;
}
/*
static unsigned make_tx_req(uint8_t buffer[], unsigned size_in_bytes, unsigned thread_id) {
  uintptr_t buffer_pointer;
  asm("mov %0, %1" : "=r"(buffer_pointer) : "r"(buffer));
  return make_tx_req_p(buffer_pointer, size_in_bytes, thread_id);
}
*/
#endif

#endif /* THREAD_ID_H_ */
