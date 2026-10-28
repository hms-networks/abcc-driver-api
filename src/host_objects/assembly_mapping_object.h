/*******************************************************************************
** Copyright 2015-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Assembly Mapping Object 0xEB - Public interfaces
**
** To enable dynamic assemblies, the following definition must be:
** ASM_SUPPORT_DYNAMIC_INSTANCES 1.
**
** In abcc_driver_config.h make sure that the following definitions are set to:
** ABCC_CFG_REMAP_SUPPORT_ENABLED 1.
**
** -----------------------------------------------------------------------------
** EtherNet/IP specific requirements
** -----------------------------------------------------------------------------
** If there is at least one mappable assembly mapping instance defined, make
** sure that the following definition is set in abcc_driver_config.h:
** EIP_OBJ_ENABLE 1
** ABCC_ETHERNETIP_OBJ_PROD_INSTANCE_GET_*(x, y)
** ABCC_ETHERNETIP_OBJ_CONS_INSTANCE_GET_*(x, y)
**
********************************************************************************
*/

#ifndef ASM_OBJ_H_
#define ASM_OBJ_H_

#include "abp.h"

/*------------------------------------------------------------------------------
** Initializes the Assembly Mapping object.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ASM_Init( void );

/*------------------------------------------------------------------------------
** Process an object request against the Assembly Mapping Object.
**------------------------------------------------------------------------------
** Arguments:
**    psMsgBuffer - Pointer to ABCC command message.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ASM_ProcessCmdMsg( ABP_MsgType* psNewMessage );

/*------------------------------------------------------------------------------
** Copies the current data of all ADIs mapped in an assembly mapping instance
** to a buffer, packed the same way as the Read_Assembly_Data command response.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance     - Assembly mapping instance number (1 to the number of
**                    instances returned by ABCC_API_CbfGetNumAsmInstances()).
**    pxBuffer      - Pointer to the destination buffer. The caller must make
**                    sure the buffer can hold the complete mapped data.
**    piOctetOffset - Pointer to the octet offset in pxBuffer where writing
**                    starts. On return it is advanced past the written data.
**
** Returns:
**    TRUE  - Data was written to the buffer.
**    FALSE - The instance number is invalid. Nothing was written.
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ASM_GetData( UINT16 iInstance,
                          void* pxBuffer,
                          UINT16* piOctetOffset );

#endif  /* inclusion lock */
