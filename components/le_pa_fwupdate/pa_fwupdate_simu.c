//--------------------------------------------------------------------------------------------------
/*
 * PA fwupdate for Fw Update Unitary test
 */
//--------------------------------------------------------------------------------------------------

#include "legato.h"
#include "pa_fwupdate_simu.h"
#include "pa_fwupdate.h"

//--------------------------------------------------------------------------------------------------
/**
 * Static variable for simulating PA API error code
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReturnCode = LE_OK;

//--------------------------------------------------------------------------------------------------
/**
 * Static variable for simulating systems synchronization state
 */
//--------------------------------------------------------------------------------------------------
static bool IsSyncLocal = true;

//--------------------------------------------------------------------------------------------------
/**
 * Static variable for simulating a reset request
 */
//--------------------------------------------------------------------------------------------------
static bool IsResetRequested = false;

//--------------------------------------------------------------------------------------------------
/**
 * Static variable for simulating Sw update state
 */
//--------------------------------------------------------------------------------------------------
static pa_fwupdate_state_t SwUpdateState = PA_FWUPDATE_STATE_INVALID;

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub return code.
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetReturnCode
(
    le_result_t res
)
{
    ReturnCode = res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub synchronization state
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetSyncState
(
    bool isSync     ///< [IN] Value for local sync test
)
{
    IsSyncLocal = isSync;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub reset request to false
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetResetState
(
    void
)
{
    IsResetRequested = false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Simulate function to check if a reset was requested
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetResetState
(
    bool* isReset   ///< [OUT] indicate if a reset was requested
)
{
    *isReset = IsResetRequested;
}

//--------------------------------------------------------------------------------------------------
/**
 * Return the simulated SW update state
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetSwUpdateState
(
    pa_fwupdate_state_t* state  ///< [OUT] simulated SW update state
)
{
    *state = SwUpdateState;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the simulated SW update state
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_setSwUpdateState
(
    pa_fwupdate_state_t state   ///< [IN] simulated SW update state
)
{
    SwUpdateState = state;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function starts a package download to the device.
 *
 * @warning This API is a blocking API. It needs to be called in a dedicated thread.
 *
 * @return
 *      - LE_OK            The function succeeded
 *      - LE_BAD_PARAMETER The parameter is invalid (needs to be positive)
 *      - LE_FAULT         The function failed
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_Download
(
    int fd  ///< [IN]File descriptor of the file to be downloaded
)
{
    if (fd < 0)
    {
        LE_ERROR ("pa_fwupdate_Download bad parameter");
        return LE_BAD_PARAMETER;
    }

    if (IsSyncLocal == false)
    {
        return LE_NOT_POSSIBLE;
    }

    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetFirmwareVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    if (ReturnCode == LE_OK )
    {
        /* Simulate a correct API behavior */
        if (versionSize > strlen (BOOT_VERSION_UT))
        {
            memset (versionPtr, 0, versionSize);
            strncpy (versionPtr, FW_VERSION_UT, strlen (FW_VERSION_UT));
        }
        else
        {
            return LE_OVERFLOW;
        }
    }
    return ReturnCode;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the bootloader version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
        if (ReturnCode == LE_OK )
    {
        /* Simulate a correct API behavior */
        if (versionSize > strlen (FW_VERSION_UT))
        {
            memset (versionPtr, 0, versionSize);
            strncpy (versionPtr, BOOT_VERSION_UT, strlen (BOOT_VERSION_UT));
        }
        else
        {
            return LE_OVERFLOW;
        }
    }
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Program a swap between active and update systems
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_DualSysSwap
(
    bool isSyncReq      ///< [IN] Indicate if a synchronization is requested after the swap
)
{
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Program a synchronization between active and update systems
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_DualSysSync
(
    void
)
{
    if (ReturnCode == LE_OK)
    {
        pa_fwupdate_SetState(PA_FWUPDATE_STATE_SYNC);
        IsSyncLocal = true;
    }
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Indicates if active and update systems are synchronized
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_DualSysGetSyncState(
    bool *isSync
)
{
    *isSync = IsSyncLocal;
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update some variables in SSDATA to indicate that systems are not synchronized
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_DualSysSetUnsyncState
(
    void
)
{
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which issue a system reset
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdate_Reset
(
    void
)
{
    IsResetRequested = true;
    LE_INFO ("Device reboots");
}

//--------------------------------------------------------------------------------------------------
/**
 * This API is to be called to set the SW update state in SSDATA
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_SetState
(
    pa_fwupdate_state_t state   ///< [IN] state to set
)
{
    if (state >= PA_FWUPDATE_STATE_INVALID)
    {
        return LE_BAD_PARAMETER;
    }

    SwUpdateState = state;
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which indicates if a Sync operation is needed (swap & sync operation)
 *
 * @return
 *      - LE_OK            on success
 *      - LE_UNSUPPORTED   the feature is not supported
 *      - LE_FAULT         else
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_DualSysCheckSync
(
    bool *isSyncReq ///< Indicates if synchronization is requested
)
{
    if (ReturnCode == LE_OK)
    {
        *isSyncReq = IsSyncLocal;
    }
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Component initializer automatically called by the application framework when the process starts.
 *
 **/
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
}

