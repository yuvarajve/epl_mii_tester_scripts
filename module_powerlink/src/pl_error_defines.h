#ifndef PL_ERROR_DEFINES_H_
#define PL_ERROR_DEFINES_H_

//Used to overload the error reporting and instead send a SoC to the eh
#define NO_ERROR_SOC 0

typedef enum {
  Status_Change,
  Error,
  Init
} error_cmd;

enum { signal_error, request_status_response};

typedef enum {
  E_SDO_PROTOCOL_TIMEDOUT = 0x05040000,
  E_CMD_ID_INVALID_UNKNOWN = 0x05040001,
  E_INVALID_BLOCK_SIZE = 0x05040002,
  E_INVALID_SEQ_NUMBER = 0x05040003,
  E_OUT_OF_MEMORY = 0x05040005,
  E_UNSUPPORTED_ACCESS = 0x06010000,
  E_READ_A_WRITE_ONLY = 0x06010001,
  E_WRITE_A_READ_ONLY = 0x06010002,
  E_OBJECT_DOES_NOT_EXIST = 0x06020000,
  E_OBJECT_CANNOT_MAP_TO_PDO = 0x06040041,
  E_PDO_MAP_OVERRUN = 0x06040042,
  E_PARAM_INCOMPATABILITY = 0x06040043,
  E_NMT_INVALID_HEARTBEAT = 0x06040044,
  E_GENERAL_PARAM = 0x06040047,
  E_ACCESS_FAIL = 0x06060000,
  E_DATA_PARAM_MISMATCH = 0x06070010,
  E_DATA_MISMATCH_HIGH = 0x06070012,
  E_DATA_MISMATCH_LOW = 0x06070013,
  E_SUBINDEX_DOES_NOT_EXIST= 0x06090011,
  E_PARAM_EXCEEDED = 0x06090030,
  E_PARAM_TOO_HIGH= 0x06090031,
  E_PARAM_TOO_LOW = 0x06090032,
  E_MAX_LESS_THAN_MIN = 0x06090036,
  E_GENERAL_ERROR = 0x08000000,
  E_DATA_TRANSFER_APP = 0x08000020,
  E_DATA_TRANSFER_LOCAL = 0x08000021,
  E_DATA_TRANSFER_DEVICE = 0x08000022,
  E_NO_OBJECT_DICT = 0x08000023,
  E_CFM_DATA_SET_EMPTY = 0x08000024

} t_sdo_abort_codes;




typedef enum {
  //HW errors
  E_DLL_BAD_PHYS_MODE = 0x8161,
  E_DLL_COLLISION = 0x8162,
  E_DLL_COLLISION_TH = 0x8163,
  E_DLL_CRC_TH = 0x8164,
  E_DLL_LOSS_OF_LINK = 0x8165,
  E_DLL_MAC_BUFFER = 0x8166,

  //Protocol errors
  E_DLL_ADDRESS_CONFLICT = 0x8201,
  E_DLL_MULTIPLE_MN = 0x8202,

  //Frame size errors
  E_PDO_SHORT_RX = 0x8210,
  E_PDO_MAP_VERS = 0x8211,
  E_NMT_ASND_MTU_DIF = 0x8212,
  E_NMT_ASND_MTU_LIM = 0x8213,
  E_NMT_ASND_TX_LIM = 0x8214,

  //Timing errors
  E_NMT_CYCLE_LEN = 0x8231,
  E_DLL_CYCLE_EXCEED = 0x8232,
  E_DLL_CYCLE_EXCEED_TH = 0x8233,
  E_NMT_IDLE_LIM = 0x8234,
  E_DLL_JITTER_TH = 0x8235,
  E_DLL_LATE_PRES_TH = 0x8236,
  E_NMT_PREQ_CN = 0x8237,
  E_NMT_PREQ_LIM = 0x8238,
  E_NMT_PRES_CN = 0x8239,
  E_NMT_PRES_RX_LIM = 0x823a,
  E_NMT_PRES_TX_LIM = 0x823b,

  //Frame errors
  E_DLL_INVALID_FORMAT = 0x8241,
  E_DLL_LOSS_PREQ_TH = 0x8242,
  E_DLL_LOSS_PRES_TH = 0x8243,
  E_DLL_LOSS_SOA_TH = 0x8244,
  E_DLL_LOSS_SOC_TH = 0x8245,
  E_DLL_LOSS_STATUSRES_TH = 0x8246,

  //BootUp Errors
  E_NMT_BA1 = 0x8410,
  E_NMT_BA1_NO_MN_SUPPORT= 0x8411,
  E_NMT_BPO1= 0x8420,
  E_NMT_BPO1_GET_IDENT= 0x8421,
  E_NMT_BPO1_DEVICE_TYPE= 0x8422,
  E_NMT_BPO1_VENDOR_ID= 0x8423,
  E_NMT_BPO1_PRODUCT_CODE= 0x8424,
  E_NMT_BPO1_REVISION_NO= 0x8425,
  E_NMT_BPO1_SERIAL_NO= 0x8426,
  E_NMT_BPO1_CF_VERIFY= 0x8428,
  E_NMT_BPO1_SW_INVALID= 0x8429,
  E_NMT_BPO1_SW_STATE= 0x842a,
  E_NMT_BPO1_SW_UPDATE= 0x842b,
  E_NMT_BPO1_SW_REJECT= 0x842c,
  E_NMT_BPO2= 0x8430,
  E_NMT_BRO= 0x8440,
  E_NMT_WRONG_STATE= 0x8480
} t_error_codes;
#endif /* PL_ERROR_DEFINES_H_ */
