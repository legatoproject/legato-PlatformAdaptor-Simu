/**
 * @file pa_ecall_simu.c
 *
 * Simulation implementation of @ref c_pa_ecall API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_ecall_simu.h"

#define MSD_BLOB_SIZE 140

//--------------------------------------------------------------------------------------------------
/**
 * Call event ID used to report ecall events to the registered event handlers.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t ECallEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Default simulation values.
 */
//--------------------------------------------------------------------------------------------------
static char                 PsapNumber[LE_MDMDEFS_PHONE_NUM_MAX_LEN] = PA_SIMU_ECALL_DEFAULT_PSAP;
static le_ecall_MsdTxMode_t MsdTxMode = PA_SIMU_ECALL_DEFAULT_MSD_TX_MODE;
static uint8_t              MsdData[MSD_BLOB_SIZE];
static uint16_t             NadDeregistrationTime = 120;
static uint16_t             FallbackTime = 60;
static uint16_t             AutoAnswerTime = 20;
static uint16_t             MSDMaxTransTime = 60;
static uint16_t             PostTestRegistrationTime = 120;
static le_ecall_OpMode_t    OperationMode = LE_ECALL_NORMAL_MODE;

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the ecall module
 *
 * @return LE_NOT_POSSIBLE  The function failed to initialize the module.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t ecall_simu_Init
(
    void
)
{
    // Create the event for signaling user handlers.
    ECallEventId = le_event_CreateId("ECallEvent",sizeof(le_ecall_State_t));

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function initializes the platform adapter layer for eCall services.
 *
 * @return LE_OK if successful.
 * @return LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_Init
(
    pa_ecall_SysStd_t sysStd ///< [IN] Choosen system (PA_ECALL_PAN_EUROPEAN or
)                            ///< PA_ECALL_ERA_GLONASS)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Report the eCall state
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_ecallSimu_ReportEcallState
(
    le_ecall_State_t  state
)
{
    le_ecall_State_t newECallState = state;
    LE_INFO("Report eCall state.%d", newECallState);
    le_event_Report(ECallEventId, &newECallState, sizeof(newECallState));
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for eCall event notifications.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_ecall_AddEventHandler
(
    pa_ecall_EventHandlerFunc_t   handlerFuncPtr ///< [IN] The event handler function
)
{
    LE_INFO("Add new eCall Event handler.");

    LE_FATAL_IF(handlerFuncPtr == NULL, "The new eCall Event handler is NULL.");

    return le_event_AddHandler("ECallEventHandler",
                               ECallEventId,
                               (le_event_HandlerFunc_t)handlerFuncPtr);

}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for eCalls handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_ecall_RemoveEventHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    LE_INFO("Remove eCall Event handler %p",handlerRef);
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Public Safely Answering Point number.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OVERFLOW  psap number is too long.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetPsapNumber
(
    char psap[LE_MDMDEFS_PHONE_NUM_MAX_LEN] ///< [IN] Public Safely Answering Point number
)
{
    return le_utf8_Copy(PsapNumber, psap, LE_MDMDEFS_PHONE_NUM_MAX_LEN, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Public Safely Answering Point number.
 *
 * @return LE_FAULT     The function failed.
 * @return LE_OVERFLOW  Retrieved PSAP number is too long for the out parameter.
 * @return LE_OK        The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetPsapNumber
(
    char*    psapPtr, ///< [OUT] Public Safely Answering Point number
    size_t   len      ///< [IN] The length of SMSC string
)
{
    return le_utf8_Copy(psapPtr, PsapNumber, len, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set push/pull transmission mode.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetMsdTxMode
(
    le_ecall_MsdTxMode_t mode
)
{
    MsdTxMode = mode;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get push/pull transmission mode.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetMsdTxMode
(
    le_ecall_MsdTxMode_t* modePtr
)
{
    *modePtr = MsdTxMode;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send the Minimum Set of Data for the eCall.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SendMsd
(
    uint8_t  *msdPtr,   ///< [IN] Encoded Msd
    size_t    msdSize   ///< [IN] msd buffer size
)
{
    if(msdSize > sizeof(MsdData))
    {
        LE_ERROR("MSD data is too big (= %zd, max %zd)", msdSize, sizeof(MsdData));
        return LE_FAULT;
    }

    memcpy(MsdData, msdPtr, msdSize);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to start the eCall.
 * @return
 *      LE_OK if successful.
 *      LE_FAULT if unsuccessful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_Start
(
    pa_ecall_StartType_t callType
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to end a eCall.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_End
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the eCall operation mode.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetOperationMode
(
    le_ecall_OpMode_t mode ///< [IN] Operation mode
)
{
    OperationMode = mode;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to retrieve the configured eCall operation mode.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetOperationMode
(
    le_ecall_OpMode_t* modePtr ///< [OUT] Operation mode
)
{
    *modePtr = OperationMode;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function can be recalled to indicate the modem to read the number to dial from the FDN/SDN
 * of the U/SIM, depending upon the eCall operating mode.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_UseUSimNumbers
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'NAD Deregistration Time' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetNadDeregistrationTime
(
    uint16_t    deregTime  ///< [IN] the 'NAD Deregistration Time' value in minutes
)
{
    NadDeregistrationTime = deregTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'NAD Deregistration Time' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetNadDeregistrationTime
(
    uint16_t*    deregTimePtr  ///< [OUT] the 'NAD Deregistration Time' value in minutes
)
{
    *deregTimePtr = NadDeregistrationTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'ECALL_CCFT' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassFallbackTime
(
    uint16_t    duration  ///< [IN] the ECALL_CCFT time value in minutes
)
{
    FallbackTime = duration;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'ECALL_CCFT' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassFallbackTime
(
    uint16_t*    durationPtr  ///< [OUT] the ECALL_CCFT time value in minutes
)
{
    *durationPtr = FallbackTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'ECALL_AUTO_ANSWER_TIME' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassAutoAnswerTime
(
    uint16_t autoAnswerTime  ///< [IN] the ECALL_AUTO_ANSWER_TIME time value in minutes
)
{
    AutoAnswerTime = autoAnswerTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'ECALL_AUTO_ANSWER_TIME' value in minutes.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassAutoAnswerTime
(
    uint16_t* autoAnswerTimePtr  ///< [OUT] the ECALL_AUTO_ANSWER_TIME time value in minutes
)
{
    if (NULL == autoAnswerTimePtr)
    {
        LE_ERROR("autoAnswerTimePtr is NULL!");
        return LE_FAULT;
    }

    *autoAnswerTimePtr = AutoAnswerTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the ECALL_MSD_MAX_TRANSMISSION_TIME time. It is a time period for MSD transmission.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassMSDMaxTransmissionTime
(
    uint16_t msdMaxTransTime ///< [IN] ECALL_MSD_MAX_TRANSMISSION_TIME time value (in seconds)
)
{
    MSDMaxTransTime = msdMaxTransTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the 'ECALL_POST_TEST_REGISTRATION_TIME' time value in seconds.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_SetEraGlonassPostTestRegistrationTime
(
    uint16_t postTestRegTime  ///< [IN] ECALL_POST_TEST_REGISTRATION_TIME time value (in seconds)
)
{
    PostTestRegistrationTime = postTestRegTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the ECALL_MSD_MAX_TRANSMISSION_TIME time.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassMSDMaxTransmissionTime
(
    uint16_t* msdMaxTransTimePtr ///< [OUT] ECALL_MSD_MAX_TRANSMISSION_TIME time value (in seconds)
)
{
    if (NULL == msdMaxTransTimePtr)
    {
        LE_ERROR("msdMaxTransTimePtr is NULL!");
        return LE_FAULT;
    }

    *msdMaxTransTimePtr = MSDMaxTransTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the 'ECALL_POST_TEST_REGISTRATION_TIME' time value in seconds.
 *
 * @return
 *  - LE_OK on success
 *  - LE_FAULT on failure
 *  - LE_UNSUPPORTED if the function is not supported by the target
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_GetEraGlonassPostTestRegistrationTime
(
    uint16_t* postTestRegTimePtr  ///< [OUT] ECALL_POST_TEST_REGISTRATION_TIME time value
                                  ///< (in seconds)
)
{
    if (NULL == postTestRegTimePtr)
    {
        LE_ERROR("postTestRegTimePtr is NULL!");
        return LE_FAULT;
    }

    *postTestRegTimePtr = PostTestRegistrationTime;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update current system standard
 *
 * @return
 *  - LE_OK    on success
 *  - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_ecall_UpdateSystemStandard
(
    pa_ecall_SysStd_t sysStandard  ///< [IN] The system standard
)
{
    return LE_OK;
}