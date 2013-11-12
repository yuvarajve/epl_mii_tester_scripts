#include <xs1.h>
#include <platform.h>
#include <xscope.h>
#include <print.h>
#include "smi.h"
#include "powerlink.h"
#include <stdint.h>
#include "thread_id.h"
#include <assert.h>

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

void xscope_user_init(void) {
  xscope_register(0, 0, "", 0, "");
  xscope_config_io(XSCOPE_IO_BASIC);
}


enum {
  frame_start,
  frame_stop,
} framer_api;
enum {
  start_framer,
  stop_framer,
  set_ack_delta,
  send,
  reset
} host_api;

typedef enum {
  None, //no event
  SOF,  //start of frame
  EOF,  //end of frame
} t_event;

typedef enum {
  dll_id, nmt_id
} t_id;

enum {
  set_ack_turnaround_in_ns,
  send_packet_at_time
} app_to_dll_nmt;

enum {
  packet_received, //the dll or nmt have rxd a frame and are ready to give it to the host
  ready_for_frame //this signals that the dll or nmt are ready to accept a frame for sending
} dll_nmt_to_app;

enum {
  error_received
} eh_to_app;


enum {
  report_error,
  report_packet,
};

#define FRAME_SIZE_LIMIT 1600
#define MAX_FRAME_COUNT 10

typedef struct {
  t_id id;
  unsigned delta;
  unsigned size;
  uintptr_t pointer;
} frame_info;

/*
 * This program runs an mii on circle and square ports of slicekit. The idea is that the app will take its commands from
 * an XTAG which is being controlled by the python test harness.
 */

void xtag_server(chanend app_to_xtag){
//TODO collect and forward the commands from the xtag and forward commands from this device


}


void dut_application(chanend app_to_dll, chanend app_to_nmt, chanend app_to_eh,
    chanend app_to_xtag, chanend app_to_framer_0, chanend app_to_framer_1){

  unsigned frame_start_time, frame_stop_time;

  int dll_ready_for_frame = 1;
  int nmt_ready_for_frame = 1;

  unsigned frames_in_progress = 0;
  unsigned current_frame;
  frame_info frames[MAX_FRAME_COUNT];
  unsigned packet_start_time;

  while(1){
    select {
      case app_to_dll :> unsigned cmd :{
        switch (cmd) {
        case packet_received :{ //TODO a transaction would be faster
          unsigned t;
          app_to_xtag <: report_packet;
          app_to_dll :> t;
          app_to_xtag <: t;//time
          app_to_dll :> t;
          app_to_xtag <: t;//pointer
          app_to_dll :> t;
          app_to_xtag <: t;//size
          break;
        }
        case ready_for_frame : {
          dll_ready_for_frame = 1;
          break;
        }
        default : {
          __builtin_unreachable();
          break;
        }
        }
        break;
      }
      case app_to_nmt :> unsigned cmd :{
        switch (cmd) {
        case packet_received :{ //TODO a transaction would be faster
          unsigned t;
          app_to_xtag <: report_packet;
          app_to_nmt :> t;
          app_to_xtag <: t;//time
          app_to_nmt :> t;
          app_to_xtag <: t;//pointer
          app_to_nmt :> t;
          app_to_xtag <: t;//size
          break;
        }
        case ready_for_frame : {
          nmt_ready_for_frame = 1;
          break;
        }
        default : {
          __builtin_unreachable();
          break;
        }
        }
        break;
      }
      case app_to_eh :> unsigned cmd :{
        switch (cmd) {
        case error_received :{
          unsigned t;
          app_to_xtag <: report_error;
          app_to_nmt :> t;
          app_to_xtag <: t;//time
          for(unsigned i=0;i<20;i++){//TODO finish this
            char x;
            app_to_eh :> x;
            app_to_xtag <: x;
          }
          break;
        }
        default : {
          __builtin_unreachable();
          break;
        }
        }
        break;
      }

      case app_to_framer_0 :> unsigned cmd :{
        switch (cmd) {
        case frame_start: {
          app_to_framer_0 :> frame_start_time;
          break;
        }
        case frame_stop : {
          app_to_framer_0 :> frame_stop_time;
          break;
        }
        default :{
          __builtin_unreachable();
          break;
        }
        }
        break;
      }
      case app_to_framer_1:> unsigned cmd :{
        switch (cmd) {
        case frame_start: {
          app_to_framer_1 :> frame_start_time;
          break;
        }
        case frame_stop : {
          app_to_framer_1 :> frame_stop_time;
          break;
        }
        default :{
          __builtin_unreachable();
          break;
        }
        }
        break;
      }
      case app_to_xtag :> unsigned cmd :{
        switch (cmd) {
        case start_framer :{
          app_to_framer_0 <: start_framer;
          app_to_framer_1 <: start_framer;
          break;
        }
        case stop_framer :{
          app_to_framer_0 <: stop_framer;
          app_to_framer_1 <: stop_framer;
          break;
        }
        case set_ack_delta :{
          unsigned id;
          unsigned turnaround_ns;
          app_to_xtag :> id;
          app_to_xtag :> turnaround_ns;
          if(id == dll_id){
            app_to_dll <: set_ack_turnaround_in_ns;
            app_to_dll <: turnaround_ns;
          } else if (id == nmt_id){
            app_to_nmt <: set_ack_turnaround_in_ns;
            app_to_nmt <: turnaround_ns;
          } else {
           //error
          }
          break;
        }
        case send : {
          unsigned frame_count;

          t_event event;

          app_to_xtag :> frame_count;
          app_to_xtag :> event;

          if(frames_in_progress){
            //FIXME error
          }

          for(unsigned i=0;i<frame_count;i++){
            app_to_xtag :> frames[frames_in_progress].id;
            app_to_xtag :> frames[frames_in_progress].delta;
            app_to_xtag :> frames[frames_in_progress].size;
            app_to_xtag :> frames[frames_in_progress].pointer;
            if(frames[frames_in_progress].size > FRAME_SIZE_LIMIT){
              //FIXME error
            }
          }
          frames_in_progress = frame_count;
          switch(event) {
          case None: {
            timer t;
            t :> packet_start_time;
            packet_start_time += 1000; //add a little to give us some slack
            break;
          }
          case SOF:
          case EOF:{
            unsigned cmd;
            timer t;
            unsigned time;
            //TODO verify the framer is running
            while(1){

              select {
                case app_to_framer_0 :> cmd :{
                  app_to_framer_0 :> time;
                  break;
                }
                case app_to_framer_1 :> cmd :{
                  app_to_framer_1 :> time;
                  break;
                }
              }



              if((cmd == frame_start && event == SOF)
                  || (cmd == frame_stop && event == EOF)){
                packet_start_time = time;
              }
            }
            break;
          }
          }
          current_frame = 0;
          break;
        }
        case reset :{
          //TODO reset the whole system
          break;
        }
        }

        break;
      }
      frames_in_progress=>default : {
        if(frames[current_frame].id == dll_id && dll_ready_for_frame){
          app_to_dll <: send_packet_at_time;
          app_to_dll <: packet_start_time + frames[current_frame].delta;
          app_to_dll <: frames[current_frame].pointer;
          app_to_dll <: frames[current_frame].size;
          current_frame++;
          frames_in_progress--;
          dll_ready_for_frame = 0;
          break;
        }
        if(frames[current_frame].id == nmt_id && nmt_ready_for_frame){
          app_to_nmt <: send_packet_at_time;
          app_to_nmt <: packet_start_time + frames[current_frame].delta;
          app_to_nmt <: frames[current_frame].pointer;
          app_to_nmt <: frames[current_frame].size;
          current_frame++;
          frames_in_progress--;
          nmt_ready_for_frame = 0;
        }
        break;
      }
    }
  }
}

void dut_dll(streaming chanend c_mii_dll, chanend app_to_dll){
  unsigned time_to_ack;
  int need_to_ack = 0;
  unsigned delta = 0;
  timer t;

  unsigned tx_ack_pending = 0;

  while(1){
    select {
      case need_to_ack => t when timerafter(time_to_ack) :> int :{
        c_mii_dll <: 0;
        break;
      }
      case c_mii_dll :> unsigned temp : {
        if(temp){
          unsigned time;
          uintptr_t rx_pointer = temp;
          c_mii_dll :> time;
          t:> time_to_ack;
          time_to_ack += delta;
          assert(need_to_ack == 0);
          need_to_ack = 1;
        } else {
          tx_ack_pending = 0;
          c_mii_dll <: ready_for_frame;
        }
        break;
      }
      case app_to_dll :> unsigned cmd : {
        switch(cmd){
          case set_ack_turnaround_in_ns:{
            app_to_dll :> delta;
            break;
          }
          case send_packet_at_time:{
            unsigned time;

            assert(tx_ack_pending = 0);
            tx_ack_pending = 1;

            break;
          }
        }
        break;
      }
    }
  }
}

void dut_nmt(streaming chanend c_mii_nmt, chanend app_to_nmt){
  dut_dll(c_mii_nmt, app_to_nmt);
}

void dut_error_handling(chanend c_mii_eh, chanend app_to_eh){
  timer t;
  unsigned time;
  while(1){
    select {
      case c_mii_eh :> int x:{
        //TODO check the protocol from the mii - this is undoubtly wrong
        t:> time;
        app_to_eh <: error_received;
        app_to_eh <: time;
        for(unsigned i=0;i<20;i++){
          char x;
          c_mii_eh :> x;
          app_to_eh <: x;
        }
        break;
      }
    }
  }
}

/*
 * SMI must start in 100 Mb/s half duplex with auto-neg off.
 */

void pl_mii(pl_mii_ports & p, streaming chanend c_dll,
    streaming chanend c_nmt, chanend error_handler);

void framer_0(chanend app_to_framer, in buffered port:4 p){
  int framer_running = 0;
  int frame_valid = 0;
  while(1){
#pragma ordered
    select {

      case app_to_framer :> unsigned cmd :{
        switch (cmd) {
        case stop_framer :{
          framer_running = 0;
          break;
        }
        case start_framer :{
          framer_running = 1;
          break;
        }
        }
        break;
      }
      case p :> int :{
        if(frame_valid == 0){
          app_to_framer <: frame_start;
        }
        frame_valid = 1;
        break;
      }
      frame_valid => default :{
        app_to_framer <: stop_framer;
        frame_valid = 0;
        break;
      }
    }
  }
}
void framer_1(chanend app_to_framer, in buffered port:1 p){
  int framer_running = 0;
  int frame_valid = 0;
  while(1){
#pragma ordered
    select {

      case app_to_framer :> unsigned cmd :{
        switch (cmd) {
        case stop_framer :{
          framer_running = 0;
          break;
        }
        case start_framer :{
          framer_running = 1;
          break;
        }
        }
        break;
      }
      case p :> int :{
        if(frame_valid == 0){
          app_to_framer <: frame_start;
        }
        frame_valid = 1;
        break;
      }
      frame_valid => default :{
        app_to_framer <: stop_framer;
        frame_valid = 0;
        break;
      }
    }
  }
}
in buffered port:4 mii_0_p_mii_rxer = PORT_ETH_ERR_0;          /**< MII RX error wire */
in buffered port:1 mii_1_p_mii_rxer = PORT_ETH_ERR_1;          /**< MII RX error wire */

int main(){
  streaming chan c_mii_dll;
  streaming chan c_mii_nmt;
  chan c_eh_mii;

  chan app_to_eh;
  chan app_to_dll;
  chan app_to_nmt;
  chan app_to_framer_0;
  chan app_to_framer_1;
  chan app_to_xtag;

  par {

    on tile[1] : {
      set_core_fast_mode_on();
      xtag_server(app_to_xtag);
    }

    on tile[1] : {
      set_core_fast_mode_on();
      dut_application(app_to_dll, app_to_nmt, app_to_eh, app_to_xtag, app_to_framer_0, app_to_framer_1);
    }
    on tile[1] : {
      set_core_fast_mode_on();
      dut_dll(c_mii_dll, app_to_dll);
    }
    on tile[1] : {
      set_core_fast_mode_on();
      dut_nmt(c_mii_nmt, app_to_nmt);
    }
    on tile[1] : {
      set_core_fast_mode_on();
      dut_error_handling(c_eh_mii, app_to_eh);
    }
    on tile[1] : {
      set_core_fast_mode_on();
      smi_init(smi0);
      smi_init(smi1);

      set_port_clock(mii_0_p_mii_rxer, p.mii_0_clk_mii_rx);
      set_port_clock(mii_1_p_mii_rxer, p.mii_1_clk_mii_rx);

      par {
        framer_0(app_to_framer_0, mii_0_p_mii_rxer);
        framer_1(app_to_framer_1, mii_1_p_mii_rxer);
        pl_mii(p, c_mii_dll, c_mii_nmt, c_eh_mii);
      }

    }
  }
}
