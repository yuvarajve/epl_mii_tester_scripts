#ifndef DEVICE_DESCRIPTION_H_
#define DEVICE_DESCRIPTION_H_

#define FALSE 0
#define TRUE  1

//known

#define D_NMT_ErrorEntries_U32          13        //maximum number of error entries (Status and History Entries) in theStatusResponse frame, value range: 2 .. 13
#define D_DLL_FeatureCN_BOOL            TRUE      //nodes ability to perform CN functions
#define D_DLL_FeatureMN_BOOL            FALSE     //nodes ability to perform MN functions


#define D_NMT_PublishConfigNodes_BOOL   FALSE     //Support of NMT Info service NMTPublishConfiguredNodes
#define D_NMT_PublishTime_BOOL          FALSE     //Support of NMT Info service NMTPublishTime

#define D_SDO_Client_BOOL               FALSE     //device implements an SDO client
#define D_SDO_Server_BOOL               TRUE      //device implements an SDO server
#define D_SDO_CmdFileRead_BOOL          FALSE     //Support of SDO command FileRead
#define D_SDO_CmdFileWrite_BOOL         FALSE     //Support of SDO command FileWrite
#define D_SDO_CmdLinkName_BOOL          FALSE     //Support of SDO command LinkName
#define D_SDO_CmdReadAllByIndex_BOOL    FALSE     //Support of SDO command ReadAllByIndex
#define D_SDO_CmdReadByName_BOOL        FALSE     //Support of SDO command ReadByName
#define D_SDO_CmdReadMultParam_BOOL     FALSE     //Support of SDO command ReadMultipleParam
#define D_SDO_CmdWriteAllByIndex_BOOL   FALSE     //Support of SDO command WriteAllByIndex
#define D_SDO_CmdWriteByName_BOOL       FALSE     //Support of SDO command WriteByName
#define D_SDO_CmdWriteMultParam_BOOL    FALSE     //Support of SDO command WriteMultParam

#define D_PHY_HubIntegrated_BOOL        TRUE      //indicates a hub integrated by the device
#define D_PHY_ExtEPLPorts_U8            2         //number of externally accessible Ethernet POWERLINK ports
#define D_NMT_MNBasicEthernet_BOOL      TRUE      //support of NMT_MS_BASIC_ETHERNET

#define D_NMT_FlushArpEntry_BOOL        FALSE     //Support of NMT Managing Command Service NMTFlushArpEntry
#define D_NMT_NetHostNameSet_BOOL       FALSE     //Support of NMT Managing Command Service NMTNetHostNameSet
#define D_DLL_ErrBadPhysMode_BOOL       FALSE     //Support of Data Link Layer Error recognition: Incorrect physical operation mode

#define D_PDO_Granularity_U8            8         //minimum size of objects to be mapped [bit]
#define D_DLL_CNFeatureMultiplex_BOOL   TRUE      //nodes ability to perform control of multiplexed isochronous communication

#define D_SDO_MaxConnections_U32          1       //max. number of SDO connections
#define D_SDO_MaxParallelConnections_U32  1       //max. number of SDO connections between an SDO client/server pair
#define D_SDO_SeqLayerTxHistorySize_U16   5       //max. number of frames in SDO sequence layer sender history value <= 31

//arbitrary
#define D_NMT_ProductCode_U32           0         //Identity Object Product Code
#define D_NMT_RevisionNo_U32            0         //Identity Object Revision Number


//unknown (set to default)
#define D_CFM_ConfigManager_BOOL        FALSE     //Ability of a node to perform Configuration Manager functions
#define D_DLL_ErrMacBuffer_BOOL         FALSE     //Support of Data Link Layer Error recognition: TX / RX buffer underrun / overrun
#define D_NMT_EmergencyQueueSize_U32    0         //maximum number of history entries in the Error Signaling emergency queue
#define D_NMT_BootTimeNotActive_U32     time      //max. boot time from cold start to NMT_MS_NOT_ACTIVEresp. NMT_CS_NOT_ACTIVE [us]
#define D_NMT_CNSoC2PReq_U32            time      //CN SoC handling maximum time [ns], a subsequent PReq won't be handled before SoC handling was finished
#define D_NMT_CycleTimeGranularity_U32  1         //POWERLINK cycle time granularity [us] Value shall be 1 us if POWERLINK cycle time settings may be taken from a continuum. Otherwise granularity should be a multiple of the base granularity values 100 us or 125 us.
#define D_NMT_CycleTimeMax_U32          time      //maximum POWERLINK cycle time [us]
#define D_NMT_CycleTimeMin_U32          time      //minimum POWERLINK cycle time [us]
#define D_NMT_MaxCNNodeID_U8            239       //maximum Node ID available for regular CNs the entry provides an upper limit to the NodeID available for cross traffic PDO reception from a regular CN
#define D_NMT_MaxCNNumber_U8            239       //maximum number of supported regular CNs in the Node ID range 1 .. 239
#define D_NMT_MaxHeartbeats_U8          254       //number of guard channels
#define D_NMT_NodeIDByHW_BOOL           TRUE      //Ability of a node to support NodeID setup by HW
#define D_NWL_Forward_BOOL              FALSE     //Ability of node to forward datagrams
#define D_NWL_ICMPSupport_BOOL          FALSE     //Support of ICMP
#define D_NWL_IPSupport_BOOL            TRUE      //Ability of the node cummunicate via IP
#define D_PDO_MaxDescrMem_U32           MAX_U32   //maximum cumulative memory consumption of TPDO and RPDO mapping describing objects [byte]
#define D_PDO_RPDOChannelObjects_U8     254       //Number of supported mapped objects per RPDO channel
#define D_PDO_RPDOChannels_U16          256       //number of supported RPDO channels
#define D_PDO_RPDOCycleDataLim_U32      MAX_U32   //maximum sum of data size of RPDO data to be received per cycle [Byte]
#define D_PDO_RPDOOverallObjects_U16    MAX_U16   //maximum number of mapped RPDO objects, sum of all channels
#define D_PDO_SelfReceipt_BOOL          FALSE     //node's ability to receive PDO data transmitted by itself
#define D_PDO_TPDOChannelObjects_U8     254       //maximum Number of mapped objects per TPDO channel
#define D_PDO_TPDOCycleDataLim_U32      MAX_U32   //maximum sum of data size of TPDO data to be transmitted per cycle [Byte]
#define D_PDO_TPDOOverallObjects_U16    MAX_U16   //maximum number of mapped RPDO objects, sum of all channels
#define D_RT1_RT1SecuritySupport_BOOL   FALSE     //Support of Routing Type 1 security functions
#define D_RT1_RT1Support_BOOL           FALSE     //Support of Routing Type 1 functions
#define D_RT2_RT2Support_BOOL           FALSE     //Support of Routing Type 2 functions


//only for MN
#define D_DLL_ErrMNMultipleMN_BOOL      FALSE     //Support of MN Data Link Layer Error recognition: Multiple MNs
#define D_DLL_MNFeatureMultiplex_BOOL   FALSE     //MNs ability to perform control of multiplexed isochronous communication
#define D_DLL_MNFeaturePResTx_BOOL      FALSE     //MNs ability to transmit PRes
#define D_NMT_MNASnd2SoC_U32            0         //minimum delay between end of reception of ASnd and start of transmission of SoC [ns]
#define D_NMT_MNMultiplCycMax_U8        0         //maximum number of POWERLINK cycles per multiplexed cycle
#define D_NMT_MNPRes2PReq_U32           0         //delay between end of PRes reception and start of PReq transmission [ns]
#define D_NMT_MNPRes2PRes_U32           0         //delay between end of reception of PRes from CNn and start of transmission of PRes by MN [ns]
#define D_NMT_MNPResRx2SoA_U32          0         //delay between end of reception of PRes from CNn and start of transmission of SoA by MN [ns]
#define D_NMT_MNPResTx2SoA_U32          0         //delay between end of PRes transmission by MN and start of transmission of SoA by MN [ns]
#define D_NMT_MNSoA2ASndTx_U32          0         //delay between end of transmission of SoA and start of transmission of ASnd by MN [ns]
#define D_NMT_MNSoC2PReq_U32            0         //MN minimum delay between end of SoC transmission and start of PReq transmission [ns]
#define D_NMT_NetTime_BOOL              FALSE     //Support of NetTime transmission via SoC
#define D_NMT_NetTimeIsRealTime_BOOL    FALSE     //Support of real time via NetTime in SoC

#define D_NMT_PublishActiveNodes_BOOL   FALSE     //Support of NMT Info service NMTPublishActiveNodes
#define D_NMT_PublishEmergencyNew_BOOL  FALSE     //Support of NMT Info service NMTPublishEmergencyNew
#define D_NMT_PublishNodeState_BOOL     FALSE     //Support of NMT Info service NMTPublishNodeStates
#define D_NMT_PublishOperational_BOOL   FALSE     //Support of NMT Info service NMTPublishOperational
#define D_NMT_PublishPreOp1_BOOL        FALSE     //Support of NMT Info service NMTPublishPreOperational1
#define D_NMT_PublishPreOp2_BOOL        FALSE     //Support of NMT Info service NMTPublishPreOperational2
#define D_NMT_PublishReadyToOp_BOOL     FALSE     //Support of NMT Info service NMTPublishReadyToOperate
#define D_NMT_PublishStopped_BOOL       FALSE     //Support of NMT Info service NMTPublishStopped
#define D_NMT_RelativeTime_BOOL         FALSE     //Support of RelativeTime transmission via SoC

#define D_NMT_SimpleBoot_BOOL           FALSE     //Ability of a MN node to perform only Simple Boot Process, if not set Indivual Boot Process shall be proviced
#define D_PDO_TPDOChannels_U16          0         //number of supported TPDO channels


//Feature Flags

#define FEATURE_Isochronous               TRUE
#define FEATURE_SDO_by_UDPIP              TRUE
#define FEATURE_SDO_by_ASnd               TRUE
#define FEATURE_SDO_by_PDO                FALSE
#define FEATURE_NMT_Info_Services         FALSE
#define FEATURE_Ext_NMT_State_Cmds        FALSE
#define FEATURE_Dynamic_PDO_Mapping       FALSE
#define FEATURE_NMT_Service_by_UDPIP      TRUE

#define FEATURE_Configuration_Manager     D_CFM_ConfigManager_BOOL
#define FEATURE_Multiplexed_Access        D_DLL_CNFeatureMultiplex_BOOL
#define FEATURE_NodeID_setup_by_SW        0
#define FEATURE_MN_Basic_Ethernet_Mode    D_NMT_MNBasicEthernet_BOOL
#define FEATURE_Routing_Type_1_Support    D_RT1_RT1Support_BOOL
#define FEATURE_Routing_Type_2_Support    D_RT2_RT2Support_BOOL
#define FEATURE_SDO_RW_All_by_Index       (D_SDO_CmdReadAllByIndex_BOOL&D_SDO_CmdWriteAllByIndex_BOOL)
#define FEATURE_SDO_RW_Mul_Param_by_Index (D_SDO_CmdReadMultParam_BOOL&D_SDO_CmdWriteMultParam_BOOL)



#define FEATURE_FLAG_OCTET_0 (FEATURE_Isochronous<<0) |  (FEATURE_SDO_by_UDPIP<<1) |  \
                             (FEATURE_SDO_by_ASnd<<2) |  (FEATURE_SDO_by_PDO<<3) |  \
                             (FEATURE_NMT_Info_Services<<4) |  (FEATURE_Ext_NMT_State_Cmds<<5) |  \
                             (FEATURE_Dynamic_PDO_Mapping<<6) |  (FEATURE_NMT_Service_by_UDPIP<<7)

#define FEATURE_FLAG_OCTET_1 (FEATURE_Configuration_Manager<<0) |  (FEATURE_Multiplexed_Access<<1) |  \
                             (FEATURE_NodeID_setup_by_SW<<2) |  (FEATURE_MN_Basic_Ethernet_Mode<<3) |  \
                             (FEATURE_Routing_Type_1_Support<<4) |  (FEATURE_Routing_Type_2_Support<<5) |  \
                             (FEATURE_SDO_RW_All_by_Index<<6) |  (FEATURE_SDO_RW_Mul_Param_by_Index<<7)

// Other stuff

#define MAC_0 0x00
#define MAC_1 0x22
#define MAC_2 0x97
#define MAC_3 0x01
#define MAC_4 0x00
#define MAC_5 0x00

#define IP_0 192
#define IP_1 168
#define IP_2 100
#define IP_3 1

#define SN_0 255
#define SN_1 255
#define SN_2 255
#define SN_3 0

#define GW_0 192
#define GW_1 168
#define GW_2 100
#define GW_3 254


#endif /* DEVICE_DESCRIPTION_H_ */
