#ifndef PL_GENERAL_PURPOSE_CONSTANTS_H_
#define PL_GENERAL_PURPOSE_CONSTANTS_H_

//App. 3.9 General Purpose Constants

#define C_ADR_BROADCAST	 		      0xFF 	//POWERLINK broadcast address
#define C_ADR_DIAG_DEF_NODE_ID 	  0xFD 	//POWERLINK default address of dignosticdevice
#define C_ADR_DUMMY_NODE_ID 	    0xFC 	//POWERLINK dummy node address
#define C_ADR_SELF_ADR_NODE_ID 	  0xFB 	//POWERLINK pseudo node address to be used by a node to adress itself
#define C_ADR_INVALID	 		        0x00 	//invalid POWERLINK address
#define C_ADR_MN_DEF_NODE_ID 	    0xF0 	//POWERLINK default address of MN
#define C_ADR_RT1_DEF_NODE_ID 	  0xFE 	//POWERLINK default address of router type 1
#define C_DLL_ASND_PRIO_NMTRQST	  7		//increased ASnd request priority to be used by NMT Requests
#define C_DLL_ASND_PRIO_STD 	    0		//standard ASnd request priority
#define C_DLL_ETHERTYPE_EPL 	    0x88AB	//
#define C_DLL_ISOCHR_MAX_PAYL 	  36 	//maximum size of PReq and PRes payload data, requires C_DLL_MAX_ASYNC_MTU
#define C_DLL_MAX_ASYNC_MTU 	    1500 	//maximum asynchronous payload in bytes including all headers (exclusive the Ethernet header)
#define C_DLL_MAX_PAYL_OFFSET 	  36 	//maximum offset of Ethernet frame payload, requires C_DLL_MAX_ASYNC_MTU
#define C_DLL_MAX_RS 			        7		//
#define C_DLL_MIN_ASYNC_MTU 	    300		//minimum asynchronous payload in bytes including all headers (exclusive the Ethernet header)
#define C_DLL_MIN_PAYL_OFFSET 	  45		//minimum offset of Ethernet frame payload

#define C_DLL_PREOP1_START_CYCLES 10	//number of unassigning SoA frames at start of NMT_MS_PRE_OPERATIONAL_1
#define C_DLL_T_BITTIME 		      10		//Transmission time per bit on 100 Mbit/s network
#define C_DLL_T_EPL_PDO_HEADER 	  10		//size of PReq and PRes POWERLINK PDO message header
#define C_DLL_T_ETH2_WRAPPER 	    18		//size of Ethernet type II wrapper consisting of header and checksum
#define C_DLL_T_IFG 			        960		//Ethernet Interframe Gap
#define C_DLL_T_MIN_FRAME 		    5120 	//Size of minimum Ethernet frame (without preamble and start-of-frame-delimiter)
#define C_DLL_T_PREAMBLE 		      640 	//Size of Ethernet frame preamble plus start-of-frame-delimiter
#define C_ERR_MONITOR_DELAY 	    10 		//Error monitoring start delay
#define C_IP_ADR_INVALID 		      0x00000000
#define C_IP_INVALID_MTU 		      0		//invalid MTU size used to indicate no change
#define C_NMT_STATE_TOLERANCE 	  5		//maximum reaction time to NMT state commands
#define C_NMT_STATREQ_CYCLE 	    5		//StatusRequest cycle time to be applied to AsyncOnly CNs
#define C_SDO_EPL_PORT 			      3819 	//port to be used POWERLINK specific UDP/IP frames

#endif /* PL_GENERAL_PURPOSE_CONSTANTS_H_ */
