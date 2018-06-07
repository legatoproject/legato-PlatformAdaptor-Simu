/**
 * @file pa_gnss_simu.c
 *
 * Simulation implementation of @ref c_pa_gnss API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include <pa_gnss.h>
#include "pa_gnss_simu.h"

//--------------------------------------------------------------------------------------------------
/**
 * Set the NMEA string for NMEA handler
 */
//--------------------------------------------------------------------------------------------------
#define NMEA_STR_LEN                  32

//--------------------------------------------------------------------------------------------------
/**
 * Set the suplCertificateId length
 */
//--------------------------------------------------------------------------------------------------
#define SUPL_CERTIFICATE_ID_LEN        9

//--------------------------------------------------------------------------------------------------
/**
 * Position event ID used to report position events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t        GnssEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Nmea event ID used to report Nmea events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t        NmeaEventId;

//--------------------------------------------------------------------------------------------------
/**
 * The computed position data.
 */
//--------------------------------------------------------------------------------------------------
static pa_Gnss_Position_t   GnssPositionData;

//--------------------------------------------------------------------------------------------------
/**
 * Memory pool for position event data.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t     PositionEventDataPool;

//--------------------------------------------------------------------------------------------------
/**
 * Memory pool for Nmea event data.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t     NmeaEventDataPool;

//--------------------------------------------------------------------------------------------------
/**
 * The configured SUPL assisted mode.
 */
//--------------------------------------------------------------------------------------------------
static le_gnss_AssistedMode_t SuplAssistedMode = LE_GNSS_STANDALONE_MODE;

//--------------------------------------------------------------------------------------------------
/**
 * The configured bit mask for NMEA.
 */
//--------------------------------------------------------------------------------------------------
static le_gnss_NmeaBitMask_t NmeaBitMask = LE_GNSS_NMEA_MASK_GPGGA;

//--------------------------------------------------------------------------------------------------
/**
 * Multiplying factor accuracy
 */
//--------------------------------------------------------------------------------------------------
#define ONE_DECIMAL_PLACE_ACCURACY   (10)
#define TWO_DECIMAL_PLACE_ACCURACY   (100)
#define THREE_DECIMAL_PLACE_ACCURACY (1000)
#define SIX_DECIMAL_PLACE_ACCURACY   (1000000)

//--------------------------------------------------------------------------------------------------
/**
 * Inline function to convert and round to the nearest
 *
 * The function firstly converts the double value to int according to the requested place after the
 * decimal given by place parameter. Secondly, a round to the nearest is done the int value.
 */
//--------------------------------------------------------------------------------------------------
static inline int32_t ConvertAndRoundToNearest
(
    double value,    ///< [IN] value to round to the nearest
    int32_t place    ///< [IN] the place after the decimal in power of 10
)
{
    return (int32_t)((int64_t)((place*value*10) + ((value > 0) ? 5 : -5))/10);
}

//--------------------------------------------------------------------------------------------------
/**
 * GNSS position default pointer initialization.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeDefaultGnssPositionData
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    posDataPtr->fixState = LE_GNSS_STATE_FIX_NO_POS;
    posDataPtr->altitudeValid = false;
    posDataPtr->altitudeAssumedValid = false;
    posDataPtr->altitudeOnWgs84Valid = false;
    posDataPtr->dateValid = false;
    posDataPtr->hdopValid = false;
    posDataPtr->hSpeedUncertaintyValid = false;
    posDataPtr->hSpeedValid = false;
    posDataPtr->hUncertaintyValid = false;
    posDataPtr->latitudeValid = false;
    posDataPtr->longitudeValid = false;
    posDataPtr->timeValid = false;
    posDataPtr->gpsTimeValid = false;
    posDataPtr->timeAccuracyValid = false;
    posDataPtr->positionLatencyValid = false;
    posDataPtr->directionUncertaintyValid = false;
    posDataPtr->directionValid = false;
    posDataPtr->vdopValid = false;
    posDataPtr->vSpeedUncertaintyValid = false;
    posDataPtr->vSpeedValid = false;
    posDataPtr->vUncertaintyValid = false;
    posDataPtr->pdopValid = false;
    posDataPtr->satMeasValid = false;
    posDataPtr->leapSecondsValid = false;
    posDataPtr->gdopValid = false;
    posDataPtr->tdopValid = false;

    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satMeas[i].satId = 0;
        posDataPtr->satMeas[i].satLatency = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * GNSS position valid pointer initialization.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeValidGnssPositionData
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    posDataPtr->fixState = LE_GNSS_STATE_FIX_NO_POS;
    posDataPtr->altitudeValid = true;
    posDataPtr->altitudeAssumed = false;
    posDataPtr->altitude = 10;
    posDataPtr->altitudeOnWgs84Valid = true;
    posDataPtr->altitudeOnWgs84 = 10378;
    posDataPtr->dateValid = true;
    posDataPtr->date.year = 2017;
    posDataPtr->date.month = 10;
    posDataPtr->date.day = 4;
    posDataPtr->hdopValid = true;
    posDataPtr->hdop = 5000;
    posDataPtr->hSpeedUncertaintyValid = true;
    posDataPtr->hSpeedUncertainty = 1000;
    posDataPtr->hSpeedValid = true;
    posDataPtr->hSpeed = 20;
    posDataPtr->hUncertaintyValid = true;
    posDataPtr->hUncertainty = 100;
    posDataPtr->latitudeValid = true;
    posDataPtr->latitude = 37981;
    posDataPtr->longitudeValid = true;
    posDataPtr->longitude = 91078;
    posDataPtr->timeValid = true;
    posDataPtr->epochTime = 1000;
    posDataPtr->gpsTimeValid = true;
    posDataPtr->gpsWeek = 7;
    posDataPtr->gpsTimeOfWeek = 5;
    posDataPtr->time.hours = 23;
    posDataPtr->time.minutes = 59;
    posDataPtr->time.seconds = 50;
    posDataPtr->time.milliseconds = 100;
    posDataPtr->timeAccuracyValid = true;
    posDataPtr->timeAccuracy = 100000;
    posDataPtr->positionLatencyValid = true;
    posDataPtr->positionLatency = 109831;
    posDataPtr->directionUncertaintyValid = true;
    posDataPtr->directionUncertainty = 21987;
    posDataPtr->directionValid = true;
    posDataPtr->direction = 11576;
    posDataPtr->vdopValid = true;
    posDataPtr->vdop = 6000;
    posDataPtr->vSpeedUncertaintyValid = true;
    posDataPtr->vSpeedUncertainty = 5000;
    posDataPtr->vSpeedValid = true;
    posDataPtr->vSpeed = 50;
    posDataPtr->vUncertaintyValid = true;
    posDataPtr->vUncertainty = 8000;
    posDataPtr->pdopValid = true;
    posDataPtr->pdop = 7000;
    posDataPtr->leapSecondsValid = true;
    posDataPtr->leapSeconds = 30;
    posDataPtr->gdopValid = true;
    posDataPtr->gdop = 8000;
    posDataPtr->tdopValid = true;
    posDataPtr->tdop = 9000;

    posDataPtr->satMeasValid = true;
    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satMeas[i].satId = 0;
        posDataPtr->satMeas[i].satLatency = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize default satellites info that are updated in SV information report indication.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeDefaultSatInfo
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    posDataPtr->satsInViewCountValid = false;
    posDataPtr->satsTrackingCountValid = false;
    posDataPtr->satInfoValid = false;
    posDataPtr->magneticDeviationValid = false;

    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satInfo[i].satId = 0;
        posDataPtr->satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_UNDEFINED;
        posDataPtr->satInfo[i].satTracked = 0;
        posDataPtr->satInfo[i].satSnr = 0;
        posDataPtr->satInfo[i].satAzim = 0;
        posDataPtr->satInfo[i].satElev = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize valid satellites info that are updated in SV information report indication.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeValidSatInfo
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    posDataPtr->satsInViewCountValid = true;
    posDataPtr->satsInViewCount = 10;
    posDataPtr->satsTrackingCountValid = true;
    posDataPtr->satsTrackingCount = 8;
    posDataPtr->magneticDeviationValid = true;
    posDataPtr->magneticDeviation = 20;
    posDataPtr->satInfoValid = true;

    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satInfo[i].satId = 0;
        posDataPtr->satInfo[i].satConst = LE_GNSS_SV_CONSTELLATION_UNDEFINED;
        posDataPtr->satInfo[i].satTracked = 0;
        posDataPtr->satInfo[i].satSnr = 0;
        posDataPtr->satInfo[i].satAzim = 0;
        posDataPtr->satInfo[i].satElev = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize default satellites info that are updated as "used" in SV information report
 * indication.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeDefaultSatUsedInfo
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    // Reset the SV marked as used in position data list.
    posDataPtr->satsUsedCountValid = false;
    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satInfo[i].satUsed = false;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize valid satellites info that are updated as "used" in SV information report
 * indication.
 */
//--------------------------------------------------------------------------------------------------
static void InitializeValidSatUsedInfo
(
    pa_Gnss_Position_t* posDataPtr  // [IN/OUT] Pointer to the position data.
)
{
    int i;
    // Reset the SV marked as used in position data list.
    posDataPtr->satsUsedCountValid = true;
    posDataPtr->satsUsedCount = 5;
    for (i=0; i<LE_GNSS_SV_INFO_MAX_LEN; i++)
    {
        posDataPtr->satInfo[i].satUsed = 5;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to simulate gnss init PA gnss Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Init
(
    void
)
{
    InitializeDefaultGnssPositionData(&GnssPositionData);
    InitializeDefaultSatInfo(&GnssPositionData);
    InitializeDefaultSatUsedInfo(&GnssPositionData);

    GnssEventId = le_event_CreateIdWithRefCounting("GnssEventId");
    NmeaEventId = le_event_CreateIdWithRefCounting("GnssNmeaEventId");

    PositionEventDataPool = le_mem_CreatePool("PositionEventDataPool", sizeof(pa_Gnss_Position_t));
    NmeaEventDataPool = le_mem_CreatePool("NmeaEventDataPool", NMEA_STR_LEN * sizeof(char));
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize valid position data
 */
//--------------------------------------------------------------------------------------------------
void pa_gnssSimu_SetGnssValidPositionData
(
    void
)
{
    InitializeValidGnssPositionData(&GnssPositionData);
    InitializeValidSatInfo(&GnssPositionData);
    InitializeValidSatUsedInfo(&GnssPositionData);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function tests the rounding to the nearest of different position values
 *
 * @return LE_FAULT         The function failed
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnssSimu_RoundingPositionValues
(
    void
)
{
    if (0 != ConvertAndRoundToNearest(0.0, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 1");
        return LE_FAULT;
    }

    if (2565656 != ConvertAndRoundToNearest(2.5656563, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 2");
        return LE_FAULT;
    }
    if (2565657 != ConvertAndRoundToNearest(2.5656566, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 3");
        return LE_FAULT;
    }
    if (2565650 != ConvertAndRoundToNearest(2.565650, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 4");
        return LE_FAULT;
    }
    if (2565600 != ConvertAndRoundToNearest(2.5656, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 5");
        return LE_FAULT;
    }
    if (100565657 != ConvertAndRoundToNearest(100.5656566, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 6");
        return LE_FAULT;
    }
    if (100700000 != ConvertAndRoundToNearest(100.7, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 7");
        return LE_FAULT;
    }
    if (-2565656 != ConvertAndRoundToNearest(-2.5656563, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 8");
        return LE_FAULT;
    }
    if (-2565657 != ConvertAndRoundToNearest(-2.5656566, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 9");
        return LE_FAULT;
    }
    if (-100565657 != ConvertAndRoundToNearest(-100.5656566, SIX_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 10");
        return LE_FAULT;
    }
    if (-100566 != ConvertAndRoundToNearest(-100.5656566, THREE_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 11");
        return LE_FAULT;
    }
    if (-10007 != ConvertAndRoundToNearest(-100.06566, TWO_DECIMAL_PLACE_ACCURACY))
    {
        LE_INFO("step 12");
        return LE_FAULT;
    }
    if (1001 != ConvertAndRoundToNearest(100.06566, ONE_DECIMAL_PLACE_ACCURACY))
    {
        return LE_FAULT;
    }
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to release the PA gnss Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Release
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the GNSS constellation bit mask
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetConstellation
(
    le_gnss_ConstellationBitMask_t constellationMask  ///< [IN] GNSS constellation used in solution.
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the GNSS constellation bit mask
 *
* @return
*  - LE_OK on success
*  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetConstellation
(
    le_gnss_ConstellationBitMask_t *constellationMaskPtr ///< [OUT] GNSS constellation used in
                                                         ///< solution
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the area for the GNSS constellation
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 *  - LE_BAD_PARAMETER on invalid constellation area
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetConstellationArea
(
    le_gnss_Constellation_t satConstellation,       ///< [IN] GNSS constellation used in solution.
    le_gnss_ConstellationArea_t constellationArea   ///< [IN] GNSS constellation area.
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the area for the GNSS constellation
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetConstellationArea
(
    le_gnss_Constellation_t satConstellation,         ///< [IN] GNSS constellation used in solution.
    le_gnss_ConstellationArea_t* constellationAreaPtr ///< [OUT] GNSS constellation area.
)
{
   return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to start the gnss acquisition.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Start
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to stop the gnss acquisition.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Stop
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the GNSS device acquisition rate.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetAcquisitionRate
(
    uint32_t rate     ///< [IN] rate in milliseconds
)
{


    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the rate of GNSS fix reception
 *
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetAcquisitionRate
(
    uint32_t* ratePtr     ///< [IN] rate in milliseconds
)
{

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Report the position event
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_gnssSimu_ReportEvent
(
    void
)
{
    // Build the data for the user's event handler.
    pa_Gnss_Position_t* posDataPtr = le_mem_ForceAlloc(PositionEventDataPool);
    memcpy(posDataPtr, &GnssPositionData, sizeof(pa_Gnss_Position_t));
    le_event_ReportWithRefCounting(GnssEventId, posDataPtr);
    char* strDataPtr = le_mem_ForceAlloc(NmeaEventDataPool);
    strncpy(strDataPtr, "nmea", NMEA_STR_LEN);
    le_event_ReportWithRefCounting(NmeaEventId, strDataPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register an handler for gnss position data notifications.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_gnss_AddPositionDataHandler
(
    pa_gnss_PositionDataHandlerFunc_t handler ///< [IN] The handler function.
)
{
    LE_FATAL_IF((handler==NULL),"gnss module cannot set handler");

    le_event_HandlerRef_t newHandlerPtr = le_event_AddHandler(
                                                            "gpsInformationHandler",
                                                            GnssEventId,
                                                            (le_event_HandlerFunc_t) handler);

    return newHandlerPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to remove a handler for gnss position data notifications.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_gnss_RemovePositionDataHandler
(
    le_event_HandlerRef_t    handlerRef ///< [IN] The handler reference.
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to load an 'Extended Ephemeris' file into the GNSS device.
 *
 * @return LE_FAULT         The function failed to inject the 'Extended Ephemeris' file.
 * @return LE_TIMEOUT       A time-out occurred.
 * @return LE_FORMAT_ERROR  'Extended Ephemeris' file format error.
 * @return LE_OK            The function succeeded.
 *
 * @TODO    implementation
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_LoadExtendedEphemerisFile
(
    int32_t       fd      ///< [IN] extended ephemeris file descriptor
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the validity of the last injected Extended Ephemeris.
 *
 * @return LE_FAULT         The function failed to get the validity
 * @return LE_OK            The function succeeded.
 *
 * @TODO    implementation
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetExtendedEphemerisValidity
(
    uint64_t *startTimePtr,    ///< [OUT] Start time in seconds (since Jan. 1, 1970)
    uint64_t *stopTimePtr      ///< [OUT] Stop time in seconds (since Jan. 1, 1970)
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function enables the use of the 'Extended Ephemeris' file into the GNSS device.
 *
 * @return LE_FAULT         The function failed to enable the 'Extended Ephemeris' file.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_EnableExtendedEphemerisFile
(
    void
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function disables the use of the 'Extended Ephemeris' file into the GNSS device.
 *
 * @return LE_FAULT         The function failed to disable the 'Extended Ephemeris' file.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_DisableExtendedEphemerisFile
(
    void
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to inject UTC time into the GNSS device.
 *
 * @return
 *  - LE_OK            The function succeeded.
 *  - LE_FAULT         The function failed to inject the UTC time.
 *  - LE_TIMEOUT       A time-out occurred.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_InjectUtcTime
(
    uint64_t timeUtc,      ///< [IN] UTC time since Jan. 1, 1970 in milliseconds
    uint32_t timeUnc       ///< [IN] Time uncertainty in milliseconds
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to restart the GNSS device.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_ForceRestart
(
    pa_gnss_Restart_t  restartType ///< [IN] type of restart
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the TTFF in milliseconds.
 *
 * @return LE_BUSY          The position is not fixed and TTFF can't be measured.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetTtff
(
    uint32_t* ttffPtr     ///< [OUT] TTFF in milliseconds
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function disables the GNSS device.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Disable
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function enables the GNSS device.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_Enable
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the SUPL Assisted-GNSS mode.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetSuplAssistedMode
(
    le_gnss_AssistedMode_t  assistedMode      ///< [IN] Assisted-GNSS mode.
)
{
    return LE_FAULT;

}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the SUPL Assisted-GNSS mode.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetSuplAssistedMode
(
    le_gnss_AssistedMode_t *assistedModePtr      ///< [OUT] Assisted-GNSS mode.
)
{
    if (assistedModePtr == NULL)
    {
        return LE_FAULT;
    }

    // Get the SUPL assisted mode
    *assistedModePtr = SuplAssistedMode;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the SUPL server URL.
 * That server URL is a NULL-terminated string with a maximum string length (including NULL
 * terminator) equal to 256. Optionally the port number is specified after a colon.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetSuplServerUrl
(
    const char*  suplServerUrlPtr      ///< [IN] SUPL server URL.
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function injects the SUPL certificate to be used in A-GNSS sessions.
 *
 * @return
 *  - LE_OK on success
 *  - LE_BAD_PARAMETER on invalid parameter
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_InjectSuplCertificate
(
    uint8_t  suplCertificateId,      ///< [IN] ID of the SUPL certificate.
                                     ///< Certificate ID range is 0 to 9
    uint16_t suplCertificateLen,     ///< [IN] SUPL certificate size in Bytes.
    const char*  suplCertificatePtr  ///< [IN] SUPL certificate contents.
)
{
    // Check input parameters
    if (NULL == suplCertificatePtr)
    {
        LE_ERROR("NULL pointer");
        return LE_BAD_PARAMETER;
    }
    if (suplCertificateId > SUPL_CERTIFICATE_ID_LEN)
    {
        LE_ERROR("Invalid certificate ID %d", suplCertificateId);
        return LE_BAD_PARAMETER;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function deletes the SUPL certificate.
 *
 * @return
 *  - LE_OK on success
 *  - LE_BAD_PARAMETER on invalid parameter
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_DeleteSuplCertificate
(
    uint8_t  suplCertificateId  ///< [IN]  ID of the SUPL certificate.
                                ///< Certificate ID range is 0 to 9
)
{
    // Check input parameters
    if (suplCertificateId > SUPL_CERTIFICATE_ID_LEN)
    {
        LE_ERROR("Invalid certificate ID %d", suplCertificateId);
        return LE_BAD_PARAMETER;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the enabled NMEA sentences bit mask
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_BUSY service is busy
 *  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetNmeaSentences
(
    le_gnss_NmeaBitMask_t nmeaMask ///< [IN] Bit mask for enabled NMEA sentences.
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the enabled NMEA sentences bit mask
 *
* @return
*  - LE_OK on success
*  - LE_FAULT on failure
*  - LE_BUSY service is busy
*  - LE_TIMEOUT a time-out occurred
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetNmeaSentences
(
    le_gnss_NmeaBitMask_t* nmeaMaskPtr ///< [OUT] Bit mask for enabled NMEA sentences.
)
{
    if (NULL == nmeaMaskPtr)
    {
        LE_ERROR("NULL pointer");
        return LE_FAULT;
    }

    *nmeaMaskPtr = NmeaBitMask;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register an handler for NMEA frames notifications.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_gnss_AddNmeaHandler
(
    pa_gnss_NmeaHandlerFunc_t handler ///< [IN] The handler function.
)
{
    LE_FATAL_IF((handler==NULL),"gnss module cannot set handler");

    le_event_HandlerRef_t newHandlerPtr = le_event_AddHandler(
                                                            "gnssNmeaHandler",
                                                            NmeaEventId,
                                                            (le_event_HandlerFunc_t) handler);
    return newHandlerPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the GNSS minimum elevation.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_SetMinElevation
(
    uint8_t  minElevation      ///< [IN] Minimum elevation in degrees [range 0..90].
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *   Get the GNSS minimum elevation.
 *
* @return
*  - LE_OK on success
*  - LE_BAD_PARAMETER if minElevationPtr is NULL
*  - LE_FAULT on failure
*  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetMinElevation
(
   uint8_t*  minElevationPtr     ///< [OUT] Minimum elevation in degrees [range 0..90].
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get original DOP values received from platform adaptor.
 */
//--------------------------------------------------------------------------------------------------
void pa_gnssSimu_GetDOPValue
(
    le_gnss_DopType_t dopType,
    uint16_t* dop
)
{
    switch (dopType)
    {
        case LE_GNSS_PDOP:
            *dop = GnssPositionData.pdop;
            break;
        case LE_GNSS_HDOP:
            *dop = GnssPositionData.hdop;
            break;
        case LE_GNSS_VDOP:
            *dop = GnssPositionData.vdop;
            break;
        case LE_GNSS_GDOP:
            *dop = GnssPositionData.gdop;
            break;
        case LE_GNSS_TDOP:
            *dop = GnssPositionData.tdop;
            break;
        case LE_GNSS_DOP_LAST:
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Get original accuracy values received from platform adaptor.
 */
//--------------------------------------------------------------------------------------------------
void pa_gnssSimu_GetAccuracyValue
(
    int32_t* hSpeedUncertainty,
    int32_t* vSpeedUncertainty,
    int32_t* vUncertainty
)
{
    *hSpeedUncertainty = GnssPositionData.hSpeedUncertainty;
    *vSpeedUncertainty = GnssPositionData.vSpeedUncertainty;
    *vUncertainty = GnssPositionData.vUncertainty;
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert a location data parameter from/to multi-coordinate system
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_BAD_PARAMETER if locationDataDstPtr is NULL
 *  - LE_UNSUPPORTED request not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_ConvertDataCoordinateSystem
(
    le_gnss_CoordinateSystem_t coordinateSrc,     ///< [IN] Coordinate system to convert from.
    le_gnss_CoordinateSystem_t coordinateDst,     ///< [IN] Coordinate system to convert to.
    le_gnss_LocationDataType_t locationDataType,  ///< [IN] Type of location data to convert.
    int64_t locationDataSrc,                      ///< [IN] Data to convert.
    int64_t* locationDataDstPtr                   ///< [OUT] Converted Data.
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Component initialization function.
 *
 * This is not used because it will be called immediately at process start by the application
 * framework, and we want to wait until we can confirm that GNSS is available before starting
 * the platform adapter.
 *
 * See pa_gnss_Init().
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
}

