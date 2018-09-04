/**
 * @file pa_gnss_simu.h
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef LEGATO_PA_GNSS_SIMU_INCLUDE_GUARD
#define LEGATO_PA_GNSS_SIMU_INCLUDE_GUARD

//--------------------------------------------------------------------------------------------------
/**
 * Position event report.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_gnssSimu_ReportEvent
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * GNSS position handler data init.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_gnssSimu_SetGnssValidPositionData
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get original DOP values received from platform adaptor.
 */
//--------------------------------------------------------------------------------------------------
void pa_gnssSimu_GetDOPValue
(
    le_gnss_DopType_t dopType,
    uint16_t* dop
);

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
);
//--------------------------------------------------------------------------------------------------
/**
 * This function gets leap seconds information
 *
 * @return
 *  - LE_OK           The function successed
 *  - LE_FAULT        The function failed
 *  - LE_TIMEOUT      The function timeout
 *  - LE_UNSUPPORTED  Not supported on this platform
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_gnss_GetLeapSeconds
(
    uint64_t* gpsTimePtr,              ///< [OUT] The number of milliseconds of GPS time since
                                       ///<       Jan. 6, 1980
    int32_t* currentLeapSecondsPtr,    ///< [OUT] Current UTC leap seconds value in milliseconds
    uint64_t* changeEventTimePtr,      ///< [OUT] The number of milliseconds since Jan. 6, 1980
                                       ///<       to the next leap seconds change event
    int32_t* nextLeapSecondsPtr        ///< [OUT] UTC leap seconds value to be applied at the
                                       ///<       change event time in milliseconds
);
#endif
