#include <xs1.h>
#include <print.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <platform.h>
#include "xassert.h"
#include "common.h"
#include <xscope.h>
#include "debug_print.h"

/*
 *
 */
void data_handler(server interface xscope_config i_xscope_config,
                  server interface data_manager i_data_manager)
{
  packet_control_t packet_control[MAX_PACKET_SEQUENCE];
  int eop_flag = 0,status_awaited = 0;
  char buff_access_flag = 0;
  unsigned char pkt_seq_tobe_sent = 0;

  while(1) {
    select {

      case i_xscope_config.put_buffer(unsigned int xscope_buff[]): {
        unsigned packet_number = 0;
        if( (!buff_access_flag) && (!eop_flag) ){
          buff_access_flag = 1;

          packet_number = (GET_PACKET_NO(xscope_buff[0]) % END_OF_PACKET_SEQUENCE);
          assert(packet_number < MAX_PACKET_SEQUENCE);

          packet_control[packet_number].frame_delay = GET_FRAME_DELAY(xscope_buff[0]);
          packet_control[packet_number].frame_size = GET_FRAME_SIZE(xscope_buff[0]);
          packet_control[packet_number].frame_crc = xscope_buff[1];

          if( (!eop_flag) && (GET_PACKET_NO(xscope_buff[0]) & END_OF_PACKET_SEQUENCE)) {
              pkt_seq_tobe_sent = packet_number+1;      /**< Added '1' since packet number always starts with '0' */
              eop_flag = 1;
          }

          buff_access_flag = 0;
        }
        else {
          debug_printf("Frame Arrived During buffer handling or During Tx\n");
        }
        break;
      }

      case i_data_manager.get_packet(packet_control_t p[]) -> unsigned char return_value:
        memcpy(p,packet_control,pkt_seq_tobe_sent * sizeof(packet_control_t));
        return_value = pkt_seq_tobe_sent;
        pkt_seq_tobe_sent = 0;
        break;

      case i_data_manager.status(status_info_t status):
        status_awaited--;

        if (status_awaited == 0)
          // Inform the host that that test sequence is finished
          xscope_int(MII_TESTER_ACK, 2);

        break;

      (eop_flag && !status_awaited) => default:
          i_data_manager.packet_arrived();
          status_awaited = 2;
          eop_flag = 0;

        break;

    }

  }
}

void data_controller(client interface data_manager i_data_manager,
                     client interface tx_config i_tx_config,
                     client interface rx_config i_rx_config)
{
  unsigned char num_of_pkt_arrived=0;
  unsigned char num_of_pkt_sent=0,num_of_pkt_to_send = 0;
  unsigned char num_of_pkt_recd=0;
  packet_control_t pkt_ctrl[MAX_PACKET_SEQUENCE];
  tx_packet_info_t txpkt_info[MAX_PACKET_SEQUENCE];
  rx_packet_info_t rxpkt_info[MAX_PACKET_SEQUENCE];
  tx_port_t tx_port = TX_PORT_0;
  rx_port_t rx_port = RX_PORT_0;

  while(1) {
    select {

      case i_data_manager.packet_arrived():
        num_of_pkt_arrived = i_data_manager.get_packet(pkt_ctrl);
        break;

      case i_tx_config.tx_completed():
        num_of_pkt_sent = i_tx_config.get_tx_pkt_info(txpkt_info,&tx_port);
        status_info_t tx_status = TX_SUCCESS;
        if(num_of_pkt_sent == num_of_pkt_to_send) {
          for(int i=0;i<num_of_pkt_sent;i++) {
            if(txpkt_info[i].no_of_bytes == 0){
              tx_status = TX_ERROR;
              debug_printf("ERROR: Tx_%d NO!! eth data Transmitted\n",tx_port);
            }
            else if(txpkt_info[i].no_of_bytes != pkt_ctrl[i].frame_size){
              tx_status = TX_ERROR;
              debug_printf("ERROR: Tx_%d Packet %d: length mismatch (%d != expected %d)\n", tx_port,
                  i,txpkt_info[i].no_of_bytes, pkt_ctrl[i].frame_size);
            }
            else if(txpkt_info[i].checksum != pkt_ctrl[i].frame_crc){
              tx_status = TX_ERROR;
              debug_printf("ERROR: Tx_%d Packet %d: checksum invalid (%d != expected %d)\n", tx_port,
                  i,txpkt_info[i].checksum, pkt_ctrl[i].frame_crc);
            }
          }
        } else {
          tx_status = TX_ERROR;
          debug_printf("ERROR: Tx_%d %d packets sent instead of %d\n",tx_port,
              num_of_pkt_sent, num_of_pkt_to_send);
        }
        i_data_manager.status(tx_status);
        break;

      case i_rx_config.rx_completed():
        num_of_pkt_recd = i_rx_config.get_rx_pkt_info(rxpkt_info,&rx_port);
        status_info_t rx_status = RX_SUCCESS;
        if(num_of_pkt_recd == num_of_pkt_to_send) {
          for(int i=0;i<num_of_pkt_recd;i++) {
            if(rxpkt_info[i].no_of_bytes == 0){
              rx_status = RX_ERROR;
              debug_printf("ERROR: Rx_%d NO!! eth data Received\n",tx_port);
            }
            else if(rxpkt_info[i].no_of_bytes != pkt_ctrl[i].frame_size){
              rx_status = RX_ERROR;
              debug_printf("ERROR: Rx_%d Packet %d: length mismatch (%d != expected %d)\n", rx_port,
                  i,rxpkt_info[i].no_of_bytes, pkt_ctrl[i].frame_size);
            }
            else if(rxpkt_info[i].checksum != pkt_ctrl[i].frame_crc){
              rx_status = RX_ERROR;
              debug_printf("ERROR: Rx_%d Packet %d: checksum invalid (%d != expected %d)\n", rx_port,
                  i,rxpkt_info[i].checksum, pkt_ctrl[i].frame_crc);
            }
            else {
              if(i<num_of_pkt_recd-1) {

                unsigned int ifg_delay=0;
                if(rxpkt_info[i+1].rx_start_tick > rxpkt_info[i].rx_start_tick)
                    ifg_delay = rxpkt_info[i+1].rx_start_tick - rxpkt_info[i].rx_start_tick;
                else
                    ifg_delay = ((0xFFFFFFFF - rxpkt_info[i+1].rx_start_tick) + rxpkt_info[i].rx_start_tick);

                // Neglect no_of_bytes received, crc, eth preamble bytes from ifg delay time
                ifg_delay = ifg_delay - ((rxpkt_info[i].no_of_bytes + CRC_BYTES + PREAMBLE_BYTES) * 8);
                ifg_delay = ifg_delay - IFG_COMPENSATION_FACTOR;  //default compensation (code,PHY,etc)

                if(ifg_delay < MIN_IFG_DELAY)
                  debug_printf("ERROR: Rx_%d ifg delay between frame %d to %d : (%d) for (%d) bytes\n",rx_port,i,i+1,ifg_delay,rxpkt_info[i].no_of_bytes+CRC_BYTES);
              }
            }
          }
        } else {
          rx_status = RX_ERROR;
          debug_printf("ERROR: Rx_%d %d packets received instead of %d\n",rx_port,
              num_of_pkt_recd, num_of_pkt_to_send);
        }
        i_data_manager.status(rx_status);
        break;

      num_of_pkt_arrived => default:
        i_rx_config.put_packet_num_to_rx(num_of_pkt_arrived);
        i_tx_config.put_packet_ctrl_to_tx(pkt_ctrl,num_of_pkt_arrived);
        num_of_pkt_to_send = num_of_pkt_arrived;
        num_of_pkt_arrived = 0;
        break;
     }
  }

}
