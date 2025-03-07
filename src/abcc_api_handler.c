/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** This is a generic implementation of an 'abcc handler'.
** It includes the following section:
** 1. ADI definition and default mappings
** 2. ABCC driver callback implementations (process data, default mapping ...)
** 3. State machine implementation for message sequencing, e.g. during user init
**    or reading exception state
** 4. ABCC handler main function to be called from the main loop, including a main
**    state machine for handling startup and restart of the ABCC.
********************************************************************************
*/

#include <stdio.h>
#include "abcc_types.h"
#include "abcc_api_config.h"
#include "abcc_api_network_settings.h"
#include "abcc_config.h"
#include "abp.h"
#include "abcc_command_sequencer_interface.h"
#include "abcc_log.h"

#include "abcc.h"
#include "abcc_port.h"

#include "host_objects/application_data_object.h"   /* Application data object:   254 */

#include "abcc_api.h"
#include "abcc_api_network_settings.h"

/*------------------------------------------------------------------------------
** Comm settings values
**------------------------------------------------------------------------------
*/
#define ABCC_API_COMM_SETTING_1_INSTANCE            7
#define ABCC_API_COMM_SETTING_2_INSTANCE            8

/*------------------------------------------------------------------------------
** The basic IPv4 settings.
**------------------------------------------------------------------------------
*/
typedef struct
{
   UINT16   iInstance;
   union
   {
      UINT8 abValue[ 4 ];
      BOOL  fValue;
   }
   uValue;
}
appl_IpElementType;

typedef struct
{
   appl_IpElementType  sAddress;
   appl_IpElementType  sNetmask;
   appl_IpElementType  sGateway;
   appl_IpElementType  sDhcp;
}
appl_IpSettingsType;

/*------------------------------------------------------------------------------
** ABCC Handler states
**------------------------------------------------------------------------------
*/
typedef enum appl_AbccHandlerState
{
   ABCC_API_INIT,
   ABCC_API_WAITCOM,
   ABCC_API_WAIT_FW_UPDATE,
   ABCC_API_RUN,
   ABCC_API_RESTART,
   ABCC_API_HALT
}
appl_AbccHandlerStateType;

/*------------------------------------------------------------------------------
** Current state of the application state machine.
**------------------------------------------------------------------------------
*/
static appl_AbccHandlerStateType appl_eAbccHandlerState = ABCC_API_INIT;

/*------------------------------------------------------------------------------
** Network Node address.
**------------------------------------------------------------------------------
*/
static UINT16 appl_iNwNodeAddress;

/*------------------------------------------------------------------------------
** Network IP address settings
**------------------------------------------------------------------------------
*/
static appl_IpSettingsType appl_sIpSettings =
{
   { 3, { ABCC_API_DEFAULT_IP_NETWORK_ADDRESS } },
   { 4, { ABCC_API_DEFAULT_NETMASK } },
   { 5, { ABCC_API_DEFAULT_GATEWAY } },
   { 6, { ABCC_API_DEFAULT_DHCP_ENABLE } }
};

/*------------------------------------------------------------------------------
** Common address variables (used for both node address and IP address depending
** on network type).
**------------------------------------------------------------------------------
*/
static BOOL appl_fSetAddr = FALSE;
static BOOL appl_fSetAddrInProgress = FALSE;

/*------------------------------------------------------------------------------
** Network baud rate.
**------------------------------------------------------------------------------
*/
static UINT8 appl_bNwBaudRate;
static BOOL  appl_fSetBaudRate = FALSE;
static BOOL  appl_fSetBaudRateInProgress = FALSE;

/*------------------------------------------------------------------------------
** Comm setting variables. Used for setting speed and duplex on ethernet based
** networks.
**------------------------------------------------------------------------------
*/
static ABCC_API_CommSettingType appl_eCommSettings1 = ABCC_API_DEFAULT_COMM_SETTING;
static ABCC_API_CommSettingType appl_eCommSettings2 = ABCC_API_DEFAULT_COMM_SETTING;
static BOOL appl_fSetCommSettings = FALSE;
static BOOL appl_fSetCommSettingsInProgress = FALSE;

/*------------------------------------------------------------------------------
** Flags to keep track of if the network type supports node ID/IP address,
** baud rate and/or Comm settings to be set.
**------------------------------------------------------------------------------
*/
static BOOL appl_fNwSupportsNodeId;
static BOOL appl_fNwSupportsBaudRate;
static BOOL appl_fNwSupportsDhcp;
static BOOL appl_fNwSupportsCommSettings;

/*------------------------------------------------------------------------------
** Set to TRUE the user init sequence is done.
**------------------------------------------------------------------------------
*/
static BOOL appl_fUserInitPrepDone = FALSE;

#if ABCC_CFG_INT_ENABLED
/*------------------------------------------------------------------------------
** Event flags used by application to invoke the corresponding
** ABCC_Trigger<event_action> function from the desired context. The flag will
** be set to TRUE in ABCC_CbfEvent().
**------------------------------------------------------------------------------
*/
static volatile BOOL appl_fMsgReceivedEvent = FALSE;
static volatile BOOL appl_fRdPdReceivedEvent = FALSE;
static volatile BOOL appl_fTransmitMsgEvent = FALSE;
static volatile BOOL appl_fAbccStatusEvent = FALSE;
#endif

/*------------------------------------------------------------------------------
** Forward declarations
**------------------------------------------------------------------------------
*/

static ABCC_CmdSeqRespStatusType HandleExceptionResp( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqRespStatusType HandleExceptionInfoResp( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType ReadExeption( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType ReadExeptionInfo( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateNodeAddress( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateIpAddress( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateNetmask( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateGateway( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateDhcp( ABP_MsgType* psMsg, void* pxUserData );
static void UpdateAddressDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateBaudRate( ABP_MsgType* psMsg, void* pxUserData );
static void UpdateBaudRateDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateCommSetting1( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType UpdateCommSetting2( ABP_MsgType* psMsg, void* pxUserData );
static void UpdateCommSettingsDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData );

#ifdef ABCC_API_CONFIG_ERROR_EVENT_NOTIFY
EXTFUNC void ABCC_API_CONFIG_ERROR_EVENT_NOTIFY( ABCC_LogSeverityType eSeverity,
                                                 ABCC_ErrorCodeType iErrorCode,
                                                 UINT32 lAddInfo );
#endif

#ifdef ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY
EXTFUNC void ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY( ABP_AnbStateType eNewAnbState );
#endif

/*------------------------------------------------------------------------------
** User init sequence. See abcc_command_sequencer_interface.h
**------------------------------------------------------------------------------
*/
static const ABCC_CmdSeqType appl_asUserInitPrepCmdSeq[] =
{
   ABCC_CMD_SEQ( UpdateIpAddress, NULL ),
   ABCC_CMD_SEQ( UpdateNetmask, NULL ),
   ABCC_CMD_SEQ( UpdateGateway, NULL ),
   ABCC_CMD_SEQ( UpdateDhcp, NULL ),
   ABCC_CMD_SEQ( UpdateNodeAddress, NULL ),
   ABCC_CMD_SEQ( UpdateBaudRate, NULL ),
   ABCC_CMD_SEQ( UpdateCommSetting1, NULL ),
   ABCC_CMD_SEQ( UpdateCommSetting2, NULL ),
   ABCC_CMD_SEQ_END()
};

/*------------------------------------------------------------------------------
** Set IP or node address.
** Sequence triggered when the address switch has changed value.
** See abcc_command_sequencer_interface.h.
**------------------------------------------------------------------------------
*/
static const ABCC_CmdSeqType appl_asAddressChangedCmdSeq[] =
{
   ABCC_CMD_SEQ( UpdateIpAddress, NULL ),
   ABCC_CMD_SEQ( UpdateNodeAddress, NULL ),
   ABCC_CMD_SEQ_END()
};

/*------------------------------------------------------------------------------
** Set baud rate.
** Sequence triggered when the baud rate switch has changed value.
** See abcc_command_sequencer_interface.h.
**------------------------------------------------------------------------------
*/
static const ABCC_CmdSeqType appl_asBaudRateChangedCmdSeq[] =
{
   ABCC_CMD_SEQ( UpdateBaudRate, NULL ),
   ABCC_CMD_SEQ_END()
};

/*------------------------------------------------------------------------------
** Set Comm settings.
** Sequence triggered when the Comm settings has changed value.
** See abcc_command_sequencer_interface.h.
**------------------------------------------------------------------------------
*/
static const ABCC_CmdSeqType appl_asCommSettingsChangedCmdSeq[] =
{
   ABCC_CMD_SEQ( UpdateCommSetting1, NULL ),
   ABCC_CMD_SEQ( UpdateCommSetting2, NULL ),
   ABCC_CMD_SEQ_END()
};

/*------------------------------------------------------------------------------
** Read exception info.
** Sequence triggered when the anybus state changes to exception state.
** See abcc_command_sequencer_interface.h.
**------------------------------------------------------------------------------
*/
static const ABCC_CmdSeqType appl_asReadExeptionCmdSeq[] =
{
   ABCC_CMD_SEQ( ReadExeption,     HandleExceptionResp ),
   ABCC_CMD_SEQ( ReadExeptionInfo, HandleExceptionInfoResp ),
   ABCC_CMD_SEQ_END()
};

/*------------------------------------------------------------------------------
**  Builds the command for reading the exception code (get attribute single).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType ReadExeption( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   ABCC_GetAttribute( psMsg, ABP_OBJ_NUM_ANB, 1, ABP_ANB_IA_EXCEPTION, ABCC_GetNewSourceId() );
   return( ABCC_CMDSEQ_CMD_SEND );
}

/*------------------------------------------------------------------------------
**  Builds the command for reading the exception info code (get attribute
**  single).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType ReadExeptionInfo( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   ABCC_GetAttribute( psMsg, ABP_OBJ_NUM_NW, 1, ABP_NW_IA_EXCEPTION_INFO, ABCC_GetNewSourceId() );
   return( ABCC_CMDSEQ_CMD_SEND );
}

/*------------------------------------------------------------------------------
**  Handles the exception code response (get attribute single).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqRespHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqRespStatusType HandleExceptionResp( ABP_MsgType* psMsg, void* pxUserData )
{
   UINT8 bException;
   (void)pxUserData;

   if( ABCC_VerifyMessage( psMsg ) != ABCC_EC_NO_ERROR )
   {
      ABCC_LOG_ERROR( ABCC_EC_RESP_MSG_E_BIT_SET,
         ABCC_GetErrorCode( psMsg ),
         "Get exception code failed (%" PRIu8 ")\n",
         ABCC_GetErrorCode( psMsg ) );
      return( ABCC_CMDSEQ_RESP_EXEC_NEXT );
   }

   ABCC_GetMsgData8( psMsg, &bException, 0 );
   ABCC_LOG_INFO( "Exception Code: %X:\n", bException );

   (void)bException;
   return( ABCC_CMDSEQ_RESP_EXEC_NEXT );
}

/*------------------------------------------------------------------------------
**  Handles the exception info code response (get attribute single).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqRespHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqRespStatusType HandleExceptionInfoResp( ABP_MsgType* psMsg, void* pxUserData )
{
   UINT8 bExceptionInfo;
   (void)pxUserData;

   if( ABCC_VerifyMessage( psMsg ) != ABCC_EC_NO_ERROR )
   {
      ABCC_LOG_ERROR( ABCC_EC_RESP_MSG_E_BIT_SET,
         ABCC_GetErrorCode( psMsg ),
         "Get exception info failed (%" PRIu8 ")\n",
         ABCC_GetErrorCode( psMsg ) );
      return( ABCC_CMDSEQ_RESP_EXEC_NEXT );
   }

   ABCC_GetMsgData8( psMsg, &bExceptionInfo, 0 );
   ABCC_LOG_INFO( "Exception Info: %X:\n", bExceptionInfo );

   (void)bExceptionInfo;
   return( ABCC_CMDSEQ_RESP_EXEC_NEXT );
}

/*------------------------------------------------------------------------------
**  Notification that the address is updated.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqDoneHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static void UpdateAddressDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData )
{
   (void)pxUserData;

   appl_fSetAddrInProgress = FALSE;

   switch( eSeqResult )
   {
   case ABCC_CMDSEQ_RESULT_COMPLETED:
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_INT:
      ABCC_LOG_INFO( "UpdateAddressDone reported internally aborted command sequence.\n" );
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_EXT:
      ABCC_LOG_INFO( "UpdateAddressDone reported externally aborted command sequence.\n" );
      break;

   default:
      ABCC_LOG_INFO( "UpdateAddressDone reported aborted command sequence.\n" );
      break;
   }
}

/*------------------------------------------------------------------------------
**  Builds the command for setting the ip-address (set attribute).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateIpAddress( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   if( ( !appl_fNwSupportsNodeId ) &&
       ( appl_fSetAddr ) )
   {
      ABCC_SetMsgHeader( psMsg,
                         ABP_OBJ_NUM_NC,
                         appl_sIpSettings.sAddress.iInstance,
                         ABP_NC_VAR_IA_VALUE,
                         ABP_CMD_SET_ATTR,
                         4,
                         ABCC_GetNewSourceId() );

      ABCC_SetMsgString( psMsg, (char*)appl_sIpSettings.sAddress.uValue.abValue, 4, 0 );

      return( ABCC_CMDSEQ_CMD_SEND );
   }

   return( ABCC_CMDSEQ_CMD_SKIP );
}

/*------------------------------------------------------------------------------
**  Builds the command for setting the netmask (set attribute).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateNetmask( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   if( ( !appl_fNwSupportsNodeId ) &&
       ( appl_fSetAddr ) )
   {
      ABCC_SetMsgHeader( psMsg,
                         ABP_OBJ_NUM_NC,
                         appl_sIpSettings.sNetmask.iInstance,
                         ABP_NC_VAR_IA_VALUE,
                         ABP_CMD_SET_ATTR,
                         4,
                         ABCC_GetNewSourceId() );

      ABCC_SetMsgString( psMsg, (char*)appl_sIpSettings.sNetmask.uValue.abValue, 4, 0 );

      return( ABCC_CMDSEQ_CMD_SEND );
   }

   return( ABCC_CMDSEQ_CMD_SKIP );
}

/*------------------------------------------------------------------------------
**  Builds the command for setting the gateway (set attribute).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateGateway( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   if( ( !appl_fNwSupportsNodeId ) &&
       ( appl_fSetAddr ) )
   {
      ABCC_SetMsgHeader( psMsg,
                         ABP_OBJ_NUM_NC,
                         appl_sIpSettings.sGateway.iInstance,
                         ABP_NC_VAR_IA_VALUE,
                         ABP_CMD_SET_ATTR,
                         4,
                         ABCC_GetNewSourceId() );

      ABCC_SetMsgString( psMsg, (char*)appl_sIpSettings.sGateway.uValue.abValue, 4, 0 );

      return( ABCC_CMDSEQ_CMD_SEND );
   }

   return( ABCC_CMDSEQ_CMD_SKIP );
}

/*------------------------------------------------------------------------------
**  Builds the command for enable/disable dhcp (set attribute).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateDhcp( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   if( ( appl_fNwSupportsDhcp ) &&
       ( appl_fSetAddr ) )
   {
      ABCC_SetByteAttribute( psMsg, ABP_OBJ_NUM_NC,
                             appl_sIpSettings.sDhcp.iInstance,
                             ABP_NC_VAR_IA_VALUE,
                             (UINT8)appl_sIpSettings.sDhcp.uValue.fValue,
                             ABCC_GetNewSourceId() );

      return( ABCC_CMDSEQ_CMD_SEND );
   }
   return( ABCC_CMDSEQ_CMD_SKIP );
}

/*------------------------------------------------------------------------------
**  Builds the command for setting the node-address (set attribute).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateNodeAddress( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   if( ( appl_fNwSupportsNodeId ) &&
       ( appl_fSetAddr ) )
   {
      ABCC_SetMsgHeader( psMsg,
                         ABP_OBJ_NUM_NC,
                         ABP_NC_INST_NUM_SW1,
                         ABP_NC_VAR_IA_VALUE,
                         ABP_CMD_SET_ATTR,
                         ABP_UINT16_SIZEOF,
                         ABCC_GetNewSourceId() );

      ABCC_SetMsgData16( psMsg, appl_iNwNodeAddress, 0 );

      return( ABCC_CMDSEQ_CMD_SEND );
   }

   return( ABCC_CMDSEQ_CMD_SKIP );
}

/*------------------------------------------------------------------------------
**  Notification that the Comm settings are updated.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqDoneHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static void UpdateCommSettingsDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData )
{
   (void)pxUserData;

   appl_fSetCommSettingsInProgress = FALSE;

   switch( eSeqResult )
   {
   case ABCC_CMDSEQ_RESULT_COMPLETED:
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_INT:
      ABCC_LOG_INFO( "UpdateCommSettingsDone reported internally aborted command sequence.\n" );
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_EXT:
      ABCC_LOG_INFO( "UpdateCommSettingsDone reported externally aborted command sequence.\n" );
      break;

   default:
      ABCC_LOG_INFO( "UpdateCommSettingsDone reported aborted command sequence.\n" );
      break;
   }
}

/*------------------------------------------------------------------------------
**  Builds the command for Comm settings.
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType SetCommSettingCmdMsgData( ABP_MsgType* psMsg,
                                                          UINT16 iInstance,
                                                          ABCC_API_CommSettingType eCommSetting )
{
   if( ( appl_fNwSupportsCommSettings ) &&
       ( appl_fSetCommSettings ) )
   {
      ABCC_SetByteAttribute( psMsg,
                             ABP_OBJ_NUM_NC,
                             iInstance,
                             ABP_NC_VAR_IA_VALUE,
                             (UINT8)eCommSetting,
                             ABCC_GetNewSourceId() );
      return( ABCC_CMDSEQ_CMD_SEND );
   }

   return( ABCC_CMDSEQ_CMD_SKIP );
}

/*------------------------------------------------------------------------------
**  Builds the command for Comm 1 setting.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateCommSetting1( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   return( SetCommSettingCmdMsgData( psMsg, ABCC_API_COMM_SETTING_1_INSTANCE, appl_eCommSettings1 ) );
}

/*------------------------------------------------------------------------------
**  Builds the command for Comm 2 setting.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateCommSetting2( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   return( SetCommSettingCmdMsgData( psMsg, ABCC_API_COMM_SETTING_2_INSTANCE, appl_eCommSettings2 ) );
}

/*------------------------------------------------------------------------------
**  Notification that the baud rate is updated.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqDoneHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static void UpdateBaudRateDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData )
{
   (void)pxUserData;

   appl_fSetBaudRateInProgress = FALSE;

   switch( eSeqResult )
   {
   case ABCC_CMDSEQ_RESULT_COMPLETED:
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_INT:
      ABCC_LOG_INFO( "UpdateBaudRateDone reported internally aborted command sequence.\n" );
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_EXT:
      ABCC_LOG_INFO( "UpdateBaudRateDone reported externally aborted command sequence.\n" );
      break;

   default:
      ABCC_LOG_INFO( "UpdateBaudRateDone reported aborted command sequence.\n" );
      break;
   }
}

/*------------------------------------------------------------------------------
**  Builds the command for setting the baud rate (set attribute).
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType UpdateBaudRate( ABP_MsgType* psMsg, void* pxUserData )
{
   (void)pxUserData;

   if( ( appl_fNwSupportsBaudRate ) &&
       ( appl_fSetBaudRate ) )
   {
      ABCC_SetByteAttribute( psMsg, ABP_OBJ_NUM_NC,
                             ABP_NC_INST_NUM_SW2,
                             ABP_NC_VAR_IA_VALUE,
                             appl_bNwBaudRate,
                             ABCC_GetNewSourceId() );

      return( ABCC_CMDSEQ_CMD_SEND );
   }
   return( ABCC_CMDSEQ_CMD_SKIP );
}

/*------------------------------------------------------------------------------
**  Notification that the user init preparation sequence is done.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqDoneHandler type in abcc_cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static void UserInitPrepDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData )
{
   (void)pxUserData;

   switch( eSeqResult )
   {
   case ABCC_CMDSEQ_RESULT_COMPLETED:
   {
      ABCC_API_FwVersionType iFirmwareVersion;
      ABCC_API_NetworkType iNetworkType;

      iNetworkType = (ABCC_API_NetworkType)ABCC_NetworkType();
      iFirmwareVersion = ABCC_FirmwareVersion();

      ABCC_API_CbfUserInit( iNetworkType, iFirmwareVersion );
      break;
   }
   case ABCC_CMDSEQ_RESULT_ABORT_INT:
      ABCC_LOG_ERROR( ABCC_EC_INTERNAL_ERROR,
         0,
         "UserInitPrepDone reported internally aborted command sequence.\n" );
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_EXT:
      ABCC_LOG_ERROR( ABCC_EC_INTERNAL_ERROR,
         0,
         "UserInitPrepDone reported externally aborted command sequence.\n" );
      break;

   default:
      ABCC_LOG_ERROR( ABCC_EC_INTERNAL_ERROR,
         eSeqResult,
         "UserInitPrepDone reported unknown abort code.\n" );
      break;
   }
}
void ABCC_API_UserInitComplete()
{
      appl_fUserInitPrepDone = TRUE;
      ABCC_UserInitComplete();
}

void ABCC_API_RunTimerSystem( const INT16 iDeltaTimeMs )
{
   if( appl_eAbccHandlerState > ABCC_API_WAITCOM )
   {
      ABCC_RunTimerSystem( iDeltaTimeMs );
   }
}

EXTFUNC ABCC_ErrorCodeType ABCC_API_Init( void )
{
   return( ABCC_HwInit() );
}

ABCC_ErrorCodeType ABCC_API_Run( void )
{
   static ABCC_ErrorCodeType eErrorCode = ABCC_EC_NO_ERROR;
   UINT32 lStartupTimeMs;
   ABCC_CommunicationStateType eAbccComState;

   switch( appl_eAbccHandlerState )
   {
   case ABCC_API_INIT:

      eErrorCode = ABCC_EC_NO_ERROR;
#if ABCC_CFG_INT_ENABLED
      appl_fMsgReceivedEvent = FALSE;
      appl_fRdPdReceivedEvent = FALSE;
      appl_fTransmitMsgEvent = FALSE;
      appl_fAbccStatusEvent = FALSE;
#endif
      appl_fUserInitPrepDone = FALSE;

      if( !ABCC_ModuleDetect() )
      {
         eErrorCode = ABCC_EC_MODULE_NOT_DECTECTED;
      }

      if( eErrorCode == ABCC_EC_NO_ERROR )
      {
         /*
         ** Init application data object
         */
         if( AD_Init( ABCC_API_asAdiEntryList,
                      ABCC_API_CbfGetNumAdi(),
                      ABCC_API_asAdObjDefaultMap ) != ABCC_EC_NO_ERROR )
         {
            eErrorCode = ABCC_EC_INTERNAL_ERROR;
         }
#if ASM_OBJ_ENABLE
#error "Assembly Mapping Object is not supported in this version of the ABCC API"
         /*
         ** Init Assembly mapping object
         */
         ASM_Init();
#endif
#if ANB_FSI_OBJ_ENABLE
         ANB_FSI_Init();
#endif
      }

      if( eErrorCode == ABCC_EC_NO_ERROR )
      {
#if APP_OBJ_ENABLE
         if( ABCC_CbfApplicationObjFirmwareAvailable_Get() == TRUE )
         {
            ABCC_LOG_INFO( "Waiting for firmware update to complete.\n" );
            lStartupTimeMs = ABCC_API_FW_UPGRADE_STARTUP_TIME_MS;
         }
         else
#endif
         {
            /*
            ** Default time will be used
            */
            lStartupTimeMs = 0;
         }

         if( ABCC_StartDriver( lStartupTimeMs ) == ABCC_EC_NO_ERROR )
         {
            ABCC_HWReleaseReset();
            appl_eAbccHandlerState = ABCC_API_WAITCOM;
         }
         else
         {
            eErrorCode = ABCC_EC_MODULE_NOT_ANSWERING;
         }
      }

      if( eErrorCode != ABCC_EC_NO_ERROR )
      {
         appl_eAbccHandlerState = ABCC_API_HALT;
      }

      break;

   case ABCC_API_WAITCOM:
   case ABCC_API_WAIT_FW_UPDATE:

      eAbccComState = ABCC_isReadyForCommunication();

      if( eAbccComState == ABCC_READY_FOR_COMMUNICATION )
      {
         appl_eAbccHandlerState = ABCC_API_RUN;
      }
      else if( eAbccComState == ABCC_STARTUP_TIMEOUT )
      {
         appl_eAbccHandlerState = ABCC_API_HALT;
         eErrorCode = ABCC_EC_MODULE_NOT_ANSWERING;
         ABCC_LOG_ERROR( ABCC_EC_MODULE_NOT_ANSWERING, 0, "ABCC did not start within the expected time\n" );
      }
#if ABCC_CFG_DRV_ASSUME_FW_UPDATE_ENABLED
      else if( eAbccComState == ABCC_ASSUME_FW_UPDATE )
      {
         appl_eAbccHandlerState = ABCC_API_WAIT_FW_UPDATE;
         ABCC_LOG_INFO( "ABCC did not start within the expected time. Assume firmware update is ongoing. \n" \
                           "Now waiting an additional %lu ms\n", ABCC_API_FW_UPGRADE_STARTUP_TIME_MS );
         ABCC_WaitForFwUpdate( ABCC_API_FW_UPGRADE_STARTUP_TIME_MS );
      }
#endif
      break;

   case ABCC_API_RUN:

#if ABCC_CFG_INT_ENABLED
      if( appl_fRdPdReceivedEvent )
      {
         appl_fRdPdReceivedEvent = FALSE;
         ABCC_TriggerRdPdUpdate();
      }

      if( appl_fMsgReceivedEvent )
      {
         appl_fMsgReceivedEvent = FALSE;
         ABCC_TriggerReceiveMessage();
      }

      if( appl_fTransmitMsgEvent )
      {
         appl_fTransmitMsgEvent = FALSE;
         ABCC_TriggerTransmitMessage();
      }

      if( appl_fAbccStatusEvent )
      {
         appl_fAbccStatusEvent = FALSE;
         ABCC_TriggerAnbStatusUpdate();
      }
#endif

      /*
      ** End event handling.
      */

#if SYNC_OBJ_ENABLE
#error "Sync Object is not supported in this version of the ABCC API"
      if( SYNC_GetMode() == SYNC_MODE_NONSYNCHRONOUS )
      {
         ABCC_TriggerWrPdUpdate();
      }
#else
      /*
      ** Always update write process data
      */
      ABCC_TriggerWrPdUpdate();
#endif

      eErrorCode = ABCC_RunDriver();

      if( eErrorCode == ABCC_EC_NO_ERROR )
      {
         ABCC_API_CbfCyclicalProcessing();
      }

      break;

   case ABCC_API_RESTART:

      ABCC_HWReset();
      appl_eAbccHandlerState = ABCC_API_INIT;
      eErrorCode = ABCC_EC_NO_ERROR;
      break;

   case ABCC_API_HALT:

      break;

   default:

      break;
   }

   return( eErrorCode );
}

void ABCC_API_SetAddress( UINT16 iSwitchValue )
{
   appl_fSetAddr = TRUE;

   if( appl_fSetAddrInProgress )
   {
      /*
      ** Address updated next time
      */
      return;
   }

   if( appl_fUserInitPrepDone == FALSE )
   {
      /*
      ** HW switch 1 will the last octet in the IP address
      ** for applicable networks ( 192.168.0.X )
      */
      appl_sIpSettings.sAddress.uValue.abValue[ 3 ] = (UINT8)iSwitchValue;

      /*
      ** Switch 1 is node address for applicable networks
      */
      appl_iNwNodeAddress = iSwitchValue;

      /*
      ** Indicate to application object that the address is set by HW switches
      */
   #if APP_OBJ_ENABLE
      APP_HwConfAddress( TRUE );
   #endif
   }
   else if( appl_iNwNodeAddress != iSwitchValue )
   {
     /*
     ** Start command sequence to update address if the value has changed.
     */
     appl_sIpSettings.sAddress.uValue.abValue[ 3 ] = (UINT8)iSwitchValue;
     appl_iNwNodeAddress = iSwitchValue;
     appl_fSetAddrInProgress = TRUE;
     ABCC_CmdSeqAdd( appl_asAddressChangedCmdSeq, UpdateAddressDone, NULL, NULL );
   }
}

void ABCC_API_SetBaudrate( UINT8 bSwitchValue )
{
   if( appl_fSetBaudRateInProgress )
   {
      /*
      ** Baud rate updated next time
      */
      return;
   }

   if( appl_fUserInitPrepDone == FALSE )
   {
      appl_bNwBaudRate = bSwitchValue;
   }
   else if( appl_bNwBaudRate != bSwitchValue )
   {
      /*
      ** Start command sequence to update baud rate if the value has changed.
      */
      appl_bNwBaudRate = bSwitchValue;
      ABCC_CmdSeqAdd( appl_asBaudRateChangedCmdSeq, UpdateBaudRateDone, NULL, NULL );
   }
   appl_fSetBaudRate = TRUE;
}

void ABCC_API_SetCommSettings( ABCC_API_CommSettingType eCommSettings1,
                               ABCC_API_CommSettingType eCommSettings2 )
{
   if( appl_fSetCommSettingsInProgress )
   {
      /*
      ** Comm settings updated next time
      */
      return;
   }

   if( appl_fUserInitPrepDone == FALSE )
   {
      appl_eCommSettings1 = eCommSettings1;
      appl_eCommSettings2 = eCommSettings2;
   }
   else if( ( appl_eCommSettings1 != eCommSettings1 ) ||
            ( appl_eCommSettings2 != eCommSettings2 ) )
   {
      /*
      ** Start command sequence to update Comm settings if values have changed.
      */
      appl_eCommSettings1 = eCommSettings1;
      appl_eCommSettings2 = eCommSettings2;
      appl_fSetCommSettingsInProgress = TRUE;
      ABCC_CmdSeqAdd( appl_asCommSettingsChangedCmdSeq, UpdateCommSettingsDone, NULL, NULL );
   }
   appl_fSetCommSettings = TRUE;
}

void ABCC_API_Shutdown( void )
{
   ABCC_HWReset();
}

void ABCC_API_Restart( void )
{
   appl_eAbccHandlerState = ABCC_API_RESTART;
}

ABP_AnbStateType ABCC_API_AnbState( void )
{
   return( (ABP_AnbStateType)ABCC_AnbState() );
}

BOOL ABCC_API_IsSupervised( void )
{
   return( ABCC_IsSupervised() );
}

UINT16  ABCC_CbfAdiMappingReq( const AD_AdiEntryType**  const ppsAdiEntry,
                               const AD_MapType** const ppsDefaultMap )
{
   return( AD_AdiMappingReq( ppsAdiEntry, ppsDefaultMap ) );
}

BOOL ABCC_CbfUpdateWriteProcessData( void* pxWritePd )
{

   /*
   ** AD_UpdatePdWriteData is a general function that updates all ADI:s according
   ** to current map.
   ** If the ADI mapping is fixed there is potential for doing that in a more
   ** optimized way, for example by using memcpy.
   */
   return( AD_UpdatePdWriteData( pxWritePd ) );
}

#if( ABCC_CFG_REMAP_SUPPORT_ENABLED )
void ABCC_CbfRemapDone( void )
{
   AD_RemapDone();
}
#endif

void ABCC_CbfNewReadPd( void* pxReadPd )
{
   /*
   ** AD_UpdatePdReadData is a general function that updates all ADI:s according
   ** to current map.
   ** If the ADI mapping is fixed there is potential for doing that in a more
   ** optimized way, for example by using memcpy.
   */
   AD_UpdatePdReadData( pxReadPd );
}

void ABCC_CbfDriverError( ABCC_LogSeverityType eSeverity, ABCC_ErrorCodeType iErrorCode, UINT32 lAddInfo )
{
   (void)eSeverity;
   (void)iErrorCode;
   (void)lAddInfo;

#ifdef ABCC_API_CONFIG_ERROR_EVENT_NOTIFY
   ABCC_API_CONFIG_ERROR_EVENT_NOTIFY( eSeverity, iErrorCode, lAddInfo );
#endif
}

void ABCC_CbfWdTimeout( void )
{
   ABCC_LOG_WARNING( ABCC_EC_MODULE_NOT_ANSWERING, 0, "ABCC watchdog timeout\n" );

#if ABCC_CFG_DRV_ASSUME_FW_UPDATE_ENABLED
   if( ABCC_IsFirstCommandPending() )
   {
      if( ABCC_WaitForFwUpdate( ABCC_API_FW_UPGRADE_STARTUP_TIME_MS ) )
      {
         appl_eAbccHandlerState = ABCC_API_WAIT_FW_UPDATE;
         ABCC_LOG_INFO( "Failed to establish communication within the expected time. Assume firmware" \
                        "update is ongoing. Now waiting an additional %lu ms\n", ABCC_API_FW_UPGRADE_STARTUP_TIME_MS );
      }
   }
#endif
}

void ABCC_CbfWdTimeoutRecovered( void )
{
   ABCC_LOG_WARNING( ABCC_EC_NO_ERROR, 0, "ABCC watchdog recovered\n" );
}

#if ABCC_CFG_SYNC_ENABLED
#error "Sync Object is not supported in this version of the ABCC API"
void ABCC_CbfSyncIsr( void )
{
   /*
   ** Call application specific handling of sync event
   */
   ABCC_API_CbfSyncIsr();
}
#endif

#if ABCC_CFG_INT_ENABLED
void ABCC_CbfEvent( UINT16 iEvents )
{

   /*
   ** Set flag to indicate that an event has occurred and the corresponding
   ** ABCC_Trigger<event_action> must be called. In the sample code the the
   ** trigger function is called from main loop context.
   */
   if( iEvents & ABCC_ISR_EVENT_RDPD )
   {
      appl_fRdPdReceivedEvent = TRUE;
   }

   if( iEvents & ABCC_ISR_EVENT_RDMSG )
   {
      appl_fMsgReceivedEvent = TRUE;
   }

   if( iEvents & ABCC_ISR_EVENT_WRMSG )
   {
      appl_fTransmitMsgEvent = TRUE;
   }

   if( iEvents & ABCC_ISR_EVENT_STATUS  )
   {
      appl_fAbccStatusEvent = TRUE;
   }
}
#endif

void ABCC_CbfAnbStateChanged( ABP_AnbStateType eNewAnbState )
{
   static const char* AnbStateString[ 8 ] =
   {
      "ABP_ANB_STATE_SETUP",
      "ABP_ANB_STATE_NW_INIT",
      "ABP_ANB_STATE_WAIT_PROCESS",
      "ABP_ANB_STATE_IDLE",
      "ABP_ANB_STATE_PROCESS_ACTIVE",
      "ABP_ANB_STATE_ERROR",
      "",
      "ABP_ANB_STATE_EXCEPTION"
   };
   (void)AnbStateString[ 0 ];

   ABCC_LOG_INFO( "ANB_STATE: %s \n",
                     AnbStateString[ eNewAnbState ] );

   switch( eNewAnbState )
   {
   case ABP_ANB_STATE_PROCESS_ACTIVE:

      ABCC_TriggerWrPdUpdate();
      break;

   case ABP_ANB_STATE_EXCEPTION:

      /* Trigger message sequence for reading exception data */
      ABCC_CmdSeqAdd( appl_asReadExeptionCmdSeq, NULL, NULL, NULL );
      break;

   default:

      break;
   }

#ifdef ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY
   ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY( eNewAnbState );
#endif
}

void ABCC_CbfUserInitReq( void )
{
   UINT16 iNetworkType;
   iNetworkType = ABCC_NetworkType();

   if( ( iNetworkType == ABP_NW_TYPE_DEV ) ||
       ( iNetworkType == ABP_NW_TYPE_PDPV0 ) ||
       ( iNetworkType == ABP_NW_TYPE_PDPV1 ) ||
       ( iNetworkType == ABP_NW_TYPE_COP ) ||
       ( iNetworkType == ABP_NW_TYPE_CNT ) ||
       ( iNetworkType == ABP_NW_TYPE_CCL ) ||
       ( iNetworkType == ABP_NW_TYPE_CFN ) ||
       ( iNetworkType == ABP_NW_TYPE_CPN ) ||
       ( iNetworkType == ABP_NW_TYPE_ECT ) ||
       ( iNetworkType == ABP_NW_TYPE_EPL ) )
   {
      appl_fNwSupportsNodeId = TRUE;
      appl_fNwSupportsDhcp = FALSE;
   }
   else
   {
      appl_fNwSupportsNodeId = FALSE;
      appl_fNwSupportsDhcp = TRUE;
   }

   if( ( iNetworkType == ABP_NW_TYPE_DEV ) ||
       ( iNetworkType == ABP_NW_TYPE_COP ) ||
       ( iNetworkType == ABP_NW_TYPE_CCL ) )
   {
      appl_fNwSupportsBaudRate = TRUE;
   }
   else
   {
      appl_fNwSupportsBaudRate = FALSE;
   }

   if( iNetworkType == ABP_NW_TYPE_CIET )
   {
      appl_fNwSupportsCommSettings = TRUE;
      appl_fNwSupportsDhcp = FALSE;
   }
   else
   {
      appl_fNwSupportsCommSettings = FALSE;
   }

   /*
   ** Start user init preparation command sequence
   */
   ABCC_CmdSeqAdd( appl_asUserInitPrepCmdSeq, UserInitPrepDone, NULL, NULL );

}
