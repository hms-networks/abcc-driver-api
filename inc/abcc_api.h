/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** This is a simplified API towards the Anybus CompactCom Driver that handles
** general tasks in a generic manner.
**
** For more advanced features and flexibility, see abcc.h in abcc_driver/.
********************************************************************************
*/

#ifndef ABCC_API_H
#define ABCC_API_H

#include "abcc_types.h"
#include "abcc.h"
#include "abcc_error_codes.h"
#include "abp.h"
#include "abcc_config.h"
#include "../src/abcc_api_config.h"
#include "abcc_application_data_interface.h"

/*******************************************************************************
** Anybus CompactCom Driver API type definitions
********************************************************************************
*/
/*------------------------------------------------------------------------------
** ABCC network type.
** (See defined macros with prefix ABP_NW_TYPE_ in abp.h for translation.)
**------------------------------------------------------------------------------
*/
typedef UINT16 ABCC_API_NetworkType;

/*------------------------------------------------------------------------------
** ABCC firmware version structure.
**------------------------------------------------------------------------------
*/
typedef ABCC_FwVersionType ABCC_API_FwVersionType;

/*******************************************************************************
** Anybus CompactCom Driver API functions.
********************************************************************************
*/
/*------------------------------------------------------------------------------
** Initializes the ABCC driver resources. This function shall be called once
** before ABCC_API_Run() to prepare the driver for operation.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    ABCC_ErrorCodeType - ABCC_EC_NO_ERROR on success, or an error code
**                         indicating the cause of failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ABCC_API_Init( void );

/*------------------------------------------------------------------------------
** Core function of the ABCC API. Drives the ABCC communication and must be
** called cyclically from the application's main loop.
**
** If an error code is returned, the appropriate action depends on the scenario
** and has to be defined by the implementor.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    ABCC_ErrorCodeType - ABCC_EC_NO_ERROR on success, or an error code
**                         indicating the cause of failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ABCC_API_Run( void );

/*------------------------------------------------------------------------------
** Handles all timers for the ABCC driver, including the timeout and
** watchdog functionality. Call this function on a regular basis, ideally from
** a cyclic timer interrupt, providing the elapsed time since the last call.
**
** If this function is never called, or called too rarely, the timeout and
** watchdog functionality will not operate correctly.
**------------------------------------------------------------------------------
** Arguments:
**    iDeltaTimeMs - Milliseconds since last call.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_RunTimerSystem( const INT16 iDeltaTimeMs );

/*------------------------------------------------------------------------------
** Forces the ABCC handler to shut down the module. After this function is
** called, subsequent calls to ABCC_API_Run() will return an error code.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_Shutdown( void );

/*------------------------------------------------------------------------------
** Forces a reset of the ABCC module and restarts the handler's state machine.
** After the restart, the driver re-runs its initialization and setup
** sequence.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_Restart( void );

#if ABCC_CFG_SPI_DYNAMIC_MSG_FRAG_LEN
/*------------------------------------------------------------------------------
** Sets the message fragment size for the SPI frames. This function applies
** to SPI mode only.
**
** The new size is used for the next SPI transaction onwards, until changed
** again.
**
** Valid range: ABCC_CFG_SPI_MIN_MSG_FRAG_LEN to
**              ABCC_CFG_SPI_MAX_MSG_FRAG_LEN.
** The default at system startup is ABCC_CFG_SPI_DEFAULT_MSG_FRAG_LEN.
**
** Calling this function in other operating modes than SPI has no effect on
** the communication with the CompactCom and always returns
** ABCC_EC_NO_ERROR.
**------------------------------------------------------------------------------
** Arguments:
**    iReqMsgFragSize    - Requested message fragment size in bytes.
**
** Returns:
**    ABCC_ErrorCodeType - ABCC_EC_NO_ERROR on success, or an error code if
**                         the requested size is out of range.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ABCC_API_SetMsgFragSize( const UINT16 iReqMsgFragSize );
#endif // ABCC_CFG_SPI_DYNAMIC_MSG_FRAG_LEN

/*------------------------------------------------------------------------------
** Returns the current Anybus state of the CompactCom. The state reflects both
** the module itself and the network connection, and is useful for deciding
** how to act in certain situations (e.g., whether process data is being
** exchanged).
**
** As an alternative to polling, the application can be notified on state
** changes via ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY, see
** abcc_api_config.h for details.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    ABP_AnbStateType - Current Anybus state.
**------------------------------------------------------------------------------
*/
EXTFUNC ABP_AnbStateType ABCC_API_AnbState( void );

/*------------------------------------------------------------------------------
** Signals that the user-specific setup is complete, allowing the driver to
** progress from the SETUP state to the NW_INIT state.
**
** This function shall be called after ABCC_API_CbfUserInit() has been invoked,
** when the last response of the user-specific setup/init sequence has been
** received to progress from the Anybus state SETUP to NW_INIT.
**
** Typical usage:
**
**    - In the response handler of the last command of the user-specific
**      setup sequence (SETUP state).
**    - Within the context of ABCC_API_CbfUserInit() itself, if no user
**      commands are issued during setup.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_UserInitComplete( void );

/*------------------------------------------------------------------------------
** Returns the current status of the network supervision bit.
**
** On some network protocols, the CompactCom can have an established network
** connection without being in the Anybus state PROCESS_ACTIVE. Such a
** connection is indicated by the supervision bit.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    TRUE  - The CompactCom is supervised by another network device.
**    FALSE - The CompactCom is not supervised.
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_IsSupervised( void );

/*------------------------------------------------------------------------------
** Returns the current application status of the ABCC module.
**
** Source: Application Status Register (parallel mode) or the corresponding
** field in the MISO frame (SPI mode).
**
** Note! Available only in SPI and parallel operating modes.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    ABP_AppStatusType - Current application status.
**------------------------------------------------------------------------------
*/
EXTFUNC ABP_AppStatusType ABCC_API_GetAppStatus( void );

/*------------------------------------------------------------------------------
** Sets the application status reported by the ABCC module.
**
** The status is exposed to the network according to the operating mode:
** written directly to the Application Status Register (parallel mode) or
** transferred in the corresponding field of the MOSI frame (SPI mode).
**
** Note! This is only supported in SPI and parallel operating modes.
** Calling the function in any other operating mode has no effect.
**------------------------------------------------------------------------------
** Arguments:
**    eAppStatus - Application status to set.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_SetAppStatus( ABP_AppStatusType eAppStatus );

#if ABCC_CFG_INT_ENABLED
/*------------------------------------------------------------------------------
** ABCC interrupt service routine. Call this function from the application's
** ISR that is triggered by the IRQ_n signal on the ABCC host interface (if
** routed). It acknowledges and handles received ABCC events.
**
** The defines ABCC_CFG_INT_ENABLE_MASK and ABCC_CFG_HANDLE_IN_ABCC_ISR_MASK
** configure which events are handled by this function in interrupt context
** and which are handled from main loop context instead.
**
** NOTE! This is a macro alias for ABCC_ISR(), redefined from a deeper driver
** layer to save a function call in interrupt context. It therefore takes no
** arguments and returns nothing:
**
** void ABCC_API_ISR( void )
**------------------------------------------------------------------------------
*/
#define ABCC_API_ISR ABCC_ISR
#endif

/*******************************************************************************
** Anybus CompactCom Driver API callback functions
********************************************************************************
*/
/*------------------------------------------------------------------------------
** Callback invoked in the SETUP state, after the driver has gathered module
** information (network type, firmware version) and mapped the network data
** parameters to the ABCC module.
**
** This gives the host application a window to make adjustments and send
** commands based on the module's network type, before the driver progresses
** to the NW_INIT state.
**
** Note: ABCC_API_UserInitComplete() must be called after this callback has
**       been invoked, to progress from the Anybus state SETUP to NW_INIT.
**       Refer to ABCC_API_UserInitComplete() for details on where to call it.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType     - The 16 bit network type code of the ABCC module. See
**                       macros starting with ABP_NW_TYPE_ in abp.h for
**                       translation.
**
**    iFirmwareVersion - The firmware version of the ABCC module.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
void ABCC_API_CbfUserInit( ABCC_API_NetworkType iNetworkType, ABCC_API_FwVersionType iFirmwareVersion );

#if ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED
/*------------------------------------------------------------------------------
** Hook for user-specific command handling. Called for every command message
** received from the ABCC, before the default command handler processes it.
**
** Returning TRUE prevents the default command handler from seeing the
** command; the callback then takes full responsibility for producing and
** sending a response. Returning FALSE passes the command on to the default
** handler.
**
** Regarding callback context, see the comment for the callback section above.
**------------------------------------------------------------------------------
** Arguments:
**    psReceivedCommandMsg - Pointer to the received command message.
**
** Returns:
**    TRUE  - Command was handled by the callback function; a response has
**            been generated and will be transmitted.
**    FALSE - Command was not handled; the default command handler will
**            process it.
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_CbfCommandMessageHook( ABP_MsgType* psReceivedCommandMsg );
#endif

/*------------------------------------------------------------------------------
** Callback invoked every cycle after read and write process data have been
** updated.
**
** This function provides a dedicated entry point for user code to operate on
** ADIs immediately after they have been synchronized with the network. It is
** intended to be modified by the application to implement custom processing
** logic.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_CbfCyclicalProcessing( void );

/*------------------------------------------------------------------------------
** Returns the number of Application Data Instances (ADIs) defined by the
** application.
**
** The driver calls this callback during setup to determine the size of the
** ADI list to install.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    UINT16 - Number of defined Application Data Instances.
**------------------------------------------------------------------------------
*/
EXTFUNC UINT16 ABCC_API_CbfGetNumAdi( void );

#if ABCC_CFG_SYNC_ENABLED
/*------------------------------------------------------------------------------
** Sync event callback. Invoked when a synchronization event occurs.
**
** This function executes in interrupt context. Depending on the sync source,
** it is invoked as follows:
**
**    - Separate SYNC pin: The application must call this function from its
**      own ISR triggered by the SYNC pin.
**    - ABCC interrupt: The driver calls this function automatically upon
**      receiving the sync event.
**------------------------------------------------------------------------------
** Arguments:
**    None.
**
** Returns:
**    None.
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_CbfSyncIsr( void );
#endif

/*******************************************************************************
** Anybus CompactCom Driver API global variables
********************************************************************************
*/
/*------------------------------------------------------------------------------
** List of Application Data Instances.
**
** NOTE: The entries in the ADI list cannot be placed in arbitrary order; they
** must be sorted in ascending order (by ADI number) for all lookup functions
** in the driver and the Application Data Object to work as intended.
** See the example provided in abcc_application_data_interface.h.
**
** PORTING ALERT!
**
** If the ADI structure is defined at system startup rather than at compile
** time, the 'const' below should be removed. This applies when ADIs are
** defined by a local configuration file or by modules plugged into the
** local backplane.
**
** The ADI structures MUST be initialized with valid data before the driver
** calls AD_Init() ( triggered by ABCC_API_Run() ) in
** 'abcc-driver-api/src/abcc_api_handler.c'.
**
** With a fixed ADI structure that can reside in ROM, keep the 'const'.
**------------------------------------------------------------------------------
*/
EXTVAR const AD_AdiEntryType ABCC_API_asAdiEntryList[];

/*------------------------------------------------------------------------------
** Default process data map.
**
** See example provided in abcc_application_data_interface.h
**
** PORTING ALERT!
**
** If the default PD map is defined during system startup rather than at compile
** time, the 'const' below should be removed. 
**
** The PD map structures MUST be initialized with valid data before the driver
** calls AD_Init() (triggered by ABCC_API_Run()) in
** 'abcc-driver-api/src/abcc_api_handler.c'.
**
** With a fixed PD map that can reside in ROM, keep the 'const'.
**------------------------------------------------------------------------------
*/
EXTVAR const AD_MapType ABCC_API_asAdObjDefaultMap[];

/*------------------------------------------------------------------------------
** Pre-defined combinations of the descriptor bit field (bDesc) in
** AD_AdiEntryType.
**------------------------------------------------------------------------------
*/
#define AD_ADI_DESC______  ( 0                            | 0                               | 0                                | 0                         | 0                         )
#define AD_ADI_DESC_____G  ( 0                            | 0                               | 0                                | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC____S_  ( 0                            | 0                               | 0                                | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC____SG  ( 0                            | 0                               | 0                                | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC___W__  ( 0                            | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | 0                         )
#define AD_ADI_DESC___W_G  ( 0                            | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC___WS_  ( 0                            | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC___WSG  ( 0                            | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC__R___  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | 0                         | 0                         )
#define AD_ADI_DESC__R__G  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC__R_S_  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC__R_SG  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC__RW__  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | 0                         )
#define AD_ADI_DESC__RW_G  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC__RWS_  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC__RWSG  ( 0                            | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_N____  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | 0                                | 0                         | 0                         )
#define AD_ADI_DESC_N___G  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | 0                                | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_N__S_  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | 0                                | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC_N__SG  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | 0                                | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_N_W__  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | 0                         )
#define AD_ADI_DESC_N_W_G  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_N_WS_  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC_N_WSG  ( ABP_APPD_DESCR_NVS_PARAMETER | 0                               | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_NR___  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | 0                         | 0                         )
#define AD_ADI_DESC_NR__G  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_NR_S_  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC_NR_SG  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | 0                                | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_NRW__  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | 0                         )
#define AD_ADI_DESC_NRW_G  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | 0                         | ABP_APPD_DESCR_GET_ACCESS )
#define AD_ADI_DESC_NRWS_  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | 0                         )
#define AD_ADI_DESC_NRWSG  ( ABP_APPD_DESCR_NVS_PARAMETER | ABP_APPD_DESCR_MAPPABLE_READ_PD | ABP_APPD_DESCR_MAPPABLE_WRITE_PD | ABP_APPD_DESCR_SET_ACCESS | ABP_APPD_DESCR_GET_ACCESS )

#endif  /* inclusion lock */
