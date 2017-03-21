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


#endif // PA_MRC_SIMU_H_INCLUDE_GUARD

