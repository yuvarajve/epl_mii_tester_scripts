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

#ifndef __od_h__
#define __od_h__

/*---------------------------------------------------------------------------
prototypes
---------------------------------------------------------------------------*/

/*==========================================================================*/
/**
* find_data_length is the function in order to find the data length based
* on index and sub index from the object dictionary.
*
* \param address index of the object dictionary entry
* \param sub_index subindex of object dictionary entry
* \return length returns data length od particular object
**/
int find_data_length(int address, unsigned char sub_index);

/*==========================================================================*/
/**
* find_index_in_od is the function in order to find the object index based
* on address from the object dictionary.
*
* \param address index of the object dictionary entry
* \return index returns index position in the object dictionary
**/
int find_index_in_od(int address);

/*==========================================================================*/
/**
* read_data_from_od is the function in order to read data from
* object dictionary based on index, subindex and data length from the
* object dictionary.
*
* \param index index of the object dictionary entry
* \param od_sub_index subindex of the object dictionary entry
* \param data_buffer data buffer to store read data from object dictionary
* \param data_length length of data to be read from object dictionary
* \return index returns index position in the object dictionary
**/
void read_data_from_od(int index,
                       unsigned char od_sub_index,
                       char data_buffer[],
                       unsigned char data_length);

/*==========================================================================*/
/**
* write_data_to_od is the function in order to write data to the
* object dictionary based on index, subindex and data length of the
* object dictionary entry.
*
* \param index index of the object dictionary entry
* \param od_sub_index subindex of the object dictionary entry
* \param data_buffer data buffer to write data to object dictionary
* \param data_length length of data to write data to object dictionary
* \return index returns index position in the object dictionary
**/
void write_data_to_od(int index,
                      unsigned char od_sub_index,
                      char data_buffer[],
                      unsigned char data_length);

/*==========================================================================*/
/**
* find_access_of_od_index is the function in read the accedss type of
* an object from the object dictionary based on index and subindex from the
* object dictionary entry.
*
* \param index index of the object dictionary entry
* \param od_sub_index subindex of the object dictionary entry
* \return index returns index position in the object dictionary
**/
unsigned char find_access_of_od_index(int index, unsigned char od_sub_index);

#endif
