/** @file pa_rtc_simu.h
 *
 * Legato @ref pa_rtc_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_RTC_SIMU_H_INCLUDE_GUARD
#define PA_RTC_SIMU_H_INCLUDE_GUARD

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub return code.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_rtcSimu_SetReturnCode
(
    le_result_t res
);

//--------------------------------------------------------------------------------------------------
/**
 * Check the current time.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_rtcSimu_CheckTime
(
    uint64_t  milliseconds
);




#endif // PA_RTC_SIMU_H_INCLUDE_GUARD

