/*
 * Note that the device and listener should be run with the same port and IP.
 * For example:
 *
 *  xrun --xscope-realtime --xscope-port 127.0.0.1:12346 app_mii_tester.xe
 *
 *  ./mii_tester_script -s 127.0.0.1 -p 12346
 *
 */
#include "shared.h"
#include "mii_tester.h"
#include <stdlib.h>

/*
 * Includes for thread support
 */
#ifdef _WIN32
  #include <winsock.h>
#else
  #include <pthread.h>
#endif

#define DEFAULT_FILE   "cap.pcapng"

packet_info_t  packet_info;
packet_control_t pkt_ctrl;

FILE *g_pcap_fptr = NULL;
const char *g_prompt = " m> ";

// Indicate whether the output should be pcap or pcapng
int g_libpcap_mode = 0;

void hook_data_received(void *data, int data_len)
{
  if (g_libpcap_mode) {
    // Convert the pcapng data from the target to libpcap format
    enhanced_packet_block_t *ehb = (enhanced_packet_block_t *) data;

    // Time resolution in pcapng is 10ns
    uint64_t packet_time = (((uint64_t)ehb->timestamp_high << 32) | ehb->timestamp_low) / 100;
    uint32_t ts_sec = packet_time / 1000000;
    uint32_t ts_usec = packet_time % 1000000;

    pcaprec_hdr_t header = { ts_sec, ts_usec, ehb->captured_len, ehb->packet_len };
    fwrite(&header, sizeof(header), 1, g_pcap_fptr);
    fwrite(&ehb->data, ehb->captured_len, 1, g_pcap_fptr);
    fflush(g_pcap_fptr);
  } else {
    // Emit the pcapng data
    fwrite(data, data_len, 1, g_pcap_fptr);
  }
}

void hook_exiting()
{
  fflush(g_pcap_fptr);
  fclose(g_pcap_fptr);
}

void print_console_usage()
{
  printf("Supported commands:\n");
  printf("  h|?     : print this help message\n");
  printf("  e <o|n> : tell app to expect (o)versubscribed or (n)ormal traffic\n");
  printf("  x <e|d> : tell app to (e)nable or (d)isable xscope packet dumping\n");
  printf("  q       : quit\n");
}

#define LINE_LENGTH 1024

char get_next_char(char *buffer)
{
  char *ptr = buffer;
  while (*ptr && isspace(*ptr))
    ptr++;
  return *ptr;
}

int ethernet_packet_sent = 0;
#if 0
/*
 * A separate thread to handle user commands to control the target.
 */
#ifdef _WIN32
DWORD WINAPI console_thread(void *arg)
#else
void *console_thread(void *arg)
#endif
{
  int sockfd = *(int *)arg;
  char buffer[LINE_LENGTH + 1];
  do {
    int i = 0;
    int c = 0;

    printf("%s", g_prompt);
    for (i = 0; (i < LINE_LENGTH) && ((c = getchar()) != EOF) && (c != '\n'); i++)
      buffer[i] = tolower(c);
    buffer[i] = '\0';

    if (buffer[0] == 'q') {
      print_and_exit("Done\n");

    } else if (buffer[0] == 'e') {
      tester_command_t cmd = MII_TESTER_CONTROL_CMD;
      if (get_next_char(&buffer[1]) == 'o')
        cmd = MII_TESTER_CC_SEND;
      xscope_ep_request_upload(sockfd, 4, (char *)&cmd);

    } else if (buffer[0] == 'x') {
      tester_command_t cmd = MII_TESTER_CONTROL_CMD;
      if (get_next_char(&buffer[1]) == 'e')
        cmd = MII_TESTER_CC_RECEIVE;
      xscope_ep_request_upload(sockfd, 4, (char *)&cmd);

    } else if ((buffer[0] == 'h') || (buffer[0] == '?')) {
      print_console_usage();

    } else {
      printf("Unrecognised command '%s'\n", buffer);
      print_console_usage();
    }
  } while (1);

#ifdef _WIN32
  return 0;
#else
  return NULL;
#endif
}
#endif
/*
* Incorporate a word into a Cyclic Redundancy Checksum.
*/
static unsigned int crc8(unsigned int checksum, unsigned int data)
{
    int i,xorBit=0;

    for(i = 0; i < 8; i++) {
	    xorBit = (checksum & 1);
	
	    checksum = ((checksum >> 1) | ((data & 1) << 31));
	    data = data >> 1;
	
	    if(xorBit)
		    checksum = checksum ^ polynomial;
    }

    return checksum;
}
// rand() returns number from 0 to 32767 (0x0000 to 0x7FFF)
unsigned char get_random_packets(void)
{
    return( (rand() %( MAX_NO_OF_PACKET - MIN_NO_OF_PACKET + 1) + MIN_NO_OF_PACKET) );
}
unsigned int get_random_packet_size(void)
{
    return( (rand() %( MAX_FRAME_SIZE - MIN_FRAME_SIZE + 1) + MIN_FRAME_SIZE) );
}
/* 
* Time of 1bit frame @ 100Mbps
* 1 bit = (1/100e6) = 10nSec = 1 Timer Tick on xcore 
* IFG delay @ 100Mbps = 10nSec * 12bytes * 8bits = 960nSec = 96 Timer Tick on xcore = 12*8*1
*
* Time of 1bit frame @ 10Mbps
* 1 bit = (1/10e6) = 100nSec = 10 Timer Tick on xcore 
* IFG delay @ 10Mbps = 100nSec * 12bytes * 8bits = 9600nSec = 960 Timer Tick on xcore = 12*8*10
*/
unsigned int get_random_ifg_delay(void)
{
    unsigned int random_ifg_delay = 0;
    unsigned int min, max;

    if(ETH_SPEED == 10){
	    min = (MIN_IFG_BYTES * 8) * 10;   // calculated in terms of 10nSec tick rate of xcore timer
	    max = (MAX_IFG_BYTES * 8) * 10;
	}
	else {  // default 100Mbps
	    min = (MIN_IFG_BYTES * 8) * 1;   // calculated in terms of 10nSec tick rate of xcore timer
		max = (MAX_IFG_BYTES * 8) * 1;
	}

    random_ifg_delay = (rand() %( max - min + 1) + min);
    return random_ifg_delay;
}

void get_initialised(void)
{
    int idx;
    	
	memcpy(&(packet_data[0]), mac_addr_destn, MAC_DST_BYTES);
	memcpy(&(packet_data[6]), mac_addr_src, MAC_SRC_BYTES);
	memcpy(&(packet_data[12]), ether_type, ETH_TYPE_BYTES);
	
	for(idx=14; idx<MAX_FRAME_SIZE; idx++){
		packet_data[idx] = ((idx-14)%255)+1;    // initialize packet data with 1,2,3,..255
	}
		
}
int send_packet(int sockfd,unsigned char pkt_no)
{
	unsigned int idx,crc_value = initial_crc;
	unsigned int delay = get_random_ifg_delay();
	unsigned int num_data_bytes = 1600;//get_random_packet_size();
	unsigned int num_byte_write = 0;
	unsigned int data_size = 0;
	unsigned int data_index = 0;
	unsigned char pBuffer[MAX_BYTES_CAN_SEND] = {0};
	
	assert((num_data_bytes >= MIN_FRAME_SIZE) && (num_data_bytes <= MAX_FRAME_SIZE));
	
	//ethernet_frame_t *ethernet_frame = &packet_info.ethernet_frame;

	memcpy(packet_info.payload, packet_data, num_data_bytes);
		
	pkt_ctrl.packet_number  = pkt_no;
	packet_info.packet_delay   = delay;
	packet_info.packet_size    = num_data_bytes;
	
	for(idx = 0; idx < num_data_bytes; idx++){
		crc_value = crc8(crc_value, packet_info.payload[idx]);
	}		
	
	memcpy(&(packet_info.payload[num_data_bytes]),&crc_value,sizeof(crc_value));

	idx = 0;
	data_size = packet_info.packet_size + (sizeof(unsigned int)*3);
    if( (data_size + DEFAULT_LEN) > MAX_BYTES_CAN_SEND)
	{  	 
	    num_byte_write = MAX_BYTES_CAN_SEND;
		
	    while(data_size > 0)
		{   
		    if( (data_size + DEFAULT_LEN) > MAX_BYTES_CAN_SEND) {
			    pkt_ctrl.frame_id = idx++;
			}
			else {
			    num_byte_write = data_size+DEFAULT_LEN;
				pkt_ctrl.frame_id = idx | LAST_FRAME;
			}
			
		    pkt_ctrl.frame_len = num_byte_write-DEFAULT_LEN;
			memcpy(pBuffer,(unsigned char *)&pkt_ctrl,DEFAULT_LEN);
			memcpy(pBuffer+DEFAULT_LEN,((unsigned char *)&packet_info) + data_index,pkt_ctrl.frame_len);
		    assert(xscope_ep_request_upload(sockfd, num_byte_write,pBuffer) == XSCOPE_EP_SUCCESS);
			
			data_size -= (num_byte_write-DEFAULT_LEN);		
            data_index += (num_byte_write-DEFAULT_LEN);	
			printf("| %02d | 0x%05d | %03d | %d |\n",pkt_ctrl.packet_number%END_OF_PACKET,pkt_ctrl.frame_id%LAST_FRAME,pkt_ctrl.frame_len,num_byte_write);
            Sleep(1200);	// data missed on practical case without this delay		
		}
	}
	else {
	
	    // no fragmentation required. This is first & last frame
		num_byte_write = data_size+DEFAULT_LEN;
	    pkt_ctrl.frame_id = idx | LAST_FRAME;
        pkt_ctrl.frame_len = data_size;
        
		memcpy(pBuffer,(unsigned char *)&pkt_ctrl,DEFAULT_LEN);
		memcpy(pBuffer+DEFAULT_LEN,((unsigned char *)&packet_info) + data_index,pkt_ctrl.frame_len);
		assert(xscope_ep_request_upload(sockfd, num_byte_write,pBuffer) == XSCOPE_EP_SUCCESS);
		
		printf("| %02d | 0x%05d | %03d | %d |\n",pkt_ctrl.packet_number%END_OF_PACKET,pkt_ctrl.frame_id%LAST_FRAME,pkt_ctrl.frame_len,num_byte_write);
	}
		
	return 0;
}
/*
 * A separate thread to generate random packets with random delay and size.
 * This code is similar to random_traffic_generator
 */
#ifdef _WIN32
DWORD WINAPI packet_generation_thread(void *arg)
#else
void *packet_generation_thread(void *arg)
#endif
{
  int sockfd = *(int *)arg;
  unsigned char no_of_packets = 0;
  int loop;

    get_initialised();

    srand(time(0)); //initialize the seed
	
    while(1){
		if(ethernet_packet_sent == 0){
	    // get random packet number
	    no_of_packets = 1; //get_random_packets();
	    printf("\nPACKET_GEN: no_of_packets : %d\n\n",no_of_packets);
		printf("+-------------------------------------------+\n");
		//printf("|  ##  |  Delay  |   Size   |   Checksum   |\n");
		printf("| ## |  Frame  | Len |\n");
		printf("+-------------------------------------------+\n");
	    // always send no of packets less than '1', on last packet number add END_OF_PACKET
	    for(loop=0; loop < no_of_packets-1; loop++)
	    {
			send_packet(sockfd,loop);
		    Sleep(2000);  // 0.5Sec Sleep  // remove this
	    }
		
		loop |= END_OF_PACKET;
		send_packet(sockfd,loop);
	    ethernet_packet_sent = 1;
	    Sleep(1000);  // 1Sec Sleep   // remove this
		}

    }
    return 0;
  
}
void usage(char *argv[])
{
  printf("Usage: %s [-s server_ip] [-p port] [-l] [file]\n", argv[0]);
  printf("  -s server_ip :   The IP address of the xscope server (default %s)\n", DEFAULT_SERVER_IP);
  printf("  -p port      :   The port of the xscope server (default %s)\n", DEFAULT_PORT);
  printf("  -l           :   Emit libpcap format instead of pcapng\n");
  printf("  file         :   File name packets are written to (default '%s')\n", DEFAULT_FILE);
  exit(1);
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
  HANDLE thread_1,thread_2;
#else
  pthread_t tid_1,tid_2;
#endif

  char *server_ip = DEFAULT_SERVER_IP;
  char *port_str = DEFAULT_PORT;
  char *filename = DEFAULT_FILE;
  int err = 0;
  int sockfd = 0;
  int c = 0;

  while ((c = getopt(argc, argv, "ls:p:")) != -1) {
    switch (c) {
      case 's':
        server_ip = optarg;
        break;
      case 'p':
        port_str = optarg;
        break;
      case 'l':
        g_libpcap_mode = 1;
        break;
      case ':': /* -f or -o without operand */
        fprintf(stderr, "Option -%c requires an operand\n", optopt);
        err++;
        break;
      case '?':
        fprintf(stderr, "Unrecognised option: '-%c'\n", optopt);
        err++;
    }
  }
  for ( ; optind < argc; optind++) {
    if (filename != DEFAULT_FILE)
      err++;
    filename = argv[optind];
    break;
  }

  if (err)
    usage(argv);

  sockfd = initialise_common(server_ip, port_str);
  g_pcap_fptr = fopen(filename, "wb");

  if (g_libpcap_mode) {
    // Emit libpcap common header
    emit_pcap_header(g_pcap_fptr);

  } else {
    // Emit common header and two interface descriptions as there are two on the tap
    emit_pcapng_section_header_block(g_pcap_fptr);
    emit_pcapng_interface_description_block(g_pcap_fptr);
    emit_pcapng_interface_description_block(g_pcap_fptr);
  }
  fflush(g_pcap_fptr);
#if 0
  // Now start the console
#ifdef _WIN32
  //thread_1 = CreateThread(NULL, 0, console_thread, &sockfd, 0, NULL);
  if (thread_1 == NULL)
    print_and_exit("ERROR: Failed to create console thread\n");
#else
  //err = pthread_create(&tid_1, NULL, &console_thread, &sockfd);
  if (err != 0)
    print_and_exit("ERROR: Failed to create console thread\n");
#endif
#endif
    // Now start the packet generation
#ifdef _WIN32
  thread_2 = CreateThread(NULL, 0, packet_generation_thread, &sockfd, 0, NULL);
  if (thread_2 == NULL)
    print_and_exit("ERROR: Failed to create packet generation thread\n");
#else
  err = pthread_create(&tid_2, NULL, &packet_generation_thread, &sockfd);
  if (err != 0)
    print_and_exit("ERROR: Failed to create packet generation thread\n");
#endif
	
    handle_socket(sockfd);
	
  return 0;
}

