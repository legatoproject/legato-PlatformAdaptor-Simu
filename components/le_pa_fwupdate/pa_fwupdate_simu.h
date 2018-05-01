/** @file pa_fwupdate_simu.h
 *
 * Legato @ref pa_fwupdate_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_FWUPDATE_SIMU_H_INCLUDE_GUARD
#define PA_FWUPDATE_SIMU_H_INCLUDE_GUARD

#include "pa_fwupdate.h"

//--------------------------------------------------------------------------------------------------
/**
 * Defined version for FW
 */
//--------------------------------------------------------------------------------------------------
#define FW_VERSION_UT "Fw version UT"

//--------------------------------------------------------------------------------------------------
/**
 * Defined version for bootloader
 */
//--------------------------------------------------------------------------------------------------
#define BOOT_VERSION_UT "Boot version UT"

//--------------------------------------------------------------------------------------------------
/**
 * Defined version for app bootloader
 */
//--------------------------------------------------------------------------------------------------
#define APP_BOOT_VERSION_UT "App Boot version UT"

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub return code.
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetReturnCode
(
    le_result_t res     ///< [IN] Value for local sync test
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub reset request to false
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetResetState
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub nvup apply request to false
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetNvupApplyState
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub synchronization state
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetSyncState
(
    bool isSync     ///< [IN] Value for local sync test
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub init download request to false
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetInitDownloadState
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Simulate function to check if a reset was requested
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetResetState
(
    bool* isReset   ///< [OUT] indicate if a reset was requested
);

//--------------------------------------------------------------------------------------------------
/**
 * Simulate function to check if a nvup apply was requested
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetNvupApplyState
(
    bool* isNvupApply   ///< [OUT] indicate if a nvup apply was requested
);

//--------------------------------------------------------------------------------------------------
/**
 * Return the simulated SW update state
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetSwUpdateState
(
    pa_fwupdate_state_t* state  ///< [OUT] simulated SW update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Simulate function to check if an init download was requested
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetInitDownloadState
(
    bool* isInitDownload   ///< [OUT] indicate if an init download was requested
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the simulated SW update state
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetSwUpdateState
(
    pa_fwupdate_state_t state   ///< [IN] simulated SW update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the simulated resume position
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetResumePosition
(
    size_t position   ///< [IN] simulated resume position
);

//--------------------------------------------------------------------------------------------------
/**
 * Simulate a bad image report
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_ReportBadImage
(
    void
);
#endif // PA_FWUPDATE_SIMU_H_INCLUDE_GUARD

