#include <xs1.h>
#include <print.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <platform.h>
#include "xassert.h"
#include "common.h"
#include <xscope.h>
#include "smi.h"
#include "debug_print.h"

#define XSCOPE_TILE     0
#define ETHERNET_TILE   1


#define XSCOPE_DATA_SIMULATION 0

#define TX_1_ENABLED    1

#ifndef TX_0_ENABLED
#define TX_0_ENABLED   1
#endif

#ifndef TX_1_ENABLED
#define TX_1_ENABLED   1
#endif

#define PORT_ETH_RXCLK_0 on tile[1]: XS1_PORT_1B
#define PORT_ETH_RXD_0 on tile[1]: XS1_PORT_4A
#define PORT_ETH_TXD_0 on tile[1]: XS1_PORT_4B
#define PORT_ETH_RXDV_0 on tile[1]: XS1_PORT_1C
#define PORT_ETH_TXEN_0 on tile[1]: XS1_PORT_1F
#define PORT_ETH_TXCLK_0 on tile[1]: XS1_PORT_1G
#define PORT_ETH_MDIOC_0 on tile[1]: XS1_PORT_4C
#define PORT_ETH_MDIOFAKE_0 on tile[1]: XS1_PORT_8A
#define PORT_ETH_ERR_0 on tile[1]: XS1_PORT_4D

#define PORT_ETH_RXCLK_1 on tile[1]: XS1_PORT_1J
#define PORT_ETH_RXD_1 on tile[1]: XS1_PORT_4E
#define PORT_ETH_TXD_1 on tile[1]: XS1_PORT_4F
#define PORT_ETH_RXDV_1 on tile[1]: XS1_PORT_1K
#define PORT_ETH_TXEN_1 on tile[1]: XS1_PORT_1L
#define PORT_ETH_TXCLK_1 on tile[1]: XS1_PORT_1I
#define PORT_ETH_MDIO_1 on tile[1]: XS1_PORT_1M
#define PORT_ETH_MDC_1 on tile[1]: XS1_PORT_1N
#define PORT_ETH_INT_1 on tile[1]: XS1_PORT_1O
#define PORT_ETH_ERR_1 on tile[1]: XS1_PORT_1P

#define ETH_SFD    0xD    /**< Start of Frame Delimiter */

on tile[1]: smi_interface_t smi0 = { 0x80000000, XS1_PORT_8A, XS1_PORT_4C };
on tile[1]: smi_interface_t smi1 = { 0, XS1_PORT_1M, XS1_PORT_1N };

on tile[1]: out port tx_prf_gpio_0 = XS1_PORT_1E;
on tile[1]: out port rx_prf_gpio_0 = XS1_PORT_1H;

typedef struct mii_tx_ports {
  out buffered port:32    txd;    /**< MII TX data wire */
  in port                 txclk;  /**< MII TX clock wire */
  out port                txen;   /**< MII TX enable wire */
  clock                   clk_tx; /**< MII TX Clock Block **/
}mii_tx_ports_t;

typedef struct mii_rx_ports {
  in buffered port:32    rxd;    /**< MII RX data wire */
  in port                rxdv;   /**< MII RX data valid wire */
  in port                rxclk;  /**< MII RX clock wire */
  clock                  clk_rx; /**< MII RX Clock Block **/
  in port                rxer;   /**< MII RX error wire */
} mii_rx_ports_t;

on tile[1] : mii_tx_ports_t tx0 = {
  PORT_ETH_TXD_0,
  PORT_ETH_TXCLK_0,
  PORT_ETH_TXEN_0,
  XS1_CLKBLK_2,
};

on tile[1] : mii_rx_ports_t rx0 = {
  PORT_ETH_RXD_0,
  PORT_ETH_RXDV_0,
  PORT_ETH_RXCLK_0,
  XS1_CLKBLK_1,
  PORT_ETH_ERR_0
};
on tile[1] : mii_rx_ports_t rx1 = {
  PORT_ETH_RXD_1,
  PORT_ETH_RXDV_1,
  PORT_ETH_RXCLK_1,
  XS1_CLKBLK_3,
  PORT_ETH_ERR_1
};
on tile[1] : mii_tx_ports_t tx1 = {
  PORT_ETH_TXD_1,
  PORT_ETH_TXCLK_1,
  PORT_ETH_TXEN_1,
  XS1_CLKBLK_4
};

// Timing tuning constants
#define PAD_DELAY_RECEIVE    0
#define PAD_DELAY_TRANSMIT   0
#define CLK_DELAY_RECEIVE    0
#define CLK_DELAY_TRANSMIT   7

/*
 * SMI must start in 100 Mb/s half duplex with auto-neg off.
 */
static void rx_init(mii_rx_ports_t &p) {
  set_port_use_on(p.rxclk);
  p.rxclk :> int x;
  set_port_use_on(p.rxd);
  set_port_use_on(p.rxdv);
  set_port_use_on(p.rxer);
  set_pad_delay(p.rxclk, PAD_DELAY_RECEIVE);
  set_port_strobed(p.rxd);
  set_port_slave(p.rxd);
  set_clock_on(p.clk_rx);
  set_clock_src(p.clk_rx, p.rxclk);
  set_clock_ready_src(p.clk_rx, p.rxdv);
  set_port_clock(p.rxd, p.clk_rx);
  set_port_clock(p.rxdv, p.clk_rx);
  set_clock_rise_delay(p.clk_rx, CLK_DELAY_RECEIVE);
  start_clock(p.clk_rx);
  clearbuf(p.rxd);
}
static void tx_init(mii_tx_ports_t &p) {
  set_port_use_on(p.txclk);
  set_port_use_on(p.txd);
  set_port_use_on(p.txen);
  set_pad_delay(p.txclk, PAD_DELAY_TRANSMIT);
  p.txd <: 0;
  p.txen <: 0;
  sync(p.txd);
  sync(p.txen);
  set_port_strobed(p.txd);
  set_port_master(p.txd);
  clearbuf(p.txd);
  set_port_ready_src(p.txen, p.txd);
  set_port_mode_ready(p.txen);
  set_clock_on(p.clk_tx);
  set_clock_src(p.clk_tx, p.txclk);
  set_port_clock(p.txd, p.clk_tx);
  set_port_clock(p.txen, p.clk_tx);
  set_clock_fall_delay(p.clk_tx, CLK_DELAY_TRANSMIT);
  start_clock(p.clk_tx);
  clearbuf(p.txd);
}
/*
 *
 */
#define RX_TIMEOUT 100000 // A timeout value of 1 ms
void rx(in buffered port:32 rxd, in port rxdv, server interface rx_config i_rx_config,rx_port_t rxd_port)
{
  timer t;
  unsigned char rx_indx=0,num_of_pkt_to_recd=0;
  unsigned buffer[MAX_BUFFER_WORDS];
  rx_packet_info_t rx_packet_info[MAX_PACKET_SEQUENCE];

  //initialise structure variables
  for(int i=0; i<MAX_PACKET_SEQUENCE;i++) {
    rx_packet_info[i].checksum = 0;
    rx_packet_info[i].no_of_bytes = 0;
    rx_packet_info[i].rx_start_tick = 0;
  }

  while(1) {
    select {
      case i_rx_config.put_packet_num_to_rx(unsigned char no_of_packets):
        num_of_pkt_to_recd = no_of_packets;
        break;
    }

    // Receive next sequence of packets
    assert(rx_indx == 0);

    unsigned rx_ticks = 0;
    t :> rx_ticks;

    do {
      unsigned end_of_frame = 0;
      unsigned word_count = 0;
      unsigned byte_count = 0;
      unsigned checksum = 0;
      unsigned timedout = 0;

      clearbuf(rxd);
      select {
        case rxd when pinseq(ETH_SFD) :> int:
          break;

        case t when timerafter(rx_ticks + RX_TIMEOUT) :> int:
          timedout = 1;
          rx_indx = 0;
          break;
      }
      
      t :> rx_ticks;

      if (!timedout) {
        while(!end_of_frame) {
          select {
            case rxd :> unsigned word: {
              buffer[word_count++] = word;
              byte_count+=4;
              break;
            }

            case rxdv when pinseq(0) :> int: {
              unsigned tail;
              unsigned taillen = endin(rxd);
              rxd :> tail;

              if(taillen) {
                tail = tail >> (32 - taillen);
                buffer[word_count++] = tail;
                byte_count += (taillen >> 3);

                checksum = ((tail << (32 - taillen)) | (buffer[word_count-2] >> taillen));
              }
              else {
                checksum = buffer[word_count-1];
              }

              end_of_frame = 1;
              break;
            } /**< rxdv low */
          }
        } /**< frame */
      } /**< timedout */

      rx_packet_info[rx_indx].rx_start_tick = rx_ticks;
      rx_packet_info[rx_indx].no_of_bytes = byte_count-CRC_BYTES;
      rx_packet_info[rx_indx++].checksum = checksum;
    } while(--num_of_pkt_to_recd);

    i_rx_config.rx_completed();

    select {
      case i_rx_config.get_rx_pkt_info(rx_packet_info_t rxpkt_info[],rx_port_t rxprt[]) -> unsigned char return_value:
        memcpy(rxpkt_info,rx_packet_info, rx_indx * sizeof(rx_packet_info_t));
        rxprt[0] = rxd_port;
        return_value = rx_indx;
        rx_indx = 0;
        break;
    }
  }
}
/*
 *
 */
void tx(out buffered port:32 txd, server interface tx_config i_tx_config,tx_port_t txd_port)
{
  unsigned tx_ticks;
  unsigned char data_buff[1522];
  uintptr_t dptr;
  tx_packet_info_t tx_packet_info[MAX_PACKET_SEQUENCE];
  timer t;

  //initialise structure variables
  for(int i=0; i<MAX_PACKET_SEQUENCE;i++) {
    tx_packet_info[i].checksum = 0;
    tx_packet_info[i].no_of_bytes = 0;
    tx_packet_info[i].tx_start_tick = 0;
  }

  for(int idx = 0;idx<12;idx++)
    data_buff[idx] = 0xFF;

  data_buff[12] = 0x88; data_buff[13] = 0xAB;   /**< Ethernet Powerlink Type */

  for(int idx=14;idx<(MAX_FRAME_SIZE-CRC_BYTES);idx++)
    data_buff[idx] = ((idx-14)%255)+1;          /**< initialize packet data with 1,2,3,..255 */

  asm volatile("mov %0, %1": "=r"(dptr):"r"(data_buff));

  unsigned tx_indx = 0;
  while(1) {
    unsigned packets_to_send = 0;
    packet_control_t packet_control[MAX_PACKET_SEQUENCE];

    select {
      case i_tx_config.put_packet_ctrl_to_tx(packet_control_t pkt_ctrl[],unsigned char no_of_packets): {
        packets_to_send = no_of_packets;
        memcpy(packet_control,pkt_ctrl, no_of_packets * sizeof(packet_control_t));
        break;
      }
    }
    
    assert(packets_to_send <= MAX_PACKET_SEQUENCE);

    while (packets_to_send) {
      unsigned wait_time = packet_control[tx_indx].frame_delay;
      unsigned size_in_bytes = packet_control[tx_indx].frame_size;
      unsigned checksum = packet_control[tx_indx].frame_crc;
      unsigned data = 0;

      tx_packet_info[tx_indx].no_of_bytes = size_in_bytes;
      tx_packet_info[tx_indx].checksum = checksum;

      memcpy(&data_buff[size_in_bytes],&checksum,CRC_BYTES);

      if (!tx_indx)
        t :> tx_ticks;

      /**< when a tx cmd come from tx_to_app then send it out */
      t when timerafter(tx_ticks+wait_time) :> tx_ticks;

      tx_packet_info[tx_indx++].tx_start_tick = tx_ticks;
      assert(tx_indx <= MAX_PACKET_SEQUENCE);

      txd <: 0x55555555;              /**< send ethernet preamble */
      txd <: 0xD5555555;              /**< send Start of frame delimiter */

      /**< send data from pointer, including checksum */
      int idx;
      for(idx=0; idx<((size_in_bytes+CRC_BYTES)>>2);idx++) {
        asm volatile("ldw %0, %1[%2]":"=r"(data):"r"(dptr), "r"(idx):"memory");
        txd <: data;
      }

      asm volatile("ldw %0, %1[%2]":"=r"(data):"r"(dptr), "r"(idx):"memory");
      /**< send the remaining no of bytes, if not in 4byte offset */
      if(size_in_bytes&3) {
        unsigned tailllen = ((size_in_bytes&3)*8);
        partout(txd, tailllen, data);
      }

      sync(txd);
      t :> tx_ticks;
      packets_to_send--;
    }

    i_tx_config.tx_completed();

    select {
      case i_tx_config.get_tx_pkt_info(tx_packet_info_t txpkt_info[],tx_port_t tx_port[]) -> unsigned char return_value:
        memcpy(txpkt_info,tx_packet_info, tx_indx * sizeof(tx_packet_info_t));
        tx_port[0] = txd_port;
        return_value = tx_indx;
        tx_indx = 0;
        break;
    }
  }
}

/**
 * \brief   A core that listens to data being sent from the host and
 *          informs the data manager to form data
 */
void xscope_listener(chanend c_host_data,client interface xscope_config i_xscope_config_0,
                                         client interface xscope_config i_xscope_config_1)
{
  unsigned int xscope_buff[256/4];
  tx_port_t port_num = TX_PORT_0;

  xscope_connect_data_from_host(c_host_data);

  while(1) {
    int num_byte_read=0;
    int data_gen_ack = 1;
    if(!XSCOPE_DATA_SIMULATION) {
      select {
        case xscope_data_from_host(c_host_data, (unsigned char *)xscope_buff, num_byte_read): {
          if(num_byte_read != 0) {
            if(port_num == TX_PORT_0) {
#if (TX_0_ENABLED)
            i_xscope_config_0.put_buffer(&xscope_buff[0]);
#endif
              if(GET_PACKET_NO(xscope_buff[0]) & END_OF_PACKET_SEQUENCE)
                port_num = TX_PORT_1;
            }
            else {
#if (TX_1_ENABLED)

            i_xscope_config_1.put_buffer(&xscope_buff[0]);
#endif
              if(GET_PACKET_NO(xscope_buff[0]) & END_OF_PACKET_SEQUENCE)
                port_num = TX_PORT_0;
            }
          }
          break;
        }
      }
    } else {
        timer t;
        unsigned tick;

        xscope_buff[0] = ((0 & 0x3F) << 26) | ((10000 & 0x7FFF) << 11) | ((64-CRC_BYTES) & 0x7FF);
        xscope_buff[1] = 0x12345678;
        i_xscope_config_0.put_buffer(&xscope_buff[0]);
        t :> tick;
        t when timerafter(tick+60000000) :> tick;
        xscope_buff[0] = ((1 & 0x3F) << 26)| ((1000 & 0x7FFF) << 11) | ((256-CRC_BYTES) & 0x7FF);
        xscope_buff[1] = 0x87654321;
        i_xscope_config_0.put_buffer(&xscope_buff[0]);
        t :> tick;
        t when timerafter(tick+60000000) :> tick;
        xscope_buff[0] = ((2 & 0x3F) << 26)| ((500 & 0x7FFF) << 11) | ((1024-CRC_BYTES) & 0x7FF);
        xscope_buff[1] = 0x87654321;
        i_xscope_config_0.put_buffer(&xscope_buff[0]);
        t :> tick;
        t when timerafter(tick+60000000) :> tick;
        xscope_buff[0] = ((3 & 0x3F) << 26)| ((960 & 0x7FFF) << 11) | ((1280-CRC_BYTES) & 0x7FF);
        xscope_buff[1] = 0x87654321;
        i_xscope_config_0.put_buffer(&xscope_buff[0]);
        t :> tick;
        t when timerafter(tick+60000000) :> tick;
        xscope_buff[0] = ((END_OF_PACKET_SEQUENCE+4 & 0x3F) << 26)| ((96 & 0x7FFF) << 11) | ((1522-CRC_BYTES) & 0x7FF);
        xscope_buff[1] = 0x87654321;
        i_xscope_config_0.put_buffer(&xscope_buff[0]);
    }
  }
}
/*
 *
 */
int main(void) {

  chan c_host_data;
  interface tx_config i_tx_0_config;
  interface tx_config i_tx_1_config;

  interface rx_config i_rx_0_config;
  interface rx_config i_rx_1_config;

  par {
    xscope_host_data(c_host_data);
    on tile [XSCOPE_TILE]: {
      interface xscope_config i_xscope_config_0;
      interface xscope_config i_xscope_config_1;
      interface data_manager i_data_manager_0;
      interface data_manager i_data_manager_1;
      set_core_fast_mode_on();
      par {
#if (TX_0_ENABLED)
          data_controller(i_data_manager_0,i_tx_0_config,i_rx_1_config);
          data_handler(i_xscope_config_0,i_data_manager_0);
#endif
#if (TX_1_ENABLED)
          data_controller(i_data_manager_1,i_tx_1_config,i_rx_0_config);
          data_handler(i_xscope_config_1,i_data_manager_1);
#endif
          xscope_listener(c_host_data,i_xscope_config_0,i_xscope_config_1);
      }
    }

    on tile [ETHERNET_TILE]: {
      streaming chan c_tx_0_to_timestamp;
      streaming chan c_tx_1_to_timestamp;
      streaming chan c_rx_0_to_timestamp;
      streaming chan c_rx_1_to_timestamp;

      set_core_fast_mode_on();
      smi_init(smi0);
      smi_init(smi1);
      tx_init(tx0);
      tx_init(tx1);
      rx_init(rx0);
      rx_init(rx1);
      par {
#if (TX_0_ENABLED)
          tx(tx0.txd,i_tx_0_config,TX_PORT_0);             /**< Transmit Frames on square slot */
          rx(rx1.rxd, rx1.rxdv, i_rx_1_config,RX_PORT_1);  /**< Receive Frames on circle slot */
#endif
#if (TX_1_ENABLED)
          tx(tx1.txd,i_tx_1_config,TX_PORT_1);             /**< Transmit Frames on circle slot */
          rx(rx0.rxd, rx0.rxdv, i_rx_0_config,RX_PORT_0);  /**< Receive Frames on square slot */
#endif
      }
    }
  }

  return 0;
}



