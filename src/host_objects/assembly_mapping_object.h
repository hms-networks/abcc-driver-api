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

#endif  /* inclusion lock */
