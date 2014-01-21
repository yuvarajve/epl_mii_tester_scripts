#include "powerlink.h"

// Timing tuning constants
#define PAD_DELAY_RECEIVE    0
#define PAD_DELAY_TRANSMIT   0
#define CLK_DELAY_RECEIVE    0
#define CLK_DELAY_TRANSMIT   7

#define MII_TEST_MODE

//TODO use configure stuff - see richard
//TODO make a lot smaller!
static void mii_init_full(pl_mii_ports &m) {
  set_port_use_on(m.mii_0_p_mii_rxclk);
  m.mii_0_p_mii_rxclk :> int x;
  set_port_use_on(m.mii_0_p_mii_rxd);
  set_port_use_on(m.mii_0_p_mii_rxdv);
  //set_port_use_on(m.mii_0_p_mii_rxer);

  set_pad_delay(m.mii_0_p_mii_rxclk, PAD_DELAY_RECEIVE);

  set_port_strobed(m.mii_0_p_mii_rxd);
  set_port_slave(m.mii_0_p_mii_rxd);

  set_clock_on(m.mii_0_clk_mii_rx);
  set_clock_src(m.mii_0_clk_mii_rx, m.mii_0_p_mii_rxclk);
  set_clock_ready_src(m.mii_0_clk_mii_rx, m.mii_0_p_mii_rxdv);
  set_port_clock(m.mii_0_p_mii_rxd, m.mii_0_clk_mii_rx);
  set_port_clock(m.mii_0_p_mii_rxdv, m.mii_0_clk_mii_rx);

  set_clock_rise_delay(m.mii_0_clk_mii_rx, CLK_DELAY_RECEIVE);

  start_clock(m.mii_0_clk_mii_rx);

  clearbuf(m.mii_0_p_mii_rxd);

  set_port_use_on(m.mii_0_p_mii_txclk);
  set_port_use_on(m.mii_0_p_mii_txd);
  set_port_use_on(m.mii_0_p_mii_txen);

  set_pad_delay(m.mii_0_p_mii_txclk, PAD_DELAY_TRANSMIT);

  m.mii_0_p_mii_txd <: 0;
  m.mii_0_p_mii_txen <: 0;
  sync(m.mii_0_p_mii_txd);
  sync(m.mii_0_p_mii_txen);

  set_port_strobed(m.mii_0_p_mii_txd);
  set_port_master(m.mii_0_p_mii_txd);
  clearbuf(m.mii_0_p_mii_txd);

  set_port_ready_src(m.mii_0_p_mii_txen, m.mii_0_p_mii_txd);
  set_port_mode_ready(m.mii_0_p_mii_txen);

  set_clock_on(m.mii_0_clk_mii_tx);
  set_clock_src(m.mii_0_clk_mii_tx, m.mii_0_p_mii_txclk);
  set_port_clock(m.mii_0_p_mii_txd, m.mii_0_clk_mii_tx);
  set_port_clock(m.mii_0_p_mii_txen, m.mii_0_clk_mii_tx);

  set_clock_fall_delay(m.mii_0_clk_mii_tx, CLK_DELAY_TRANSMIT);

  start_clock(m.mii_0_clk_mii_tx);

  clearbuf(m.mii_0_p_mii_txd);

  ///////////////////////////////////

  set_port_use_on(m.mii_1_p_mii_rxclk);
  m.mii_1_p_mii_rxclk :> int x;
  set_port_use_on(m.mii_1_p_mii_rxd);
  set_port_use_on(m.mii_1_p_mii_rxdv);
  //set_port_use_on(m.mii_1_p_mii_rxer);

  set_pad_delay(m.mii_1_p_mii_rxclk, PAD_DELAY_RECEIVE);

  set_port_strobed(m.mii_1_p_mii_rxd);
  set_port_slave(m.mii_1_p_mii_rxd);

  set_clock_on(m.mii_1_clk_mii_rx);
  set_clock_src(m.mii_1_clk_mii_rx, m.mii_1_p_mii_rxclk);
  set_clock_ready_src(m.mii_1_clk_mii_rx, m.mii_1_p_mii_rxdv);
  set_port_clock(m.mii_1_p_mii_rxd, m.mii_1_clk_mii_rx);
  set_port_clock(m.mii_1_p_mii_rxdv, m.mii_1_clk_mii_rx);

  set_clock_rise_delay(m.mii_1_clk_mii_rx, CLK_DELAY_RECEIVE);

  start_clock(m.mii_1_clk_mii_rx);

  clearbuf(m.mii_1_p_mii_rxd);

  set_port_use_on(m.mii_1_p_mii_txclk);
  set_port_use_on(m.mii_1_p_mii_txd);
  set_port_use_on(m.mii_1_p_mii_txen);

  set_pad_delay(m.mii_1_p_mii_txclk, PAD_DELAY_TRANSMIT);

  m.mii_1_p_mii_txd <: 0;
  m.mii_1_p_mii_txen <: 0;
  sync(m.mii_1_p_mii_txd);
  sync(m.mii_1_p_mii_txen);

  set_port_strobed(m.mii_1_p_mii_txd);
  set_port_master(m.mii_1_p_mii_txd);
  clearbuf(m.mii_1_p_mii_txd);

  set_port_ready_src(m.mii_1_p_mii_txen, m.mii_1_p_mii_txd);
  set_port_mode_ready(m.mii_1_p_mii_txen);

  set_clock_on(m.mii_1_clk_mii_tx);
  set_clock_src(m.mii_1_clk_mii_tx, m.mii_1_p_mii_txclk);
  set_port_clock(m.mii_1_p_mii_txd, m.mii_1_clk_mii_tx);
  set_port_clock(m.mii_1_p_mii_txen, m.mii_1_clk_mii_tx);

  set_clock_fall_delay(m.mii_1_clk_mii_tx, CLK_DELAY_TRANSMIT);

  start_clock(m.mii_1_clk_mii_tx);

  clearbuf(m.mii_1_p_mii_txd);
}

extern void mii(pl_mii_ports & p, streaming chanend c_dll,
    streaming chanend c_nmt, chanend c_error_handler);

void pl_mii(pl_mii_ports & p, streaming chanend c_dll,
    streaming chanend c_nmt, chanend c_error_handler){
  mii_init_full(p);
  mii(p, c_dll, c_nmt, c_error_handler);
}
