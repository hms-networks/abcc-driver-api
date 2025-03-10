/*******************************************************************************
** Copyright 2024-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Handle incoming ABCC commands.
********************************************************************************
*/

#include "abcc.h"
#include "abcc_api.h"
#include "abcc_api_command_handler.h"
#include "abcc_api_command_handler_lookup.h"
#include "host_objects/application_data_object.h"

#define ABCC_MATH_MIN( nX, nY )   ( ( (nX) < (nY) ) ? (nX) : (nY) )
#define ABCC_OBJ_SPECIFIC_COMMAND( X ) ( ( X >= 0x10 && X <= 0x30 ) || X == 0x3F )

#ifdef ABCC_API_COMMAND_RESPONSE_LIST
static const Command_Handler_Lookup_Type command_handler_lookup_table[] = { ABCC_API_OBJ_ATTRIBUTE_RESPONSE_LIST ABCC_API_COMMAND_RESPONSE_LIST };
#else
static const Command_Handler_Lookup_Type command_handler_lookup_table[] = { ABCC_API_OBJ_ATTRIBUTE_RESPONSE_LIST };
#endif

static void SetResponseMessage( ABP_MsgType* psReceivedCommandMsg );
static BOOL FindCommandHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry );
static void GetAttributeHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry );
static void SetAttributeHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry );
static BOOL ObjectSpecificHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry );

void ABCC_CbfHandleCommandMessage( ABP_MsgType* psReceivedCommandMsg )
{
#if ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED
   if( ABCC_API_CbfCommandMessageHook( psReceivedCommandMsg ) )
   {
      return;
   }
#endif

   switch( ABCC_GetMsgDestObj( psReceivedCommandMsg ) )
   {
   case ABP_OBJ_NUM_APPD:
      AD_ProcObjectRequest( psReceivedCommandMsg );
      return;
   default:
   {
      SetResponseMessage( psReceivedCommandMsg  );
      ABCC_SendRespMsg( psReceivedCommandMsg );
      return;
   }
   }
}

static void SetResponseMessage( ABP_MsgType* psReceivedCommandMsg )
{
   const Command_Handler_Lookup_Type* psEntry;
   UINT8 bDestObj      = ABCC_GetMsgDestObj( psReceivedCommandMsg );
   UINT16 bInst        = ABCC_GetMsgInstance( psReceivedCommandMsg );
   ABP_MsgCmdType bCmd = ABCC_GetMsgCmdBits( psReceivedCommandMsg );

   const UINT32 lTableLength = sizeof( command_handler_lookup_table ) / sizeof( Command_Handler_Lookup_Type );

      for( size_t i = 0; i < lTableLength; i++ )
      {
         psEntry = &command_handler_lookup_table[i];
         if( bDestObj == psEntry->bObject )
         {
            for( ; i < lTableLength; i++ )
            {
               psEntry = &command_handler_lookup_table[i];
               if( ( bDestObj == psEntry->bObject ) &&
                   ( bInst    == psEntry->bInstance ) )
               {
                  for( ; i < lTableLength; i++ )
                  {
                     psEntry = &command_handler_lookup_table[i];
                     if( ( bDestObj == psEntry->bObject ) &&
                         ( bInst    == psEntry->bInstance ) &&
                         ( bCmd     == psEntry->bCommand ) )
                     {
                        /*
                        ** Entry found.
                        */
                        if( FindCommandHandler( psReceivedCommandMsg, psEntry ) )
                        {
                           /*
                           ** Command handler found.
                           */
                           return;
                        }
                     }
                  }
                  /*
                  ** No command handler entry was found due to unsupported command.
                  */
                  ABP_SetMsgErrorResponse( psReceivedCommandMsg, 0, ABP_ERR_UNSUP_CMD );
                  return;
               }
            }
            /*
            ** No command handler entry was found due to unsupported instance.
            */
            ABP_SetMsgErrorResponse( psReceivedCommandMsg, 0, ABP_ERR_UNSUP_INST );
            return;
         }
      }
      /*
      ** No command handler entry was found due to unsupported object.
      */
      ABP_SetMsgErrorResponse( psReceivedCommandMsg, 0, ABP_ERR_UNSUP_OBJ );
      return;
}

static BOOL FindCommandHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry )
{
   switch( psEntry->bCommand )
   {
   case ABP_CMD_GET_ATTR:
      if( ABCC_GetMsgCmdExt0( psReceivedCommandMsg ) == psEntry->uCmdExt.bAttr )
      {
         GetAttributeHandler( psReceivedCommandMsg, psEntry );
         return( TRUE );
      }
      break;
   case ABP_CMD_SET_ATTR:
      if( ABCC_GetMsgCmdExt0( psReceivedCommandMsg ) == psEntry->uCmdExt.bAttr )
      {
         SetAttributeHandler( psReceivedCommandMsg, psEntry );
         return( TRUE );
      }
      break;
   case ABP_CMD_RESET:
      psEntry->uCbx.pnResetObj( ABCC_GetMsgCmdExt1( psReceivedCommandMsg ) );
      ABP_SetMsgResponse( psReceivedCommandMsg, 0 );
      return( TRUE );
   case ABP_CMD_GET_ENUM_STR:
      if( ABCC_GetMsgCmdExt0( psReceivedCommandMsg ) == psEntry->uCmdExt.bAttr )
      {
         if( psEntry->uCbx.pnGetEnumStrAttr( ABCC_GetMsgCmdExt1( psReceivedCommandMsg ),
                                             (char*)ABCC_GetMsgDataPtr( psReceivedCommandMsg ),
                                             ABCC_GetMaxMessageSize() ) )
         {
            UINT16 iStrLength = (UINT16)strlen((char*)ABCC_GetMsgDataPtr( psReceivedCommandMsg ));
            if( iStrLength > ABCC_GetMaxMessageSize() )
            {
               ABCC_LOG_FATAL( ABCC_EC_MSG_BUFFER_OVERRUN, iStrLength, "Message buffer size exceeded.\n" );
            }
            ABP_SetMsgResponse( psReceivedCommandMsg, iStrLength );
         }
         else
         {
            ABP_SetMsgErrorResponse( psReceivedCommandMsg, 0, ABP_ERR_INV_CMD_EXT_1 );
         }
         return( TRUE );
      }
      break;
   default:
      if( ABCC_OBJ_SPECIFIC_COMMAND( psEntry->bCommand ) )
      {
         return( ObjectSpecificHandler( psReceivedCommandMsg, psEntry ) );
      }
      /*
      ** The command code is unknown.
      */
      ABP_SetMsgErrorResponse( psReceivedCommandMsg, 0, ABP_ERR_UNSUP_CMD );
      return( TRUE );
   }
   return( FALSE );
}

static void GetAttributeHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry )
{
   switch( psEntry->eServiceTag )
   {
      case SERVICE_BOOL8:
      case SERVICE_INT8:
      case SERVICE_UINT8:
         ABCC_SetMsgData8( psReceivedCommandMsg,
                           psEntry->uCbx.pnGetUint8Attr ?
                           psEntry->uCbx.pnGetUint8Attr() :
                           psEntry->uData.bUnsigned8,
                           0 );
         ABP_SetMsgResponse( psReceivedCommandMsg, ABP_UINT8_SIZEOF );
         return;
      case SERVICE_INT16:
      case SERVICE_UINT16:
         ABCC_SetMsgData16( psReceivedCommandMsg,
                            psEntry->uCbx.pnGetUint16Attr ?
                            psEntry->uCbx.pnGetUint16Attr() :
                            psEntry->uData.iUnsigned16,
                            0 );
         ABP_SetMsgResponse( psReceivedCommandMsg, ABP_UINT16_SIZEOF );
         return;
      case SERVICE_INT32:
      case SERVICE_UINT32:
         ABCC_SetMsgData32( psReceivedCommandMsg,
                            psEntry->uCbx.pnGetUint32Attr ?
                            psEntry->uCbx.pnGetUint32Attr() :
                            psEntry->uData.lUnsigned32,
                            0 );
         ABP_SetMsgResponse( psReceivedCommandMsg, ABP_UINT32_SIZEOF );
         return;
      case SERVICE_STR:
      {
         UINT16 iStrLength;
         if( psEntry->uCbx.pnGetStrAttr )
         {
            iStrLength = psEntry->uCbx.pnGetStrAttr( (char*)ABCC_GetMsgDataPtr( psReceivedCommandMsg ),
                                                     ABCC_MATH_MIN( ABCC_GetMaxMessageSize(),
                                                     psEntry->uAttrLength.iMaxDataSize ) );
         }
         else
         {
            const char* pStr = psEntry->uData.pacString;
            iStrLength = ( UINT16 )strlen( pStr );
            ABCC_SetMsgString( psReceivedCommandMsg,
                               pStr,
                               iStrLength,
                               0 );
         }
         ABP_SetMsgResponse( psReceivedCommandMsg, iStrLength );
         return;
      }
      case SERVICE_BUFFER:
         if( psEntry->uCbx.pnGetArrAttr )
         {
            UINT16 iSizeBytes = psEntry->uCbx.pnGetArrAttr( ABCC_GetMsgDataPtr( psReceivedCommandMsg ),
                                                            psEntry->uAttrLength.iMaxDataSize ?
                                                            psEntry->uAttrLength.iMaxDataSize :
                                                            ABCC_GetMaxMessageSize() );
            ABCC_SetMsgString( psReceivedCommandMsg,
                               psEntry->uData.pacStringBuffer,
                               iSizeBytes,
                               0 );
            ABP_SetMsgResponse( psReceivedCommandMsg, iSizeBytes );
         }
         else
         {
            ABCC_SetMsgString( psReceivedCommandMsg,
                               psEntry->uData.pacStringBuffer,
                               psEntry->uAttrLength.iDataSize,
                               0 );
            ABP_SetMsgResponse( psReceivedCommandMsg, psEntry->uAttrLength.iDataSize );
         }
         return;
      default:
         ABP_SetMsgErrorResponse( psReceivedCommandMsg, 1, ABP_ERR_GENERAL_ERROR );
         ABCC_LOG_ERROR( ABCC_EC_UNSUPPORTED_DATA_TYPE, (UINT32)psEntry->eServiceTag, "Response to Get_Attribute command not possible (unsupported datatype).\n" );
         break;
   }
}

static void SetAttributeHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry )
{
   switch( psEntry->eServiceTag )
   {
   case SERVICE_BOOL8:
   case SERVICE_INT8:
   case SERVICE_UINT8:
   {
      UINT8 bValue = 0;
      ABCC_GetMsgData8( psReceivedCommandMsg, &bValue, 0 );
      psEntry->uCbx.pnSetUint8Attr( bValue );
      ABP_SetMsgResponse( psReceivedCommandMsg, 0 );
      return;
   }
   case SERVICE_INT16:
   case SERVICE_UINT16:
   {
      UINT16 iValue = 0;
      ABCC_GetMsgData16( psReceivedCommandMsg, &iValue, 0 );
      psEntry->uCbx.pnSetUint16Attr( iValue );
      ABP_SetMsgResponse( psReceivedCommandMsg, 0 );
      return;
   }
   case SERVICE_INT32:
   case SERVICE_UINT32:
   {
      UINT32 lValue = 0;
      ABCC_GetMsgData32( psReceivedCommandMsg, &lValue, 0 );
      psEntry->uCbx.pnSetUint32Attr( lValue );
      ABP_SetMsgResponse( psReceivedCommandMsg, 0 );
      return;
   }
   case SERVICE_STR:
      psEntry->uCbx.pnSetStrAttr(
         (char*)ABCC_GetMsgDataPtr( psReceivedCommandMsg ),
         ABCC_GetMsgDataSize( psReceivedCommandMsg ) );
      return;
   case SERVICE_BUFFER:
      psEntry->uCbx.pnSetArrAttr(
         ABCC_GetMsgDataPtr( psReceivedCommandMsg ),
         ABCC_GetMsgDataSize( psReceivedCommandMsg ) );
      return;
   default:
     ABP_SetMsgErrorResponse( psReceivedCommandMsg, 1, ABP_ERR_GENERAL_ERROR );
     ABCC_LOG_ERROR( ABCC_EC_UNSUPPORTED_DATA_TYPE, (UINT32)psEntry->eServiceTag, "Unsupported Set_Attribute datatype.\n" );
     break;
   }
}

static BOOL ObjectSpecificHandler( ABP_MsgType* psReceivedCommandMsg, const Command_Handler_Lookup_Type* psEntry )
{
   switch( psEntry->bObject )
   {
#if MQTT_OBJ_ENABLE
   case ABP_OBJ_NUM_MQTT:
   break;
#endif
#if OPCUA_OBJ_ENABLE
   case ABP_OBJ_NUM_OPCUA:
   break;
#endif
#if SAFE_OBJ_ENABLE
   case ABP_OBJ_NUM_SAFE:
   break;
#endif
#if EPL_OBJ_ENABLE
   case ABP_OBJ_NUM_EPL:
   break;
#endif
#if EIP_OBJ_ENABLE
   case ABP_OBJ_NUM_EIP:
   break;
#endif
#if PRT_OBJ_ENABLE
   case ABP_OBJ_NUM_PNIO:
   break;
#endif
#if DPV1_OBJ_ENABLE
   case ABP_OBJ_NUM_DPV1:
   break;
#endif
#if DEV_OBJ_ENABLE
   case ABP_OBJ_NUM_DEV:
   break;
#endif
#if MOD_OBJ_ENABLE
   case ABP_OBJ_NUM_MOD:
   break;
#endif
#if COP_OBJ_ENABLE
   case ABP_OBJ_NUM_COP:
   break;
#endif
#if ETN_OBJ_ENABLE
   case ABP_OBJ_NUM_ETN:
   break;
#endif
#if ECT_OBJ_ENABLE
   case ABP_OBJ_NUM_ECT:
   break;
#endif
#if CCL_OBJ_ENABLE
   case ABP_OBJ_NUM_CCL:
   break;
#endif
#if CFN_OBJ_ENABLE
   case ABP_OBJ_NUM_CFN:
   break;
#endif
#if CIET_OBJ_ENABLE
   case ABP_OBJ_NUM_CIET:
   break;
#endif
#if BAC_OBJ_ENABLE
   case ABP_OBJ_NUM_BAC:
   break;
#endif
#if APP_OBJ_ENABLE
   case ABP_OBJ_NUM_APP:
      switch( psEntry->bCommand )
      {
      case ABP_APP_CMD_RESET_REQUEST:
         if( psEntry->uCbx.pnResetReqObj( ABCC_GetMsgCmdExt1( psReceivedCommandMsg ) ) )
         {
            ABP_SetMsgResponse( psReceivedCommandMsg, 0 );
         }
         else
         {
            ABP_SetMsgErrorResponse( psReceivedCommandMsg, 0, ABP_ERR_OBJ_SPECIFIC );
         }
         return( TRUE );
      default:
         /*
         ** The entry states support for an object specific command not found
         ** in this handler.
         */
         ABP_SetMsgErrorResponse( psReceivedCommandMsg, 1, ABP_ERR_UNSUP_CMD );
         return( TRUE );
      }
      break;
#endif
   default:
      /*
      ** The entry states support for an object specific command to an object
      ** not found in this handler.
      */
      ABP_SetMsgErrorResponse( psReceivedCommandMsg, 1, ABP_ERR_UNSUP_CMD );
      return( TRUE );
   }
   /*
   ** The entry matched object, instance, and command, but other data did not
   ** match. Keep looking for another entry.
   */
   return( FALSE );
}
