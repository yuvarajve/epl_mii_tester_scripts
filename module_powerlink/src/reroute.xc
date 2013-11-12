#include "powerlink.h"
/*
void reroute(mii_interface &mii_from, mii_interface &mii_to, chanend c_dll, chanend c_nmt){
  unsigned word;
  unsigned poly = 0xEDB88320;
  unsigned crc = 0x9226F562;

  mii_to.p_mii_txd <: 0x55555555;
  mii_to.p_mii_txd <: 0xD5555555;

  while(1){
    select {
      case mii_from.p_mii_rxd :> word:
        mii_to.p_mii_txd <: word;
        break;
      case mii_from.p_mii_rxdv when pinseq(0) :> int: {
        unsigned tail;
        unsigned taillen = endin(mii_from.p_mii_rxd);
        mii_from.p_mii_rxd :> tail;
        if(taillen){
          tail = tail >> (32 - taillen);
          partout(mii_to.p_mii_txd, taillen, tail);
        }
        return;
      }
    }
  }
}
*/
/*
  rxd when pinseq(0xd) :> int;

  while(1){
    select {
      case rxd :> word:

        break;
      case rxdv when pinseq(0) :> int: {
        unsigned tail;
        unsigned taillen = endin(rxd);
        rxd :> tail;
        if(taillen){
          tail = tail >> (32 - taillen);
        }
        return;
      }
    }
  }
  */
