/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Public header file for the Diagnostic object.
********************************************************************************
*/

#ifndef DI_OBJ_H
#define DI_OBJ_H

#include "abcc.h"

#if DI_OBJ_ENABLE

/*******************************************************************************
** Public constants
********************************************************************************
*/

#if PRT_OBJ_ENABLE

/*------------------------------------------------------------------------------
** Values, bitmasks, and similar for creation of PROFINET-specific diagnostic
** events.
**------------------------------------------------------------------------------
*/

/*
** Constants for the ChannelNumber field.
** Reference: PROFINET International doc.no 2.722, chapter 5.2.8.3, "Coding of the field ChannelNumber"
*/
#define DI_PRT_CHAN_NUM_ENTIRE_SUBMODULE              ( 0x8000U )

/*
** Constants for the ChannelProperties field.
** Reference: PROFINET International doc.no 2.722, chapter 5.2.8.4, "Coding of the field ChannelProperties"
**
** NOTE:
** Bitmasks for the ChannelProperties.Specifier field are deliberately missing
** in this list. This field is handled by the message builder function, and the
** caller shall leave those bits cleared.
*/
#define DI_PRT_CHAN_PROP_TYPE_UNDEF                   ( 0x0000U )
#define DI_PRT_CHAN_PROP_TYPE_1BIT                    ( 0x0001U )
#define DI_PRT_CHAN_PROP_TYPE_2BIT                    ( 0x0002U )
#define DI_PRT_CHAN_PROP_TYPE_4BIT                    ( 0x0003U )
#define DI_PRT_CHAN_PROP_TYPE_8BIT                    ( 0x0004U )
#define DI_PRT_CHAN_PROP_TYPE_16BIT                   ( 0x0005U )
#define DI_PRT_CHAN_PROP_TYPE_32BIT                   ( 0x0006U )
#define DI_PRT_CHAN_PROP_TYPE_64BIT                   ( 0x0007U )
#define DI_PRT_CHAN_PROP_TYPE_MASK                    ( 0x00FFU )
#define DI_PRT_CHAN_PROP_ACC_SINGLE                   ( 0x0000U )
#define DI_PRT_CHAN_PROP_ACC_MULTI                    ( 0x0100U )
#define DI_PRT_CHAN_PROP_ACC_MASK                     ( 0x0100U )
#define DI_PRT_CHAN_PROP_MAINT_FAULT                  ( 0x0000U )
#define DI_PRT_CHAN_PROP_MAINT_REQUIRED               ( 0x0200U )
#define DI_PRT_CHAN_PROP_MAINT_DEMANDED               ( 0x0400U )
#define DI_PRT_CHAN_PROP_MAINT_MASK                   ( 0x0600U )
#define DI_PRT_CHAN_PROP_DIR_MANF_SPEC                ( 0x0000U )
#define DI_PRT_CHAN_PROP_DIR_IN                       ( 0x2000U )
#define DI_PRT_CHAN_PROP_DIR_OUT                      ( 0x4000U )
#define DI_PRT_CHAN_PROP_DIR_INOUT                    ( 0x6000U )
#define DI_PRT_CHAN_PROP_DIR_MASK                     ( 0xE000U )

/*
** Constants for the 'ChannelErrorType' field.
** Reference: PROFINET International doc.no 2.722, chapter 5.2.8.2, "Coding of the field ChannelErrorType"
**
** NOTE:
** - Only the predefined error codes from 'range 1' has corresponding constants
**   defined here.
** - Vendor- and profile-specific error codes are possible to use, but the ABCC
**   reserves vendor-specific error code 0x7000 for the reporting of various
**   SYNC-related problems. Check the PROFINET network manual for details.
** - Error codes related to functional safety (0x0040 - 0x005F) and to
**   interface/media-related error (0x8000 - 0x8010) are reserved for the ABCC.
*/
enum
{
   DI_PRT_CHAN_ERR_TYPE_RESERVED_0000          = 0x0000, /* Reserved, do not use. */
   DI_PRT_CHAN_ERR_TYPE_SHORT_CIRCUIT          = 0x0001,
   DI_PRT_CHAN_ERR_TYPE_UNDERVOLTAGE           = 0x0002,
   DI_PRT_CHAN_ERR_TYPE_OVERVOLTAGE            = 0x0003,
   DI_PRT_CHAN_ERR_TYPE_OVERLOAD               = 0x0004,
   DI_PRT_CHAN_ERR_TYPE_OVERTEMPERATURE        = 0x0005,
   DI_PRT_CHAN_ERR_TYPE_WIRE_BREAK             = 0x0006,
   DI_PRT_CHAN_ERR_TYPE_UPPER_LIMIT_EXCEEDED   = 0x0007,
   DI_PRT_CHAN_ERR_TYPE_LOWER_LIMIT_EXCEEDED   = 0x0008,
   DI_PRT_CHAN_ERR_TYPE_ERROR                  = 0x0009,
   DI_PRT_CHAN_ERR_TYPE_SIMULATION_ACTIVE      = 0x000A,
   DI_PRT_CHAN_ERR_TYPE_RESERVED_000B          = 0x000B, /* Reserved, do not use. */
   DI_PRT_CHAN_ERR_TYPE_RESERVED_000C          = 0x000C, /* Reserved, do not use. */
   DI_PRT_CHAN_ERR_TYPE_RESERVED_000D          = 0x000D, /* Reserved, do not use. */
   DI_PRT_CHAN_ERR_TYPE_RESERVED_000E          = 0x000E, /* Reserved, do not use. */
   DI_PRT_CHAN_ERR_TYPE_PARAMETER_MISSING      = 0x000F,
   DI_PRT_CHAN_ERR_TYPE_PARAMETERIZATION_FAULT = 0x0010,
   DI_PRT_CHAN_ERR_TYPE_POWER_SUPPLY_FAULT     = 0x0011,
   DI_PRT_CHAN_ERR_TYPE_FUSE_BLOWN             = 0x0012,
   DI_PRT_CHAN_ERR_TYPE_COMMUNICATION_FAULT    = 0x0013,
   DI_PRT_CHAN_ERR_TYPE_GROUND_FAULT           = 0x0014,
   DI_PRT_CHAN_ERR_TYPE_REFERENCE_POINT_LOST   = 0x0015,
   DI_PRT_CHAN_ERR_TYPE_SAMPLING_ERROR         = 0x0016,
   DI_PRT_CHAN_ERR_TYPE_THRESHOLD_WARNING      = 0x0017,
   DI_PRT_CHAN_ERR_TYPE_OUTPUT_DISABLED        = 0x0018,
   DI_PRT_CHAN_ERR_TYPE_SAFETY_EVENT           = 0x0019,
   DI_PRT_CHAN_ERR_TYPE_EXTERNAL_FAULT         = 0x001A,
   DI_PRT_CHAN_ERR_TYPE_MANF_SPEC_001B         = 0x001B,
   DI_PRT_CHAN_ERR_TYPE_MANF_SPEC_001C         = 0x001C,
   DI_PRT_CHAN_ERR_TYPE_MANF_SPEC_001D         = 0x001D,
   DI_PRT_CHAN_ERR_TYPE_MANF_SPEC_001E         = 0x001E,
   DI_PRT_CHAN_ERR_TYPE_TEMPORARY_FAULT        = 0x001F
};

/*
** At the moment this file does not define any constants for the
** ExtChannelErrorType or ExtChannelAddValue fields.
** Reference: PROFINET International doc.no 2.722, chapter 5.2.8.5, "Coding of the field ExtChannelErrorType"
** Reference: PROFINET International doc.no 2.722, chapter 5.2.8.6, "Coding of the field ExtChannelAddValue"
*/

/*
** Constants for the QualifiedChannelQualifier field.
** Reference: PROFINET International doc.no 2.722, chapter 5.2.8.7, "Coding of the field QualifiedChannelQualifier"
*/
enum
{
   DI_PRT_CHAN_QUAL_03_ADVICE    = 3,
   DI_PRT_CHAN_QUAL_04_ADVICE    = 4,
   DI_PRT_CHAN_QUAL_05_ADVICE    = 5,
   DI_PRT_CHAN_QUAL_06_ADVICE    = 6,
   DI_PRT_CHAN_QUAL_07_MAINT_REQ = 7,
   DI_PRT_CHAN_QUAL_08_MAINT_REQ = 8,
   DI_PRT_CHAN_QUAL_09_MAINT_REQ = 9,
   DI_PRT_CHAN_QUAL_10_MAINT_REQ = 10,
   DI_PRT_CHAN_QUAL_11_MAINT_REQ = 11,
   DI_PRT_CHAN_QUAL_12_MAINT_REQ = 12,
   DI_PRT_CHAN_QUAL_13_MAINT_REQ = 13,
   DI_PRT_CHAN_QUAL_14_MAINT_REQ = 14,
   DI_PRT_CHAN_QUAL_15_MAINT_REQ = 15,
   DI_PRT_CHAN_QUAL_16_MAINT_REQ = 16,
   DI_PRT_CHAN_QUAL_17_MAINT_DEM = 17,
   DI_PRT_CHAN_QUAL_18_MAINT_DEM = 18,
   DI_PRT_CHAN_QUAL_19_MAINT_DEM = 19,
   DI_PRT_CHAN_QUAL_20_MAINT_DEM = 20,
   DI_PRT_CHAN_QUAL_21_MAINT_DEM = 21,
   DI_PRT_CHAN_QUAL_22_MAINT_DEM = 22,
   DI_PRT_CHAN_QUAL_23_MAINT_DEM = 23,
   DI_PRT_CHAN_QUAL_24_MAINT_DEM = 24,
   DI_PRT_CHAN_QUAL_25_MAINT_DEM = 25,
   DI_PRT_CHAN_QUAL_26_MAINT_DEM = 26,
   DI_PRT_CHAN_QUAL_27_FAULT     = 27,
   DI_PRT_CHAN_QUAL_28_FAULT     = 28,
   DI_PRT_CHAN_QUAL_29_FAULT     = 29,
   DI_PRT_CHAN_QUAL_30_FAULT     = 30,
   DI_PRT_CHAN_QUAL_31_FAULT     = 31
};

/*------------------------------------------------------------------------------
** DI object-specific and PROFINET-specific message error codes.
**------------------------------------------------------------------------------
*/
#define DI_PRT_NW_SPEC_ERR_NO_SUCH_API                      ( 0x03U )
#define DI_PRT_NW_SPEC_ERR_NO_MODULE_IN_SPEC_SLOT           ( 0x04U )
#define DI_PRT_NW_SPEC_ERR_NO_SUBMODULE_IN_SPEC_SUBSLOT     ( 0x05U )
#define DI_PRT_NW_SPEC_ERR_SLOT_OUT_OF_RANGE                ( 0x06U )
#define DI_PRT_NW_SPEC_ERR_SUBSLOT_OUT_OF_RANGE             ( 0x07U )
#define DI_PRT_NW_SPEC_ERR_ADD_ENTRY_FAILED                 ( 0x08U )
#define DI_PRT_NW_SPEC_ERR_SEND_ALARM_FAILED                ( 0x09U )
#define DI_PRT_NW_SPEC_ERR_CHAN_NUM_OUT_OF_RANGE            ( 0x0AU )
#define DI_PRT_NW_SPEC_ERR_CHAN_PROP_TYPE_OUT_OF_RANGE      ( 0x0BU )
#define DI_PRT_NW_SPEC_ERR_CHAN_PROP_DIR_OUT_OF_RANGE       ( 0x0CU )
#define DI_PRT_NW_SPEC_ERR_CHAN_PROP_ACC_OUT_OF_RANGE       ( 0x0DU )
#define DI_PRT_NW_SPEC_ERR_CHAN_PROP_MAINTREQ_OUT_OF_RANGE  ( 0x0EU )
#define DI_PRT_NW_SPEC_ERR_CHAN_PROP_MAINTDEM_OUT_OF_RANGE  ( 0x0FU )
#define DI_PRT_NW_SPEC_ERR_USER_STRUCT_IDENT_OUT_OF_RANGE   ( 0x10U )
#define DI_PRT_NW_SPEC_ERR_CHAN_ERRTYPE_OUT_OF_RANGE        ( 0x11U )
#define DI_PRT_NW_SPEC_ERR_UNKNOWN_ERROR                    ( 0xFFU )

#endif /* PRT_OBJ_ENABLE */

/*******************************************************************************
** Public typedefs
********************************************************************************
*/

/*******************************************************************************
** Public globals
********************************************************************************
*/

/*******************************************************************************
** Public services
********************************************************************************
*/

/*------------------------------------------------------------------------------
** NOTE:
**
** - The message builder functions declared here only *builds* a command, i.e.
**   writes each argument with correct formatting and alignment to the correct
**   position in the given message buffer, but it does not *send* the command.
**   That task is left to the user, either via the standard message API 
**   functions or via the command sequencer.
**
** - The message builder functions do not assign a new 'Source ID' value to the
**   built command. That must be done by the user before sending the command.
**
** - It is not possible to fully validate all aspects of all arguments when
**   building the command, some checks can only be made by the ABCC, so the
**   response from the ABCC *must* be evaluated by the user even if the command
**   builders here do not return errors.
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
** Message builder function for a basic 'create diagnostic event' command for
** the Diagnostic object.
**------------------------------------------------------------------------------
** Arguments:
**    psMsg     - Pointer to the message buffer to write to.
**    eSeverity - Severity level of the event, see "ABP_DI_EVENT_SEVERITY_..."
**                in "abp.h"
**    eEvent    - Event code, see "ABP_DI_EVENT_..." in "abp.h"
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID for invalid arguments.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType DI_BuildCmdCreateDiag( ABP_MsgType* psMsg, ABP_DiEventSeverityType eSeverity, ABP_DiEventCodeType eEvent );

/*------------------------------------------------------------------------------
** Message builder function that adds 'extended diagnostic data' to an existing
** 'create diagnostic event' command. The "DI_BuildCmdCreateDiag()" must first
** have been called. 
**------------------------------------------------------------------------------
** Arguments:
**    psMsg    - Pointer to the message buffer to write to.
**    iSlot    - Slot number associated with the diag. event. Set to '0' if
**               unknown or unsupported
**    iAdi     - ADI associated with the diag. event. Set to '0' if unknown or
**               unsupported.
**    bElement - Element assciated with the diag. event. Set to '255' if
**               unknown or unsupported.
**    bBit     - Bit in element associated with the diag. event. Set to '255'
**               if unknown or unsupported.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID for invalid arguments.
**    ABCC_EC_INCORRECT_STATE if the message buffer does not already contain a
**    'create diagnostic event' command for the DI object.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType DI_BuildCmdAddExtDiag( ABP_MsgType* psMsg, UINT16 iSlot, UINT16 iAdi, UINT8 bElement, UINT8 bBit );

#if PRT_OBJ_ENABLE

/*------------------------------------------------------------------------------
** Message builder functions that adds a ChannelDiagnosis,
** ExtChannelDiagnosis, or QualifiedChannelDiagnosis structures to an existing
** 'create diagnostic event' command. The generic command builders must first
** have been called.
**
** As with the generic diagnostics builders above those functions are intended
** to be used 'in series':
**
** - "DI_PRT_BuildCmdAddChannelDiag()" will add a ChannelDiagnosis structure to
**   an existing 'create diagnostic event' command.
**
** - "DI_PRT_BuildCmdAddExtChannelDiag()" will convert a ChannelDiagnosis
**   structure to an ExtChannelDiagnosis structure, and append the extended
**   error codes.
**
** - "DI_PRT_BuildCmdAddQualChannelDiag()" will convert a ExtChannelDiagnosis
**   structure to a QualifiedChannelDiagnosis structure, and append the channel
**   qualifier.
**------------------------------------------------------------------------------
** Arguments:
**    psMsg                - Pointer to the message buffer to write to.
**    iChannelNumber       - Channel number, or DI_PRT_CHAN_NUM_ENTIRE_SUBMODULE
**                           if the diagnostics is applicable to the entire
**                           submodule.
**    iChannelProperties   - Bitfield for the DI_PRT_CHAN_PROP_xxx masks above.
**    iChannelErrorType    - Error code, see DI_PRT_CHAN_ERR_TYPE_xxx above.
**    iExtChannelErrorType - Extended error code.
**    lExtChannelAddValue  - Additional error code value.
**    bChannelQualifier    - Severity qualifier, see DI_PRT_CHAN_QUAL_xxx
**                           above.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID for invalid arguments.
**    ABCC_EC_INCORRECT_STATE if the message buffer does not contain a valid
**    'Create' command for the DI object.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType DI_PRT_BuildCmdAddChannelDiag( ABP_MsgType* psMsg, UINT16 iChannelNumber, UINT16 iChannelProperties, UINT16 iChannelErrorType );
EXTFUNC ABCC_ErrorCodeType DI_PRT_BuildCmdAddExtChannelDiag( ABP_MsgType* psMsg, UINT16 iExtChannelErrorType, UINT32 lExtChannelAddValue );
EXTFUNC ABCC_ErrorCodeType DI_PRT_BuildCmdAddQualChannelDiag( ABP_MsgType* psMsg, UINT8 bChannelQualifier );

#endif /* PRT_OBJ_ENABLE */

#endif /* DI_OBJ_ENABLE */

#endif  /* inclusion lock */

#if 0
/*------------------------------------------------------------------------------
** Example of how to use the generic "DI_BuildCmdCreateDiag()" and
** "DI_BuildCmdAddExtDiag()" functions to report diagnostic information.
**------------------------------------------------------------------------------
*/

/*
** NOTE:
** The message API of the ABCC driver is not covered here, it is assumed that
** the "psMsg" pointer used here points to a previously allocated message
** buffer that we can build the 'create diagnostic event' command in. Likewise
** the return value of the builder function is not validated in this example.
*/

/*
** Let's assume two ADIs which are both mapped as Process Data. We also assume
** that the LSB of each UINT8 is channel / bit number 0.
*/

{ 1, "In8",  ABP_UINT8, 1, AD_ADI_DESC___W_G, { { &bInputs,  NULL } } },
{ 2, "Out8", ABP_UINT8, 1, AD_ADI_DESC__R_SG, { { &bOutputs, NULL } } },

{ 1, PD_WRITE, AD_MAP_ALL_ELEM, 0 },
{ 2, PD_READ,  AD_MAP_ALL_ELEM, 0 },

/*
** Now, our monitoring hardware for the outputs has detected a short circut on
** channel / output bit 2 of our output module, and using the generic severity
** and event codes we will report that as a "major, recoverable" severity and
** "current, device, output" event code.
*/

/*
** Build a 'Create diag. event' command with a "major, recovrable" severity,
** and "'current, device, output" event code.
*/
DI_BuildCmdCreateDiag(
   psMsg,
   ABP_DI_EVENT_SEVERITY_MAJOR_REC,
   ABP_DI_EVENT_CURRENT_DEVICE_OUTPUT
);

/*
** Also add an 'Extended diagnostic data' block in order to provide some more
** detail about the event to the network-side diag. data.
**
** NOTE:
** The exact translation mechanism for the extended diag. data is network-
** specific. Check the network manual for the specific network for details.
*/
DI_BuildCmdAddExtDiag(
   psMsg,
   0,    /* Slot 0, the SWDG says "Set to zero if unknown or unsupported. */
   2,    /* ADI 2 */
   0,    /* There is only one element in the ADI (the UINT8) which is */
         /* index/element 0.                                          */
   2     /* Bit in element */
);

/*
** Do not forget to assign a free 'Source ID' to the command message before
** sending it, the message builder functions do not do that.
*/
ABCC_SetMsgSourceId( psMsg, ABCC_GetNewSourceId() );

/*
** The command message is now ready to be sent to the ABCC. This will be
** handled by the command sequencer if that is used, otherwise one need to 
** call ABCC_SendCmdMessage() manually.
**
** The response from the ABCC must also be evaluated when it arrives. The
** message builders here are not aware of things like resource constraints
** inside the ABCC, and cannot check for this before the command message is
** sent.
*/
#endif

#if 0
/*------------------------------------------------------------------------------
** Example of how to add PROFINET-specific diagnostic information to an
** existing 'create diagnostic event' command.
**
** NOTE:
** The functions, constants, bitmasks, values, and descriptions here are not
** intended to be a substitue for the PROFINET specification. It is assumed
** that an implementor that decides to add PROFINET-specific diagnostics to
** their device understands how PROFINET diagnostics works, and has access to
** this specification.
** 
** References:
** - PROFINET International doc.no 2.712, chapter 7.3.4, "Diagnosis ASE"
** - PROFINET International doc.no 2.722, chapter 5.2.8, "Coding section related to Alarm and Diagnosis Data"
** - PROFINET International doc.no 7.142, "Diagonsis for PROFINET, Guideline for PROFINET"
**------------------------------------------------------------------------------
*/

/*
** We are assuming the same ADIs as above, and that we want to report the same
** fault (a short circuit) using PROFINET-specific diagnostics.
*/

/*
** Call "DI_BuildCmdCreateDiag()" to build the basic 'create diag. event'
** command. The severity and event values will not matter as the network-
** specific data will override them.
*/
DI_BuildCmdCreateDiag(
   psMsg,
   ABP_DI_EVENT_SEVERITY_MINOR_REC, /* Placeholder value. */
   ABP_DI_EVENT_GENERIC_ERROR       /* Placeholder value. */
);

/*
** Add an 'Extended diagnostic data' block to the message. This is required by
** this implementation since the Module and Submodule identifier values in the
** PROFINET Alarm data otherwise only will have placeholder values.
**
** Note that the 'Bit number' argument is presently not used by the ABCC40-PIR.
*/
DI_BuildCmdAddExtDiag(
   psMsg,
   2,    /* Slot 2 */
   2,    /* ADI 2 */
   0,    /* There is only one element in the ADI */
   255   /* Not used by the ABCC40-PIR */
);

/*
** Add the ChannelDiagnosis structure to the command. This message builder
** will change the existing "ABP_DiEventCodeType" event type to
** ABP_DI_EVENT_NW_SPECIFIC, add the correct PROFINET UserStructureIdentifier,
** and write the ChannelNumber, ChannelProperties.Direction,
** ChannelProperties.Maintenance, ChannelProperties.Accumulative,
** ChannelProperties.Type, and the ChannelErrorType values to the correct
** places.
*/
DI_PRT_BuildCmdAddChannelDiag(
   psMsg,
   2,
   DI_PRT_CHAN_PROP_DIR_OUT | DI_PRT_CHAN_PROP_MAINT_FAULT | DI_PRT_CHAN_PROP_ACC_SINGLE | DI_PRT_CHAN_PROP_TYPE_1BIT,
   DI_PRT_CHAN_ERR_TYPE_SHORT_CIRCUIT
);

/*
** The command can be sent as-is, but can also be extended with the two
** remaining command builders. Calling "DI_PRT_BuildCmdAddExtChannelDiag()"
** will change the UserStructureIdentifier from ChannelDiagnosis to
** ExtChannelDiagnosis, and add the ExtChannelErrorType and ExtChannelAddValue
** values to the correct places. In this case there is no more error
** information to supply, but those fields needs to be persent if a qualifier
** is to be added.
*/
DI_PRT_BuildCmdAddExtChannelDiag(
   psMsg,
   0, /* "Unspecified" */
   0  /* "No additional information" */
);

/*
** If a channel qualifier is to be added, "DI_PRT_BuildCmdAddQualChannelDiag()"
** will change the UserStructureIdentifier to QualifiedChannelDiagnosis and add
** the qualifier in the correct location.
*/
DI_PRT_BuildCmdAddQualChannelDiag(
   psMsg,
   DI_PRT_CHAN_QUAL_31_FAULT
);

/*
** And as above, do not forget to assign a free 'Source ID' to the command
** message before sending it, the message builder functions do not do that.
*/
ABCC_SetMsgSourceId( psMsg, ABCC_GetNewSourceId() );

/*
** If the ABCC responds with the error code ABP_DI_ERR_NW_SPECIFIC the
** remaining error codes are network-specific and can be matched with the
** constants given above.
*/

#endif