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

#endif
