/*******************************************************************************
** Copyright 2025-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** This file is an extension of the simplified API for Anybus CompactCom driver
** and provides an interface to some network configuration settings.
********************************************************************************
*/

#ifndef ABCC_API_NW_CONF_H
#define ABCC_API_NW_CONF_H

#include "abcc_types.h"

#define PROFINET_STATION_NAME_MAX_LENGTH 240

typedef struct
{
    char   stationName[PROFINET_STATION_NAME_MAX_LENGTH];
    UINT8  length;
} abStationName_t;

/*------------------------------------------------------------------------------
**  Comm settings:
**
**  ABCC_API_COMM_SETTING_AUTO         - Auto negotiation
**  ABCC_API_COMM_SETTING_10HDX        - 10 Mbit/s half duplex
**  ABCC_API_COMM_SETTING_10FDX        - 10 Mbit/s full duplex
**  ABCC_API_COMM_SETTING_100HDX       - 100 Mbit/s half duplex
**  ABCC_API_COMM_SETTING_100FDX       - 100 Mbit/s full duplex
**  ABCC_API_COMM_SETTING_AUTO_100FDX  - Auto negotiation with only 100 Mbit/s
**                                       full duplex in advertisment
**  ABCC_API_COMM_SETTING_AUTO_1000FDX - Auto negotiation with only 1 Gbit/s
**                                       full duplex in advertisment
**------------------------------------------------------------------------------
*/
typedef enum ABCC_API_CommSetting
{
   ABCC_API_COMM_SETTING_AUTO = 0,
   ABCC_API_COMM_SETTING_10HDX,
   ABCC_API_COMM_SETTING_10FDX,
   ABCC_API_COMM_SETTING_100HDX,
   ABCC_API_COMM_SETTING_100FDX,
   ABCC_API_COMM_SETTING_AUTO_100FDX,
   ABCC_API_COMM_SETTING_AUTO_1000FDX,
}
ABCC_API_CommSettingType;

/*------------------------------------------------------------------------------
** Set HW switch 1 value to the application. The value is used to generate the
** the node address or the IP address (192.168.0.X) depending on network.
** For networks that uses an 8 bit node address the upper 8 bits of the address
** will be discarded.
**
** NOTE: For networks that uses an IP address and if this function is called
**       with address 0 no address will be set to the ABCC.
**------------------------------------------------------------------------------
** Arguments:
**    iSwitchValue - Switch 1 value
**
** Returns:
**    True:  bSwitchValue will be the new address
**    False: Update of address already in progress
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_SetAddress( UINT16 iSwitchValue );

/*------------------------------------------------------------------------------
** Set HW switch 2 value to the application. The value is used to generate the
** baud rate for networks that implements network configuration object instance
** 2.
**------------------------------------------------------------------------------
** Arguments:
**    bSwitchValue - Switch 2 value
**
** Returns:
**    True:  bSwitchValue will be the new baudrate
**    False: Update of baudrate already in progress
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_SetBaudrate( UINT8 bSwitchValue );

/*------------------------------------------------------------------------------
** Set Comm settings (speed/duplex) for ports.
**------------------------------------------------------------------------------
** Arguments:
**
**    eCommSettings1 - Settings for port 1
**    eCommSettings2 - Settings for port 2
**
** Returns:
**    None
**------------------------------------------------------------------------------
*/
EXTFUNC void ABCC_API_SetCommSettings( ABCC_API_CommSettingType eCommSettings1,
                                       ABCC_API_CommSettingType eCommSettings2 );


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

/*------------------------------------------------------------------------------
** Start the sequence to write the station name to the module
**------------------------------------------------------------------------------
** Arguments:
**    stationName - Station name to write to the module
**
** Returns:
**    BOOL - TRUE if the sequence was started, FALSE if a sequence is already in progress
**------------------------------------------------------------------------------
*/
EXTFUNC BOOL ABCC_API_SetStationId(abStationName_t* stationName);

#endif  /* inclusion lock */
