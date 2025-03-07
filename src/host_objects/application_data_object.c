/*******************************************************************************
** Copyright 2015-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Implementation of the AD object.
********************************************************************************
*/
#include "abcc_types.h"
#include "abcc_software_port.h"
#include "abcc_config.h"
#include "abcc_api_config.h"
#include "abp.h"
#include "abcc_application_data_interface.h"
#include "abcc.h"
#include "abcc_api.h"
#include "abcc_hardware_abstraction.h"
#include "application_data_object.h"

#define AD_OA_REV_VALUE                        3

#if( ABCC_CFG_REMAP_SUPPORT_ENABLED )
#if( AD_MAX_NUM_WRITE_MAP_ENTRIES > AD_MAX_NUM_READ_MAP_ENTRIES )
#define AD_MAX_OF_READ_WRITE_TO_MAP AD_MAX_NUM_WRITE_MAP_ENTRIES
#else
#define AD_MAX_OF_READ_WRITE_TO_MAP AD_MAX_NUM_READ_MAP_ENTRIES
#endif
#endif

/*
** Value used in ad_MapType as ADI index when ADI 0 is mapped.
*/
#define AD_MAP_PAD_INDEX                     ( 0xfffe )

/*
** Invalid ADI index.
*/
#define AD_INVALID_ADI_INDEX                 ( 0xffff )

/*
** All ADI indexes.
*/
#define AD_ALL_ADI_INDEX                     ( 0xffff )

/*------------------------------------------------------------------------------
** Union for all different property types.
**------------------------------------------------------------------------------
*/
typedef union ad_AllProperties
{
   AD_UINT8Type   sPropUint8;
   AD_SINT8Type   sPropInt8;
   AD_UINT16Type  sPropUint16;
   AD_SINT16Type  sPropInt16;
   AD_UINT32Type  sPropUint32;
   AD_SINT32Type  sPropInt32;
   AD_FLOAT32Type sPropFloat32;
#if( ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
   AD_FLOAT64Type sPropFloat64;
#endif
#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED )
   AD_UINT64Type  sPropUint64;
   AD_SINT64Type  sPropInt64;
#endif
}
ad_AllPropertiesType;

/*------------------------------------------------------------------------------
** Union for all different value types.
**------------------------------------------------------------------------------
*/
typedef union ad_AllData
{
#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED )
   UINT64   l64Unsigned;
   INT64    l64Signed;
#endif
   FLOAT32  rFloat;
#if( ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
   FLOAT64  dDouble;
#endif
   UINT32   lUnsigned;
   INT32    lSigned;
   UINT16   iUnsigned;
   INT16    iSigned;
   UINT8    bUnsigned;
   INT8     bSigned;
   BOOL     fBool;
}
ad_AllDataType;

/*------------------------------------------------------------------------------
** Type with mapping information for a single ADI
**------------------------------------------------------------------------------
** iAdiIndex      - Index to ADI entry table.
** bNumElements   - Number of mapped elements.
** bStartIndex    - Element start index for the mapping
**------------------------------------------------------------------------------
*/
typedef struct ad_Map
{
  UINT16           iAdiIndex;
  UINT8            bNumElements;
  UINT8            bStartIndex;
}
ad_MapType;

/*------------------------------------------------------------------------------
** Type with mapping information for a specific direction (read/write).
**------------------------------------------------------------------------------
** paiMappedAdiList    - Pointer to list of all mapped items.
** iNumMappedAdi       - Number of mapped ADI:s
** iMaxNumMappedAdi    - Maximum number of mapped ADI:s.
** iPdSize             - Current process data size in octets.
**------------------------------------------------------------------------------
*/
typedef struct ad_MapInfo
{
   ad_MapType*   paiMappedAdiList;
   UINT16        iNumMappedAdi;
   UINT16        iMaxNumMappedAdi;
   UINT16        iPdSize;
}
ad_MapInfoType;

static BOOL ad_fDoNetworkEndianSwap = FALSE;
static const AD_MapType* ad_asDefaultMap = NULL;
static const AD_AdiEntryType* ad_asADIEntryList = NULL;
static UINT16  ad_iNumOfADIs;
static UINT16  ad_iHighestInstanceNumber;
static ad_MapType ad_PdReadMapping[ AD_MAX_NUM_READ_MAP_ENTRIES ];
static ad_MapType ad_PdWriteMapping[ AD_MAX_NUM_WRITE_MAP_ENTRIES ];
static ad_MapInfoType ad_ReadMapInfo;
static ad_MapInfoType ad_WriteMapInfo;

/*------------------------------------------------------------------------------
** Converts number of octet offset to byte offset.
**------------------------------------------------------------------------------
*/
#ifdef ABCC_SYS_16_BIT_CHAR
#define OctetToByteOffset( x )  ( ( x ) >> 1 )
#else
#define OctetToByteOffset( x )  ( x )
#endif

/*------------------------------------------------------------------------------
** Checks if a ABP data type is either a bit or pad type.
**------------------------------------------------------------------------------
*/
#define Is_BITx_Or_PADx( type ) ( ABP_Is_PADx( type ) || ABP_Is_BITx( type ) )

/*------------------------------------------------------------------------------
** Min/max verification is not supported for bit data types, char and enum.
**------------------------------------------------------------------------------
*/
#define MIN_MAX_DEFAULT_NOT_SUPPORTED( type ) ( ( (type) == ABP_CHAR ) ||  Is_BITx_Or_PADx( type ) || ( type == ABP_BOOL1 ) )

/*------------------------------------------------------------------------------
** The bit variable is converted to whole octets and added to the octet variable.
** The remaining bits are saved to the bit variable.
** For a 16 bit char system the number of octets are kept even.
** Note that both input variables are updated.
**------------------------------------------------------------------------------
*/
#ifdef ABCC_SYS_16_BIT_CHAR
#define AddBitsToOctetSize( octet, bits ) \
do                                        \
{                                         \
   (octet) += ( (bits) >> 4 ) << 1;       \
   (bits) %= 16;                          \
}                                         \
while( 0 )
#else
#define AddBitsToOctetSize( octet, bits ) \
do                                        \
{                                         \
   (octet) += (bits) >> 3;                \
   (bits) %= 8;                           \
}                                         \
while( 0 )
#endif

/*------------------------------------------------------------------------------
** Calculates the bit offset to the startindex element in the ADI.
**------------------------------------------------------------------------------
*/
#define CalcStartIndexBitOffset( bDataType, iStartIndex )      \
   ABCC_GetDataTypeSizeInBits( bDataType ) * ( iStartIndex )

/*------------------------------------------------------------------------------
** Add octet variable and bit variable and round up to nearest octet.
**------------------------------------------------------------------------------
*/
#define SizeInOctets( octet, bits ) ( (octet) + ( (bits) + 7 ) / 8 )

/*------------------------------------------------------------------------------
** Convert bit offset to octet offset.
**------------------------------------------------------------------------------
*/
#define BitToOctetOffset( bitOffset ) ( (bitOffset) >> 3 )

/*------------------------------------------------------------------------------
** Copies a 16 bit values from a source to a destination. Each value will be
** endian swapped. The function support octet alignment.
**------------------------------------------------------------------------------
** Arguments:
**    pxDest            - Base pointer to the destination.
**    iDestOctetOffset  - Octet offset to the destination where the copy will
**                        begin.
**    pxSrc             - Bsse pointer to source data.
**    iSrcOctetOffset   - Octet offset to the source where the copy will begin.
**    iNumElem          - Number of 16 bit values to copy.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
static void Copy16WithEndianSwap( void* pxDest, UINT16 iDestOctetOffset,
                                  const void* pxSrc, UINT16 iSrcOctetOffset,
                                  UINT16 iNumElem )
{
   UINT16 i;
   UINT16 iConv;

   for( i = 0; i < iNumElem; i++ )
   {
      ABCC_PORT_Copy16( &iConv, 0, pxSrc, iSrcOctetOffset + ( i << 1 ) );
      iConv = ABCC_iEndianSwap( iConv );
      ABCC_PORT_Copy16( pxDest, iDestOctetOffset + ( i << 1 ), &iConv, 0 );
   }
}

/*------------------------------------------------------------------------------
** Copies a 32 bit values from a source to a destination. Each value will be
** endian swapped. The function support octet alignment.
**------------------------------------------------------------------------------
** Arguments:
**    pxDest            - Base pointer to the destination.
**    iDestOctetOffset  - Octet offset to the destination where the copy will
**                        begin.
**    pxSrc             - Bsse pointer to source data.
**    iSrcOctetOffset   - Octet offset to the source where the copy will begin.
**    iNumElem          - Number of 32 bit values to copy.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
static void Copy32WithEndianSwap( void* pxDest, UINT16 iDestOctetOffset,
                                  const void* pxSrc, UINT16 iSrcOctetOffset,
                                  UINT16 iNumElem )
{
   UINT16 i;
   UINT32 lConv;

   for( i = 0; i < iNumElem; i++ )
   {
      ABCC_PORT_Copy32( &lConv, 0, pxSrc, iSrcOctetOffset + ( i << 2 ) );
      lConv = ABCC_lEndianSwap( lConv );
      ABCC_PORT_Copy32( pxDest, iDestOctetOffset + ( i << 2 ), &lConv, 0 );
   }
}


/*------------------------------------------------------------------------------
** Copies a 64 bit values from a source to a destination. Each value will be
** endian swapped. The function support octet alignment.
**------------------------------------------------------------------------------
** Arguments:
**    pxDest            - Base pointer to the destination.
**    iDestOctetOffset  - Octet offset to the destination where the copy will
**                        begin.
**    pxSrc             - Bsse pointer to source data.
**    iSrcOctetOffset   - Octet offset to the source where the copy will begin.
**    iNumElem          - Number of 64 bit values to copy.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED || ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
static void Copy64WithEndianSwap( void* pxDest, UINT16 iDestOctetOffset,
                                  const void* pxSrc, UINT16 iSrcOctetOffset,
                                  UINT16 iNumElem )
{
   UINT16 i;
   UINT64 lConv;

   for( i = 0; i < iNumElem; i++ )
   {
      ABCC_PORT_Copy64( &lConv, 0, pxSrc, iSrcOctetOffset + ( i << 3 ) );
      lConv = ABCC_l64EndianSwap( lConv );
      ABCC_PORT_Copy64( pxDest, iDestOctetOffset + ( i << 3 ), &lConv, 0 );
   }
}
#endif

/*------------------------------------------------------------------------------
** Calculates the size of a part of or a complete ADI, in bits.
**------------------------------------------------------------------------------
** Arguments:
**    psAdiEntry         -  Pointer to ADI entry.
**    bNumElem           -  Number of elements
**    bElemStartIndex    -  First element index.
**
** Returns:
**    Size in bits.
**------------------------------------------------------------------------------
*/
static UINT16 GetAdiSizeInBits( const AD_AdiEntryType* psAdiEntry,
                                UINT8 bNumElem,
                                UINT8 bElemStartIndex )
{
   UINT16 iSize;
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
   UINT16 i;
   if( psAdiEntry->psStruct != NULL )
   {
      iSize = 0;
      for( i = bElemStartIndex; i < ( bNumElem + bElemStartIndex ); i++ )
      {
         iSize +=
            ABCC_GetDataTypeSizeInBits( psAdiEntry->psStruct[ i ].bDataType ) *
            psAdiEntry->psStruct[ i ].iNumSubElem;
      }
   }
   else
   {
      iSize = ABCC_GetDataTypeSizeInBits( psAdiEntry->bDataType ) * bNumElem;
   }
#else
      (void)bElemStartIndex;
      iSize = ABCC_GetDataTypeSizeInBits( psAdiEntry->bDataType ) * bNumElem;
#endif

   return( iSize );
}

/*------------------------------------------------------------------------------
** Calculates size of ADI rounded up to nearest octet.
**------------------------------------------------------------------------------
** Arguments:
**    psAdiEntry  -  Pointer to ADI entry.
**
** Returns:
**    Size in octets.
**------------------------------------------------------------------------------
*/
static UINT16 GetAdiSizeInOctets( const AD_AdiEntryType* psAdiEntry )
{
   UINT16 iSize;

   iSize = GetAdiSizeInBits( psAdiEntry, psAdiEntry->bNumOfElements, 0 );
   iSize = ( iSize + 7 ) / 8;

   return( iSize );
}

/*------------------------------------------------------------------------------
** Calculates total map size in octets.
**------------------------------------------------------------------------------
** Arguments:
**    psMap         -  Pointer to mapping information. The size is
**                     calculated based on the on the mapping list provided
**                     in the structure. The iPdSize member is updated with
**                     the new size.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
static void UpdateMapSize( ad_MapInfoType* psMap )
{
   UINT16 iMapIndex;
   UINT16 iAdiIndex;

   psMap->iPdSize = 0;
   for( iMapIndex = 0; iMapIndex < psMap->iNumMappedAdi; iMapIndex++ )
   {
      iAdiIndex = psMap->paiMappedAdiList[ iMapIndex ].iAdiIndex;

      if( iAdiIndex != AD_MAP_PAD_INDEX )
      {
         if( iAdiIndex >= ad_iNumOfADIs )
         {
            /*
            ** Pull the plug! The data in these tables should already have
            ** been checked and should be OK!
            */
            ABCC_LOG_FATAL( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
               (UINT32)iAdiIndex,
               "Error in PD map configuration, ADI index out of range (%" PRIu16 ")\n",
               iAdiIndex );
         }

         psMap->iPdSize +=
            GetAdiSizeInBits( &ad_asADIEntryList[ iAdiIndex ],
                                 psMap->paiMappedAdiList[ iMapIndex ].bNumElements,
                                 psMap->paiMappedAdiList[ iMapIndex ].bStartIndex );
      }
      else
      {
         psMap->iPdSize += psMap->paiMappedAdiList[ iMapIndex ].bNumElements;
      }
   }
   psMap->iPdSize = SizeInOctets( 0, psMap->iPdSize );
}

/*------------------------------------------------------------------------------
** Find ADI entry table index for the specified instance number.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance         -  Instance number.
**
** Returns:
**    0 - 0xfffe                - Index in ADI entry table.
**    AD_INVALID_ADI_INDEX      - Instance was not found.
**------------------------------------------------------------------------------
*/
static UINT16 GetAdiIndex( UINT16 iInstance )
{
   UINT16   iLow;
   UINT16   iMid;
   UINT16   iHigh;

   if( iInstance == 0 )
   {
      return( AD_MAP_PAD_INDEX );
   }

   if( ad_iNumOfADIs == 0 )
   {
      return( AD_INVALID_ADI_INDEX );
   }

   iLow = 0;
   iHigh = ad_iNumOfADIs - 1;

   while( iLow != iHigh )
   {
      iMid = iLow + ( ( iHigh - iLow + 1 ) / 2 );
      if( ad_asADIEntryList[ iMid ].iInstance > iInstance )
      {
         iHigh = iMid - 1;
      }
      else
      {
         iLow = iMid;
      }
   }

   if( ad_asADIEntryList[ iLow ].iInstance != iInstance )
   {
      iLow = AD_INVALID_ADI_INDEX;
   }

   return( iLow );
}

#if( ABCC_CFG_REMAP_SUPPORT_ENABLED )
/*------------------------------------------------------------------------------
** Check if the targeted ADI/element descriptor says that it is PD mappable in
** the requested PD direction.
**------------------------------------------------------------------------------
** Arguments:
**    bDataType         - Data type of the ADI/element in question.
**    bCmd              - Remap message command code.
**    bDescriptor       - Descriptor for the ADI/element in question.
**
** Returns:
**    TRUE if the ADI/element is mappable in the indicated direction.
**------------------------------------------------------------------------------
*/
static BOOL IsElementRemapAllowed( UINT8 bDataType, UINT8 bCmd, UINT8 bDesc )
{
   /*
   ** PADx is allowed, there does not have to be any valid descriptor to check.
   */
   if( ABP_Is_PADx( bDataType ) )
   {
      return( TRUE );
   }

   if( ( ( bCmd == ABP_APPD_REMAP_ADI_WRITE_AREA ) &&
         ( bDesc & ABP_APPD_DESCR_MAPPABLE_WRITE_PD ) ) ||
       ( ( bCmd == ABP_APPD_REMAP_ADI_READ_AREA ) &&
         ( bDesc & ABP_APPD_DESCR_MAPPABLE_READ_PD ) ) )
   {
      return( TRUE );
   }

   return( FALSE );
}

/*------------------------------------------------------------------------------
** Process of remap command.
**------------------------------------------------------------------------------
** Arguments:
**    ABP_MsgType         - Pointer to remap command.
**    psCurrMap           - Pointer to current mapping. Will hold the new
**                          mapping when processing is done.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
static void RemapProcessDataCommand( ABP_MsgType* psMsg,
                                     ad_MapInfoType* psCurrMap )
{

   UINT16 iAdi;
   UINT16 iMsgIndex;
   UINT16 iAddItemIndex;
   UINT16 iMapIndex;
   UINT16 bStartOfRemap;
   UINT8  bErrCode;
   UINT16 iItemsToRemove;
   UINT16 iItemsToAdd;
   UINT16 iDataSize;
   ad_MapType sMap;
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
   UINT16 iCnt;
#endif

   iDataSize = 1;
   bStartOfRemap = ABCC_GetMsgCmdExt( psMsg );

   ABCC_GetMsgData16( psMsg, &iItemsToRemove, 0 );
   ABCC_GetMsgData16( psMsg, &iItemsToAdd, 2 );

   /*
   ** A lot of sanity checks first since all actions of the command shall
   ** either be carried out or rejected.
   */
   if( ABCC_GetMsgDataSize( psMsg ) < 4 )
   {
      /*
      ** Not enough data provided
      */
      bErrCode = ABP_ERR_NOT_ENOUGH_DATA;
   }
   else if( bStartOfRemap > psCurrMap->iNumMappedAdi )
   {
      /*
      ** Not an allowed mapping number
      */
      bErrCode = ABP_ERR_INV_CMD_EXT_0;
   }
   else if( ( bStartOfRemap + iItemsToRemove ) > psCurrMap->iNumMappedAdi )
   {
      /*
      ** Cannot remove more than currently is mapped
      */
      bErrCode = ABP_ERR_OUT_OF_RANGE;
   }
   else if( ( psCurrMap->iNumMappedAdi + iItemsToAdd - iItemsToRemove )  >
              psCurrMap->iMaxNumMappedAdi )
   {
      /*
      ** This will result in more maps than we can handle
      */
      bErrCode = ABP_ERR_NO_RESOURCES;
   }
   else if( ABCC_GetMsgDataSize( psMsg ) < 4 + ( iItemsToAdd * 4 ) )
   {
      bErrCode = ABP_ERR_NOT_ENOUGH_DATA;
   }
   else if( ABCC_GetMsgDataSize( psMsg ) > 4 + ( iItemsToAdd * 4 ) )
   {
      bErrCode = ABP_ERR_TOO_MUCH_DATA;
   }
   else
   {
      bErrCode = ABP_ERR_NO_ERROR;
   }

   /*
   ** Check New ADI:s
   */
   if( bErrCode == ABP_ERR_NO_ERROR )
   {
      iAddItemIndex = 0;
      iMsgIndex = 4;
      while( iAddItemIndex < iItemsToAdd )
      {
         ABCC_GetMsgData16( psMsg, &iAdi, iMsgIndex );
         sMap.iAdiIndex = GetAdiIndex( iAdi );

         if( sMap.iAdiIndex == AD_INVALID_ADI_INDEX )
         {
            bErrCode = ABP_ERR_OBJ_SPECIFIC;
            /*
            ** The ADI does not exist.
            */
            ABCC_SetMsgData8( psMsg, ABP_APPD_ERR_MAPPING_ITEM_NAK, 1 );
            iDataSize = 2;
            /*
            ** Break early! It is not a good idea to use AD_INVALID_ADI_INDEX
            ** as an indexing value.
            */
            break;
         }

         iMsgIndex += 2;
         ABCC_GetMsgData8( psMsg, &sMap.bStartIndex, iMsgIndex++ );
         ABCC_GetMsgData8( psMsg, &sMap.bNumElements, iMsgIndex++ );

         if( sMap.iAdiIndex != AD_MAP_PAD_INDEX )
         {

            if( ( sMap.bStartIndex + sMap.bNumElements ) >
                   ad_asADIEntryList[ sMap.iAdiIndex ].bNumOfElements )
            {
               bErrCode = ABP_ERR_OBJ_SPECIFIC;
               /*
               ** Invalid number of elements
               */
               ABCC_SetMsgData8( psMsg, ABP_APPD_ERR_INVALID_TOTAL_SIZE, 1 );
               iDataSize = 2;
               /*
               ** One error is enough for a NAK, break the 'while'.
               */
               break;
            }

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
            if( ad_asADIEntryList[ sMap.iAdiIndex ].psStruct != NULL )
            {
               for( iCnt = 0; iCnt < sMap.bNumElements; iCnt++ )
               {
                  if( !( IsElementRemapAllowed(
                     ad_asADIEntryList[ sMap.iAdiIndex ].psStruct[ sMap.bStartIndex + iCnt ].bDataType,
                     ABCC_GetMsgCmdBits( psMsg ),
                     ad_asADIEntryList[ sMap.iAdiIndex ].psStruct[ sMap.bStartIndex + iCnt ].bDesc ) ) )
                  {
                     bErrCode = ABP_ERR_OBJ_SPECIFIC;
                     /*
                     ** Error found, skip the remaining elements.
                     */
                     break;
                  }
               }
            }
            else
#endif
            {
               if( !( IsElementRemapAllowed(
                  ad_asADIEntryList[ sMap.iAdiIndex ].bDataType,
                  ABCC_GetMsgCmdBits( psMsg ),
                  ad_asADIEntryList[ sMap.iAdiIndex ].bDesc ) ) )
               {
                  bErrCode = ABP_ERR_OBJ_SPECIFIC;
               }
            }
            if( bErrCode != ABP_ERR_NO_ERROR )
            {
               /*
               ** At least one ADI/element was not mappable in the indicated
               ** direction.
               */
               ABCC_SetMsgData8( psMsg, ABP_APPD_ERR_MAPPING_ITEM_NAK, 1 );
               iDataSize = 2;
               /*
               ** One error is enough for a NAK, break the 'while'.
               */
               break;
            }

         }
         iAddItemIndex++;
      }
   }

   if( bErrCode == ABP_ERR_NO_ERROR )
   {
      /*
      ** Move ADI if required
      */
      if( ( iItemsToRemove != iItemsToAdd ) &&
          ( iItemsToRemove  <  ( psCurrMap->iNumMappedAdi - bStartOfRemap  ) ) )
      {
         INT16  iItemsToMove;
         UINT16 iMoveFrom;
         UINT16 iMoveTo;
         UINT16 iIndex;

         /*
         ** Data needs to be moved
         */
         iMoveFrom = bStartOfRemap + iItemsToRemove;
         iMoveTo = bStartOfRemap  + iItemsToAdd;
         iItemsToMove = psCurrMap->iNumMappedAdi - bStartOfRemap - iItemsToRemove;

         if( iItemsToRemove > iItemsToAdd )
         {
            for( iIndex = 0; iIndex < iItemsToMove; iIndex++ )
            {
               psCurrMap->paiMappedAdiList[ iMoveTo + iIndex ] =
                  psCurrMap->paiMappedAdiList[ iMoveFrom + iIndex ];
            }
         }
         else
         {
            for( iIndex = iItemsToMove; iIndex > 0; iIndex-- )
            {
               psCurrMap->paiMappedAdiList[ iMoveTo + iIndex - 1 ] =
                  psCurrMap->paiMappedAdiList[ iMoveFrom + iIndex - 1 ];
            }
         }
      }

      iMapIndex = bStartOfRemap;
      psCurrMap->iNumMappedAdi -= iItemsToRemove;
      psCurrMap->iNumMappedAdi += iItemsToAdd;

      iMsgIndex = 4;
      for( iAddItemIndex = 0; iAddItemIndex < iItemsToAdd; iAddItemIndex++ )
      {
         ABCC_GetMsgData16( psMsg, &iAdi, iMsgIndex );
         psCurrMap->paiMappedAdiList[ iMapIndex ].iAdiIndex = GetAdiIndex( iAdi );
         iMsgIndex += 2;
         ABCC_GetMsgData8( psMsg, &psCurrMap->paiMappedAdiList[ iMapIndex ].bStartIndex, iMsgIndex++ );
         ABCC_GetMsgData8( psMsg, &psCurrMap->paiMappedAdiList[ iMapIndex ].bNumElements, iMsgIndex++ );
         iMapIndex++;
      }

      UpdateMapSize( psCurrMap );

      ABCC_SetMsgData16(psMsg, psCurrMap->iPdSize, 0);
      ABP_SetMsgResponse( psMsg, 2 );
      ABCC_SendRemapRespMsg( psMsg, ad_ReadMapInfo.iPdSize,
                             ad_WriteMapInfo.iPdSize);
   }
   else
   {
      ABP_SetMsgErrorResponse( psMsg, iDataSize, bErrCode );
      ABCC_SendRespMsg( psMsg );
   }
}
#endif


/*------------------------------------------------------------------------------
** Copy bit data. Any alignment is allowed.
**------------------------------------------------------------------------------
** Arguments:
**    pxDest            - Destination base pointer.
**    iDestBitOffset    - Bit offset relative destination pointer.
**    pxSrc             - Source base pointer.
**    iSrcBitOffset     - Bit offset relative source pointer.
**    bDataType         - Data type according to ABP_<X> types in abp.h
**    iNumElem          - Number of elements to copy.
**
** Returns:
**    Size of copied data in bits.
**------------------------------------------------------------------------------
*/
static UINT16 CopyBitData( void* pxDest,
                           UINT16 iDestBitOffset,
                           const void* pxSrc,
                           UINT16 iSrcBitOffset,
                           UINT8 bDataType,
                           UINT16 iNumElem )
{
   UINT8  bCopySize;
   UINT16 i;
   UINT16 iSetBitSize = 0;
   UINT32 lBitMask;
   UINT32 lSrc;
   UINT32 lDest;
   UINT16 iSrcOctetOffset;
   UINT16 iDestOctetOffset;

   iSrcOctetOffset = 0;
   iDestOctetOffset = 0;

   if( ABP_Is_PADx( bDataType ) )
   {
      /*
      ** This is only a pad. No copy is done.
      */
      iSetBitSize += bDataType - ABP_PAD0;
   }
   else
   {
      /*
      ** Separate offsets into octets and bits.
      */
      AddBitsToOctetSize( iSrcOctetOffset, iSrcBitOffset );
      AddBitsToOctetSize( iDestOctetOffset, iDestBitOffset );

      /*
      ** Calculate number of bits to be set.
      */
      if( bDataType == ABP_BOOL1 )
      {
         iSetBitSize += 1;
      }
      else
      {
         iSetBitSize += ( ( bDataType - ABP_BIT1 ) + 1 );
      }

      for( i = 0; i < iNumElem; i++ )
      {
         /*
         ** Calculate the number of octets that has to be copied
         ** to include both destination bit offset and bit size.
         */
         bCopySize = (UINT8)( ( iSetBitSize + iDestBitOffset + 7 ) / 8 );

         /*
         ** Copy parts to be manipulated into local 32 bit variables to
         ** guarantee correct alignment.
         */
         ABCC_PORT_CopyOctets( &lSrc, 0, pxSrc, iSrcOctetOffset,
                               ABP_UINT32_SIZEOF );
         ABCC_PORT_CopyOctets( &lDest, 0, pxDest, iDestOctetOffset, bCopySize );

         /*
         ** Bit data types crossing octet boundaries are always little endian.
         */
         lSrc = lLeTOl( lSrc );
         lDest = lLeTOl( lDest );

         /*
         ** Calculate bit mask and align it with destination bit offset.
         */
         lBitMask = ( (UINT32)1 << iSetBitSize ) - 1;
         lBitMask <<= iDestBitOffset;

         /*
         ** Align source bits with destination bits
         */
         if( iSrcBitOffset <  iDestBitOffset )
         {
            lSrc <<= iDestBitOffset - iSrcBitOffset;
         }
         else
         {
            lSrc >>= iSrcBitOffset - iDestBitOffset;
         }

         /*
         ** Clear destinations bits and mask source bits an insert source bits
         ** into destination bit position.
         */
         lDest &=  ~lBitMask;
         lSrc &=  lBitMask;
         lDest |= lSrc;

         /*
         ** Restore endian.
         */
         lDest = lTOlLe( lDest );

         /*
         ** Copy local updated data into final destination.
         */
         ABCC_PORT_CopyOctets( pxDest, iDestOctetOffset, &lDest, 0, bCopySize );

         /*
         ** Update bit offsets to next bit field.
         */
         iSrcBitOffset += iSetBitSize;
         AddBitsToOctetSize( iSrcOctetOffset, iSrcBitOffset );
         iDestBitOffset += iSetBitSize;
         AddBitsToOctetSize( iDestOctetOffset, iDestBitOffset );
      }
      iSetBitSize *= iNumElem;
   }
   return( iSetBitSize );
}

/*------------------------------------------------------------------------------
**  Copy value (single element or parts of an array) of a specific type
**  from a specified source to a destination. If the host platform endian
**  differs from network endian a swap will be done.
**  NOTE!! For all non-bit data types the source and destination must be octet
**  aligned.
**------------------------------------------------------------------------------
** Arguments:
**    pxDst             - Destination base pointer.
**    iDestBitOffset    - Bit offset relative destination pointer.
**    pxSrc             - Source base pointer.
**    iSrcBitOffset     - Bit offset relative source pointer.
**    bDataType         - Data type according to ABP_<X> types in abp.h
**    iNumElem          - Number of elements to copy.
**
** Returns:
**    Size of copied data in bits.
**------------------------------------------------------------------------------
*/
static UINT16 CopyValue( void* pxDst,
                         UINT16 iDestBitOffset,
                         const void* pxSrc,
                         UINT16 iSrcBitOffset,
                         UINT8 bDataType,
                         UINT16 iNumElem )
{
   UINT8 bDataTypeSizeInOctets;
   UINT16 iBitSetSize;

   if( Is_BITx_Or_PADx( bDataType ) || ( bDataType == ABP_BOOL1 ) )
   {
      iBitSetSize = CopyBitData( pxDst,
                                 iDestBitOffset,
                                 pxSrc,
                                 iSrcBitOffset,
                                 bDataType,
                                 iNumElem );
   }
   else
   {
      bDataTypeSizeInOctets = ABCC_GetDataTypeSize( bDataType );

      if( ad_fDoNetworkEndianSwap )
      {
         switch( bDataTypeSizeInOctets )
         {
         case 1:
            ABCC_PORT_CopyOctets( pxDst, BitToOctetOffset( iDestBitOffset ),
                                  pxSrc, BitToOctetOffset( iSrcBitOffset ),
                                  iNumElem );
            break;

         case 2:
            Copy16WithEndianSwap( pxDst, BitToOctetOffset( iDestBitOffset ),
                                  pxSrc, BitToOctetOffset( iSrcBitOffset ),
                                  iNumElem );
            break;

         case 4:
            Copy32WithEndianSwap( pxDst, BitToOctetOffset( iDestBitOffset ),
                                  pxSrc, BitToOctetOffset( iSrcBitOffset ),
                                  iNumElem );
            break;

#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED || ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
         case 8:
            Copy64WithEndianSwap( pxDst, BitToOctetOffset( iDestBitOffset ),
                                  pxSrc, BitToOctetOffset( iSrcBitOffset ),
                                  iNumElem );
            break;
#endif
         default:
            break;
         }
      }
      else
      {
         ABCC_PORT_CopyOctets( pxDst, BitToOctetOffset( iDestBitOffset ),
                               pxSrc, BitToOctetOffset( iSrcBitOffset ),
                               bDataTypeSizeInOctets * iNumElem );
      }

      iBitSetSize = ( iNumElem * bDataTypeSizeInOctets ) << 3;
   }

   return( iBitSetSize );
}

#if( AD_IA_MIN_MAX_DEFAULT_ENABLE )
/*------------------------------------------------------------------------------
** Get theoretical min and max properties for a data type.
**------------------------------------------------------------------------------
** Arguments:
**    bDataType         -  Data type
**
** Returns:
**    Pointer to union of all property types
**------------------------------------------------------------------------------
*/
const ad_AllPropertiesType* GetDefaultProperties( UINT8 bDataType )
{
   const ad_AllPropertiesType* puDataProp;

   static const AD_UINT8Type   ad_sBool1DefaultProp   = { { ABP_BOOL1_MIN, ABP_BOOL1_MAX, 0 } };
   static const AD_UINT32Type  ad_sUint32DefaultProp  = { { ABP_UINT32_MIN, ABP_UINT32_MAX, 0 } };
   static const AD_SINT32Type  ad_sSint32DefaultProp  = { { ABP_SINT32_MIN, ABP_SINT32_MAX, 0 } };
   static const AD_UINT16Type  ad_sUint16DefaultProp  = { { ABP_UINT16_MIN, ABP_UINT16_MAX, 0 } };
   static const AD_SINT16Type  ad_sSint16DefaultProp  = { { ABP_SINT16_MIN, ABP_SINT16_MAX, 0 } };
   static const AD_UINT8Type   ad_sUint8DefaultProp   = { { ABP_UINT8_MIN, ABP_UINT8_MAX, 0 } };
   static const AD_SINT8Type   ad_sSint8DefaultProp   = { { ABP_SINT8_MIN, ABP_SINT8_MAX, 0 } };
   static const AD_FLOAT32Type ad_sFloat32DefaultProp = { { -ABP_FLOAT_MAX, ABP_FLOAT_MAX, 0.0 } };
#if( ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
   static const AD_FLOAT64Type ad_sFloat64DefaultProp = { { -ABP_DOUBLE_MAX, ABP_DOUBLE_MAX, 0.0 } };
#endif
   static const AD_ENUMType    ad_sEnumDefaultProp    = { { ABP_ENUM_MIN, ABP_ENUM_MAX, 0 }, 0, NULL };
#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED )
   static const AD_UINT64Type  ad_lUint64DefaultProp  = { { ABP_UINT64_MIN, ABP_UINT64_MAX, 0 } };
   static const AD_SINT64Type  ad_lInt64DefaultProp   = { { ABP_SINT64_MIN, ABP_SINT64_MAX, 0 } };
#endif

   switch( bDataType )
   {
   case ABP_BOOL1:
      puDataProp = (const ad_AllPropertiesType*)&ad_sBool1DefaultProp;
      break;

   case ABP_ENUM:
      puDataProp = (const ad_AllPropertiesType*)&ad_sEnumDefaultProp;
      break;

   case ABP_BOOL:
   case ABP_UINT8:
   case ABP_OCTET:
   case ABP_CHAR:
   case ABP_BITS8:
      puDataProp = (const ad_AllPropertiesType*)&ad_sUint8DefaultProp;
      break;

   case ABP_SINT8:
      puDataProp = (const ad_AllPropertiesType*)&ad_sSint8DefaultProp;
      break;

   case ABP_UINT16:
   case ABP_BITS16:
      puDataProp = (const ad_AllPropertiesType*)&ad_sUint16DefaultProp;
      break;

   case ABP_SINT16:
      puDataProp = (const ad_AllPropertiesType*)&ad_sSint16DefaultProp;
      break;

   case ABP_UINT32:
   case ABP_BITS32:
      puDataProp = (const ad_AllPropertiesType*)&ad_sUint32DefaultProp;
      break;

   case ABP_SINT32:
      puDataProp = (const ad_AllPropertiesType*)&ad_sSint32DefaultProp;
      break;

   case ABP_FLOAT:
      puDataProp = (const ad_AllPropertiesType*)&ad_sFloat32DefaultProp;
      break;

#if( ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
   case ABP_DOUBLE:
      puDataProp = (const ad_AllPropertiesType*)&ad_sFloat64DefaultProp;
      break;

#endif
#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED )
   case ABP_SINT64:
      puDataProp = (const ad_AllPropertiesType*)&ad_lInt64DefaultProp;
      break;

   case ABP_UINT64:
      puDataProp = (const ad_AllPropertiesType*)&ad_lUint64DefaultProp;
      break;

#endif
   default:
      if( Is_BITx_Or_PADx( bDataType ) )
      {
         puDataProp = (const ad_AllPropertiesType*)&ad_sUint8DefaultProp;
      }
      else
      {
         ABCC_LOG_WARNING( ABCC_EC_UNSUPPORTED_DATA_TYPE,
            (UINT32)bDataType,
            "Unsupported data type (%" PRIu8 ")\n",
            bDataType );
         puDataProp = NULL;
      }
      break;
   }

   return( puDataProp );
}

/*------------------------------------------------------------------------------
**  Get min, max or default value of a single ADI element.
**  The value is converted to network endian.
**------------------------------------------------------------------------------
** Arguments:
**    psAdiEntry        - Entry of ADI
**    pxDest            - Pointer to destination
**    pxDest            - Destination bit offset
**    eMinMaxDefault    - Get min, max or default value described by
**                        AD_MinMaxDefaultIndexType
**    bDataType         - Data type
**
** Returns:
**    Size in bits of written values.
**------------------------------------------------------------------------------
*/
static UINT16 GetSingleMinMaxDefault( const ad_AllPropertiesType* puProps,
                                      void* pxDest,
                                      UINT16 iDestBitOffset,
                                      AD_MinMaxDefaultIndexType eMinMaxDefault,
                                      UINT8 bDataType )
{
   UINT16 iSrcOctetOffset;
   UINT16 iBitSize;

   iSrcOctetOffset = ABCC_GetDataTypeSize( bDataType );
#ifdef ABCC_SYS_16_BIT_CHAR
   if( iSrcOctetOffset == 1 )
   {
      iSrcOctetOffset = 2;
   }
#endif

   iSrcOctetOffset *= eMinMaxDefault;

   iBitSize = CopyValue( pxDest,
                         iDestBitOffset,
                         puProps,
                         iSrcOctetOffset * 8,
                         bDataType,
                         1 );

   return( iBitSize );
}

/*------------------------------------------------------------------------------
**  Range check of value
**------------------------------------------------------------------------------
** Arguments:
**    puValue           - Value to be checked
**    puProp            - Properties of the value (min, max)
**    bDataType         - Data type
**
** Returns:
**    ABP error code.
**------------------------------------------------------------------------------
*/
static UINT8 checkMinMax( ad_AllDataType* puValue,
                          ad_AllPropertiesType* puProp,
                          UINT8 bDataType )
{
   UINT8 bErrCode;

   bErrCode = ABP_ERR_NO_ERROR;

   if( MIN_MAX_DEFAULT_NOT_SUPPORTED( bDataType ) )
   {
      return( ABP_ERR_NO_ERROR );
   }

   switch( bDataType )
   {
   case ABP_BOOL:
   case ABP_UINT8:
   case ABP_ENUM:
   case ABP_OCTET:
#ifdef ABCC_SYS_16_BIT_CHAR
      /*
      ** Clear msb
      */
      puValue->bUnsigned &= 0x00ff;
#endif
      if( puValue->bUnsigned < puProp->sPropUint8.bMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->bUnsigned > puProp->sPropUint8.bMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

   case ABP_SINT8:
#ifdef ABCC_SYS_16_BIT_CHAR
      if( puValue->bSigned & 0x0080 )
      {
         /*
         ** Extend sign bit to msb
         */
         puValue->bSigned |= 0xff00;
      }
      else
      {
         /*
         ** Clear msb
         */
         puValue->bSigned &= 0x00ff;
      }
#endif
      if( puValue->bSigned < puProp->sPropInt8.bMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->bSigned > puProp->sPropInt8.bMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

   case ABP_UINT16:
      if( puValue->iUnsigned < puProp->sPropUint16.iMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->iUnsigned > puProp->sPropUint16.iMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

   case ABP_SINT16:
      if( puValue->iSigned < puProp->sPropInt16.iMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->iSigned > puProp->sPropInt16.iMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

   case ABP_UINT32:
      if( puValue->lUnsigned < puProp->sPropUint32.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->lUnsigned > puProp->sPropUint32.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

   case ABP_SINT32:
      if( puValue->lSigned < puProp->sPropInt32.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->lSigned > puProp->sPropInt32.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

   case ABP_FLOAT:
      if( puValue->rFloat < puProp->sPropFloat32.rMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->rFloat > puProp->sPropFloat32.rMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

#if( ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
   case ABP_DOUBLE:
      if( puValue->dDouble < puProp->sPropFloat64.dMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->dDouble > puProp->sPropFloat64.dMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

#endif
#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED )
   case ABP_UINT64:
      if( puValue->l64Unsigned < puProp->sPropUint64.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->l64Unsigned > puProp->sPropUint64.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

   case ABP_SINT64:
      if( puValue->l64Signed < puProp->sPropInt64.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->l64Signed > puProp->sPropInt64.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;

#endif
   case ABP_CHAR:
   case ABP_BITS8:
   case ABP_BITS16:
   case ABP_BITS32:
   case ABP_PAD0:
   case ABP_PAD1:
   case ABP_PAD2:
   case ABP_PAD3:
   case ABP_PAD4:
   case ABP_PAD5:
   case ABP_PAD6:
   case ABP_PAD7:
   case ABP_PAD8:
   case ABP_PAD9:
   case ABP_PAD10:
   case ABP_PAD11:
   case ABP_PAD12:
   case ABP_PAD13:
   case ABP_PAD14:
   case ABP_PAD15:
   case ABP_PAD16:
   case ABP_BOOL1:
   case ABP_BIT1:
   case ABP_BIT2:
   case ABP_BIT3:
   case ABP_BIT4:
   case ABP_BIT5:
   case ABP_BIT6:
   case ABP_BIT7:
      /*
      ** Valid type, but min/max check is not supported for type
      */
      break;

   default:
      ABCC_LOG_WARNING( ABCC_EC_UNSUPPORTED_DATA_TYPE,
         (UINT32)bDataType,
         "Unsupported data type (%" PRIu8 ")\n",
         bDataType );
      break;
   }

   return( bErrCode );
}

/*------------------------------------------------------------------------------
**  Range check of ADI.
**------------------------------------------------------------------------------
** Arguments:
**    psAdiEntry        - Entry of ADI
**    pxSrc             - Adi to be verified
**    iIndex            - Index to be checked (AD_ALL_ADI_INDEX to check the
**                        whole ADI)
**
** Returns:
**    ABP error code.
**------------------------------------------------------------------------------
*/
static UINT8 VerifyRange( const AD_AdiEntryType* psAdiEntry, void* pxSrc, UINT16 iIndex )
{
   ad_AllDataType uValue;
   UINT16 iSrcBitOffset;
   UINT8 bStartIndex;
   UINT8 bEndIndex;
   UINT8 i;
   UINT8 bErrCode;
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
   UINT8 j;
#endif

   iSrcBitOffset = 0;
   bErrCode = ABP_ERR_NO_ERROR;

   if( iIndex < 256 )
   {
      bStartIndex = (UINT8)iIndex;
      bEndIndex = bStartIndex + 1;
   }
   else
   {
      bStartIndex = 0;
      bEndIndex = psAdiEntry->bNumOfElements;
   }

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
   if( psAdiEntry->psStruct != NULL )
   {
      for( i = bStartIndex; i < bEndIndex; i++ )
      {
         if( psAdiEntry->psStruct[ i ].uData.sVOID.pxValueProps != NULL )
         {
            for( j = 0; j < psAdiEntry->psStruct[ i ].iNumSubElem; j++ )
            {
               iSrcBitOffset += CopyValue( &uValue,
                                           0,
                                           pxSrc,
                                           iSrcBitOffset,
                                           psAdiEntry->psStruct[ i ].bDataType,
                                           1 );

               bErrCode = checkMinMax( &uValue,
                                       psAdiEntry->psStruct[ i ].uData.sVOID.pxValueProps,
                                       psAdiEntry->psStruct[ i ].bDataType );

               if( bErrCode != ABP_ERR_NO_ERROR )
               {
                  break;
               }
            }

            if( bErrCode != ABP_ERR_NO_ERROR )
            {
               break;
            }
         }
         else
         {
            iSrcBitOffset += ABCC_GetDataTypeSizeInBits( psAdiEntry->psStruct[ i ].bDataType );
         }
      }
   }
   else
#endif
   {
      if( psAdiEntry->uData.sVOID.pxValueProps != NULL )
      {
         for( i = bStartIndex; i < bEndIndex; i++ )
         {
            iSrcBitOffset += CopyValue( &uValue,
                                        0,
                                        pxSrc,
                                        iSrcBitOffset,
                                        psAdiEntry->bDataType,
                                        1 );

            bErrCode = checkMinMax( &uValue,
                                    psAdiEntry->uData.sVOID.pxValueProps,
                                    psAdiEntry->bDataType );

            if( bErrCode != ABP_ERR_NO_ERROR )
            {
               break;
            }
         }
      }
   }

   if( ( bErrCode == ABP_ERR_VAL_TOO_LOW ) ||
       ( bErrCode == ABP_ERR_VAL_TOO_HIGH ) )
   {
      if( ( bEndIndex - bStartIndex ) > 1 )
      {
         /*
         ** Since we don't know if more elements are out of range no more
         ** specific error message is returned.
         */
         bErrCode = ABP_ERR_OUT_OF_RANGE;
      }
   }

   return( bErrCode );
}

/*------------------------------------------------------------------------------
**  Evaluate what property to use when reporting min/max/default.
**------------------------------------------------------------------------------
** Arguments:
**    puDataProp        - Pointer to user defined property
**    bDataType         - Data type
**
** Returns:
**    Pointer to valid property. NULL if not valid.
**------------------------------------------------------------------------------
*/
static const ad_AllPropertiesType* EvaluateProperties( const ad_AllPropertiesType* puDataProp,
                                                       UINT8                       bDataType )
{
   if( MIN_MAX_DEFAULT_NOT_SUPPORTED( bDataType ) )
   {
      return( NULL );
   }

   if( puDataProp == NULL )
   {
      return( GetDefaultProperties( bDataType ) );
   }
   else
   {
      return( puDataProp );
   }
}

/*------------------------------------------------------------------------------
**  Get min, max or default value(s) for an ADI. If the ADI is a structured ADI
**  all default values will have the same format as the structure describes.
**  The result is in network endian format.
**------------------------------------------------------------------------------
** Arguments:
**    psAdiEntry        - Pointer to ADI entry
**    pxDest            - Destination pointer
**    eMinMaxDefault    - Get min, max or default value described by
**                        AD_MinMaxDefaultIndexType
**    piOctetSize       - Size in octets of the written default values
**
** Returns:
**    ABP error code.
**------------------------------------------------------------------------------
*/
static UINT8 GetMinMaxDefault( const AD_AdiEntryType* psAdiEntry,
                               void* pxDest,
                               AD_MinMaxDefaultIndexType eMinMaxDefault,
                               UINT16* piBitSize )
{
   const ad_AllPropertiesType* puProp;

   *piBitSize = 0;

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
   if( psAdiEntry->psStruct != NULL )
   {
      UINT16 i;
      UINT16 j;

      for( i = 0; i < psAdiEntry->bNumOfElements; i++ )
      {
         puProp = EvaluateProperties( psAdiEntry->psStruct[ i ].uData.sVOID.pxValueProps,
                                      psAdiEntry->psStruct[ i ].bDataType );
         if( puProp == NULL )
         {
            *piBitSize = 0;
            return( ABP_ERR_INV_CMD_EXT_0 );
         }

         for( j = 0; j < psAdiEntry->psStruct[ i ].iNumSubElem; j++ )
         {
            *piBitSize += GetSingleMinMaxDefault( puProp,
                                                  pxDest,
                                                  *piBitSize,
                                                  eMinMaxDefault,
                                                  psAdiEntry->psStruct[ i ].bDataType );
         }
      }
   }
   else
#endif
   {
      puProp = EvaluateProperties( psAdiEntry->uData.sVOID.pxValueProps,
                                   psAdiEntry->bDataType );
      if( puProp == NULL )
      {
         *piBitSize = 0;
         return( ABP_ERR_INV_CMD_EXT_0 );
      }
      else
      {
         *piBitSize += GetSingleMinMaxDefault( puProp,
                                               pxDest,
                                               *piBitSize,
                                               eMinMaxDefault,
                                               psAdiEntry->bDataType );
      }
   }
   return( ABP_ERR_NO_ERROR );
}
#endif

/*------------------------------------------------------------------------------
**  Set ADI of any data type. The provided data must have network endian format.
**------------------------------------------------------------------------------
** Arguments:
**    psAdiEntry        - Pointer to ADI entry.
**    pxData            - Source base pointer.
**    bNumElements      - Number of elements to write.
**    bStartIndex       - Index to first element to write.
**    piSrcBitOffset    - Pointer to source bit offset.
**                        This offset will be incremented according to the size
**                        written.
**    fExplicit         - Indicates whether the set request originated from an
**                        explicit set request or a read process data request
** Returns:
**    None
**------------------------------------------------------------------------------
*/
static void SetAdiValue( const AD_AdiEntryType* psAdiEntry,
                         void* pxData,
                         UINT8 bNumElements,
                         UINT8 bStartIndex,
                         UINT16* piSrcBitOffset,
                         BOOL fExplicit )
{
   UINT16 iDestBitOffset;

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
   if( psAdiEntry->psStruct != NULL )
   {
      UINT16 i;

      /*
      ** For structures each element is handled separately.
      */
      for( i = bStartIndex; i < bNumElements + bStartIndex; i++ )
      {
         if( fExplicit )
         {
            if( !( psAdiEntry->psStruct[ i ].bDesc &
                   ABP_APPD_DESCR_SET_ACCESS ) )
            {
               *piSrcBitOffset += ( ABCC_GetDataTypeSizeInBits( psAdiEntry->psStruct[ i ].bDataType ) *
                                   psAdiEntry->psStruct[ i ].iNumSubElem );
               continue;
            }
         }
         iDestBitOffset = psAdiEntry->psStruct[ i ].bBitOffset;
         *piSrcBitOffset += CopyValue( psAdiEntry->psStruct[ i ].uData.sVOID.pxValuePtr,
                                       iDestBitOffset,
                                       pxData,
                                       *piSrcBitOffset,
                                       psAdiEntry->psStruct[ i ].bDataType,
                                       psAdiEntry->psStruct[ i ].iNumSubElem );
      }
   }
   else
#else
   (void)fExplicit;
#endif
   {
      iDestBitOffset = CalcStartIndexBitOffset( psAdiEntry->bDataType, bStartIndex );
      *piSrcBitOffset += CopyValue( psAdiEntry->uData.sVOID.pxValuePtr,
                                    iDestBitOffset,
                                    pxData,
                                    *piSrcBitOffset,
                                    psAdiEntry->bDataType,
                                    bNumElements );
   }
#if( ABCC_CFG_ADI_GET_SET_CALLBACK_ENABLED )
   if( psAdiEntry->pnSetAdiValue != NULL )
   {
      /*
      ** If a set callback is registered the user is notified that the ADI is
      ** updated.
      */
      psAdiEntry->pnSetAdiValue( psAdiEntry,
                                 bNumElements,
                                 bStartIndex );
   }
#endif
}

/*------------------------------------------------------------------------------
** Write to a buffer using data from a PD map.
**------------------------------------------------------------------------------
** Arguments:
**    pxDstPdDataBuf - Destination data buffer.
**    piPdBitOffset  - Pointer to bit offset relative pxPdDataBuf.
**                     This offset will be incremented according to the size
**                     written to the buffer.
**    pasPdMap       - Pointer to PD map.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
static void WriteBufferFromPdMap( void* pxDstPdDataBuf,
                                  UINT16* piPdBitOffset,
                                  ad_MapInfoType* pasPdMap )
{
   UINT16 iIndex;
   const ad_MapType* paiPdMap = pasPdMap->paiMappedAdiList;

   if( paiPdMap )
   {
      for( iIndex = 0; iIndex < pasPdMap->iNumMappedAdi; iIndex++ )
      {
         if( paiPdMap->iAdiIndex != AD_MAP_PAD_INDEX )
         {
            if( paiPdMap->iAdiIndex >= ad_iNumOfADIs )
            {
               /*
               ** Pull the plug! The data in these tables should already have
               ** been checked and should be OK!
               */
               ABCC_LOG_FATAL( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
                  (UINT32)paiPdMap->iAdiIndex,
                  "Error in PD map configuration, ADI index out of range (%" PRIu16 ")\n",
                  paiPdMap->iAdiIndex );
            }
            AD_GetAdiValue( &ad_asADIEntryList[ paiPdMap->iAdiIndex ],
                            pxDstPdDataBuf,
                            paiPdMap->bNumElements,
                            paiPdMap->bStartIndex,
                            piPdBitOffset,
                            FALSE );
         }
         else
         {
            *piPdBitOffset += paiPdMap->bNumElements;
         }

         paiPdMap++;
      }
   }
}

/*------------------------------------------------------------------------------
** Write to a PD map using data from a buffer.
**------------------------------------------------------------------------------
** Arguments:
**    pasPdMap       - Pointer to PD map.
**    pxSrcPdDataBuf - Source data buffer.
**    piPdBitOffset  - Pointer to bit offset relative pxPdDataBuf.
**                     This offset will be incremented according to the size
**                     read from the buffer.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
static void WritePdMapFromBuffer( const ad_MapInfoType* pasPdMap,
                                  void* pxSrcPdDataBuf,
                                  UINT16* piPdBitOffset )
{
   UINT16 iIndex;
   const ad_MapType* paiPdMap = pasPdMap->paiMappedAdiList;

   if( paiPdMap )
   {
      for( iIndex = 0; iIndex < pasPdMap->iNumMappedAdi; iIndex++ )
      {
         if( paiPdMap->iAdiIndex != AD_MAP_PAD_INDEX )
         {
            if( paiPdMap->iAdiIndex >= ad_iNumOfADIs )
            {
               /*
               ** Pull the plug! The data in these tables should already have
               ** been checked and should be OK!
               */
               ABCC_LOG_FATAL( ABCC_EC_ERROR_IN_READ_MAP_CONFIG,
                  (UINT32)paiPdMap->iAdiIndex,
                  "Error in read map configuration, ADI index out of range (%" PRIu16 ")\n",
                  paiPdMap->iAdiIndex );
            }
            SetAdiValue( &ad_asADIEntryList[ paiPdMap->iAdiIndex ],
                         pxSrcPdDataBuf,
                         paiPdMap->bNumElements,
                         paiPdMap->bStartIndex,
                         piPdBitOffset,
                         FALSE );
         }
         else
         {
            *piPdBitOffset += paiPdMap->bNumElements;
         }

         paiPdMap++;
      }
   }
}
EXTFUNC ABCC_ErrorCodeType AD_Init( const AD_AdiEntryType* psAdiEntry,
                                  UINT16 iNumAdi,
                                  const AD_MapType* psDefaultMap )
{
   UINT16 iMapIndex = 0;
   UINT16 iAdiIndex = 0;
   UINT8 bNumElem;
   UINT8 bElemStartIndex;
   /*
   ** In this context we should initialize the AD object to be prepared for
   ** startup.
   */
   ad_asADIEntryList = psAdiEntry;
   ad_asDefaultMap = psDefaultMap;

   ad_iNumOfADIs =  iNumAdi;
   ad_iHighestInstanceNumber = 0;

   ad_ReadMapInfo.paiMappedAdiList = ad_PdReadMapping;
   ad_ReadMapInfo.iPdSize = 0;
   ad_ReadMapInfo.iNumMappedAdi = 0;
   ad_ReadMapInfo.iMaxNumMappedAdi = AD_MAX_NUM_READ_MAP_ENTRIES;

   ad_WriteMapInfo.paiMappedAdiList = ad_PdWriteMapping;
   ad_WriteMapInfo.iPdSize = 0;
   ad_WriteMapInfo.iNumMappedAdi = 0;
   ad_WriteMapInfo.iMaxNumMappedAdi = AD_MAX_NUM_WRITE_MAP_ENTRIES;

   if( ad_asDefaultMap != NULL )
   {
      while( ad_asDefaultMap[ iMapIndex ].eDir != PD_END_MAP )
      {
         iAdiIndex = GetAdiIndex( ad_asDefaultMap[ iMapIndex ].iInstance );

         if( iAdiIndex == AD_INVALID_ADI_INDEX )
         {
            ABCC_LOG_ERROR( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
               ad_asDefaultMap[ iMapIndex ].iInstance,
               "Requested ADI could not be found (%" PRIu16 ")\n",
               ad_asDefaultMap[ iMapIndex ].iInstance );

            return( ABCC_EC_ERROR_IN_PD_MAP_CONFIG );
         }

         bNumElem = ad_asDefaultMap[ iMapIndex ].bNumElem;
         bElemStartIndex = ad_asDefaultMap[ iMapIndex ].bElemStartIndex;

         if( iAdiIndex != AD_MAP_PAD_INDEX )
         {
            if( ad_asDefaultMap[ iMapIndex ].bNumElem == AD_MAP_ALL_ELEM )
            {
               bNumElem = ad_asADIEntryList[ iAdiIndex ].bNumOfElements;
               bElemStartIndex = 0;
            }
         }

         if( ad_asDefaultMap[ iMapIndex ].eDir == PD_READ )
         {
            if( ad_ReadMapInfo.iNumMappedAdi >= ad_ReadMapInfo.iMaxNumMappedAdi )
            {
               ABCC_LOG_ERROR( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
                  ad_ReadMapInfo.iNumMappedAdi,
                  "Too many read mappings. Max: %" PRIu16 "\n",
                  ad_ReadMapInfo.iMaxNumMappedAdi );

               return( ABCC_EC_ERROR_IN_PD_MAP_CONFIG );
            }

            ad_ReadMapInfo.paiMappedAdiList[ ad_ReadMapInfo.iNumMappedAdi ].bNumElements = bNumElem;
            ad_ReadMapInfo.paiMappedAdiList[ ad_ReadMapInfo.iNumMappedAdi ].bStartIndex = bElemStartIndex;
            ad_ReadMapInfo.paiMappedAdiList[ ad_ReadMapInfo.iNumMappedAdi ].iAdiIndex = iAdiIndex;
            ad_ReadMapInfo.iNumMappedAdi++;
         }
         else
         {
            if( ad_WriteMapInfo.iNumMappedAdi >= ad_WriteMapInfo.iMaxNumMappedAdi )
            {
               ABCC_LOG_ERROR( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
                  ad_WriteMapInfo.iNumMappedAdi,
                  "Too many write mappings. Max: %" PRIu16 "\n",
                  ad_WriteMapInfo.iMaxNumMappedAdi );

               return( ABCC_EC_ERROR_IN_PD_MAP_CONFIG );
            }

            ad_WriteMapInfo.paiMappedAdiList[ ad_WriteMapInfo.iNumMappedAdi ].bNumElements = bNumElem;
            ad_WriteMapInfo.paiMappedAdiList[ ad_WriteMapInfo.iNumMappedAdi ].bStartIndex = bElemStartIndex;
            ad_WriteMapInfo.paiMappedAdiList[ ad_WriteMapInfo.iNumMappedAdi ].iAdiIndex = iAdiIndex;
            ad_WriteMapInfo.iNumMappedAdi++;
         }
         iMapIndex++;
      }
   }

   UpdateMapSize( &ad_WriteMapInfo );
   UpdateMapSize( &ad_ReadMapInfo );

   if( ad_ReadMapInfo.iPdSize > ABCC_CFG_MAX_PROCESS_DATA_SIZE )
   {
      ABCC_LOG_ERROR( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
         ad_ReadMapInfo.iPdSize,
         "Read map size too big. Max: %d Actual: %" PRIu16 "\n",
         ABCC_CFG_MAX_PROCESS_DATA_SIZE,
         ad_ReadMapInfo.iPdSize );

      return( ABCC_EC_ERROR_IN_PD_MAP_CONFIG );
   }

   if( ad_WriteMapInfo.iPdSize > ABCC_CFG_MAX_PROCESS_DATA_SIZE )
   {
      ABCC_LOG_ERROR( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
         ad_WriteMapInfo.iPdSize,
         "Write map size too big. Max: %d Actual: %" PRIu16 "\n",
         ABCC_CFG_MAX_PROCESS_DATA_SIZE,
         ad_WriteMapInfo.iPdSize );

      return( ABCC_EC_ERROR_IN_PD_MAP_CONFIG );
   }

   for( iAdiIndex = 0; iAdiIndex < ad_iNumOfADIs; iAdiIndex++ )
   {
      if( ad_asADIEntryList[ iAdiIndex ].iInstance > ad_iHighestInstanceNumber )
      {
         ad_iHighestInstanceNumber = ad_asADIEntryList[ iAdiIndex ].iInstance;
      }
   }

   return( ABCC_EC_NO_ERROR );
}

const AD_AdiEntryType* AD_GetAdiInstEntry( UINT16 iInstance )
{
   UINT16 iIndex;
   const AD_AdiEntryType* psEntry = NULL;

   iIndex = GetAdiIndex( iInstance );

   if( ( iIndex != AD_INVALID_ADI_INDEX ) &&
       ( iIndex != AD_MAP_PAD_INDEX ) )
   {
      psEntry = &ad_asADIEntryList[ iIndex ];
   }

   return( psEntry );
}

UINT16 AD_GetMapSizeOctets( const AD_MapType* pasMap )
{
   UINT8 bNumElements;
   UINT8 bStartIndex;
   UINT16 iSize;
   UINT16 iAdiIndex;

   iSize = 0;

   if( pasMap != NULL )
   {
      while( pasMap->eDir != PD_END_MAP )
      {
         iAdiIndex = GetAdiIndex( pasMap->iInstance );

         if( iAdiIndex == AD_INVALID_ADI_INDEX )
         {
            ABCC_LOG_WARNING( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
               pasMap->iInstance,
               "Requested ADI could not be found (%" PRIu16 ")\n",
               pasMap->iInstance );

            return( 0 );
         }
         else if( iAdiIndex == AD_MAP_PAD_INDEX )
         {
            iSize += pasMap->bNumElem;
         }
         else
         {
            if( pasMap->bNumElem == AD_MAP_ALL_ELEM )
            {
               /*
               ** Convert internal representation for all elements to the
               ** actual number of elements.
               */
               bNumElements = ad_asADIEntryList[ iAdiIndex ].bNumOfElements;
               bStartIndex = 0;
            }
            else
            {
               bNumElements = pasMap->bNumElem;
               bStartIndex = pasMap->bElemStartIndex;
            }

            iSize += GetAdiSizeInBits( &ad_asADIEntryList[ iAdiIndex ],
                                       bNumElements,
                                       bStartIndex );
         }

         pasMap++;
      }
   }

   return( SizeInOctets( 0, iSize ) );
}

UINT16 AD_GetNumAdisInMap( const AD_MapType* pasMap )
{
   UINT16 iNumAdis;

   iNumAdis = 0;

   while( pasMap->eDir != PD_END_MAP )
   {
      iNumAdis++;
      pasMap++;
   }

   return( iNumAdis );
}

void AD_ProcObjectRequest( ABP_MsgType* psMsgBuffer )
{
   const AD_AdiEntryType* psAdiEntry;
   UINT16 iMsgBitOffset;
   UINT16 iItemSize;
   UINT16 iTemp;
   UINT16 iDataSize;
   UINT8  bErrCode;

   iMsgBitOffset = 0;
   iDataSize = 0;
   bErrCode = ABP_ERR_NO_ERROR;

   if( iLeTOi( psMsgBuffer->sHeader.iInstance ) == ABP_INST_OBJ )
   {
      /*
      ** A request to the object instance.
      */
      switch( ABCC_GetMsgCmdBits( psMsgBuffer ) )
      {
      case ABP_CMD_GET_ATTR:
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
         case ABP_OA_NAME:
            ABCC_SetMsgString( psMsgBuffer, "Application data", 16, 0 );
            iDataSize = 16;
            break;

         case ABP_OA_REV:
            ABCC_SetMsgData8( psMsgBuffer, AD_OA_REV_VALUE, 0 );
            iDataSize = ABP_OA_REV_DS;
            break;

         case ABP_OA_NUM_INST:
            ABCC_SetMsgData16( psMsgBuffer, ad_iNumOfADIs, 0 );
            iDataSize = ABP_OA_NUM_INST_DS;
            break;

         case ABP_OA_HIGHEST_INST:
            ABCC_SetMsgData16( psMsgBuffer, ad_iHighestInstanceNumber, 0 );
            iDataSize = ABP_OA_HIGHEST_INST_DS;
            break;

         case ABP_APPD_OA_NR_READ_PD_MAPPABLE_INSTANCES:
            {
               UINT16 iIndex;
               UINT16 iCnt = 0;

               for( iIndex = 0; iIndex < ad_iNumOfADIs; iIndex++ )
               {
                  if( ad_asADIEntryList[iIndex ].bDesc & ABP_APPD_DESCR_MAPPABLE_READ_PD )
                  {
                     iCnt++;
                  }
               }
               ABCC_SetMsgData16( psMsgBuffer, iCnt, 0 );
               iDataSize = ABP_UINT16_SIZEOF;
            }
            break;

         case ABP_APPD_OA_NR_WRITE_PD_MAPPABLE_INSTANCES:
            {
               UINT16 iIndex;
               UINT16 iCnt = 0;

               for( iIndex = 0; iIndex < ad_iNumOfADIs; iIndex++ )
               {
                  if( ad_asADIEntryList[ iIndex ].bDesc & ABP_APPD_DESCR_MAPPABLE_WRITE_PD )
                  {
                     iCnt++;
                  }
               }
               ABCC_SetMsgData16( psMsgBuffer, iCnt, 0 );
               iDataSize = ABP_UINT16_SIZEOF;
            }
            break;

         case ABP_APPD_OA_NR_NV_INSTANCES:
            {
               UINT16 iIndex;
               UINT16 iCnt = 0;

               for( iIndex = 0; iIndex < ad_iNumOfADIs; iIndex++ )
               {
                  if( ad_asADIEntryList[ iIndex ].bDesc & ABP_APPD_DESCR_NVS_PARAMETER )
                  {
                     iCnt++;
                  }
               }
               ABCC_SetMsgData16( psMsgBuffer, iCnt, 0 );
               iDataSize = ABP_UINT16_SIZEOF;
            }
            break;

         default:
            /*
            ** Unsupported attribute.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
            break;
         }
         break;

      case ABP_APPD_CMD_GET_INST_BY_ORDER:
         iTemp = ABCC_GetMsgCmdExt( psMsgBuffer );

         if( ( iTemp == 0 ) ||
             ( iTemp > ad_iNumOfADIs ) )
         {
            /*
            ** Requested order number does not exist.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
         }
         else
         {
            ABCC_SetMsgData16( psMsgBuffer,
                               ad_asADIEntryList[ iTemp - 1 ].iInstance,
                               0 );
            iDataSize = ABP_UINT16_SIZEOF;
         }
         break;

#if( ABCC_CFG_REMAP_SUPPORT_ENABLED )
      case ABP_APPD_REMAP_ADI_WRITE_AREA:
         RemapProcessDataCommand( psMsgBuffer, &ad_WriteMapInfo );
         psMsgBuffer = NULL;
         break;

      case ABP_APPD_REMAP_ADI_READ_AREA:
         RemapProcessDataCommand( psMsgBuffer, &ad_ReadMapInfo );
         psMsgBuffer = NULL;
         break;
#endif
      case ABP_APPD_GET_INSTANCE_NUMBERS:
      {
         UINT16 iStartingOrder;
         UINT16 iReqInstances;
         UINT16 iAdiIndex;
         UINT16 iLocalOrder;
         UINT8  bDescrMask = 0;

         if( ABCC_GetMsgCmdExt0( psMsgBuffer ) != 0 )
         {
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
            break;
         }

         switch( ABCC_GetMsgCmdExt1( psMsgBuffer ) )
         {
         case ABP_APPD_LIST_TYPE_ALL:
            break;

         case ABP_APPD_LIST_TYPE_RD_PD_MAPPABLE:
            bDescrMask = ABP_APPD_DESCR_MAPPABLE_READ_PD;
            break;

         case ABP_APPD_LIST_TYPE_WR_PD_MAPPABLE:
            bDescrMask = ABP_APPD_DESCR_MAPPABLE_WRITE_PD;
            break;

         case ABP_APPD_LIST_TYPE_NVS_PARAMS:
            bDescrMask = ABP_APPD_DESCR_NVS_PARAMETER;
            break;

         default:
            bErrCode = ABP_ERR_INV_CMD_EXT_1;
            break;
         }
         if( bErrCode != ABP_ERR_NO_ERROR )
         {
            break;
         }

         ABCC_GetMsgData16( psMsgBuffer, &iStartingOrder, 0 );
         if( iStartingOrder < 1 )
         {
            bErrCode = ABP_ERR_OUT_OF_RANGE;
            break;
         }

         ABCC_GetMsgData16( psMsgBuffer, &iReqInstances, 2 );
         if( iReqInstances < 1 )
         {
            bErrCode = ABP_ERR_OUT_OF_RANGE;
            break;
         }

         iDataSize = 0;
         iLocalOrder = 0;
         for( iAdiIndex = 0; iAdiIndex < ad_iNumOfADIs; iAdiIndex++ )
         {
            if( ( bDescrMask == 0 ) || ( ad_asADIEntryList[ iAdiIndex ].bDesc & bDescrMask ) )
            {
               iLocalOrder++;
               if( iLocalOrder >= iStartingOrder )
               {
                  if( ( iDataSize + ABP_UINT16_SIZEOF ) > ABCC_GetMaxMessageSize() )
                  {
                     bErrCode = ABP_ERR_OUT_OF_RANGE;
                     break;
                  }
                  ABCC_SetMsgData16( psMsgBuffer, ad_asADIEntryList[ iAdiIndex ].iInstance, iDataSize );
                  iDataSize += ABP_UINT16_SIZEOF;
                  iReqInstances--;
                  if( iReqInstances == 0 )
                  {
                     break;
                  }
               }
            }
         }
         break;
      }

      default:
         bErrCode = ABP_ERR_UNSUP_CMD;
         break;
      }
   }
   else if( ( psAdiEntry = AD_GetAdiInstEntry( ABCC_GetMsgInstance( psMsgBuffer ) ) ) != NULL )
   {
      /*
      ** The ADI instance was found. Now switch on command.
      */
      switch( ABCC_GetMsgCmdBits( psMsgBuffer ) )
      {
      case ABP_CMD_GET_ATTR:
         /*
         ** Switch on attribute.
         */
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
         case ABP_APPD_IA_NAME:
            if( psAdiEntry->pacName )
            {
               iDataSize = (UINT16)strlen( psAdiEntry->pacName );
               ABCC_SetMsgString( psMsgBuffer,
                                  psAdiEntry->pacName, iDataSize, 0 );
            }
            else
            {
               iDataSize = 0;
            }
            break;

         case ABP_APPD_IA_DATA_TYPE:
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
            if( psAdiEntry->psStruct != NULL )
            {
               UINT16 i;

               iDataSize = ABP_APPD_IA_DATA_TYPE_DS*psAdiEntry->bNumOfElements;
               for( i = 0; i < psAdiEntry->bNumOfElements; i++ )
               {
                  ABCC_SetMsgData8( psMsgBuffer,
                                    psAdiEntry->psStruct[i].bDataType, i );
               }
            }
            else
#endif
            {
               ABCC_SetMsgData8( psMsgBuffer, psAdiEntry->bDataType, 0 );
               iDataSize = ABP_APPD_IA_DATA_TYPE_DS;
            }
            break;

         case ABP_APPD_IA_NUM_ELEM:
            ABCC_SetMsgData8( psMsgBuffer, psAdiEntry->bNumOfElements, 0 );
            iDataSize = ABP_APPD_IA_NUM_ELEM_DS;
            break;

         case ABP_APPD_IA_DESCRIPTOR:
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
            if( psAdiEntry->psStruct != NULL )
            {
               UINT16 i;

               iDataSize = ABP_APPD_IA_DESCRIPTOR_DS * psAdiEntry->bNumOfElements;
               for( i = 0; i < psAdiEntry->bNumOfElements; i++ )
               {
                  ABCC_SetMsgData8( psMsgBuffer,
                                    psAdiEntry->psStruct[i].bDesc, i );
               }
            }
            else
#endif
            {
               ABCC_SetMsgData8( psMsgBuffer, psAdiEntry->bDesc, 0 );
               iDataSize = ABP_APPD_IA_DESCRIPTOR_DS;
            }
            break;

         case ABP_APPD_IA_VALUE: /* Value. */
            if( !( psAdiEntry->bDesc & ABP_APPD_DESCR_GET_ACCESS ) )
            {
               bErrCode = ABP_ERR_ATTR_NOT_GETABLE;
               break;
            }
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
            else if( ( psAdiEntry->psStruct != NULL ) &&
                       !( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].bDesc &
                                                ABP_APPD_DESCR_GET_ACCESS ) )
            {
               bErrCode = ABP_ERR_ATTR_NOT_GETABLE;
               break;
            }
#endif
            ABCC_GetMsgDataPtr( psMsgBuffer )[ 0 ] = 0;

            AD_GetAdiValue( psAdiEntry, ABCC_GetMsgDataPtr( psMsgBuffer ),
                            psAdiEntry->bNumOfElements, 0,
                            &iMsgBitOffset, TRUE );
            iDataSize = SizeInOctets( 0, iMsgBitOffset );
            break;
#if( AD_IA_MIN_MAX_DEFAULT_ENABLE )
         case ABP_APPD_IA_MAX_VALUE:
            bErrCode = GetMinMaxDefault( psAdiEntry,
                                         ABCC_GetMsgDataPtr( psMsgBuffer ),
                                         AD_MAX_VALUE_INDEX,
                                         &iDataSize );
            iDataSize = SizeInOctets( 0, iDataSize );
            break;
         case ABP_APPD_IA_MIN_VALUE:
            bErrCode = GetMinMaxDefault( psAdiEntry,
                                         ABCC_GetMsgDataPtr( psMsgBuffer ),
                                         AD_MIN_VALUE_INDEX,
                                         &iDataSize );
            iDataSize = SizeInOctets( 0, iDataSize );
            break;
         case ABP_APPD_IA_DFLT_VALUE:
            bErrCode = GetMinMaxDefault( psAdiEntry,
                                         ABCC_GetMsgDataPtr( psMsgBuffer ),
                                         AD_DEFAULT_VALUE_INDEX,
                                         &iDataSize );
            iDataSize = SizeInOctets( 0, iDataSize );
            break;
#endif
         case ABP_APPD_IA_NUM_SUB_ELEM:
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
            if( psAdiEntry->psStruct != NULL )
            {
               UINT16 i;

               iDataSize = ABP_APPD_IA_NUM_SUB_ELEM_DS * psAdiEntry->bNumOfElements;
               for( i = 0; i < psAdiEntry->bNumOfElements; i++ )
               {
                  ABCC_SetMsgData16( psMsgBuffer,
                                     psAdiEntry->psStruct[i].iNumSubElem,
                                     ( i * ABP_APPD_IA_NUM_SUB_ELEM_DS ) );
               }
            }
            else
#endif
            {
               bErrCode = ABP_ERR_INV_CMD_EXT_0;
            }
            break;

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
         case ABP_APPD_IA_ELEM_NAME:
            if( psAdiEntry->psStruct != NULL )
            {
               UINT16 i;

               for( i = 0; i < psAdiEntry->bNumOfElements; i++ )
               {
                  if( psAdiEntry->psStruct[ i ].pacElementName != NULL )
                  {
                     ABCC_SetMsgString( psMsgBuffer,
                                        psAdiEntry->psStruct[ i ].pacElementName,
                                        (UINT16)strlen( psAdiEntry->psStruct[ i ].pacElementName ),
                                        iDataSize );
                     iDataSize += (UINT16)strlen( psAdiEntry->psStruct[ i ].pacElementName );
                  }
                  else
                  {
                     /*
                     ** There is an element name wit the value NULL. This
                     ** invalidates the element names of all sub-elements. This
                     ** is to differentiate between the empty string "" and
                     ** NULL.
                     */
                     bErrCode = ABP_ERR_INV_CMD_EXT_0;
                     break;
                  }

                  if( i < ( psAdiEntry->bNumOfElements - 1 ) )
                  {
                     ABCC_SetMsgData8( psMsgBuffer, 0, iDataSize );
                     iDataSize += ABP_CHAR_SIZEOF;
                  }
               }
            }
            else
            {
               bErrCode = ABP_ERR_INV_CMD_EXT_0;
            }

            break;
#endif

         default:
            /*
            ** Unsupported attribute.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
            break;
         }
         break;
      case ABP_CMD_SET_ATTR:
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
         case ABP_APPD_IA_NAME:
         case ABP_APPD_IA_DATA_TYPE:
         case ABP_APPD_IA_NUM_ELEM:
         case ABP_APPD_IA_DESCRIPTOR:
         case ABP_APPD_IA_ELEM_NAME:
            /*
            ** Attributes are not settable.
            */
            bErrCode = ABP_ERR_ATTR_NOT_SETABLE;
            break;

         case ABP_APPD_IA_VALUE:

            if( !( psAdiEntry->bDesc & ABP_APPD_DESCR_SET_ACCESS ) )
            {
               bErrCode = ABP_ERR_ATTR_NOT_SETABLE;
               break;
            }

            /*
            ** Check the length of each array.
            */
            iItemSize = GetAdiSizeInOctets( psAdiEntry );
            if( iLeTOi( psMsgBuffer->sHeader.iDataSize ) > iItemSize )
            {
               bErrCode = ABP_ERR_TOO_MUCH_DATA;
               break;
            }
            else if( iLeTOi( psMsgBuffer->sHeader.iDataSize ) < iItemSize )
            {
               bErrCode = ABP_ERR_NOT_ENOUGH_DATA;
               break;
            }
#if( AD_IA_MIN_MAX_DEFAULT_ENABLE )
            bErrCode = VerifyRange( psAdiEntry, ABCC_GetMsgDataPtr( psMsgBuffer ),
                                    AD_ALL_ADI_INDEX );
#endif

            if( bErrCode == ABP_ERR_NO_ERROR )
            {
#if( ABCC_CFG_ADI_TRANS_SET_CALLBACK_ENABLED )
               if( psAdiEntry->pnSetAdiValueTransparent != NULL )
               {
                  bErrCode = psAdiEntry->pnSetAdiValueTransparent( psAdiEntry,
                                                                   psAdiEntry->bNumOfElements,
                                                                   0,
                                                                   iLeTOi( psMsgBuffer->sHeader.iDataSize ),
                                                                   ABCC_GetMsgDataPtr( psMsgBuffer ) );
               }
               else
#endif
               {
                  SetAdiValue( psAdiEntry, ABCC_GetMsgDataPtr( psMsgBuffer ),
                               psAdiEntry->bNumOfElements, 0,
                               &iMsgBitOffset, TRUE );
                  /*
                  ** Success.
                  */
                  iDataSize = 0;
               }
            }

            break;

         default:
            /*
            ** Unsupported attribute.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
            break;
         }
         break;

      case ABP_CMD_GET_ENUM_STR:

         if( ( ABCC_GetMsgCmdExt0( psMsgBuffer ) == ABP_APPD_IA_VALUE ) &&
             ( psAdiEntry->bDataType == ABP_ENUM ) )
         {
            if( ( psAdiEntry->uData.sENUM.psValueProps == NULL ) ||
                ( psAdiEntry->uData.sENUM.psValueProps->pasEnumStrings == NULL ) )
            {
               /*
               ** The application has not defined properties or strings for
               ** this ABP_ENUM ADI, and the ABP_ENUM data type is quite
               ** pointless without those. Notify the user.
               */
               ABCC_LOG_WARNING( ABCC_EC_NO_RESOURCES,
                  (UINT32)psAdiEntry->iInstance,
                  "Properties not defined for ENUM ADI (instance %" PRIu16 ")\n",
                  psAdiEntry->iInstance );
               bErrCode = ABP_ERR_NO_RESOURCES;
            }
            else
            {
               UINT8 bStringIndex;

               for( bStringIndex = 0; bStringIndex < psAdiEntry->uData.sENUM.psValueProps->bNumOfEnumStrings; bStringIndex++ )
               {
                  if( psAdiEntry->uData.sENUM.psValueProps->pasEnumStrings[ bStringIndex ].eValue ==
                      ABCC_GetMsgCmdExt1( psMsgBuffer ) )
                  {
                     break;
                  }
               }

               if( bStringIndex < psAdiEntry->uData.sENUM.psValueProps->bNumOfEnumStrings )
               {
                  iDataSize = (UINT16)strlen( psAdiEntry->uData.sENUM.psValueProps->pasEnumStrings[ bStringIndex ].acEnumStr );
                  ABCC_SetMsgString( psMsgBuffer,
                                     psAdiEntry->uData.sENUM.psValueProps->pasEnumStrings[ bStringIndex ].acEnumStr,
                                     iDataSize, 0 );
               }
               else
               {
                  /*
                  ** The enum value was not found in the string lookup.
                  */
                  bErrCode = ABP_ERR_INV_CMD_EXT_1;
               }
            }
         }
         else
         {
            /*
            ** The attribute targeted by CmdExt0 does not exist, or is not an
            ** ABP_ENUM.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
         }
         break;

      case ABP_CMD_GET_INDEXED_ATTR:
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
            case ABP_APPD_IA_VALUE:
               if( !( psAdiEntry->bDesc & ABP_APPD_DESCR_GET_ACCESS ) )
               {
                  bErrCode = ABP_ERR_ATTR_NOT_GETABLE;
                  break;
               }
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
               else if( ( psAdiEntry->psStruct != NULL ) &&
                          !( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].bDesc &
                                                   ABP_APPD_DESCR_GET_ACCESS ) )
               {
                  bErrCode = ABP_ERR_ATTR_NOT_GETABLE;
                  break;
               }
#endif
               else
               {
                  AD_GetAdiValue( psAdiEntry, ABCC_GetMsgDataPtr( psMsgBuffer ),
                                  1, ABCC_GetMsgCmdExt1( psMsgBuffer ),
                                  &iMsgBitOffset, TRUE );

                  iDataSize = SizeInOctets( 0, iMsgBitOffset );

                  if( iDataSize == 0 )
                  {
                     bErrCode = ABP_ERR_OUT_OF_RANGE;
                  }
               }
            break;

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
            case ABP_APPD_IA_ELEM_NAME:
               if( ( psAdiEntry->psStruct != NULL ) &&
                   ( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName != NULL ) )
               {
                  if( ABCC_GetMsgCmdExt1( psMsgBuffer ) < psAdiEntry->bNumOfElements )
                  {
                     ABCC_SetMsgString( psMsgBuffer,
                                        psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName,
                                        (UINT16)strlen( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName ),
                                        0 );
                     iDataSize = (UINT16)strlen( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName );
                  }
                  else
                  {
                     bErrCode = ABP_ERR_INV_CMD_EXT_1;
                  }
               }
               else
               {
                  bErrCode = ABP_ERR_INV_CMD_EXT_0;
               }
               break;
#endif

         default:
            bErrCode = ABP_ERR_UNSUP_CMD;
            break;
         }
         break;
      case ABP_CMD_SET_INDEXED_ATTR:
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
            case ABP_APPD_IA_VALUE:
               if( !( psAdiEntry->bDesc & ABP_APPD_DESCR_SET_ACCESS ) )
               {
                  bErrCode = ABP_ERR_ATTR_NOT_SETABLE;
                  break;
               }
#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
               else if( ( psAdiEntry->psStruct != NULL ) &&
                        !( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].bDesc &
                                                 ABP_APPD_DESCR_SET_ACCESS ) )
               {
                  bErrCode = ABP_ERR_ATTR_NOT_SETABLE;
                  break;
               }
#endif
               else
               {
                  iItemSize = ( GetAdiSizeInBits( psAdiEntry, 1, ABCC_GetMsgCmdExt1( psMsgBuffer ) ) + 7 ) / 8;

                  if( ABCC_GetMsgDataSize( psMsgBuffer ) > iItemSize )
                  {
                     bErrCode = ABP_ERR_TOO_MUCH_DATA;
                     break;
                  }
                  else if( ABCC_GetMsgDataSize( psMsgBuffer ) < iItemSize )
                  {
                     bErrCode = ABP_ERR_NOT_ENOUGH_DATA;
                     break;
                  }

#if( AD_IA_MIN_MAX_DEFAULT_ENABLE )
                  bErrCode = VerifyRange( psAdiEntry, ABCC_GetMsgDataPtr( psMsgBuffer ),
                                          ABCC_GetMsgCmdExt1( psMsgBuffer ) );
#endif
                  if( bErrCode == ABP_ERR_NO_ERROR )
                  {
#if( ABCC_CFG_ADI_TRANS_SET_CALLBACK_ENABLED )
                     if( psAdiEntry->pnSetAdiValueTransparent != NULL )
                     {
                        bErrCode = psAdiEntry->pnSetAdiValueTransparent( psAdiEntry,
                                                                         1,
                                                                         ABCC_GetMsgCmdExt1( psMsgBuffer ),
                                                                         iLeTOi( psMsgBuffer->sHeader.iDataSize ),
                                                                         ABCC_GetMsgDataPtr( psMsgBuffer ) );
                     }
                     else
#endif
                     {
                        SetAdiValue( psAdiEntry,
                                     ABCC_GetMsgDataPtr( psMsgBuffer ),
                                     1, ABCC_GetMsgCmdExt1( psMsgBuffer ),
                                     &iMsgBitOffset, TRUE );
                        /*
                        ** Success.
                        */
                        iDataSize = 0;
                     }
                  }
               }
               break;

         default:
            bErrCode =  ABP_ERR_UNSUP_CMD;
            break;
         }
         break;

      default:
         /*
         ** Unsupported command.
         */
         bErrCode = ABP_ERR_UNSUP_CMD;
         break;
      }
   }
   else
   {
      /*
      ** The instance was not found.
      */
      bErrCode = ABP_ERR_UNSUP_INST;
   }

   /*
   ** Special handling. The remap response is already handled.
   */
   if( psMsgBuffer != NULL )
   {
      if( bErrCode == ABP_ERR_NO_ERROR )
      {
         ABP_SetMsgResponse( psMsgBuffer, iDataSize );
      }
      else
      {
         ABP_SetMsgErrorResponse( psMsgBuffer, 1, bErrCode );
      }

      ABCC_SendRespMsg( psMsgBuffer );
   }
}

void AD_UpdatePdReadData( void* pxPdDataBuf )
{
   if( ad_ReadMapInfo.paiMappedAdiList )
   {
      UINT16 iBitOffset = 0;

      WritePdMapFromBuffer( &ad_ReadMapInfo,
                            pxPdDataBuf,
                            &iBitOffset );
   }
}

BOOL AD_UpdatePdWriteData( void* pxPdDataBuf )
{
   if( ad_WriteMapInfo.paiMappedAdiList )
   {
      UINT16 iBitOffset = 0;

      WriteBufferFromPdMap( pxPdDataBuf,
                            &iBitOffset,
                            &ad_WriteMapInfo );
   }
   else
   {
      return( FALSE );
   }

   return( TRUE );
}

void AD_WriteBufferFromPdMap( void* pxDstPdDataBuf,
                              UINT16* piOctetOffset,
                              const AD_MapType* pasMap )
{
   ad_MapInfoType sMapInfo;
   ad_MapType asPdMap;
   UINT16 iBitOffset;
   UINT16 iAdiIndex;

   sMapInfo.iMaxNumMappedAdi = 1;
   sMapInfo.iNumMappedAdi = 1;
   iBitOffset = *piOctetOffset * 8;

   if( pasMap != NULL )
   {
      while( pasMap->eDir != PD_END_MAP )
      {
         iAdiIndex = GetAdiIndex( pasMap->iInstance );

         if( iAdiIndex == AD_INVALID_ADI_INDEX )
         {
            ABCC_LOG_WARNING( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
               pasMap->iInstance,
               "Requested ADI could not be found %" PRIu16 "\n",
               pasMap->iInstance );

            return;
         }

         if( pasMap->bNumElem == AD_MAP_ALL_ELEM )
         {
            /*
            ** Convert internal representation for all elements to the
            ** actual number of elements.
            */
            asPdMap.bNumElements = ad_asADIEntryList[ iAdiIndex ].bNumOfElements;
            asPdMap.bStartIndex = 0;
         }
         else
         {
            asPdMap.bNumElements = pasMap->bNumElem;
            asPdMap.bStartIndex = pasMap->bElemStartIndex;
         }

         asPdMap.iAdiIndex = iAdiIndex;
         sMapInfo.paiMappedAdiList = &asPdMap;

         WriteBufferFromPdMap( pxDstPdDataBuf,
                               &iBitOffset,
                               &sMapInfo );

         pasMap++;
      }

      *piOctetOffset = SizeInOctets( 0, iBitOffset );
   }
}

void AD_WritePdMapFromBuffer( const AD_MapType* pasMap,
                              void* pxSrcPdDataBuf,
                              UINT16* piOctetOffset )
{
   ad_MapInfoType sMapInfo;
   ad_MapType asPdMap;
   UINT16 iBitOffset;
   UINT16 iAdiIndex;

   sMapInfo.iMaxNumMappedAdi = 1;
   sMapInfo.iNumMappedAdi = 1;
   iBitOffset = *piOctetOffset * 8;

   if( pasMap != NULL )
   {
      while( pasMap->eDir != PD_END_MAP )
      {
         iAdiIndex = GetAdiIndex( pasMap->iInstance );

         if( iAdiIndex == AD_INVALID_ADI_INDEX )
         {
            ABCC_LOG_WARNING( ABCC_EC_ERROR_IN_PD_MAP_CONFIG,
               pasMap->iInstance,
               "Requested ADI could not be found %" PRIu16 "\n",
               pasMap->iInstance );

            return;
         }

         if( pasMap->bNumElem == AD_MAP_ALL_ELEM )
         {
            /*
            ** Convert internal representation for all elements to the
            ** actual number of elements.
            */
            asPdMap.bNumElements = ad_asADIEntryList[ iAdiIndex ].bNumOfElements;
            asPdMap.bStartIndex = 0;
         }
         else
         {
            asPdMap.bNumElements = pasMap->bNumElem;
            asPdMap.bStartIndex = pasMap->bElemStartIndex;
         }

         asPdMap.iAdiIndex = iAdiIndex;
         sMapInfo.paiMappedAdiList = &asPdMap;

         WritePdMapFromBuffer( &sMapInfo,
                               pxSrcPdDataBuf,
                               &iBitOffset );

         pasMap++;
      }

      *piOctetOffset = SizeInOctets( 0, iBitOffset );
   }
}

UINT16 AD_AdiMappingReq( const AD_AdiEntryType** ppsAdiEntry,
                         const AD_MapType** ppsDefaultMap )
{
   ABCC_NetFormatType eNetFormat;
   eNetFormat = ABCC_NetFormat();
#ifdef ABCC_SYS_BIG_ENDIAN
   ad_fDoNetworkEndianSwap = ( eNetFormat == NET_LITTLEENDIAN ) ? TRUE : FALSE;
#else
   ad_fDoNetworkEndianSwap = ( eNetFormat == NET_LITTLEENDIAN ) ? FALSE : TRUE;
#endif

   *ppsAdiEntry = ad_asADIEntryList;
   *ppsDefaultMap = ad_asDefaultMap;

   return( ad_iNumOfADIs );
}

void AD_RemapDone( void )
{
   /*
   ** This Write Process Data update is to ensure that the write process data
   ** is updated with the right content.
   */
   ABCC_TriggerWrPdUpdate();
}

void AD_GetAdiValue( const AD_AdiEntryType* psAdiEntry,
                     void* pxDest,
                     UINT8 bNumElements,
                     UINT8 bStartIndex,
                     UINT16* piDestBitOffset,
                     BOOL fExplicit )
{
   UINT16 iSrcBitOffset;

#if( ABCC_CFG_ADI_GET_SET_CALLBACK_ENABLED )
   /*
   ** If a get callback is registered the user is notified that the ADI will be
   ** read.
   */
   if( psAdiEntry->pnGetAdiValue != NULL )
   {
      psAdiEntry->pnGetAdiValue( psAdiEntry,
                                 bNumElements,
                                 bStartIndex );
   }
#endif

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
   if( psAdiEntry->psStruct != NULL )
   {
      UINT16 i;
      UINT16 iAdiBitSize;
      UINT8 bZero;

      i = 0;
      bZero = 0;

      if( fExplicit )
      {
         /*
         ** Begins by zeroing the destination buffer since all non-gettable
         ** elements are to be returned with zeros as data.
         */
         iAdiBitSize = GetAdiSizeInBits( psAdiEntry, bNumElements, bStartIndex );

         for( i = 0; i < ( ( iAdiBitSize + 7 ) / 8 ); i++ )
         {
            ABCC_PORT_CopyOctets( pxDest, ( *piDestBitOffset / 8 ) + i,
                                  &bZero, 0, 1 );
         }

         for( i = bStartIndex; i < bNumElements + bStartIndex; i++ )
         {
            if( !( psAdiEntry->psStruct[ i ].bDesc & ABP_APPD_DESCR_GET_ACCESS ) )
            {
               *piDestBitOffset += ( ABCC_GetDataTypeSizeInBits( psAdiEntry->psStruct[ i ].bDataType ) *
                                     psAdiEntry->psStruct[ i ].iNumSubElem );
               continue;
            }

            iSrcBitOffset = psAdiEntry->psStruct[ i ].bBitOffset;
            *piDestBitOffset += CopyValue( pxDest,
                                           *piDestBitOffset,
                                           psAdiEntry->psStruct[ i ].uData.sVOID.pxValuePtr,
                                           iSrcBitOffset,
                                           psAdiEntry->psStruct[ i ].bDataType,
                                           psAdiEntry->psStruct[ i ].iNumSubElem );
         }
      }
      else
      {
         for( i = bStartIndex; i < bNumElements + bStartIndex; i++ )
         {
            iSrcBitOffset = psAdiEntry->psStruct[ i ].bBitOffset;
            *piDestBitOffset += CopyValue( pxDest,
                                           *piDestBitOffset,
                                           psAdiEntry->psStruct[ i ].uData.sVOID.pxValuePtr,
                                           iSrcBitOffset,
                                           psAdiEntry->psStruct[ i ].bDataType,
                                           psAdiEntry->psStruct[ i ].iNumSubElem );
         }
      }
   }
   else
#else
   (void)fExplicit;
#endif
   {
      iSrcBitOffset = CalcStartIndexBitOffset( psAdiEntry->bDataType, bStartIndex );
      *piDestBitOffset += CopyValue( pxDest,
                                     *piDestBitOffset,
                                     psAdiEntry->uData.sVOID.pxValuePtr,
                                     iSrcBitOffset,
                                     psAdiEntry->bDataType,
                                     bNumElements );
   }
}

/*------------------------------------------------------------------------------
** Get the total data size of the present WR/RD PD map.
**------------------------------------------------------------------------------
*/
UINT16 AD_GetPresentPdSizeInOctets( PD_DirType eDir )
{
   UINT16 iSize = 0;

   switch( eDir )
   {
   case PD_READ:
      iSize = ad_ReadMapInfo.iPdSize;
      break;

   case PD_WRITE:
      iSize = ad_WriteMapInfo.iPdSize;
      break;

   default:
      break;
   }

   return( iSize );
}

/*------------------------------------------------------------------------------
** Copy ADI values to a buffer using the present WR/RD PD map.
**------------------------------------------------------------------------------
*/
void AD_CopyPresentPdToExtBuffer( PD_DirType eDir, void* pxBuffer )
{
   UINT16 iSrcBitOffset = 0;

   switch( eDir )
   {
   case PD_READ:
      WriteBufferFromPdMap( pxBuffer, &iSrcBitOffset, &ad_ReadMapInfo );
      break;

   case PD_WRITE:
      WriteBufferFromPdMap( pxBuffer, &iSrcBitOffset, &ad_WriteMapInfo );
      break;

   default:
      break;
   }

   return;
}
