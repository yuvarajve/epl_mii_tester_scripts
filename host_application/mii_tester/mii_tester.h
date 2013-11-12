#ifndef __MII_TESTER_H__
#define __MII_TESTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ETH_SPEED              100    // in Mbps
#define MAC_DST_BYTES          6
#define MAC_SRC_BYTES          6
#define ETH_TYPE_BYTES         2
#define MIN_NO_OF_PACKET       1
#define MAX_NO_OF_PACKET       20
#define MIN_FRAME_SIZE         64      
#define MAX_FRAME_SIZE         1600   
#define CRC_BYTES			   sizeof(unsigned int)
#define MIN_IFG_BYTES          8
#define MAX_IFG_BYTES          12     // Caution: Don't exceed this
#define LAST_FRAME             (1<<7)
#define MAX_BYTES_CAN_SEND     255
#define END_OF_PACKET_SEQUENCE (3<<6)

/* The entire Ethernet frame generated and sent from host are fragmented 
* into sizeof MAX_BYTES_CAN_SEND. This size includes sizeof(packet_info)
* +------------------------------------------------------------------+
* |  packet_number  |  frame_id  |  frame_len  |         data        |
* +------------------------------------------------------------------+
* Frame data     : 1600
* CRC32          :    4
* packet size    :    4
* packet delay   :    4
* packet number  :    1
* frame id       :    1
* frame length   :    2
*/

// packet info
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

		
#define PKT_CTRL_BYTES       sizeof(packet_control_t)
#define ETH_FRAME_BYTES      sizeof(ethernet_frame_t)
#define DEFAULT_LEN          (sizeof(packet_info_t) - MAX_FRAME_SIZE +  CRC_BYTES)  

#endif // __MII_TESTER_H__
