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
#endif
