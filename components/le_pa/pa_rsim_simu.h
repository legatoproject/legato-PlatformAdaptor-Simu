/**
 * @file pa_rsim_simu.h
 *
 * Legato @ref pa_rsim_simu include file.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 */

#ifndef PA_RSIM_SIMU_H_INCLUDE_GUARD
#define PA_RSIM_SIMU_H_INCLUDE_GUARD

//--------------------------------------------------------------------------------------------------
/**
 * Send a SIM action request to the remote SIM service
 */
//--------------------------------------------------------------------------------------------------
void pa_rsimSimu_SendSimActionRequest
(
    pa_rsim_Action_t action
);

//--------------------------------------------------------------------------------------------------
/**
 * Send an APDU indication to the remote SIM service
 */
//--------------------------------------------------------------------------------------------------
void pa_rsimSimu_SendApduInd
(
    const uint8_t* apduPtr,
    size_t apduLen
);

#endif // PA_RSIM_SIMU_H_INCLUDE_GUARD
