/*
* Note that the device and listener should be run with the same port and IP.
* For example:
*
*  xrun --xscope-realtime --xscope-port 127.0.0.1:12346 app_mii_tester.xe
*  xrun --xscope-realtime --xscope-port 127.0.0.1:12347 app_mii_regression.xe
*
*  ./mii_tester_script -s 127.0.0.1 -p 12346 -r 12347
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "xscope_host_shared.h"
#include "mii.h"
/*
* Includes for thread support
*/
#ifdef _WIN32
#include <winsock.h>
#else
#include <pthread.h>
#endif

extern int xscope_ep_upload_pending;
volatile int packet_sequence_active = 0;
int g_tester_sockfd=0, g_dut_sockfd=0;

/* The ID of the mii tester probe determined from the registrations */
int g_tester_probe = -1;
/* The ID of the mii dut probe determined from the registrations */
int g_dut_probe = -1;

void hook_registration_received(int sockfd, int xscope_probe, char *name)
{
   if(strcmp(name, "Mii_Tester_Ack") == 0) {
     printf("Mii xScope Registration : %s, Probe ID : %d\n",name,xscope_probe);
     g_tester_probe = xscope_probe;
   }

   if(strcmp(name, "Mii_Dut_Ack") == 0) {
     printf("Mii xScope Registration : %s, Probe ID : %d\n",name,xscope_probe);
     g_dut_probe = xscope_probe;
   }

}
void hook_data_received(int sockfd,int xscope_probe, void *data, int data_len)
{
  int value = *((int*)data);
  assert(data_len == 8);
  
  if((g_tester_probe != xscope_probe) || (g_dut_probe != xscope_probe)){
     printf("Invalid xscope probe : %d (expected %d )\n",xscope_probe,g_tester_probe);
     return;
  }
  
  // packet_sequence_active should be cleared only when ack received from dut_sockfd 
  if(sockfd == g_tester_sockfd) {
    if (value == 2) 
      packet_sequence_active = 0;
  }
  else if(sockfd == g_dut_sockfd) {
    if (value == 2)  
      packet_sequence_active = 0;
  }

}

void hook_exiting()
{
}
//**************************************************************************************//
const unsigned char mac_addr_destn[MAC_DST_BYTES] = {255,255,255,255,255,255};          //
const unsigned char mac_addr_src[MAC_SRC_BYTES]   = {255,255,255,255,255,255};          //
const unsigned char ether_type[ETH_TYPE_BYTES]    = {0x88,0xAB};                        //
unsigned char packet_data[MAX_FRAME_SIZE]         = {0};                                //
const unsigned int polynomial                     = 0xEDB88320;                         //
const unsigned int initial_crc                    = 0x9226F562;                         //
                                                                                        // 
#define DEFAULT_PORT_TESTER   DEFAULT_PORT                                              //
#define DEFAULT_PORT_DUT      "12347"                                                   //
#define INVALID_CRC_VALUE     0x12345678                                                //  
#define INVALID_IFG_DELAY     (10*8*1)  // 10 bytes * 8 bits * 1 Tick(10nSec) = 800nSec //
#define INVALID_DATA_LENGTH    50                                                       // 
//**************************************************************************************//
/*
* Incorporate a word into a Cyclic Redundancy Checksum.
*/
static unsigned int crc8(unsigned int checksum, unsigned int data)
{
  int i;

  for(i = 0; i < 8; i++) {
    int xorBit = (checksum & 1);

    checksum = ((checksum >> 1) | ((data & 1) << 31));
    data = data >> 1;

    if(xorBit)
	  checksum = checksum ^ polynomial;
  }

  return checksum;
}

// rand() returns number from 0 to 32767 (0x0000 to 0x7FFF)
static unsigned char get_random_packets(void)
{
  return( (rand() %( MAX_NO_OF_PACKET - MIN_NO_OF_PACKET + 1) + MIN_NO_OF_PACKET) );
}

static unsigned int get_random_packet_size(void)
{
  return( (rand() %( MAX_FRAME_SIZE - MIN_FRAME_SIZE + 1) + MIN_FRAME_SIZE) );
}

/*
* Time of 1bit frame @ 100Mbps
* 1 bit = (1/100e6) = 10nSec = 1 Timer Tick on xcore
* IFG delay @ 100Mbps = 10nSec * 12bytes * 8bits = 960nSec = 96 Timer Tick on xcore = 12*8*1
*
*/
static unsigned int get_random_ifg_delay(void)
{
  unsigned int random_ifg_delay = 0;
  unsigned int min, max;

   min = (MIN_IFG_BYTES * 8) * 1;   // calculated in terms of 10nSec tick rate of xcore timer
   max = (MAX_IFG_BYTES * 8) * 1;

   random_ifg_delay = (rand() %(max - min + 1) + min);

  return random_ifg_delay;
}

static unsigned char get_crc_err_packet_num(unsigned char max)
{
  unsigned char min = 5, crc_err_packet = 5;
  
  if(max > min) 
    crc_err_packet = (rand() %(max - min + 1) + min);

  return crc_err_packet;
}

static unsigned char get_ifg_err_packet_num(unsigned char max)
{
  unsigned char min = 10, ifg_err_packet = 10;

  if(max > min) 
    ifg_err_packet = (rand() %(max - min + 1) + min);
  
  return ifg_err_packet;
}

static unsigned char get_len_err_packet_num(unsigned char max)
{
  unsigned char min = 15, len_err_packet = 15;
  
  if(max > min) 
    len_err_packet = (rand() %(max - min +1) + min);

  return len_err_packet;
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

int send_packet(int sockfd,unsigned char pkt_no,unsigned char err_flag)
{
  unsigned int idx,crc_value = initial_crc;
  unsigned int delay = 96;//get_random_ifg_delay();
  unsigned int num_data_bytes = 128;//get_random_packet_size();
  unsigned char pBuffer_tester[MAX_BYTES_CAN_SEND] = {0};
  time_t start_time, current_time;
  int retval = XSCOPE_EP_FAILURE;

  packet_control_t packet_control;

  assert((num_data_bytes >= MIN_FRAME_SIZE) && (num_data_bytes <= MAX_FRAME_SIZE));
  
  if(err_flag & 4)      // inject invalid data length
    num_data_bytes = INVALID_DATA_LENGTH;
  if (err_flag & 2)     // inject invalid ifg delay 
    delay = INVALID_IFG_DELAY;
  if (err_flag & 1)          // inject invalid crc (initial_crc as crc)
    crc_value = INVALID_CRC_VALUE;
  else {                      // do crc calculation only if err_flag is not set to '1'
    for(idx = 0; idx < ((num_data_bytes-CRC_BYTES)); idx++)
      crc_value = crc8(crc_value,packet_data[idx]);
  }

  packet_control.frame_crc = crc_value; 
  packet_control.frame_info  = ((pkt_no & 0x3F) << 26);
  packet_control.frame_info |= ((delay & 0x7FFF) << 11);
  packet_control.frame_info |= ((num_data_bytes-CRC_BYTES) & 0x7FF);

 
  memcpy(pBuffer_tester,(unsigned char *)&packet_control,PKT_CTRL_BYTES);
  time(&current_time);
  start_time = current_time;
  while ((retval != XSCOPE_EP_SUCCESS) && ((current_time - start_time) < 5)) {
    retval = xscope_ep_request_upload(sockfd, PKT_CTRL_BYTES, pBuffer_tester);
    time(&current_time); // wait till we get success
  }

  assert(retval == XSCOPE_EP_SUCCESS);

  return 0;
}
/*
* A thread to generate random packets with random delay and size.
* and validate dut 
*/
#ifdef _WIN32
DWORD WINAPI mii_thread(void *arg)
#else
void *mii_thread(void *arg)
#endif
{
  int sockfd[2] = {0}; 
  unsigned char no_of_packets = 0,err_flag=0;
  unsigned char crc_err_packet_num = 0;
  unsigned char ifg_err_packet_num = 0;
  unsigned char len_err_packet_num = 0;
  unsigned int total_no_of_packets = 0;
  int loop;
  unsigned char *pBuffer_dut;

  sockfd[0] = *((int *)arg+0);
  sockfd[1] = *((int *)arg+1);

  get_initialised();

  srand(time(0)); //initialize the seed

  // Allow time for the links to come up
#ifdef _WIN32
  Sleep(2000);
#else
  sleep(2);
#endif

  while(1) {
    while (packet_sequence_active);  // wait for sequence completion
    printf(" -done\n");

    packet_sequence_active = 1;

    // get random packet number
    no_of_packets = get_random_packets();
    total_no_of_packets += no_of_packets;
    crc_err_packet_num = get_crc_err_packet_num(no_of_packets);
    ifg_err_packet_num = get_ifg_err_packet_num(no_of_packets);
    len_err_packet_num = get_len_err_packet_num(no_of_packets);
    printf("no_of_packets: %10d (%10d)\n", no_of_packets, total_no_of_packets);
    
    // send no_of_packets to be received to DUT
    *pBuffer_dut = no_of_packets;
    if(xscope_ep_request_upload(sockfd[1],4,pBuffer_dut) != XSCOPE_EP_SUCCESS)
       printf("dut xscope_ep_request_upload : failed !!\n");

    while (packet_sequence_active); // wait for sequence completion

    packet_sequence_active = 1;
    
    // always send no of packets less than '1', on last packet number add END_OF_PACKET
    for(loop=0; loop < no_of_packets-1; loop++)
    {
      // enable err_flag based on loop number and crc_err_packet_num (loop is packet number)
      // crc error injected only if there is more than 5 packets tobe send.
      if((crc_err_packet_num > 5) && (loop == crc_err_packet_num))
        err_flag = 1;    // bit 0 is set 
      // ifg delay error injected only if there is more than 10 packets tobe send.
      if((ifg_err_packet_num > 10) && (loop == ifg_err_packet_num))
        err_flag |= 2;   // ORed - because crc error and ifg error can occur together
      // datalength error injected only if there is more than 15 packets tobe send.
      if((len_err_packet_num > 15) && (loop == len_err_packet_num))
        err_flag |= 4;   // bit 2 is set

      if(send_packet(sockfd[0],loop,err_flag) != XSCOPE_EP_SUCCESS)
        printf("tester send_packet : Failed !!\n");

      err_flag = 0;
    }
    
    /* send end of packet, so that mii tester xcore code starts
     * doing tx on mii lines. crc_err_flag is not set for last packet
     */
    err_flag = 0;
    loop |= END_OF_PACKET_SEQUENCE;
    if(send_packet(sockfd[0],loop,err_flag) != XSCOPE_EP_SUCCESS)
      printf("tester send_packet : Failed !!\n");

    printf("Waiting for Sequence Completion...");fflush(stdout);
  }
  return 0;

}

void usage(char *argv[])
{
  printf("Usage: %s [-s server_ip] [-p port_0] [-r port_1]\n", argv[0]);
  printf("  -s server_ip :   The IP address of the xscope server (default %s)\n", DEFAULT_SERVER_IP);
  printf("  -p port_0    :   The port_0 of the mii tester xscope server (default %s)\n", DEFAULT_PORT_TESTER);
  printf("  -r port_1    :   The port_1 of the mii dut xscope server (default %s)\n", DEFAULT_PORT_DUT);
  exit(1);
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
  HANDLE pg_thread;
#else
  pthread_t pg_tid;
#endif

  char *server_ip  = DEFAULT_SERVER_IP;
  char *port_str_0 = DEFAULT_PORT_TESTER;
  char *port_str_1 = DEFAULT_PORT_DUT;
  int err = 0;
  int sockfd[2] = {0};
  int c = 0;

  while ((c = getopt(argc, argv, "s:p:r:")) != -1) {
    switch (c) {
      case 's':
        server_ip = optarg;
        break;
      case 'p':
        port_str_0 = optarg;
        break;
      case 'r':
        port_str_1 = optarg;
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

  if (err)
    usage(argv);

  printf("\nNote: MII-Tester will run on port: %s and MII-DUT will run on port: %s\n",port_str_0,port_str_1);
  g_tester_sockfd = initialise_socket(server_ip, port_str_0);
  g_dut_sockfd = initialise_socket(server_ip, port_str_1);
  printf("sockfd for tester port: %s (%d), dut port: %s (%d)\n\n",
          port_str_0,g_tester_sockfd,port_str_1,g_dut_sockfd);

  sockfd[0] = g_tester_sockfd;
  sockfd[1] = g_dut_sockfd;
  // Now start the packet generation
#ifdef _WIN32

  pg_thread = CreateThread(NULL, 0, mii_thread, sockfd, 0, NULL);
  if (pg_thread == NULL) 
    print_and_exit("ERROR: Failed to create mii thread\n");
  
#else
  err = pthread_create(&pg_tid, NULL, &mii_thread, sockfd);
  if (err != 0)
    print_and_exit("ERROR: Failed to create mii thread\n");
#endif
  
  handle_sockets(sockfd,2);
  

  return 0;
}

