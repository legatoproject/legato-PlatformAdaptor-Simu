/** @file pa_mrc_simu.h
 *
 * Legato @ref pa_mrc_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_MRC_SIMU_H_INCLUDE_GUARD
#define PA_MRC_SIMU_H_INCLUDE_GUARD

#include "pa_mrc.h"

#define PA_SIMU_MRC_DEFAULT_NAME    "Simu"
#define PA_SIMU_MRC_DEFAULT_RAT     "UMTS"
#define PA_SIMU_MRC_DEFAULT_MCC     "01"
#define PA_SIMU_MRC_DEFAULT_MNC     "001"

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for jamming detection activation
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    PA_MRCSIMU_JAMMING_UNSUPPORTED,     ///< Jamming is not supported
    PA_MRCSIMU_JAMMING_ACTIVATED,       ///< Jamming is activated
    PA_MRCSIMU_JAMMING_DEACTIVATED      ///< Jamming is deactivated
}
pa_mrcSimu_JammingDetection_t;

//--------------------------------------------------------------------------------------------------
/**
 * This function set the current Radio Access Technology in use.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrcSimu_SetRadioAccessTechInUse
(
    le_mrc_Rat_t   rat  ///< [IN] The Radio Access Technology.
);

le_result_t mrc_simu_Init
(
    void
);

bool mrc_simu_IsOnline
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * Set the jamming detection activation status
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrcSimu_SetJammingDetection
(
    pa_mrcSimu_JammingDetection_t activation    ///< [IN] Jamming activation state
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the jamming detection activation status
 *
 */
//--------------------------------------------------------------------------------------------------
pa_mrcSimu_JammingDetection_t pa_mrcSimu_GetJammingDetection
(
     void
);

//--------------------------------------------------------------------------------------------------
/**
 * Report jamming detection event.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrcSimu_ReportJammingDetection
(
    le_mrc_JammingReport_t  report,     ///< Notification type
    le_mrc_JammingStatus_t  status      ///< Jamming status
);

#endif // PA_MRC_SIMU_H_INCLUDE_GUARD

