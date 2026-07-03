/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Configuration options for the ABCC API
********************************************************************************
*/

#ifndef ABCC_API_CONFIG_H_
#define ABCC_API_CONFIG_H_

#include "abcc_driver_config.h"

/*------------------------------------------------------------------------------
** Maximum start up time when the module is upgrading its firmware
**------------------------------------------------------------------------------
*/
#ifndef ABCC_API_FW_UPGRADE_STARTUP_TIME_MS
#define ABCC_API_FW_UPGRADE_STARTUP_TIME_MS     ( 3 * 60 * (UINT32)1000 )
#endif

/*------------------------------------------------------------------------------
** Default IP configuration when using HW switches
**------------------------------------------------------------------------------
*/
#ifndef ABCC_API_DEFAULT_IP_NETWORK_ADDRESS
#define ABCC_API_DEFAULT_IP_NETWORK_ADDRESS         { 192, 168, 0, 0 }
#endif

#ifndef ABCC_API_DEFAULT_NETMASK
#define ABCC_API_DEFAULT_NETMASK                    { 255, 255, 255, 0 }
#endif

#ifndef ABCC_API_DEFAULT_GATEWAY
#define ABCC_API_DEFAULT_GATEWAY                    { 0, 0, 0, 0 }
#endif

#ifndef ABCC_API_DEFAULT_DHCP_ENABLE
#define ABCC_API_DEFAULT_DHCP_ENABLE                { FALSE }
#endif

/*------------------------------------------------------------------------------
** Comm settings values
**------------------------------------------------------------------------------
*/
#ifndef ABCC_API_DEFAULT_COMM_SETTING
#define ABCC_API_DEFAULT_COMM_SETTING               ABCC_API_COMM_SETTING_AUTO
#endif

/*------------------------------------------------------------------------------
** #define ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED   1 - Enable / 0 - Disable
**
** Allows the user to handle ABCC commands instead of the default handler.
** This can be used to implement custom handling of commands to any object or
** instance. The invoked callback function may also opt to let the default
** handler take care of the command.
**
** ABCC_API_CbfCommanMessageHook() must be implemented if this is enabled.
**------------------------------------------------------------------------------
*/
#ifndef ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED
    #define ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED 0
#endif

/*------------------------------------------------------------------------------
** Define this to be notified about error events reported by the driver. If the
** severity is of type ABCC_LOG_SEVERITY_FATAL the driver will get stuck in
** an infintite loop once/if this function returns in abcc_driver_config.h.
**
** Example:
** #define ABCC_API_CONFIG_ERROR_EVENT_NOTIFY(eSeverity, iErrorCode, lAddInfo) \
** Example_AbccErrorEventNotify( eSeverity, iErrorCode, lAddInfo )
**
** The Example_AbccErrorEventNotify function definition is then implemented by
** the user.
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
** Define this to be notified about ABCC state changes in abcc_driver_config.h.
**
** Example:
** #define ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY( eNewAnbState ) \
** Example_AnybusStateChangeNotify( eNewAnbState )
**
** The Example_AnybusStateChangeNotify function definition is then implemented
** by the user.
**------------------------------------------------------------------------------
*/

/*******************************************************************************
** Object configuration macros
********************************************************************************
*/
/*------------------------------------------------------------------------------
** Supported host-side network objects - Define to 1 in abcc_driver_config.h to
** enable the object, and 0 to disable.
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
** Supported host-side objects - Define to 1 in abcc_driver_config.h to
** enable the object, and 0 to disable.
**------------------------------------------------------------------------------
*/
#ifndef APP_OBJ_ENABLE
   #define APP_OBJ_ENABLE                          1
#endif
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
** Supported module-side objects - Define to 1 in abcc_driver_config.h to
** enable the object, and 0 to disable.
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
** Application data Object (0xFE)
** This object is required and always enabled.
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

/*
** Swapping of data values
**
** When ADI values are read/written, they are normally swapped between
** application byte order and network byte order by the application data object
** handler.
** In some use cases, configurable swapping is needed. In these cases, the
** defines AD_CFG_DISABLE_ADI_BYTE_SWAP_PD, AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
** and AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL can be used to disable value byte
** swapping within the API code according to application's needs.When swapping
** is disabled, values are copied without endian conversion; ensure that stored
** ADI values/properties match the expected on-wire byte order, wich can be
** determined by using ABCC_NetFormat().
**
** AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
** This define disables byte swapping for the mapped process data blocks.
**
** AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
** This define disables byte swapping for messages accessing the ADI values.
** These messages are typically triggered by an acyclic access command from the
** supervising PLC.
** This define will also disable byte swapping for min, max and default values
** of the ADIs.
**
** AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
** This define disables byte swapping for both access channels.
** To make the code structure simpler, this define will override the other two
** defines if set to 1.
*/ 
#ifndef AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_PD         0
#endif // !AD_CFG_DISABLE_ADI_BYTE_SWAP_PD

#ifndef AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE    0
#endif // !AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE

#ifndef AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL      0
#endif // !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL

#if AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
   #undef AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_PD         0
   #undef AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE    0
#endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL

#endif
