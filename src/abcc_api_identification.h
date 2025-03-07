/*******************************************************************************
** Copyright 2019-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** User configuration to set the identification parameters of an ABCC module
**
** All configurations in this file should be possible to override if defined
** in abcc_driver_config.h. This allows the target platform to adapt the
** configuration to realize its implementation for specific needs.
********************************************************************************
*/

#ifndef ABCC_IDENTIFICATION_H_
#define ABCC_IDENTIFICATION_H_

#include "abcc_driver_config.h"
#include "abcc_config.h"

#ifndef ABCC_OBJ_CFG_H_
#error "Do not include abcc_api_object_config.h, include abcc_api_config.h instead."
#endif

/*------------------------------------------------------------------------------
** CC-Link IE TSN Object (0xE1)
**------------------------------------------------------------------------------
*/
#if CIET_OBJ_ENABLE
/*
** Attribute 1: Vendor Code (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CIET_IA_VENDOR_CODE_ENABLE
#define CIET_IA_VENDOR_CODE_ENABLE                 1
#define CIET_IA_VENDOR_CODE_VALUE                  0xFFFF
#endif

/*
** Attribute 2: Vendor Name (Array of CHAR)
*/
#ifndef CIET_IA_VENDOR_NAME_ENABLE
#define CIET_IA_VENDOR_NAME_ENABLE                 1
#define CIET_IA_VENDOR_NAME_VALUE                  "Vendor Name"
#endif

/*
** Attribute 3: Model Code (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef CIET_IA_MODEL_CODE_ENABLE
#define CIET_IA_MODEL_CODE_ENABLE                  1
#define CIET_IA_MODEL_CODE_VALUE                   0x0000007F
#endif

/*
** Attribute 4: Expansion Model Code (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CIET_IA_EXPANSION_MODEL_CODE_ENABLE
#define CIET_IA_EXPANSION_MODEL_CODE_ENABLE        1
#define CIET_IA_EXPANSION_MODEL_CODE_VALUE         0x0000
#endif

/*
** Attribute 5: Model Name (Array of CHAR)
*/
#ifndef CIET_IA_MODEL_NAME_ENABLE
#define CIET_IA_MODEL_NAME_ENABLE                  1
#define CIET_IA_MODEL_NAME_VALUE                   "Model Name"
#endif

/*
** Attribute 6: Device Type (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CIET_IA_DEVICE_TYPE_ENABLE
#define CIET_IA_DEVICE_TYPE_ENABLE                 1
#define CIET_IA_DEVICE_TYPE_VALUE                  0x0001
#endif

/*
** Attribute 7: Device Version (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CIET_IA_DEVICE_VERSION_ENABLE
#define CIET_IA_DEVICE_VERSION_ENABLE              1
#define CIET_IA_DEVICE_VERSION_VALUE               0x0001
#endif

/*
** Attribute 8: Firmware Version (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CIET_IA_FW_VERSION_ENABLE
#define CIET_IA_FW_VERSION_ENABLE                  1
#define CIET_IA_FW_VERSION_VALUE                   0x0001
#endif

/*
** Attribute 9: Hardware Version (UINT8 - 0x00-0xFF)
*/
#ifndef CIET_IA_HW_VERSION_ENABLE
#define CIET_IA_HW_VERSION_ENABLE                  1
#define CIET_IA_HW_VERSION_VALUE                   0x01
#endif

/*
** Attribute 10: Serial Number (Array of CHAR)
*/
#ifndef CIET_IA_SERIAL_NUMBER_ENABLE
#define CIET_IA_SERIAL_NUMBER_ENABLE               1
#define CIET_IA_SERIAL_NUMBER_VALUE                "1234567890"
#endif

#endif /* CIET_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** CC-Link IE Field Object (0xE6)
**------------------------------------------------------------------------------
*/
#if CFN_OBJ_ENABLE
/*
** Attribute 1: Vendor Code (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CFN_IA_VENDOR_CODE_ENABLE
   #define CFN_IA_VENDOR_CODE_ENABLE                1
   #define CFN_IA_VENDOR_CODE_VALUE                 0xFFFF
#endif

/*
** Attribute 2: Vendor Name (Array of CHAR)
*/
#ifndef CFN_IA_VENDOR_NAME_ENABLE
   #define CFN_IA_VENDOR_NAME_ENABLE                1
   #define CFN_IA_VENDOR_NAME_VALUE                 "Vendor Name"
#endif

/*
** Attribute 3: Model Type / Device Type (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CFN_IA_MODEL_TYPE_ENABLE
   #define CFN_IA_MODEL_TYPE_ENABLE                1
   #define CFN_IA_MODEL_TYPE_VALUE                 0x007F
#endif

/*
** Attribute 4: Model Name (Array of CHAR)
*/
#ifndef CFN_IA_MODEL_NAME_ENABLE
   #define CFN_IA_MODEL_NAME_ENABLE                1
   #define CFN_IA_MODEL_NAME_VALUE                 "Model Name"
#endif

/*
** Attribute 5: Unit Model Code (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef CFN_IA_MODEL_CODE_ENABLE
   #define CFN_IA_MODEL_CODE_ENABLE                1
   #define CFN_IA_MODEL_CODE_VALUE                 0xFFFFFFFF
#endif

/*
** Attribute 6: Version (UINT8 - 0x00-0xFF)
*/
#ifndef CFN_IA_SW_VERSION_ENABLE
   #define CFN_IA_SW_VERSION_ENABLE                1
   #define CFN_IA_SW_VERSION_VALUE                 0x01
#endif

#endif /* CFN_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Ethernet POWERLINK Object (0xE9)
**------------------------------------------------------------------------------
*/
#if EPL_OBJ_ENABLE
/*
** Attribute 1: Vendor ID (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef EPL_IA_VENDOR_ID_ENABLE
   #define EPL_IA_VENDOR_ID_ENABLE                 1
   #define EPL_IA_VENDOR_ID_VALUE                  0xFFFFFFFF
#endif

/*
** Attribute 2: Product Code type (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef EPL_IA_PRODUCT_CODE_ENABLE
   #define EPL_IA_PRODUCT_CODE_ENABLE              1
   #define EPL_IA_PRODUCT_CODE_VALUE               0xFFFFFFFF
#endif

/*
** Attribute 3: Revision High Word (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EPL_IA_REVISION_HW_ENABLE
   #define EPL_IA_REVISION_HW_ENABLE               1
   #define EPL_IA_REVISION_HW_VALUE                0xFFFF
#endif

/*
** Attribute 4: Revision Low Word (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EPL_IA_REVISION_LW_ENABLE
   #define EPL_IA_REVISION_LW_ENABLE               1
   #define EPL_IA_REVISION_LW_VALUE                0xFFFF
#endif

/*
** Attribute 5: Serial number (UINT32 - function/variable/0x00000000-0xFFFFFFFF)
*/
#ifndef EPL_IA_SERIAL_NUMBER_ENABLE
   #define EPL_IA_SERIAL_NUMBER_ENABLE             1
   #define EPL_IA_SERIAL_NUMBER_VALUE              0x12345678
#endif

/*
** Attribute 6: Manufacturer device name (Array of CHAR)
*/
#ifndef EPL_IA_MANF_DEVICE_NAME_ENABLE
   #define EPL_IA_MANF_DEVICE_NAME_ENABLE          1
   #define EPL_IA_MANF_DEVICE_NAME_VALUE           "Manufacturer device name"
#endif

/*
** Attribute 7: Manufacturer hardware version (Array of CHAR)
*/
#ifndef EPL_IA_MANF_HW_VERSION_ENABLE
   #define EPL_IA_MANF_HW_VERSION_ENABLE           1
   #define EPL_IA_MANF_HW_VERSION_VALUE            "01"
#endif

/*
** Attribute 8: Manufacturer software version (Array of CHAR)
*/
#ifndef EPL_IA_MANF_SW_VERSION_ENABLE
   #define EPL_IA_MANF_SW_VERSION_ENABLE           1
   #define EPL_IA_MANF_SW_VERSION_VALUE            "01"
#endif

/*
** Attribute 10: Device type (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef EPL_IA_DEVICE_TYPE_ENABLE
   #define EPL_IA_DEVICE_TYPE_ENABLE               1
   #define EPL_IA_DEVICE_TYPE_VALUE                0xFFFFFFFF
#endif

/*
** Attribute 14: Manufacturer name (Array of CHAR)
*/
#ifndef EPL_IA_MANF_NAME_ENABLE
   #define EPL_IA_MANF_NAME_ENABLE                 1
   #define EPL_IA_MANF_NAME_VALUE                  "Manufacturer name"
#endif

#endif /* #if EPL_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** BACnet Host Object (0xEF)
**------------------------------------------------------------------------------
*/
#if BAC_OBJ_ENABLE
/*
** Attribute 1: Object Name (Array of Char)
**
** Note that platform-specific adaptations are needed for SET support to work
** as intended. See the comments in "bacnet_object.c" for more information.
*/
#ifndef BAC_IA_OBJECT_NAME_ENABLE
   #define BAC_IA_OBJECT_NAME_ENABLE               0
   #define BAC_IA_OBJECT_NAME_VALUE                "Example Object Name"
   #define BAC_IA_OBJECT_NAME_SET                  FALSE
#endif

/*
** Attribute 2: Vendor Name (Array of Char)
*/
#ifndef BAC_IA_VENDOR_NAME_ENABLE
   #define BAC_IA_VENDOR_NAME_ENABLE               0
   #define BAC_IA_VENDOR_NAME_VALUE                "Example Vendor Name"
#endif

/*
** Attribute 3: Vendor Identifier (UINT16)
*/
#ifndef BAC_IA_VENDOR_IDENTIFIER_ENABLE
   #define BAC_IA_VENDOR_IDENTIFIER_ENABLE         0
   #define BAC_IA_VENDOR_IDENTIFIER_VALUE          0xFFFF
#endif

/*
** Attribute 4: Model Name (Array of Char)
*/
#ifndef BAC_IA_MODEL_NAME_ENABLE
   #define BAC_IA_MODEL_NAME_ENABLE                0
   #define BAC_IA_MODEL_NAME_VALUE                 "Example Model Name"
#endif

/*
** Attribute 5: Firmware Revision (Array of Char)
*/
#ifndef BAC_IA_FIRMWARE_REVISION_ENABLE
   #define BAC_IA_FIRMWARE_REVISION_ENABLE         0
   #define BAC_IA_FIRMWARE_REVISION_VALUE          "99.99.99"
#endif

/*
** Attribute 6: Application Software Version (Array of Char)
*/
#ifndef BAC_IA_APP_SOFTWARE_VERSION_ENABLE
   #define BAC_IA_APP_SOFTWARE_VERSION_ENABLE      0
   #define BAC_IA_APP_SOFTWARE_VERSION_VALUE       "99.99.99"
#endif

#endif  /* BAC_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** EtherCAT Object (0xF5)
**------------------------------------------------------------------------------
*/
#if ECT_OBJ_ENABLE
/*
** Attribute 1: Vendor ID (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef ECT_IA_VENDOR_ID_ENABLE
   #define ECT_IA_VENDOR_ID_ENABLE                 1
   #define ECT_IA_VENDOR_ID_VALUE                  0xFFFFFFFF
#endif

/*
** Attribute 2: Product Code type (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef ECT_IA_PRODUCT_CODE_ENABLE
   #define ECT_IA_PRODUCT_CODE_ENABLE              1
   #define ECT_IA_PRODUCT_CODE_VALUE               0xFFFFFFFF
#endif

/*
** Attribute 3: Revision High Word (UINT16 - 0x0000-0xFFFF)
*/
#ifndef ECT_IA_MAJOR_REV_ENABLE
   #define ECT_IA_MAJOR_REV_ENABLE                 1
   #define ECT_IA_REVISION_HW_VALUE                0xFFFF
#endif

/*
** Attribute 4: Revision Low Word (UINT16 - 0x0000-0xFFFF)
*/
#ifndef ECT_IA_MINOR_REV_ENABLE
   #define ECT_IA_MINOR_REV_ENABLE                 1
   #define ECT_IA_REVISION_LW_VALUE                0xFFFF
#endif

/*
** Attribute 5: Serial number (UINT32 - function/variable/0x00000000-0xFFFFFFFF)
*/
#ifndef ECT_IA_SERIAL_NUMBER_ENABLE
   #define ECT_IA_SERIAL_NUMBER_ENABLE             1
   #define ECT_IA_SERIAL_NUMBER_VALUE              0x12345678
#endif

/*
** Attribute 6: Manufacturer device name (Array of CHAR)
*/
#ifndef ECT_IA_MANF_DEVICE_NAME_ENABLE
   #define ECT_IA_MANF_DEVICE_NAME_ENABLE          1
   #define ECT_IA_MANF_DEVICE_NAME_VALUE           "Manufacturer device name"
#endif

/*
** Attribute 7: Manufacturer hardware version (Array of CHAR)
*/
#ifndef ECT_IA_MANF_HW_VERSION_ENABLE
   #define ECT_IA_MANF_HW_VERSION_ENABLE           1
   #define ECT_IA_MANF_HW_VERSION_VALUE            "FF"
#endif

/*
** Attribute 8: Manufacturer software version (Array of CHAR)
*/
#ifndef ECT_IA_MANF_SW_VERSION_ENABLE
   #define ECT_IA_MANF_SW_VERSION_ENABLE           1
   #define ECT_IA_MANF_SW_VERSION_VALUE            "FF"
#endif

/*
** Attribute 10: Device type (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef ECT_IA_DEVICE_TYPE_ENABLE
   #define ECT_IA_DEVICE_TYPE_ENABLE               1
   #define ECT_IA_DEVICE_TYPE_VALUE                0xFFFFFFFF
#endif

/*
** Attribute 25: SII Order Number String (Array of CHAR)
*/
#ifndef ECT_IA_SII_ORDER_NUM_ENABLE
   #define ECT_IA_SII_ORDER_NUM_ENABLE             0
   #define ECT_IA_SII_ORDER_NUM_VALUE              "SII Order Number"
#endif

/*
** Attribute 26: SII Device Name String (Array of CHAR)
*/
#ifndef ECT_IA_SII_DEV_NAME_ENABLE
   #define ECT_IA_SII_DEV_NAME_ENABLE              0
   #define ECT_IA_SII_DEV_NAME_VALUE               "SII Device Name"
#endif

#endif /* #if ECT_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** PROFINET IO Object (0xF6)
**------------------------------------------------------------------------------
*/
#if PRT_OBJ_ENABLE
/*
** Attribute 1: Device ID (UINT16 - 0x0000-0xFFFF)
*/
#ifndef PRT_IA_DEVICE_ID_ENABLE
   #define PRT_IA_DEVICE_ID_ENABLE                 1
   #define PRT_IA_DEVICE_ID_VALUE                  0xFFFF
#endif

/*
** Attribute 2: Vendor ID (UINT16 - 0x0000-0xFFFF)
*/
#ifndef PRT_IA_VENDOR_ID_ENABLE
   #define PRT_IA_VENDOR_ID_ENABLE                 1
   #define PRT_IA_VENDOR_ID_VALUE                  0xFFFF
#endif

/*
** Attribute 3: Station Type (Array of CHAR)
*/
#ifndef PRT_IA_STATION_TYPE_ENABLE
   #define PRT_IA_STATION_TYPE_ENABLE              1
   #define PRT_IA_STATION_TYPE_VALUE               "Station Type"
#endif

/*
** Attribute 8: IM Order ID (Array of CHAR)
*/
#ifndef PRT_IA_IM_ORDER_ID_ENABLE
   #define PRT_IA_IM_ORDER_ID_ENABLE               1
   #define PRT_IA_IM_ORDER_ID_VALUE                "Order ID"
#endif

/*
** Attribute 9: IM Serial Number (Array of CHAR)
*/
#ifndef PRT_IA_IM_SERIAL_NBR_ENABLE
   #define PRT_IA_IM_SERIAL_NBR_ENABLE             1
   #define PRT_IA_IM_SERIAL_NBR_VALUE              "0123456789ABCDEF"
#endif

/*
** Attribute 10: IM Hardware revision (UINT16 - 0x0000-0xFFFF)
*/
#ifndef PRT_IA_IM_HW_REV_ENABLE
   #define PRT_IA_IM_HW_REV_ENABLE                 1
   #define PRT_IA_IM_HW_REV_VALUE                  0x0000
#endif

/*
** Attribute 11: IM Software revision (CHAR, 3 * UINT8)
*/
#ifndef PRT_IA_IM_SW_REV_ENABLE
   #define PRT_IA_IM_SW_REV_ENABLE                 1
   #define PRT_IA_IM_SW_REV_SYMBOL_VALUE           'V'
   #define PRT_IA_IM_SW_REV_MAJOR_VALUE            99
   #define PRT_IA_IM_SW_REV_MINOR_VALUE            99
   #define PRT_IA_IM_SW_REV_BUILD_VALUE            99
#endif

/*
** Attribute 19: System Description (Array of CHAR)
*/
#ifndef PRT_IA_SYSTEM_DESCRIPTION_ENABLE
   #define PRT_IA_SYSTEM_DESCRIPTION_ENABLE        1
   #define PRT_IA_SYSTEM_DESCRIPTION_VALUE         "System Description"
#endif

/*
** Attribute 22: System Contact (Array of CHAR)
*/
#ifndef PRT_IA_SYSTEM_CONTACT_ENABLE
   #define PRT_IA_SYSTEM_CONTACT_ENABLE            1
   #define PRT_IA_SYSTEM_CONTACT_VALUE             "System Contact"
#endif

#endif /* #if PRT_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** CC-Link Object (0xF7)
**------------------------------------------------------------------------------
*/
#if CCL_OBJ_ENABLE
/*
** Attribute 1: Vendor Code (UINT16 - 0x0000-0xFFFF)
*/
#ifndef CCL_IA_VENDOR_CODE_ENABLE
   #define CCL_IA_VENDOR_CODE_ENABLE                1
   #define CCL_IA_VENDOR_CODE_VALUE                 0xFFFF
#endif

/*
** Attribute 2: SW Version (UINT8 - 0x01 to 0x3F)
*/
#ifndef CCL_IA_SOFTWARE_VERSION_ENABLE
   #define CCL_IA_SOFTWARE_VERSION_ENABLE           1
   #define CCL_IA_SOFTWARE_VERSION_VALUE            0x01
#endif

/*
** Attribute 3: Model Code (UINT8 - 0x00-0xFF)
*/
#ifndef CCL_IA_MODEL_CODE_ENABLE
   #define CCL_IA_MODEL_CODE_ENABLE                1
   #define CCL_IA_MODEL_CODE_VALUE                 0x7f
#endif

#endif /* CCL_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** EtherNet/IP Object (0xF8)
**------------------------------------------------------------------------------
*/
#if EIP_OBJ_ENABLE
/*
** Attribute 1: Vendor ID (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_VENDOR_ID_ENABLE
   #define EIP_IA_VENDOR_ID_ENABLE                 1
   #define EIP_IA_VENDOR_ID_VALUE                  0xFFFF
#endif

/*
** Attribute 2: Device type (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_DEVICE_TYPE_ENABLE
   #define EIP_IA_DEVICE_TYPE_ENABLE               1
   #define EIP_IA_DEVICE_TYPE_VALUE                0xFFFF
#endif

/*
** Attribute 3: Product code (UINT16 - 0x0000-0xFFFF)
*/
#ifndef EIP_IA_PRODUCT_CODE_ENABLE
   #define EIP_IA_PRODUCT_CODE_ENABLE              1
   #define EIP_IA_PRODUCT_CODE_VALUE               0xFFFF
#endif

/*
** Attribute 4: Revision (Array of UINT8 - {0x01-0xFF})
*/
#ifndef EIP_IA_REVISION_ENABLE
   #define EIP_IA_REVISION_ENABLE                  1
   #define EIP_IA_REVISION_MAJOR_VALUE             99
   #define EIP_IA_REVISION_MINOR_VALUE             99
#endif

/*
** Attribute 5: Serial number (UINT32 - function/variable/0x00000000-0xFFFFFFFF)
*/
#ifndef EIP_IA_SERIAL_NUMBER_ENABLE
   #define EIP_IA_SERIAL_NUMBER_ENABLE             1
   #define EIP_IA_SERIAL_NUMBER_VALUE              0x12345678
#endif

/*
** Attribute 6: Product name (Array of CHAR)
*/
#ifndef EIP_IA_PRODUCT_NAME_ENABLE
   #define EIP_IA_PRODUCT_NAME_ENABLE              1
   #define EIP_IA_PRODUCT_NAME_VALUE               "Product Name"
#endif

#endif /* #if EIP_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Ethernet Object (0xF9)
**------------------------------------------------------------------------------
*/
#if ETN_OBJ_ENABLE
/*
** PORTING ALERT!
**
** MAC address values *must* be unique for each produced unit!
**
** The hard-coded values listed here are only inteded for evaluation and basic
** pre-release testing in an isolated network, but hard-coded values must NOT
** be used in a finished implementation! If customised MAC address values
** should be used it us up to the implementor to provide unique values to each
** built unit.
**
** The PORT1 and PORT2 MAC address attributes are presently only required by
** PROFINET, and should only be enabled if a custom 'base' MAC address also has
** been enabled. They should be in incremental order from the 'base' MAC
** address.
*/

/*
** Attribute 1: MAC Address (Array[6] of UINT8)
*/
#ifndef ETN_IA_MAC_ADDRESS_ENABLE
   #define ETN_IA_MAC_ADDRESS_ENABLE               0
   #define ETN_IA_MAC_ADDRESS_VALUE                "\x02\x58\x4D\x50\x4C\x30" /* "XMPL0" */
#endif

/*
** Attribute 9: Port 1 MAC Address (Array[6] of UINT8)
*/
#ifndef ETN_IA_PORT1_MAC_ADDRESS_ENABLE
   #define ETN_IA_PORT1_MAC_ADDRESS_ENABLE         0
   #define ETN_IA_PORT1_MAC_ADDRESS_VALUE          "\x02\x58\x4D\x50\x4C\x31" /* "XMPL1" */
#endif

/*
** Attribute 10: Port 2 MAC Address (Array[6] of UINT8)
*/
#ifndef ETN_IA_PORT2_MAC_ADDRESS_ENABLE
   #define ETN_IA_PORT2_MAC_ADDRESS_ENABLE         0
   #define ETN_IA_PORT2_MAC_ADDRESS_VALUE          "\x02\x58\x4D\x50\x4C\x32" /* "XMPL2" */
#endif

#endif /* #if ETN_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Modbus Object (0xFA)
**------------------------------------------------------------------------------
*/
#if MOD_OBJ_ENABLE
/*
** Attribute 1: Vendor Name (Array of CHAR)
*/
#ifndef MOD_IA_VENDOR_NAME_ENABLE
   #define MOD_IA_VENDOR_NAME_ENABLE               1
   #define MOD_IA_VENDOR_NAME_VALUE                "Vendor Name"
#endif

/*
** Attribute 2: Product Code (Array of CHAR)
*/
#ifndef MOD_IA_PRODUCT_CODE_ENABLE
   #define MOD_IA_PRODUCT_CODE_ENABLE              1
   #define MOD_IA_PRODUCT_CODE_VALUE               "Product Code"
#endif

/*
** Attribute 3: Revision (Array of CHAR)
*/
#ifndef MOD_IA_REVISION_ENABLE
   #define MOD_IA_REVISION_ENABLE                  1
   #define MOD_IA_REVISION_VALUE                   "99.99"
#endif

/*
** Attribute 4: Vendor URL (Array of CHAR)
*/
#ifndef MOD_IA_VENDOR_URL_ENABLE
   #define MOD_IA_VENDOR_URL_ENABLE                1
   #define MOD_IA_VENDOR_URL_VALUE                 "www.vendor.url"
#endif

/*
** Attribute 5: Product Name (Array of CHAR)
*/
#ifndef MOD_IA_PRODUCT_NAME_ENABLE
   #define MOD_IA_PRODUCT_NAME_ENABLE              1
   #define MOD_IA_PRODUCT_NAME_VALUE               "Product Name"
#endif

/*
** Attribute 6: Model Name (Array of CHAR)
*/
#ifndef MOD_IA_MODEL_NAME_ENABLE
   #define MOD_IA_MODEL_NAME_ENABLE                1
   #define MOD_IA_MODEL_NAME_VALUE                 "Model Name"
#endif

/*
** Attribute 7: User Application Name (Array of CHAR)
*/
#ifndef MOD_IA_USER_APP_NAME_ENABLE
   #define MOD_IA_USER_APP_NAME_ENABLE             1
   #define MOD_IA_USER_APP_NAME_VALUE              "User Application Name"
#endif

/*
** Attribute 8: Device ID (Array of UINT8)
*/
#ifndef MOD_IA_DEVICE_ID_ENABLE
   #define MOD_IA_DEVICE_ID_ENABLE                 1
   #define MOD_IA_DEVICE_ID_VALUE                  "\x00\x01\x02\x03"
   #define MOD_IA_DEVICE_ID_ARRAY_SIZE             4
#endif

#endif /* #if MOD_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** CANopen Object (0xFB)
**------------------------------------------------------------------------------
*/
#if COP_OBJ_ENABLE
/*
** Attribute 1: Vendor ID (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef COP_IA_VENDOR_ID_ENABLE
   #define COP_IA_VENDOR_ID_ENABLE                 1
   #define COP_IA_VENDOR_ID_VALUE                  0xFFFFFFFF
#endif

/*
** Attribute 2: Product Code (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef COP_IA_PRODUCT_CODE_ENABLE
   #define COP_IA_PRODUCT_CODE_ENABLE              1
   #define COP_IA_PRODUCT_CODE_VALUE               0xFFFFFFFF
#endif

/*
** Attribute 3: Major Revision (UINT16 - 0x0000-0xFFFF)
*/
#ifndef COP_IA_REV_MAJOR_ENABLE
   #define COP_IA_REV_MAJOR_ENABLE                 1
   #define COP_IA_REV_MAJOR_VALUE                  0xFFFF
#endif

/*
** Attribute 4: Minor Revision (UINT16 - 0x0000-0xFFFF)
*/
#ifndef COP_IA_REV_MINOR_ENABLE
   #define COP_IA_REV_MINOR_ENABLE                 1
   #define COP_IA_REV_MINOR_VALUE                  0xFFFF
#endif

/*
** Attribute 5: Serial Number (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef COP_IA_SERIAL_NUMBER_ENABLE
   #define COP_IA_SERIAL_NUMBER_ENABLE             1
   #define COP_IA_SERIAL_NUMBER_VALUE              0xFFFFFFFF
#endif

/*
** Attribute 6: Manufacturer Device Name (Array of CHAR)
*/
#ifndef COP_IA_MANF_DEV_NAME_ENABLE
   #define COP_IA_MANF_DEV_NAME_ENABLE             1
   #define COP_IA_MANF_DEV_NAME_VALUE              "Device Name"
#endif

/*
** Attribute 7: Manufacturer Hardware Version (Array of CHAR)
*/
#ifndef COP_IA_MANF_HW_VER_ENABLE
   #define COP_IA_MANF_HW_VER_ENABLE               1
   #define COP_IA_MANF_HW_VER_VALUE                "Hardware Version"
#endif

/*
** Attribute 8: Manufacturer Software Version (Array of CHAR)
*/
#ifndef COP_IA_MANF_SW_VER_ENABLE
   #define COP_IA_MANF_SW_VER_ENABLE               1
   #define COP_IA_MANF_SW_VER_VALUE                "Software Version"
#endif

/*
** Attribute 10: Device Type (UINT32 - 0x00000000-0xFFFFFFFF)
*/
#ifndef COP_IA_DEVICE_TYPE_ENABLE
   #define COP_IA_DEVICE_TYPE_ENABLE               1
   #define COP_IA_DEVICE_TYPE_VALUE                0xFFFFFFFF
#endif

#endif /* #if COP_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** DeviceNet Object (0xFC)
**------------------------------------------------------------------------------
*/
#if DEV_OBJ_ENABLE
/*
** Attribute 1: Vendor ID (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DEV_IA_VENDOR_ID_ENABLE
   #define DEV_IA_VENDOR_ID_ENABLE                 1
   #define DEV_IA_VENDOR_ID_VALUE                  0xFFFF
#endif

/*
** Attribute 2: Device type (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DEV_IA_DEVICE_TYPE_ENABLE
   #define DEV_IA_DEVICE_TYPE_ENABLE               1
   #define DEV_IA_DEVICE_TYPE_VALUE                0xFFFF
#endif

/*
** Attribute 3: Product code (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DEV_IA_PRODUCT_CODE_ENABLE
   #define DEV_IA_PRODUCT_CODE_ENABLE              1
   #define DEV_IA_PRODUCT_CODE_VALUE               0xFFFF
#endif

/*
** Attribute 4: Revision (Array of UINT8 - {0x01-0xFF})
*/
#ifndef DEV_IA_REVISION_ENABLE
   #define DEV_IA_REVISION_ENABLE                  1
   #define DEV_IA_REVISION_MAJOR_VALUE             99
   #define DEV_IA_REVISION_MINOR_VALUE             99
#endif

/*
** Attribute 5: Serial number (UINT32 - function/variable/0x00000000-0xFFFFFFFF)
*/
#ifndef DEV_IA_SERIAL_NUMBER_ENABLE
   #define DEV_IA_SERIAL_NUMBER_ENABLE             1
   #define DEV_IA_SERIAL_NUMBER_VALUE              0x12345678
#endif

/*
** Attribute 6: Product name (Array of CHAR)
*/
#ifndef DEV_IA_PRODUCT_NAME_ENABLE
   #define DEV_IA_PRODUCT_NAME_ENABLE              1
   #define DEV_IA_PRODUCT_NAME_VALUE               "Product Name"
#endif

#endif /* #if DEV_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** PROFIBUS DP-V1 Object (0xFD)
**------------------------------------------------------------------------------
*/
#if DPV1_OBJ_ENABLE
/*
** Attribute 1: PNO Ident number (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DPV1_IA_IDENT_NUMBER_ENABLE
   #define DPV1_IA_IDENT_NUMBER_ENABLE             1
   #define DPV1_IA_IDENT_NUMBER_VALUE              0xFFFF
#endif

/*
** Attribute 8: Manufacturer Ident number (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DPV1_IA_MANUFACTURER_ID_ENABLE
   #define DPV1_IA_MANUFACTURER_ID_ENABLE          1
   #define DPV1_IA_MANUFACTURER_ID_VALUE           0xFFFF
#endif

/*
** Attribute 9: IM Order ID (Array of CHAR)
*/
#ifndef DPV1_IA_ORDER_ID_ENABLE
   #define DPV1_IA_ORDER_ID_ENABLE                 1
   #define DPV1_IA_ORDER_ID_VALUE                  "Order ID"
#endif

/*
** Attribute 10: IM Serial Number (Array of CHAR)
*/
#ifndef DPV1_IA_SERIAL_NO_ENABLE
   #define DPV1_IA_SERIAL_NO_ENABLE                1
   #define DPV1_IA_SERIAL_NO_VALUE                 "0123456789ABCDEF"
#endif

/*
** Attribute 11: IM Hardware revision (UINT16 - 0x0000-0xFFFF)
*/
#ifndef DPV1_IA_HW_REV_ENABLE
   #define DPV1_IA_HW_REV_ENABLE                   1
   #define DPV1_IA_HW_REV_VALUE                    0x0000
#endif

/*
** Attribute 12: IM Software revision (CHAR, 3 * UINT8)
*/
#ifndef DPV1_IA_SW_REV_ENABLE
   #define DPV1_IA_SW_REV_ENABLE                   1
   #define DPV1_IA_SW_REV_SYMBOL_VALUE             'V'
   #define DPV1_IA_SW_REV_MAJOR_VALUE              99
   #define DPV1_IA_SW_REV_MINOR_VALUE              99
   #define DPV1_IA_SW_REV_BUILD_VALUE              99
#endif

#endif  /* DPV1_OBJ_ENABLE */

/*------------------------------------------------------------------------------
** Application Object (0xFF)
**------------------------------------------------------------------------------
*/
#if APP_OBJ_ENABLE
/*
** Attribute 8: Vendor Name (Array of CHAR - {0x00-0xFF)
**
** NOTE: This attribute is superseded by the vendor name in the corresponding
** network specific host object if existent and enabled.
*/
#ifndef APP_IA_VENDOR_NAME_ENABLE
   #define APP_IA_VENDOR_NAME_ENABLE               0
   #define APP_IA_VENDOR_NAME_VALUE                "Vendor Name"
#endif

/*
** Attribute 9: Product Name (Array of CHAR - {0x00-0xFF)
**
** NOTE: This attribute is superseded by the product name in the corresponding
** network specific host object if existent and enabled.
*/
#ifndef APP_IA_PRODUCT_NAME_ENABLE
   #define APP_IA_PRODUCT_NAME_ENABLE              0
   #define APP_IA_PRODUCT_NAME_VALUE               "Product Name"
#endif

/*
** Attribute 10: Firmware Version (Struct of {UINT8, UINT8, UINT8} - {0x00-0xFF)
**
** NOTE: This attribute is superseded by the firmware version in the corresponding
** network specific host object if existent and enabled.
*/
#ifndef APP_IA_FIRMWARE_VERSION_ENABLE
   #define APP_IA_FIRMWARE_VERSION_ENABLE          0
   #define APP_IA_FIRMWARE_VERSION_MAJOR_VALUE     1
   #define APP_IA_FIRMWARE_VERSION_MINOR_VALUE     2
   #define APP_IA_FIRMWARE_VERSION_BUILD_VALUE     3
#endif

/*
** Attribute 11: Hardware Version (UINT16 - {0x0000-0xFFFF)
**
** NOTE: This attribute is superseded by the hardware version in the corresponding
** network specific host object if existent and enabled.
*/
#ifndef APP_IA_HARDWARE_VERSION_ENABLE
   #define APP_IA_HARDWARE_VERSION_ENABLE          0
   #define APP_IA_HARDWARE_VERSION_VALUE           0x0102
#endif

#endif /* #if APP_OBJ_ENABLE */

#endif  /* inclusion lock */
