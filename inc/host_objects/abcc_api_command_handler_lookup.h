/*******************************************************************************
** Copyright 2018-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Command handler lookup macros and callback function definitions.
********************************************************************************
*/
#include "abcc_api.h"
#include "abp.h"
#include "abp_eip.h"
#include "abp_pnio.h"
#include "abp_mod.h"
#include "abcc_api_object_config.h"

#ifndef ABCC_HOST_ATTR_LOOKUP_H_
#define ABCC_HOST_ATTR_LOOKUP_H_

#define ABCC_API_OBJ_ATTRIBUTE_RESPONSE_LIST \
        APP_OBJ_OBJ_ATTRIBUTES \
        EIP_OBJ_OBJ_ATTRIBUTES \
        PIR_OBJ_OBJ_ATTRIBUTES \
        MOD_OBJ_OBJ_ATTRIBUTES

/*******************************************************************************
** Predefined macros to be registred into the user defined list
** ABCC_API_COMMAND_RESPONSE_LIST to select what commands to be responded to and
** how (callback function of constant value).
**
** To use constat value:
**    The macros ending with _VALUE(x) shall be registred with x replaced by the
**    value of the datatype specified in .uData.
**
** To use callback function:
**    The macros ending with _CBFUNC shall be registred and the function pointed
**    to by .uCbx shall be implemented.
**
** While both constant values and callback functions can be mixed in
** ABCC_API_COMMAND_RESPONSE_LIST, both types for a specific command cannot be
** registred simultaneously.
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Application Object (0xFF)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if APP_OBJ_ENABLE
#define APP_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "Application" }, \
                               { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x02 }, \
                               { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define APP_OBJ_OBJ_ATTRIBUTES
#endif

#if APP_OBJ_ENABLE
/* Reset (object command) */
#define ABCC_APPLICATION_OBJ_OA_RESET_CBFUNC              { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x00,                                           .bCommand = ABP_CMD_RESET,                                                                                                   .uCbx.pnResetObj       = ABCC_CbfApplicationObj_Reset }

/* Reset request (object command) */
#define ABCC_APPLICATION_OBJ_OA_RESET_REQUEST_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x00,                                           .bCommand = ABP_APP_CMD_RESET_REQUEST,                                                                                       .uCbx.pnResetReqObj    = ABCC_CbfApplicationObj_ResetRequest }

/* Configured */
#define ABCC_APPLICATION_OBJ_CONFIGURED_GET_CBFUNC        { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_CONFIGURED,   .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetUint32Attr  = ABCC_CbfApplicationObjConfigured_Get }

/* Supported languages */
#define ABCC_APPLICATION_OBJ_SUP_LANG_GET_VALUE(x, y)     { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_SUP_LANG,     .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BUFFER, .uData.pacStringBuffer    = (x),                                                                                                 .uAttrLength.iDataSize = (y) }
#define ABCC_APPLICATION_OBJ_SUP_LANG_GET_CBFUNC          { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_SUP_LANG,     .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BUFFER,                                                             .uCbx.pnGetArrAttr     = ABCC_CbfApplicationObjSupportedLang_Get }
#define ABCC_APPLICATION_OBJ_SUP_LANG_GET_ENUM_STR_CBFUNC { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_SUP_LANG,     .bCommand = ABP_CMD_GET_ENUM_STR,                                                                                            .uCbx.pnGetEnumStrAttr = ABCC_CbfApplicationObjSupportedLang_GetEnumString }

/* Serial number */
#define ABCC_APPLICATION_OBJ_SERIAL_NR_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_SER_NUM,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }
#define ABCC_APPLICATION_OBJ_SERIAL_NR_GET_CBFUNC         { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_SER_NUM,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32,                                                             .uCbx.pnGetUint32Attr  = ABCC_CbfApplicationObjSerialNum_Get }

/* Parameter control sum */
#define ABCC_APPLICATION_OBJ_PARAM_CTRL_SUM_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_PAR_CRTL_SUM, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BUFFER, .uAttrLength.iMaxDataSize = ABP_APP_IA_PAR_CRTL_SUM_DS,     .uCbx.pnGetArrAttr     = ABCC_CbfApplicationObjParamControlSum_Get }

/* Vendor name */
#define ABCC_APPLICATION_OBJ_VENDOR_NAME_GET_VALUE(x)     { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_VENDOR_NAME,  .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uData.pacString          = (x) }
#define ABCC_APPLICATION_OBJ_VENDOR_NAME_GET_CBFUNC       { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_VENDOR_NAME,  .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_APP_IA_VENDOR_NAME_MAX_DS,  .uCbx.pnGetStrAttr     = ABCC_CbfApplicationObjVendorName_Get }

/* Product name */
#define ABCC_APPLICATION_OBJ_PRODUCT_NAME_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_PRODUCT_NAME, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uData.pacString          = (x) }
#define ABCC_APPLICATION_OBJ_PRODUCT_NAME_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_PRODUCT_NAME, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_APP_IA_PRODUCT_NAME_MAX_DS, .uCbx.pnGetStrAttr     = ABCC_CbfApplicationObjProductName_Get }

/* Firmware version */
#define ABCC_APPLICATION_OBJ_FW_VERSION_GET_VALUE(x)      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_FW_VERSION,   .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BUFFER, .uData.pacStringBuffer    = (x),                                                                                                 .uAttrLength.iDataSize = ABP_APP_IA_FW_VERSION_DS }
#define ABCC_APPLICATION_OBJ_FW_VERSION_GET_CBFUNC        { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_FW_VERSION,   .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BUFFER, .uAttrLength.iMaxDataSize = ABP_APP_IA_FW_VERSION_DS,       .uCbx.pnGetArrAttr     = ABCC_CbfApplicationObjFWVersion_Get }

/* Hardware version */
#define ABCC_APPLICATION_OBJ_HW_VERSION_GET_VALUE(x)      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_VERSION,   .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.lUnsigned16        = (x) }
#define ABCC_APPLICATION_OBJ_HW_VERSION_GET_CBFUNC        { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_VERSION,   .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16,                                                             .uCbx.pnGetUint16Attr  = ABCC_CbfApplicationObjHWVersion_Get }

/* Firmware Available flag */
#define ABCC_APPLICATION_OBJ_FW_AVAILABLE_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_FW_AVAILABLE, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr   = ABCC_CbfApplicationObjFirmwareAvailable_Get }
#define ABCC_APPLICATION_OBJ_FW_AVAILABLE_SET_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_FW_AVAILABLE, .bCommand = ABP_CMD_SET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnSetBool8Attr   = ABCC_CbfApplicationObjFirmwareAvailable_Set }

/* Hardware configurable address */
#define ABCC_APPLICATION_OBJ_HW_CONF_ADDR_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_CONF_ADDR, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_APPLICATION_OBJ_HW_CONF_ADDR_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_CONF_ADDR, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfApplicationObjHWConfAddress_Get }
#endif

/*------------------------------------------------------------------------------
** EtherNet/IP Host Object (0xF8)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if EIP_OBJ_ENABLE
#define EIP_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,            .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "EtherNet/IP" }, \
                               { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,             .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x02 }, \
                               { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,        .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST,    .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define EIP_OBJ_OBJ_ATTRIBUTES
#endif

#if EIP_OBJ_ENABLE
/* Vendor ID */
#define ABCC_ETHERNETIP_OBJ_VENDOR_ID_GET_VALUE(x)   { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_VENDOR_ID,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16   = (x) }
#define ABCC_ETHERNETIP_OBJ_VENDOR_ID_GET_CBFUNC     { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_VENDOR_ID,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                             .uCbx.pnGetUint16Attr = ABCC_CbfEthernetIpObjVendorId_Get }

/* Device Type */
#define ABCC_ETHERNETIP_OBJ_DEVICE_TYPE_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_DEVICE_TYPE,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16   = (x) }
#define ABCC_ETHERNETIP_OBJ_DEVICE_TYPE_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_DEVICE_TYPE,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                             .uCbx.pnGetUint16Attr = ABCC_CbfEthernetIpObjDeviceType_Get }

/* Product Code */
#define ABCC_ETHERNETIP_OBJ_PRODUCT_CODE_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_PRODUCT_CODE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = (x) }
#define ABCC_ETHERNETIP_OBJ_PRODUCT_CODE_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_PRODUCT_CODE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                             .uCbx.pnGetUint16Attr = ABCC_CbfEthernetIpObjProductCode_Get }

#define ABCC_ETHERNETIP_OBJ_REVISION_GET_VALUE(x)     { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_REVISION,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uData.pacStringBuffer = (x),                                                             .uAttrLength.iDataSize = ABP_EIP_IA_REVISION_DS }

#endif

/*------------------------------------------------------------------------------
** PROFINET IO Object (0xF6)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if PIR_OBJ_ENABLE
#define PIR_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "PROFINET IO" }, \
                               { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x02 }, \
                               { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define PIR_OBJ_OBJ_ATTRIBUTES
#endif

#if PIR_OBJ_ENABLE
/* Device ID */
#define ABCC_PROFINET_OBJ_DEVICE_ID_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_DEVICE_ID,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16      = (x) }
#define ABCC_PROFINET_OBJ_DEVICE_ID_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_DEVICE_ID,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                                                       .uCbx.pnGetUint16Attr = ABCC_CbfProfinetIoObjDeviceId_Get }

/* Vendor ID */
#define ABCC_PROFINET_OBJ_VENDOR_ID_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_VENDOR_ID,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16      = (x) }
#define ABCC_PROFINET_OBJ_VENDOR_ID_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_VENDOR_ID,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                                                       .uCbx.pnGetUint16Attr = ABCC_CbfProfinetIoObjVendorId_Get }

/* Order ID */
#define ABCC_PROFINET_OBJ_ORDER_ID_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_ORDER_ID, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }
#define ABCC_PROFINET_OBJ_ORDER_ID_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_ORDER_ID, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_PNIO_IA_IM_ORDER_ID_DS, .uCbx.pnGetStrAttr    = ABCC_CbfProfinetIoObjOrderId_Get }
#endif

/*------------------------------------------------------------------------------
** Modbus Host Object (0xFA)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if MOD_OBJ_ENABLE
#define MOD_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "Modbus" }, \
                               { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x01 }, \
                               { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define MOD_OBJ_OBJ_ATTRIBUTES
#endif

#if MOD_OBJ_ENABLE
/* Order ID */
#define ABCC_MODBUS_OBJ_RW_OFFSET_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_RW_OFFSET, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uData.pacStringBuffer = (x),                                                                                  .uAttrLength.iDataSize = ABP_MOD_IA_RW_OFFSET_DS }
#define ABCC_MODBUS_OBJ_RW_OFFSET_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_RW_OFFSET, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uAttrLength.iMaxDataSize = ABP_MOD_IA_RW_OFFSET_DS, .uCbx.pnGetArrAttr = ABCC_CbfModbusObjReadWriteOffset_Get }
#define ABCC_MODBUS_OBJ_RW_OFFSET_SET_CBFUNC   { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_RW_OFFSET, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uAttrLength.iMaxDataSize = ABP_MOD_IA_RW_OFFSET_DS, .uCbx.pnSetArrAttr = ABCC_CbfModbusObjReadWriteOffset_Set }
#endif

/*******************************************************************************
** Predefined callback function prototypes used by command_handler_lookup_table.
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Application Object (0xFF)
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** Callback function to perform a reset of the device, requested by the network.
**------------------------------------------------------------------------------
** Arguments:
**       eResetType - The type of reset to be performed.
**          ABP_RESET_POWER_ON                 - Power-on reset
**          ABP_RESET_FACTORY_DEFAULT          - Factory default reset
**          ABP_RESET_POWER_ON_FACTORY_DEFAULT - Power-on + Factory reset
**
** Returns:
**       None.
**------------------------------------------------------------------------------
*/
void ABCC_CbfApplicationObj_Reset( ABP_ResetType eResetType );

/*------------------------------------------------------------------------------
** Callback function to accept or decline a reset of the device, requested by
** the network. This is only a request and not the command to perform the actual
** reset.
**------------------------------------------------------------------------------
** Arguments:
**       eResetType - The type of reset that is requested.
**          ABP_RESET_POWER_ON                 - Power-on reset
**          ABP_RESET_FACTORY_DEFAULT          - Factory default reset
**          ABP_RESET_POWER_ON_FACTORY_DEFAULT - Power-on + Factory reset
**
** Returns:
**       True:  The reset request is accepted and a reset command of the type
**              specified in argument eResetType can be sent by the CompactCom.
**       False: The reset request is declined and no reset command shall be sent
**              by the CompactCom.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfApplicationObj_ResetRequest( ABP_ResetType eResetType );

/*------------------------------------------------------------------------------
** Callback function to provide wether the device is configured from it's
** out-of-box (default) values or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  The device is configured.
**       False: The device is in out-of-box (default) state.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfApplicationObjConfigured_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the supported languages of the device to the
** CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedArrDest - Pointer to buffer where a packed ENUM array with
**                        little-endian byte order shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       Size of the inserted array in bytes.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfApplicationObjSupportedLang_Get( UINT16* pPackedArrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to provide the language strings supported by the device.
**------------------------------------------------------------------------------
** Arguments:
**       bEnumValue     - The enum value of the requested string.
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       True: Enumeration value is in range (supported).
**       True: Enumeration value is out of range (not supported).
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfApplicationObjSupportedLang_GetEnumString( UINT8 bEnumValue, char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to provide the device's serial number to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       Serial number of the device.
**------------------------------------------------------------------------------
*/
UINT32 ABCC_CbfApplicationObjSerialNum_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the parameter control sum. The parameter control
** sum can help improving startup time on some network protocols.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedArrDest - Pointer to buffer where a packed UINT8 array of
**                        maximum length 16 (size 16 in bytes) and little-endian
**                        byte order shallbe written.
**       iBuffSizeBytes - Size of the buffer in bytes. Expect this to be 16.
**
** Returns:
**       Size of the inserted array in bytes.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfApplicationObjParamControlSum_Get( UINT16* pPackedArrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to provide the network-independent vendor name to the
** CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       Vendor name of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfApplicationObjVendorName_Get( char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to provide the network-independent product name to the
** CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       Product name of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfApplicationObjProductName_Get( char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to provide the Firmware Available flag that is stored in
** non-volatile storage on the device.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  New firmware image in the CompactCom candidate area.
**       False: No new firmware image in the CompactCom candidate area.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfApplicationObjFirmwareAvailable_Get( void );

/*------------------------------------------------------------------------------
** Callback function to set the Firmware Available flag in non-volatile storage
** on the device.
**------------------------------------------------------------------------------
** Arguments:
**       fValue - Firmware Available flag.
**         True:  New firmware image in the CompactCom candidate area.
**         False: No new firmware image in the CompactCom candidate area.
**
** Returns:
**       None.
**------------------------------------------------------------------------------
*/
void ABCC_CbfApplicationObjFirmwareAvailable_Set( BOOL8 fValue );

/*------------------------------------------------------------------------------
** Callback function to state wether the network address is configurable via
** hardware, such as switches or similar. This information is required by some
** network protocols, e.g., EtherNet/IP.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Address of the module is configurable via hardware switches.
**       False: Address of the module is not hardware configurable.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfApplicationObjHWConfAddress_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the firmware version of the device.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedArrDest - Pointer to buffer where a packed UINT8 array of length
**                        3 (size 3 in bytes) and little-endian byte order shall
**                        be written. The most significant byte is the major
**                        version, the middle is the minor version, and the
**                        least significant is the build version. Use
**                        APPL_FwVersionType as assistance.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       Size of the array in bytes, always 3 in this case. Needed for internal
**       logic of the driver.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfApplicationObjFWVersion_Get( UINT16* pPackedArrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to provide the hardware version of the device.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       Hardware version.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfApplicationObjHWVersion_Get( void );

/*------------------------------------------------------------------------------
** EtherNet/IP Host Object (0xF8)
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** Callback function to provide the EtherNet/IP Vendor ID to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       EtherNet/IP Vendor ID of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfEthernetIpObjVendorId_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the EtherNet/IP Device Type to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       EtherNet/IP Device Type of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfEthernetIpObjDeviceType_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the EtherNet/IP Product Code to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       EtherNet/IP Product Code of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfEthernetIpObjProductCode_Get( void );

/*------------------------------------------------------------------------------
** PROFINET IO Object (0xF6)
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** Callback function to provide the PROFINET Device ID to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       PROFINET Device ID of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjDeviceId_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the PROFINET Vendor ID to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       PROFINET Vendor ID of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjVendorId_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the PROFINET Order ID to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       PROFINET Order ID of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjOrderId_Get( char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Modbus Host Object (0xFA)
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** Callback function to provide the offsets for the various Modbus read/write
** holding register commands. The offset will be added to the requested register
** before forwarded internally.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedArrDest - Pointer to buffer where a packed SINT16 array of
**                        length 2 (size 4 in bytes) and little-endian byte
**                        order shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       Size of the array in bytes, always 4 in this case. Needed for internal
**       logic of the driver.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfModbusObjReadWriteOffset_Get( UINT16* pPackedArrDest, UINT16 iBuffSizeBytes );
#endif
