#include <platform.h>
#include <xscope.h>
#include "can.h"
#include "can_utils.h"
#include "can_open.h"
#include <print.h>
#include "i2c.h"
#include "canopen_client.h"

#define I2C_NO_REGISTER_ADDRESS 1
#define debounce_time XS1_TIMER_HZ/50
#define BUTTON_PRESS_VALUE 2

on tile[0]: can_ports p = {XS1_PORT_1L, XS1_PORT_1I, XS1_CLKBLK_1};
on tile[0]: port shutdown = XS1_PORT_4E;

on tile[1]: port p_led = XS1_PORT_4A;
on tile[1]: port p_PORT_BUT_1 = XS1_PORT_4C;
on tile[1]: struct r_i2c i2cOne = {XS1_PORT_1F, XS1_PORT_1B, 1000};

void application(streaming chanend c_application);

void xscope_user_init(void)
{
  xscope_register(0, 0, "", 0, "");
  xscope_config_io(XSCOPE_IO_BASIC);
}

int main()
{
  chan c_rx_tx;
  streaming chan c_application;
  par
  {
    on tile[0]:
    can_open_manager(c_rx_tx, c_application);
    on tile[0]:
    {
      shutdown <: 0;
      can_server(p, c_rx_tx);
    }on tile[1] :
    application(c_application);
  }
  return 0;
}

void application(streaming chanend c_application)
{
  unsigned button_press_1, button_press_2, time, time_i2c;
  int button = 1;
  char toggle0 = 0xF, toggle1 = 0xF, toggle2 = 0xF, toggle3 = 0xF;
  timer t, i2c_timer;
  unsigned char data[1] = {0x13}, pdo_number, pdo_data[2];
  unsigned char data1[2];
  int adc_value;
  p_PORT_BUT_1:> button_press_1;
  set_port_drive_low(p_PORT_BUT_1);
  i2c_master_write_reg(0x28, 0x00, data, 1, i2cOne); //Write configuration information to ADC
  t:>time;
  while(1)
  {
    select
    {
      case button=> p_PORT_BUT_1 when pinsneq(button_press_1):> button_press_1: //checks if any button is pressed
      button=0;
      t:>time;
      break;

      case !button => t when timerafter(time+debounce_time):>void: //waits for 20ms and checks if the same button is pressed or not
      p_PORT_BUT_1:> button_press_2;
      if(button_press_1==button_press_2)
      if(button_press_1 == BUTTON_PRESS_VALUE) //Button 1 is pressed

      {
        pdo_data[0] = 0xFF;
        send_data_to_canopen_stack(c_application, 1, 1, pdo_data);
      }
      if(button_press_1 == BUTTON_PRESS_VALUE-1) //Button 2 is pressed

      {
        pdo_data[0] = 0xFF;
        send_data_to_canopen_stack(c_application, 2, 1, pdo_data);
      }
      button=1;
      break;

      case c_application:> pdo_number:
      {
        char temp_data;
        char length,data;
        receive_data_from_canopen_stack(c_application, length, pdo_data);
        if(pdo_number == 0)
        {
          toggle0=!toggle0;
          p_led:>temp_data;
          p_led<:(unsigned)(toggle0 | 0xE);
        }
        else if(pdo_number == 1)
        {
          p_led:>temp_data;
          toggle1=!toggle1;
          p_led<:(unsigned)((toggle1<<1) | 0xD);
        }
        else if(pdo_number == 2)
        {
          p_led:>temp_data;
          toggle2=!toggle2;
          p_led<:(unsigned)((toggle2<<2) | 0xB);
        }
        else if(pdo_number == 3)
        {
          p_led:>temp_data;
          toggle3=!toggle3;
          p_led<:(unsigned)((toggle3<<3) | 0x7);
        }
      }
      break;

      case i2c_timer when timerafter(time_i2c + 500000000):> time_i2c:
      i2c_master_rx(0x28, data1, 2, i2cOne); //Read ADC value using I2C read
      printstrln("Reading Temperature value....");
      data1[0]=data1[0]&0x0F;
      adc_value=(data1[0]<<6)|(data1[1]>>2);
      pdo_data[0] = ((adc_value & 0xFF00)>>8);
      pdo_data[1] = (adc_value & 0xFF);
      send_data_to_canopen_stack(c_application, 0, 2, pdo_data);
      break;
    }
  }
}
