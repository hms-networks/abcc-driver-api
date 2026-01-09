/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
*/

#include "abcc_api_select_firmware.h"

#if ANB_FSI_OBJ_ENABLE

#include "anybus_file_system_interface_object.h"
#include "abcc.h"

/*******************************************************************************
** Defines
********************************************************************************
*/

#define ABCC_API_FIRMWARE_SRC_FOLDER "Network FW"
#define ABCC_API_FIRMWARE_SRC_FOLDER_FILENAME_OFFSET ( sizeof( ABCC_API_FIRMWARE_SRC_FOLDER "\\" ) - 1 )
#define ABCC_API_FIRMWARE_DST_FOLDER "firmware"
#define ABCC_API_FIRMWARE_DST_FOLDER_FILENAME_OFFSET ( sizeof( ABCC_API_FIRMWARE_DST_FOLDER "\\" ) - 1 )

#if ABCC_API_SELECT_FIRMWARE_DEBUG_ENABLED
#define ABCC_API_SELECT_FIRMWARE_DEBUG_PRINT( ... ) ABCC_LOG_DEBUG( __VA_ARGS__ )
#else
#define ABCC_API_SELECT_FIRMWARE_DEBUG_PRINT( ... )
#endif

/*******************************************************************************
** Typedefs
********************************************************************************
*/
/*------------------------------------------------------------------------------
** Select firmware state.
**------------------------------------------------------------------------------
*/
typedef enum
{
   SELECT_FW_STATE_NOT_STARTED,
   SELECT_FW_STATE_STARTED,
   SELECT_FW_STATE_CREATE_INSTANCE_WAIT_RSP,
   SELECT_FW_STATE_DOPEN_WAIT_RSP,
   SELECT_FW_STATE_DREAD_WAIT_RSP,
   SELECT_FW_STATE_COPY_FILE_WAIT_RSP,
   SELECT_FW_STATE_DCLOSE_WAIT_RSP,
   SELECT_FW_STATE_DELETE_INSTANCE_WAIT_RSP,
}
appl_SelectFwState;

/*******************************************************************************
** Private Globals
********************************************************************************
*/
static appl_SelectFwState appl_eSelectFwState = SELECT_FW_STATE_NOT_STARTED;
static ABCC_API_CommonEtnFirmwareType appl_eTargetFirmware;
static ANB_FSI_DirEntryType appl_sDirectoryEntry = { 0 };
static ABCC_API_pnSelectFwResultCallback appl_pnResultCallback;
static const char* appl_acNetworkIdentifiers[] = { "PIR", "EIP", "ECT", "EIT" };

static char appl_acFirmwareFilesSrcFolder[ ABP_FSI_MAX_PATH_LENGTH + 1 ] = ABCC_API_FIRMWARE_SRC_FOLDER "\\";
static char appl_acFirmwareFilesDstFolder[ ABP_FSI_MAX_PATH_LENGTH + 1 ] = ABCC_API_FIRMWARE_DST_FOLDER "\\";

#if ABCC_API_SELECT_FIRMWARE_DEBUG_ENABLED
static const char* appl_acStateToText[] =
{
   "NOT_STARTED",
   "STARTED",
   "CREATE_INSTANCE_WAIT_RSP",
   "DOPEN_WAIT_RSP",
   "DREAD_WAIT_RSP",
   "COPY_FILE_WAIT_RSP",
   "DCLOSE_WAIT_RSP",
   "DELETE_INSTANCE_WAIT_RSP"
};
#endif

/*******************************************************************************
** Private forward declaration
********************************************************************************
*/
static void SetState( appl_SelectFwState eState );
static void FsiObjectResponse( UINT16 iInstance, ABP_MsgErrorCodeType eMsgResult, UINT8 bFsiError );
static void RunStateMachine( UINT16 iInstance );
static ABCC_ErrorCodeType CreateInstance( void );
static ABCC_ErrorCodeType DirectoryOpen( UINT16 iInstance );
static ABCC_ErrorCodeType DirectoryRead( UINT16 iInstance );
static ABCC_ErrorCodeType DirectoryReadNext( UINT16 iInstance );
static BOOL SetupFirmwareFilePaths( void );
static BOOL EndOfDirectoryReached( void );
static BOOL TargetFwFileFound( void );
static ABCC_ErrorCodeType DirectoryClose( UINT16 iInstance );
static ABCC_ErrorCodeType CopyFile( UINT16 iInstance );
static ABCC_ErrorCodeType DeleteInstance( UINT16 iInstance );
static void NotifyResult( ABCC_ErrorCodeType iResult );

/*******************************************************************************
** Private Services
********************************************************************************
*/

static BOOL IsDirectoryOpen( BOOL fInResponseCallback )
{
   if( ( !fInResponseCallback &&
         ( appl_eSelectFwState >= SELECT_FW_STATE_DOPEN_WAIT_RSP &&
           appl_eSelectFwState < SELECT_FW_STATE_DCLOSE_WAIT_RSP ) ) ||
       ( fInResponseCallback &&
         ( appl_eSelectFwState > SELECT_FW_STATE_DOPEN_WAIT_RSP &&
           appl_eSelectFwState < SELECT_FW_STATE_DCLOSE_WAIT_RSP ) ) )
   {
      return( TRUE );
   }

   return( FALSE );
}

static BOOL IsInstanceCreated( BOOL fInResponseCallback )
{
   if( ( !fInResponseCallback &&
         ( appl_eSelectFwState >= SELECT_FW_STATE_CREATE_INSTANCE_WAIT_RSP &&
           appl_eSelectFwState < SELECT_FW_STATE_DELETE_INSTANCE_WAIT_RSP ) ) ||
       ( fInResponseCallback &&
         ( appl_eSelectFwState > SELECT_FW_STATE_CREATE_INSTANCE_WAIT_RSP &&
           appl_eSelectFwState < SELECT_FW_STATE_DELETE_INSTANCE_WAIT_RSP ) ) )
   {
      return( TRUE );
   }

   return( FALSE );
}

static void CleanUp( UINT16 iInstance, BOOL fInResponseCallback )
{
   if( IsDirectoryOpen( fInResponseCallback ) )
   {
      if( DirectoryClose( iInstance ) != ABCC_EC_NO_ERROR )
      {
         SetState( SELECT_FW_STATE_NOT_STARTED );
      }
   }
   else if( IsInstanceCreated( fInResponseCallback ) )
   {
      if( DeleteInstance( iInstance ) != ABCC_EC_NO_ERROR )
      {
         SetState( SELECT_FW_STATE_NOT_STARTED );
      }
   }
   else
   {
      SetState( SELECT_FW_STATE_NOT_STARTED );
   }
}

static void SetState( appl_SelectFwState eState )
{
   appl_eSelectFwState = eState;
   ABCC_API_SELECT_FIRMWARE_DEBUG_PRINT( "Firmware select state %d (%s)\n",
      eState,
      appl_acStateToText[ eState ] );
}

static void FsiObjectResponse( UINT16 iInstance, ABP_MsgErrorCodeType eMsgResult, UINT8 bFsiError )
{
   (void)bFsiError;

   if( eMsgResult != ABP_ERR_NO_ERROR )
   {
      ABCC_API_SELECT_FIRMWARE_DEBUG_PRINT( "Command failed with error code %d (FSI error: %u)\n", eMsgResult, bFsiError );
      NotifyResult( ABCC_EC_INTERNAL_ERROR );
      CleanUp( iInstance, TRUE );
      return;
   }

   RunStateMachine( iInstance );
}

static ABCC_ErrorCodeType CreateInstance( void )
{
   ABCC_ErrorCodeType eError;

   eError = ANB_FSI_Create( FsiObjectResponse );

   if( eError == ABCC_EC_NO_ERROR )
   {
      SetState( SELECT_FW_STATE_CREATE_INSTANCE_WAIT_RSP );
   }

   return( eError );
}

static ABCC_ErrorCodeType DirectoryOpen( UINT16 iInstance )
{
   ABCC_ErrorCodeType eError;

   eError = ANB_FSI_DirectoryOpen( iInstance, ABCC_API_FIRMWARE_SRC_FOLDER, FsiObjectResponse );

   if( eError == ABCC_EC_NO_ERROR )
   {
      SetState( SELECT_FW_STATE_DOPEN_WAIT_RSP );
   }

   return( eError );
}

static ABCC_ErrorCodeType DirectoryRead( UINT16 iInstance )
{
   ABCC_ErrorCodeType eError;

   eError = ANB_FSI_DirectoryRead( iInstance, &appl_sDirectoryEntry, FsiObjectResponse );

   if( eError == ABCC_EC_NO_ERROR )
   {
      SetState( SELECT_FW_STATE_DREAD_WAIT_RSP );
   }

   return( eError );
}

static ABCC_ErrorCodeType DirectoryReadNext( UINT16 iInstance )
{
   if( EndOfDirectoryReached() )
   {
      ABCC_API_SELECT_FIRMWARE_DEBUG_PRINT( "Requested network not found in firmware repository\n" );

      return( ABCC_EC_PARAMETER_NOT_VALID );
   }

   if( !TargetFwFileFound() )
   {
      return( ANB_FSI_DirectoryRead( iInstance, &appl_sDirectoryEntry, FsiObjectResponse ) );
   }

   if( !SetupFirmwareFilePaths() )
   {
      return( ABCC_EC_INTERNAL_ERROR );
   }

   return( CopyFile( iInstance ) );
}

static BOOL EndOfDirectoryReached( void )
{
   if( appl_sDirectoryEntry.acName[ 0 ] == '\0' )
   {
      return( TRUE );
   }
   else
   {
      return( FALSE );
   }
}

static BOOL TargetFwFileFound( void )
{
   /*
   ** The firmware files are named according to the
   ** ABCC_40_(EIP|PIR|ECT|EIT)_.*\.hiff format which means the filename length
   ** has to be at least 11 (actually 17 but 11 is enough for the comparison).
   ** The network identifier starts at index 8.
   */
#define MINIMUM_FILENAME_LENGTH 11
#define NETWORK_IDENTIFIER_INDEX 8

   if( ( appl_sDirectoryEntry.bFlags & ABP_FSI_DIRECTORY_READ_DIRECTORY ) ||
      strlen( appl_sDirectoryEntry.acName ) < MINIMUM_FILENAME_LENGTH )
   {
      return( FALSE );
   }
   else if( strncmp( &appl_sDirectoryEntry.acName[ NETWORK_IDENTIFIER_INDEX ],
      appl_acNetworkIdentifiers[ appl_eTargetFirmware ],
      strlen( appl_acNetworkIdentifiers[ appl_eTargetFirmware ] ) ) == 0 )
   {
      return( TRUE );
   }
   else
   {
      return( FALSE );
   }
}

static BOOL SetupFirmwareFilePaths( void )
{
   size_t xSrcPathLength = ABCC_API_FIRMWARE_SRC_FOLDER_FILENAME_OFFSET + strlen( appl_sDirectoryEntry.acName ) + 1;
   size_t xDstPathLength = ABCC_API_FIRMWARE_DST_FOLDER_FILENAME_OFFSET + strlen( appl_sDirectoryEntry.acName ) + 1;

   if( xSrcPathLength > sizeof( appl_acFirmwareFilesSrcFolder ) )
   {
      return( FALSE );
   }

   strncpy( &appl_acFirmwareFilesSrcFolder[ ABCC_API_FIRMWARE_SRC_FOLDER_FILENAME_OFFSET ],
      appl_sDirectoryEntry.acName,
      strlen( appl_sDirectoryEntry.acName ) + 1 );

   if( xDstPathLength > sizeof( appl_acFirmwareFilesDstFolder ) )
   {
      return( FALSE );
   }

   strncpy( &appl_acFirmwareFilesDstFolder[ ABCC_API_FIRMWARE_DST_FOLDER_FILENAME_OFFSET ],
      appl_sDirectoryEntry.acName,
      strlen( appl_sDirectoryEntry.acName ) + 1 );

   return( TRUE );
}

static ABCC_ErrorCodeType DirectoryClose( UINT16 iInstance )
{
   ABCC_ErrorCodeType eError;
   eError = ANB_FSI_DirectoryClose( iInstance, FsiObjectResponse );

   if( eError == ABCC_EC_NO_ERROR )
   {
      SetState( SELECT_FW_STATE_DCLOSE_WAIT_RSP );
   }

   return( eError );
}

static ABCC_ErrorCodeType CopyFile( UINT16 iInstance )
{
   ABCC_ErrorCodeType eError;
   eError = ANB_FSI_FileCopy( iInstance, appl_acFirmwareFilesSrcFolder, appl_acFirmwareFilesDstFolder, FsiObjectResponse );

   if( eError == ABCC_EC_NO_ERROR )
   {
      SetState( SELECT_FW_STATE_COPY_FILE_WAIT_RSP );
   }

   return( eError );
}

static ABCC_ErrorCodeType DeleteInstance( UINT16 iInstance )
{
   ABCC_ErrorCodeType eError;
   eError = ANB_FSI_Delete( iInstance, FsiObjectResponse );

   if( eError == ABCC_EC_NO_ERROR )
   {
      SetState( SELECT_FW_STATE_DELETE_INSTANCE_WAIT_RSP );
   }

   return( eError );
}

static void RunStateMachine( UINT16 iInstance )
{
   ABCC_ErrorCodeType eErrorCode = ABCC_EC_NO_ERROR;

   switch( appl_eSelectFwState )
   {
   case SELECT_FW_STATE_NOT_STARTED:
   case SELECT_FW_STATE_STARTED:
      eErrorCode = ABCC_EC_INTERNAL_ERROR;
      ABCC_API_SELECT_FIRMWARE_DEBUG_PRINT( "Statemachine shouldn't run in this state\n" );
      break;
   case SELECT_FW_STATE_CREATE_INSTANCE_WAIT_RSP:
      eErrorCode = DirectoryOpen( iInstance );
      break;
   case SELECT_FW_STATE_DOPEN_WAIT_RSP:
      eErrorCode = DirectoryRead( iInstance );
      break;
   case SELECT_FW_STATE_DREAD_WAIT_RSP:
      eErrorCode = DirectoryReadNext( iInstance );
      break;
   case SELECT_FW_STATE_COPY_FILE_WAIT_RSP:
      eErrorCode = DirectoryClose( iInstance );
      break;
   case SELECT_FW_STATE_DCLOSE_WAIT_RSP:
      eErrorCode = DeleteInstance( iInstance );
      break;
   case SELECT_FW_STATE_DELETE_INSTANCE_WAIT_RSP:
      NotifyResult( eErrorCode );
      SetState( SELECT_FW_STATE_NOT_STARTED );
      break;
   default:
      eErrorCode = ABCC_EC_INTERNAL_ERROR;
      ABCC_API_SELECT_FIRMWARE_DEBUG_PRINT( "Non-existant state\n" );
      break;
   }

   if( eErrorCode != ABCC_EC_NO_ERROR )
   {
      NotifyResult( eErrorCode );
      CleanUp( iInstance, FALSE );
   }
}

static void NotifyResult( ABCC_ErrorCodeType eResult )
{
   if( appl_pnResultCallback )
   {
      appl_pnResultCallback( eResult );
   }

   /* Only notify user once per select firmware attempt */
   appl_pnResultCallback = NULL;
}

/*******************************************************************************
** Public Services
********************************************************************************
*/

void ABCC_API_SelectFirmware(
   ABCC_API_CommonEtnFirmwareType eFirmware,
   ABCC_API_pnSelectFwResultCallback pnResultCallback )
{

   if( eFirmware < 0 || eFirmware >= ABCC_API_NW_TYPE_LAST )
   {
      if( pnResultCallback )
      {
         pnResultCallback( ABCC_EC_PARAMETER_NOT_VALID );
      }
      return;
   }

   if( appl_eSelectFwState > SELECT_FW_STATE_NOT_STARTED )
   {
      if( pnResultCallback )
      {
         pnResultCallback( ABCC_EC_NO_RESOURCES );
      }
      return;
   }

   SetState( SELECT_FW_STATE_STARTED );

   appl_eTargetFirmware = eFirmware;
   appl_pnResultCallback = pnResultCallback;
   CreateInstance();
}

#endif
