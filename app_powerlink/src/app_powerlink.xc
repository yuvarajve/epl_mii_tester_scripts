#include <xs1.h>
#include <platform.h>
#include "smi.h"
#include "powerlink.h"
#include <stdint.h>
#include "can_open_interface.h"

#define PORT_ETH_RXD_0 on tile[1]: XS1_PORT_4A
#define PORT_ETH_RXDV_0 on tile[1]: XS1_PORT_1C

#define PORT_ETH_RXCLK_0 on tile[1]: XS1_PORT_1B
#define PORT_ETH_TXCLK_0 on tile[1]: XS1_PORT_1G
#define PORT_ETH_RXCLK_1 on tile[1]: XS1_PORT_1J
#define PORT_ETH_TXCLK_1 on tile[1]: XS1_PORT_1I

#define PORT_ETH_TXD_0 on tile[1]: XS1_PORT_4B
#define PORT_ETH_TXEN_0 on tile[1]: XS1_PORT_1F
#define PORT_ETH_MDIOC_0 on tile[1]: XS1_PORT_4C
#define PORT_ETH_MDIOFAKE_0 on tile[1]: XS1_PORT_8A
#define PORT_ETH_ERR_0 on tile[1]: XS1_PORT_4D

#define PORT_ETH_RXD_1 on tile[1]: XS1_PORT_4E
#define PORT_ETH_RXDV_1 on tile[1]: XS1_PORT_1K

#define PORT_ETH_TXD_1 on tile[1]: XS1_PORT_4F
#define PORT_ETH_TXEN_1 on tile[1]: XS1_PORT_1L
#define PORT_ETH_MDIO_1 on tile[1]: XS1_PORT_1M
#define PORT_ETH_MDC_1 on tile[1]: XS1_PORT_1N
#define PORT_ETH_INT_1 on tile[1]: XS1_PORT_1O
#define PORT_ETH_ERR_1 on tile[1]: XS1_PORT_1P

on tile[1]: smi_interface_t smi0 = { 0x80000000, XS1_PORT_8A, XS1_PORT_4C };
on tile[1]: smi_interface_t smi1 = { 0, XS1_PORT_1M, XS1_PORT_1N };

on tile[0]: port p_led1=XS1_PORT_4A;
on tile[0]: port p_button1=XS1_PORT_4C;

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
    PORT_ETH_TXCLK_0,
    PORT_ETH_TXEN_0,

    XS1_CLKBLK_3,
    XS1_CLKBLK_4,
    PORT_ETH_RXCLK_1,
    PORT_ETH_TXCLK_1,
    PORT_ETH_TXEN_1
};

void wait_for_operational(){
  unsigned t,x = 0;
  while(x!=0xfd){
    asm("ldaw %0, dp[g_nmt_status]":"=r"(t));
    asm("ldw %0, %1[0]":"=r"(x):"r"(t));
  }
}


void app(chanend c_can_open, chanend c_gpio){
  //switch to ready
  wait_for_operational();

  timer t;
  unsigned time;
  t:> time;
  uint8_t d = 1;
  co_write(c_can_open, 0x6000, 0x1, &d, 1);

  while(1){
    select {
      case c_gpio :> uint8_t d:{
        co_write(c_can_open, 0x6000, 0x1, &d, 1);
        break;
      }
      case t when timerafter(time + 20000000):> time:{
        unsigned n;
        uint8_t data;
        co_read(c_can_open, 0x6200, 0x1, &data, n);
        c_gpio <: data;
        break;
      }
    }

  }

}

void gpio(chanend c){
  timer t;
  unsigned wait;
  int waiting = 1;
  uint8_t output = 1;
  uint8_t b;
  p_button1 :> b;
  while(1){
    select {
      case waiting => t when timerafter(wait+100000) :> wait :{
        waiting = 0;
        break;
      }
      case !waiting =>p_button1 when pinsneq(b) :> uint8_t new_b :{
        t:> wait;
        waiting = 1;
        if((new_b^b) == 1)
          if((new_b&1) == 1)
            if(output != 128)
              output = output << 1;
        if((new_b^b) == 2)
          if((new_b&2) == 2)
            if(output != 1)
              output = output >> 1;
        c <: output;
        b = new_b;
        break;
      }
      case c :> uint8_t d:{
        uint8_t count = 0;
        while(d){
          count++;
          d>>=1;
        }
        p_led1 <: ~count;
        break;
      }
    }
  }
}

int main(){
  chan c;
  chan c_gpio;
  par {
    on tile[0]: gpio(c_gpio);
    on tile[1] : {
      app(c, c_gpio);
    }
    on tile[1] : {
      smi_init(smi0);
      smi_init(smi1);
      powerlink(p, c);
    }
  }
  return 0;
}
