/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Interface for selecting a firmware from the internal file system of the
** CompactCom B40 Mini (Loaded) and copying it to the firmware candidate area in
** preparation for installation after next reset of the CompactCom B40 Mini.
********************************************************************************
*/

#ifndef ABCC_API_SELECT_FW_H
#define ABCC_API_SELECT_FW_H

#include "../src/abcc_api_config.h"
#include "abcc_error_codes.h"

#if ANB_FSI_OBJ_ENABLE

/*******************************************************************************
** Public defines
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Enable/disable debugging of the select firmware feature.
**------------------------------------------------------------------------------
*/
#ifndef ABCC_API_SELECT_FIRMWARE_DEBUG_ENABLED
#define ABCC_API_SELECT_FIRMWARE_DEBUG_ENABLED 0
#endif

/*******************************************************************************
** Public typedefs
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Common Ethernet Loaded firmwares.
**------------------------------------------------------------------------------
*/
typedef enum ABCC_API_CommonEtnFirmware
{
    ABCC_API_NW_TYPE_PROFINET = 0,
    ABCC_API_NW_TYPE_ETHERNET_IP,
    ABCC_API_NW_TYPE_ETHERCAT,
    ABCC_API_NW_TYPE_MODBUS_TCP,
    ABCC_API_NW_TYPE_LAST
}
ABCC_API_CommonEtnFirmwareType;

/*------------------------------------------------------------------------------
** Callback function type used to report the result of the firmware
** selection operation.
**------------------------------------------------------------------------------
** Arguments:
**    eResult - ABCC error code indicating success or failure.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
typedef void (*ABCC_API_pnSelectFwResultCallback)( ABCC_ErrorCodeType eResult );

/*******************************************************************************
** Public globals
********************************************************************************
*/

/*******************************************************************************
** Public services
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Some CompactCom versions are pre-loaded with firmware for the most common
** Ethernet network protocols, also referred to as "Common Ethernet Loaded".
**
** This function copies a pre-loaded firmware file to the CompactCom's
** firmware candidate area. The source file shall be located in a
** "/Network FW/" folder with the filename format:
**
**    ABCC_40_(EIP|PIR|ECT|EIT)_.*\.hiff.
**
** The provided callback is invoked upon completion with a success or error
** result. If successful, the Anybus CompactCom must be restarted to install
** the new firmware.
**------------------------------------------------------------------------------
** Arguments:
**    eFirmware        - Firmware to be selected for update.
**    pnResultCallback - Callback invoked with the operation result.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_SelectFirmware(
   ABCC_API_CommonEtnFirmwareType eFirmware,
   ABCC_API_pnSelectFwResultCallback pnResultCallback );

#endif

#endif /* inclusion lock */
