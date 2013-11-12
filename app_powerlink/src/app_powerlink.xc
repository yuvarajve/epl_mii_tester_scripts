#include <xs1.h>
#include <platform.h>
#include <xscope.h>
//#include <print.h>
#include "smi.h"
#include "powerlink.h"

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
    //PORT_ETH_ERR_0,
    PORT_ETH_TXCLK_0,
    PORT_ETH_TXEN_0,

    XS1_CLKBLK_3,
    XS1_CLKBLK_4,
    PORT_ETH_RXCLK_1,
   // PORT_ETH_ERR_1,
    PORT_ETH_TXCLK_1,
    PORT_ETH_TXEN_1
};

void xscope_user_init(void) {
  xscope_register(2,
      XSCOPE_STATEMACHINE, "Event", XSCOPE_INT, "PL frame Type",
      XSCOPE_STATEMACHINE, "Event", XSCOPE_INT, "no name");
  xscope_config_io(XSCOPE_IO_BASIC);
}

void app(chanend c){
  //do something
}

/*
 * SMI must start in 100 Mb/s half duplex with auto-neg off.
 */

int main(){
  chan c;
  par {
    on tile[1] : {
      set_core_fast_mode_on();
      app(c);
    }
    on tile[1] : {
      set_core_fast_mode_on();
      smi_init(smi0);
      smi_init(smi1);
      powerlink(p, c);
    }
  }
}
