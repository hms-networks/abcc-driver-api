/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Configuration options for the ABCC API
********************************************************************************
*/

#ifndef ABCC_API_CONFIG_H
#define ABCC_API_CONFIG_H

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

#include "abcc_api_object_config.h"

#endif
