/**
* The copyrights, all other intellectual and industrial
* property rights are retained by XMOS and/or its licensors.
* Terms and conditions covering the use of this code can
* be found in the Xmos End User License Agreement.
*
* Copyright XMOS Ltd 2012
*
* In the case where this code is a modification of existing code
* under a separate license, the separate license terms are shown
* below. The modifications to the code are still covered by the
* copyright notice above.
*
**/

/*---------------------------------------------------------------------------
 include files
 ---------------------------------------------------------------------------*/
#include "od.h"

/*---------------------------------------------------------------------------
 Find PDO transmission type from the object dictionary based on address
 ---------------------------------------------------------------------------*/
char find_pdo_transmission_type(int address)
{
  char od_sub_index = 2;
  int index = find_index_in_od(address);
  char data_buffer[2];
  read_data_from_od(index, od_sub_index, data_buffer, 1);
  return data_buffer[0];
}

/*---------------------------------------------------------------------------
 Find PDO event type from the object dictionary based on address
 ---------------------------------------------------------------------------*/
unsigned find_pdo_event_type(int address)
{
  char od_sub_index = 5;
  unsigned event_time;
  int index = find_index_in_od(address);
  char data_buffer[2];
  read_data_from_od(index, od_sub_index, data_buffer, 2);
  event_time  = data_buffer[0] | (data_buffer[1] << 8);
  return event_time;
}

/*---------------------------------------------------------------------------
 Write data to the object dictionary based on address
 ---------------------------------------------------------------------------*/
char write_pdo_data_to_od(int address, char data_buffer[])
{
  int index = find_index_in_od(address), temp_index;
  char entries[4], count = 0, no_of_entries, sub_index, data_length,
      data_counter = 0;
  read_data_from_od(index, 0, entries, 1);
  no_of_entries = entries[0];
  while(count != no_of_entries)
  {
    read_data_from_od(index, count + 1, entries, 4);
    address = (entries[2]) | (entries[3] << 8);
    sub_index = entries[1];
    data_length = entries[0];
    count++;
    if (data_length == 8)
    {
      temp_index = find_index_in_od(address);
      write_data_to_od(temp_index, sub_index, &data_buffer[data_counter], 1);
      data_counter++;
    }
    else if (data_length == 16)
    {
      temp_index = find_index_in_od(address);
      write_data_to_od(temp_index, sub_index, &data_buffer[data_counter], 2);
      data_counter += 2;
    }
    if (data_length == 32)
    {
      temp_index = find_index_in_od(address);
      write_data_to_od(temp_index, sub_index, &data_buffer[data_counter], 4);
      data_counter += 4;
    }
  }
  return data_counter;
}

/*---------------------------------------------------------------------------
 Find PDO cobid from the object dictionary based on address
 ---------------------------------------------------------------------------*/
unsigned find_pdo_cob_id(int address)
{
  char od_sub_index = 1;
  unsigned cob_id = 0x00;
  int index = find_index_in_od(address);
  char data_buffer[4];
  read_data_from_od(index, od_sub_index, data_buffer, 4);
  cob_id = ((data_buffer[3] << 8) | (data_buffer[2]));
  cob_id = (cob_id << 16) | ((data_buffer[1] << 8) | (data_buffer[0]));
  return cob_id;
}

/*---------------------------------------------------------------------------
 Find PDO inhibit time from the object dictionary based on address
 ---------------------------------------------------------------------------*/
unsigned find_pdo_inhibit_time(int address)
{
  char od_sub_index = 3;
  unsigned inhibit_time = 0x00;
  int index = find_index_in_od(address);
  char data_buffer[4];
  read_data_from_od(index, od_sub_index, data_buffer, 2);
  inhibit_time = ((data_buffer[1] << 8) | (data_buffer[0]));
  return inhibit_time;
}

/*---------------------------------------------------------------------------
 Read PDO data from OD and transmit on to the can bus based on the tx type
 ---------------------------------------------------------------------------*/
char read_pdo_data_from_od(unsigned comm_parameter,
                           unsigned mapping_parameter,
                           char data_buffer[8])
{
  int index = find_index_in_od(mapping_parameter), temp_index;
  char entries[4], count = 0, no_of_entries, sub_index, data_length,
      data_counter = 0;
  unsigned cob_id;
  read_data_from_od(index, 0, entries, 1);
  no_of_entries = entries[0];
  cob_id = find_pdo_cob_id(comm_parameter);
  while(count != no_of_entries)
  {
    read_data_from_od(index, count + 1, entries, 4);
    mapping_parameter = (entries[2]) | (entries[3] << 8);
    sub_index = entries[1];
    data_length = entries[0];
    count++;
    if (data_length == 8)
    {
      temp_index = find_index_in_od(mapping_parameter);
      read_data_from_od(temp_index, sub_index, &data_buffer[data_counter], 1);
      data_counter++;
    }
    else if (data_length == 16)
    {
      temp_index = find_index_in_od(mapping_parameter);
      read_data_from_od(temp_index, sub_index, &data_buffer[data_counter], 2);
      data_counter += 2;
    }
    if (data_length == 32)
    {
      temp_index = find_index_in_od(mapping_parameter);
      read_data_from_od(temp_index, sub_index, &data_buffer[data_counter], 4);
      data_counter += 4;
    }
  }
  return (data_counter);
}
