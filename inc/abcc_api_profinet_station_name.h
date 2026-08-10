/*******************************************************************************
** Copyright 2026-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Interface for reading and writing profinet station name.
********************************************************************************
*/

#ifndef ABCC_API_PROFINET_STATION_NAME_H
#define ABCC_API_PROFINET_STATION_NAME_H

#include "../src/abcc_api_config.h"
#include "abcc_error_codes.h"
#include "abcc_types.h"
#include "abcc_api_network_settings.h"

/*------------------------------------------------------------------------------
** Report back the current station name in the module. This is a part of the update station id sequence. The station
** name is read from the module and reported back to the application.
**------------------------------------------------------------------------------
** Arguments:
**    stationName - Station name read from the module
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
void ABCC_CbfApplicationProfinetStationName(abStationName_t stationName);

/*------------------------------------------------------------------------------
** Start the sequence to read out the station name from the module
**------------------------------------------------------------------------------
** Arguments:
**    None
**
** Returns:
**    BOOL - TRUE if the sequence was started, FALSE if a sequence is already in progress
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_UpdateStationId(void);

EXTFUNC BOOL ABCC_API_SetStationId(abStationName_t* stationName);

#endif // ABCC_API_PROFINET_STATION_NAME_H
