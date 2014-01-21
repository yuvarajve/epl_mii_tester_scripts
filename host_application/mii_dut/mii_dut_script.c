/*
* Note that the device and listener should be run with the same port and IP.
* For example:
*
*  xrun --xscope-realtime --xscope-port 127.0.0.1:12346 app_mii_tester.xe
*
*  ./mii_tester_script -s 127.0.0.1 -p 12346
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "xscope_host_shared.h"
#include "mii_dut.h"
/*
* Includes for thread support
*/
#ifdef _WIN32
#include <winsock.h>
#else
#include <pthread.h>
#endif

extern int xscope_ep_upload_pending;
int packet_sequence_active = 0;
void hook_data_received(int xscope_probe, void *data, int data_len)
{
  int value = *((int*)data);
  assert(data_len == 8);

  if (value == 2)
    packet_sequence_active = 0;
}

void hook_exiting()
{
}

/*
*/
#ifdef _WIN32
DWORD WINAPI packet_generation_thread(void *arg)
#else
void *packet_generation_thread(void *arg)
#endif
{
  int sockfd = *(int *)arg;
  
  unsigned char *pBuffer;
  *pBuffer = 0xEF;
  *(pBuffer+1) = 0xBE;
  *(pBuffer+2) = 0xAD;
  *(pBuffer+3) = 0xDE;
  
  //Allow   
#ifdef _WIN32
  Sleep(2000);
#else
  sleep(2);
#endif

  while(1) {
   sleep(1);
   xscope_ep_request_upload(sockfd, 4, pBuffer);	  
  }
  return 0;

}
void usage(char *argv[])
{
  printf("Usage: %s [-s server_ip] [-p port]\n", argv[0]);
  printf("  -s server_ip :   The IP address of the xscope server (default %s)\n", DEFAULT_SERVER_IP);
  printf("  -p port      :   The port of the xscope server (default %s)\n", DEFAULT_PORT);
  exit(1);
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
  HANDLE pg_thread;
#else
  pthread_t pg_tid;
#endif

  char *server_ip = DEFAULT_SERVER_IP;
  char *port_str = DEFAULT_PORT+1;
  int err = 0;
  int sockfd = 0;
  int c = 0;

  while ((c = getopt(argc, argv, "s:p:")) != -1) {
    switch (c) {
      case 's':
        server_ip = optarg;
        break;
      case 'p':
        port_str = optarg;
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

  sockfd = initialise_common(server_ip, port_str);

  // Now start the packet generation
#ifdef _WIN32
  pg_thread = CreateThread(NULL, 0, packet_generation_thread, &sockfd, 0, NULL);
  if (pg_thread == NULL)
    print_and_exit("ERROR: Failed to create packet generation thread\n");
#else
  err = pthread_create(&pg_tid, NULL, &packet_generation_thread, &sockfd);
  if (err != 0)
    print_and_exit("ERROR: Failed to create packet generation thread\n");
#endif

  handle_socket(sockfd);

  return 0;
}

