/*******************************************************************************
** Copyright 2019-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** User configuration of the Anybus objects implementation.
**
** All configurations in this file should be possible to override if defined
** in abcc_driver_config.h. This allows the target platform to adapt the
** product configuration to realize its implementation for specific needs.
********************************************************************************
*/

#ifndef ABCC_OBJ_CFG_H_
#define ABCC_OBJ_CFG_H_

#include "abcc_driver_config.h"

#ifndef ABCC_API_CONFIG_H_
#error "Do not include abcc_api_identification.h, include abcc_api_config.h instead."
#endif

/*------------------------------------------------------------------------------
** Supported host-side network objects - Set to 1 to include and enable the
** handler for the corresponding network object.
**------------------------------------------------------------------------------
*/
#ifndef CIET_OBJ_ENABLE
   #define CIET_OBJ_ENABLE                         0
#endif
#ifndef CFN_OBJ_ENABLE
   #define CFN_OBJ_ENABLE                          0
#endif
#ifndef EPL_OBJ_ENABLE
   #define EPL_OBJ_ENABLE                          0
#endif
#ifndef BAC_OBJ_ENABLE
   #define BAC_OBJ_ENABLE                          0
#endif
#ifndef ECT_OBJ_ENABLE
   #define ECT_OBJ_ENABLE                          0
#endif
#ifndef PRT_OBJ_ENABLE
   #define PRT_OBJ_ENABLE                          0
#endif
#ifndef CCL_OBJ_ENABLE
   #define CCL_OBJ_ENABLE                          0
#endif
#ifndef EIP_OBJ_ENABLE
   #define EIP_OBJ_ENABLE                          0
#endif
#ifndef MOD_OBJ_ENABLE
   #define MOD_OBJ_ENABLE                          0
#endif
#ifndef COP_OBJ_ENABLE
   #define COP_OBJ_ENABLE                          0
#endif
#ifndef DEV_OBJ_ENABLE
   #define DEV_OBJ_ENABLE                          0
#endif
#ifndef DPV1_OBJ_ENABLE
   #define DPV1_OBJ_ENABLE                         0
#endif

/*------------------------------------------------------------------------------
** Supported host-side objects - Set to 1 to enable
**------------------------------------------------------------------------------
*/
#ifndef SAFE_OBJ_ENABLE
   #define SAFE_OBJ_ENABLE                         0
#endif
#ifndef SYNC_OBJ_ENABLE
   #define SYNC_OBJ_ENABLE                         ABCC_CFG_SYNC_ENABLED
#endif
#ifndef ETN_OBJ_ENABLE
   #define ETN_OBJ_ENABLE                          0
#endif
#ifndef OPCUA_OBJ_ENABLE
   #define OPCUA_OBJ_ENABLE                        0
#endif
#ifndef MQTT_OBJ_ENABLE
   #define MQTT_OBJ_ENABLE                         0
#endif
#ifndef ASM_OBJ_ENABLE
   #define ASM_OBJ_ENABLE                          0
#endif

/*------------------------------------------------------------------------------
** Supported module-side objects - Set to 1 to enable
**------------------------------------------------------------------------------
*/
#ifndef ANB_FSI_OBJ_ENABLE
   #define ANB_FSI_OBJ_ENABLE                      0
#endif

/*------------------------------------------------------------------------------
** The max. number of *concurrent FSI operations* that the FSI object keeps
** track of. Affects static memory consumption inside the FSI object.
**
** NOTE: This is *not* the same as the max. number of existing FSI instance or
** max. number of open files/directories.
**------------------------------------------------------------------------------
*/
#ifndef ANB_FSI_MAX_CONCURRENT_OPERATIONS
   #define ANB_FSI_MAX_CONCURRENT_OPERATIONS       ( 4 )
#endif


/*------------------------------------------------------------------------------
** MQTT Object (0xE2)
**------------------------------------------------------------------------------
*/
#if MQTT_OBJ_ENABLE
/*
** Attribute 1: MQTT mode (UINT8)
** Refer to abp_mqtt.h for details.
*/
#ifndef MQTT_IA_MODE_ENABLE
   #define MQTT_IA_MODE_ENABLE                     0
   #define MQTT_IA_MODE_VALUE                      ABP_MQTT_MODE_DISABLED
#endif

/*
** Attribute 2: MQTT last will configuration
** Struct of:
** UINT8          - QoS level (0/1/2)
** BOOL           - Retain bit (TRUE/FALSE)
** UINT16         - Topic string length (0-128)
** Array of CHAR  - Topic string (0x00-0xFF),
**                  length of array must match "Topic string length"
** UINT16         - Message data length (0-256)
** Array of OCTET - Message data (0x00-0xFF),
**                  length of array must match "Message data length"
*/
#ifndef MQTT_IA_LAST_WILL_ENABLE
   #define MQTT_IA_LAST_WILL_ENABLE                0
   #define MQTT_IA_LAST_WILL_QOS                   0
   #define MQTT_IA_LAST_WILL_RETAIN                FALSE
   #define MQTT_IA_LAST_WILL_TOPIC_STR             "UserDefinedTopic"
   #define MQTT_IA_LAST_WILL_MESSAGE_STR           "UserDefinedMessage"
#endif

/*
** Object specific command: Publish Configuration (0x10)
**
** If enabled, MQTT_GetPublishConfigurationCfn() must be ported to respond with
** host application specific data.
*/
#ifndef MQTT_CMD_PUBLISH_CFG_ENABLE
   #define MQTT_CMD_PUBLISH_CFG_ENABLE             0
#endif

#endif /* #if MQTT_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** OPC UA Object (0xE3)
**------------------------------------------------------------------------------
*/
#if OPCUA_OBJ_ENABLE
/*
** Attribute 1: OPC UA Model (UINT8)
** Refer to abp_opcua.h for details.
*/
#ifndef OPCUA_IA_MODEL_ENABLE
   #define OPCUA_IA_MODEL_ENABLE                   0
   #define OPCUA_IA_MODEL_VALUE                    ABP_OPCUA_MODEL_DISABLED
#endif

/*
** Attribute 2: Application/Local namespace URI (Array of CHAR - {0x00-0xFF})
** A function returning a string may be used to generate a unique string
*/
#ifndef OPCUA_IA_APPLICATION_URI_ENABLE
   #define OPCUA_IA_APPLICATION_URI_ENABLE         0
   #define OPCUA_IA_APPLICATION_URI_VALUE          "UniqueApplicationUriPerUnit"
#endif

/*
** Attribute 3: Vendor namespace URI (Array of CHAR - {0x00-0xFF})
*/
#ifndef OPCUA_IA_VENDOR_NAMESPACE_URI_ENABLE
   #define OPCUA_IA_VENDOR_NAMESPACE_URI_ENABLE    0
   #define OPCUA_IA_VENDOR_NAMESPACE_URI_VALUE     "UniqueVendorNamespaceUri"
#endif

/*
** Attribute 4: DeviceType name (Array of CHAR - {0x00-0xFF))
*/
#ifndef OPCUA_IA_DEVICE_TYPE_NAME_ENABLE
   #define OPCUA_IA_DEVICE_TYPE_NAME_ENABLE        0
   #define OPCUA_IA_DEVICE_TYPE_NAME_VALUE         "VendorDeviceType"
#endif

/*
** Attribute 5: Device instance name (Array of CHAR - {0x00-0xFF))
*/
#ifndef OPCUA_IA_DEVICE_INST_NAME_ENABLE
   #define OPCUA_IA_DEVICE_INST_NAME_ENABLE        0
   #define OPCUA_IA_DEVICE_INST_NAME_VALUE         "VendorDeviceInstance"
#endif

/*
** Attribute 6: Product URI (Array of CHAR - {0x00-0xFF))
*/
#ifndef OPCUA_IA_PRODUCT_URI_ENABLE
   #define OPCUA_IA_PRODUCT_URI_ENABLE             0
   #define OPCUA_IA_PRODUCT_URI_VALUE              "UniqueProductUri"
#endif

/*
** Attribute 7: Limits
** Struct of:
** UINT16 - Max number of monitored items (8-100)
** UINT32 - Minimum publishing interval in milliseconds (1000-86400000)
** UINT32 - Minimum sampling interval in milliseconds (10-86400000)
** UINT32 - Maximum queue size (1-110)
*/
#ifndef OPCUA_IA_LIMITS_ENABLE
   #define OPCUA_IA_LIMITS_ENABLE                  0
   #define OPCUA_IA_LIMITS_MAX_NUM_MON_ITEMS       8
   #define OPCUA_IA_LIMITS_MIN_PUBLISHING_INT_MS   1000
   #define OPCUA_IA_LIMITS_MIN_SAMPLING_INT_MS     1000
   #define OPCUA_IA_LIMITS_MAX_QUEUE_SIZE          1
#endif

/*
** Attribute 8: Application models
**
** If this attribute is enabled the Application models-related callback
** functions defined in "opcua_object.h" must be implemented by the
** application.
**
** If enabled the OPC UA Object will also handle the object-specific commands
** Get_Application_Models_Filename and Get_Application_Models_Namespace.
*/
#ifndef OPCUA_IA_APPLICATION_MODELS_ENABLE
   #define OPCUA_IA_APPLICATION_MODELS_ENABLE      0
#endif

/*
** Attribute 9: Server status - (Set by the ABCC)
** Struct of:
** ENUM  - State of OPC UA server (see ABP_OpcuaStateType)
** UINT8 - Number of connected clients (0-255)
*/
#ifndef OPCUA_IA_SERVER_STATUS_ENABLE
   #define OPCUA_IA_SERVER_STATUS_ENABLE           0
#endif

#endif /* #if OPCUA_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Functional Safety object (0xE8)
**------------------------------------------------------------------------------
*/
#if SAFE_OBJ_ENABLE
/*
** Attribute 1: Safety Enabled (BOOL - TRUE/FALSE)
**
** PORTING ALERT!
**
** If the ABCCs serial application interface is used, please double-check that
** the application hardware is correctly set up before enabling the interface
** for the safety hardware via the 'Safety Enabled' attribute. When a serial
** Operation Mode is used the serial communication signals for the safety
** hardware uses the same I/O pins on the ABCC as the A12 and A13 address bus
** signals, and they must then not be driven from any other application-side
** hardware in this case.
**
** Also note that the value of the 'Safety Enabled' attribute does not have to
** be initialized to TRUE in this file. It can be set during system startup via
** the "SAFE_SetSafetyEnable()" function in "safety_object.c" before the ABCC
** driver itself is started.
*/
#ifndef SAFE_IA_SAFETY_ENABLED_ENABLE
   #define SAFE_IA_SAFETY_ENABLED_ENABLE           0
   #define SAFE_IA_SAFETY_ENABLED_VALUE            FALSE
#endif

/*
** Attribute 2: Baud rate (UINT32 - 625000, 1000000 or 1020000)
*/
#ifndef SAFE_IA_BAUD_RATE_ENABLE
   #define SAFE_IA_BAUD_RATE_ENABLE                0
   #define SAFE_IA_BAUD_RATE_VALUE                 1020000L
#endif

/*
** Attribute 3: IO Configuration (Array of UINT8 - {0x00-0xFF})
*/
#ifndef SAFE_IA_IO_CONFIGURATION_ENABLE
   #define SAFE_IA_IO_CONFIGURATION_ENABLE         0
   #define SAFE_IA_IO_CONFIGURATION_VALUE          { 0x00 }
   #define SAFE_IA_IO_CONFIGURATION_ARRAY_SIZE     1
#endif

/*
** Attribute 4: Cycle time (UINT8 - 2, 4, 8 or 16)
*/
#ifndef SAFE_IA_CYCLE_TIME_ENABLE
#define SAFE_IA_CYCLE_TIME_ENABLE                  0
#define SAFE_IA_CYCLE_TIME_VALUE                   2
#endif

#endif /* #if SAFE_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Assembly mapping Object (0xEB)
**------------------------------------------------------------------------------
*/
#if ASM_OBJ_ENABLE
/*
** Attribute 13: Name (Array of CHAR - {0x00-0xFF))
*/
#ifndef ASM_IA_NAME_ENABLE
   #define ASM_IA_NAME_ENABLE                      0
#endif

/*
** Attribute 14: Max number of ADI maps (Array of UINT16)
*/
#ifndef ASM_IA_MAX_NUM_ADI_MAPS_ENABLE
   #define ASM_IA_MAX_NUM_ADI_MAPS_ENABLE          0
#endif

/*
** Enable to support dynamic assembly mapping instances.
**
** Note: When this configuration define is set to 1, the arrays of type
** AD_MapType assigned to psMap in ASM_InstanceType shall not be declared const.
** It is also necessary to specify the size of the AD_MapType arrays to
** ASM_MAX_ADI_MAPS_PER_INSTANCE.
*/
#ifndef ASM_SUPPORT_DYNAMIC_INSTANCES
   #define ASM_SUPPORT_DYNAMIC_INSTANCES           0
#endif

/*
** Maximum number of ADI maps per assembly mapping instance. Only used if
** dynamic assembly instances are supported.
*/
#if ASM_SUPPORT_DYNAMIC_INSTANCES
   #ifndef ASM_MAX_ADI_MAPS_PER_INSTANCE
      #define ASM_MAX_ADI_MAPS_PER_INSTANCE        64
   #endif
#endif

#endif /* #if ASM_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Sync Object (0xEE)
**------------------------------------------------------------------------------
*/
#if SYNC_OBJ_ENABLE
/*
** Attribute 1: Cycle time (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef SYNC_IA_CYCLE_TIME_ENABLE
   #define SYNC_IA_CYCLE_TIME_ENABLE               1
   #define SYNC_IA_CYCLE_TIME_VALUE                0L
#endif

/*
** Attribute 2: Output valid (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef SYNC_IA_OUTPUT_VALID_ENABLE
   #define SYNC_IA_OUTPUT_VALID_ENABLE             1
   #define SYNC_IA_OUTPUT_VALID_VALUE              0
#endif

/*
** Attribute 3: Input capture (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef SYNC_IA_INPUT_CAPTURE_ENABLE
   #define SYNC_IA_INPUT_CAPTURE_ENABLE            1
   #define SYNC_IA_INPUT_CAPTURE_VALUE             0
#endif

/*
** Attribute 4: Output processing (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef SYNC_IA_OUTPUT_PROCESSING_ENABLE
   #define SYNC_IA_OUTPUT_PROCESSING_ENABLE        1
   #define SYNC_IA_OUTPUT_PROCESSING_VALUE         1000L
#endif

/*
** Attribute 5: Input processing (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef SYNC_IA_INPUT_PROCESSING_ENABLE
   #define SYNC_IA_INPUT_PROCESSING_ENABLE         1
   #define SYNC_IA_INPUT_PROCESSING_VALUE          1000L
#endif

/*
** Attribute 6: Min cycle time (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef SYNC_IA_MIN_CYCLE_TIME_ENABLE
   #define SYNC_IA_MIN_CYCLE_TIME_ENABLE           1
   #define SYNC_IA_MIN_CYCLE_TIME_VALUE            1L
#endif

/*
** Attribute 7: Sync mode (UINT16 - 0: Nonsynchronous operation
**                                  1: Synchronous operation)
*/
#ifndef SYNC_IA_SYNC_MODE_ENABLE
   #define SYNC_IA_SYNC_MODE_ENABLE                1
   #define SYNC_IA_SYNC_MODE_VALUE                 0x0000
#endif

/*
** Attribute 8: Supported sync modes (UINT16 - Bit 0: 1 = Nonsynchronous mode supported
**                                             Bit 1: 1 = Synchronous mode supported
**                                             Bit 2-15: Reserved)
*/
#ifndef SYNC_IA_SUPPORTED_SYNC_MODES_ENABLE
   #define SYNC_IA_SUPPORTED_SYNC_MODES_ENABLE     1
   #define SYNC_IA_SUPPORTED_SYNC_MODES_VALUE      0x0003
#endif

#endif /* #if SYNC_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** BACnet Host Object (0xEF)
**------------------------------------------------------------------------------
*/
#if BAC_OBJ_ENABLE

/*
** Attribute 7: Support advanced mapping (BOOL)
*/
#ifndef BAC_IA_SUPPORT_ADV_MAPPING_ENABLE
   #define BAC_IA_SUPPORT_ADV_MAPPING_ENABLE       0
   #define BAC_IA_SUPPORT_ADV_MAPPING_VALUE        FALSE
#endif

/*
** Attribute 8: Current Date and Time
**
** Note that platform-specific adaptations are needed for the Current Date and
** Time to work as intended. See the comments in "abcc_time.h" for more information.
*/
#ifndef BAC_IA_CURR_DATE_AND_TIME_ENABLE
   #define BAC_IA_CURR_DATE_AND_TIME_ENABLE        0
   #define BAC_IA_CURR_DATE_AND_TIME_SET           FALSE
#endif

/*
** Attribute 9: Password (Array of Char)
*/
#ifndef BAC_IA_PASSWORD_ENABLE
   #define BAC_IA_PASSWORD_ENABLE                  0
   #define BAC_IA_PASSWORD_VALUE                   "examplepassword"
#endif

#endif  /* BAC_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** EtherCAT Object (0xF5)
**------------------------------------------------------------------------------
*/
#if ECT_OBJ_ENABLE

/*------------------------------------------------------------------------------
** Configuration of the Get_Object_Description command on the EtherCAT host
** object. This command is used to change the EtherCAT data type and EtherCAT
** object code for selected objects.
**------------------------------------------------------------------------------
*/
#ifndef ECT_OBJ_CMD_GET_OBJ_DESC_ENABLE
   #define ECT_OBJ_CMD_GET_OBJ_DESC_ENABLE   0
   #define ECT_GET_OBJ_DESC_NUM_OBJ          1
   #define ECT_GET_OBJ_DESC_CONTENT          { { 0x2001, ECT_DATATYPE_UNSIGNED32, ECT_OBJCODE_ARRAY } }
#endif

/*------------------------------------------------------------------------------
** ECT_SUPPORT_SYNC_ERR_COUNTERS
**
** Setting this configuration option to 1 will enable the synchronization
** related error counters on the sync manager parameter objects (0x1C32 and
** 0x1C33).
** The following object entries will then be accessible by the EtherCAT master:
** 0x1C32:11                 -  SM event missed counter
** 0x1C32:12 and 0x1C33:12   -  Cycle time too small
** 0x1C32:13                 -  Shift time too short
** 0x1C32:32                 -  Sync error
**
** Note: These object entries are specified in the EtherCAT specification
** ETG.1020, Protocol enhancements. See that document for more information.
**
** Note: These object entries are mandatory to support when synchronization
** is supported by the EtherCAT slave. They are not used when not operating
** synchronously. Therefore the recommendation is to set this configuration
** option to 1 if bit 1 in SYNC_IA_SUPPORTED_SYNC_MODES_VALUE is 1 and
** the EtherCAT network is supported.
**------------------------------------------------------------------------------
*/
#ifndef ECT_SUPPORT_SYNC_ERR_COUNTERS
   #define ECT_SUPPORT_SYNC_ERR_COUNTERS  0
#endif

#if ECT_SUPPORT_SYNC_ERR_COUNTERS
   /*
   ** Sub-index translation functionality must be enabled when the sync
   ** error counters are enabled.
   */
   #ifndef ECT_IA_OBJ_SUB_TRANS_ENABLE
      #define ECT_IA_OBJ_SUB_TRANS_ENABLE       1
   #endif

   /*
   ** ADIs used for the sub-index translation
   */
   #ifndef ECT_SUB_TRANS_1C32_11
      #define ECT_SUB_TRANS_1C32_11 0xF20B
   #endif
   #ifndef ECT_SUB_TRANS_1C32_12
      #define ECT_SUB_TRANS_1C32_12 0xF20C
   #endif
   #ifndef ECT_SUB_TRANS_1C32_13
      #define ECT_SUB_TRANS_1C32_13 0xF20D
   #endif
   #ifndef ECT_SUB_TRANS_1C32_32
      #define ECT_SUB_TRANS_1C32_32 0xF220
   #endif
   #ifndef ECT_SUB_TRANS_1C33_12
      #define ECT_SUB_TRANS_1C33_12 0xF30C
   #endif

   /*
   ** Example value for the sub-index translation attribute.
   */
   #ifndef ECT_IA_OBJ_SUB_TRANS_VALUE
      /*
      ** The ADIs created for sync error counters are translated into entries
      ** on the 0x1C32 and 0x1C33 objects.
      */
      #define ECT_IA_OBJ_SUB_TRANS_VALUE             \
         {                                           \
            { ECT_SUB_TRANS_1C32_11, 0x1C32, 0x0B }, \
            { ECT_SUB_TRANS_1C32_12, 0x1C32, 0x0C }, \
            { ECT_SUB_TRANS_1C32_13, 0x1C32, 0x0D }, \
            { ECT_SUB_TRANS_1C32_32, 0x1C32, 0x20 }, \
            { ECT_SUB_TRANS_1C33_12, 0x1C33, 0x0C }  \
         }
   #endif
   #ifndef ECT_IA_OBJ_SUB_TRANS_SIZE
      #define ECT_IA_OBJ_SUB_TRANS_SIZE         5
   #endif
#endif /* #if ECT_SUPPORT_SYNC_ERR_COUNTERS */

/*
** Attribute 9: ENUM ADIs (BOOL - TRUE/FALSE)
**
** Enabling support for this attribute will also include code that generates
** the 'ENUM ADI list' from the defined ADIs. Check the CompactCom 40 EtherCAT
** Network Guide for more details about this attribute.
*/
#ifndef ECT_IA_ENUM_ADIS_ENABLE
   #define ECT_IA_ENUM_ADIS_ENABLE                 0
#endif

/*
** Attribute 11: Write PD assembly instance translation
** (Array of UINT16 - {0x1A00-0x1BFF})
*/
#ifndef ECT_IA_WR_PD_ASSY_INST_TRANS_ENABLE
   #define ECT_IA_WR_PD_ASSY_INST_TRANS_ENABLE     0
   #define ECT_IA_WR_PD_ASSY_INST_TRANS_VALUE      { 0x1BFF }
   #define ECT_IA_WR_PD_ASSY_INST_TRANS_SIZE       1
#endif

/*
** Attribute 12: Read PD assembly instance translation
** (Array of UINT16 - {0x1600-0x17FF})
*/
#ifndef ECT_IA_RD_PD_ASSY_INST_TRANS_ENABLE
   #define ECT_IA_RD_PD_ASSY_INST_TRANS_ENABLE     0
   #define ECT_IA_RD_PD_ASSY_INST_TRANS_VALUE      { 0x17FF }
   #define ECT_IA_RD_PD_ASSY_INST_TRANS_SIZE       1
#endif

/*
** Attribute 13: ADI translation (Array of (Struct of {UINT16, UINT16}))
*/
#ifndef ECT_IA_ADI_TRANS_ENABLE
   #define ECT_IA_ADI_TRANS_ENABLE                 0
   #define ECT_IA_ADI_TRANS_VALUE                  { { 0xE000, 0x10F8 } }
   #define ECT_IA_ADI_TRANS_SIZE                   1
#endif

/*
** Attribute 15: Object subindex translation
** (Array of (Struct of {UINT16, UINT16, UINT8 }))
*/
#ifndef ECT_IA_OBJ_SUB_TRANS_ENABLE
   #define ECT_IA_OBJ_SUB_TRANS_ENABLE             0
   #define ECT_IA_OBJ_SUB_TRANS_VALUE              { { 0xE000, 0x10F3, 0x01 } }
   #define ECT_IA_OBJ_SUB_TRANS_SIZE               1
#endif

/*
** Attribute 16: Enable FoE (BOOL - TRUE/FALSE)
*/
#ifndef ECT_IA_ENABLE_FOE_ENABLE
   #define ECT_IA_ENABLE_FOE_ENABLE                0
   #define ECT_IA_ENABLE_FOE_VALUE                 TRUE
#endif

/*
** Attribute 17: Enable EoE (BOOL - TRUE/FALSE)
*/
#ifndef ECT_IA_ENABLE_EOE_ENABLE
   #define ECT_IA_ENABLE_EOE_ENABLE                0
   #define ECT_IA_ENABLE_EOE_VALUE                 TRUE
#endif

/*
** Attribute 19: Set Device ID as Configured Station Alias (BOOL - TRUE/FALSE)
*/
#ifndef ECT_IA_SET_DEV_ID_AS_CSA_ENABLE
   #define ECT_IA_SET_DEV_ID_AS_CSA_ENABLE         0
   #define ECT_IA_SET_DEV_ID_AS_CSA_VALUE          FALSE
#endif

/*
** Attribute 20: EtherCAT state
*/
#ifndef ECT_IA_ETHERCAT_STATE_ENABLE
   #define ECT_IA_ETHERCAT_STATE_ENABLE            0
#endif

/*
** Attribute 21: State Transition Timeouts (Array of UINT32)
*/
#ifndef ECT_IA_STATE_TIMEOUTS_ENABLE
   #define ECT_IA_STATE_TIMEOUTS_ENABLE            0
   #define ECT_IA_STATE_TIMEOUTS_VALUE             { 1000, 5000, 1000, 200 }
#endif

/*
** Attribute 22: Compare Identity Lists (BOOL - TRUE/FALSE)
*/
#ifndef ECT_IA_COMP_IDENT_LISTS_ENABLE
   #define ECT_IA_COMP_IDENT_LISTS_ENABLE          0
   #define ECT_IA_COMP_IDENT_LISTS_VALUE           TRUE
#endif

/*
** Attribute 23: FSoE Status Indicator
*/
#ifndef ECT_IA_FSOE_STATUS_IND_ENABLE
   #define ECT_IA_FSOE_STATUS_IND_ENABLE           0
#endif

/*
** Attribute 24: Clear IdentALSts (BOOL - TRUE/FALSE)
*/
#ifndef ECT_IA_CLEAR_IDENT_AL_STS_ENABLE
   #define ECT_IA_CLEAR_IDENT_AL_STS_ENABLE        0
   #define ECT_IA_CLEAR_IDENT_AL_STS_VALUE         FALSE
#endif

/*
** Attribute 28: Default TxPDO Assignment (Array of UINT16)
*/
#ifndef ECT_IA_DEF_TXPDO_ASSIGN_ENABLE
   #define ECT_IA_DEF_TXPDO_ASSIGN_ENABLE          0
   #define ECT_IA_DEF_TXPDO_ASSIGN_VALUE           { 0x1A00 }
   #define ECT_IA_DEF_TXPDO_ASSIGN_SIZE            1
#endif

/*
** Attribute 29: Default RxPDO Assignment (Array of UINT16)
*/
#ifndef ECT_IA_DEF_RXPDO_ASSIGN_ENABLE
   #define ECT_IA_DEF_RXPDO_ASSIGN_ENABLE          0
   #define ECT_IA_DEF_RXPDO_ASSIGN_VALUE           { 0x1600 }
   #define ECT_IA_DEF_RXPDO_ASSIGN_SIZE            1
#endif

#endif /* if ECT_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** PROFINET IO Object (0xF6)
**------------------------------------------------------------------------------
*/

#if PRT_OBJ_ENABLE
/*
** Attribute 4: MaxAr (UINT32 - 0x00000000-0x00000003)
*/
#ifndef PRT_IA_MAX_AR_ENABLE
   #define PRT_IA_MAX_AR_ENABLE                    0
   #define PRT_IA_MAX_AR_VALUE                     0x00000000
#endif

/*
** Attribute 7: Record data Transparent Mode (UINT8 - 0x00-0xFF)
*/
#ifndef PRT_IA_RTM_ENABLE
   #define PRT_IA_RTM_ENABLE                       0
   #define PRT_IA_RTM_VALUE                        0x00
#endif

/*
** Attribute 12: IM Revision counter (UINT16 - 0x0000-0xFFFF)
*/
#ifndef PRT_IA_IM_REV_CNT_ENABLE
   #define PRT_IA_IM_REV_CNT_ENABLE                0
   #define PRT_IA_IM_REV_CNT_VALUE                 0xFFFF
#endif

/*
** Attribute 13: IM Profile ID (UINT16 - 0x0000-0xFFFF)
*/
#ifndef PRT_IA_IM_PROFILE_ID_ENABLE
   #define PRT_IA_IM_PROFILE_ID_ENABLE             0
   #define PRT_IA_IM_PROFILE_ID_VALUE              0xFFFF
#endif

/*
** Attribute 14: IM Profile specific type (UINT16 - 1-6))
*/
#ifndef PRT_IA_IM_PROFILE_SPEC_TYPE_ENABLE
   #define PRT_IA_IM_PROFILE_SPEC_TYPE_ENABLE      0
   #define PRT_IA_IM_PROFILE_SPEC_TYPE_VALUE       0x0004
#endif

/*
** Attribute 17: Port 1 MAC Address (Array[6] of UINT8)
**
** PORTING ALERT!
**
** Read the comments about MAC addresses in "abcc_api_identification.h" before
** enabling this attribute!
*/
#ifndef PRT_IA_PORT1_MAC_ADDRESS_ENABLE
   #define PRT_IA_PORT1_MAC_ADDRESS_ENABLE         0
   #define PRT_IA_PORT1_MAC_ADDRESS_VALUE          "\x00\x00\x00\x00\x00\x00"
#endif

/*
** Attribute 18: Port 2 MAC Address (Array[6] of UINT8)
**
** PORTING ALERT!
**
** Read the comments about MAC addresses in "abcc_api_identification.h" before
** enabling this attribute!
*/
#ifndef PRT_IA_PORT2_MAC_ADDRESS_ENABLE
   #define PRT_IA_PORT2_MAC_ADDRESS_ENABLE         0
   #define PRT_IA_PORT2_MAC_ADDRESS_VALUE          "\x00\x00\x00\x00\x00\x00"
#endif

/*
** Attribute 25: IM5 Module Order ID (Array of CHAR).
*/
#ifndef PRT_IA_IM_MODULE_ORDER_ID_ENABLE
   #define PRT_IA_IM_MODULE_ORDER_ID_ENABLE        0
   #define PRT_IA_IM_MODULE_ORDER_ID_VALUE         "ARTICLE_ABCD_1234_PIR"
#endif

/*
** Attribute 26: IM5 Annotation (Array of CHAR).
*/
#ifndef PRT_IA_IM_ANNOTATION_ENABLE
   #define PRT_IA_IM_ANNOTATION_ENABLE             0
   #define PRT_IA_IM_ANNOTATION_VALUE              "ExampleCompany PROFINET IRT Adapter"
#endif

/*
** Attribute 27: IM5 Enabled (BOOL - TRUE/FALSE).
*/
#ifndef PRT_IA_IM5_ENABLED_ENABLE
   #define PRT_IA_IM5_ENABLED_ENABLE               0
   #define PRT_IA_IM5_ENABLED_VALUE                TRUE
#endif

/*
** Attribute 28: S2 Enabled (BOOL - TRUE/FALSE).
*/
#ifndef PRT_IA_S2_ENABLED_ENABLE
   #define PRT_IA_S2_ENABLED_ENABLE                0
   #define PRT_IA_S2_ENABLED_VALUE                 TRUE
#endif

/*
** Attribute 29: S2 Primary AR handle (UINT16).
*/
#ifndef PRT_IA_S2_PRIMARY_AR_HANDLE_ENABLE
   #define PRT_IA_S2_PRIMARY_AR_HANDLE_ENABLE      0
#endif

#endif /* #if PRT_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** CC-Link Object (0xF7)
**------------------------------------------------------------------------------
*/
#if CCL_OBJ_ENABLE

/*
** Attribute 4: Network Setting (Array[3] of UINT8)
*/
#ifndef CCL_IA_NETWORK_SETTING_ENABLE
   #define CCL_IA_NETWORK_SETTING_ENABLE           1
   #define CCL_IA_CCLINK_VERSION_VALUE             0x02
   #define CCL_IA_NUM_OF_OCCUPIED_STATION_VALUE    0x02
   #define CCL_IA_NUM_OF_EXTENSION_CYCLES_VALUE    0x04
#endif

/*
** Attribute 5: System Area Hander (SINT16 - -1:Host application/0...880:Module)
*/
#ifndef CCL_IA_SYSTEM_AREA_HANDLER_ENABLE
   #define CCL_IA_SYSTEM_AREA_HANDLER_ENABLE       0
   #define CCL_IA_SYSTEM_AREA_HANDLER_VALUE        ( -1 )
#endif

/*
** Attribute 6: Output Hold/Clear (UINT8 - 0:Clear/1:Hold)
*/
#ifndef CCL_IA_OUTPUT_HOLD_CLEAR_ENABLE
   #define CCL_IA_OUTPUT_HOLD_CLEAR_ENABLE         1
   #define CCL_IA_OUTPUT_HOLD_CLEAR_VALUE          0x00
#endif

#endif /* #if CCL_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** EtherNet/IP Object (0xF8)
**------------------------------------------------------------------------------
*/

#if EIP_OBJ_ENABLE
/*
** Attribute 7: Producing instance number (Array of UINT16 - {0x0001-0xFFFF})
*/
#ifndef EIP_IA_PROD_INSTANCE_ENABLE
   #define EIP_IA_PROD_INSTANCE_ENABLE             0
   #define EIP_IA_PROD_INSTANCE_VALUE              { 0x0064 }
   #define EIP_IA_PROD_INSTANCE_ARRAY_SIZE         1
#endif

/*
** Attribute 8: Consuming instance number (Array of UINT16 - {0x0001-0xFFFF})
*/
#ifndef EIP_IA_CONS_INSTANCE_ENABLE
   #define EIP_IA_CONS_INSTANCE_ENABLE             0
   #define EIP_IA_CONS_INSTANCE_VALUE              { 0x0096 }
   #define EIP_IA_CONS_INSTANCE_ARRAY_SIZE         1
#endif

/*
** Attribute 9: Enable communication settings from net (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_COMM_SETTINGS_FROM_NET_ENABLE
   #define EIP_IA_COMM_SETTINGS_FROM_NET_ENABLE    0
   #define EIP_IA_COMM_SETTINGS_FROM_NET_VALUE     TRUE
#endif

/*
** Attribute 11: Enable CIP request forwarding (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_ENABLE_APP_CIP_OBJECTS_ENABLE
   #define EIP_IA_ENABLE_APP_CIP_OBJECTS_ENABLE    0
   #define EIP_IA_ENABLE_APP_CIP_OBJECTS_VALUE     FALSE
#endif

/*
** Attribute 12: Enable Parameter object (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_ENABLE_PARAM_OBJECT_ENABLE
   #define EIP_IA_ENABLE_PARAM_OBJECT_ENABLE       0
   #define EIP_IA_ENABLE_PARAM_OBJECT_VALUE        TRUE
#endif

/*
** Attribute 13: Input only heartbeat instance number (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_INPUT_INSTANCE_OBJECT_ENABLE
   #define EIP_IA_INPUT_INSTANCE_OBJECT_ENABLE     0
   #define EIP_IA_INPUT_INSTANCE_OBJECT_VALUE      0x0003
#endif

/*
** Attribute 14: Listen only heartbeat instance number (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_LISTEN_INSTANCE_OBJECT_ENABLE
   #define EIP_IA_LISTEN_INSTANCE_OBJECT_ENABLE    0
   #define EIP_IA_LISTEN_INSTANCE_OBJECT_VALUE     0x0004
#endif

/*
** Attribute 15: Assembly object Configuration instance number (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_CONFIG_INSTANCE_ENABLE
   #define EIP_IA_CONFIG_INSTANCE_ENABLE           0
   #define EIP_IA_CONFIG_INSTANCE_VALUE            0x0005
#endif

/*
** Attribute 16: Disable Strict IO match (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_DISABLE_STRICT_IO_MATCH_ENABLE
   #define EIP_IA_DISABLE_STRICT_IO_MATCH_ENABLE   0
   #define EIP_IA_DISABLE_STRICT_IO_MATCH_VALUE    FALSE
#endif

/*
** Attribute 17: Enable unconnected routing (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_ENABLE_UNCONNECTED_SEND_ENABLE
   #define EIP_IA_ENABLE_UNCONNECTED_SEND_ENABLE   0
   #define EIP_IA_ENABLE_UNCONNECTED_SEND_VALUE    FALSE
#endif

/*
** Attribute 18: Input only extended heartbeat instance number (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_INPUT_EXT_INSTANCE_OBJECT_ENABLE
   #define EIP_IA_INPUT_EXT_INSTANCE_OBJECT_ENABLE 0
   #define EIP_IA_INPUT_EXT_INSTANCE_OBJECT_VALUE  0x0006
#endif

/*
** Attribute 19: Listen only extended heartbeat instance number (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_LISTEN_EXT_INSTANCE_OBJECT_ENABLE
   #define EIP_IA_LISTEN_EXT_INSTANCE_OBJECT_ENABLE   0
   #define EIP_IA_LISTEN_EXT_INSTANCE_OBJECT_VALUE    0x0007
#endif

/*
** Attribute 20: Interface label port 1 (Array of CHAR)
*/
#ifndef EIP_IA_IF_LABEL_PORT_1_ENABLE
   #define EIP_IA_IF_LABEL_PORT_1_ENABLE           0
   #define EIP_IA_IF_LABEL_PORT_1_VALUE            "Port 1"
#endif

/*
** Attribute 21: Interface label port 2 (Array of CHAR)
*/
#ifndef EIP_IA_IF_LABEL_PORT_2_ENABLE
   #define EIP_IA_IF_LABEL_PORT_2_ENABLE           0
   #define EIP_IA_IF_LABEL_PORT_2_VALUE            "Port 2"
#endif

/*
** Attribute 22: Interface label internal port (Array of CHAR)
*/
#ifndef EIP_IA_IF_LABEL_PORT_INT_ENABLE
   #define EIP_IA_IF_LABEL_PORT_INT_ENABLE         0
   #define EIP_IA_IF_LABEL_PORT_INT_VALUE          "Internal"
#endif

/*
** Attribute 26: Enable EtherNet/IP QuickConnect (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_ENABLE_EIP_QC_ENABLE
   #define EIP_IA_ENABLE_EIP_QC_ENABLE             0
   #define EIP_IA_ENABLE_EIP_QC_VALUE              FALSE
#endif

/*
** Attribute 29: Ignore Sequence Count Check (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_IGNORE_SEQ_COUNT_CHECK_ENABLE
   #define EIP_IA_IGNORE_SEQ_COUNT_CHECK_ENABLE    0
   #define EIP_IA_IGNORE_SEQ_COUNT_CHECK_VALUE     FALSE
#endif

/*
** Attribute 30: ABCC ADI Object number (UINT16 - 0x0064-0x00C7, 0x0300-0x04FF)
*/
#ifndef EIP_IA_ABCC_ADI_OBJECT_ENABLE
   #define EIP_IA_ABCC_ADI_OBJECT_ENABLE           0
   #define EIP_IA_ABCC_ADI_OBJECT_VALUE            0x00A2
#endif

/*
** Attribute 31: Enable DLR (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_ABCC_ENABLE_DLR_ENABLE
   #define EIP_IA_ABCC_ENABLE_DLR_ENABLE           0
   #define EIP_IA_ABCC_ENABLE_DLR_VALUE            TRUE
#endif

/*
** Attribute 32: Enable CIP Sync (BOOL - TRUE/FALSE)
*/
#ifndef EIP_IA_ABCC_ENABLE_CIP_SYNC_ENABLE
   #define EIP_IA_ABCC_ENABLE_CIP_SYNC_ENABLE       0
   #define EIP_IA_ABCC_ENABLE_CIP_SYNC_VALUE        TRUE
#endif

#endif /* #if EIP_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Ethernet Object (0xF9)
**------------------------------------------------------------------------------
*/

#if ETN_OBJ_ENABLE
/*
** Attribute 2: Enable Host IP Configuration Protocol  (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_HICP_ENABLE
   #define ETN_IA_ENABLE_HICP_ENABLE               0
   #define ETN_IA_ENABLE_HICP_VALUE                TRUE
#endif

/*
** Attribute 3: Enable Web Server  (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_WEB_ENABLE
   #define ETN_IA_ENABLE_WEB_ENABLE                0
   #define ETN_IA_ENABLE_WEB_VALUE                 TRUE
#endif

/*
** Attribute 5: Enable Web ADI access  (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_WEB_ADI_ACCESS_ENABLE
   #define ETN_IA_ENABLE_WEB_ADI_ACCESS_ENABLE     0
   #define ETN_IA_ENABLE_WEB_ADI_ACCESS_VALUE      TRUE
#endif

/*
** Attribute 6: Enable FTP server  (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_FTP_ENABLE
   #define ETN_IA_ENABLE_FTP_ENABLE                0
   #define ETN_IA_ENABLE_FTP_VALUE                 TRUE
#endif

/*
** Attribute 7: Enable admin mode  (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_ADMIN_MODE_ENABLE
   #define ETN_IA_ENABLE_ADMIN_MODE_ENABLE         0
   #define ETN_IA_ENABLE_ADMIN_MODE_VALUE          FALSE
#endif

/*
** Attribute 8: Network status  (UINT16)
*/
#ifndef ETN_IA_NETWORK_STATUS_ENABLE
   #define ETN_IA_NETWORK_STATUS_ENABLE            0
#endif

/*
** Attribute 11: Enable Address Conflict Detection   (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_ACD_ENABLE
   #define ETN_IA_ENABLE_ACD_ENABLE                0
   #define ETN_IA_ENABLE_ACD_VALUE                 TRUE
#endif

/*
** Attribute 12: Enable Port 1 state   (ENUM)
*/
#ifndef ETN_IA_PORT1_STATE_ENABLE
   #define ETN_IA_PORT1_STATE_ENABLE               0
   #define ETN_IA_PORT1_STATE_VALUE                ABP_ETN_IA_PORT_STATE_ENABLE
#endif

/*
** Attribute 13: Enable Port 2 state   (ENUM)
*/
#ifndef ETN_IA_PORT2_STATE_ENABLE
   #define ETN_IA_PORT2_STATE_ENABLE               0
   #define ETN_IA_PORT2_STATE_VALUE                ABP_ETN_IA_PORT_STATE_ENABLE
#endif

/*
** Attribute 14: Enable web update  (BOOL - TRUE/FALSE).
*/
#ifndef ETN_IA_ENABLE_WEB_UPDATE_ENABLE
   #define ETN_IA_ENABLE_WEB_UPDATE_ENABLE         0
   #define ETN_IA_ENABLE_WEB_UPDATE_VALUE          TRUE
#endif

/*
** Attribute 15: Enable HICP reset (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_HICP_RESET_ENABLE
   #define ETN_IA_ENABLE_HICP_RESET_ENABLE         0
   #define ETN_IA_ENABLE_HICP_RESET_VALUE          FALSE
#endif

/*
** Attribute 16: IP Configuration  (Struct of {UINT32, UINT32, UINT32})
*/
#ifndef ETN_IA_IP_CONFIGURATION_ENABLE
   #define ETN_IA_IP_CONFIGURATION_ENABLE          0
#endif

/*
** Attribute 17: IP address byte 0-2 (Array[3] of UINT8)
*/
#ifndef ETN_IA_IP_ADDRESS_BYTE_0_2_ENABLE
   #define ETN_IA_IP_ADDRESS_BYTE_0_2_ENABLE       0
   #define ETN_IA_IP_ADDRESS_BYTE_0_2_VALUE        "\xC0\xA8\x00"
#endif

/*
** Attribute 18: Ethernet PHY Configuration (Array of BITS16)
*/
#ifndef ETN_IA_ETH_PHY_CONFIG_ENABLE
   #define ETN_IA_ETH_PHY_CONFIG_ENABLE            0
   #define ETN_IA_ETH_PHY_CONFIG_PORT1_VALUE       0x0001
   #define ETN_IA_ETH_PHY_CONFIG_PORT2_VALUE       0x0001
#endif

/*
** Attribute 20: SNMP read-only community string (Array of CHAR - {0x00-0xFF})
*/
#ifndef ETN_IA_SNMP_READ_ONLY_ENABLE
   #define ETN_IA_SNMP_READ_ONLY_ENABLE            0
   #define ETN_IA_SNMP_READ_ONLY_VALUE             "public"
#endif

/*
** Attribute 21: SNMP read-write community string (Array of CHAR - {0x00-0xFF})
*/
#ifndef ETN_IA_SNMP_READ_WRITE_ENABLE
   #define ETN_IA_SNMP_READ_WRITE_ENABLE           0
   #define ETN_IA_SNMP_READ_WRITE_VALUE            "private"
#endif

/*
** Attribute 22: DHCP Option 61 source (ENUM)
*/
#ifndef ETN_IA_DHCP_OPTION_61_SOURCE_ENABLE
   #define ETN_IA_DHCP_OPTION_61_SOURCE_ENABLE     0
   #define ETN_IA_DHCP_OPTION_61_SOURCE_VALUE      ABP_ETN_IA_DHCP_OPTION_61_SOURCE_DISABLE
#endif

/*
** Attribute 23: DHCP Option 61 generic string (Array of UINT8)
*/
#ifndef ETN_IA_DHCP_OPTION_61_GENERIC_STR_ENABLE
   #define ETN_IA_DHCP_OPTION_61_GENERIC_STR_ENABLE       0
   #define ETN_IA_DHCP_OPTION_61_GENERIC_STR_VALUE        "\x01\x00\x30\x11\x00\x00\x01"
   #define ETN_IA_DHCP_OPTION_61_GENERIC_STR_ARRAY_SIZE   7
#endif

/*
** Attribute 24: Enable DHCP Client (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_DHCP_CLIENT_ENABLE
   #define ETN_IA_ENABLE_DHCP_CLIENT_ENABLE        0
   #define ETN_IA_ENABLE_DHCP_CLIENT_VALUE         TRUE
#endif

/*
** Attribute 25: Enable WebDAV Server (BOOL - TRUE/FALSE)
*/
#ifndef ETN_IA_ENABLE_WEBDAV_ENABLE
   #define ETN_IA_ENABLE_WEBDAV_ENABLE             0
   #define ETN_IA_ENABLE_WEBDAV_VALUE              TRUE
#endif

#ifndef ETN_OBJ_USE_SET_ATTR_SUCCESS_CALLBACK
   #define ETN_OBJ_USE_SET_ATTR_SUCCESS_CALLBACK   0
#endif

#endif /* #if ETN_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Modbus Object (0xFA)
**------------------------------------------------------------------------------
*/

#if MOD_OBJ_ENABLE
/*
** Attribute 9: No. of ADI indexing bits (UINT8 - 0x00-0xFF)
*/
#ifndef MOD_IA_ADI_INDEXING_BITS_ENABLE
   #define MOD_IA_ADI_INDEXING_BITS_ENABLE         0
   #define MOD_IA_ADI_INDEXING_BITS_VALUE          0x00
#endif

/*
** Attribute 10: Enable Modbus Message forwarding (BOOL - TRUE/FALSE)
*/
#ifndef MOD_IA_MESSAGE_FORWARDING_ENABLE
   #define MOD_IA_MESSAGE_FORWARDING_ENABLE        0
   #define MOD_IA_MESSAGE_FORWARDING_VALUE         FALSE
#endif

/*
** Attribute 11: Modbus read/write registers command offset (Array of SINT16 - {0x0000-0xFFFF})
*/
#ifndef MOD_IA_RW_OFFSET_ENABLE
   #define MOD_IA_RW_OFFSET_ENABLE                 0
   #define MOD_IA_RW_OFFSET_READ_VALUE             0x0000
   #define MOD_IA_RW_OFFSET_WRITE_VALUE            0x0000
#endif

#endif /* #if MOD_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** CANopen Object (0xFB)
**------------------------------------------------------------------------------
*/

#if COP_OBJ_ENABLE

/*
** Attribute 17: Read PD buffer initial value (Array of UINT8)
*/
#ifndef COP_IA_READ_PD_BUF_INIT_VAL_ENABLE
   #define COP_IA_READ_PD_BUF_INIT_VAL_ENABLE      0
#endif

#endif /* #if COP_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** DeviceNet Object (0xFC)
**------------------------------------------------------------------------------
*/

#if DEV_OBJ_ENABLE
/*
** Attribute 7: Producing instance number (UINT16 - 0x0001-0xFFFF)
*/
#ifndef DEV_IA_PROD_INSTANCE_ENABLE
   #define DEV_IA_PROD_INSTANCE_ENABLE             0
   #define DEV_IA_PROD_INSTANCE_VALUE              0x0064
#endif

/*
** Attribute 8: Consuming instance number (UINT16 - 0x0001-0xFFFF)
*/
#ifndef DEV_IA_CONS_INSTANCE_ENABLE
   #define DEV_IA_CONS_INSTANCE_ENABLE             0
   #define DEV_IA_CONS_INSTANCE_VALUE              0x0096
#endif

/*
** Attribute 9: Enable address from net (BOOL - TRUE/FALSE)
*/
#ifndef DEV_IA_ADDRESS_FROM_NET_ENABLE
   #define DEV_IA_ADDRESS_FROM_NET_ENABLE          0
   #define DEV_IA_ADDRESS_FROM_NET_VALUE           FALSE
#endif

/*
** Attribute 10: Enable baud rate from net (BOOL - TRUE/FALSE)
*/
#ifndef DEV_IA_BAUD_RATE_FROM_NET_ENABLE
   #define DEV_IA_BAUD_RATE_FROM_NET_ENABLE        0
   #define DEV_IA_BAUD_RATE_FROM_NET_VALUE         FALSE
#endif

/*
** Attribute 11: Enable CIP request forwarding (BOOL - TRUE/FALSE)
*/
#ifndef DEV_IA_ENABLE_APP_CIP_OBJECTS_ENABLE
   #define DEV_IA_ENABLE_APP_CIP_OBJECTS_ENABLE    0
   #define DEV_IA_ENABLE_APP_CIP_OBJECTS_VALUE     FALSE
#endif

/*
** Attribute 12: Enable Parameter object (BOOL - TRUE/FALSE)
*/
#ifndef DEV_IA_ENABLE_PARAM_OBJECT_ENABLE
   #define DEV_IA_ENABLE_PARAM_OBJECT_ENABLE       0
   #define DEV_IA_ENABLE_PARAM_OBJECT_VALUE        FALSE
#endif

/*
** Attribute 13: Enable Quick Connect (BOOL - TRUE/FALSE)
*/
#ifndef DEV_IA_ENABLE_QUICK_CONNECT_ENABLE
   #define DEV_IA_ENABLE_QUICK_CONNECT_ENABLE      0
   #define DEV_IA_ENABLE_QUICK_CONNECT_VALUE       FALSE
#endif

/*
** Attribute 18: Prepend producing profile instance (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DEV_IA_PREPEND_PRODUCING_ENABLE
   #define DEV_IA_PREPEND_PRODUCING_ENABLE         0
   #define DEV_IA_PREPEND_PRODUCING_VALUE          0x0000
#endif

/*
** Attribute 19: Prepend consuming profile instance (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DEV_IA_PREPEND_CONSUMING_ENABLE
   #define DEV_IA_PREPEND_CONSUMING_ENABLE         0
   #define DEV_IA_PREPEND_CONSUMING_VALUE          0x0000
#endif

/*
** Attribute 20: ABCC ADI Object number (UINT16 - 0x0064-0x00C7, 0x0300-0x04FF)
*/
#ifndef DEV_IA_ABCC_ADI_OBJECT_ENABLE
   #define DEV_IA_ABCC_ADI_OBJECT_ENABLE           0
   #define DEV_IA_ABCC_ADI_OBJECT_VALUE            0x00A2
#endif

/*
** Attribute 21: Producing Instance Number list (Array of UINT16 - {0x0001-0xFFFF})
** The value of this attribute depends on the ADIs and the Assembly object data.
** An example is available in
** "/example_application/application_data_instance_mapping_asm.c".
*/

/*
** Attribute 22: Consuming Instance Number list (Array of UINT16 - {0x0001-0xFFFF})
** The value of this attribute depends on the ADIs and the Assembly object data.
** An example is available in
** "/example_application/application_data_instance_mapping_asm.c".
*/

#endif /* #if DEV_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** PROFIBUS DPV1 Object (0xFD)
**------------------------------------------------------------------------------
*/

#if DPV1_OBJ_ENABLE
/*
** Attribute 2: Parameterization data (Array of UINT8)
*/
#ifndef DPV1_IA_PRM_DATA_ENABLE
   #define DPV1_IA_PRM_DATA_ENABLE                 0
   #define DPV1_IA_PRM_DATA_VALUE                  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   #define DPV1_IA_PRM_DATA_ARRAY_SIZE             10
#endif

/*
** Attribute 3: Expected configuration (Array of UINT8)
*/
#ifndef DPV1_IA_EXPECTED_CFG_DATA_ENABLE
   #define DPV1_IA_EXPECTED_CFG_DATA_ENABLE        0
   #define DPV1_IA_EXPECTED_CFG_DATA_VALUE         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   #define DPV1_IA_EXPECTED_CFG_DATA_ARRAY_SIZE    10
#endif

/*
** Attribute 4: SSA enabled (BOOL - TRUE/FALSE)
*/
#ifndef DPV1_IA_SSA_ENABLED_ENABLE
   #define DPV1_IA_SSA_ENABLED_ENABLE              0
   #define DPV1_IA_SSA_ENABLED_VALUE               FALSE
#endif

/*
** Attribute 13: Revision counter (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DPV1_IA_REV_COUNTER_ENABLE
   #define DPV1_IA_REV_COUNTER_ENABLE              0
   #define DPV1_IA_REV_COUNTER_VALUE               0xFFFF
#endif

/*
** Attribute 14: Profile ID (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DPV1_IA_PROFILE_ID_ENABLE
   #define DPV1_IA_PROFILE_ID_ENABLE               0
   #define DPV1_IA_PROFILE_ID_VALUE                0xFFFF
#endif

/*
** Attribute 15: Profile specific type (UINT16 - 1-6)
*/
#ifndef DPV1_IA_PROFILE_SPEC_TYPE_ENABLE
   #define DPV1_IA_PROFILE_SPEC_TYPE_ENABLE        0
   #define DPV1_IA_PROFILE_SPEC_TYPE_VALUE         0x0004
#endif

/*
** Attribute 18: IM header (Array of UINT8)
*/
#ifndef DPV1_IA_IM_HEADER_ENABLE
   #define DPV1_IA_IM_HEADER_ENABLE                0
   #define DPV1_IA_IM_HEADER_VALUE                 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   #define DPV1_IA_IM_HEADER_ARRAY_SIZE            10
#endif

#endif /* #if DPV1_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Application data Object (0xFE)
**------------------------------------------------------------------------------
*/

/*
** These defines shall be set to the max number of process data mapping entries
** that will be required by the implementation.
** Note that each mapping entry represents a 'range' of elements from one ADI,
** meaning that if only some elements from a multi-element ADI are to be mapped
** it will require as many mapping entries as there are separate and non-
** continuous ranges of elements to map.
** Do not forget to consider remap scenarios if ABCC_CFG_REMAP_SUPPORT_ENABLED
** is enabled in abcc_driver_config.h.
*/
#ifndef AD_MAX_NUM_WRITE_MAP_ENTRIES
   #define AD_MAX_NUM_WRITE_MAP_ENTRIES             ( 64 )
#endif
#ifndef AD_MAX_NUM_READ_MAP_ENTRIES
   #define AD_MAX_NUM_READ_MAP_ENTRIES              ( 64 )
#endif

/*
** Attributes 5, 6, 7: Min, max and default attributes
**
** Enabling this will also enable and include functions that performs runtime
** min/max range checks for 'SetAttribute' operations targeting ADI elements,
** which will increase code ROM consumption.
** If disabled no range checks will be made, and the min/max will be the full
** range of each data type.
*/
#ifndef AD_IA_MIN_MAX_DEFAULT_ENABLE
   #define AD_IA_MIN_MAX_DEFAULT_ENABLE            0
#endif

/*------------------------------------------------------------------------------
** Application Object (0xFF)
**------------------------------------------------------------------------------
*/
#ifndef APP_OBJ_ENABLE
   #define APP_OBJ_ENABLE                          1
#endif

#if APP_OBJ_ENABLE
/*
** Attribute 1: Configured (BOOL - TRUE/FALSE)
*/
#ifndef APP_IA_CONFIGURED_ENABLE
   #define APP_IA_CONFIGURED_ENABLE                1
   #define APP_IA_CONFIGURED_VALUE                 FALSE
#endif

/*
** Attribute 2: Supported languages (Array of ENUM - {ABP_LANG_*})
*/
#ifndef APP_IA_SUP_LANG_ENABLE
   #define APP_IA_SUP_LANG_ENABLE                  1
   #define APP_IA_SUP_LANG_VALUE                   { ABP_LANG_ENG }
   #define APP_IA_SUP_LANG_ARRAY_SIZE              1
#endif

/*
** Attribute 3: Serial number (UINT32 - function/variable/0x00000000-0xFFFFFFFF)
*/
#ifndef APP_IA_SER_NUM_ENABLE
   #define APP_IA_SER_NUM_ENABLE                   0
   #define APP_IA_SER_NUM_VALUE                    0
#endif

/*
** Attribute 4: Parameter control sum (Array[16] of UINT8, {0x00-0xFF})
*/
#ifndef APP_IA_PAR_CRTL_SUM_ENABLE
   #define APP_IA_PAR_CRTL_SUM_ENABLE              0
   #define APP_IA_PAR_CRTL_SUM_VALUE               { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#endif

/*
** Attribute 5: Candidate firmware available (BOOL - TRUE/FALSE)
*/
#ifndef APP_IA_FW_AVAILABLE_ENABLE
   #define APP_IA_FW_AVAILABLE_ENABLE              1
#endif

/*
** Attribute 6: Hardware configurable address (BOOL - TRUE/FALSE)
*/
#ifndef APP_IA_HW_CONF_ADDR_ENABLE
   #define APP_IA_HW_CONF_ADDR_ENABLE              1
   #define APP_IA_HW_CONF_ADDR_VALUE               FALSE
#endif

/*
** Attribute 7: LED Mode (BITS32 - {0x00000000 - 0xFFFFFFFF})
*/
#ifndef APP_IA_MODE_ENABLE
   #define APP_IA_MODE_ENABLE                      0
   #define APP_IA_MODE_VALUE                       0x00000000
#endif

/*
** Application Object command support: "Get Data Notification", 0x13, (BOOL - TRUE/FALSE)
*/
#ifndef APP_CMD_GET_DATA_NOTIFICATION_ENABLE
#if MQTT_OBJ_ENABLE
   #define APP_CMD_GET_DATA_NOTIFICATION_ENABLE    1
#else
   #define APP_CMD_GET_DATA_NOTIFICATION_ENABLE    0
#endif
#endif

#endif /* #if APP_OBJ_ENABLE */

#include "abcc_api_identification.h"

#endif  /* inclusion lock */
