//--------------------------------------------------------------------------------------------------
/*
 * PA fwupdate for Fw Update Unitary test
 */
//--------------------------------------------------------------------------------------------------

#include "legato.h"
#include "pa_fwupdate.h"
#include "pa_fwupdate_simu.h"

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
 * Static variable for simulating a NVUP apply request
 */
//--------------------------------------------------------------------------------------------------
static bool IsNvupApplyRequested = false;

//--------------------------------------------------------------------------------------------------
/**
 * Static variable for simulating an init download request
 */
//--------------------------------------------------------------------------------------------------
static bool IsInitDownloadRequested = false;

//--------------------------------------------------------------------------------------------------
/**
 * Static variable for simulating Sw update state
 */
//--------------------------------------------------------------------------------------------------
static pa_fwupdate_state_t SwUpdateState = PA_FWUPDATE_STATE_INVALID;

//--------------------------------------------------------------------------------------------------
/**
 * Static variable for simulating resume position
 */
//--------------------------------------------------------------------------------------------------
static size_t ResumePosition = 0;

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
 * Set the stub nvup apply request to false
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetNvupApplyState
(
    void
)
{
    IsNvupApplyRequested = false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub init download request to false
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetInitDownloadState
(
    void
)
{
    IsInitDownloadRequested = false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Simulate function to check if a reset was requested
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetResetState
(
    bool* isResetPtr   ///< [OUT] indicate if a reset was requested
)
{
    if (isResetPtr)
    {
        *isResetPtr = IsResetRequested;
    }
    else
    {
        LE_CRIT("isResetPtr is NULL");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Simulate function to check if a nvup apply was requested
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetNvupApplyState
(
    bool* isNvupApplyPtr   ///< [OUT] indicate if a nvup apply was requested
)
{
    if (isNvupApplyPtr)
    {
        *isNvupApplyPtr = IsNvupApplyRequested;
    }
    else
    {
        LE_CRIT("isNvupApplyPtr is NULL");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Return the simulated SW update state
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetSwUpdateState
(
    pa_fwupdate_state_t* statePtr  ///< [OUT] simulated SW update state
)
{
    if (statePtr)
    {
        *statePtr = SwUpdateState;
    }
    else
    {
        LE_CRIT("statePtr is NULL");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Simulate function to check if an init download was requested
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_GetInitDownloadState
(
    bool* isInitDownloadPtr   ///< [OUT] indicate if an init download was requested
)
{
    if (isInitDownloadPtr)
    {
        *isInitDownloadPtr = IsInitDownloadRequested;
    }
    else
    {
        LE_CRIT("isInitDownloadPtr is NULL");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the simulated SW update state
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetSwUpdateState
(
    pa_fwupdate_state_t state   ///< [IN] simulated SW update state
)
{
    SwUpdateState = state;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the simulated resume position
 */
//--------------------------------------------------------------------------------------------------
void pa_fwupdateSimu_SetResumePosition
(
    size_t position   ///< [IN] simulated resume position
)
{
    ResumePosition = position;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function starts a package download to the device.
 *
 * @warning This API is a blocking API. It needs to be called in a dedicated thread.
 *
 * @return
 *      - LE_OK              On success
 *      - LE_BAD_PARAMETER   If an input parameter is not valid
 *      - LE_TIMEOUT         After 900 seconds without data received
 *      - LE_NOT_POSSIBLE    The systems are not synced
 *      - LE_FAULT           On failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_Download
(
    int fd  ///< [IN]File descriptor of the file to be downloaded
)
{
    if (fd < 0)
    {
        LE_ERROR ("bad parameter");
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
 * Return the update package write position.
 *
 * @note This is actually the position within the update package, not the one once the update
 * package is processed (unzipping, extracting, ... ).
 *
 * @return
 *      - LE_OK            on success
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT         on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetResumePosition
(
    size_t *positionPtr
)
{
    // Check the parameter
    if (NULL == positionPtr)
    {
        LE_ERROR("Invalid parameter.");
        return LE_BAD_PARAMETER;
    }

    if (ReturnCode == LE_OK)
    {
        *positionPtr = ResumePosition;
    }

    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware update status label
 *
 * @return
 *      - The address of the FW update status description matching the given status.
 *      - NULL if the given status is invalid.
 */
//--------------------------------------------------------------------------------------------------
const char *pa_fwupdate_GetUpdateStatusLabel
(
    pa_fwupdate_UpdateStatus_t status    ///< [IN] Firmware update status
)
{
    const char *FwUpdateStatusLabel[] =
    {
        "Unknown status"                    // PA_FWUPDATE_UPDATE_STATUS_UNKNOWN
    };

    // Check parameters
    if (status > PA_FWUPDATE_UPDATE_STATUS_UNKNOWN)
    {
        LE_ERROR("Invalid status parameter (%d)!", status);
        return NULL;
    }

    // Point to the default known status label.
    return FwUpdateStatusLabel[0];
}

//--------------------------------------------------------------------------------------------------
/**
 * Return the last update status.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER Invalid parameter
 *      - LE_FAULT on failure
 *      - LE_UNSUPPORTED not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetUpdateStatus
(
    pa_fwupdate_UpdateStatus_t *statusPtr, ///< [OUT] Returned update status
    char *statusLabelPtr,                  ///< [OUT] String matching the status
    size_t statusLabelLength               ///< [IN] Maximum length of the status description
)
{
    // Check the parameter
    if (NULL == statusPtr)
    {
        LE_ERROR("Invalid parameter.");
        return LE_BAD_PARAMETER;
    }

    // Assuming that everything is fine
    *statusPtr = PA_FWUPDATE_UPDATE_STATUS_OK;
    if (NULL != statusLabelPtr)
    {
        if (statusLabelLength > 0)
        {
            // Update the status label
            strncpy(statusLabelPtr,
                pa_fwupdate_GetUpdateStatusLabel(*statusPtr),
                statusLabelLength);
        }
        else
        {
            // At least, reset the label
            *statusLabelPtr = '\0';
        }
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware version string
 *
 * @return
 *      - LE_OK            on success
 *      - LE_NOT_FOUND     if the version string is not available
 *      - LE_OVERFLOW      if version string to big to fit in provided buffer
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetFirmwareVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    if (!versionPtr)
    {
        return LE_BAD_PARAMETER;
    }
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
 *      - LE_OK            on success
 *      - LE_NOT_FOUND     if the version string is not available
 *      - LE_OVERFLOW      if version string to big to fit in provided buffer
 *      - LE_BAD_PARAMETER bad parameter
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_GetBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    if (!versionPtr)
    {
        return LE_BAD_PARAMETER;
    }
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
 * Install the firmware package. On dual system this api performs a swap between active and update
 * systems.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_Install
(
    bool isSyncReq      ///< [IN] Indicate if a synchronization is requested after the swap
)
{
    if (ReturnCode == LE_OK)
    {
        if (isSyncReq)
        {
            pa_fwupdate_MarkGood();
        }
        pa_fwupdate_Reset();
        pa_fwupdate_NvupApply();
    }
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Mark the system as good.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_MarkGood
(
    void
)
{
    if (ReturnCode == LE_OK)
    {
        pa_fwupdate_SetState(PA_FWUPDATE_STATE_SYNC);
        IsSyncLocal = true;
    }
    else
    {
        IsSyncLocal = false;
        pa_fwupdate_SetState(PA_FWUPDATE_STATE_NORMAL);
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
le_result_t pa_fwupdate_GetSystemState
(
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
 * request the modem to apply the NVUP files in UD system
 *
 * @return
 *      - LE_OK             on success
 *      - LE_UNSUPPORTED    the feature is not supported
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_NvupApply
(
    void
)
{
    IsNvupApplyRequested = true;
    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the resume context
 *
 * @return
 *      - LE_OK             on success
 *      - LE_UNSUPPORTED    the feature is not supported
 *      - LE_FAULT          on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_fwupdate_InitDownload
(
    void
)
{
    if (ReturnCode == LE_OK)
    {
        IsInitDownloadRequested = true;
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

