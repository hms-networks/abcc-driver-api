/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
*/

#include "abcc_object_config.h"

#ifndef ABCC_API_SELECT_FW_H
#define ABCC_API_SELECT_FW_H

#include "abcc.h"

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
** Common Ethernet Loaded firmwares
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
** Function pointer definition which is used to report the result of the
** select firmware function.
**------------------------------------------------------------------------------
** Arguments:
**    eResult - ABCC error code indicating success or failure.
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
** Ethernet network protocols, also refered to as "Common Ethernet Loaded".
**
** This function is used to copy a pre-loaded firmware to the CompactCom's
** firmware candidate area. The firmware to be copied shall be located in a
** folder named "/Network FW/" with the filename format
** ABCC_40_(EIP|PIR|ECT|EIT)_.*\.hiff. The provided callback is invoked after the
** operation has completed with either a success or error response. In case of
** a success response the Anybus CompactCom has to be restarted for the new
** firmware to be installed.
**------------------------------------------------------------------------------
** Arguments:
**    eFirmware - Firmware to be selected for update
**    pnResultCallback - Callback function to be called with the result of the
**                       operation.
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_SelectFirmware(
   ABCC_API_CommonEtnFirmwareType eFirmware,
   ABCC_API_pnSelectFwResultCallback pnResultCallback );

#endif

#endif /* inclusion lock */
