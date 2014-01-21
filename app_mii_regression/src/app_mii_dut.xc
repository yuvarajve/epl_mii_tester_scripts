#include <xs1.h>
#include <platform.h>
#include <xscope.h>
#include <print.h>
#include "smi.h"
#include "powerlink.h"
#include <stdio.h>
#include <stdint.h>
#include "thread_id.h"
#include <assert.h>
#include "debug_print.h"

#define XSCOPE_TILE     0
#define ETHERNET_TILE   1

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

on tile[1]: smi_interface_t smi0 = { 0x80000000, XS1_PORT_8A, XS1_PORT_4C };
on tile[1]: smi_interface_t smi1 = { 0, XS1_PORT_1M, XS1_PORT_1N };

on tile[1] : pl_mii_ports p = {
    PORT_ETH_RXD_0,
    PORT_ETH_RXDV_0,
    PORT_ETH_TXD_1,

    PORT_ETH_RXD_1,
    PORT_ETH_RXDV_1,
    PORT_ETH_TXD_0,

    XS1_CLKBLK_1,
    XS1_CLKBLK_2,
    PORT_ETH_RXCLK_0,
  //  PORT_ETH_ERR_0,
    PORT_ETH_TXCLK_0,
    PORT_ETH_TXEN_0,

    XS1_CLKBLK_3,
    XS1_CLKBLK_4,
    PORT_ETH_RXCLK_1,
  //  PORT_ETH_ERR_1,
    PORT_ETH_TXCLK_1,
    PORT_ETH_TXEN_1
};

/*
 * SMI must start in 100 Mb/s half duplex with auto-neg off.
 */
void pl_mii(pl_mii_ports & p, streaming chanend c_dll,
    streaming chanend c_nmt, chanend error_handler);

void dll_handler(streaming chanend c_mii_to_dll)
{
    while(1) {
      select {
          case c_mii_to_dll :> unsigned mii_ptr:  // receive pointer from mii
              c_mii_to_dll <: 0; //acknowledge
              break;
      }
    }
}

void nmt_handler(streaming chanend c_mii_to_nmt)
{
    while(1) {
      select {
          case c_mii_to_nmt :> unsigned mii_ptr:  // receive pointer from mii
              c_mii_to_nmt <: 0; //acknowledge
              break;
      }
    }
}

void err_handler(chanend c_eh_mii)
{
    while(1) {
      select {
          case c_eh_mii :> unsigned mii_ptr:  // receive pointer from mii
              c_eh_mii <: 0; //acknowledge
              break;
      }
    }
}

/**
 * \brief   A core that listens to data being sent from the host and
 *          informs the data manager to form data
 */
void xscope_listener(chanend c_host_data)
{
  unsigned int xscope_buff[256/4];
  int data_gen_ack = 1;

  xscope_connect_data_from_host(c_host_data);

  while(1) {
    int num_byte_read=0;

      select {
        case xscope_data_from_host(c_host_data, (unsigned char *)xscope_buff, num_byte_read): {
          if(num_byte_read != 0) {
              debug_printf("dut: no_of_packets   %d\n",xscope_buff[0]);
              xscope_int(MII_DUT_ACK,2);
          }
          break;
        }
      }
  }
}

in buffered port:4 mii_0_p_mii_rxer = PORT_ETH_ERR_0;          /**< MII RX error wire */
in buffered port:1 mii_1_p_mii_rxer = PORT_ETH_ERR_1;          /**< MII RX error wire */

int main(){

  chan c_host_data;
  streaming chan c_mii_to_dll;
  streaming chan c_mii_to_nmt;
  chan c_eh_mii;

  par {

    xscope_host_data(c_host_data);
    on tile[XSCOPE_TILE] : {
        xscope_listener(c_host_data);
    }

    on tile[ETHERNET_TILE] : {

      set_core_fast_mode_on();
      smi_init(smi0);
      smi_init(smi1);

      set_port_clock(mii_0_p_mii_rxer, p.mii_0_clk_mii_rx);
      set_port_clock(mii_1_p_mii_rxer, p.mii_1_clk_mii_rx);

      par {
        pl_mii(p, c_mii_to_dll, c_mii_to_nmt, c_eh_mii);
        dll_handler(c_mii_to_dll);
        nmt_handler(c_mii_to_nmt);
        err_handler(c_eh_mii);
      }

    }
  }

  return 0;
}
