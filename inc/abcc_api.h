/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** This a simplified API towards the Anybus CompactCom Driver that handles
** general tasks in a generic manner.
**
** For more advanced features and flexibility, see abcc.h in abcc_driver/.
********************************************************************************
*/

#ifndef ABCC_API_H
#define ABCC_API_H
#include "abcc.h"
#include "../src/abcc_api_config.h"

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
** Anybus CompactCom Driver API global variables
********************************************************************************
*/
/*------------------------------------------------------------------------------
** List of Application data instances
**
** NOTE: The entries in the ADI list cannot be placed in arbitrary order, they
** must be sorted in ascending order for all lookup functions in the driver and
** the Application Data Object to work as intended! See example provided in the
** abcc_application_data_interface.h file.
**
** PORTING ALERT!
**
** If the ADI structure is defined at system startup rather than at compile-time
** the 'const' below should be removed. This would be the case when the ADIs
** are defined by a local configuration file or by the modules plugged into the
** local backplane. Note that the ADI structures MUST have been initialised with
** valid data before the driver makes the call to AD_Init() in
** './example_application/abcc_api_handler.c'.
**
** With a fixed ADI structure that can go into ROM, the 'const' should be left
** in place.
**------------------------------------------------------------------------------
*/
EXTVAR const AD_AdiEntryType ABCC_API_asAdiEntryList[];

/*------------------------------------------------------------------------------
** Default process data map
**
** See example provided in the abcc_application_data_interface.h file.
**
** PORTING ALERT!
**
** If the default PD map is defined during system startup rather than at
** compile-time the 'const' below should be removed. Note that the default PD
** map structures MUST have been initialised with valid data before the driver
** makes the call to AD_Init() in
** './example_application/abcc_api_handler.c'.
**
** With a fixed PD map that can go into ROM the 'const' should be left in place.
**------------------------------------------------------------------------------
*/
EXTVAR const AD_MapType ABCC_API_asAdObjDefaultMap[];

/*******************************************************************************
** Anybus CompactCom Driver API functions
********************************************************************************
*/
/*------------------------------------------------------------------------------
** This function shall be called once before ABCC_API_Run() to initialize
** resources to run the ABCC.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    ABCC driver error code
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ABCC_API_Init( void );

/*------------------------------------------------------------------------------
** This is the core function of the ABCC API. The function shall be called on
** a cyclic basis from the main loop to run the driver and to handle the ABCC.
**
** If an error code indicating an error is returned, the appropriate action to
** take depends on the scenario and has to be defined by the implementor.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    ABCC driver error code
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ABCC_API_Run( void );

/*------------------------------------------------------------------------------
** This function is responsible for handling all timers for the ABCC driver. It
** is recommended to call this function on a regular basis from a timer
** interrupt. Without this function no timeout and watchdog functionality will
** work.
**------------------------------------------------------------------------------
** Arguments:
**    iDeltaTimeMs - Milliseconds since last call.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_RunTimerSystem( const INT16 iDeltaTimeMs );

/*------------------------------------------------------------------------------
** This function will force the ABCC handler to shutdown the ABCC module.
** ABCC_API_Run() will return error if called after this function.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_Shutdown( void );

/*------------------------------------------------------------------------------
** This function will force the ABCC handler to reset the ABCC and restart the
** handler's state machine.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_Restart( void );

/*------------------------------------------------------------------------------
** Polls the current Anybus state of the CompactCom. The Anybus state is useful
** to determine how to act in certain situations, since it reflects the state
** of the CompactCom and the network connection.
** Read about ABCC_API_CONFIG_ANYBUS_STATE_CHANGE_NOTIFY in abcc_api_config.h
** on how to get notified when the state changes instead of polling.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    Anybus state
**------------------------------------------------------------------------------
*/
EXTFUNC ABP_AnbStateType ABCC_API_AnbState( void );

/*------------------------------------------------------------------------------
** This function shall be called within ABCC_API_CbfUserInit() context to
** progress from Anybus state SETUP to NW_INIT.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    Anybus state
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_UserInitComplete( void );

/*------------------------------------------------------------------------------
** Returns the current status of the supervision bit. On some network protocols,
** the CompactCom can have an established connection without being in the Anybus
** state PROCESS_ACTIVE. This type of cnnection is indicated by this bit.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    TRUE  - Is supervised by another network device.
**    FALSE - Not supervised.
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_IsSupervised( void );

#if ABCC_CFG_INT_ENABLED
/*------------------------------------------------------------------------------
** This function should be called from inside the ABCC interrupt service routine
** (triggered by the IRQ_n signal on the ABCC host interface if routed) to
** acknowledge and handle received ABCC events.
** The configurable defines, ABCC_CFG_INT_ENABLE_MASK and
** ABCC_CFG_HANDLE_IN_ABCC_ISR_MASK, allows configuration of which events to
** handle by the ISR in interrupt context and which events to handle from main
** loop context.
**
** To reduce function calls within interrupt context, the function is just
** redefined from a deeper layer. Therefore, to be clear, there is no return
*' value nor argument.
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
** A function called in SETUP state, after module information such as network
** type and firmware version has been gathered and network data parameters has
** been mapped to the ABCC module. The function provides a window for the host
** application to make adjustments and send commands based on the ABCC module's
** network type, before moving to NW_INIT state.
**
** Note: ABCC_API_UserInitComplete() must be called inside this function to
**       progress.
**------------------------------------------------------------------------------
** Arguments:
**    iNetworkType     - The 16 bit network type code of the ABCC module. See
**                       macros starting with ABP_NW_TYPE_ in abp.h for
**                       translation.
**    iFirmwareVersion - The firmware version of the ABCC module.
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
void ABCC_API_CbfUserInit( ABCC_API_NetworkType iNetworkType, ABCC_API_FwVersionType iFirmwareVersion );

/*------------------------------------------------------------------------------
** A function called every cycle after read and write data have been updated.
** The purpose of it is to have a function that can operate on ADIs when they
** are updated. This function is made to be modifiable and is a good place for
** user code.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_CbfCyclicalProcessing( void );

/*------------------------------------------------------------------------------
** Returns the number of defined Application data instances.
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    UINT16 - Number of Application data instances
**------------------------------------------------------------------------------
*/
EXTFUNC UINT16 ABCC_API_CbfGetNumAdi( void );

#if ABCC_API_COMMAND_MESSAGE_HOOK_ENABLED
/*------------------------------------------------------------------------------
** A command message has been received from the ABCC. If enabled all received
** commands from the ABCC will be passed to this function before handled by the
** default command handler. This allows the user to implement user specific
** handling of commands to any object or instance.
** Regarding callback context, see comment for callback section above.
**------------------------------------------------------------------------------
** Arguments:
**    psReceivedCommandMsg - Pointer to received command.
**
** Returns:
**    TRUE -  Command handled by the callback function (this typically means
**            that a response is generated and will be transmitted).
**    FALSE - Command was not handled by the callback function, let the default
**            command handler handle the command.
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_CbfCommandMessageHook( ABP_MsgType* psReceivedCommandMsg );
#endif

#endif  /* inclusion lock */
