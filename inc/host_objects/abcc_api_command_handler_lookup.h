/*******************************************************************************
** Copyright 2018-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Command handler lookup macros and callback function definitions.
********************************************************************************
*/

#ifndef ABCC_HOST_ATTR_LOOKUP_H_
#define ABCC_HOST_ATTR_LOOKUP_H_

#include "../../src/abcc_api_config.h"
#include "abp.h"
#include "abp_bac.h"
#include "abp_cop.h"
#include "abp_cfn.h"
#include "abp_ect.h"
#include "abp_eip.h"
#include "abp_pnio.h"
#include "abp_mod.h"
#include "abp_etn.h"
#include "abcc_types.h"

#define ABCC_API_OBJ_ATTRIBUTE_RESPONSE_LIST \
        APP_OBJ_OBJ_ATTRIBUTES \
        BAC_OBJ_OBJ_ATTRIBUTES \
        COP_OBJ_OBJ_ATTRIBUTES \
        CFN_OBJ_OBJ_ATTRIBUTES \
        ECT_OBJ_OBJ_ATTRIBUTES \
        EIP_OBJ_OBJ_ATTRIBUTES \
        PIR_OBJ_OBJ_ATTRIBUTES \
        MOD_OBJ_OBJ_ATTRIBUTES \
        ETN_OBJ_OBJ_ATTRIBUTES

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
#define ABCC_APPLICATION_OBJ_HW_VERSION_GET_VALUE(x)      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_VERSION,   .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16        = (x) }
#define ABCC_APPLICATION_OBJ_HW_VERSION_GET_CBFUNC        { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_VERSION,   .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16,                                                             .uCbx.pnGetUint16Attr  = ABCC_CbfApplicationObjHWVersion_Get }

/* Firmware Available flag */
#define ABCC_APPLICATION_OBJ_FW_AVAILABLE_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_FW_AVAILABLE, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr   = ABCC_CbfApplicationObjFirmwareAvailable_Get }
#define ABCC_APPLICATION_OBJ_FW_AVAILABLE_SET_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_FW_AVAILABLE, .bCommand = ABP_CMD_SET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnSetBool8Attr   = ABCC_CbfApplicationObjFirmwareAvailable_Set }

/* Hardware configurable address */
#define ABCC_APPLICATION_OBJ_HW_CONF_ADDR_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_CONF_ADDR, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_APPLICATION_OBJ_HW_CONF_ADDR_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_APP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_APP_IA_HW_CONF_ADDR, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfApplicationObjHWConfAddress_Get }
#endif

/*------------------------------------------------------------------------------
** BACnet Host Object (0xEF)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if BAC_OBJ_ENABLE
#define BAC_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "BACnet" }, \
                               { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x01 }, \
                               { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define BAC_OBJ_OBJ_ATTRIBUTES
#endif

#if BAC_OBJ_ENABLE
/* Object name (max length: 64)*/
#define ABCC_BACNET_OBJ_OA_OBJ_NAME_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x01, .uCmdExt.bAttr = ABP_BAC_IA_OBJECT_NAME,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Vendor name (max length: 64)*/
#define ABCC_BACNET_OBJ_OA_VENDOR_NAME_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x01, .uCmdExt.bAttr = ABP_BAC_IA_VENDOR_NAME,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Vendor identifier */
#define ABCC_BACNET_OBJ_OA_VENDOR_ID_GET_VALUE(x)   { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x01, .uCmdExt.bAttr = ABP_BAC_IA_VENDOR_IDENTIFIER,    .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16    = (x) }

/* Model name (max length: ?)*/
#define ABCC_BACNET_OBJ_OA_MODLE_NAME_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x01, .uCmdExt.bAttr = ABP_BAC_IA_MODEL_NAME,           .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Firmware revision (max length: 16)*/
#define ABCC_BACNET_OBJ_OA_FW_REV_GET_VALUE(x)      { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x01, .uCmdExt.bAttr = ABP_BAC_IA_FIRMWARE_REVISION,    .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Application software version (max length: 16)*/
#define ABCC_BACNET_OBJ_OA_APP_SW_VER_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x01, .uCmdExt.bAttr = ABP_BAC_IA_APP_SOFTWARE_VERSION, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Password (max length: 20)*/
#define ABCC_BACNET_OBJ_OA_PASSWORD_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_BAC, .bInstance = 0x01, .uCmdExt.bAttr = ABP_BAC_IA_PASSWORD,             .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }
#endif

/*------------------------------------------------------------------------------
** CANopen Object (0xFB)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if COP_OBJ_ENABLE
#define COP_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "CANopen" }, \
                               { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x01 }, \
                               { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define COP_OBJ_OBJ_ATTRIBUTES
#endif

#if COP_OBJ_ENABLE
/* Vendor ID */
#define ABCC_CANOPEN_OBJ_VENDOR_ID_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_VENDOR_ID,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }

/* Product code */
#define ABCC_CANOPEN_OBJ_PRODUCT_CODE_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_PRODUCT_CODE,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }

/* Major revision */
#define ABCC_CANOPEN_OBJ_MAJOR_REV_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_MAJOR_REV,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16        = (x) }

/* Minor revision */
#define ABCC_CANOPEN_OBJ_MINOR_REV_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_MINOR_REV,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16        = (x) }

/* Serial number */
#define ABCC_CANOPEN_OBJ_SERIAL_NUM_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_SERIAL_NUMBER,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }
#define ABCC_CANOPEN_OBJ_SERIAL_NUM_GET_CBFUNC       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_SERIAL_NUMBER,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32,                                                             .uCbx.pnGetUint32Attr  = ABCC_CbfCANopenObjSerialNumber_Get }

/* Manufacturer device name (max length: 64)*/
#define ABCC_CANOPEN_OBJ_MANF_DEV_NAME_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_MANF_DEV_NAME,      .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Manufacturer hardware version (max length: 64)*/
#define ABCC_CANOPEN_OBJ_MANF_HW_VER_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_MANF_HW_VER,      .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Manufacturer software version (max length: 64)*/
#define ABCC_CANOPEN_OBJ_MANF_SW_VER_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_MANF_SW_VER,      .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Device type */
#define ABCC_CANOPEN_OBJ_DEVICE_TYPE_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_COP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_COP_IA_DEVICE_TYPE,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }
#endif

/*------------------------------------------------------------------------------
** CC-Link IE Field Network Host Object (0xE6)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if CFN_OBJ_ENABLE
#define CFN_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "CC-Link IE Field Network" }, \
                               { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x01 }, \
                               { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define CFN_OBJ_OBJ_ATTRIBUTES
#endif

#if CFN_OBJ_ENABLE
/* Vendor code */
#define ABCC_CCLINKIE_OBJ_VENDOR_CODE_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_CFN_IA_VENDOR_CODE,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16        = (x) }

/* Vendor name (max length: 31)*/
#define ABCC_CCLINKIE_OBJ_VENDOR_NAME_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_CFN_IA_VENDOR_NAME,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Model/Device type */
#define ABCC_CCLINKIE_OBJ_MODEL_TYPE_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_CFN_IA_MODEL_TYPE,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16        = (x) }

/* Model name (max length: 19)*/
#define ABCC_CCLINKIE_OBJ_MODEL_NAME_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_CFN_IA_MODEL_NAME,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Model code */
#define ABCC_CCLINKIE_OBJ_MODEL_CODE_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_CFN_IA_MODEL_CODE,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }

/* Software version */
#define ABCC_CCLINKIE_OBJ_SW_VERSION_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_CFN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_CFN_IA_SW_VERSION,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT8, .uData.bUnsigned8        = (x) }
#endif

/*------------------------------------------------------------------------------
** EtherCAT Object (0xF5)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if ECT_OBJ_ENABLE
#define CFN_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "EtherCAT" }, \
                               { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x01 }, \
                               { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define ECT_OBJ_OBJ_ATTRIBUTES
#endif

#if ECT_OBJ_ENABLE
/* Vendor ID */
#define ABCC_ETHERCAT_OBJ_VENDOR_ID_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_VENDOR_ID,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }

/* Product code */
#define ABCC_ETHERCAT_OBJ_PRODUCT_CODE_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_PRODUCT_CODE,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }

/* Major revision */
#define ABCC_ETHERCAT_OBJ_MAJOR_REV_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_MAJOR_REV,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16        = (x) }

/* Minor revision */
#define ABCC_ETHERCAT_OBJ_MINOR_REV_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_MINOR_REV,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16        = (x) }

/* Serial number */
#define ABCC_ETHERCAT_OBJ_SERIAL_NUM_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_SERIAL_NUMBER,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }
#define ABCC_ETHERCAT_OBJ_SERIAL_NUM_GET_CBFUNC       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_SERIAL_NUMBER,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32,                                                             .uCbx.pnGetUint32Attr  = ABCC_CbfEtherCATObjSerialNmber_Get }

/* Device name (max length: 64)*/
#define ABCC_ETHERCAT_OBJ_DEVICE_NAME_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_MANF_DEV_NAME,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Device hardware version ("X.YY")  (max length: 64)*/
#define ABCC_ETHERCAT_OBJ_HW_VER_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_MANF_HW_VER,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Device software version ("X.YY.ZZ") (max length: 64)*/
#define ABCC_ETHERCAT_OBJ_SW_VER_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_MANF_SW_VER,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Device type */
#define ABCC_ETHERCAT_OBJ_DEVICE_TYPE_GET_VALUE(x)       { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_DEVICE_TYPE,      .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32        = (x) }

/* Enable FoE */
#define ABCC_ETHERCAT_OBJ_FOE_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_ENABLE_FOE, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERCAT_OBJ_FOE_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_ENABLE_FOE, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEtherCATObjEnableFoE_Get }

/* Enable EoE */
#define ABCC_ETHERCAT_OBJ_EOE_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_ENABLE_EOE, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERCAT_OBJ_EOE_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_ENABLE_EOE, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEtherCATObjEnableEoE_Get }

/* Set Device ID as configured alias */
#define ABCC_ETHERCAT_OBJ_DEV_ID_AS_CSA_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_SET_DEV_ID_AS_CSA, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERCAT_OBJ_DEV_ID_AS_CSA_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_ECT, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ECT_IA_SET_DEV_ID_AS_CSA, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEtherCATObjDeviceIDAsAlias_Get }
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

/* Device type */
#define ABCC_ETHERNETIP_OBJ_DEVICE_TYPE_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_DEVICE_TYPE,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16   = (x) }
#define ABCC_ETHERNETIP_OBJ_DEVICE_TYPE_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_DEVICE_TYPE,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                             .uCbx.pnGetUint16Attr = ABCC_CbfEthernetIpObjDeviceType_Get }

/* Product code */
#define ABCC_ETHERNETIP_OBJ_PRODUCT_CODE_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_PRODUCT_CODE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = (x) }
#define ABCC_ETHERNETIP_OBJ_PRODUCT_CODE_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_PRODUCT_CODE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                             .uCbx.pnGetUint16Attr = ABCC_CbfEthernetIpObjProductCode_Get }

/* Revision */
#define ABCC_ETHERNETIP_OBJ_REVISION_GET_VALUE(x)     { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_REVISION,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uData.pacStringBuffer = (x),                                                             .uAttrLength.iDataSize = ABP_EIP_IA_REVISION_DS }

/* Serial number */
#define ABCC_ETHERNETIP_OBJ_SERIAL_NUM_GET_VALUE(x)   { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_SERIAL_NUMBER,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32   = (x) }
#define ABCC_ETHERNETIP_OBJ_SERIAL_NUM_GET_CBFUNC     { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_SERIAL_NUMBER,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT32,                             .uCbx.pnGetUint32Attr = ABCC_CbfEthernetIpObjSerialNumber_Get }

/* Product name */
#define ABCC_ETHERNETIP_OBJ_PRODUCT_NAME_VALUE(x)     { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_PRODUCT_NAME,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Enable communication settings from network */
#define ABCC_ETHERNETIP_OBJ_COMM_SETTINGS_FROM_NET_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_COMM_SETTINGS_FROM_NET, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNETIP_OBJ_COMM_SETTINGS_FROM_NET_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_COMM_SETTINGS_FROM_NET, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEthernetIpObjCommSettingsFromNet_Get }

/* CIP Request forwarding */
#define ABCC_ETHERNETIP_OBJ_CIP_FORWARDING_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNETIP_OBJ_CIP_FORWARDING_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ENABLE_APP_CIP_OBJECTS, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEthernetIpObjCipForwarding_Get }

/* Disable strict IO match */
#define ABCC_ETHERNETIP_OBJ_STRICT_IO_MATCH_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_DISABLE_STRICT_IO_MATCH, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNETIP_OBJ_STRICT_IO_MATCH_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_DISABLE_STRICT_IO_MATCH, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEthernetIpObjDisableStrictIoMatch_Get }

/* Enable EtherNet/IP QuickConnect */
#define ABCC_ETHERNETIP_OBJ_QUICK_CONNECT_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ENABLE_EIP_QC, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNETIP_OBJ_QUICK_CONNECT_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ENABLE_EIP_QC, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEthernetIpObjEnableQuickConnect_Get }

/* ABCC ADI Object Number */
#define ABCC_ETHERNETIP_OBJ_ADI_OBJECT_NR_GET_VALUE(x)   { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ABCC_ADI_OBJECT,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16   = (x) }
#define ABCC_ETHERNETIP_OBJ_ADI_OBJECT_NR_GET_CBFUNC     { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ABCC_ADI_OBJECT,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                             .uCbx.pnGetUint16Attr = ABCC_CbfEthernetIpObjAdiObjectNumber_Get }

/* Enable EtherNet/IP QuickConnect */
#define ABCC_ETHERNETIP_OBJ_DLR_GET_VALUE(x)    { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ENABLE_EIP_QC, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNETIP_OBJ_DLR_GET_CBFUNC      { .bObject = ABP_OBJ_NUM_EIP, .bInstance = 0x01, .uCmdExt.bAttr = ABP_EIP_IA_ENABLE_EIP_QC, .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,                                                              .uCbx.pnGetBool8Attr    = ABCC_CbfEthernetIpObjEnableDLR_Get }



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

/* Station Type */
#define ABCC_PROFINET_OBJ_STATION_TYPE_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_STATION_TYPE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }
#define ABCC_PROFINET_OBJ_STATION_TYPE_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_STATION_TYPE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_PNIO_IA_STATION_TYPE_DS, .uCbx.pnGetStrAttr    = ABCC_CbfProfinetIoObjStationType_Get }

/* Max. no. of simultaneous application relationships (ARs) (valid range: 1-4) */
#define ABCC_PROFINET_OBJ_MAX_AR_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_MAX_AR,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT32, .uData.lUnsigned32      = (x) }
#define ABCC_PROFINET_OBJ_MAX_AR_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_MAX_AR,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT32,                                                       .uCbx.pnGetUint32Attr = ABCC_CbfProfinetIoObjMaxAr_Get }

/* Order ID */
#define ABCC_PROFINET_OBJ_ORDER_ID_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_ORDER_ID, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }
#define ABCC_PROFINET_OBJ_ORDER_ID_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_ORDER_ID, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_PNIO_IA_IM_ORDER_ID_DS, .uCbx.pnGetStrAttr    = ABCC_CbfProfinetIoObjOrderId_Get }

/* Serial Number */
#define ABCC_PROFINET_OBJ_SERIAL_NUM_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_SERIAL_NBR, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }
#define ABCC_PROFINET_OBJ_SERIAL_NUM_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_SERIAL_NBR, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_PNIO_IA_IM_SERIAL_NBR_DS, .uCbx.pnGetStrAttr    = ABCC_CbfProfinetIoObjSerialNumber_Get }

/* Hardware revision */
#define ABCC_PROFINET_OBJ_HW_REV_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_HW_REV,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16      = (x) }
#define ABCC_PROFINET_OBJ_HW_REV_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_HW_REV,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                                                       .uCbx.pnGetUint16Attr = ABCC_CbfProfinetIoObjHwRevision_Get }

/* Revision counter */
#define ABCC_PROFINET_OBJ_REV_COUNTER_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_REV_CNT,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16      = (x) }
#define ABCC_PROFINET_OBJ_REV_COUNTER_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_REV_CNT,   .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                                                       .uCbx.pnGetUint16Attr = ABCC_CbfProfinetIoObjRevisionCounter_Get }

/* I&M5 Module order ID */
#define ABCC_PROFINET_OBJ_MODULE_ORDER_ID_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_MODULE_ORDER_ID, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }

/* I&M5 Annotation */
#define ABCC_PROFINET_OBJ_ANNOTATION_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM_ANNOTATION, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }

/* I&M5 enabled */
#define ABCC_PROFINET_OBJ_IM5_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_IM5_ENABLED, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }

/* S2 redundancy protocol */
#define ABCC_PROFINET_OBJ_S2_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_S2_ENABLED, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_PROFINET_OBJ_S2_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_S2_ENABLED, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfProfinetIoObjS2Enabled_Get }
#define ABCC_PROFINET_OBJ_S2_ENABLED_SET_CBFUNC    { .bObject = ABP_OBJ_NUM_PNIO, .bInstance = 0x01, .uCmdExt.bAttr = ABP_PNIO_IA_S2_PRIMARY_AR_HANDLE,   .bCommand = ABP_CMD_SET_ATTR, .eServiceTag = SERVICE_UINT16,                                                   .uCbx.pnSetUint16Attr = ABCC_CbfProfinetIoObjS2PrimaryArHandle_Set }
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
/* Vendor name (max length: 244)*/
#define ABCC_MODBUS_OBJ_VENDOR_NAME_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_VENDOR_NAME,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Product code (max length: 244)*/
#define ABCC_MODBUS_OBJ_PRODUCT_CODE_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_PRODUCT_CODE,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Revision (max length: 244)*/
#define ABCC_MODBUS_OBJ_REVISION_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_REVISION,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Vendor URL (max length: 244)*/
#define ABCC_MODBUS_OBJ_VENDOR_URL_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_VENDOR_URL,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Product name (max length: 244)*/
#define ABCC_MODBUS_OBJ_PRODUCT_NAME_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_PRODUCT_NAME,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* Model name (max length: 244)*/
#define ABCC_MODBUS_OBJ_MODEL_NAME_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_MODEL_NAME,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* User application name (max length: 244)*/
#define ABCC_MODBUS_OBJ_SER_APP_NAME_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_USER_APP_NAME,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString      = (x) }

/* No. of ADI indexing bits */
#define ABCC_APPLICATION_OBJ_ADI_INDEXING_BITS_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_ADI_INDEXING_BITS,     .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_UINT8, .uData.bUnsigned8 = (x) }

/* No. of ADI indexing bits */
#define ABCC_APPLICATION_OBJ_ADI_INDEXING_BITS_VALUE(x)     { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_MESSAGE_FORWARDING,     .bCommand = ABP_CMD_GET_ATTR,     .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }

/* Modbus read/write offset, value format: _VALUE("\xRD\xWR") */
#define ABCC_MODBUS_OBJ_RW_OFFSET_GET_VALUE(x) { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_RW_OFFSET, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uData.pacStringBuffer = (x),                                                                                  .uAttrLength.iDataSize = ABP_MOD_IA_RW_OFFSET_DS }
#define ABCC_MODBUS_OBJ_RW_OFFSET_GET_CBFUNC   { .bObject = ABP_OBJ_NUM_MOD, .bInstance = 0x01, .uCmdExt.bAttr = ABP_MOD_IA_RW_OFFSET, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uAttrLength.iMaxDataSize = ABP_MOD_IA_RW_OFFSET_DS, .uCbx.pnGetArrAttr = ABCC_CbfModbusObjReadWriteOffset_Get }
#endif

/*------------------------------------------------------------------------------
** Ethernet Host Object (F9h)
**------------------------------------------------------------------------------
*/
/* Object attributes */
#if ETN_OBJ_ENABLE
#define ETN_OBJ_OBJ_ATTRIBUTES { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NAME,         .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString   = "Ethernet" }, \
                               { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_REV,          .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT8,  .uData.bUnsigned8  = 0x02 }, \
                               { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_NUM_INST,     .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 }, \
                               { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x00, .uCmdExt.bAttr = ABP_OA_HIGHEST_INST, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16, .uData.iUnsigned16 = 0x0001 },
#else
#define ETN_OBJ_OBJ_ATTRIBUTES
#endif

#if ETN_OBJ_ENABLE

/* HICP enabled */
#define ABCC_ETHERNET_OBJ_HICP_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_HICP, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_HICP_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_HICP, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjHicpEnabled_Get }

/* Web server enabled */
#define ABCC_ETHERNET_OBJ_WEB_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_WEB, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_WEB_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_WEB, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjWebEnabled_Get }

/* Web server ADI access enabled */
#define ABCC_ETHERNET_OBJ_WEB_ADI_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_WEB_ADI_ACCESS, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_WEB_ADI_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_WEB_ADI_ACCESS, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjWebAdiEnabled_Get }

/* FTP server enabled */
#define ABCC_ETHERNET_OBJ_FTP_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_FTP, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_FTP_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_FTP, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjFtpEnabled_Get }

/* Admin mode enabled */
#define ABCC_ETHERNET_OBJ_ADMIN_MODE_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_ADMIN_MODE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_ADMIN_MODE_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_ADMIN_MODE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjAdminMode_Get }

/* Network status bitfield */
#define ABCC_ETHERNET_OBJ_NW_STATUS_SET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_NETWORK_STATUS, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_UINT16,                                                                .uCbx.pnSetUint16Attr = ABCC_CbfEthernetObjNetworkStatus_Set }

/* Address conflict detection (ACD) enabled */
#define ABCC_ETHERNET_OBJ_ACD_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_ACD, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_ACD_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_ACD, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjAcdEnabled_Get }

/* Reset from HICP enabled */
#define ABCC_ETHERNET_OBJ_HICP_RESET_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_HICP_RESET, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_HICP_RESET_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_HICP_RESET, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjHicpResetEnabled_Get }

/* IP configuration */
#define ABCC_ETHERNET_OBJ_IP_CONFIGURATION_SET_CBFUNC   { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_IP_CONFIGURATION, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BUFFER, .uAttrLength.iMaxDataSize = ABP_ETN_IA_IP_CONFIGURATION_DS, .uCbx.pnSetArrAttr = ABCC_CbfEthernetObjIpConfiguration_Set }

/* SNMP read-only community string */
#define ABCC_ETHERNET_OBJ_SNMP_READ_ONLY_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_SNMP_READ_ONLY, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }
#define ABCC_ETHERNET_OBJ_SNMP_READ_ONLY_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_SNMP_READ_ONLY, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_ETN_IA_SNMP_READ_ONLY_DS, .uCbx.pnGetStrAttr    = ABCC_CbfEthernetObjSnmpReadOnlyString_Get }

/* SNMP read-write community string */
#define ABCC_ETHERNET_OBJ_SNMP_READ_WRITE_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_SNMP_READ_WRITE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uData.pacString        = (x) }
#define ABCC_ETHERNET_OBJ_SNMP_READ_WRITE_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_SNMP_READ_WRITE, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_STR,    .uAttrLength.iMaxDataSize = ABP_ETN_IA_SNMP_READ_WRITE_DS, .uCbx.pnGetStrAttr    = ABCC_CbfEthernetObjSnmpReadWriteString_Get }

/* Reset from HICP enabled */
#define ABCC_ETHERNET_OBJ_DHCP_CLIENT_ENABLED_GET_VALUE(x)  { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_DHCP_CLIENT, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,  .uData.fBool8              = (x) }
#define ABCC_ETHERNET_OBJ_DHCP_CLIENT_ENABLED_GET_CBFUNC    { .bObject = ABP_OBJ_NUM_ETN, .bInstance = 0x01, .uCmdExt.bAttr = ABP_ETN_IA_ENABLE_DHCP_CLIENT, .bCommand = ABP_CMD_GET_ATTR, .eServiceTag = SERVICE_BOOL8,                                                                .uCbx.pnGetBool8Attr = ABCC_CbfEthernetObjDhcpClientEnabled_Get }

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
**       True:  Enumeration value is in range (supported).
**       False: Enumeration value is out of range (not supported).
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
** CANopen Object (0xFB)
**------------------------------------------------------------------------------
*/
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
UINT32 ABCC_CbfCANopenObjSerialNmber_Get( void );

/*------------------------------------------------------------------------------
** EtherCAT Object (0xF5)
**------------------------------------------------------------------------------
*/
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
UINT32 ABCC_CbfEtherCATObjSerialNmber_Get( void );

/*------------------------------------------------------------------------------
** Callback function to state wether the file over EtherCAT (FoE) is enabled or
** not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  FoE is enabled.
**       False: FoE is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEtherCATObjEnableFoE_Get( void );

/*------------------------------------------------------------------------------
** Callback function to state wether the Ethernet over EtherCAT (FoE) is enabled
** or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  EoE is enabled.
**       False: EoE is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEtherCATObjEnableEoE_Get( void );

/*------------------------------------------------------------------------------
** Callback function to state wether to use the device ID as configured alias.
** The alias is what appears in EtherCAT configuration tools, etc.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Device ID is used as alias.
**       False: Device ID is not used as alias.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEtherCATObjDeviceIDAsAlias_Get( void );

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
** Callback function to provide the device's serial number to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       Serial number of the device.
**------------------------------------------------------------------------------
*/
UINT32 ABCC_CbfEthernetIpObjSerialNumber_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide wheter the communication settings shall be
** settable from the network or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Communication settings shall be settable from the network.
**       False: Communication settings shall not be settable from the network.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetIpObjCommSettingsFromNet_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide wheter requests to unknown CIP objects shall be
** forwarded to the application or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Requests to unknown CIP objects will be forwarded to the
**              application.
**       False: Requests to unknown CIP objects will not be forwarded to the
**              application.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetIpObjCipForwarding_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide wheter CompactCom will accept Class1 connection
** requests that have sizes less than or equal to the configured IO sizes.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Strict IO match is disabled.
**       False: Strict IO match is enabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetIpObjDisableStrictIoMatch_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter CompactCom will support EtherNet/IP
** QuickConnect or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  EtherNet/IP QuickConnect is enabled.
**       False: EtherNet/IP QuickConnect is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetIpObjEnableQuickConnect_Get( void );

/*------------------------------------------------------------------------------
** Callback function to set the CIP object number where the network data
** parameters (ADIs) shall be accessible.
** Valid object numbers are within the vendor specific ranges (0064h - 00C7h and
** 0300h - 04FFh). Any other value will disable the ADI object.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       CIP object number where the network data parameters (ADIs) shall be
**       accessible.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfEthernetIpObjAdiObjectNumber_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter CompactCom will support EtherNet/IP
** Device Level Ring (DLR) redundancy protocol or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  EtherNet/IP DLR is enabled.
**       False: EtherNet/IP DLR is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetIpObjEnableDLR_Get( void );

/*------------------------------------------------------------------------------
** PROFINET IO Object (0xF6)
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** Callback function to retrieve the PROFINET Device ID to the CompactCom.
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
** Callback function to retrieve the PROFINET Vendor ID to the CompactCom.
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
** Callback function to retrieve the PROFINET Station Type to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       PROFINET Station Type.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjStationType_Get( char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to retrieve the maximum number of simultaneous application
** relationships (ARs) that the device supports.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       Maximum number of simultaneous application relationships (ARs).
**          Valid range: 1-4
**          Note: With S2 redundancy enabled, the minimum number of ARs is 2.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjMaxAr_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve the PROFINET Order ID to the CompactCom.
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
** Callback function to retrieve the PROFINET Serial Number to the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       PROFINET Serial Number of the device.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjSerialNumber_Get( char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to retrieve the PROFINET hardware revision.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       PROFINET hardware revision.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjHwRevision_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve the PROFINET revision counter.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       PROFINET revision counter.
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfProfinetIoObjRevisionCounter_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter CompactCom will support PROFINET S2
** redundancy protocol or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  PROFINET S2 redundancy is enabled.
**       False: PROFINET S2 redundancy is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfProfinetIoObjS2Enabled_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the primary AR handle for the PROFINET S2
** redundancy protocol.
**------------------------------------------------------------------------------
** Arguments:
**       iPrimaryAR - Primary AR handle.
**                       0 indicates "no S2 connection".
**
** Returns:
**       None.
**------------------------------------------------------------------------------
*/
void ABCC_CbfProfinetIoObjS2PrimaryArHandle_Set( UINT16 iPrimaryAR );

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

/*------------------------------------------------------------------------------
** Ethernet Host Object (0xF9)
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** Callback function to retrieve wheter CompactCom will support HICP or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True: HICP is enabled.
**       False: HICP is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjHicpEnabled_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter the web server in the CompactCom is
** enabled or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Web server is enabled.
**       False: Web server is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjWebEnabled_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter the network data parameters (ADI) shall
** be accessible through the web server in the CompactCom or not.
** enabled or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Web server ADI access is enabled.
**       False: Web server ADI access is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjWebAdiEnabled_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter the FTP server in the CompactCom is
** enabled or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  FTP server is enabled.
**       False: FTP server is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjFtpEnabled_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter the Admin mode in the CompactCom is
** enabled or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Admin mode is enabled.
**       False: Admin mode is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjAdminMode_Get( void );

/*------------------------------------------------------------------------------
** Callback function to set the network status bitfield.
**------------------------------------------------------------------------------
** Arguments:
**       iNwStatus - Netork status bitfield.
**                   Bit 0 (Link) - 0: Link down, 1: Link up
**                   Bit 1 (IP established) - 0: IP address not established, 
**                                            1: IP address established
**                   Bit 2 (reserved)
**                   Bit 3 (Link port 1) - 0: Link down, 1: Link up
**                   Bit 4 (Link port 2) - 0: Link down, 1: Link up
**                   Bit 5-15 (reserved)
**
** Returns:
**       None.
**------------------------------------------------------------------------------
*/
void ABCC_CbfEthernetObjNetworkStatus_Set( UINT16 iNwStatus );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter Address Conflict Detection (ACD) is
** enabled or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  ACD is enabled.
**       False: ACD is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjAcdEnabled_Get( void );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter reset via HICP shall be possible or not.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  Reset via HICP is enabled.
**       False: Reset via HICP is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjHicpResetEnabled_Get( void );

/*------------------------------------------------------------------------------
** Callback function to provide the current IP address configuration to the
** host application.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedArrDest - Pointer to buffer where a packed UINT32 array of
**                        length 3 (size 12 in bytes) shall be written.
**                           Element 1 (IP address)
**                           Element 2 (Subnet mask)
**                           Element 3 (Default gateway)
**       iBuffSizeBytes - Size of the buffer in bytes, 12 in this case.
**
** Returns:
**       None.
**------------------------------------------------------------------------------
*/
void ABCC_CbfEthernetObjIpConfiguration_Set( void* pPackedArrSrc, UINT16 iSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to retrieve the SNMP read-only community string.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       SNMP read-only community string
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfEthernetObjSnmpReadOnlyString_Get( char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to retrieve the SNMP read-write community string.
**------------------------------------------------------------------------------
** Arguments:
**       pPackedStrDest - Pointer to buffer where string shall be written.
**       iBuffSizeBytes - Size of the buffer in bytes.
**
** Returns:
**       SNMP read-write community string
**------------------------------------------------------------------------------
*/
UINT16 ABCC_CbfEthernetObjSnmpReadWriteString_Get( char* pPackedStrDest, UINT16 iBuffSizeBytes );

/*------------------------------------------------------------------------------
** Callback function to retrieve wheter DHCP client is enabled or not.
** Note: only available on PROFINET and EtherNet/IP.
**------------------------------------------------------------------------------
** Arguments:
**       None.
**
** Returns:
**       True:  DHCP client is enabled.
**       False: DHCP client is disabled.
**------------------------------------------------------------------------------
*/
BOOL8 ABCC_CbfEthernetObjDhcpClientEnabled_Get( void );

#endif
