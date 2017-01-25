/**
 * @file pa_antenna_simu.h
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_antenna.h"


#ifndef LEGATO_PA_ANTENNA_SIMU_INCLUDE_GUARD
#define LEGATO_PA_ANTENNA_SIMU_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * Set the return code.
 *
 * @return
 *     - void
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_antennaSimu_SetReturnCode
(
    le_result_t res
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the short limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetShortLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t             shortLimit
);


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the short limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetShortLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t*            shortLimitPtr
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the open limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetOpenLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t             openLimit
);


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the open limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetOpenLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t*            openLimitPtr
);


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the antenna status
 *
 * @return
 * - LE_OK on success
 * - LE_UNSUPPORTED request not supported
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetStatus
(
    le_antenna_Type_t    antennaType,
    le_antenna_Status_t* statusPtr
);


//--------------------------------------------------------------------------------------------------
/**
 * Set the external ADC used to monitor the requested antenna.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT on other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetExternalAdc
(
    le_antenna_Type_t    antennaType,   ///< Antenna type
    int8_t               adcId          ///< The external ADC used to monitor the requested antenna
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the external ADC used to monitor the requested antenna.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT on other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetExternalAdc
(
    le_antenna_Type_t    antennaType,  ///< Antenna type
    int8_t*              adcIdPtr      ///< The external ADC used to monitor the requested antenna
);


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the status indication on a specific antenna
 *
 * * @return
 * - LE_OK on success
 * - LE_BUSY if the status indication is already set for the requested antenna
 * - LE_FAULT on other failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetStatusIndication
(
    le_antenna_Type_t antennaType
);


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to remove the status indication on a specific antenna
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_RemoveStatusIndication
(
    le_antenna_Type_t antennaType
);


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to add a status notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t* pa_antenna_AddStatusHandler
(
    pa_antenna_StatusIndHandlerFunc_t   msgHandler
);


//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to intialize the PA antenna
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_Init
(
    void
);



#endif

