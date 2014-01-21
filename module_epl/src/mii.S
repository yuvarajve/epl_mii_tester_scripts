#include <xs1.h>

/*
POWERLINK MII

Functionality
- top level there is a select waiting on both of the mii ports and c_dll
	- c_dll channel is waiting on a pointer and number of bytes as two words.
	  Upon reception it

The pointer pushed on to the output channels is a pointer to the first
word of the frame when the CRC has been confirmed as correct.

The input size is from the first word of the frame to the end of the frame
not including the CRC (as the CRC is computed and inserted by the MII).


Extension work
- Make more generic
	- accept an array of channels to broadcast on
	- variable number of buffers
	- mac filtering
*/

.cc_top mii.function
#define E_DLL_CRC_TH 0x8164

#define BUFFER_COUNT 2
#define BUFFER_SIZE_BYTES 1518
#define BUFFER_SIZE_WORDS ((BUFFER_SIZE_BYTES+3)>>2)

// Number of connected clients - changing this number requires a significant re-write
#define NUM_CLIENTS 2
#define CLIENT_DLL 0
#define CLIENT_NMT 1

    .section .cp.const4 ,"acM", @progbits, 4
    .align 4
pl_mii_polynomial:
    .word 0xEDB88320
pl_mii_initialCRC:
    .word 0x9226F562
pl_mii_first_word:
    .word 0x55555555
pl_mii_second_word:
    .word 0xD5555555

#define p_mii_from_rxd  0
#define p_mii_from_rxdv 1
#define p_mii_tx_to     2
#define PORT_BUNDLE_COUNT 3

// stack
#define S_CUR_BUFFER_POINTER 	0
#define S_C_EH 				1
#define S_TX_OUTPUT_BYTE_COUNT 2
#define S_TIMESTAMP 			3
#define S_TX_BUFFER 			4
//#define S_TX_BUFFER 			5
#define S_LAST_DV_TIME 		6
#define S_RX_BUFFER 			7
// Don't add items on the stack here - this is the RX buffer

#define S_MAX 					8


/*
PENDING WORK
 |             3 - 2            |          1 - 0           |
 | count of pending tx requests | count of pending rx acks |

A transmit request looks like:
 | Thread ID | Size in bytes | pointer |
 |    31     |    30 - 19    | 18 - 0  |

*/


// registers
#define PORT_STRUCT		r0
#define C_DLL  			r1
#define C_NMT  			r2
#define TIMER 			r3
#define PENDING_WORK	r4
#define TXD_0  			r5
#define TXD_1  			r6

//These are the defines for rx
//(before the common code)
#define RXD_0  			r7
#define RXD_1  			r8
//(during the common code)
#define RX_CRC			r6
#define TXD    			r7
#define RXDV   			r8
#define RXD    			r9

//These are the defines for tx
#define TX_DATA  		r7
#define TX_CRC 			r8

    .text
    .align 4
    .globl mii
.type  mii, @function
mii:
	//we are never returning so no need to save the registers
	entsp (S_MAX + BUFFER_SIZE_WORDS*BUFFER_COUNT)

//save off c_eh
    stw r3, sp[S_C_EH]

//setup rxdv event vectors for both receive PHYS
    ldw r10, PORT_STRUCT[p_mii_from_rxdv]
    ldw r9, PORT_STRUCT[p_mii_from_rxdv + PORT_BUNDLE_COUNT]
	ldc r11, 0
	setd res[r10], r11
	setd res[r9], r11
	setc res[r10], 17
	setc res[r9], 17
	ldap r11, input_from_rxdv
	setv res[r10], r11
	setv res[r9], r11

//get a timer and put it on the stack
	getr r3, XS1_RES_TYPE_TIMER
	setc res[r3], 1//what does this do?


	ldaw r11, sp[S_RX_BUFFER]
	set dp, r11
	stw r11, sp[S_CUR_BUFFER_POINTER]

	ldw RXD_0, PORT_STRUCT[p_mii_from_rxd]
	ldw RXD_1, PORT_STRUCT[p_mii_from_rxd + PORT_BUNDLE_COUNT]

	//init with no work to do
	ldc PENDING_WORK, 0

	ldap r11, do_tx
	setv res[TIMER], r11

	ldw TXD_0, PORT_STRUCT[p_mii_tx_to+PORT_BUNDLE_COUNT]
	ldw TXD_1, PORT_STRUCT[p_mii_tx_to]

	clre

wait_until_idle:
	ldw r11, PORT_STRUCT[p_mii_from_rxdv]
	ldw r10, PORT_STRUCT[p_mii_from_rxdv + PORT_BUNDLE_COUNT]
	in r9, res[r10]
	bt r9, waiting_loop
	in r9, res[r11]
	bf r9, skip_waiting_loop

waiting_loop:
	in r9, res[r10]
	bt r9, waiting_loop
	in r9, res[r11]
	bt r9, waiting_loop
	setc res[TIMER], XS1_SETC_COND_NONE
	in r9, res[TIMER]
	stw r9, sp[S_LAST_DV_TIME]
skip_waiting_loop:

	clre

	//set the timer to event at the end of IFG
	ldw r9, sp[S_LAST_DV_TIME]
	ldc r10, 148		//TODO this controls the interframe gap
	add r9, r9, r10
	setd res[TIMER], r9
	setc res[TIMER], XS1_SETC_COND_AFTER

	//if there are tx_requests pending then enable the timer event
	//else disable timer event
	shr r9, PENDING_WORK, 2
	eet r9, res[TIMER]

	ldw RXD_0, r0[p_mii_from_rxd]
	ldw RXD_1, r0[p_mii_from_rxd + PORT_BUNDLE_COUNT]

	ldc r9, 0xD

// setup rxd_0 event vector
	setd res[RXD_0], r9
	setc res[RXD_0], 17//put the define in
	ldap r11, mii_0_rx
	setv res[RXD_0], r11

//setup rxd_1 event vector
	setd res[RXD_1], r9
	setc res[RXD_1], 17//put the define in
	ldap r11, mii_1_rx
	setv res[RXD_1], r11

//setup c_dll event vector
	ldap r11, handle_c_dll
	setv res[C_DLL], r11

//setup c_nmt event vector
	ldap r11, handle_c_nmt
	setv res[C_NMT], r11

	eeu res[C_NMT]
	eeu res[C_DLL]
	eeu res[RXD_0]
	eeu res[RXD_1]

	//preload the 0x55555555
	ldw r9, cp[pl_mii_first_word]

	.xtabranch handle_c_dll, handle_c_nmt, mii_0_rx, mii_1_rx, do_tx
	waiteu
/*
at the wait the registers look like:
// registers
r0	port struct
r1	c_dll
r2	c_nmt
r3	timer
r4	pending_work
r5	p_txd_0
r6	p_txd_1
r7	p_rxd_0
r8	p_rxd_1
r9	0x55555555
r10	temp
r11	temp
*/
////////////////////////////////////////////////////////////////////////
handle_c_dll:
	in r11, res[C_DLL]
	bt r11, is_tx_req
handle_dll_ack:
	sub PENDING_WORK, PENDING_WORK, 1
	.xtabranch handle_c_dll, handle_c_nmt, mii_0_rx, mii_1_rx, do_tx
	waiteu
handle_c_nmt:
	in r11, res[C_NMT]
	bt r11, is_tx_req
handle_nmt_ack:
	sub PENDING_WORK, PENDING_WORK, 1
	.xtabranch handle_c_dll, handle_c_nmt, mii_0_rx, mii_1_rx, do_tx
	waiteu
is_tx_req:
	ldaw r9, sp[S_TX_BUFFER]
	shr r10, PENDING_WORK, 2
	stw r11, r9[r10]	//store the tx request
	add PENDING_WORK, PENDING_WORK, 1<<2 // Increment pending TX

	eeu res[TIMER]
	ldw r9, cp[pl_mii_first_word]
	.xtabranch handle_c_dll, handle_c_nmt, mii_0_rx, mii_1_rx, do_tx
	waiteu

////////////////////////////////////////////////////////////////////////
mii_0_rx:
	in r11, res[RXD_0] //r11 happens to be pl_mii_second_word after this
	ldaw r11, PORT_STRUCT[0]
	ldw TXD, PORT_STRUCT[p_mii_tx_to]
	bu common

mii_1_rx:
	in r11, res[RXD_1]	//r11 happens to be pl_mii_second_word after this
	ldaw r11, PORT_STRUCT[PORT_BUNDLE_COUNT]
	ldw TXD, PORT_STRUCT[p_mii_tx_to+PORT_BUNDLE_COUNT]
	bu common

// from here onwards r4, r5 and r6 are set to RXD, RXDV and TX respectivly.
common:
	out res[TXD], r9	//output the preamble preload

	setc res[TIMER], XS1_SETC_COND_NONE
	in r10, res[TIMER]	//TODO maybe move this closer to the in rxd
	stw r10, sp[S_TIMESTAMP]

	ldw r9, cp[pl_mii_second_word]
	out res[TXD], r9

	ldw RXD, r11[p_mii_from_rxd]
	ldw RXDV, r11[p_mii_from_rxdv]

	clre
	eeu res[RXDV]
	eeu res[RXD]

	setc res[RXD], XS1_SETC_COND_NONE
	ldap r11, input_from_rxd
	setv res[RXD], r11

	//mkmsk RX_CRC, 32
	ldw RX_CRC, cp[pl_mii_initialCRC]
	ldw r10, cp[pl_mii_polynomial]

	.xtabranch input_from_rxd, input_from_rxdv
	waiteu

//Note: from mii_x_rx to input_from_rxd must be 320ns
input_from_rxd:
    in r11, res[RXD]
	out res[TXD], r11	//this will block for n cycles
	stw r11, dp[0] 	//save the word to the pointer
	ldaw dp, dp[1]  //increment the pointer by one word
	crc32 RX_CRC, r11, r10
	.xtabranch input_from_rxd, input_from_rxdv
	waiteu

input_from_rxdv:
    in r11, res[RXDV]		//final use of RXDV -> r8 is free
    endin r10, res[RXD]
    setc res[RXD], XS1_SETC_COND_NONE
	in r11, res[RXD]
	stw r11, dp[0] 			//save the word to the pointer
/*
	shr r8, r10, 5
	bf r8, less_than_32_bits
		ldaw dp, dp[1]  //increment the pointer by one word
		in r11, res[RXD]		//final use of RXD -> r9 is free
		ldc r8, 32
		sub r10, r10, r8
less_than_32_bits:
*/
	shr r9, r10, 3			//convert bit to byte count

	.xtabranch no_align, shift_24, shift_16, shift_8
	bru r9
	no_align: bu test_crc
	shift_24: shr r11, r11, 8
	shift_16: shr r11, r11, 8
	shift_8:  shr r11, r11, 8

	setpsc res[TXD], r10
	out res[TXD], r11
	ldw r10, cp[pl_mii_polynomial]
finish_crc:
	crc8 RX_CRC, r11, r11, r10
	sub r9, r9, 1
	bt r9, finish_crc



test_crc:	//if crc is correct, i.e. the crc==0xffffffff

	//record the time the ifg starts
	//NOTE: this is curently when the transmitted frame ends,
	// is that correct?
	setc res[TIMER], XS1_SETC_COND_NONE
	in r10, res[TIMER]
	stw r10, sp[S_LAST_DV_TIME]

    add RX_CRC, RX_CRC, 1


    //mov r10, r0
   // ldc r0, 0
    //bl _plugins
	//mov r0, r10

//	bf RX_CRC, crc_correct
bu crc_correct
crc_incorrect:
	//inform the error_handler of a bad frame
	//ldw r10, sp[S_C_EH] //this is the chanend of eh
	//outct res[r10], 1
	//chkct res[r10], 1
	//ldc r11, E_DLL_CRC_TH
	//out res[r10], r11

	//outct res[r10], 1
	//chkct res[r10], 1

    bu waiting_on_acks//TODO check

crc_correct:
	clre
	//check the channels for comms

	eeu res[C_DLL]
	eeu res[C_NMT]
	ldap r11, dll_cmd_waiting
	setv res[C_DLL], r11
	ldap r11, nmt_cmd_waiting
	setv res[C_NMT], r11
	setsr 1
	clrsr 1
	bu check_acks

	//this is where the channel handle code goes
dll_cmd_waiting:
	in r11, res[C_DLL]
	bu test_cmd
nmt_cmd_waiting:
	in r11, res[C_NMT]
test_cmd:
	bt r11, cmd_is_tx_req
	sub PENDING_WORK, PENDING_WORK, 1
	setsr 1
	clrsr 1
	bu check_acks

cmd_is_tx_req:
	ldaw r9, sp[S_TX_BUFFER]
	shr r10, PENDING_WORK, 2
	stw r11, r9[r10]	//store the tx request
	add PENDING_WORK, PENDING_WORK, 1<<2 // Increment pending TX
	setsr 1
	clrsr 1
	bu check_acks

check_acks:
	mov r11, PENDING_WORK
	zext r11, NUM_CLIENTS
	bf r11, no_acks_waiting

waiting_on_acks:
	//don't broadcast the pointer
	//reset the pointer back to the start of the current buffer
	ldw r11, sp[S_CUR_BUFFER_POINTER]
	set dp, r11

	ldw TXD_1, PORT_STRUCT[p_mii_tx_to]
	bu wait_until_idle

no_acks_waiting:
	//send out the address of the rx_buffer
	ldw r11, sp[S_CUR_BUFFER_POINTER]
	out res[r1], r11
	out res[r2], r11

	//and the timestamp
	//ldw r10, sp[S_TIMESTAMP] //might drop this as it could cause a hang
	//out res[r1], r10
	//out res[r2], r10

	//set both acks pending
	add PENDING_WORK, PENDING_WORK, 2

	//flip the buffer
	ldc r10, BUFFER_SIZE_WORDS*4*(BUFFER_COUNT-1)
	ldaw r9, sp[S_RX_BUFFER]

	add r10, r9, r10
	eq r10, r10, r11
	bf r10, move_pointer_to_next_buffer
	bu save_new_pointer

move_pointer_to_next_buffer:
	ldc r10, BUFFER_SIZE_WORDS*4
	add r9, r10, r9

save_new_pointer:
	stw r9, sp[S_CUR_BUFFER_POINTER]
	set dp, r9

	//restore  r6
	ldw TXD_1, PORT_STRUCT[p_mii_tx_to]
	bu wait_until_idle



////////////////////////////////////////////////////////////////////////
do_tx:
	//send out the preamble
	out res[TXD_0], r9
	out res[TXD_1], r9
	ldw r9, cp[pl_mii_second_word]
	out res[TXD_0], r9
	out res[TXD_1], r9
	in r7, res[TIMER] //to clear the event

/*
 * A transmit request looks like:
 *
 * | Thread ID | Size in bytes | pointer |
 * |    31     |    30 - 19    | 18 - 0  |
 *
 */
	ldw r9, sp[S_TX_BUFFER]

	shr r10, r9, 16
	shr r10, r10, 3
	ldc r11, 12
	zext r10, r11		//r10 is the size in bytes
	stw r10, sp[S_TX_OUTPUT_BYTE_COUNT]

	ldc r11, 19
	zext r9, r11			//r9 is the pointer

	//here we transmit r10 bytes and do the crc
	//it goes out on both mii_0_p_mii_txd and mii_1_p_mii_txd

	//mkmsk TX_CRC, 32 //the initial crc
	ldw TX_CRC, cp[pl_mii_initialCRC]
	ldw TX_DATA, r9[0]
	//assume more than 4 bytes (invalid packet if not)
tx_word:
	ldaw r9, r9[1]	//move the pointer along one word

	ldw r11, cp[pl_mii_polynomial]

	crc32 TX_CRC, TX_DATA, r11

	sub r10, r10, 4

	out res[TXD_0], TX_DATA
	out res[TXD_1], TX_DATA

	ldw TX_DATA, r9[0]

	shr r11, r10, 2	//this is the same as r11 = r10 > 3
	bt r11, tx_word

	ldw r11, cp[pl_mii_polynomial]

	bf r10, end_of_data	//if r10 == 0 then there is no need for outputting bytes

tx_bytes:
    ldaw r10, r10[r10]	//r10 = r10 * 5
    sub r10, r10, 5

	.xtabranch tx_one_byte, tx_two_bytes, tx_three_bytes
	bru r10
tx_one_byte:
	outpw res[TXD_0], TX_DATA, 8
	outpw res[TXD_1], TX_DATA, 8
	BRFU_u6 crc_one_byte

tx_two_bytes:
	outpw res[TXD_0], TX_DATA, 16
	outpw res[TXD_1], TX_DATA, 16
	BRFU_u6 crc_two_bytes

tx_three_bytes:
	outpw res[TXD_0], TX_DATA, 24
	outpw res[TXD_1], TX_DATA, 24

crc_three_bytes:
	crc8 TX_CRC, TX_DATA, TX_DATA, r11
crc_two_bytes:
	crc8 TX_CRC, TX_DATA, TX_DATA, r11
crc_one_byte:
	crc8 TX_CRC, TX_DATA, TX_DATA, r11

end_of_data:
	//if outputted bytes < 60 then pad
	ldw r10, sp[S_TX_OUTPUT_BYTE_COUNT]

	ldc r9, 60
	sub r10, r9, r10	//r10 = 60 - output byte count

	ldc r9, 0
	lss r9, r9, r10

	bf r9, tx_output_crc

	ldc r9, 0x0
tx_padding:
	sub r10, r10, 1
	outpw res[TXD_0], r9, 8
	outpw res[TXD_1], r9, 8
	crc8 TX_CRC, r9, r9, r11
	bt r10, tx_padding

tx_output_crc:
	mkmsk r9, 32
	crc32 TX_CRC, r9, r11
	out res[TXD_0], TX_CRC
	out res[TXD_1], TX_CRC

	//save off the time dv going low
	setc res[TIMER], XS1_SETC_COND_NONE
	in r10, res[TIMER]
	//TODO add or subtract a constant to align the dv going low with the in (for worst case)
	stw r10, sp[S_LAST_DV_TIME]

	//send out ack
	ldw r11, sp[S_TX_BUFFER]

	// Extract the ID field from the packet descriptor
	shr r11, r11, 24
	shr r11, r11, 7
	ldc r10, 0


	// Send ack to the right client
	//DLL 0
	//NMT 1
	shl r11, r11, 1

	.xtabranch tx_ack_to_dll, tx_ack_to_nmt
	bru r11
tx_ack_to_dll:
	out res[C_DLL], r10
	bu shift_tx_buffers
tx_ack_to_nmt:
	out res[C_NMT], r10
	nop
shift_tx_buffers:
//sort the buffers and pending stuff out
	sub PENDING_WORK, PENDING_WORK, 1<<NUM_CLIENTS
	shr r10, PENDING_WORK, NUM_CLIENTS

	bf r10, wait_until_idle  	//if there is a second tx request pending then move it into the first slot
	ldw r10, sp[S_TX_BUFFER+1]	//this need to be a loop for an arbitrary size buffer
	stw r10, sp[S_TX_BUFFER]
	bu wait_until_idle


.globl mii.nstackwords
.linkset mii.nstackwords, (S_MAX + BUFFER_SIZE_WORDS*BUFFER_COUNT)
.globl mii.maxtimers
.linkset mii.maxtimers, 0
.globl mii.maxchanends
.linkset mii.maxchanends, 0
.globl mii.maxthreads
.linkset mii.maxthreads, 1
.size mii, .-mii
.cc_bottom mii.function
