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

#ifndef __sdo_h__
#define __sdo_h__

/*---------------------------------------------------------------------------
typedefs
---------------------------------------------------------------------------*/

/**
* \enum sdo_request_types
* \brief CANOpen Condition values
*/
enum sdo_request_types
{
  EXPEDITED_DWNLD_RQST_4BYTES           = 0x23, /**<CANOpen sdo request 4 bytes data download */
  EXPEDITED_DWNLD_RQST_3BYTES           = 0x27, /**<CANOpen sdo request 3 bytes data download */
  EXPEDITED_DWNLD_RQST_2BYTES           = 0x2B, /**<CANOpen sdo request 2 bytes data download */
  EXPEDITED_DWNLD_RQST_1BYTE            = 0x2F, /**<CANOpen sdo request 1 byte data download */
  INTITATE_DWNLD_DATA_LEN_UNKNOWN       = 0x22, /**<CANOpen sdo request unknown length data download */
  NON_EXPEDITED_DWNLD_REQUEST           = 0x20, /**<CANOpen sdo non expedited download request */
  NON_EXPEDITED_DWNLD_SEGMENTED_REQUEST = 0x21, /**<CANOpen sdo segmenetd download request */
  INITIATE_SDO_UPLOAD_REQUEST           = 0x40  /**<CANOpen sdo initiate upload request */
};

/**
* \enum sdo_abort_codes
* \brief CANOpen sdo abort codes to transmit on canbus to abort sdo transmission
*/
enum sdo_abort_codes
{
  SDO_TOGGLE_BIT_NOT_ALTERED                    = 0x05030000, /**<abort code toggle bit not altered */
  SDO_PROTOCOL_TIME_OUT                         = 0x05040000, /**<abort code sdo protocol timeout */
  SDO_COMMAND_SPECIFIER_NOT_VALID               = 0x05040001, /**<abort code invalid command specifier */
  SDO_INVALID_BLOCK_SIZE                        = 0x05040002, /**<abort code invalid blaock size */
  SDO_INVALID_SEQ_NUMBER                        = 0x05040003, /**<abort code invalid sequence number */
  SDO_CRC_ERROR                                 = 0x05040004, /**<abort code crc error */
  SDO_OUT_OF_MEMORY                             = 0x05040005, /**<abort code out of memory */
  SDO_UNSUPPORTED_ACCESS_OD                     = 0x06010000, /**<abort code unsupported access */
  SDO_ATTEMPT_TO_READ_WO_OD                     = 0x06010001, /**<abort code reading wo data */
  SDO_ATTEMPR_TO_WRITE_RO_OD                    = 0x06010002, /**<abort code writing ro data */
  SDO_NO_OBJECT_IN_OD                           = 0x06020000, /**<abort code object does not exist */
  SDO_OBJ_NOT_MAPPED_TO_PDO                     = 0x06040041, /**<abort code parameter incompatable */
  SDO_GENERAL_PARAMETER_INCOMPATABLE            = 0x06040043, /**<abort code internal incompatable */
  SDO_GENERAL_INTERNAL_INCOMPATABLE             = 0x06040047, /**<abort code general internal incompatable */
  SDO_ACCES_FAIL_HARDWARE_ERROR                 = 0x06060000, /**<abort code access fail hardware */
  SDO_DATA_TYPE_DOES_NOT_MATCH                  = 0x06070010, /**<abort code data type mismatch */
  SDO_NO_SUB_INDEX_EXIST                        = 0x06090011, /**<abort code subindex does not exists */
  SDO_VALUE_RANGE_PARAMETER_EXCEEDED            = 0x06090030, /**<abort code range exceeded */
  SDO_GENERAL_ERROR                             = 0x08000000, /**<abort code general error */
  SDO_OD_DYN_GEN_FAIL                           = 0x08000023 /**<abort code object dictionary dynamic generaion fail */
};

/*---------------------------------------------------------------------------
prototypes
---------------------------------------------------------------------------*/

/*==========================================================================*/
/**
* upload_expedited_data is the function in order to upload expedited sdo data
*
* \param c_rx_tx channel to communicate to the can module
* \param od_index index of object the object dictionary
* \param od_sub_index sub index of object in the object dictionary
* \param data_length data length od object to be uploaded
* \param data_buffer data to be uploaded
* \return none
**/
void upload_expedited_data(chanend c_rx_tx,
                           int od_index,
                           char od_sub_index,
                           char data_length,
                           char data_buffer[]);

/*==========================================================================*/
/**
* send_sdo_download_response is the function in order to send download
* resposne to the master
*
* \param od_index index of object the object dictionary
* \param od_sub_index sub index of object in the object dictionary
* \param c_rx_tx channel to communicate to the can module
* \return none
**/
void send_sdo_download_response(int od_index,
                                char od_sub_index,
                                chanend c_rx_tx);

/*==========================================================================*/
/**
* download_sdo_segment_response is the function in order to send download
* resposne to the master for segmented download
*
* \param c_rx_tx channel to communicate to the can module
* \param sdo_toggle toggle bit to check the toggle response
* \return none
**/
void download_sdo_segment_response(chanend c_rx_tx, char sdo_toggle);


/*==========================================================================*/
/**
* initiate_sdo_upload_response is the function in order to initiate upload
* request to the master
*
* \param c_rx_tx channel to communicate to the can module
* \param od_index index of object the object dictionary
* \param od_sub_index sub index of object in the object dictionary
* \param data_length data length od object to be uploaded
* \return none
**/
void initiate_sdo_upload_response(chanend c_rx_tx,
                                  int od_index,
                                  char od_sub_index,
                                  char data_length);

/*==========================================================================*/
/**
* upload_segmented_data is the function in order to upload segmenetd data
*
* \param c_rx_tx channel to communicate to the can module
* \param od_index index of object the object dictionary
* \param od_sub_index sub index of object in the object dictionary
* \param sdo_toggle toggle bit for sdo segmenetd upload
* \param data_buffer data to be uploaded
* \param segment_number data segment number
* \return none
**/
void upload_segmented_data(chanend c_rx_tx,
                           int od_index,
                           char od_sub_index,
                           char sdo_toggle,
                           char data_length,
                           char data_buffer[],
                           char segment_number);

#endif
