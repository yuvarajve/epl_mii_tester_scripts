#include "powerlink.h"

void pl_mii(pl_mii_ports & p, streaming chanend c_dll,
    streaming chanend c_nmt, chanend error_handler);
void pl_dll(streaming chanend c_mii, chanend error_handler, chanend c_nmt, chanend c_can_open);
void pl_nmt(streaming chanend c_mii, chanend error_handler, chanend c_dll, chanend c_can_open);
void pl_error_handling(chanend c_mii, chanend c_dll, chanend c_nmt);
void can_open_interface(chanend c_dll, chanend c_nmt, chanend c_app);



void powerlink(pl_mii_ports & p, chanend c_app){
  streaming chan c_mii_dll;
  streaming chan c_mii_nmt;

  chan c_eh_dll;
  chan c_eh_nmt;
  chan c_eh_mii;

  chan c_dll_co;
  chan c_nmt_co;

  chan c_dll_nmt;
  par {
    pl_mii(p, c_mii_dll, c_mii_nmt, c_eh_mii);
    pl_dll(c_mii_dll, c_eh_dll, c_dll_nmt, c_dll_co);
    pl_nmt(c_mii_nmt, c_eh_nmt, c_dll_nmt, c_nmt_co);
    pl_error_handling(c_eh_mii, c_eh_dll, c_eh_nmt);
    can_open_interface(c_dll_co, c_nmt_co, c_app);
  }
}

