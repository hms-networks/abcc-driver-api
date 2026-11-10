/*******************************************************************************
** Copyright 2026-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
*/

#include "abcc_api_profinet_station_name.h"

#include "abcc.h"
#include "abcc_api_network_settings.h"
#include "abcc_command_sequencer_interface.h"
#include "anybus_file_system_interface_object.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

/*******************************************************************************
** Defines
********************************************************************************
*/



/*******************************************************************************
** Constants
********************************************************************************
*/



/*******************************************************************************
** Private Globals
********************************************************************************
*/


/*------------------------------------------------------------------------------
** Update station id variables
**------------------------------------------------------------------------------
*/
static BOOL appl_fGetStationIdInProgress = FALSE;

/*------------------------------------------------------------------------------
** Set station id variables
**------------------------------------------------------------------------------
*/
static BOOL appl_fSetStationIdInProgress = FALSE;
static abStationName_t newStationName;


/*******************************************************************************
** Private forward declaration
********************************************************************************
*/

static ABCC_CmdSeqCmdStatusType  SetStationName( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqCmdStatusType  GetStationName( ABP_MsgType* psMsg, void* pxUserData );
static ABCC_CmdSeqRespStatusType HandleGetStationNameResp( ABP_MsgType* psMsg, void* pxUserData );


/*******************************************************************************
** Private Services
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Read station id.
** Sequence triggered cyclicaly to update station id in parameter database
** See abcc_command_sequencer_interface.h.
**------------------------------------------------------------------------------
*/
static const ABCC_CmdSeqType appl_asReadStationNameCmdSeq[] =
{
   ABCC_CMD_SEQ(GetStationName, HandleGetStationNameResp),
   ABCC_CMD_SEQ_END()
};

/*------------------------------------------------------------------------------
** Write station id.
** Sequence triggered cyclicaly to update station id in anybus module
** See abcc_command_sequencer_interface.h.
**------------------------------------------------------------------------------
*/
static const ABCC_CmdSeqType appl_asWriteStationNameCmdSeq[] =
{
   ABCC_CMD_SEQ(SetStationName, NULL),
   ABCC_CMD_SEQ_END()
};

/*------------------------------------------------------------------------------
**  Builds the command for writing station id.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType SetStationName( ABP_MsgType* psMsg, void* pxUserData )
{
    (void)pxUserData;

    ABCC_SetMsgHeader(psMsg, ABP_OBJ_NUM_NC, 20, 5, ABP_CMD_SET_ATTR, newStationName.length, ABCC_GetNewSourceId());

    ABCC_SetMsgString(psMsg, newStationName.stationName, newStationName.length, 0);

    return (ABCC_CMDSEQ_CMD_SEND);

}

/*------------------------------------------------------------------------------
**  Notification that get station name is done.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqDoneHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static void SetStationNameDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData )
{
   (void)pxUserData;

   appl_fSetStationIdInProgress = FALSE;

   switch( eSeqResult )
   {
   case ABCC_CMDSEQ_RESULT_COMPLETED:
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_INT:
      ABCC_LOG_INFO( "GetStationName reported internally aborted command sequence.\n" );
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_EXT:
      ABCC_LOG_INFO( "GetStationName reported externally aborted command sequence.\n" );
      break;

   default:
      ABCC_LOG_INFO( "GetStationName reported aborted command sequence.\n" );
      break;
   }
}

/*------------------------------------------------------------------------------
**  Builds the command for reqesting station id.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqCmdStatusType GetStationName( ABP_MsgType* psMsg, void* pxUserData )
{
  (void)pxUserData;

  ABCC_GetAttribute(psMsg, ABP_OBJ_NUM_NC, 20, 6, ABCC_GetNewSourceId());
  return (ABCC_CMDSEQ_CMD_SEND);
}




/*------------------------------------------------------------------------------
**  Builds the command for reading station id.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqCmdHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static ABCC_CmdSeqRespStatusType HandleGetStationNameResp( ABP_MsgType* psMsg, void* pxUserData )
{
  abStationName_t stationName;

   (void)pxUserData;

   if( ABCC_VerifyMessage( psMsg ) != ABCC_EC_NO_ERROR )
   {
      ABCC_LOG_ERROR( ABCC_EC_RESP_MSG_E_BIT_SET,
         ABCC_GetErrorCode( psMsg ),
         "Get exception code failed (%" PRIu8 ")\n",
         ABCC_GetErrorCode( psMsg ) );
      return( ABCC_CMDSEQ_RESP_EXEC_NEXT );
   }

  stationName.length = (UINT8)ABCC_GetMsgDataSize(psMsg);

  ABCC_GetMsgString(psMsg, stationName.stationName, stationName.length, 0);

  ABCC_CbfApplicationProfinetStationName(stationName);

  return (ABCC_CMDSEQ_RESP_EXEC_NEXT);
}


/*------------------------------------------------------------------------------
**  Notification that get station name is done.
**
**  This function is a part of a command sequence. See description of
**  ABCC_CmdSeqDoneHandler type in cmd_seq_if.h
**------------------------------------------------------------------------------
*/
static void GetStationNameDone( const ABCC_CmdSeqResultType eSeqResult, void* pxUserData )
{
   (void)pxUserData;

   appl_fGetStationIdInProgress = FALSE;

   switch( eSeqResult )
   {
   case ABCC_CMDSEQ_RESULT_COMPLETED:
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_INT:
      ABCC_LOG_INFO( "GetStationName reported internally aborted command sequence.\n" );
      break;

   case ABCC_CMDSEQ_RESULT_ABORT_EXT:
      ABCC_LOG_INFO( "GetStationName reported externally aborted command sequence.\n" );
      break;

   default:
      ABCC_LOG_INFO( "GetStationName reported aborted command sequence.\n" );
      break;
   }
}


/*******************************************************************************
 ** Public Services
 ********************************************************************************
 */


BOOL ABCC_API_UpdateStationId(void)
{
  if ( appl_fGetStationIdInProgress == FALSE && appl_fSetStationIdInProgress == FALSE)
  {
    appl_fGetStationIdInProgress = TRUE;

    // HandleGetStationNameResp in the command sequence will update the station name in the application
    // after reading it from the module.
    ABCC_CmdSeqAdd( appl_asReadStationNameCmdSeq, GetStationNameDone, NULL, NULL );

    return TRUE;
  }
  return FALSE;
}

BOOL ABCC_API_SetStationId(abStationName_t* stationName)
{
  if (stationName != NULL &&  appl_fSetStationIdInProgress == FALSE)
  {
    appl_fSetStationIdInProgress = TRUE;

    newStationName    = *stationName;

    ABCC_CmdSeqAdd( appl_asWriteStationNameCmdSeq, SetStationNameDone, NULL, NULL );

    return TRUE;
    
  }
  return FALSE;
}


