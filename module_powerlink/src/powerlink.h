#ifndef POWERLINK_H_
#define POWERLINK_H_
#include <xs1.h>


typedef struct pl_mii_ports {

  in buffered port:32 mii_0_p_mii_rxd; /**< MII RX data wire */
  in port mii_0_p_mii_rxdv; /**< MII RX data valid wire */
  out buffered port:32 mii_1_p_mii_txd;


  in buffered port:32 mii_1_p_mii_rxd; /**< MII RX data wire */
  in port mii_1_p_mii_rxdv;          /**< MII RX data valid wire */
  out buffered port:32 mii_0_p_mii_txd;


  clock mii_0_clk_mii_rx;            /**< MII RX Clock Block **/
  clock mii_0_clk_mii_tx;            /**< MII TX Clock Block **/
  in port mii_0_p_mii_rxclk;         /**< MII RX clock wire */
 // in port mii_0_p_mii_rxer;          /**< MII RX error wire */
  in port mii_0_p_mii_txclk;       /**< MII TX clock wire */
  out port mii_0_p_mii_txen;       /**< MII TX enable wire */

  clock mii_1_clk_mii_rx;            /**< MII RX Clock Block **/
  clock mii_1_clk_mii_tx;            /**< MII TX Clock Block **/
  in port mii_1_p_mii_rxclk;         /**< MII RX clock wire */
  //in port mii_1_p_mii_rxer;          /**< MII RX error wire */
  in port mii_1_p_mii_txclk;       /**< MII TX clock wire */
  out port mii_1_p_mii_txen;       /**< MII TX enable wire */

} pl_mii_ports;


void powerlink(pl_mii_ports & p, chanend c_app);

#endif /* POWERLINK_H_ */
