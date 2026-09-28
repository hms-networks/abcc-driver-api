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
#include "../abcc_api_config.h"
#include "abp.h"
#include "abcc_application_data_interface.h"
#include "abcc.h"
#include "abcc_api.h"
#include "abcc_hardware_abstraction.h"
#include "application_data_object.h"
#if AD_CFG_ADI_SANITY_CHECK_ENABLE
#include "abp_bac.h"
#include "abp_ccl.h"
#include "abp_dev.h"
#include "abp_eip.h"
#include "abp_mod.h"
#endif

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

#if AD_CFG_ADI_SANITY_CHECK_ENABLE
/*
** All valid/defined ADI descriptor bits.
*/
#define AD_ADI_DESC_BIT_ALL      ( ABP_APPD_DESCR_GET_ACCESS | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_NVS_PARAMETER )
#endif

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

#if AD_CFG_ADI_SANITY_CHECK_ENABLE
/*------------------------------------------------------------------------------
** Network ID / Network name list for the ADI sanity check printouts.
**------------------------------------------------------------------------------
*/
typedef struct
{
   const UINT16      iValue;
   const char* const pacName;
} ad_schk_NetworkNamesEntryType;
#endif

#if !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
static BOOL ad_fDoNetworkEndianSwap = FALSE;
#endif // !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
static const AD_MapType* ad_asDefaultMap = NULL;
static const AD_AdiEntryType* ad_asADIEntryList = NULL;
static UINT16  ad_iNumOfADIs;
static UINT16  ad_iHighestInstanceNumber;
static ad_MapType ad_PdReadMapping[ AD_MAX_NUM_READ_MAP_ENTRIES ];
static ad_MapType ad_PdWriteMapping[ AD_MAX_NUM_WRITE_MAP_ENTRIES ];
static ad_MapInfoType ad_ReadMapInfo;
static ad_MapInfoType ad_WriteMapInfo;

#if AD_CFG_ADI_SANITY_CHECK_ENABLE
static ad_schk_NetworkNamesEntryType ad_schk_NetworkNamesList[] =
{
   { ABP_NW_TYPE_PDPV1,          "PROFIBUS DP-V1"       },
   { ABP_NW_TYPE_COP,            "CANopen"              },
   { ABP_NW_TYPE_DEV,            "DeviceNet"            },
   { ABP_NW_TYPE_ETN_2P,         "Modbus TCP"           },
   { ABP_NW_TYPE_PIR,            "PROFINET"             },
   { ABP_NW_TYPE_PIR_FO,         "PROFINET"             },
   { ABP_NW_TYPE_PIR_IIOT,       "PROFINET"             },
   { ABP_NW_TYPE_PIR_FO_IIOT,    "PROFINET"             },
   { ABP_NW_TYPE_EIP_2P_BB,      "EtherNet/IP"          },
   { ABP_NW_TYPE_EIP_2P_BB_IIOT, "EtherNet/IP"          },
   { ABP_NW_TYPE_ECT,            "EtherCAT"             },
   { ABP_NW_TYPE_CCL,            "CC-Link"              },
   { ABP_NW_TYPE_BIP,            "BACnet/IP"            },
   { ABP_NW_TYPE_EPL,            "POWERLINK"            },
   { ABP_NW_TYPE_CFN,            "CC-Link IE Field"     },
   { ABP_NW_TYPE_CIET,           "CC-Link IE Field TSN" },
   { 0, NULL }
};

static UINT16 ad_schk_iErrorCount;
static UINT16 ad_schk_iWarningCount;

static BOOL  ad_schk_fBacAdvMapping;
static UINT8 ad_schk_abCclNetworkSettings[ 3 ];
static BOOL  ad_schk_fDevParameterObject;
static BOOL  ad_schk_fEipParameterObject;
static UINT8 ad_schk_bModIndexingBits;
#endif

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

#if !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
/*------------------------------------------------------------------------------
** Copies a 16 bit value from a source to a destination. Each value will be
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
** Copies a 32 bit value from a source to a destination. Each value will be
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
** Copies a 64 bit value from a source to a destination. Each value will be
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

#endif // !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL

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
**  differs from network endian a swap will be done if not disabled (see note 2
**  below).
**
**  NOTE 1 !! For all non-bit data types the source and destination must be
**  octet aligned.
**
**  NOTE 2 !! The defines AD_CFG_DISABLE_ADI_BYTE_SWAP_PD,
**  AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE and AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
**  can be used to disable the byte swap functionality for process data, message
**  data or both, respectively. If AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL is set to
**  1, the other two defines are ignored.
**------------------------------------------------------------------------------
** Arguments:
**    pxDst             - Destination base pointer.
**    iDestBitOffset    - Bit offset relative destination pointer.
**    pxSrc             - Source base pointer.
**    iSrcBitOffset     - Bit offset relative source pointer.
**    bDataType         - Data type according to ABP_<X> types in abp.h
**    iNumElem          - Number of elements to copy.
**
**    fExplicit         - Only present if AD_CFG_DISABLE_ADI_BYTE_SWAP_PD or
**                        AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE is set to 1.
**                        TRUE:  Copy is triggered by a message
**                        FALSE: Copy is triggered by process data
**
** Returns:
**    Size of copied data in bits.
**------------------------------------------------------------------------------
*/
#if AD_CFG_DISABLE_ADI_BYTE_SWAP_PD || AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
   static UINT16 CopyValue( void* pxDst,
                            UINT16 iDestBitOffset,
                            const void* pxSrc,
                            UINT16 iSrcBitOffset,
                            UINT8 bDataType,
                            UINT16 iNumElem,
                            BOOL fExplicit )
#else
   static UINT16 CopyValue( void* pxDst,
                            UINT16 iDestBitOffset,
                            const void* pxSrc,
                            UINT16 iSrcBitOffset,
                            UINT8 bDataType,
                            UINT16 iNumElem )
#endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_PD || AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE
{
   UINT8 bDataTypeSizeInOctets;
   UINT16 iBitSetSize;
#if !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
   BOOL fDoNetworkEndianSwap = ad_fDoNetworkEndianSwap;
#endif // !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL

#if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
   if( ( fExplicit && AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE ) ||
       ( !fExplicit && AD_CFG_DISABLE_ADI_BYTE_SWAP_PD ) )
   {
      fDoNetworkEndianSwap = FALSE;
   }
#endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD


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

   #if !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
      if( fDoNetworkEndianSwap )
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
   #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
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
**  The value is converted to network endian if not disabled (see note below).
**
**  NOTE !! The defines AD_CFG_DISABLE_ADI_BYTE_SWAP_PD,
**  AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE and AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
**  can be used to disable the byte swap functionality for process data, message
**  data or both, respectively. If AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL is set to
**  1, the other two defines are ignored.
**------------------------------------------------------------------------------
** Arguments:
**    psAdiEntry        - Entry of ADI
**    pxDest            - Pointer to destination
**    iDestBitOffset    - Destination bit offset
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
                         1
              #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                       , TRUE // min/max/default values of ADIs are read by message-based access, only
              #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                       );

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
   {
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
   }

   case ABP_SINT8:
   {
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
   }


   case ABP_UINT16:
   {
      if( puValue->iUnsigned < puProp->sPropUint16.iMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->iUnsigned > puProp->sPropUint16.iMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

   case ABP_SINT16:
   {
      if( puValue->iSigned < puProp->sPropInt16.iMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->iSigned > puProp->sPropInt16.iMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

   case ABP_UINT32:
   {
      if( puValue->lUnsigned < puProp->sPropUint32.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->lUnsigned > puProp->sPropUint32.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

   case ABP_SINT32:
   {
      if( puValue->lSigned < puProp->sPropInt32.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->lSigned > puProp->sPropInt32.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

   case ABP_FLOAT:
   {
      if( puValue->rFloat < puProp->sPropFloat32.rMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->rFloat > puProp->sPropFloat32.rMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

#if( ABCC_CFG_DOUBLE_ADI_SUPPORT_ENABLED )
   case ABP_DOUBLE:
   {
      if( puValue->dDouble < puProp->sPropFloat64.dMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->dDouble > puProp->sPropFloat64.dMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

#endif
#if( ABCC_CFG_64BIT_ADI_SUPPORT_ENABLED )
   case ABP_UINT64:
   {
      if( puValue->l64Unsigned < puProp->sPropUint64.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->l64Unsigned > puProp->sPropUint64.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

   case ABP_SINT64:
   {
      if( puValue->l64Signed < puProp->sPropInt64.lMinMaxDefault[ AD_MIN_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_LOW;
      }
      else if( puValue->l64Signed > puProp->sPropInt64.lMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
      {
         bErrCode = ABP_ERR_VAL_TOO_HIGH;
      }
      break;
   }

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
                                           1
                                #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                         , TRUE // range check is done for message based ADI access, only
                                #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                         );

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
                                        1
                             #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                      , TRUE // range check is done for message based ADI access, only
                             #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                      );

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
**  By default it is swapped to application byte order. This swap can be
**  disabled by configuration (see note below).
**
**  NOTE !! The defines AD_CFG_DISABLE_ADI_BYTE_SWAP_PD,
**  AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE and AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
**  can be used to disable the byte swap functionality for process data, message
**  data or both, respectively. If AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL is set to
**  1, the other two defines are ignored.
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
                                       psAdiEntry->psStruct[ i ].iNumSubElem
                            #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                     , fExplicit
                            #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                      );
      }
   }
   else
#elif !(AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD)
   (void)fExplicit;
#endif
   {
      iDestBitOffset = CalcStartIndexBitOffset( psAdiEntry->bDataType, bStartIndex );
      *piSrcBitOffset += CopyValue( psAdiEntry->uData.sVOID.pxValuePtr,
                                    iDestBitOffset,
                                    pxData,
                                    *piSrcBitOffset,
                                    psAdiEntry->bDataType,
                                    bNumElements
                         #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                  , fExplicit
                         #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                  );
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

#if AD_CFG_ADI_SANITY_CHECK_ENABLE
/*------------------------------------------------------------------------------
** Print error or warning headers to the terminal.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    -
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_BeginErrorMessage( void )
{
   ad_schk_iErrorCount++;
   ABCC_PORT_printf( "ERROR: " );
}
static void ad_schk_BeginWarningMessage( void )
{
   ad_schk_iWarningCount++;
   ABCC_PORT_printf( "WARNING: " );
}

/*------------------------------------------------------------------------------
** Print ADI number in hex + dec.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    psAdi - Pointer to ADI entry.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_PrintAdiHeader( const AD_AdiEntryType* const psAdi )
{
   if( psAdi == NULL )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   ABCC_PORT_printf( "ADI 0x%04"PRIx16"/%"PRIu16, psAdi->iInstance, psAdi->iInstance );

   return;
}

/*------------------------------------------------------------------------------
** Print PD map entry number in hex + dec.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iIndex - Index value.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_PrintPdEntryHeader( const UINT16 iIndex )
{
   ABCC_PORT_printf( "PD map index 0x%04"PRIx16"/%"PRIu16, iIndex, iIndex );

   return;
}

/*------------------------------------------------------------------------------
** Fetch relevant network-specific settings from the host application object
** space.
**------------------------------------------------------------------------------
** Arguments:
**    -
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_OUT_OF_MSG_BUFFERS if message buffer allocation failed.
**------------------------------------------------------------------------------
*/
static ABCC_ErrorCodeType ad_schk_GetNwSpecSettings( void )
{
   ABP_MsgType*       psMsg;
   ABCC_ErrorCodeType eStatus;
   UINT8              bTemp;

   ad_schk_fBacAdvMapping = FALSE;
   ad_schk_abCclNetworkSettings[ 0 ] = 0x01;
   ad_schk_abCclNetworkSettings[ 1 ] = 0x00;
   ad_schk_abCclNetworkSettings[ 2 ] = 0x01;
   ad_schk_fDevParameterObject = TRUE;
   ad_schk_fEipParameterObject = TRUE;
   ad_schk_bModIndexingBits = 4; /* 4 bits -> 16 registers -> 32 bytes */

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "Message buffer allocation failed.\n" );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgDataSize( psMsg, 0 );
   ABCC_SetMsgSourceId( psMsg, 0 );
   ABCC_SetMsgDestObj( psMsg, ABP_OBJ_NUM_BAC );
   ABCC_SetMsgInstance( psMsg, 1 );
   ABCC_SetMsgCmdField( psMsg, ABP_MSG_HEADER_C_BIT | ABP_CMD_GET_ATTR );
   ABCC_SetMsgCmdExt0( psMsg, ABP_BAC_IA_SUPPORT_ADV_MAPPING );
   ABCC_SetMsgCmdExt1( psMsg, 0 );
   eStatus = ABCC_SendLoopbackCmdMsg( psMsg );
   if( ( eStatus == ABCC_EC_NO_ERROR ) &&
       ( ABCC_VerifyMessage( psMsg ) == ABCC_EC_NO_ERROR ) &&
       ( ABCC_GetMsgDataSize( psMsg ) == ABP_BAC_IA_SUPPORT_ADV_MAPPING_DS ) )
   {
      ABCC_GetMsgData8( psMsg, &bTemp, 0 );
      if( bTemp == 0 )
      {
         ad_schk_fBacAdvMapping = FALSE;
      }
      else
      {
         ad_schk_fBacAdvMapping = TRUE;
      }
   }

   ABCC_SetMsgDataSize( psMsg, 0 );
   ABCC_SetMsgSourceId( psMsg, 0 );
   ABCC_SetMsgDestObj( psMsg, ABP_OBJ_NUM_CCL );
   ABCC_SetMsgInstance( psMsg, 1 );
   ABCC_SetMsgCmdField( psMsg, ABP_MSG_HEADER_C_BIT | ABP_CMD_GET_ATTR );
   ABCC_SetMsgCmdExt0( psMsg, ABP_CCL_IA_NETWORK_SETTINGS );
   ABCC_SetMsgCmdExt1( psMsg, 0 );
   eStatus = ABCC_SendLoopbackCmdMsg( psMsg );
   if( ( eStatus == ABCC_EC_NO_ERROR ) &&
       ( ABCC_VerifyMessage( psMsg ) == ABCC_EC_NO_ERROR ) &&
       ( ABCC_GetMsgDataSize( psMsg ) == ABP_CCL_IA_NETWORK_SETTINGS_DS ) )
   {
      ABCC_GetMsgData8( psMsg, &ad_schk_abCclNetworkSettings[ 0 ], 0 );
      ABCC_GetMsgData8( psMsg, &ad_schk_abCclNetworkSettings[ 1 ], 1 );
      ABCC_GetMsgData8( psMsg, &ad_schk_abCclNetworkSettings[ 2 ], 2 );
   }

   ABCC_SetMsgDataSize( psMsg, 0 );
   ABCC_SetMsgSourceId( psMsg, 0 );
   ABCC_SetMsgDestObj( psMsg, ABP_OBJ_NUM_DEV );
   ABCC_SetMsgInstance( psMsg, 1 );
   ABCC_SetMsgCmdField( psMsg, ABP_MSG_HEADER_C_BIT | ABP_CMD_GET_ATTR );
   ABCC_SetMsgCmdExt0( psMsg, ABP_DEV_IA_ENABLE_PARAM_OBJECT );
   ABCC_SetMsgCmdExt1( psMsg, 0 );
   eStatus = ABCC_SendLoopbackCmdMsg( psMsg );
   if( ( eStatus == ABCC_EC_NO_ERROR ) &&
       ( ABCC_VerifyMessage( psMsg ) == ABCC_EC_NO_ERROR ) &&
       ( ABCC_GetMsgDataSize( psMsg ) == ABP_DEV_IA_ENABLE_PARAM_OBJECT_DS ) )
   {
      ABCC_GetMsgData8( psMsg, &bTemp, 0 );
      if( bTemp == 0 )
      {
         ad_schk_fDevParameterObject = FALSE;
      }
      else
      {
         ad_schk_fDevParameterObject = TRUE;
      }
   }

   ABCC_SetMsgDataSize( psMsg, 0 );
   ABCC_SetMsgSourceId( psMsg, 0 );
   ABCC_SetMsgDestObj( psMsg, ABP_OBJ_NUM_EIP );
   ABCC_SetMsgInstance( psMsg, 1 );
   ABCC_SetMsgCmdField( psMsg, ABP_MSG_HEADER_C_BIT | ABP_CMD_GET_ATTR );
   ABCC_SetMsgCmdExt0( psMsg, ABP_EIP_IA_ENABLE_PARAM_OBJECT );
   ABCC_SetMsgCmdExt1( psMsg, 0 );
   eStatus = ABCC_SendLoopbackCmdMsg( psMsg );
   if( ( eStatus == ABCC_EC_NO_ERROR ) &&
       ( ABCC_VerifyMessage( psMsg ) == ABCC_EC_NO_ERROR ) &&
       ( ABCC_GetMsgDataSize( psMsg ) == ABP_EIP_IA_ENABLE_PARAM_OBJECT_DS ) )
   {
      ABCC_GetMsgData8( psMsg, &bTemp, 0 );
      if( bTemp == 0 )
      {
         ad_schk_fEipParameterObject = FALSE;
      }
      else
      {
         ad_schk_fEipParameterObject = TRUE;
      }
   }

   ABCC_SetMsgDataSize( psMsg, 0 );
   ABCC_SetMsgSourceId( psMsg, 0 );
   ABCC_SetMsgDestObj( psMsg, ABP_OBJ_NUM_MOD );
   ABCC_SetMsgInstance( psMsg, 1 );
   ABCC_SetMsgCmdField( psMsg, ABP_MSG_HEADER_C_BIT | ABP_CMD_GET_ATTR );
   ABCC_SetMsgCmdExt0( psMsg, ABP_MOD_IA_ADI_INDEXING_BITS );
   ABCC_SetMsgCmdExt1( psMsg, 0 );
   eStatus = ABCC_SendLoopbackCmdMsg( psMsg );
   if( ( eStatus == ABCC_EC_NO_ERROR ) &&
       ( ABCC_VerifyMessage( psMsg ) == ABCC_EC_NO_ERROR ) &&
       ( ABCC_GetMsgDataSize( psMsg ) == ABP_MOD_IA_ADI_INDEXING_BITS_DS ) )
   {
      ABCC_GetMsgData8( psMsg, &ad_schk_bModIndexingBits, 0 );
      if( ad_schk_bModIndexingBits > 7 )
      {
         /*
         ** On out-of-range the ABCC will set this to the default value,
         ** i.e. 4.
         */
         ad_schk_bModIndexingBits = 4;
      }
   }

   ABCC_ReturnMsgBuffer( &psMsg );

   return( ABCC_EC_NO_ERROR );
}

/*------------------------------------------------------------------------------
** Get the name string for a network type.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType - ABCC40 network type value.
** Returns:
**    Pointer to string for the network in question, if supported.
**    NULL if the network is not supported by any ABCC40.
**------------------------------------------------------------------------------
*/
static const char* ad_schk_GetNetworkName( const UINT16 iNetworkType )
{
   UINT8 bIndex;

   bIndex = 0;
   while( ad_schk_NetworkNamesList[ bIndex ].pacName != NULL )
   {
      if( ad_schk_NetworkNamesList[ bIndex ].iValue == iNetworkType )
      {
         return( ad_schk_NetworkNamesList[ bIndex ].pacName );
      }
      bIndex++;
   }

   return( NULL );
}

/*------------------------------------------------------------------------------
** Check if a string conforms to the ISO 8859-1 character table.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    pacString - Pointer to string.
** Returns:
**    TRUE on success, FALSE on failure.
**------------------------------------------------------------------------------
*/
static BOOL ad_schk_IsNameStringIso88591( const char* pacString )
{
   UINT8* pbValue;

   if( pacString == NULL )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return( FALSE );
   }

   pbValue = (UINT8*)pacString;
   while( *pbValue != 0 )
   {
      /*
      ** ISO 8859-1 uses values 32..126 and 160..255.
      */
      if( ( *pbValue < 32U ) || ( ( *pbValue > 126U ) && ( *pbValue < 160U ) ) )
      {
         return( FALSE );
      }
      pbValue++;
   }

   return( TRUE );
}

/*------------------------------------------------------------------------------
** Check if a listed ABP data type is valid or not.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iAdiNum - ADI number.
**    bElement - Element in ADI.
**    bDataType - Element APB data type.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_TestAdiDataType( const AD_AdiEntryType* const psAdi, const UINT8 bElement, const UINT16 iNetworkType )
{
   UINT8 bDataType;

   if( psAdi == NULL )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
   if( psAdi->psStruct != NULL )
   {
      bDataType = psAdi->psStruct[ bElement ].bDataType;
   }
   else
#endif
   {
      bDataType = psAdi->bDataType;
   }

   switch( bDataType )
   {
   case ABP_BOOL:
   case ABP_SINT8:
   case ABP_SINT16:
   case ABP_SINT32:
   case ABP_UINT8:
   case ABP_UINT16:
   case ABP_UINT32:
   case ABP_CHAR:
   case ABP_ENUM:
   case ABP_BITS8:
   case ABP_BITS16:
   case ABP_BITS32:
   case ABP_OCTET:
   case ABP_SINT64:
   case ABP_UINT64:
   case ABP_FLOAT:
   case ABP_DOUBLE:
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
      break;

   default:
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( " element %"PRIu8": 'Data type' value (0x%02"PRIx8") is invalid.\n", bElement, bDataType );
      break;
   }

   if( iNetworkType == ABP_NW_TYPE_BIP )
   {
      if( ( bDataType == ABP_UINT64 ) || ( bDataType == ABP_SINT64 ) ||
          ( ( bDataType >= ABP_BIT2 ) && ( bDataType <= ABP_BIT7 ) ) ||
          ( ( bDataType >= ABP_BITS8 ) && ( bDataType <= ABP_BITS32 ) ) ||
          ( bDataType == ABP_OCTET ) ||
          ( bDataType >= ABP_PAD0 ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( " element %"PRIu8": 'Data type' value (0x%02"PRIx8") is not supported by BACnet.\n", bElement, bDataType );
      }
   }

   return;
}

/*------------------------------------------------------------------------------
** Check if any invalid descriptor bit combinations exists.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iAdiNum - ADI number.
**    bElement - Element in ADI.
**    bDesc - ADI descriptor field.
**    bDataType - Element APB data type.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_TestDescComb( const AD_AdiEntryType* const psAdi, const UINT8 bElement, const UINT16 iNetworkType )
{
   UINT8 bDesc;
   UINT8 bDataType;

   if( psAdi == NULL )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
   if( psAdi->psStruct != NULL )
   {
      bDesc = psAdi->psStruct[ bElement ].bDesc;
      bDataType = psAdi->psStruct[ bElement ].bDataType;
   }
   else
#endif
   {
      bDesc = psAdi->bDesc;
      bDataType = psAdi->bDataType;
   }

   if( bDesc & (UINT8)~( AD_ADI_DESC_BIT_ALL ) )
   {
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( " element %"PRIu8": 'Descriptor' has reserved bits set.\n", bElement );
   }

   if( ( ( bDataType == ABP_CHAR ) || ( bDataType == ABP_OCTET ) ) &&
       ( ( bDesc & ( ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_MAPPABLE_READ_PD ) ) != 0 ) )
   {
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( " element %"PRIu8": 'Descriptor' can not include 'PD mappable' for data types APB_CHAR or ABP_OCTET.\n", bElement );
   }

   if( ( bDataType == ABP_PAD0 ) && ( bDesc != 0 ) )
   {
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( " element %"PRIu8": 'Descriptor' must be '0' with data type APB_PAD0.\n", bElement );
   }

   /*
   ** Network-specific tests.
   */

   if( ( iNetworkType == ABP_NW_TYPE_COP ) ||
       ( iNetworkType == ABP_NW_TYPE_ECT ) ||
       ( iNetworkType == ABP_NW_TYPE_EPL ) )
   {
      if( ( bDataType != ABP_PAD0 ) &&
          ( bDesc & ( ABP_APPD_DESCR_GET_ACCESS | ABP_APPD_DESCR_SET_ACCESS ) ) == 0 )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( " element %"PRIu8": 'Descriptor' supports neither 'Get' nor 'Set'. This is incompatible with CANopen, EtherCAT and POWERLINK.\n", bElement );
      }

      if( ( bDesc & ABP_APPD_DESCR_MAPPABLE_READ_PD ) && !( bDesc & ABP_APPD_DESCR_SET_ACCESS ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( " element %"PRIu8": 'Descriptor' indicates 'RDPD mappable' without 'Set'. This is incompatible with CANopen, EtherCAT and POWERLINK.\n", bElement );
      }

      if( ( bDesc & ABP_APPD_DESCR_MAPPABLE_WRITE_PD ) && !( bDesc & ABP_APPD_DESCR_GET_ACCESS ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( " element %"PRIu8": 'Descriptor' indicates 'WRPD mappable' without 'Get'. This is incompatible with CANopen, EtherCAT and POWERLINK.\n", bElement );
      }
   }

   if( iNetworkType == ABP_NW_TYPE_BIP )
   {
      if( bDesc & ABP_APPD_DESCR_MAPPABLE_READ_PD )
      {
         ad_schk_BeginWarningMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( " element %"PRIu8": 'RDPD mappable' is not supported by BACnet.\n", bElement );
      }

      if( ( bDesc & ( ABP_APPD_DESCR_GET_ACCESS | ABP_APPD_DESCR_SET_ACCESS ) ) == 0 )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( " element %"PRIu8": 'Descriptor' supports neither 'Get' nor 'Set'. This is incompatible with BACnet.\n", bElement );
      }
   }

   return;
}

/*------------------------------------------------------------------------------
** Check the value and properties for an ABP_ENUM.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iAdiNum - ADI number.
**    puAdiData - Pointer to value/properties union.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_TestAbpEnum( const AD_AdiEntryType* const psAdi )
{
   UINT16 iEnumValue;
   UINT16 iElementIndex;
   UINT16 iCount;

   if( psAdi == NULL )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }
   if( psAdi->bDataType != ABP_ENUM )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   if( psAdi->uData.sENUM.psValueProps == NULL  )
   {
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( ": ADI is ABP_ENUM but has no properties.\n" );
      return;
   }

   /*
   ** The values for an APB_ENUM must be in the 0..N range, check that the
   ** Min/Max/Default properties matches this.
   */
   if( psAdi->uData.sENUM.psValueProps->bMinMaxDefault[ AD_MIN_VALUE_INDEX ] != 0 )
   {
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( ": ABP_ENUM 'Min' is not '0'.\n" );
   }
   if( psAdi->uData.sENUM.psValueProps->bMinMaxDefault[ AD_DEFAULT_VALUE_INDEX ] >
       psAdi->uData.sENUM.psValueProps->bMinMaxDefault[ AD_MAX_VALUE_INDEX ] )
   {
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( ": ABP_ENUM 'Default' is larger than 'Max'.\n" );
   }

   /*
   ** There must be at least as many strings as there are possible enum
   ** values.
   */
   if( psAdi->uData.sENUM.psValueProps->iNumOfEnumStrings <
     ( psAdi->uData.sENUM.psValueProps->bMinMaxDefault[ AD_MAX_VALUE_INDEX ] + 1 ) )
   {
      ad_schk_BeginErrorMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( ": The number of ABP_ENUM strings does not match the 'Max'.\n" );
   }

   /*
   ** There must be one, and only one, string for each possible enum value.
   */
   for( iEnumValue = 0; iEnumValue < psAdi->uData.sENUM.psValueProps->bMinMaxDefault[ AD_MAX_VALUE_INDEX ]; iEnumValue++ )
   {
      iCount = 0;
      for( iElementIndex = 0; iElementIndex < psAdi->uData.sENUM.psValueProps->iNumOfEnumStrings; iElementIndex++ )
      {
         if( psAdi->uData.sENUM.psValueProps->pasEnumStrings[ iElementIndex ].bValue == iEnumValue )
         {
            iCount++;
         }
      }
      if( iCount == 0 )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( ": No ABP_ENUM strings for value '%"PRIu16"'.\n", iEnumValue );
      }
      if( iCount > 1 )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( ": Multiple ABP_ENUM strings for value '%"PRIu16"'.\n", iEnumValue );
      }
   }

   /*
   ** Each enum string must be ISO 8859-1 compliant.
   */
   for( iElementIndex = 0; iElementIndex < psAdi->uData.sENUM.psValueProps->iNumOfEnumStrings; iElementIndex++ )
   {
      if( !ad_schk_IsNameStringIso88591( psAdi->uData.sENUM.psValueProps->pasEnumStrings[ iElementIndex ].acEnumStr ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( psAdi );
         ABCC_PORT_printf( ": ABP_ENUM string index %"PRIu16" is not compliant with the ISO 8859-1 character set.\n", iElementIndex );
      }
   }

   /*
   ** The 'Value' must be inside the given Min/Max.
   */
   if( ( *(psAdi->uData.sENUM.pbValuePtr) < psAdi->uData.sENUM.psValueProps->bMinMaxDefault[ AD_MIN_VALUE_INDEX ] ) ||
       ( *(psAdi->uData.sENUM.pbValuePtr) > psAdi->uData.sENUM.psValueProps->bMinMaxDefault[ AD_MAX_VALUE_INDEX ] ) )
   {
      ad_schk_BeginWarningMessage();
      ad_schk_PrintAdiHeader( psAdi );
      ABCC_PORT_printf( ": Present value is out-of-range.\n" );
   }

   return;
}

/*------------------------------------------------------------------------------
** Check the value and properties for an ADI.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    psAdi - Pointer to ADI entry.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_TestValueAndProps( const AD_AdiEntryType* const psAdi )
{
   if( psAdi == NULL )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   /*
   ** At the moment only ABP_ENUMs are checked since they pointless without
   ** valid properties.
   */

   switch( psAdi->bDataType )
   {
   case ABP_ENUM:
      ad_schk_TestAbpEnum( psAdi );
      break;

   case ABP_BOOL:
   case ABP_SINT8:
   case ABP_SINT16:
   case ABP_SINT32:
   case ABP_UINT8:
   case ABP_UINT16:
   case ABP_UINT32:
   case ABP_CHAR:
   case ABP_BITS8:
   case ABP_BITS16:
   case ABP_BITS32:
   case ABP_OCTET:
   case ABP_SINT64:
   case ABP_UINT64:
   case ABP_FLOAT:
   case ABP_DOUBLE:
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
      break;

   default:
      break;
   }

   return;
}

/*------------------------------------------------------------------------------
** Return the max. 'Number of instances' applicable to a certain network.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType - ABP network type value.
**    piLimit - Limit value, set by this function.
**    ppacComment - Comment string, set by this function.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_GetNumberOfInstancesLimit( const UINT16 iNetworkType, UINT16* const piLimit, const char ** const ppacComment )
{
   if( ( piLimit == NULL ) || ( ppacComment == NULL ) )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   switch( iNetworkType )
   {
   case ABP_NW_TYPE_PDPV1:
      *piLimit = 65025;
      *ppacComment = "This corresponds to 255 slots with 255 indexes per slot";
      break;

   case ABP_NW_TYPE_COP:
      *piLimit = 0xDFFF;
      *ppacComment = "This corresponds to 0x3FFF vendor-specific objects and 0xA000 profile-specific objects.";
      break;

   case ABP_NW_TYPE_ETN_2P:
      /*
      ** 0xEFF0 corresponds to the number of registers in the 'transparent'
      ** Modbus Holding Register range, registers 0x1010 - 0xFFFF.
      */
      *piLimit = 0xEFF0 / ( 1 << ad_schk_bModIndexingBits );
      *ppacComment = "See 'Number of indexing bits' in the Host Modbus Object.";
      break;

   case ABP_NW_TYPE_PIR:
   case ABP_NW_TYPE_PIR_FO:
   case ABP_NW_TYPE_PIR_IIOT:
   case ABP_NW_TYPE_PIR_FO_IIOT:
      *piLimit = 0x7FFF;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_ECT:
      *piLimit = 0xDFFF;
      *ppacComment = "This corresponds to 0x3FFF vendor-specific objects and 0xA000 profile-specific objects.";
      break;

   case ABP_NW_TYPE_BIP:
      if( ad_schk_fBacAdvMapping )
      {
         *piLimit = 4 * 256;
         *ppacComment = "256 ADIs of each of the 4 supported BACnet object types are reachable from BACnet when the 'Advanced mapping' in the Host BACnet Object is enabled.";
      }
      else
      {
         *piLimit = 256;
         *ppacComment = "256 ADIs (1 - 256) are reachable from BACnet when the 'Advanced mapping' in the Host BACnet Object is disabled.";
      }
      break;

   case ABP_NW_TYPE_EPL:
      *piLimit = 0xDFFF;
      *ppacComment = "This corresponds to 0x3FFF vendor-specific objects and 0xA000 profile-specific objects.";
      break;

   default:
      *piLimit = 0xFFFF;
      *ppacComment = NULL;
      break;
   }

   return;
}

/*------------------------------------------------------------------------------
** Return the 'Highest instance number' applicable to a certain network.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType - ABP network type value.
**    piLimit - Limit value, set by this function.
**    ppacComment - Comment string, set by this function.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_GetHighestInstanceNumberLimit( const UINT16 iNetworkType, UINT16* const piLimit, const char ** const ppacComment )
{
   if( ( piLimit == NULL ) || ( ppacComment == NULL ) )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   switch( iNetworkType )
   {
   case ABP_NW_TYPE_PDPV1:
      *piLimit = 65025;
      *ppacComment = "This corresponds to 255 slots with 255 indexes per slot";
      break;

   case ABP_NW_TYPE_COP:
      *piLimit = 0xDFFF;
      *ppacComment = "ADIs 0x0001-0x3FFF corresponds to vendor-specific objects and ADIs 0x4000-0xDFFF corresponds to profile-specific objects.";
      break;

   case ABP_NW_TYPE_ETN_2P:
      /*
      ** 0xEFF0 corresponds to the number of registers in the 'transparent'
      ** Modbus Holding Register range, registers 0x1010 - 0xFFFF.
      */
      *piLimit = 0xEFF0 / ( 1 << ad_schk_bModIndexingBits );
      break;

   case ABP_NW_TYPE_PIR:
   case ABP_NW_TYPE_PIR_FO:
   case ABP_NW_TYPE_PIR_IIOT:
   case ABP_NW_TYPE_PIR_FO_IIOT:
      *piLimit = 0x7FFF;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_ECT:
      *piLimit = 0xDFFF;
      *ppacComment = "ADIs 0x0001-0x3FFF corresponds to vendor-specific objects and ADIs 0x4000-0xDFFF corresponds to profile-specific objects.";
      break;

   case ABP_NW_TYPE_BIP:
      if( ad_schk_fBacAdvMapping )
      {
         *piLimit = 0xFFFF;
         *ppacComment = NULL;
      }
      else
      {
         *piLimit = 256;
         *ppacComment = "256 ADIs (1 - 256) are reachable from BACnet when the 'Advanced mapping' in the Host BACnet Object is disabled.";
      }
      break;

   case ABP_NW_TYPE_EPL:
      *piLimit = 0xDFFF;
      *ppacComment = "ADIs 0x0001-0x3FFF corresponds to vendor-specific objects and ADIs 0x4000-0xDFFF corresponds to profile-specific objects.";
      break;

   default:
      *piLimit = 0xFFFF;
      *ppacComment = NULL;
      break;
   }

   return;
}

/*------------------------------------------------------------------------------
** Return the ADI name string limit applicable to a certain network.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType - ABP network type value.
**    piLimit - Limit value, set by this function.
**    ppacComment - Comment string, set by this function.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_GetNameLengthLimit( const UINT16 iNetworkType, UINT16* const piLimit, const char ** const ppacComment )
{
   if( ( piLimit == NULL ) || ( ppacComment == NULL ) )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   switch( iNetworkType )
   {
   case ABP_NW_TYPE_DEV:
      if( ad_schk_fDevParameterObject )
      {
         *piLimit = 16;
         *ppacComment = "The CIP Parameter Object will truncate ADI names to 16 characters. This is according to the CIP specification.";
      }
      else
      {
         *piLimit = 0;
         *ppacComment = NULL;
      }
      break;

   case ABP_NW_TYPE_EIP_2P_BB:
   case ABP_NW_TYPE_EIP_2P_BB_IIOT:
      if( ad_schk_fEipParameterObject )
      {
         *piLimit = 16;
         *ppacComment = "The CIP Parameter Object will truncate ADI names to 16 characters. This is according to the CIP specification.";
      }
      else
      {
         *piLimit = 0;
         *ppacComment = NULL;
      }
      break;

   case ABP_NW_TYPE_BIP:
      if( ad_schk_fBacAdvMapping )
      {
         *piLimit = 252;
         *ppacComment = "BACnet object names are limited to 252 characters when the 'Advanced mapping' in the Host BACnet Object is enabled.";
      }
      else
      {
         *piLimit = 0;
         *ppacComment = NULL;
      }
      break;

   case ABP_NW_TYPE_ECT:
      *piLimit = 258;
      *ppacComment = "Note that this limit depends on the EtherCAT mailbox sizes. 258 characters is valid for the default mailbox size of 276 bytes.";
      break;

   case ABP_NW_TYPE_PDPV1:
   case ABP_NW_TYPE_COP:
   case ABP_NW_TYPE_ETN_2P:
   case ABP_NW_TYPE_PIR:
   case ABP_NW_TYPE_PIR_FO:
   case ABP_NW_TYPE_PIR_IIOT:
   case ABP_NW_TYPE_PIR_FO_IIOT:
   case ABP_NW_TYPE_CCL:
   case ABP_NW_TYPE_EPL:
   case ABP_NW_TYPE_CFN:
   case ABP_NW_TYPE_CIET:
   default:
      /*
      ** Zero is used as a 'skip size check' signal. The listed networks do
      ** not have a way to access the ADI name, so we skip the network-
      ** specific check with them.
      */
      *piLimit = 0;
      *ppacComment = NULL;
      break;
   }

   return;
}

/*------------------------------------------------------------------------------
** Return the ADI size limit applicable to a certain network.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType - ABP network type value.
**    piLimit - Limit value, set by this function.
**    ppacComment - Comment string, set by this function.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_GetAdiSizeLimit( const UINT16 iNetworkType, UINT16* const piLimit, const char ** const ppacComment )
{
   if( ( piLimit == NULL ) || ( ppacComment == NULL ) )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   switch( iNetworkType )
   {
   case ABP_NW_TYPE_PDPV1:
      *piLimit = 240;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_DEV:
      *piLimit = 512;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_ETN_2P:
      *piLimit = 1 << ( ad_schk_bModIndexingBits + 1 );
      *ppacComment = "See 'Number of indexing bits' in the Host Modbus Object.";
      break;

   case ABP_NW_TYPE_PIR:
   case ABP_NW_TYPE_PIR_FO:
   case ABP_NW_TYPE_PIR_IIOT:
   case ABP_NW_TYPE_PIR_FO_IIOT:
      *piLimit = 1308;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_CCL:
      /*
      ** Zero is used as a 'skip size check' signal. CC-Link does not support
      ** acyclical accesses.
      */
      *piLimit = 0;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_BIP:
      *piLimit = 4;
      *ppacComment = NULL;
      break;

   default:
      *piLimit = ABP_MAX_MSG_DATA_BYTES;
      *ppacComment = NULL;
      break;
   }

   return;
}

/*------------------------------------------------------------------------------
** Return the PD size limit applicable to a certain network.
** Used by the ADI + PD map sanity check functions.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType - ABP network type value.
**    piLimit - Limit value, set by this function.
**    ppacComment - Comment string, set by this function.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void ad_schk_GetPdSizeLimit( const UINT16 iNetworkType, UINT16* const piPdLimit, const char ** const ppacComment )
{
   if( ( piPdLimit == NULL ) || ( ppacComment == NULL ) )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR, 0, "Unexpected NULL pointer\n" );
      return;
   }

   switch( iNetworkType )
   {
   case ABP_NW_TYPE_PDPV1:
      *piPdLimit = 244;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_COP:
      *piPdLimit = 512;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_DEV:
      *piPdLimit = 512;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_ETN_2P:
      *piPdLimit = 1536;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_PIR:
   case ABP_NW_TYPE_PIR_FO:
   case ABP_NW_TYPE_PIR_IIOT:
   case ABP_NW_TYPE_PIR_FO_IIOT:
      /*
      ** PROFINET allows up to 1440 bytes of PD, but one must also account for
      ** the IOPS/IOCS (IO Producer/Consumer Status) bytes for each submodule.
      ** With the maximum amount of submodules (128) 1308 bytes will remain.
      ** This is checked with code in the PD map check function.
      */
      *piPdLimit = 1440;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_EIP_2P_BB:
   case ABP_NW_TYPE_EIP_2P_BB_IIOT:
      *piPdLimit = 1448;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_ECT:
      *piPdLimit = 1486;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_CCL:
      if( ad_schk_abCclNetworkSettings[ 0 ] == 2 )
      {
         *piPdLimit = 368;
         *ppacComment = "This size is applicable if CC-Link V2.00 is used, and corresponds to 896 bits & 128 words.";
      }
      else
      {
         *piPdLimit = 48;
         *ppacComment = "This size is applicable if CC-Link V1.10 is used, and corresponds to 128 bits & 16 words.";
      }
      break;

   case ABP_NW_TYPE_BIP:
      /*
      ** BACnet has assymetrical limits and it it is the number of WRPD-mapped
      ** ADIs that is important rather than the PD size, which is checked with
      ** code in the PD map check function. The PD size limit is set to the
      ** maximum number of WRPD-mappable ADIs (64) times the size of the
      ** largest supported data type (32 bits).
      */
      *piPdLimit = 64 * ABP_UINT32_SIZEOF;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_EPL:
      *piPdLimit = 1490;
      *ppacComment = NULL;
      break;

   case ABP_NW_TYPE_CFN:
      *piPdLimit = 1536;
      *ppacComment = "This size corresponds to max 2048 bits, max 768 words, or max 1536 bytes.";
      break;

   case ABP_NW_TYPE_CIET:
      *piPdLimit = 1420;
      *ppacComment = NULL;
      break;

   default:
      *piPdLimit = 0;
      *ppacComment = NULL;
      break;
   }

   return;
}
#endif

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
      {
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
               if( ad_asADIEntryList[ iIndex ].bDesc & ABP_APPD_DESCR_MAPPABLE_READ_PD )
               {
                  iCnt++;
               }
            }
            ABCC_SetMsgData16( psMsgBuffer, iCnt, 0 );
            iDataSize = ABP_UINT16_SIZEOF;
            break;
         }

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
            break;
         }

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
            break;
         }

         default:
            /*
            ** Unsupported attribute.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
            break;
         }
         break;
      }

      case ABP_APPD_CMD_GET_INST_BY_ORDER:
      {
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
      }

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
      {
         /*
         ** Switch on attribute.
         */
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
         case ABP_APPD_IA_NAME:
         {
            if( psAdiEntry->pacName )
            {
               iDataSize = (UINT16) strlen( psAdiEntry->pacName );
               ABCC_SetMsgString( psMsgBuffer,
                                  psAdiEntry->pacName, iDataSize, 0 );
            }
            else
            {
               iDataSize = 0;
            }
            break;
         }

         case ABP_APPD_IA_DATA_TYPE:
         {
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
         }

         case ABP_APPD_IA_NUM_ELEM:
            ABCC_SetMsgData8( psMsgBuffer, psAdiEntry->bNumOfElements, 0 );
            iDataSize = ABP_APPD_IA_NUM_ELEM_DS;
            break;

         case ABP_APPD_IA_DESCRIPTOR:
         {
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
         }

         case ABP_APPD_IA_VALUE: /* Value. */
         {
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
         }

#if( AD_IA_MIN_MAX_DEFAULT_ENABLE )
         case ABP_APPD_IA_MAX_VALUE:
         {
            bErrCode = GetMinMaxDefault( psAdiEntry,
                                         ABCC_GetMsgDataPtr( psMsgBuffer ),
                                         AD_MAX_VALUE_INDEX,
                                         &iDataSize );
            iDataSize = SizeInOctets( 0, iDataSize );
            break;
         }

         case ABP_APPD_IA_MIN_VALUE:
         {
            bErrCode = GetMinMaxDefault( psAdiEntry,
                                         ABCC_GetMsgDataPtr( psMsgBuffer ),
                                         AD_MIN_VALUE_INDEX,
                                         &iDataSize );
            iDataSize = SizeInOctets( 0, iDataSize );
            break;
         }

         case ABP_APPD_IA_DFLT_VALUE:
         {
            bErrCode = GetMinMaxDefault( psAdiEntry,
                                         ABCC_GetMsgDataPtr( psMsgBuffer ),
                                         AD_DEFAULT_VALUE_INDEX,
                                         &iDataSize );
            iDataSize = SizeInOctets( 0, iDataSize );
            break;
         }
#endif

         case ABP_APPD_IA_NUM_SUB_ELEM:
         {
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
         }

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
         case ABP_APPD_IA_ELEM_NAME:
         {
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
         }
#endif

         default:
            /*
            ** Unsupported attribute.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
            break;
         }
         break;
      }
      case ABP_CMD_SET_ATTR:
      {
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
         {
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
   #if !( AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL )

            bErrCode = VerifyRange( psAdiEntry, ABCC_GetMsgDataPtr( psMsgBuffer ),
                                    AD_ALL_ADI_INDEX );

   #endif // !( AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL )
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
         }

         default:
            /*
            ** Unsupported attribute.
            */
            bErrCode = ABP_ERR_INV_CMD_EXT_0;
            break;
         }
         break;
      }

      case ABP_CMD_GET_ENUM_STR:
      {
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
               UINT16 iStringIndex;

               for( iStringIndex = 0; iStringIndex < psAdiEntry->uData.sENUM.psValueProps->iNumOfEnumStrings; iStringIndex++ )
               {
                  if( psAdiEntry->uData.sENUM.psValueProps->pasEnumStrings[ iStringIndex ].bValue ==
                      ABCC_GetMsgCmdExt1( psMsgBuffer ) )
                  {
                     break;
                  }
               }

               if( iStringIndex < psAdiEntry->uData.sENUM.psValueProps->iNumOfEnumStrings )
               {
                  iDataSize = (UINT16)strlen( psAdiEntry->uData.sENUM.psValueProps->pasEnumStrings[ iStringIndex ].acEnumStr );
                  ABCC_SetMsgString( psMsgBuffer,
                                     psAdiEntry->uData.sENUM.psValueProps->pasEnumStrings[ iStringIndex ].acEnumStr,
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
      }

      case ABP_CMD_GET_INDEXED_ATTR:
      {
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
         case ABP_APPD_IA_VALUE:
         {
            if( !( psAdiEntry->bDesc & ABP_APPD_DESCR_GET_ACCESS ) )
            {
               bErrCode = ABP_ERR_ATTR_NOT_GETABLE;
               break;
            }
            else if( ABCC_GetMsgCmdExt1( psMsgBuffer ) >= psAdiEntry->bNumOfElements )
            {
               bErrCode = ABP_ERR_INV_CMD_EXT_1;
               break;
            }
            else if( psAdiEntry->bDataType == ABP_CHAR )
            {
               /* This command cannot be used for CHAR arrays. */
               bErrCode = ABP_ERR_GENERAL_ERROR;
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
         }

#if( ABCC_CFG_STRUCT_DATA_TYPE_ENABLED )
         case ABP_APPD_IA_ELEM_NAME:
         {
            if( ABCC_GetMsgCmdExt1( psMsgBuffer ) >= psAdiEntry->bNumOfElements )
            {
               bErrCode = ABP_ERR_INV_CMD_EXT_1;
               break;
            }
            else if( ( psAdiEntry->psStruct != NULL ) &&
                     ( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName != NULL ) )
            {
               ABCC_SetMsgString( psMsgBuffer,
                                  psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName,
                                  (UINT16)strlen( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName ),
                                  0 );
               iDataSize = (UINT16)strlen( psAdiEntry->psStruct[ ABCC_GetMsgCmdExt1( psMsgBuffer ) ].pacElementName );
            }
            else
            {
               bErrCode = ABP_ERR_INV_CMD_EXT_0;
            }
            break;
         }
#endif

         default:
            bErrCode = ABP_ERR_UNSUP_CMD;
            break;
         }
         break;
      }

      case ABP_CMD_SET_INDEXED_ATTR:
      {
         switch( ABCC_GetMsgCmdExt0( psMsgBuffer ) )
         {
         case ABP_APPD_IA_VALUE:
         {
            if( !( psAdiEntry->bDesc & ABP_APPD_DESCR_SET_ACCESS ) )
            {
               bErrCode = ABP_ERR_ATTR_NOT_SETABLE;
               break;
            }
            else if( ABCC_GetMsgCmdExt1( psMsgBuffer ) >= psAdiEntry->bNumOfElements )
            {
               bErrCode = ABP_ERR_INV_CMD_EXT_1;
               break;
            }
            else if( psAdiEntry->bDataType == ABP_CHAR )
            {
               /* This command cannot be used for CHAR arrays. */
               bErrCode = ABP_ERR_GENERAL_ERROR; 
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
   #if !( AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL )

               bErrCode = VerifyRange( psAdiEntry, ABCC_GetMsgDataPtr( psMsgBuffer ),
                                       ABCC_GetMsgCmdExt1( psMsgBuffer ) );

   #endif // !( AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL )
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
         }

         default:
            bErrCode =  ABP_ERR_UNSUP_CMD;
            break;
         }
         break;
      }

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
#if !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL
   ABCC_NetFormatType eNetFormat;
   eNetFormat = ABCC_NetFormat();
#ifdef ABCC_SYS_BIG_ENDIAN
   ad_fDoNetworkEndianSwap = ( eNetFormat == NET_LITTLEENDIAN ) ? TRUE : FALSE;
#else
   ad_fDoNetworkEndianSwap = ( eNetFormat == NET_LITTLEENDIAN ) ? FALSE : TRUE;
#endif
#endif // !AD_CFG_DISABLE_ADI_BYTE_SWAP_TOTAL

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
                                           psAdiEntry->psStruct[ i ].iNumSubElem
                                #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                         , fExplicit
                                #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                         );
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
                                           psAdiEntry->psStruct[ i ].iNumSubElem
                                #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                         , fExplicit
                                #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                         );
         }
      }
   }
   else
#elif !(AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD)
   (void)fExplicit;
#endif
   {
      iSrcBitOffset = CalcStartIndexBitOffset( psAdiEntry->bDataType, bStartIndex );
      *piDestBitOffset += CopyValue( pxDest,
                                     *piDestBitOffset,
                                     psAdiEntry->uData.sVOID.pxValuePtr,
                                     iSrcBitOffset,
                                     psAdiEntry->bDataType,
                                     bNumElements
                          #if AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                   , fExplicit
                          #endif // AD_CFG_DISABLE_ADI_BYTE_SWAP_MESSAGE || AD_CFG_DISABLE_ADI_BYTE_SWAP_PD
                                   );
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

#if AD_CFG_ADI_SANITY_CHECK_ENABLE
/*------------------------------------------------------------------------------
** ADI sanity checks and PD map sanity checks. See comments in
** "application_data_object.h" for details.
**------------------------------------------------------------------------------
*/
void AD_SCHK_TestAdiList( const AD_AdiEntryType* const pasAdiList, const UINT16 iNumOfAdis, const UINT16 iNetworkType )
{
   const char* pacNetworkName;

   UINT16      iNOILimit;
   const char* pacNOIComment;
   UINT16      iHINLimit;
   const char* pacHINComment;
   UINT16      iNameLengthLimit;
   const char* pacNameLengthComment;
   UINT16      iADISizeLimit;
   const char* pacADISizeComment;

   UINT16 iAdiIndex;
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
   UINT8  bElementIndex;
#endif

   UINT16 iTemp;
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
   UINT8  bTemp;
#endif

   ad_schk_iErrorCount = 0;
   ad_schk_iWarningCount = 0;

   if( ( pasAdiList == NULL ) || ( iNumOfAdis == 0 ) )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "Invalid ADI list or ADI list size.\n" );
      goto PRINT_COUNT_AND_EXIT;
   }

   if( ad_schk_GetNwSpecSettings() != ABCC_EC_NO_ERROR )
   {
      goto PRINT_COUNT_AND_EXIT;
   }

   pacNetworkName = ad_schk_GetNetworkName( iNetworkType );
   ad_schk_GetNumberOfInstancesLimit( iNetworkType, &iNOILimit, &pacNOIComment );
   ad_schk_GetHighestInstanceNumberLimit( iNetworkType, &iHINLimit, &pacHINComment );
   ad_schk_GetNameLengthLimit( iNetworkType, &iNameLengthLimit, &pacNameLengthComment );
   ad_schk_GetAdiSizeLimit( iNetworkType, &iADISizeLimit, &pacADISizeComment );

   /*----------------------------------------------------------------
   ** Checks applicable to the ADI list and the ADI numbers.
   **----------------------------------------------------------------
   */

   /*
   ** ADI 0 is reserved for padding purposes.
   */
   for( iAdiIndex = 0; iAdiIndex < iNumOfAdis; iAdiIndex++ )
   {
      if( pasAdiList[ iAdiIndex ].iInstance == 0 )
      {
         ad_schk_BeginErrorMessage();
         ABCC_PORT_printf( "Relative ADI entry 0x%04"PRIx16"/%"PRIu16": ADI number 0 is reserved.\n", iAdiIndex, iAdiIndex );
      }
   }

   if( iNumOfAdis > 1 )
   {
      /*
      ** The list must be sorted in incremental order, other functions in this
      ** implementation of the AD object will not work correctly otherwise.
      */
      for( iAdiIndex = 0; iAdiIndex < ( iNumOfAdis - 1 ); iAdiIndex++ )
      {
         if( pasAdiList[ iAdiIndex + 1 ].iInstance < pasAdiList[ iAdiIndex ].iInstance )
         {
            ad_schk_BeginErrorMessage();
            ABCC_PORT_printf( "ADI list is not sorted in incremental order.\n" );
            ABCC_PORT_printf( "Skipping remaining checks.\n" );
            goto PRINT_COUNT_AND_EXIT;
         }
      }

      /*
      ** Check that there are no duplicates, i.e. that each ADI number only
      ** appears once.
      */
      for( iAdiIndex = 0; iAdiIndex < ( iNumOfAdis - 1 ); iAdiIndex++ )
      {
         if( pasAdiList[ iAdiIndex + 1 ].iInstance == pasAdiList[ iAdiIndex ].iInstance )
         {
            ad_schk_BeginErrorMessage();
            ABCC_PORT_printf( "ADI 0x%04"PRIx16"/%"PRIu16" appears more than once.\n", pasAdiList[ iAdiIndex ].iInstance, pasAdiList[ iAdiIndex ].iInstance );
            ABCC_PORT_printf( "Skipping remaining checks.\n" );
            goto PRINT_COUNT_AND_EXIT;
         }
      }
   }

   /*
   ** Network-specific check of 'Number of instances' and 'Highest instance
   ** number'.
   */
   if( pacNetworkName != NULL )
   {
      if( iNumOfAdis > iNOILimit )
      {
         ad_schk_BeginWarningMessage();
         ABCC_PORT_printf( "'Number of ADIs' is larger than what is reachable via %s, the limit is %"PRIu16"/0x%04"PRIx16" ADIs.", pacNetworkName, iNOILimit, iNOILimit );
         if( pacNOIComment != NULL )
         {
            ABCC_PORT_printf( " %s", pacNOIComment );
         }
         ABCC_PORT_printf( "\n" );
      }

      for( iAdiIndex = 0; iAdiIndex < iNumOfAdis; iAdiIndex++ )
      {
         if( pasAdiList[ iAdiIndex ].iInstance > iHINLimit )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": This ADI is beyond the range accessible by %s, the limit is ADI 1/0x0001-%"PRIu16"/0x%04"PRIx16".", pacNetworkName, iHINLimit, iHINLimit );
            if( pacHINComment != NULL )
            {
               ABCC_PORT_printf( " %s", pacHINComment );
            }
            ABCC_PORT_printf( "\n" );
         }
      }
   }

   /*----------------------------------------------------------------
   ** Checks applicable to the individual ADI elements/fields.
   **----------------------------------------------------------------
   */
   for( iAdiIndex = 0; iAdiIndex < iNumOfAdis; iAdiIndex++ )
   {
      /*----------------------------------------------------------------
      ** 'Number of elements'
      **----------------------------------------------------------------
      */

      /*
      ** An ADI can only have 1..255 elements, and we skip the remaining checks
      ** if this fails.
      **
      ** NOTE:
      ** The test against '255' can not be true while bNumOfElements is an
      ** UINT8, but is present to catch invalid changes to the AD_AdiEntryType
      ** itself. The bNumOfElements must be an UINT8, and changing it to a
      ** wider type will still not allow for more than 255 elements in an ADI.
      */
      if( ( pasAdiList[ iAdiIndex ].bNumOfElements < 1 ) ||
          ( pasAdiList[ iAdiIndex ].bNumOfElements > 255 ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
         ABCC_PORT_printf( ": 'Number of elements' is invalid, skipping further checks on this ADI.\n" );
         continue;
      }

      if( iNetworkType == ABP_NW_TYPE_BIP )
      {
         if( pasAdiList[ iAdiIndex ].bNumOfElements > 1 )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": ADI has more than 1 element, this is not supported by BACnet.\n" );
         }
      }

      /*----------------------------------------------------------------
      ** 'Name'
      **----------------------------------------------------------------
      */

      /*
      ** All name strings must comply with the ISO 8859-1 character set, correct string
      ** translation is not guaranteed otherwise.
      */
      if( ( pasAdiList[ iAdiIndex ].pacName != NULL ) &&
          ( !ad_schk_IsNameStringIso88591( pasAdiList[ iAdiIndex ].pacName ) ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
         ABCC_PORT_printf( ": 'Name' is not compliant with the ISO 8859-1 character set.\n" );
      }
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            if( ( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].pacElementName != NULL ) &&
                ( !ad_schk_IsNameStringIso88591( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].pacElementName ) ) )
            {
               ad_schk_BeginErrorMessage();
               ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
               ABCC_PORT_printf( " element %"PRIu8": 'Name' is not compliant with the ISO 8859-1 character set.\n", bElementIndex );
            }
         }
      }
#endif

      /*
      ** The ADI and element names should not exceed X characters.
      */
      if( pasAdiList[ iAdiIndex ].pacName != NULL )
      {
         iTemp = (UINT16)strlen( pasAdiList[ iAdiIndex ].pacName );
         if( iTemp > ABP_MAX_MSG_DATA_BYTES )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": 'Name' is too long (%"PRIu16" characters) to fit in an ABCC40 message (%u characters)\n", iTemp, ABP_MAX_MSG_DATA_BYTES );
         }
         if( iTemp > ABCC_CFG_MAX_MSG_SIZE )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": 'Name' is too long (%"PRIu16" characters) to fit in a message buffer ('ABCC_CFG_MAX_MSG_SIZE', %u bytes).\n", iTemp, ABCC_CFG_MAX_MSG_SIZE );
         }
         if( ( pacNetworkName != NULL ) && ( iNameLengthLimit > 0 ) )
         {
            if( iTemp > iNameLengthLimit )
            {
               ad_schk_BeginWarningMessage();
               ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
               ABCC_PORT_printf( ": 'Name' will be truncated or dropped with %s.", pacNetworkName );
               if( pacNameLengthComment != NULL )
               {
                  ABCC_PORT_printf( " %s", pacNameLengthComment );
               }
               ABCC_PORT_printf( "\n" );
            }
         }
      }
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            if( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].pacElementName != NULL )
            {
               iTemp = (UINT16)strlen( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].pacElementName );
               if( iTemp > ABP_MAX_MSG_DATA_BYTES )
               {
                  ad_schk_BeginWarningMessage();
                  ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
                  ABCC_PORT_printf( " element %"PRIu8": 'Name' is too long (%"PRIu16" characters) to fit in an ABCC40 message (%u characters)\n", bElementIndex, iTemp, ABP_MAX_MSG_DATA_BYTES );
               }
               if( iTemp > ABCC_CFG_MAX_MSG_SIZE )
               {
                  ad_schk_BeginWarningMessage();
                  ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
                  ABCC_PORT_printf( " element %"PRIu8": 'Name' is too long (%"PRIu16" characters) to fit in a message buffer ('ABCC_CFG_MAX_MSG_SIZE', %u, bytes).\n", bElementIndex, iTemp, ABCC_CFG_MAX_MSG_SIZE );
               }
               if( ( pacNetworkName != NULL ) && ( iNameLengthLimit > 0 ) )
               {
                  if( iTemp > iNameLengthLimit )
                  {
                     ad_schk_BeginWarningMessage();
                     ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
                     ABCC_PORT_printf( " element %"PRIu8": 'Name' will be truncated or dropped with %s.", bElementIndex, pacNetworkName );
                     if( pacNameLengthComment != NULL )
                     {
                        ABCC_PORT_printf( " %s", pacNameLengthComment );
                     }
                     ABCC_PORT_printf( "\n" );
                  }
               }
            }
         }

         /*
         ** All element names together must fit in one message since that is
         ** how they are returned via the ABP_APPD_IA_ELEM_NAME instance
         ** attribute.
         */
         iTemp = 0;
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            if( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].pacElementName != NULL )
            {
               iTemp += (UINT16)strlen( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].pacElementName );
            }
            iTemp++;
         }
         iTemp--;
         if( iTemp > ABP_MAX_MSG_DATA_BYTES )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": The concatenated element names are too long (%"PRIu16" characters) to fit in an ABCC40 message (%u characters)\n", iTemp, ABP_MAX_MSG_DATA_BYTES );
         }
         if( iTemp > ABCC_CFG_MAX_MSG_SIZE )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": The concatenated element names are too long (%"PRIu16" characters) to fit in a message buffer ('ABCC_CFG_MAX_MSG_SIZE', %u characters)\n", iTemp, ABCC_CFG_MAX_MSG_SIZE );
         }
      }
#endif

      /*----------------------------------------------------------------
      ** 'Data type'
      **----------------------------------------------------------------
      */

      /*
      ** Check that only valid ABP data types are present.
      */
      iTemp = ad_schk_iErrorCount;
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            ad_schk_TestAdiDataType( &pasAdiList[ iAdiIndex ], bElementIndex, iNetworkType );
         }
      }
      else
#endif
      {
         ad_schk_TestAdiDataType( &pasAdiList[ iAdiIndex ], 0, iNetworkType );
      }
      if( iTemp != ad_schk_iErrorCount )
      {
         ABCC_PORT_printf( "Invalid data types found, skipping further checks on this ADI.\n" );
         continue;
      }

#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         /*
         ** ABP_ENUM can not be used in a struct ADI.
         */
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            if( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].bDataType == ABP_ENUM )
            {
               ad_schk_BeginErrorMessage();
               ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
               ABCC_PORT_printf( " element %"PRIu8": ABP_ENUM is not allowed in a struct ADI.\n", bElementIndex );
            }
         }
      }
#endif

#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      /*
      ** Check alignment for struct ADIs. The non-ABP_BITx/ABP_PADx types must
      ** be byte-aligned.
      */
      if( ( pasAdiList[ iAdiIndex ].psStruct != NULL ) &&
          ( pasAdiList[ iAdiIndex ].bNumOfElements > 1 ) )
      {
         iTemp = 0;
         for( bElementIndex = 0; bElementIndex < ( pasAdiList[ iAdiIndex ].bNumOfElements - 1 ); bElementIndex++ )
         {
            if( !ABP_Is_BITx( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].bDataType ) &&
                !ABP_Is_PADx( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].bDataType ) )
            {
               if( ( iTemp & 0x7 ) != 0 )
               {
                  ad_schk_BeginErrorMessage();
                  ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
                  ABCC_PORT_printf( " element %"PRIu8": Data type must be byte-aligned.\n", bElementIndex );
               }
            }
            iTemp += ABCC_GetDataTypeSizeInBits( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].bDataType );
         }
      }
#endif

      /*----------------------------------------------------------------
      ** 'Descriptor'
      **----------------------------------------------------------------
      */

      /*
      ** Check for invalid descriptor combinations.
      */
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            ad_schk_TestDescComb( &pasAdiList[ iAdiIndex ], bElementIndex, iNetworkType );
         }
      }
      else
#endif
      {
         ad_schk_TestDescComb( &pasAdiList[ iAdiIndex ], 0, iNetworkType );
      }

#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         /*
         ** Check that the main descriptor byte of a struct AD is consistent
         ** with the descriptor of the elements.
         **
         ** I.e. if at least one element in a struct ADI supports 'Get' the
         ** main descriptor byte shall also indicate 'Get', and so on.
         */
         bTemp = 0;
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            bTemp = bTemp | pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].bDesc;
         }
         if( bTemp != pasAdiList[ iAdiIndex ].bDesc )
         {
            ad_schk_BeginErrorMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( " Main 'Descriptor' value is not consistent with the 'Descriptor' values of the elements.\n" );
         }
      }
#endif

      /*----------------------------------------------------------------
      ** 'Value' and 'Properties'
      **----------------------------------------------------------------
      */

      /*
      ** Check that all non-PADx elements has a non-NULL value pointer.
      */
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         for( bElementIndex = 0; bElementIndex < pasAdiList[ iAdiIndex ].bNumOfElements; bElementIndex++ )
         {
            if( !ABP_Is_PADx( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].bDataType ) &&
                ( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].uData.sVOID.pxValuePtr == NULL ) )
            {
               ad_schk_BeginErrorMessage();
               ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
               ABCC_PORT_printf( " element %"PRIu8": 'Value' pointer for a non-ABP_PADx element is NULL.\n", bElementIndex );
               continue;
            }
            if( ABP_Is_PADx( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].bDataType ) &&
                ( pasAdiList[ iAdiIndex ].psStruct[ bElementIndex ].uData.sVOID.pxValuePtr != NULL ) )
            {
               ad_schk_BeginWarningMessage();
               ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
               ABCC_PORT_printf( " element %"PRIu8": 'Value' pointer for a ABP_PADx element is not NULL.\n", bElementIndex );
               continue;
            }
         }
         if( pasAdiList[ iAdiIndex ].uData.sVOID.pxValuePtr != NULL )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": 'Value' pointer for a struct ADI is not NULL.\n" );
         }
      }
      else
#endif
      {
         if( !ABP_Is_PADx( pasAdiList[ iAdiIndex ].bDataType ) &&
             ( pasAdiList[ iAdiIndex ].uData.sVOID.pxValuePtr == NULL ) )
         {
            ad_schk_BeginErrorMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( " element %"PRIu8": 'Value' pointer for a non-ABP_PADx element is NULL.\n", 0 );
            continue;
         }
         if( ABP_Is_PADx( pasAdiList[ iAdiIndex ].bDataType ) &&
             ( pasAdiList[ iAdiIndex ].uData.sVOID.pxValuePtr != NULL ) )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( " element %"PRIu8": 'Value' pointer for a ABP_PADx element is not NULL.\n", 0 );
            continue;
         }
      }

      /*
      ** Check that 'Value' is in-range considering the 'Properties', and that
      ** the 'Properties' are correct.
      */
      ad_schk_TestValueAndProps( &pasAdiList[ iAdiIndex ] );

      /*
      ** Check that the 'Value' will fit in an ABCC message, and if it breaks
      ** any network-specific size limits.
      */
      iTemp = GetAdiSizeInOctets( &pasAdiList[ iAdiIndex ] );
      if( iTemp > ABP_MAX_MSG_DATA_BYTES )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
         ABCC_PORT_printf( ": 'Value' is too long (%"PRIu16" bytes) to fit an ABCC40 message ('ABP_MAX_MSG_DATA_BYTES', %u bytes).\n", iTemp, ABP_MAX_MSG_DATA_BYTES );
      }
      if( iTemp > ABCC_CFG_MAX_MSG_SIZE )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
         ABCC_PORT_printf( ": 'Value' is too long (%"PRIu16" bytes) to fit in a message buffer ('ABCC_CFG_MAX_MSG_SIZE', %u bytes).\n", iTemp, ABCC_CFG_MAX_MSG_SIZE );
      }
      if( ( pacNetworkName != NULL ) && ( iADISizeLimit > 0 ) )
      {
         if( iTemp > iADISizeLimit )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": 'Value' is too long (%"PRIu16" bytes) for acyclic access with %s (%"PRIu16" bytes).", iTemp, pacNetworkName, iADISizeLimit );
            if( pacADISizeComment != NULL )
            {
               ABCC_PORT_printf( " %s", pacADISizeComment );
            }
            ABCC_PORT_printf( "\n" );
         }
      }

      /*
      ** Check if the size of PD-mappable ADIs is a multiple of 8 bits, and
      ** print a warning otherwise. Some networks can implicitly pad such ADIs
      ** during the PD map process, but other requires explict padding in the
      ** PD map for such ADIs.
      */
      if( pasAdiList[ iAdiIndex ].bDesc & ( ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_MAPPABLE_READ_PD ) )
      {
         iTemp = GetAdiSizeInBits( &pasAdiList[ iAdiIndex ], pasAdiList[ iAdiIndex ].bNumOfElements, 0 );
         if( ( iTemp & 0x7 ) != 0 )
         {
            ad_schk_BeginWarningMessage();
            ad_schk_PrintAdiHeader( &pasAdiList[ iAdiIndex ] );
            ABCC_PORT_printf( ": Size of PD-mappable ADI is not a multiple of 8 bits, manual padding in the PD map list will be required with some networks.\n" );
         }
      }
   }

PRINT_COUNT_AND_EXIT:

   ABCC_PORT_printf( "ADI check finished: %"PRIu16" errors, %"PRIu16" warnings.\n", ad_schk_iErrorCount, ad_schk_iWarningCount );

   return;
}

void AD_SCHK_TestPdMapList( const AD_MapType* pasPdMapList, const AD_AdiEntryType* pasAdiList, const UINT16 iNumOfAdis, const UINT16 iNetworkType )
{
   const char* pacNetworkName;

   UINT16 iMapIndex;
   UINT16 iAdiIndex;
#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
   UINT16 iElementIndex;
#endif
   UINT16 iReadEntries;
   UINT16 iWriteEntries;

   UINT16      iPdSizeLimit;
   const char* pacPdSizeComment;
   UINT16      iRdPdSize;
   UINT16      iWrPdSize;
   UINT16      iLastMappedAdi;

   UINT8 bTemp;

   ad_schk_iErrorCount = 0;
   ad_schk_iWarningCount = 0;

   if( ( pasPdMapList == NULL ) ||
       ( pasPdMapList[ 0 ].eDir == PD_END_MAP ) )
   {
      /*
      ** No PD map given or PD map is empty, which is legally OK.
      */
      goto PRINT_COUNT_AND_EXIT;
   }

   if( ( pasAdiList == NULL ) || ( iNumOfAdis == 0 ) )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "Invalid ADI list or ADI list size.\n" );
      goto PRINT_COUNT_AND_EXIT;
   }

   if( ad_schk_GetNwSpecSettings() != ABCC_EC_NO_ERROR )
   {
      goto PRINT_COUNT_AND_EXIT;
   }

   pacNetworkName = ad_schk_GetNetworkName( iNetworkType );

   /*----------------------------------------------------------------
   ** PD map sanity checks. This is to check that the PD map list is correct
   ** given the available ADIs.
   **----------------------------------------------------------------
   */

   iMapIndex = 0;
   iReadEntries = 0;
   iWriteEntries = 0;

   /*
   ** Network-independent checks.
   */

   while( pasPdMapList[ iMapIndex ].eDir != PD_END_MAP )
   {
      if( pasPdMapList[ iMapIndex ].iInstance == AD_MAP_PAD_ADI )
      {
         /*
         ** ADI 0 is 255 x ABP_PAD1, no need to check the remaining data.
         */
         iMapIndex++;
         continue;
      }

      /*
      ** Manual search for ADI 'x' in the given ADI list - we can't use the
      ** existing "GetAdiIndex()" as it operates on a global variable set by
      ** "AD_Init()" rather than the ADI list supplied to this function.
      */
      for( iAdiIndex = 0; iAdiIndex < iNumOfAdis; iAdiIndex++ )
      {
         if( pasAdiList[ iAdiIndex ].iInstance == pasPdMapList[ iMapIndex ].iInstance )
         {
            break;
         }
      }
      if( iAdiIndex == iNumOfAdis )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintPdEntryHeader( iMapIndex );
         ABCC_PORT_printf( ": ADI %"PRIu16" does not exist.\n", pasPdMapList[ iMapIndex ].iInstance );
         iMapIndex++;
         continue;
      }

      if( ( pasPdMapList[ iMapIndex ].eDir != PD_READ ) && ( pasPdMapList[ iMapIndex ].eDir != PD_WRITE ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintPdEntryHeader( iMapIndex );
         ABCC_PORT_printf( ": Illegal 'eDir' value (%d).\n", pasPdMapList[ iMapIndex ].eDir );
      }

      bTemp = pasPdMapList[ iMapIndex ].bNumElem;
      if( bTemp == AD_MAP_ALL_ELEM )
      {
         bTemp = pasAdiList[ iAdiIndex ].bNumOfElements;
      }

      if( bTemp > pasAdiList[ iAdiIndex ].bNumOfElements )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintPdEntryHeader( iMapIndex );
         ABCC_PORT_printf( ": 'bNumElem' (%"PRIu8") is larger than 'bNumOfElements' (%"PRIu8") for ADI %"PRIu16".\n", bTemp, pasAdiList[ iAdiIndex ].bNumOfElements, pasAdiList[ iAdiIndex ].iInstance );
      }
      else if( pasPdMapList[ iMapIndex ].bElemStartIndex > ( pasAdiList[ iAdiIndex ].bNumOfElements - 1 ) )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintPdEntryHeader( iMapIndex );
         ABCC_PORT_printf( ": 'bElemStartIndex' (%"PRIu8") is after the last element index (%"PRIu8") of ADI %"PRIu16".\n", pasPdMapList[ iMapIndex ].bElemStartIndex, pasAdiList[ iAdiIndex ].bNumOfElements - 1, pasAdiList[ iAdiIndex ].iInstance );
      }
      else if( ( pasPdMapList[ iMapIndex ].bElemStartIndex + bTemp ) > pasAdiList[ iAdiIndex ].bNumOfElements )
      {
         ad_schk_BeginErrorMessage();
         ad_schk_PrintPdEntryHeader( iMapIndex );
         ABCC_PORT_printf( ": 'bElemStartIndex' (%"PRIu8") + 'bNumElem' (%"PRIu8") goes beyond the last element index (%"PRIu8") of ADI %"PRIu16".\n", pasPdMapList[ iMapIndex ].bElemStartIndex, bTemp, pasAdiList[ iAdiIndex ].bNumOfElements - 1, pasAdiList[ iAdiIndex ].iInstance );
      }

#if ABCC_CFG_STRUCT_DATA_TYPE_ENABLED
      if( pasAdiList[ iAdiIndex ].psStruct != NULL )
      {
         for( iElementIndex = pasPdMapList[ iMapIndex ].bElemStartIndex;
              iElementIndex < ( pasPdMapList[ iMapIndex ].bElemStartIndex + bTemp );
              iElementIndex++ )
         {
            if( ( pasPdMapList[ iMapIndex ].eDir == PD_READ ) &&
               !( pasAdiList[ iAdiIndex ].psStruct[ iElementIndex ].bDesc & ABP_APPD_DESCR_MAPPABLE_READ_PD ) )
            {
               ad_schk_BeginErrorMessage();
               ad_schk_PrintPdEntryHeader( iMapIndex );
               ABCC_PORT_printf( ": PD map requests PD_READ but descriptor bit 'RDPD mappable' is not set for ADI %"PRIu16", element %"PRIu8".\n", pasAdiList[ iAdiIndex ].iInstance, iElementIndex );
            }
            if( ( pasPdMapList[ iMapIndex ].eDir == PD_WRITE ) &&
               !( pasAdiList[ iAdiIndex ].psStruct[ iElementIndex ].bDesc & ABP_APPD_DESCR_MAPPABLE_WRITE_PD ) )
            {
               ad_schk_BeginErrorMessage();
               ad_schk_PrintPdEntryHeader( iMapIndex );
               ABCC_PORT_printf( ": PD map requests PD_WRITE but descriptor bit 'WRPD mappable' is not set for ADI %"PRIu16", element %"PRIu8".\n", pasAdiList[ iAdiIndex ].iInstance, iElementIndex );
            }
         }
      }
      else
#endif
      {
         if( ( pasPdMapList[ iMapIndex ].eDir == PD_READ ) &&
            !( pasAdiList[ iAdiIndex ].bDesc & ABP_APPD_DESCR_MAPPABLE_READ_PD ) )
         {
            ad_schk_BeginErrorMessage();
            ad_schk_PrintPdEntryHeader( iMapIndex );
            ABCC_PORT_printf( ": PD map requests PD_READ but descriptor bit 'RDPD mappable' is not set for ADI %"PRIu16".\n", pasAdiList[ iAdiIndex ].iInstance );
         }
         if( ( pasPdMapList[ iMapIndex ].eDir == PD_WRITE ) &&
            !( pasAdiList[ iAdiIndex ].bDesc & ABP_APPD_DESCR_MAPPABLE_WRITE_PD ) )
         {
            ad_schk_BeginErrorMessage();
            ad_schk_PrintPdEntryHeader( iMapIndex );
            ABCC_PORT_printf( ": PD map requests PD_WRITE but descriptor bit 'WRPD mappable' is not set for ADI %"PRIu16".\n", pasAdiList[ iAdiIndex ].iInstance );
         }
      }

      if( pasPdMapList[ iMapIndex ].eDir == PD_READ )
      {
         iReadEntries++;
      }
      if( pasPdMapList[ iMapIndex ].eDir == PD_WRITE )
      {
         iWriteEntries++;
      }

      iMapIndex++;
   }

   if( iReadEntries > AD_MAX_NUM_READ_MAP_ENTRIES )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "Too many PD_READ entries (%"PRIu16") given 'AD_MAX_NUM_READ_MAP_ENTRIES'.\n", iReadEntries );
   }
   if( iWriteEntries > AD_MAX_NUM_WRITE_MAP_ENTRIES )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "Too many PD_WRITE entries (%"PRIu16") given 'AD_MAX_NUM_WRITE_MAP_ENTRIES'.\n", iWriteEntries );
   }


   /*
   ** Network-specific checks.
   */

   switch( iNetworkType )
   {
   case ABP_NW_TYPE_PDPV1:
      /*
      ** With the default CfgData handling of the ABCC40 PROFIBUS DP-V1 the
      ** possible number of mapping operations depends on the size of the ADIs
      ** (larger ADIs requires more than one CfgData identifier), but it is in
      ** either case not possible to map more than 48 ADIs with the default
      ** translation model.
      */
      if( ( iReadEntries + iWriteEntries ) > 48 )
      {
         ad_schk_BeginWarningMessage();
         ABCC_PORT_printf( "PROFIBUS DP-V1 is limited to 48 mapping operations.\n" );
      }
      break;
   case ABP_NW_TYPE_BIP:
      /*
      ** With BACnet the 'number of mapped ADIs' has assymetic limits, RDPD is
      ** not supported while WRPD mapping is used to allow 'COV notification'
      ** for an ADI.
      */
      if( iReadEntries > 0 )
      {
         ad_schk_BeginErrorMessage();
         ABCC_PORT_printf( "BACnet does not support RDPD mapping.\n" );
      }
      if( iWriteEntries > 64 )
      {
         ad_schk_BeginErrorMessage();
         ABCC_PORT_printf( "BACnet does not support WRPD mapping (COV notification) for more than 64 ADIs.\n" );
      }
      break;
   default:
      break;
   }

   /*
   ** Skip the remaining tests if any errors were detected above. The PD map
   ** list must be OK for the remaining tests to be reliable.
   */
   if( ad_schk_iErrorCount > 0 )
   {
      ad_schk_BeginWarningMessage();
      ABCC_PORT_printf( "Skipping remaining PD checks due to PD map inconsistencies.\n" );
      goto PRINT_COUNT_AND_EXIT;
   }

   /*----------------------------------------------------------------
   ** PD size sanity checks.
   **----------------------------------------------------------------
   */

   ad_schk_GetPdSizeLimit( iNetworkType, &iPdSizeLimit, &pacPdSizeComment );
   iRdPdSize = 0;
   iWrPdSize = 0;
   iLastMappedAdi = ~pasPdMapList[ 0 ].iInstance;
   iMapIndex = 0;

   /*
   ** Calculate the actual RDPD and WRPD sizes given the PD map and the ADIs.
   */

   while( pasPdMapList[ iMapIndex ].eDir != PD_END_MAP )
   {
      /*
      ** Check that we are on a byte boundary every time that the PD map list
      ** changes to a 'new' ADI number. Some networks may implicitly pad to
      ** byte boundaries, but other requires explict padding in the PD map
      ** list.
      */
      if( ( pasPdMapList[ iMapIndex ].iInstance != iLastMappedAdi ) &&
          ( pasPdMapList[ iMapIndex ].iInstance != AD_MAP_PAD_ADI ) )
      {
         iLastMappedAdi = pasPdMapList[ iMapIndex ].iInstance;
         if( pasPdMapList[ iMapIndex ].eDir == PD_READ )
         {
            if( ( iRdPdSize & 0x7 ) != 0 )
            {
               ad_schk_BeginWarningMessage();
               ad_schk_PrintPdEntryHeader( iMapIndex );
               ABCC_PORT_printf( ": RDPD map entry does not start on a byte boundry.\n" );
            }
         }
         else
         {
            if( ( iWrPdSize & 0x7 ) != 0 )
            {
               ad_schk_BeginWarningMessage();
               ad_schk_PrintPdEntryHeader( iMapIndex );
               ABCC_PORT_printf( ": WRPD map entry does not start on a byte boundry.\n" );
            }
         }
      }

      if( pasPdMapList[ iMapIndex ].iInstance != AD_MAP_PAD_ADI )
      {
         for( iAdiIndex = 0; iAdiIndex < iNumOfAdis; iAdiIndex++ )
         {
            if( pasAdiList[ iAdiIndex ].iInstance == pasPdMapList[ iMapIndex ].iInstance )
            {
               break;
            }
         }
         /*
         ** An explicit check for 'ADI not found' is deliberately left out
         ** here, this should have been caught by earlier tests and we should
         ** not be here if that check failed.
         */

         bTemp = pasPdMapList[ iMapIndex ].bNumElem;
         if( bTemp == AD_MAP_ALL_ELEM )
         {
            bTemp = pasAdiList[ iAdiIndex ].bNumOfElements;
         }
         if( pasPdMapList[ iMapIndex ].eDir == PD_READ )
         {
            iRdPdSize += GetAdiSizeInBits( &pasAdiList[ iAdiIndex ], bTemp, pasPdMapList[ iMapIndex ].bElemStartIndex );
         }
         else
         {
            iWrPdSize += GetAdiSizeInBits( &pasAdiList[ iAdiIndex ], bTemp, pasPdMapList[ iMapIndex ].bElemStartIndex );
         }
      }
      else
      {
         /*
         ** ADI 0 / PAD_ADI is '255 x ABP_PAD1', so just add the number of
         ** elements to the sum.
         */
         if( pasPdMapList[ iMapIndex ].eDir == PD_READ )
         {
            iRdPdSize += pasPdMapList[ iMapIndex ].bNumElem;
         }
         else
         {
            iWrPdSize += pasPdMapList[ iMapIndex ].bNumElem;
         }
      }

      iMapIndex++;
   }
   iRdPdSize = SizeInOctets( 0, iRdPdSize );
   iWrPdSize = SizeInOctets( 0, iWrPdSize );

   if( iRdPdSize > ABCC_CFG_MAX_PROCESS_DATA_SIZE )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "RDPD size implied by the PD map (%"PRIu16" bytes) is larger than 'ABCC_CFG_MAX_PROCESS_DATA_SIZE' (%"PRIu16" bytes).\n", iRdPdSize, ABCC_CFG_MAX_PROCESS_DATA_SIZE );
   }
   if( iWrPdSize > ABCC_CFG_MAX_PROCESS_DATA_SIZE )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "WRPD size implied by the PD map (%"PRIu16" bytes) is larger than 'ABCC_CFG_MAX_PROCESS_DATA_SIZE' (%"PRIu16" bytes).\n", iWrPdSize, ABCC_CFG_MAX_PROCESS_DATA_SIZE );
   }

#if ABCC_CFG_DRV_SERIAL_ENABLED
   if( iRdPdSize > ABP_MAX_PROCESS_DATA )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "RDPD size implied by the PD map (%"PRIu16" bytes) is larger than the ABCC30-compatible OpMode supports (256 bytes).\n", iRdPdSize );
   }
   if( iWrPdSize > ABP_MAX_PROCESS_DATA )
   {
      ad_schk_BeginErrorMessage();
      ABCC_PORT_printf( "WRPD size implied by the PD map (%"PRIu16" bytes) is larger than the ABCC30-compatible OpMode modes supports (256 bytes).\n", iWrPdSize );
   }
#endif

   /*
   ** Network-specific PD size checks.
   */

   if( pacNetworkName != NULL )
   {
      if( iRdPdSize > iPdSizeLimit )
      {
         ad_schk_BeginWarningMessage();
         ABCC_PORT_printf( "RDPD size implied by the PD map (%"PRIu16" bytes) map is larger than the %s limit (%"PRIu16" bytes).", iRdPdSize, pacNetworkName, iPdSizeLimit );
         if( pacPdSizeComment != NULL )
         {
            ABCC_PORT_printf( " %s", pacPdSizeComment );
         }
         ABCC_PORT_printf( "\n" );
      }

      if( iWrPdSize > iPdSizeLimit )
      {
         ad_schk_BeginWarningMessage();
         ABCC_PORT_printf( "WRPD size implied by the PD map (%"PRIu16" bytes) map is larger than the %s limit (%"PRIu16" bytes).", iWrPdSize, pacNetworkName, iPdSizeLimit );
         if( pacPdSizeComment != NULL )
         {
            ABCC_PORT_printf( " %s", pacPdSizeComment );
         }
         ABCC_PORT_printf( "\n" );
      }
   }

   if( ( iNetworkType == ABP_NW_TYPE_PIR ) ||
       ( iNetworkType == ABP_NW_TYPE_PIR_FO ) ||
       ( iNetworkType == ABP_NW_TYPE_PIR_IIOT ) ||
       ( iNetworkType == ABP_NW_TYPE_PIR_FO_IIOT ) )
   {
      UINT8  bSubmoduleCount;

      /*
      ** With the ABCC40 PROFINET the module must add the IOPS/IOCS bytes to the
      ** existing PD, which can increase the PD size beyond the 'raw' 1440 bytes
      ** that PROFINET allows. IOxS needs to be added for all submodules before
      ** this can be checked.
      */

      /*
      ** There is one IOxS pair for each submodule (DAP, Interface, Port 1, and
      ** Port 2) in the DAP module in Slot 0. This is added automatically by the
      ** ABCC40.
      */
      iRdPdSize += ( 4 * ABP_OCTET_SIZEOF );
      iWrPdSize += ( 4 * ABP_OCTET_SIZEOF );

      /*
      ** Each PD mapping operation will result in one submodule being added, with
      ** one IOxS pair for each submodule.
      */
      bSubmoduleCount = 0;
      iMapIndex = 0;
      while( pasPdMapList[ iMapIndex ].eDir != PD_END_MAP )
      {
         if( pasPdMapList[ iMapIndex ].iInstance == AD_MAP_PAD_ADI )
         {
            iMapIndex++;
            continue;
         }

         iRdPdSize += ABP_OCTET_SIZEOF;
         iWrPdSize += ABP_OCTET_SIZEOF;
         bSubmoduleCount++;

         iMapIndex++;
      }

      if( iRdPdSize > 1440 )
      {
         ad_schk_BeginErrorMessage();
         ABCC_PORT_printf( "Actual RDPD size implied by the PD map list (%"PRIu16" bytes) is too large for PROFINET (1440 bytes).\n", iRdPdSize );
      }
      if( iWrPdSize > 1440 )
      {
         ad_schk_BeginErrorMessage();
         ABCC_PORT_printf( "Actual WRPD size implied by the PD map list (%"PRIu16" bytes) is too large for PROFINET (1440 bytes).\n", iWrPdSize );
      }
      if( bSubmoduleCount > 128 )
      {
         ad_schk_BeginErrorMessage();
         ABCC_PORT_printf( "Submodule count implied by the PD map list (%"PRIu8") is too large for the ABCC40 PROFINET (128 submodules).\n", bSubmoduleCount );
      }
   }

PRINT_COUNT_AND_EXIT:

   ABCC_PORT_printf( "PD map check finished: %"PRIu16" errors, %"PRIu16" warnings.\n", ad_schk_iErrorCount, ad_schk_iWarningCount );

   return;
}
#endif
