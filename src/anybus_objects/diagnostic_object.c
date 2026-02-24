/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Source file for the Diagnostic object.
********************************************************************************
*/

#include "abcc.h"

#if DI_OBJ_ENABLE

#include "diagnostic_object.h"

/*******************************************************************************
** Private constants.
********************************************************************************
*/

#define DI_EXT_DIAG_BLOCK_LEN                         ( ABP_UINT16_SIZEOF + ABP_UINT16_SIZEOF + ABP_UINT8_SIZEOF + ABP_UINT8_SIZEOF + ABP_UINT16_SIZEOF )

#if PRT_OBJ_ENABLE

#define DI_PRT_CHAN_DIAG_LEN                          ( ABP_UINT16_SIZEOF + ABP_UINT16_SIZEOF + ABP_UINT16_SIZEOF + ABP_UINT16_SIZEOF )
#define DI_PRT_EXT_CHAN_DIAG_LEN                      ( ABP_UINT16_SIZEOF + ABP_UINT32_SIZEOF )

#define DI_PRT_USI_CHAN_DIAG                          ( 0x8000U )
#define DI_PRT_USI_EXT_CHAN_DIAG                      ( 0x8002U )
#define DI_PRT_USI_QUAL_CHAN_DIAG                     ( 0x8003U )

/*
** Private masks for the 'ChannelProperties' field, a caller is not supposed to
** set or use those directly.
*/
#define DI_PRT_CHAN_PROP_MAINT_QUALIFIED              ( 0x0600U )
#define DI_PRT_CHAN_PROP_SPEC_APPEARS                 ( 0x0800U )
#define DI_PRT_CHAN_PROP_SPEC_MASK                    ( 0x1800U )

#endif /* PRT_OBJ_ENABLE */

/*******************************************************************************
** Private typedefs
********************************************************************************
*/

/*******************************************************************************
** Private forward declarations
********************************************************************************
*/

/*******************************************************************************
** Private Globals
********************************************************************************
*/

/*******************************************************************************
** Public Globals
********************************************************************************
*/

/*******************************************************************************
** Private Services
********************************************************************************
*/

#if PRT_OBJ_ENABLE

/*------------------------------------------------------------------------------
** Perform some basic sanity checks on the PROFINET-specific values that goes
** into a ChannelDiagnosis of ExtChannelDiagnosis structure.
**------------------------------------------------------------------------------
** Arguments:
**    iChannelNumber, iChannelProperties, iChannelErrorType,
**    iExtChannelErrorType, lExtChannelAddValue - The combination of PROFINET
**    diagnosis values to check.
** Returns:
**    TRUE if all checks passed, FALSE otherwise.
**------------------------------------------------------------------------------
*/

static BOOL di_prt_ValidateChannelDiagArgs( UINT16 iChannelNumber, UINT16 iChannelProperties, UINT16 iChannelErrorType )
{
   if( iChannelNumber > DI_PRT_CHAN_NUM_ENTIRE_SUBMODULE )
   {
      /*
      ** ChannelNumber must be 0x0000-0x7FFF, or Submodule (0x8000).
      ** PROFINET International doc.no 2.722, table 743.
      */
      return( FALSE );
   }

   if( ( iChannelNumber == DI_PRT_CHAN_NUM_ENTIRE_SUBMODULE ) &&
       ( ( iChannelProperties & DI_PRT_CHAN_PROP_ACC_MASK ) == DI_PRT_CHAN_PROP_ACC_MULTI ) )
   {
      /*
      ** ChannelNumber == Submodule is only allowed if
      ** ChannelProperties.Accumulative is zero.
      */
      return( FALSE );
   }

   if( ( iChannelProperties & DI_PRT_CHAN_PROP_TYPE_MASK ) > DI_PRT_CHAN_PROP_TYPE_64BIT )
   {
      /*
      ** ChannelProperties.Type must be 0x00-0x07.
      ** PROFINET International doc.no 2.722, table 744.
      */
      return( FALSE );
   }

   if( ( ( iChannelProperties & DI_PRT_CHAN_PROP_TYPE_MASK ) != DI_PRT_CHAN_PROP_TYPE_UNDEF ) &&
       ( iChannelNumber == DI_PRT_CHAN_NUM_ENTIRE_SUBMODULE ) )
   {
      /*
      ** ChannelProperties.Type must be Undefined if ChannelNumber is Submodule.
      ** PROFINET International doc.no 2.722, table 744.
      */
      return( FALSE );
   }

   if( ( iChannelProperties & DI_PRT_CHAN_PROP_MAINT_MASK ) == DI_PRT_CHAN_PROP_MAINT_QUALIFIED )
   {
      /*
      ** The caller can not select a ChannelQualifier at this stage, use
      ** DI_PRT_BuildCmdAddExtChannelDiag() and DI_PRT_BuildCmdAddQualChannelDiag()
      ** in that order to add a ChannelQualifier to the diag. data.
      */
      return( FALSE );
   }

   if( ( iChannelProperties & DI_PRT_CHAN_PROP_SPEC_MASK ) != 0 )
   {
      /*
      ** The caller can not select the ChannelProperties.Specifier value.
      ** It must be locked to Appears, and the ABCC generates the Disappears
      ** when the associated DI instance is deleted.
      */
      return( FALSE );
   }

   if( ( iChannelProperties & DI_PRT_CHAN_PROP_DIR_MASK ) > DI_PRT_CHAN_PROP_DIR_INOUT )
   {
      /*
      ** ChannelProperties.Direction must be 0x00-0x03.
      ** PROFINET International doc.no 2.722, table 750.
      */
      return( FALSE );
   }

   /*
   ** ChannelErrorType "Range 1"
   ** PROFINET International doc.no 2.722, table 739.
   **
   ** ChannelErrorType "Range 2"
   ** PROFINET International doc.no 2.722, table 740.
   **
   ** ChannelErrorType "Range 3"
   ** PROFINET International doc.no 2.722, table 741.
   **
   ** ChannelErrorType "Range 4"
   ** PROFINET International doc.no 2.722, table 742.
   */
   if( ( iChannelErrorType == 0x0000 ) ||
       ( ( iChannelErrorType >= 0x0040 ) && ( iChannelErrorType <= 0x004F ) ) ||
       ( ( iChannelErrorType >= 0x0050 ) && ( iChannelErrorType <= 0x005F ) ) ||
       ( iChannelErrorType == 0x7000 ) ||
       ( ( iChannelErrorType >= 0x8000 ) && ( iChannelErrorType <= 0x8FFF ) ) ||
       ( ( iChannelErrorType >= 0xA000 ) && ( iChannelErrorType <= 0xFFFF ) ) )
   {
      /*
      ** 0x0000 is reserved.
      ** 0x0040-0x004F is for Functional safety faults, the ABCC handles that.
      ** 0x0050-0x005F is for Functional safety faults, the ABCC handles that.
      ** 0x7000 is used by the ABCC to report SYNC-related faults.
      ** 0x8000-0x8010 is for interface/media faults, the ABCC handles that.
      ** 0x8011-0x8FFF are reserved.
      ** 0xA000-0xFFFF are reserved.
      */
      return( FALSE );
   }

   return( TRUE );
}

static BOOL di_prt_ValidateExtChannelDiagArgs( UINT16 iChannelErrorType, UINT16 iExtChannelErrorType, UINT32 lExtChannelAddValue )
{
   /*
   ** PROFINET International doc.no 2.722, table 754.
   */
   if( ( iChannelErrorType == 0x000F ) ||
       ( iChannelErrorType == 0x0010 ) )
   {
      if( ( iExtChannelErrorType == 0x8001 ) ||
          ( iExtChannelErrorType == 0x8002 ) )
      {
         /*
         ** PROFINET International doc.no 2.722, chapter 5.2.8.6.2.2 and 5.2.8.6.2.3.
         ** ExtChannelAddValue is an Index/Offset pair, we can't validate
         ** them here.
         */

         (void)lExtChannelAddValue;

         return( TRUE );
      }
   }

   if( ( ( iChannelErrorType >= 0x0000 ) && ( iChannelErrorType <= 0x00FF ) ) ||
       ( ( iChannelErrorType >= 0x0100 ) && ( iChannelErrorType <= 0x7FFF ) ) )
   {
      /*
      ** PROFINET International doc.no 2.722, table 753.
      ** PROFINET International doc.no 2.722, table 755.
      */
      if( ( iExtChannelErrorType == 0x0000 ) ||
          ( ( iExtChannelErrorType >= 0x0001 ) && ( iExtChannelErrorType <= 0x7FFF ) ) ||
          ( ( iExtChannelErrorType >= 0x9000 ) && ( iExtChannelErrorType <= 0x9FFF ) ) )
      {
         return( TRUE );
      }

      /*
      ** PROFINET International doc.no 2.722, table 753.
      ** PROFINET International doc.no 2.722, table 755.
      */
      if( iExtChannelErrorType == 0x8000 )
      {
         /*
         ** PROFINET International doc.no 2.722, chapter 5.2.8.6.2.1.
         ** ExtChannelAddValue is a bitfield for Accumulative info, we can't
         ** validate that here.
         */

         (void)lExtChannelAddValue;

         return( TRUE );
      }
   }

   return( FALSE );
}

#endif /* PRT_OBJ_ENABLE */

/*******************************************************************************
** Public Services
********************************************************************************
*/

/*------------------------------------------------------------------------------
** See "diagnostic_object.h" for information about the public calls.
**------------------------------------------------------------------------------
*/

ABCC_ErrorCodeType DI_BuildCmdCreateDiag( ABP_MsgType* psMsg, ABP_DiEventSeverityType eSeverity, ABP_DiEventCodeType eEvent )
{
   if( psMsg == NULL )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   if( ( eSeverity & ABP_DI_CREATE_CMDEXT0_EXT_DIAG_BIT ) != 0 )
   {
      /*
      ** The caller is supposed to use DI_BuildCmdAddExtDiag() to add
      ** the standard extended diag. data.
      */
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   /*
   ** Sanity checks passed, update the message buffer.
   */

   ABCC_SetMsgDataSize( psMsg, 0 );
   ABCC_SetMsgDestObj( psMsg, ABP_OBJ_NUM_DI );
   ABCC_SetMsgInstance( psMsg, ABP_INST_OBJ );
   ABCC_SetMsgCmdField( psMsg, ABP_MSG_HEADER_C_BIT | ABP_CMD_CREATE );

   ABCC_SetMsgCmdExt0( psMsg, (UINT8)eSeverity );
   ABCC_SetMsgCmdExt1( psMsg, (UINT8)eEvent );

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType DI_BuildCmdAddExtDiag( ABP_MsgType* psMsg, UINT16 iSlot, UINT16 iAdi, UINT8 bElement, UINT8 bBit )
{
   UINT8  bTemp;
   UINT16 iOffset;

   if( psMsg == NULL )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iOffset = ABCC_GetMsgDataSize( psMsg );

   /*
   ** DI_BuildCmdAddExtDiag() is supposed to be called after
   ** DI_BuildCmdCreateDiag() has written its data to the message buffer.
   ** Check that the buffer has the expected values in place.
   */
   if( ( iOffset != 0 ) ||
       ( ABCC_GetMsgDestObj( psMsg ) != ABP_OBJ_NUM_DI ) ||
       ( ABCC_GetMsgInstance( psMsg ) != ABP_INST_OBJ ) ||
       ( ABCC_GetMsgCmdField( psMsg ) != ( ABP_MSG_HEADER_C_BIT | ABP_CMD_CREATE ) ) ||
       ( ( ABCC_GetMsgCmdExt0( psMsg ) & ABP_DI_CREATE_CMDEXT0_EXT_DIAG_BIT ) != 0 ) )
   {
      return( ABCC_EC_INCORRECT_STATE );
   }

   /*
   ** Sanity checks passed, update the message buffer.
   */

   bTemp = ABCC_GetMsgCmdExt0( psMsg );
   bTemp |= ABP_DI_CREATE_CMDEXT0_EXT_DIAG_BIT;
   ABCC_SetMsgCmdExt0( psMsg, bTemp );

   ABCC_SetMsgData16( psMsg, iSlot, iOffset );
   iOffset += ABP_UINT16_SIZEOF;

   ABCC_SetMsgData16( psMsg, iAdi, iOffset );
   iOffset += ABP_UINT16_SIZEOF;

   ABCC_SetMsgData8( psMsg, bElement, iOffset );
   iOffset += ABP_UINT8_SIZEOF;

   ABCC_SetMsgData8( psMsg, bBit, iOffset );
   iOffset += ABP_UINT8_SIZEOF;

   ABCC_SetMsgData16( psMsg, 0, iOffset ); /* "Reserved, set to zero." */
   iOffset += ABP_UINT16_SIZEOF;

   ABCC_SetMsgDataSize( psMsg, iOffset );

   return( ABCC_EC_NO_ERROR );
}

#if PRT_OBJ_ENABLE

ABCC_ErrorCodeType DI_PRT_BuildCmdAddChannelDiag( ABP_MsgType* psMsg, UINT16 iChannelNumber, UINT16 iChannelProperties, UINT16 iChannelErrorType )
{
   UINT16 iOffset;

   if( psMsg == NULL )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   if( !di_prt_ValidateChannelDiagArgs( iChannelNumber, iChannelProperties, iChannelErrorType ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iOffset = ABCC_GetMsgDataSize( psMsg );

   /*
   ** DI_PRT_BuildCmdAddChannelDiag() is supposed to be called after
   ** DI_BuildCmdCreateDiag() and DI_BuildCmdAddExtDiag() has written their
   ** data to the message buffer. Check that the buffer has the expected values
   ** in place.
   */
   if( ( iOffset != DI_EXT_DIAG_BLOCK_LEN ) ||
       ( ABCC_GetMsgDestObj( psMsg ) != ABP_OBJ_NUM_DI ) ||
       ( ABCC_GetMsgInstance( psMsg ) != ABP_INST_OBJ ) ||
       ( ABCC_GetMsgCmdField( psMsg ) != ( ABP_MSG_HEADER_C_BIT | ABP_CMD_CREATE ) ) ||
       ( !( ABCC_GetMsgCmdExt0( psMsg ) & ABP_DI_CREATE_CMDEXT0_EXT_DIAG_BIT ) ) )
   {
      return( ABCC_EC_INCORRECT_STATE );
   }

   /*
   ** Sanity checks passed, update the message buffer.
   */

   ABCC_SetMsgCmdExt1( psMsg, ABP_DI_EVENT_NW_SPECIFIC );

   ABCC_SetMsgData16( psMsg, DI_PRT_USI_CHAN_DIAG, iOffset );
   iOffset += ABP_UINT16_SIZEOF;

   ABCC_SetMsgData16( psMsg, iTOiBe( iChannelNumber ), iOffset );
   iOffset += ABP_UINT16_SIZEOF;

   iChannelProperties = ( iChannelProperties & (UINT16)~( DI_PRT_CHAN_PROP_SPEC_MASK ) ) | DI_PRT_CHAN_PROP_SPEC_APPEARS;
   ABCC_SetMsgData16( psMsg, iTOiBe( iChannelProperties ), iOffset );
   iOffset += ABP_UINT16_SIZEOF;

   ABCC_SetMsgData16( psMsg, iTOiBe( iChannelErrorType ), iOffset );
   iOffset += ABP_UINT16_SIZEOF;

   ABCC_SetMsgDataSize( psMsg, iOffset );

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType DI_PRT_BuildCmdAddExtChannelDiag( ABP_MsgType* psMsg, UINT16 iExtChannelErrorType, UINT32 lExtChannelAddValue )
{
   UINT16 iOffset;
   UINT16 iTemp;

   if( psMsg == NULL )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iOffset = ABCC_GetMsgDataSize( psMsg );

   /*
   ** DI_PRT_BuildCmdAddExtChannelDiag() is supposed to be called after
   ** DI_PRT_BuildCmdAddChannelDiag() has written its data to the message
   ** buffer. Check that the buffer has the expected values in place.
   */
   if( ( iOffset != ( DI_EXT_DIAG_BLOCK_LEN + DI_PRT_CHAN_DIAG_LEN ) ) ||
       ( ABCC_GetMsgDestObj( psMsg ) != ABP_OBJ_NUM_DI ) ||
       ( ABCC_GetMsgInstance( psMsg ) != ABP_INST_OBJ ) ||
       ( ABCC_GetMsgCmdField( psMsg ) != ( ABP_MSG_HEADER_C_BIT | ABP_CMD_CREATE ) ) ||
       ( ABCC_GetMsgCmdExt1( psMsg ) != ABP_DI_EVENT_NW_SPECIFIC ) ||
       ( !( ABCC_GetMsgCmdExt0( psMsg ) & ABP_DI_CREATE_CMDEXT0_EXT_DIAG_BIT ) ) )
   {
      return( ABCC_EC_INCORRECT_STATE );
   }
   ABCC_GetMsgData16( psMsg, &iTemp, iOffset - DI_PRT_CHAN_DIAG_LEN );
   if( iTemp != DI_PRT_USI_CHAN_DIAG )
   {
      return( ABCC_EC_INCORRECT_STATE );
   }

   /*
   ** We need to recover the ChannelErrorType in order be able to check if
   ** ExtChannelErrorType and ExtChannelAddValue are valid.
   */
   ABCC_GetMsgData16( psMsg, &iTemp, iOffset - ABP_UINT16_SIZEOF );
   iTemp = iBeTOi( iTemp );
   if( !di_prt_ValidateExtChannelDiagArgs( iTemp, iExtChannelErrorType, lExtChannelAddValue ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   /*
   ** Sanity checks passed, update the message buffer.
   */

   ABCC_SetMsgData16( psMsg, DI_PRT_USI_EXT_CHAN_DIAG, iOffset - DI_PRT_CHAN_DIAG_LEN );

   ABCC_SetMsgData16( psMsg, iTOiBe( iExtChannelErrorType ), iOffset );
   iOffset += ABP_UINT16_SIZEOF;

   ABCC_SetMsgData32( psMsg, lTOlBe( lExtChannelAddValue ), iOffset );
   iOffset += ABP_UINT32_SIZEOF;

   ABCC_SetMsgDataSize( psMsg, iOffset );

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType DI_PRT_BuildCmdAddQualChannelDiag( ABP_MsgType* psMsg, UINT8 bChannelQualifier )
{
   UINT16 iOffset;
   UINT16 iTemp;
   UINT32 lTemp;

   if( psMsg == NULL )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   if( ( bChannelQualifier < DI_PRT_CHAN_QUAL_03_ADVICE ) || ( bChannelQualifier > DI_PRT_CHAN_QUAL_31_FAULT ) )
   {
      /*
      ** ChannelQualifier must be 3-31.
      ** PROFINET International doc.no 2.722, table 781.
      */
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iOffset = ABCC_GetMsgDataSize( psMsg );

   /*
   ** DI_PRT_BuildCmdAddQualChannelDiag() is supposed to be called after
   ** DI_PRT_BuildCmdAddExtChannelDiag() has written its data to the message
   ** buffer. Check that the buffer has the expected values in place.
   */
   if( ( iOffset != ( DI_EXT_DIAG_BLOCK_LEN + DI_PRT_CHAN_DIAG_LEN + DI_PRT_EXT_CHAN_DIAG_LEN ) ) ||
       ( ABCC_GetMsgDestObj( psMsg ) != ABP_OBJ_NUM_DI ) ||
       ( ABCC_GetMsgInstance( psMsg ) != ABP_INST_OBJ ) ||
       ( ABCC_GetMsgCmdField( psMsg ) != ( ABP_MSG_HEADER_C_BIT | ABP_CMD_CREATE ) ) ||
       ( ABCC_GetMsgCmdExt1( psMsg ) != ABP_DI_EVENT_NW_SPECIFIC ) ||
       ( !( ABCC_GetMsgCmdExt0( psMsg ) & ABP_DI_CREATE_CMDEXT0_EXT_DIAG_BIT ) ) )

   {
      return( ABCC_EC_INCORRECT_STATE );
   }
   ABCC_GetMsgData16( psMsg, &iTemp, iOffset - DI_PRT_EXT_CHAN_DIAG_LEN );
   if( iTemp != DI_PRT_USI_EXT_CHAN_DIAG )
   {
      return( ABCC_EC_INCORRECT_STATE );
   }

   /*
   ** Sanity checks passed, update the message buffer.
   */

   ABCC_SetMsgData16( psMsg, DI_PRT_USI_QUAL_CHAN_DIAG, iOffset - ( DI_PRT_CHAN_DIAG_LEN + DI_PRT_EXT_CHAN_DIAG_LEN ) );

   ABCC_GetMsgData16( psMsg, &iTemp, ( iOffset - ( DI_PRT_CHAN_DIAG_LEN + DI_PRT_EXT_CHAN_DIAG_LEN ) ) + ABP_UINT16_SIZEOF + ABP_UINT16_SIZEOF );
   iTemp = iBeTOi( iTemp );
   iTemp = ( iTemp & ~( DI_PRT_CHAN_PROP_MAINT_MASK ) ) | DI_PRT_CHAN_PROP_MAINT_QUALIFIED;
   ABCC_SetMsgData16( psMsg, iTOiBe( iTemp ), ( iOffset - ( DI_PRT_CHAN_DIAG_LEN + DI_PRT_EXT_CHAN_DIAG_LEN ) ) + ABP_UINT16_SIZEOF + ABP_UINT16_SIZEOF );

   lTemp = 1U << bChannelQualifier;
   ABCC_SetMsgData32( psMsg, lTOlBe( lTemp ), iOffset );
   iOffset += ABP_UINT32_SIZEOF;

   ABCC_SetMsgDataSize( psMsg, iOffset );

   return( ABCC_EC_NO_ERROR );
}

#endif /* PRT_OBJ_ENABLE */

#endif /* DI_OBJ_ENABLE */
