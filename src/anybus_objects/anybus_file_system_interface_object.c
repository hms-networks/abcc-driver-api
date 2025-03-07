/*******************************************************************************
** Copyright 2019-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Source file for the Anybus File System Interface object.
********************************************************************************
*/

#include "abcc_api_config.h"

#if ANB_FSI_OBJ_ENABLE

#include "abcc.h"
#include "abp_fsi.h"
#include "anybus_file_system_interface_object.h"

/*******************************************************************************
** Private constants.
********************************************************************************
*/

/*******************************************************************************
** Private typedefs
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Bookkeeping data for the FSI object.
**------------------------------------------------------------------------------
*/
typedef struct
{
   BOOL                      fInUse;
   UINT8                     bSrcId;
   ANB_FSI_CompletionCbfType pnCallback;

   union
   {
      struct
      {
         UINT32*  plFileSize;
      }
      sFClose;

      struct
      {
         UINT8*   pbDest;
         UINT16*  piActualSize;
      }
      sFRead;

      struct
      {
         UINT16*  piActualSize;
      }
      sFWrite;

      struct
      {
         ANB_FSI_DirEntryType*   psDirEntry;
      }
      sDRead;
   } uArgs;
}
anb_fsi_TransactionEntryType;

/*******************************************************************************
** Private forward declarations
********************************************************************************
*/

/*******************************************************************************
** Private Globals
********************************************************************************
*/

static anb_fsi_TransactionEntryType anb_fsi_TransactionList[ ANB_FSI_MAX_CONCURRENT_OPERATIONS ];

/*******************************************************************************
** Public Globals
********************************************************************************
*/

/*******************************************************************************
** Private Services
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Allocate a transaction entry.
**------------------------------------------------------------------------------
** Arguments:
**    -
** Returns:
**    Pointer to a transaction entry.
**    NULL if no free entry was available.
**------------------------------------------------------------------------------
*/
static anb_fsi_TransactionEntryType* anb_fsi_AllocTransactionEntry( void )
{
   int xIndex;

   for( xIndex = 0; xIndex < ANB_FSI_MAX_CONCURRENT_OPERATIONS; xIndex++ )
   {
      if( !anb_fsi_TransactionList[ xIndex ].fInUse )
      {
         anb_fsi_TransactionList[ xIndex ].fInUse = TRUE;

         return( &anb_fsi_TransactionList[ xIndex ] );
      }
   }

   return( NULL );
}

/*------------------------------------------------------------------------------
** Free a transaction entry
**------------------------------------------------------------------------------
** Arguments:
**    psEntry - Transaction entry to free.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void anb_fsi_FreeTransactionEntry( anb_fsi_TransactionEntryType* psEntry )
{
   if( !psEntry )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR,
         0,
         "Attempt to free NULL entry\n" );
   }

   psEntry->fInUse = FALSE;
   psEntry->bSrcId = 0;
   psEntry->pnCallback = NULL;
}

/*------------------------------------------------------------------------------
** Find transcation entry associated with a certain Source ID value.
**------------------------------------------------------------------------------
** Arguments:
**    bSrcId - Source ID value from the response message.
** Returns:
**    Pointer to transaction entry.
**    NULL if no match was found for the provided source ID.
**------------------------------------------------------------------------------
*/
static anb_fsi_TransactionEntryType* anb_fsi_FindTransactionEntry( UINT8 bSrcId )
{
   int xIndex;

   for( xIndex = 0; xIndex < ANB_FSI_MAX_CONCURRENT_OPERATIONS; xIndex++ )
   {
      if( ( anb_fsi_TransactionList[ xIndex ].fInUse ) &&
          ( anb_fsi_TransactionList[ xIndex ].bSrcId == bSrcId ) )
      {
         return( &anb_fsi_TransactionList[ xIndex ] );
      }
   }

   return( NULL );
}

/*------------------------------------------------------------------------------
** Sanity check and length calculation for filename/path strings.
**------------------------------------------------------------------------------
** Arguments:
**    pacString - Pointer to filename/path string buffer.
** Returns:
**    Size of string, or zero on error.
**------------------------------------------------------------------------------
*/
static UINT16 anb_fsi_ValidateFilePathName( const char* pacString )
{
   UINT16 iSize;

   if( pacString == NULL )
   {
      return( 0 );
   }

   iSize = (UINT16)strlen( pacString );
   if( ( iSize < 1 ) || ( iSize > ABP_FSI_MAX_PATH_LENGTH ) || ( iSize > ABCC_CFG_MAX_MSG_SIZE ) )
   {
      return( 0 );
   }

   return( iSize );
}

/*------------------------------------------------------------------------------
** Message handler for answers from the FSI object. This function is
** responsible for validating FSI answers, and routing the relevant data to the
** correct completion callback function.
**------------------------------------------------------------------------------
** Arguments:
**    psMsg - Pointer to message buffer.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
static void anb_fsi_MsgResponseHandler( ABP_MsgType* psMsg )
{
   ABP_MsgErrorCodeType          eMsgResult;
   UINT8                         bFsiError;
   UINT16                        iInstance;

   UINT16                        iDataSize;
   anb_fsi_TransactionEntryType* psEntry;

   psEntry = anb_fsi_FindTransactionEntry( ABCC_GetMsgSourceId( psMsg ) );
   if( !psEntry )
   {
      ABCC_LOG_FATAL( ABCC_EC_UNEXPECTED_NULL_PTR,
         0,
         "Failed to find transaction entry\n" );
   }

   iInstance = ABCC_GetMsgInstance( psMsg );

   if( ABCC_VerifyMessage( psMsg ) == ABCC_EC_NO_ERROR )
   {
      eMsgResult = ABP_ERR_NO_ERROR;
      bFsiError = 0;
   }
   else
   {
      eMsgResult = ABCC_GetErrorCode( psMsg );
      if( eMsgResult == ABP_ERR_OBJ_SPECIFIC )
      {
         ABCC_GetMsgData8( psMsg, &bFsiError, 1 );
      }
      else
      {
         bFsiError = 0;
      }
   }

   /*
   ** Some commands require extra operations or assignments before the
   ** completion callback function can be called.
   */
   switch( ABCC_GetMsgCmdBits( psMsg ) )
   {
      case ABP_CMD_CREATE:
         /*
         ** The instance number for the created instance is available in the
         ** data field, not in the 'iInstance' field in the message header.
         */
         if( eMsgResult == ABP_ERR_NO_ERROR )
         {
            ABCC_GetMsgData16( psMsg, &iInstance, 0 );
         }
         else
         {
            iInstance = 0;
         }
         break;

      case ABP_CMD_DELETE:
         /*
         ** The instance number of the deleted instance is available in the
         ** 'MsgExt' field, not in the 'iInstance' field header.
         */
         iInstance = ABCC_GetMsgCmdExt( psMsg );
         break;

      case ABP_FSI_CMD_FILE_CLOSE:
         /*
         ** FileClose has a 'total file size' value that should be propagated
         ** back to the caller.
         */
         if( eMsgResult == ABP_ERR_NO_ERROR )
         {
            ABCC_GetMsgData32( psMsg, psEntry->uArgs.sFClose.plFileSize, 0 );
         }
         else
         {
            *psEntry->uArgs.sFClose.plFileSize = 0;
         }
         break;

      case ABP_FSI_CMD_FILE_READ:
         /*
         ** A FileRead has data that needs to be copied back to the caller.
         ** Also propagate the actual size back, it is not certain that the
         ** response contained all the data that the caller asked for.
         */
         if( eMsgResult == ABP_ERR_NO_ERROR )
         {
            iDataSize = ABCC_GetMsgDataSize( psMsg );
            ABCC_GetMsgString( psMsg, (char*)psEntry->uArgs.sFRead.pbDest, iDataSize, 0 );
         }
         else
         {
            iDataSize = 0;
         }
         *psEntry->uArgs.sFRead.piActualSize = iDataSize;
         break;

      case ABP_FSI_CMD_FILE_WRITE:
         /*
         ** A FileWrite may not store the data size that the command asked
         ** for, propagate the actual size back to the caller.
         */
         if( eMsgResult == ABP_ERR_NO_ERROR )
         {
            iDataSize = ABCC_GetMsgCmdExt( psMsg );
         }
         else
         {
            iDataSize = 0;
         }
         *psEntry->uArgs.sFWrite.piActualSize = iDataSize;
         break;

      case ABP_FSI_CMD_DIRECTORY_READ:
         /*
         ** Propagate the directory entry data back to the caller.
         */
         if( eMsgResult == ABP_ERR_NO_ERROR )
         {
            iDataSize = ABCC_GetMsgDataSize( psMsg );
            if( iDataSize > 0 )
            {
               ABCC_GetMsgData32( psMsg, &(psEntry->uArgs.sDRead.psDirEntry->lSize), 0 );
               ABCC_GetMsgData8( psMsg, &(psEntry->uArgs.sDRead.psDirEntry->bFlags), ABP_UINT32_SIZEOF );
               iDataSize -= ( ABP_UINT32_SIZEOF + ABP_UINT8_SIZEOF );
               ABCC_GetMsgString( psMsg, psEntry->uArgs.sDRead.psDirEntry->acName, iDataSize, ABP_UINT32_SIZEOF + ABP_UINT8_SIZEOF );
               psEntry->uArgs.sDRead.psDirEntry->acName[ iDataSize ] = '\0';
            }
            else
            {
               /*
               ** No data - end of directory TOD
               */
               psEntry->uArgs.sDRead.psDirEntry->lSize = 0;
               psEntry->uArgs.sDRead.psDirEntry->bFlags = 0;
               psEntry->uArgs.sDRead.psDirEntry->acName[ 0 ] = '\0';
            }
         }
         else
         {
            iDataSize = 0;
         }
         break;

      default:
         /*
         ** Command needed no extra handling.
         */
         break;
   }

   {
      ANB_FSI_CompletionCbfType pnCallback;

      pnCallback = psEntry->pnCallback;
      anb_fsi_FreeTransactionEntry( psEntry );

      pnCallback( iInstance, eMsgResult, bFsiError );
   }

   return;
}

/*******************************************************************************
** Public Services
********************************************************************************
*/

void ANB_FSI_Init( void )
{
   int xIndex;

   for( xIndex = 0; xIndex < ANB_FSI_MAX_CONCURRENT_OPERATIONS; xIndex++ )
   {
      anb_fsi_TransactionList[ xIndex ].fInUse = FALSE;
      anb_fsi_TransactionList[ xIndex ].bSrcId = 0;
      anb_fsi_TransactionList[ xIndex ].pnCallback = NULL;
   }

   return;
}

ABCC_ErrorCodeType ANB_FSI_Create( ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   if( pnCallback == NULL )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, ABP_INST_OBJ, 0, ABP_CMD_CREATE, 0, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_Delete( UINT16 iInstance, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   if( ( iInstance == 0 ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, ABP_INST_OBJ, 0, ABP_CMD_DELETE, 0, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, iInstance );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_FileOpen( UINT16 iInstance, char* pacName, UINT8 bMode, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16                        iSize;

   if( ( iInstance == 0 ) ||
       ( pacName == NULL ) ||
       ( ( bMode != ABP_FSI_FILE_OPEN_READ_MODE ) &&
         ( bMode != ABP_FSI_FILE_OPEN_WRITE_MODE ) &&
         ( bMode != ABP_FSI_FILE_OPEN_APPEND_MODE ) ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSize = anb_fsi_ValidateFilePathName( pacName );
   if( iSize == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_FILE_OPEN, iSize, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt0( psMsg, bMode );
   ABCC_SetMsgCmdExt1( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacName, iSize, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_FileClose( UINT16 iInstance, UINT32* plFileSize, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   if( ( iInstance == 0 ) ||
       ( plFileSize == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_FILE_CLOSE, 0, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;
   psEntry->uArgs.sFClose.plFileSize = plFileSize;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_FileDelete( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16                        iSize;

   if( ( iInstance == 0 ) ||
       ( pacName == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSize = anb_fsi_ValidateFilePathName( pacName );
   if( iSize == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_FILE_DELETE, iSize, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacName, iSize, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_FileCopy( UINT16 iInstance, char* pacSrc, char* pacDest, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16                        iSizeSrc;
   UINT16                        iSizeDest;

   if( ( iInstance == 0 ) ||
       ( pacSrc == NULL ) ||
       ( pacDest == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSizeSrc = anb_fsi_ValidateFilePathName( pacSrc );
   if( iSizeSrc == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }
   iSizeDest = anb_fsi_ValidateFilePathName( pacDest );
   if( iSizeDest == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }
   if( ( iSizeSrc + ABP_CHAR_SIZEOF + iSizeDest ) > ABCC_CFG_MAX_MSG_SIZE )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_FILE_COPY, iSizeSrc + ABP_CHAR_SIZEOF + iSizeDest, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacSrc, iSizeSrc, 0 );
   ABCC_SetMsgData8( psMsg, 0, iSizeSrc ); /* NUL character */
   ABCC_SetMsgString( psMsg, pacDest, iSizeDest, iSizeSrc + ABP_CHAR_SIZEOF );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_FileRename( UINT16 iInstance, char* pacOld, char* pacNew, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16                        iSizeOld;
   UINT16                        iSizeNew;

   if( ( iInstance == 0 ) ||
       ( pacOld == NULL ) ||
       ( pacNew == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSizeOld = anb_fsi_ValidateFilePathName( pacOld );
   if( iSizeOld == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }
   iSizeNew = anb_fsi_ValidateFilePathName( pacNew );
   if( iSizeNew == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }
   if( ( iSizeOld + ABP_CHAR_SIZEOF + iSizeNew ) > ABCC_CFG_MAX_MSG_SIZE )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_FILE_RENAME, iSizeOld + ABP_CHAR_SIZEOF + iSizeNew, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacOld, iSizeOld, 0 );
   ABCC_SetMsgData8( psMsg, 0, iSizeOld ); /* NUL character */
   ABCC_SetMsgString( psMsg, pacNew, iSizeNew, iSizeOld + ABP_CHAR_SIZEOF );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_FileRead( UINT16 iInstance, UINT8* pbDest, UINT16 iReqSize, UINT16* piActSize, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   if( ( iInstance == 0 ) ||
       ( pbDest == NULL ) ||
       ( iReqSize == 0 ) ||
       ( iReqSize > ABCC_CFG_MAX_MSG_SIZE ) ||
       ( piActSize == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_FILE_READ, 0, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, iReqSize );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;
   psEntry->uArgs.sFRead.pbDest = pbDest;
   psEntry->uArgs.sFRead.piActualSize = piActSize;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_FileWrite( UINT16 iInstance, UINT8* pbSrc, UINT16 iReqSize, UINT16* piActSize, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   if( ( iInstance == 0 ) ||
       ( pbSrc == NULL ) ||
       ( iReqSize == 0 ) ||
       ( iReqSize > ABCC_CFG_MAX_MSG_SIZE ) ||
       ( piActSize == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_FILE_WRITE, iReqSize, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, (char*)pbSrc, iReqSize, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;
   psEntry->uArgs.sFWrite.piActualSize = piActSize;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_DirectoryOpen( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16       iSize;

   if( ( iInstance == 0 ) ||
       ( pacName == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSize = anb_fsi_ValidateFilePathName( pacName );
   if( iSize == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_DIRECTORY_OPEN, iSize, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacName, iSize, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_DirectoryClose( UINT16 iInstance, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   if( ( iInstance == 0 ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_DIRECTORY_CLOSE, 0, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_DirectoryDelete( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16                        iSize;

   if( ( iInstance == 0 ) ||
       ( pacName == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSize = anb_fsi_ValidateFilePathName( pacName );
   if( iSize == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_DIRECTORY_DELETE, iSize, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacName, iSize, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_DirectoryRead( UINT16 iInstance, ANB_FSI_DirEntryType* psDest, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   if( ( iInstance == 0 ) ||
       ( psDest == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_DIRECTORY_READ, 0, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;
   psEntry->uArgs.sDRead.psDirEntry = psDest;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_DirectoryCreate( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16                        iSize;

   if( ( iInstance == 0 ) ||
       ( pacName == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSize = anb_fsi_ValidateFilePathName( pacName );
   if( iSize == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_DIRECTORY_CREATE, iSize, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacName, iSize, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

ABCC_ErrorCodeType ANB_FSI_DirectoryChange( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback )
{
   ABP_MsgType*                  psMsg;
   anb_fsi_TransactionEntryType* psEntry;

   UINT16                        iSize;

   if( ( iInstance == 0 ) ||
       ( pacName == NULL ) ||
       ( pnCallback == NULL ) )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   iSize = anb_fsi_ValidateFilePathName( pacName );
   if( iSize == 0 )
   {
      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   psEntry = anb_fsi_AllocTransactionEntry();
   if( !psEntry )
   {
      return( ABCC_EC_NO_RESOURCES );
   }

   psMsg = ABCC_GetCmdMsgBuffer();
   if( psMsg == NULL )
   {
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_OUT_OF_MSG_BUFFERS );
   }

   ABCC_SetMsgHeader( psMsg, ABP_OBJ_NUM_FSI, iInstance, 0, ABP_FSI_CMD_DIRECTORY_CHANGE, iSize, ABCC_GetNewSourceId() );
   ABCC_SetMsgCmdExt( psMsg, 0 );
   ABCC_SetMsgString( psMsg, pacName, iSize, 0 );

   psEntry->bSrcId = ABCC_GetMsgSourceId( psMsg );
   psEntry->pnCallback = pnCallback;

   if( ABCC_SendCmdMsg( psMsg, anb_fsi_MsgResponseHandler ) != ABCC_EC_NO_ERROR )
   {
      ABCC_ReturnMsgBuffer( &psMsg );
      anb_fsi_FreeTransactionEntry( psEntry );
      return( ABCC_EC_NO_RESOURCES );
   }

   return( ABCC_EC_NO_ERROR );
}

#endif /* ANB_FSI_OBJ_ENABLE */
