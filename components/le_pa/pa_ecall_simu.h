/** @file pa_ecall_simu.h
 *
 * Legato @ref pa_ecall_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_ECALL_SIMU_H_INCLUDE_GUARD
#define PA_ECALL_SIMU_H_INCLUDE_GUARD

#include "pa_ecall.h"

#define PA_SIMU_ECALL_DEFAULT_PSAP                  "+4953135409300"
#define PA_SIMU_ECALL_DEFAULT_MAX_REDIAL_ATTEMPTS   3
#define PA_SIMU_ECALL_DEFAULT_MSD_TX_MODE           LE_ECALL_TX_MODE_PUSH

//--------------------------------------------------------------------------------------------------
/**
 * Report the eCall state
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_ecallSimu_ReportEcallState
(
    le_ecall_State_t  state
);

//--------------------------------------------------------------------------------------------------
/**
 * simu init
 *
 **/
//--------------------------------------------------------------------------------------------------
le_result_t ecall_simu_Init
(
    void
);


#endif // PA_ECALL_SIMU_H_INCLUDE_GUARD

