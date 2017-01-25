/** @file pa_riPin_simu.h
 *
 * Legato @ref pa_riPin_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_RIPIN_SIMU_H_INCLUDE_GUARD
#define PA_RIPIN_SIMU_H_INCLUDE_GUARD

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub return code.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_riPinSimu_SetReturnCode
(
    le_result_t res
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the "AmIOwner" flag
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_riPinSimu_SetAmIOwnerOfRingSignal
(
    bool amIOwner
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the "AmIOwner" value
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_riPinSimu_CheckAmIOwnerOfRingSignal
(
    bool amIOwner
);

//--------------------------------------------------------------------------------------------------
/**
 * Get RI signal value
 *
 */
//--------------------------------------------------------------------------------------------------
uint8_t pa_riPinSimu_Get
(
       void
);

#endif // PA_RIPIN_SIMU_H_INCLUDE_GUARD
