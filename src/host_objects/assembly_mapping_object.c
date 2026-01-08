/*******************************************************************************
** Copyright 2015-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Implementation of the Assembly Mapping Object.
********************************************************************************
*/

#include "../abcc_api_config.h"

#if ASM_OBJ_ENABLE

#include "abcc.h"
#include "abp_asm.h"
#include "abcc_api.h"
#include "abcc_application_data_interface.h"
#include "application_data_object.h"
#include "assembly_mapping_object.h"

#if !ABCC_CFG_REMAP_SUPPORT_ENABLED
   #error "ABCC_CFG_REMAP_SUPPORT_ENABLED must be set to 1 to use the assembly mapping ojbect"
#endif

/*------------------------------------------------------------------------------
** Object attribute values
**------------------------------------------------------------------------------
*/
#define ASM_OA_NAME_VALUE           "Assembly mapping"
#define ASM_OA_REV_VALUE            1

/*------------------------------------------------------------------------------
** Structure describing an Assembly mapping object.
**------------------------------------------------------------------------------
*/
typedef struct asm_Object
{
   const  char* pcName;
   UINT8  bRevision;
   UINT16 iNumberOfInstances;
   UINT16 iHighestInstanceNo;
   /* Write PD Instance list is generated in runtime */
   /* Read PD Instance list is generated in runtime */
}
asm_ObjectType;

/*------------------------------------------------------------------------------
** Declare the Assembly mapping object instance.
**------------------------------------------------------------------------------
*/
static asm_ObjectType asm_sObject =
{
   ASM_OA_NAME_VALUE,       /* Name.                                          */
   ASM_OA_REV_VALUE,        /* Revision.                                      */
   0,                       /* Number of instances. (assigned in runtime)     */
   0                        /* Highest instance number. (assigned in runtime) */
};

/*------------------------------------------------------------------------------
** Processes commands to the Assembly mapping Instance
**------------------------------------------------------------------------------
** Arguments:
**    psNewMessage - Pointer to a ABP_MsgType message.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
static void InstanceCommand( ABP_MsgType* psNewMessage )
{
   const ASM_InstanceType* psInstance;
   UINT16                  iMapSize;
   UINT16                  iOctetOffset;

   if( ABCC_GetMsgInstance( psNewMessage ) > ABCC_API_CbfGetNumAsmInstances() )
   {
      /*
      ** The requested instance does not exist.
      ** Respond with an error.
      */
      ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_UNSUP_INST );

      return;
   }

   psInstance = ABCC_API_CbfGetAsmInstances()[ ABCC_GetMsgInstance( psNewMessage ) - 1 ];

   switch( ABCC_GetMsgCmdBits( psNewMessage ) )
   {
   case ABP_CMD_GET_ATTR:
   {
      switch( ABCC_GetMsgCmdExt0( psNewMessage ) )
      {
      case ABP_ASM_IA_DESCRIPTOR:
      {
         /*
         ** The 'descriptor' attribute is requested.
         ** Copy the attribute to a response message.
         */
         ABCC_SetMsgData32( psNewMessage,
                            psInstance->lDescriptor,
                            0 );

         ABP_SetMsgResponse( psNewMessage, ABP_UINT32_SIZEOF );

         break;
      }
#if ASM_IA_NAME_ENABLE
      case ABP_ASM_IA_NAME:
      {
         /*
         ** The 'name' attribute is requested.
         ** Copy the attribute to a response message.
         */
         UINT16 iStrLength;

         iStrLength = (UINT16)strlen( psInstance->pacName );
         ABCC_SetMsgString( psNewMessage, psInstance->pacName, iStrLength, 0 );
         ABP_SetMsgResponse( psNewMessage, (UINT8)iStrLength );

         break;
      }
#endif /* #if ASM_IA_NAME_ENABLE */
#if ASM_IA_MAX_NUM_ADI_MAPS_ENABLE
      case ABP_ASM_IA_MAX_NUM_ADI_MAPS:
         ABCC_SetMsgData16( psNewMessage, psInstance->iMaxNumAdiMaps, 0 );
         ABP_SetMsgResponse( psNewMessage, ABP_ASM_IA_MAX_NUM_ADI_MAPS_DS );

         break;
#endif
      default:
      {
         /*
         ** The 'ADI Map X' attribute or any other attribute is requested, verify
         ** if the requested attribute is a valid 'ADI MAP X' attribute..
         */
         UINT16 iAdisPerAttribute;
         UINT16 iNumAdisInMap;
         UINT16 iNumAdisInAttribute;
         UINT16 iIndex;
         UINT8  bAttribute;
         UINT8  bAdiMapNum;
         UINT8  bHighestAttribute;

         if( ABCC_GetMessageChannelSize() == ABP_MAX_MSG_255_DATA_BYTES )
         {
            iAdisPerAttribute = 62;
         }
         else
         {
            iAdisPerAttribute = 380;
         }

         iNumAdisInMap = AD_GetNumAdisInMap( psInstance->psMap );
         bAttribute = ABCC_GetMsgCmdExt0( psNewMessage );
         bHighestAttribute = (UINT8)( ( iNumAdisInMap / iAdisPerAttribute ) + ABP_ASM_IA_ADI_MAP_XX );

         if( ( bAttribute < ABP_ASM_IA_ADI_MAP_XX ) ||
             ( bAttribute >= ABP_ASM_IA_NAME ) ||
             ( bAttribute > bHighestAttribute ) )
         {
            /*
            ** This attribute does not exists.
            */
            ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_INV_CMD_EXT_0 );
            break;
         }

         bAdiMapNum = bAttribute - ABP_ASM_IA_ADI_MAP_XX;

         if( iNumAdisInMap >= ( ( bAdiMapNum + 1 ) * iAdisPerAttribute ) )
         {
            iNumAdisInAttribute = iAdisPerAttribute;
         }
         else
         {
            iNumAdisInAttribute = iNumAdisInMap - ( bAdiMapNum * iAdisPerAttribute );
         }

         if( ( iNumAdisInAttribute * ABP_BITS32_SIZEOF ) > ABCC_GetMaxMessageSize() )
         {
            ABCC_LOG_WARNING( ABCC_EC_BAD_ASSEMBLY_INSTANCE, (UINT32)ABCC_GetMsgInstance( psNewMessage ),
                              "Too many ADI's in this assembly mapping instance (%" PRIu16 ") for the current configuration of " \
                              "ABCC_CFG_MAX_MSG_SIZE (%d) or message channel size too small (%" PRIu16 ")\n" \
                              "Minimum message size required: %d\n",
                              ABCC_GetMsgInstance( psNewMessage ),
                              ABCC_CFG_MAX_MSG_SIZE,
                              ABCC_GetMessageChannelSize(),
                              iNumAdisInAttribute * ABP_BITS32_SIZEOF );

            break;
         }

         iOctetOffset = 0;

         for( iIndex = bAdiMapNum * iAdisPerAttribute;
              ( iIndex < iNumAdisInMap ) && ( iIndex < ( ( bAdiMapNum + 1 ) * iAdisPerAttribute ) );
              iIndex++ )
         {
            UINT8 bNumElem;
            UINT8 bStartIndex;

            if( psInstance->psMap[ iIndex ].bNumElem == AD_MAP_ALL_ELEM )
            {
               /*
               ** Convert internal representation for all elements to the
               ** actual number of elements.
               */
               bNumElem = AD_GetAdiInstEntry( psInstance->psMap[ iIndex ].iInstance )->bNumOfElements;
               bStartIndex = 0;
            }
            else
            {
               bNumElem = psInstance->psMap[ iIndex ].bNumElem;
               bStartIndex = psInstance->psMap[ iIndex ].bElemStartIndex;
            }

            ABCC_SetMsgData32( psNewMessage,
                               (UINT32)psInstance->psMap[ iIndex ].iInstance |
                               ( (UINT32)bStartIndex << 16 ) |
                               ( (UINT32)bNumElem << 24 ),
                               iOctetOffset );

            iOctetOffset += ABP_BITS32_SIZEOF;
         }

         ABP_SetMsgResponse( psNewMessage, iOctetOffset );

         break;
      }
      } /* end of switch( ABCC_GetMsgCmdExt0( psNewMessage ) ) */

      break;
   }

#if ASM_SUPPORT_DYNAMIC_INSTANCES
   case ABP_CMD_SET_ATTR:
   {
      switch( ABCC_GetMsgCmdExt0( psNewMessage ) )
      {
      case ABP_ASM_IA_DESCRIPTOR:
#if ASM_IA_NAME_ENABLE
      case ABP_ASM_IA_NAME:
#endif
#if ASM_IA_MAX_NUM_ADI_MAPS_ENABLE
      case ABP_ASM_IA_MAX_NUM_ADI_MAPS:
#endif
         ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_ATTR_NOT_SETABLE );

         break;

      default:
      {
         UINT16 iAdisPerAttribute;
         UINT16 iMaxAdisForThisAttribute;
         UINT16 iIndex;
         UINT16 iStartIndex;
         UINT16 iEndIndex;
         UINT8  bHighestAttribute;

         iAdisPerAttribute = ( ABCC_GetMessageChannelSize() / ABP_BITS32_SIZEOF ) - 1;
         bHighestAttribute = (UINT8)( ( ASM_MAX_ADI_MAPS_PER_INSTANCE / iAdisPerAttribute ) + ABP_ASM_IA_ADI_MAP_XX );

         if( ( ABCC_GetMsgCmdExt0( psNewMessage ) < ABP_ASM_IA_ADI_MAP_XX ) ||
             ( ABCC_GetMsgCmdExt0( psNewMessage ) >= ABP_ASM_IA_NAME ) ||
             ( ABCC_GetMsgCmdExt0( psNewMessage ) > bHighestAttribute ) )
         {
            /*
            ** This attribute does not exists.
            */
            ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_INV_CMD_EXT_0 );
            break;
         }

         if( ( psInstance->lDescriptor & ABP_ASM_IA_DESC_STATIC_DYNAMIC_MASK ) == ABP_ASM_IA_DESC_STATIC )
         {
            ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_ATTR_NOT_SETABLE );
            break;
         }

         /*
         ** Calculate how many ADIs there is room for in the written
         ** attribute.
         */
         if( ABCC_GetMsgCmdExt0( psNewMessage ) == bHighestAttribute )
         {
            iMaxAdisForThisAttribute = ASM_MAX_ADI_MAPS_PER_INSTANCE % iAdisPerAttribute;
         }
         else
         {
            iMaxAdisForThisAttribute = iAdisPerAttribute;
         }

         if( ABCC_GetMsgDataSize( psNewMessage ) > ( iMaxAdisForThisAttribute * ABP_BITS32_SIZEOF ) )
         {
            ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_TOO_MUCH_DATA );
            break;
         }

         if( ( ABCC_GetMsgDataSize( psNewMessage ) % ABP_BITS32_SIZEOF ) != 0 )
         {
            /*
            ** Not a valid ADI map since data size isn't divisible by 4.
            */
            ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_OUT_OF_RANGE );
            break;
         }

         iStartIndex = ( ( ABCC_GetMsgCmdExt0( psNewMessage ) - ABP_ASM_IA_ADI_MAP_XX ) * iAdisPerAttribute );
         iEndIndex   = iStartIndex + ( ABCC_GetMsgDataSize( psNewMessage ) / ABP_BITS32_SIZEOF );

         iOctetOffset = 0;

         /*
         ** Validate the data in the command before altering the ADI map.
         */
         for( iIndex = 0; iIndex < ( ABCC_GetMsgDataSize( psNewMessage ) / ABP_BITS32_SIZEOF ); iIndex++ )
         {
            const  AD_AdiEntryType* psAdiEntry;
            UINT16                  iInstance;
            UINT8                   bFirstElement;
            UINT8                   bNumberOfElements;

            ABCC_GetMsgData16( psNewMessage, &iInstance, iOctetOffset );
            iOctetOffset += ABP_BITS16_SIZEOF;
            ABCC_GetMsgData8( psNewMessage, &bFirstElement, iOctetOffset );
            iOctetOffset += ABP_BITS8_SIZEOF;
            ABCC_GetMsgData8( psNewMessage, &bNumberOfElements, iOctetOffset );
            iOctetOffset += ABP_BITS8_SIZEOF;

            /*
            ** Skip padding entries.
            */
            if( iInstance == AD_MAP_PAD_ADI )
            {
               continue;
            }

            /*
            ** Make sure that the ADI exists.
            */
            psAdiEntry = AD_GetAdiInstEntry( iInstance );

            if( psAdiEntry == NULL )
            {
               ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_OUT_OF_RANGE );
               return;
            }

            /*
            ** Make sure the element range is valid.
            */
            if( ( bFirstElement + bNumberOfElements ) > psAdiEntry->bNumOfElements )
            {
               ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_OUT_OF_RANGE );
               return;
            }

            /*
            ** If the assembly mapping instance is mappable to process data we
            ** need to make sure that each element also is mappable.
            */
            if( ( psInstance->lDescriptor & ABP_ASM_IA_DESC_PD_MAPPABLE_MASK ) == ABP_ASM_IA_DESC_PD_MAPPABLE )
            {
               UINT8 bDescriptorBit;

               if( ( psInstance->lDescriptor & ABP_ASM_IA_DESC_WRITE_READ_MASK ) == ABP_ASM_IA_DESC_WRITE )
               {
                  bDescriptorBit = ABP_APPD_DESCR_MAPPABLE_WRITE_PD;
               }
               else
               {
                  bDescriptorBit = ABP_APPD_DESCR_MAPPABLE_READ_PD;
               }
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
               if( psAdiEntry->psStruct != NULL )
               {
                  UINT8 bIndex;

                  /*
                  ** This is a struct, we need to go through the descriptors of
                  ** all the elements indicated in the ADI map.
                  */
                  for( bIndex = bFirstElement; bIndex < ( bFirstElement + bNumberOfElements ); bIndex++ )
                  {
                     if( !( psAdiEntry->psStruct[ bIndex ].bDesc & bDescriptorBit ) )
                     {
                        ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_OUT_OF_RANGE );
                        return;
                     }
                  }
               }
               else
#endif
               {
                  /*
                  ** Not a struct, all elements have the same descriptor.
                  */
                  if( !( psAdiEntry->bDesc & bDescriptorBit ) )
                  {
                     ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_OUT_OF_RANGE );
                     return;
                  }
               }
            }
         }

         iOctetOffset = 0;

         for( iIndex = iStartIndex; iIndex < iEndIndex; iIndex++ )
         {
            ABCC_GetMsgData16( psNewMessage, &psInstance->psMap[ iIndex ].iInstance, iOctetOffset );
            iOctetOffset += ABP_BITS16_SIZEOF;
            ABCC_GetMsgData8( psNewMessage, &psInstance->psMap[ iIndex ].bElemStartIndex, iOctetOffset );
            iOctetOffset += ABP_BITS8_SIZEOF;
            ABCC_GetMsgData8( psNewMessage, &psInstance->psMap[ iIndex ].bNumElem, iOctetOffset );
            iOctetOffset += ABP_BITS8_SIZEOF;

            if( ( psInstance->lDescriptor & ABP_ASM_IA_DESC_WRITE_READ_MASK ) == ABP_ASM_IA_DESC_WRITE )
            {
               psInstance->psMap[ iIndex ].eDir = PD_WRITE;
            }
            else
            {
               psInstance->psMap[ iIndex ].eDir = PD_READ;
            }
         }

         /*
         ** Finalize the ADI map by indicating end of mapping.
         */
         psInstance->psMap[ iIndex ].eDir = PD_END_MAP;

         ABP_SetMsgResponse( psNewMessage, 0 );
      }
      }

      break;
   }
#endif /* #if ASM_SUPPORT_DYNAMIC_INSTANCES */

   case ABP_ASM_CMD_WRITE_ASSEMBLY_DATA:
   {
      iMapSize = AD_GetMapSizeOctets( psInstance->psMap );

      if( ABCC_GetMsgDataSize( psNewMessage ) < iMapSize )
      {
         ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_NOT_ENOUGH_DATA );

         break;
      }
      else if( ABCC_GetMsgDataSize( psNewMessage ) > iMapSize )
      {
         ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_TOO_MUCH_DATA );

         break;
      }

      iOctetOffset = 0;

      AD_WritePdMapFromBuffer( psInstance->psMap,
                               ABCC_GetMsgDataPtr( psNewMessage ),
                               &iOctetOffset );

      ABP_SetMsgResponse( psNewMessage, 0 );

      break;
   }
   case ABP_ASM_CMD_READ_ASSEMBLY_DATA:
   {
      iMapSize = AD_GetMapSizeOctets( psInstance->psMap );

      if( ABCC_GetMaxMessageSize() < iMapSize )
      {
         ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_MSG_CHANNEL_TOO_SMALL );

         break;
      }

      iOctetOffset = 0;

      AD_WriteBufferFromPdMap( ABCC_GetMsgDataPtr( psNewMessage ),
                               &iOctetOffset,
                               psInstance->psMap );

      ABP_SetMsgResponse( psNewMessage, iOctetOffset );

      break;
   }
   default:

      /*
      ** Unsupported command.
      */
      ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_UNSUP_CMD );

      break;
   }
}

/*------------------------------------------------------------------------------
** Processes commands to the Assembly mapping Object (Instance 0)
**------------------------------------------------------------------------------
** Arguments:
**    psNewMessage - Pointer to a ABP_MsgType message.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
static void ObjectCommand( ABP_MsgType* psNewMessage )
{
   UINT16                   iStrLength;
   UINT16                   iIndex;
   UINT16                   iRspSize;
   const ASM_InstanceType** ppsInstance;

   /*
   ** This function processes commands to the Assembly Mapping Object (Instance 0).
   */
   switch( ABCC_GetMsgCmdBits( psNewMessage ) )
   {
   case ABP_CMD_GET_ATTR:
   {
      switch( ABCC_GetMsgCmdExt0( psNewMessage ) )
      {
      case ABP_OA_NAME:

         /*
         ** The 'name' attribute is requested.
         ** Copy the attribute to a response message.
         */
         iStrLength = (UINT16)strlen( asm_sObject.pcName );
         ABCC_SetMsgString( psNewMessage, asm_sObject.pcName, iStrLength, 0 );
         ABP_SetMsgResponse( psNewMessage, (UINT8)iStrLength );

         break;

      case ABP_OA_REV:

         /*
         ** The 'revision' attribute is requested.
         ** Copy the attribute to a response message.
         */
         ABCC_SetMsgData8( psNewMessage, asm_sObject.bRevision, 0 );
         ABP_SetMsgResponse( psNewMessage, ABP_OA_REV_DS );

         break;

      case ABP_OA_NUM_INST:

         /*
         ** The 'Number of Instances' attribute is requested.
         ** Copy the attribute to a response message.
         */
         ABCC_SetMsgData16( psNewMessage, asm_sObject.iNumberOfInstances, 0 );
         ABP_SetMsgResponse( psNewMessage, ABP_OA_NUM_INST_DS );

         break;

      case ABP_OA_HIGHEST_INST:

         /*
         ** The 'Highest Instance Number' attribute is requested.
         ** Copy the attribute to a response message.
         */
         ABCC_SetMsgData16( psNewMessage, asm_sObject.iHighestInstanceNo, 0 );
         ABP_SetMsgResponse( psNewMessage, ABP_OA_HIGHEST_INST_DS );

         break;

      case ABP_ASM_OA_WRITE_PD_INST_LIST:

         /*
         ** The 'Write PD Instance list' attribute is requested.
         ** Copy the attribute to a response message.
         */

         ppsInstance = ABCC_API_CbfGetAsmInstances();
         iRspSize = 0;

         for( iIndex = 0; iIndex < ABCC_API_CbfGetNumAsmInstances(); iIndex++ )
         {
            if( ( (*ppsInstance)->lDescriptor & ( ABP_ASM_IA_DESC_WRITE_READ_MASK | ABP_ASM_IA_DESC_PD_MAPPABLE_MASK ) ) ==
                ( ABP_ASM_IA_DESC_WRITE | ABP_ASM_IA_DESC_PD_MAPPABLE ) )
            {
               ABCC_SetMsgData16( psNewMessage,
                                  iIndex + 1,
                                  iRspSize );

               iRspSize += ABP_UINT16_SIZEOF;
            }

            ppsInstance++;
         }

         ABP_SetMsgResponse( psNewMessage,
                             iRspSize );

         break;

      case ABP_ASM_OA_READ_PD_INST_LIST:

         /*
         ** The 'Read PD Instance list' attribute is requested.
         ** Copy the attribute to a response message.
         */

         ppsInstance = ABCC_API_CbfGetAsmInstances();
         iRspSize = 0;

         for( iIndex = 0; iIndex < ABCC_API_CbfGetNumAsmInstances(); iIndex++ )
         {
            if( ( (*ppsInstance)->lDescriptor & ( ABP_ASM_IA_DESC_WRITE_READ_MASK | ABP_ASM_IA_DESC_PD_MAPPABLE_MASK ) ) ==
                ( ABP_ASM_IA_DESC_READ | ABP_ASM_IA_DESC_PD_MAPPABLE ) )
            {
               ABCC_SetMsgData16( psNewMessage,
                                  iIndex + 1,
                                  iRspSize );

               iRspSize += ABP_UINT16_SIZEOF;
            }

            ppsInstance++;
         }

         ABP_SetMsgResponse( psNewMessage,
                             iRspSize );

         break;

      default:

         /*
         ** Unsupported attribute.
         */
         ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_INV_CMD_EXT_0 );

         break;
      }
      break;
   }
   default:

      /*
      ** Unsupported command.
      */
      ABP_SetMsgErrorResponse( psNewMessage, 1, ABP_ERR_UNSUP_CMD );

      break;

   } /* End of switch( Command number ) */
}

EXTFUNC BOOL ASM_GetData( UINT16 iInstance,
                          void* pxBuffer,
                          UINT16* piOctetOffset )
{
   const ASM_InstanceType* psInstance;

   if( ( iInstance == 0 ) ||
       ( iInstance > ABCC_API_CbfGetNumAsmInstances() ) )
   {
      return( FALSE );
   }

   psInstance = ABCC_API_CbfGetAsmInstances()[ iInstance - 1 ];

   AD_WriteBufferFromPdMap( pxBuffer,
                            piOctetOffset,
                            psInstance->psMap );

   return( TRUE );
}

void ASM_Init( void )
{
#if !ASM_SUPPORT_DYNAMIC_INSTANCES
   const ASM_InstanceType** ppsInstance;
   UINT16 iIndex;

   ppsInstance = ABCC_API_CbfGetAsmInstances();

   /*
   ** Verify that there are no dynamic assemblies.
   */
   for( iIndex = 0; iIndex < ABCC_API_CbfGetNumAsmInstances(); iIndex++ )
   {
      if( ( (*ppsInstance)->lDescriptor & ( ABP_ASM_IA_DESC_STATIC_DYNAMIC_MASK ) ) ==
          ABP_ASM_IA_DESC_DYNAMIC )
      {
         ABCC_LOG_ERROR( ABCC_EC_BAD_ASSEMBLY_INSTANCE,
            (UINT32)iIndex,
            "Assembly instance %" PRIu16 " is defined as dynamic although not supported\n", iIndex );
      }

      ppsInstance++;
   }
#endif

   asm_sObject.iNumberOfInstances = ABCC_API_CbfGetNumAsmInstances();
   asm_sObject.iHighestInstanceNo = ABCC_API_CbfGetNumAsmInstances();
}

void ASM_ProcessCmdMsg( ABP_MsgType* psNewMessage )
{
   if( ABCC_GetMsgInstance( psNewMessage ) == ABP_INST_OBJ )
   {
      /*
      ** Process the Assembly mapping object command
      */
      ObjectCommand( psNewMessage );
   }
   else
   {
      /*
      ** Process the Assembly mapping instance command
      */
      InstanceCommand( psNewMessage );
   }

   ABCC_SendRespMsg( psNewMessage );
}

#endif /* ASM_OBJ_ENABLE */
