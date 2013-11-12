#include <xs1.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "stdint.h"
#include "xc_utils.h"
#include "debug_print.h"

typedef struct packet_info{
	unsigned int packet_delay;
	unsigned int packet_size;
	unsigned char   payload[MAX_FRAME_SIZE];         // payload data
}packet_info_t;

// packet control
typedef struct packet_control {
    unsigned char packet_number;
	unsigned char frame_id;       // last bit says end of frame
	unsigned short frame_len;
}packet_control_t;

void print_frame(unsigned char *ptr, unsigned int size){

	for(unsigned int i=0; i<size+12;i++)
		printf("[%02d] : 0x%x\n",i,ptr[i]);
}

void data_handler(CHANEND_PARAM(chanend, c_listener_to_data_handler),CHANEND_PARAM(chanend, c_data_handler_to_tx))
{
	uintptr_t dptr;
	event_t event_flag = EVENT_WAIT;
	packet_control_t *packet_control;
	packet_info_t eth_buffer[MAX_PACKET_SIZE];
	unsigned char packet_number;
	unsigned short data_index = 0;

	while(1)
	{
		dptr = get_buffer(c_listener_to_data_handler);
		packet_control = (packet_control_t *)dptr;
		packet_number = (packet_control->packet_number%END_OF_PACKET_SEQUENCE);

		if( (event_flag == EVENT_WAIT) || (event_flag == EVENT_EOF) ) {
		    memcpy(&(eth_buffer[packet_number]),(unsigned int *)dptr+1,packet_control->frame_len);

		    data_index = (packet_control->frame_len-(END_OF_PACKET_SEQUENCE+PKT_SIZE_BYTES));

		}
		else if(event_flag == EVENT_FRAME_INCOMPLETE)
		{
			memcpy(&(eth_buffer[packet_number].payload[data_index]),(unsigned int *)dptr+1,packet_control->frame_len);
			data_index = data_index+packet_control->frame_len;
		}

		if( (packet_control->frame_id & LAST_FRAME) || (eth_buffer[packet_number].packet_size == (data_index-CRC_BYTES)) ){
		    event_flag = EVENT_EOF;
		    data_index = 0;
		}
		else
		    event_flag = EVENT_FRAME_INCOMPLETE;

		if( (packet_control->packet_number & END_OF_PACKET_SEQUENCE) && (packet_control->frame_id & LAST_FRAME)){
			event_flag = EVENT_EOP;
			data_index = 0;
		}

		if(event_flag == EVENT_EOP) {
			unsigned idx = 0;
			while(idx <= packet_number) {
				printf("%02d.",idx+1);
			    put_buffer_int(c_data_handler_to_tx,HOST_CMD_TX);
			    put_buffer_int(c_data_handler_to_tx,eth_buffer[idx].packet_delay);
			    put_buffer(c_data_handler_to_tx,(uintptr_t )&(eth_buffer[idx].payload[0]));
			    put_buffer_int(c_data_handler_to_tx,eth_buffer[idx].packet_size+CRC_BYTES);
			    get_buffer_int(c_data_handler_to_tx);
			    idx++;
			}

			event_flag = EVENT_WAIT;
		}
	}
}
