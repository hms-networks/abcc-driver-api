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
** ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED   1 - Enable / 0 - Disable
**
** Allows the application to intercept commands received from the ABCC
** before the default command handler processes them. Use this to implement
** custom handling of commands to any object or instance. The callback may
** also pass a command on to the default handler by returning FALSE.
**
** ABCC_API_CbfCommandMessageHook() must be implemented if this is enabled.
**------------------------------------------------------------------------------
*/
#ifndef ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED
    #define ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED 0
#endif

/*------------------------------------------------------------------------------
** Define this in abcc_driver_config.h to be notified about error events
** reported by the driver.
**
** If the severity is ABCC_LOG_SEVERITY_FATAL, the driver will trap in an
** infinite loop once the notification function returns. The notification is
** therefore the last chance to log, save state, or trigger a system reset.
**
** Example:
** #define ABCC_API_CONFIG_ERROR_EVENT_NOTIFY(eSeverity, iErrorCode, lAddInfo) \
** Example_AbccErrorEventNotify( eSeverity, iErrorCode, lAddInfo )
**
** Example_AbccErrorEventNotify is then implemented by the user.
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
** Define this in abcc_driver_config.h to be notified about Anybus state
** changes.
**
** Example:
** #define ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY( eNewAnbState ) \
** Example_AnybusStateChangeNotify( eNewAnbState )
**
** The Example_AnybusStateChangeNotify function definition is then implemented
** by the user. As an alternative to this notification, the Anybus state can be
** polled with ABCC_API_AnbState().
**------------------------------------------------------------------------------
*/

/*******************************************************************************
** Object configuration macros
********************************************************************************
*/
/*------------------------------------------------------------------------------
** Host-side network object support.
**
** Define to 1 in abcc_driver_config.h to enable the object, 0 to disable.
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
** Host-side object support.
**
** Define to 1 in abcc_driver_config.h to enable the object, 0 to disable.
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
** Anybus module-side object support.
**
** Define to 1 in abcc_driver_config.h to enable the object, 0 to disable.
**------------------------------------------------------------------------------
*/
#ifndef ANB_FSI_OBJ_ENABLE
   #define ANB_FSI_OBJ_ENABLE                      0
#endif
#ifndef DI_OBJ_ENABLE
   #define DI_OBJ_ENABLE                           0
#endif
/*------------------------------------------------------------------------------
** Maximum number of concurrent FSI operations tracked by the FSI object.
** Affects static memory consumption inside the FSI object.
**
** NOTE: This is *not* the same as the maximum number of existing FSI
** instances or the maximum number of open files/directories.
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
** These defines shall be set to the maximum number of process data mapping
** entries required by the implementation.
**
** NOTE: Each mapping entry represents a contiguous 'range' of elements from
** one ADI. Mapping only some elements of a multi-element ADI therefore
** requires one mapping entry per separate, non-contiguous range of
** elements.
**
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
** Enabling this also enables functions that perform runtime min/max range
** checks for SetAttribute operations targeting ADI elements, increasing
** ROM consumption.
**
** If disabled, no range checks are performed and the min/max range will be
** the full range of each data type.
*/
#ifndef AD_IA_MIN_MAX_DEFAULT_ENABLE
   #define AD_IA_MIN_MAX_DEFAULT_ENABLE            0
#endif

/*
** ADI Data Value Swapping
**
** By default, ADI values are byte-swapped between application and network byte
** order by the application data object handler. However, some use cases require
** swapping to be configurable.
**
** The defines below allow disabling the swap for specific access channels.
** When disabled, values are copied as-is (no endian conversion); stored ADI
** values must then already match the desired on-wire byte order. The default
** byte order can be determined using ABCC_NetFormat().
**
** AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
**   Disables swapping for mapped process data blocks.
** 
** AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
**    Disables swapping for message-based ADI access (typically triggered by
**    an acyclic command from the supervising PLC). Also covers the min, max,
**    and default values of the ADIs.
** 
** AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
**    Disables swapping for both channels at once. When set, it takes
**    precedence over the other two defines (which are then forced to 0),
**    simplifying configuration and reducing code size.
**
** Note: Disabling swapping for message-based access will also disable min/max
**       range checks for Set operations targeting ADI elements, since the
**       application data object handler will no longer be able to determine the
**       correct min/max values for the ADI elements (their byte order may
**       differ from application byte order).
**       To perform the range check in the application instead, use the
**       transparent set callbacks (enabled by
**       ABCC_CFG_ADI_TRANS_SET_CALLBACK_ENABLED). This conflict is reported
**       as a compiler error, which can be suppressed by defining
**       AD_CFG_OVERRIDE_WARNING_RANGE_CHECK.
** 
*/
#ifndef AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_PD         0
#endif

#ifndef AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE    0
#endif

#ifndef AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL      0
#endif

#if AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
   #undef AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_PD         0
   #undef AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
   #define AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE    0
#endif

#ifndef AD_CFG_OVERRIDE_WARNING_RANGE_CHECK
   #define AD_CFG_OVERRIDE_WARNING_RANGE_CHECK 0
#endif

#if( AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL )
   #if AD_IA_MIN_MAX_DEFAULT_ENABLE
      #if !ABCC_CFG_ADI_TRANS_SET_CALLBACK_ENABLED
         #if !AD_CFG_OVERRIDE_WARNING_RANGE_CHECK
         
            #error "WARNING: Range check (for message-based set accesses) does not work if ADI byte swap is disabled for message-based access. This warning can be overridden using the `AD_CFG_OVERRIDE_WARNING_RANGE_CHECK` define."
         
         #endif // !AD_CFG_OVERRIDE_WARNING_RANGE_CHECK
      #endif // !ABCC_CFG_ADI_TRANS_SET_CALLBACK_ENABLED
   #endif // AD_IA_MIN_MAX_DEFAULT_ENABLE
#endif

#endif
