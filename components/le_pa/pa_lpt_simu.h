/** @file pa_lpt_simu.h
 *
 * Legato @ref pa_lpt_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_LPT_SIMU_H_INCLUDE_GUARD
#define PA_LPT_SIMU_H_INCLUDE_GUARD

#include "pa_lpt.h"


//--------------------------------------------------------------------------------------------------
/**
 * LPT simulated PA initialization.
 *
 * @return
 *  - LE_OK            The function succeeded.
 *  - LE_FAULT         The function failed to initialize the module.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lptSimu_Init
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Report a change in the eDRX parameters.
 */
//--------------------------------------------------------------------------------------------------
void pa_lptSimu_ReportEDrxParamsChange
(
    le_lpt_EDrxRat_t rat,       ///< [IN] Radio Access Technology.
    le_onoff_t activation,      ///< [IN] eDRX activation state.
    uint8_t eDrxValue,          ///< [IN] eDRX cycle value, defined in 3GPP
                                ///<      TS 24.008 Rel-13 section 10.5.5.32.
    uint8_t pagingTimeWindow    ///< [IN] Paging Time Window, defined in 3GPP
                                ///<      TS 24.008 Rel-13 section 10.5.5.32.
);
#endif // PA_LPT_SIMU_H_INCLUDE_GUARD
