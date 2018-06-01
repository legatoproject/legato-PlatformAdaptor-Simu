/**
 * @file pa_sim_simu.c
 *
 * Simulation implementation of @ref c_pa_sim API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_simu.h"
#include "pa_sim_simu.h"
#include "simuConfig.h"

//--------------------------------------------------------------------------------------------------
/**
 * Declaration of constants.
 */
//--------------------------------------------------------------------------------------------------
#define MAX_FPLMN_OPERATOR 5

//--------------------------------------------------------------------------------------------------
/**
 * Declaration of variables used by the simulation engine.
 */
//--------------------------------------------------------------------------------------------------
static uint32_t PinRemainingAttempts = PA_SIMU_SIM_DEFAULT_PIN_REMAINING_ATTEMPTS;
static uint32_t PukRemainingAttempts = PA_SIMU_SIM_DEFAULT_PUK_REMAINING_ATTEMPTS;
static le_sim_Id_t SelectedCard = 1;
static le_sim_States_t SimState = LE_SIM_READY;
static char HomeMcc[LE_MRC_MCC_BYTES] = PA_SIMU_SIM_DEFAULT_MCC;
static char HomeMnc[LE_MRC_MNC_BYTES] = PA_SIMU_SIM_DEFAULT_MNC;
static pa_sim_Imsi_t Imsi = PA_SIMU_SIM_DEFAULT_IMSI;
static pa_sim_CardId_t Iccid = PA_SIMU_SIM_DEFAULT_ICCID;
static pa_sim_Eid_t Eid = PA_SIMU_SIM_DEFAULT_EID;
static char PhoneNumber[LE_MDMDEFS_PHONE_NUM_MAX_BYTES] = PA_SIMU_SIM_DEFAULT_PHONE_NUMBER;
static char* HomeNetworkOperator = PA_SIMU_SIM_DEFAULT_HOME_NETWORK;
static char Pin[PA_SIM_PIN_MAX_LEN+1] = PA_SIMU_SIM_DEFAULT_PIN;
static bool IsPinSecurityEnabled = true;
static char Puk[PA_SIM_PUK_MAX_LEN+1] = PA_SIMU_SIM_DEFAULT_PUK;
static bool STKConfirmation = false;
static le_event_Id_t SimToolkitEvent;
static pa_sim_NewStateHdlrFunc_t SimStateHandler;
static le_mem_PoolRef_t SimStateEventPool;
static bool SimAccessTest = false;
static pa_sim_MobileCode_t FPLMNOperator[MAX_FPLMN_OPERATOR];
static le_sim_StkEvent_t StkEvent = LE_SIM_STK_EVENT_MAX;
static le_sim_StkRefreshMode_t StkRefreshMode = LE_SIM_REFRESH_MODE_MAX;
static le_sim_StkRefreshStage_t StkRefreshStage = LE_SIM_STAGE_MAX;
static le_sem_Ref_t SyncSemaphore = NULL;
static le_onoff_t SimPower;

//--------------------------------------------------------------------------------------------------
/**
 * Set the SIM state using a string as a parameter.
 */
//--------------------------------------------------------------------------------------------------
static void SetStateFromString
(
    const char* stateStr    ///< [IN] SIM state as a string
);

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the International Mobile Subscriber Identity (IMSI) from a string.
 */
//--------------------------------------------------------------------------------------------------
static void SetIMSIFromString
(
    const char* imsiStr     ///< [IN] IMSI value
);

//--------------------------------------------------------------------------------------------------
/**
 * Definition of settings that are settable through simuConfig.
 */
//--------------------------------------------------------------------------------------------------
static const simuConfig_Property_t ConfigProperties[] = {
    { .name = "state",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = SetStateFromString } } },
    { .name = "mcc",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetHomeNetworkMcc } } },
    { .name = "mnc",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetHomeNetworkMnc } } },
    { .name = "imsi",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = SetIMSIFromString } } },
    { .name = "iccid",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetCardIdentification } } },
    { .name = "eid",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetEID } } },
    { .name = "phoneNumber",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetSubscriberPhoneNumber } } },
    { .name = "operator",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetHomeNetworkOperator } } },
    { .name = "pin",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetPIN } } },
    { .name = "pinSecurity",
      .setter = { .type = SIMUCONFIG_HANDLER_BOOL,
                  .handler = { .boolFn = pa_simSimu_SetPINSecurity } } },
    { .name = "puk",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_simSimu_SetPUK } } },
    {0}
};

//--------------------------------------------------------------------------------------------------
/**
 * Services available for configuration.
 */
//--------------------------------------------------------------------------------------------------
static const simuConfig_Service_t ConfigService = {
    "sim",
    PA_SIMU_CFG_MODEM_ROOT "/sim",
    ConfigProperties
};

//--------------------------------------------------------------------------------------------------
/**
 * Powers up or down the current SIM card.
 *
 * @return
 *      - LE_OK           On success
 *      - LE_FAULT        For unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SetPower
(
    le_onoff_t power     ///< [IN] The power state.
)
{
    if (LE_ON == power)
    {
        SimPower = LE_ON;
        return LE_OK;
    }
    else if (LE_OFF == power)
    {
        SimPower = LE_OFF;
        return LE_OK;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the PUK code.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetPUK
(
    const char* puk
)
{
    le_utf8_Copy(Puk, puk, NUM_ARRAY_MEMBERS(Puk), NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the PIN code.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetPIN
(
    const char* pin
)
{
    le_utf8_Copy(Pin, pin, NUM_ARRAY_MEMBERS(Pin), NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Enable/disable the PIN code.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetPINSecurity
(
    bool enable ///< [IN] Should the PIN code be used or not.
)
{
    IsPinSecurityEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
/**
 * Select the SIM card currently in use.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetSelectCard
(
    le_sim_Id_t simId     ///< [IN] The SIM currently selected
)
{
    SelectedCard = simId;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function selects the Card on which all further SIM operations have to be operated.
 *
 * @return
 * - LE_OK            The function succeeded.
 * - LE_FAULT         on failure.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SelectCard
(
    le_sim_Id_t  sim     ///< [IN] The SIM to be selected
)
{
    LE_ASSERT(sim == SelectedCard);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the card on which operations are operated.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetSelectedCard
(
    le_sim_Id_t*  simIdPtr     ///< [OUT] The SIM identifier selected.
)
{
    *simIdPtr = SelectedCard;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Report the SIM state.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_ReportSIMState
(
    le_sim_States_t state
)
{
    SimState = state;

    LE_DEBUG("Report SIM state %d", state);

    if (SimStateHandler)
    {
        pa_sim_Event_t* eventPtr = le_mem_ForceAlloc(SimStateEventPool);
        eventPtr->simId = SelectedCard;
        eventPtr->state = SimState;

        SimStateHandler(eventPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the SIM state using a string as a parameter.
 *
 * Valid SIM states values are:
 * - INSERTED
 * - ABSENT
 * - READY
 * - BLOCKED
 * - BUSY
 */
//--------------------------------------------------------------------------------------------------
static void SetStateFromString
(
    const char* stateStr    ///< [IN] SIM state as a string
)
{
    le_sim_States_t state;

    if (0 == strcmp(stateStr, "INSERTED"))
    {
        state = LE_SIM_INSERTED;
    }
    else if (0 == strcmp(stateStr, "ABSENT"))
    {
        state = LE_SIM_ABSENT;
    }
    else if (0 == strcmp(stateStr, "READY"))
    {
        state = LE_SIM_READY;
    }
    else if (0 == strcmp(stateStr, "BLOCKED"))
    {
        state = LE_SIM_BLOCKED;
    }
    else if (0 == strcmp(stateStr, "BUSY"))
    {
        state = LE_SIM_BUSY;
    }
    else if (0 == strcmp(stateStr, "POWER_DOWN"))
    {
        state = LE_SIM_POWER_DOWN;
    }
    else
    {
        LE_ERROR("Unknown SIM state '%s'", stateStr);
        return;
    }

    pa_simSimu_ReportSIMState(state);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the STK refresh mode.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetRefreshMode
(
    le_sim_StkRefreshMode_t mode
)
{
    StkRefreshMode = mode;
}


//--------------------------------------------------------------------------------------------------
/**
 * Set the STK refresh stage.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetRefreshStage
(
    le_sim_StkRefreshStage_t stage
)
{
    StkRefreshStage = stage;
}

//--------------------------------------------------------------------------------------------------
/**
 * Report the STK event.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_ReportSTKEvent
(
    le_sim_StkEvent_t  leSTKEvent
)
{
    StkEvent = leSTKEvent;

    pa_sim_StkEvent_t  paSTKEvent;
    paSTKEvent.simId = SelectedCard;
    paSTKEvent.stkEvent = leSTKEvent;
    paSTKEvent.stkRefreshStage = StkRefreshStage;
    paSTKEvent.stkRefreshMode = StkRefreshMode;

    le_event_Report(SimToolkitEvent, &paSTKEvent, sizeof(paSTKEvent));
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the card identification (ICCID).
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetCardIdentification
(
    const pa_sim_CardId_t iccid     ///< [IN] ICCID value
)
{
    le_utf8_Copy(Iccid, iccid, sizeof(pa_sim_CardId_t), NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the EID.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetEID
(
    const pa_sim_Eid_t eid     ///< [IN] EID value
)
{
    le_utf8_Copy(Eid, eid, sizeof(pa_sim_Eid_t), NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the card identification (ICCID).
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to get the value.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetCardIdentification
(
    pa_sim_CardId_t iccid     ///< [OUT] ICCID value
)
{
    switch (SimState)
    {
        case LE_SIM_BLOCKED:
        case LE_SIM_INSERTED:
        case LE_SIM_READY:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    le_utf8_Copy(iccid, Iccid, sizeof(pa_sim_CardId_t), NULL);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the International Mobile Subscriber Identity (IMSI).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetIMSI
(
    pa_sim_Imsi_t imsi   ///< [IN] IMSI value
)
{
    le_utf8_Copy(Imsi, imsi, sizeof(pa_sim_Imsi_t), NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the International Mobile Subscriber Identity (IMSI) from a string.
 */
//--------------------------------------------------------------------------------------------------
static void SetIMSIFromString
(
    const char* imsiStr   ///< [IN] IMSI value
)
{
    le_utf8_Copy(Imsi, imsiStr, sizeof(pa_sim_Imsi_t), NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the International Mobile Subscriber Identity (IMSI).
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to get the value.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetIMSI
(
    pa_sim_Imsi_t imsi   ///< [OUT] IMSI value
)
{
    switch (SimState)
    {
        case LE_SIM_READY:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    le_utf8_Copy(imsi, Imsi, sizeof(pa_sim_Imsi_t), NULL);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the SIM Status.
 *
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to get the value.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetState
(
    le_sim_States_t* statePtr    ///< [OUT] SIM state
)
{
    *statePtr = SimState;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function rerieves the identifier for the embedded Universal Integrated Circuit Card (EID)
 * (16 digits)
 *
 * @return LE_OK            The function succeeded.
 * @return LE_FAULT         The function failed.
 * @return LE_UNSUPPORTED   The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetCardEID
(
   pa_sim_Eid_t eid               ///< [OUT] the EID value
)
{
    switch (SimState)
    {
        case LE_SIM_BLOCKED:
        case LE_SIM_INSERTED:
        case LE_SIM_READY:
            break;
        default:
            return LE_FAULT;
    }

    le_utf8_Copy(eid, Eid, sizeof(pa_sim_Eid_t), NULL);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for new SIM state notification handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_sim_AddNewStateHandler
(
    pa_sim_NewStateHdlrFunc_t handler ///< [IN] The handler function.
)
{
    SimStateHandler = handler;

    // just for returning something
    return (le_event_HandlerRef_t) SimStateHandler;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for new SIM state notification handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_RemoveNewStateHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    SimStateHandler = NULL;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function enter the PIN code.
 *
 *
 * @return
 *      \return LE_BAD_PARAMETER The parameter is invalid.
 *      \return LE_NOT_POSSIBLE  The function failed to enter the value.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_EnterPIN
(
    pa_sim_PinType_t   type,  ///< [IN] pin type
    const pa_sim_Pin_t pin    ///< [IN] pin code
)
{
    switch (SimState)
    {
        case LE_SIM_INSERTED:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    // add a function to check the PIN
    if (strncmp(Pin, pin, strlen(Pin) ) != 0)
    {
        if (PinRemainingAttempts == 1)
        {
            /* Blocked */
            LE_INFO("SIM Blocked");
            pa_simSimu_ReportSIMState(LE_SIM_BLOCKED);
        }

        PinRemainingAttempts--;

        return LE_BAD_PARAMETER;
    }

    LE_INFO("PIN OK");
    PinRemainingAttempts = PA_SIMU_SIM_DEFAULT_PIN_REMAINING_ATTEMPTS;
    pa_simSimu_ReportSIMState(LE_SIM_READY);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the new PIN code.
 *
 *  - use to set pin code by providing the PUK
 *
 * All depends on SIM state which must be retrieved by @ref pa_sim_GetState
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to set the value.
 *      \return LE_BAD_PARAMETER The parameters are invalid.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_EnterPUK
(
    pa_sim_PukType_t   type, ///< [IN] PUK type
    const pa_sim_Puk_t puk,  ///< [IN] PUK code
    const pa_sim_Pin_t pin   ///< [IN] new PIN code
)
{
    switch (SimState)
    {
        case LE_SIM_BLOCKED:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    /* Check PUK code is valid */
    if (strncmp(puk, Puk, sizeof(pa_sim_Puk_t)) != 0)
    {
        if (PukRemainingAttempts <= 1)
        {
            /* TODO */
            PukRemainingAttempts = PA_SIMU_SIM_DEFAULT_PUK_REMAINING_ATTEMPTS;
        }
        else
        {
            PukRemainingAttempts--;
        }

        LE_INFO("PUK not OK");
        return LE_BAD_PARAMETER;
    }

    LE_INFO("PUK OK");
    PukRemainingAttempts = PA_SIMU_SIM_DEFAULT_PUK_REMAINING_ATTEMPTS;
    PinRemainingAttempts = PA_SIMU_SIM_DEFAULT_PIN_REMAINING_ATTEMPTS;
    pa_simSimu_ReportSIMState(LE_SIM_READY);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the remaining attempts of a pin code.
 *
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to get the value.
 *      \return LE_BAD_PARAMETER The 'type' parameter is invalid.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetPINRemainingAttempts
(
    pa_sim_PinType_t type,       ///< [IN] The PIN type
    uint32_t*        attemptsPtr ///< [OUT] The number of attempts still possible
)
{
    switch (SimState)
    {
        case LE_SIM_BUSY:
        case LE_SIM_STATE_UNKNOWN:
            return LE_NOT_POSSIBLE;
        default:
        break;
    }

    *attemptsPtr = PinRemainingAttempts;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the remaining attempts of a puk code.
 *
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to get the value.
 *      \return LE_BAD_PARAMETER The 'type' parameter is invalid.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetPUKRemainingAttempts
(
    pa_sim_PukType_t type,       ///< [IN] The puk type
    uint32_t*        attemptsPtr ///< [OUT] The number of attempts still possible
)
{
    switch (SimState)
    {
        case LE_SIM_BUSY:
        case LE_SIM_STATE_UNKNOWN:
            return LE_NOT_POSSIBLE;
        default:
        break;
    }

    *attemptsPtr = (PukRemainingAttempts-1);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function change a code.
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to set the value.
 *      \return LE_BAD_PARAMETER The parameters are invalid.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_ChangePIN
(
    pa_sim_PinType_t   type,    ///< [IN] The code type
    const pa_sim_Pin_t oldcode, ///< [IN] Old code
    const pa_sim_Pin_t newcode  ///< [IN] New code
)
{
    switch (SimState)
    {
        case LE_SIM_READY:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    if (strncmp(Pin, oldcode, strlen(Pin)) != 0)
    {
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function enables PIN locking (PIN or PIN2).
 *
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to set the value.
 *      \return LE_BAD_PARAMETER The parameters are invalid.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_EnablePIN
(
    pa_sim_PinType_t   type,  ///< [IN] The pin type
    const pa_sim_Pin_t code   ///< [IN] code
)
{
    switch (SimState)
    {
        case LE_SIM_READY:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    if (strncmp(code,Pin,strlen(Pin)) != 0)
    {
        return LE_NOT_POSSIBLE;
    }

    pa_simSimu_SetPINSecurity(true);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function disables PIN locking (PIN or PIN2).
 *
 *
 * @return
 *      \return LE_NOT_POSSIBLE  The function failed to set the value.
 *      \return LE_BAD_PARAMETER The parameters are invalid.
 *      \return LE_COMM_ERROR    The communication device has returned an error.
 *      \return LE_TIMEOUT       No response was received from the SIM card.
 *      \return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_DisablePIN
(
    pa_sim_PinType_t   type,  ///< [IN] The code type.
    const pa_sim_Pin_t code   ///< [IN] code
)
{
    if (code[0] == '\0')
        return LE_BAD_PARAMETER;

    switch (SimState)
    {
        case LE_SIM_INSERTED:
        case LE_SIM_READY:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    if (strncmp(code,Pin,strlen(Pin)) != 0)
    {
        return LE_NOT_POSSIBLE;
    }

    pa_simSimu_SetPINSecurity(false);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the SIM Phone Number.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetSubscriberPhoneNumber
(
    const char *phoneNumberStr
)
{
    le_utf8_Copy(PhoneNumber, phoneNumberStr, LE_MDMDEFS_PHONE_NUM_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the SIM Phone Number.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Phone Number can't fit in phoneNumberStr
 *      - LE_NOT_POSSIBLE on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetSubscriberPhoneNumber
(
    char        *phoneNumberStr,    ///< [OUT] The phone Number
    size_t       phoneNumberStrSize ///< [IN]  Size of phoneNumberStr
)
{
    switch (SimState)
    {
        case LE_SIM_READY:
            break;
        default:
            return LE_NOT_POSSIBLE;
    }

    if (phoneNumberStrSize < strlen(PhoneNumber))
    {
        return LE_OVERFLOW;
    }

    return le_utf8_Copy(phoneNumberStr, PhoneNumber, phoneNumberStrSize, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network Name information.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkOperator
(
    const char *nameStr
)
{
    int len = strlen(nameStr)+1;
    HomeNetworkOperator = malloc(len);
    le_utf8_Copy(HomeNetworkOperator, nameStr, len, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network Name information.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Home Network Name can't fit in nameStr
 *      - LE_NOT_POSSIBLE on any other failure
 *
 * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *       function will not return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetHomeNetworkOperator
(
    char       *nameStr,               ///< [OUT] the home network Name
    size_t      nameStrSize            ///< [IN] the nameStr size
)
{
    switch (SimState)
    {
        case LE_SIM_READY:
            break;
        default:
            return LE_FAULT;
    }

    if ( nameStrSize < strlen(HomeNetworkOperator) )
    {
        return LE_OVERFLOW;
    }

    le_utf8_Copy(nameStr, HomeNetworkOperator, nameStrSize, NULL);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network MCC MNC.
 *
 * If a parameter exceeds the allowed size the application will exit.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkMccMnc
(
    const char *mccPtr, ///< [IN] MCC (max length LE_MRC_MCC_BYTES)
    const char *mncPtr  ///< [IN] MNC (max length LE_MRC_MNC_BYTES)
)
{
    pa_simSimu_SetHomeNetworkMcc(mccPtr);
    pa_simSimu_SetHomeNetworkMnc(mncPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network MCC.
 *
 * If a parameter exceeds the allowed size the application will exit.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkMcc
(
    const char *mccPtr  ///< [IN] MCC (max length LE_MRC_MCC_BYTES)
)
{
    LE_ASSERT(strlen(mccPtr) <= LE_MRC_MCC_BYTES);
    le_utf8_Copy(HomeMcc, mccPtr, LE_MRC_MCC_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network MNC.
 *
 * If a parameter exceeds the allowed size the application will exit.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkMnc
(
    const char *mncPtr  ///< [IN] MNC (max length LE_MRC_MNC_BYTES)
)
{
    LE_ASSERT(strlen(mncPtr) <= LE_MRC_MNC_BYTES);
    le_utf8_Copy(HomeMnc, mncPtr, LE_MRC_MNC_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Home Network MCC MNC.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the Home Network MCC/MNC can't fit in mccPtr and mncPtr
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetHomeNetworkMccMnc
(
    char     *mccPtr,                ///< [OUT] Mobile Country Code
    size_t    mccPtrSize,            ///< [IN] mccPtr buffer size
    char     *mncPtr,                ///< [OUT] Mobile Network Code
    size_t    mncPtrSize             ///< [IN] mncPtr buffer size
)
{
    le_result_t res;

    switch (SimState)
    {
        case LE_SIM_READY:
            break;
        default:
            return LE_FAULT;
    }

    res = le_utf8_Copy(mccPtr, HomeMcc, mccPtrSize, NULL);
    if(res != LE_OK)
    {
        return res;
    }

    res = le_utf8_Copy(mncPtr, HomeMnc, mncPtrSize, NULL);
    if(res != LE_OK)
    {
        return res;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to open a logical channel on the SIM card.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_OpenLogicalChannel
(
    uint8_t* channelPtr  ///< [OUT] channel number
)
{
    if (!channelPtr)
    {
        LE_ERROR("No channel pointer");
        return LE_FAULT;
    }

    *channelPtr = 1;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to close a logical channel on the SIM card.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_CloseLogicalChannel
(
    uint8_t channel  ///< [IN] channel number
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set SimAccessTest variable
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetSIMAccessTest
(
    bool testInProgress
)
{
    SimAccessTest = testInProgress;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send an APDU message to the SIM card.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW the response length exceed the maximum buffer length.
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SendApdu
(
    uint8_t        channel, ///< [IN] Logical channel.
    const uint8_t* apduPtr, ///< [IN] APDU message buffer
    uint32_t       apduLen, ///< [IN] APDU message length in bytes
    uint8_t*       respPtr, ///< [OUT] APDU message response.
    size_t*        lenPtr   ///< [IN,OUT] APDU message response length in bytes.
)
{
    // Response for APDU command successfully executed
    uint8_t result[] = {0x90, 0x00};

    LE_ASSERT(NULL != apduPtr);
    LE_ASSERT(*lenPtr >= sizeof(result));
    LE_ASSERT(NULL != respPtr);

    if (SimAccessTest)
    {
        uint8_t expectedApdu[]={0x00, 0xA4, 0x00, 0x0C, 0x02, 0x6F, 0x07};
        LE_ASSERT(apduLen == sizeof(expectedApdu));
        LE_ASSERT(0 == memcmp(apduPtr, expectedApdu, apduLen));
    }

    memcpy(respPtr, result, sizeof(result));
    *lenPtr = sizeof(result);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to trigger a SIM refresh.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for unexpected error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_Refresh
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for SIM Toolkit event notification handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_sim_AddSimToolkitEventHandler
(
    pa_sim_SimToolkitEventHdlrFunc_t handler,    ///< [IN] The handler function.
    void*                            contextPtr  ///< [IN] The context to be given to the handler.
)
{
    le_event_HandlerRef_t handlerRef = le_event_AddHandler("SimToolkitEventHandler",
                                           SimToolkitEvent,
                                           (le_event_HandlerFunc_t) handler);

    le_event_SetContextPtr (handlerRef, contextPtr);

    return handlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for SIM Toolkit event notification
 * handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_RemoveSimToolkitEventHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function creates a semaphore that should be used to wait for an STK confirmation call
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_CreateSempahoreForSTKConfirmation
(
    void
)
{
    if (NULL == SyncSemaphore)
    {
        SyncSemaphore = le_sem_Create("SyncSemaphore", 0);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function deletes the semaphore used in STK confirmation
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_DeleteSempahoreForSTKConfirmation
(
    void
)
{
    if (SyncSemaphore)
    {
        le_sem_Delete(SyncSemaphore);
        SyncSemaphore = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function  waits for an STK confirmation call. This function requires a semaphore to be
 * created by calling pa_simSimu_CreateSempahoreForSTKConfirmation()
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_WaitForSTKConfirmation
(
    void
)
{
    if (SyncSemaphore)
    {
        le_sem_Wait(SyncSemaphore);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the expected confirmation command.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetExpectedSTKConfirmationCommand
(
    bool  confirmation ///< [IN] true to accept, false to reject
)
{
    STKConfirmation = confirmation;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to confirm a SIM Toolkit command.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_ConfirmSimToolkitCommand
(
    bool  confirmation ///< [IN] true to accept, false to reject
)
{
    LE_ASSERT(STKConfirmation == confirmation);

    if (NULL != SyncSemaphore)
    {
        le_sem_Post(SyncSemaphore);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * SIM Stub initialization.
 *
 * @return LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_simSimu_Init
(
    void
)
{
    LE_INFO("PA SIM Init");

    SimStateEventPool = le_mem_CreatePool("SimEventPool", sizeof(pa_sim_Event_t));
    SimToolkitEvent = le_event_CreateId("SimToolkitEvent", sizeof(pa_sim_StkEvent_t));

    simuConfig_RegisterService(&ConfigService);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to send a generic command to the SIM.
 *
 * @return
 *      - LE_OK             Function succeeded.
 *      - LE_FAULT          The function failed.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_NOT_FOUND      - The function failed to select the SIM card for this operation
 *                          - The requested SIM file is not found
 *      - LE_OVERFLOW       Response buffer is too small to copy the SIM answer.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_SendCommand
(
    le_sim_Command_t command,               ///< [IN] The SIM command
    const char*      fileIdentifierPtr,     ///< [IN] File identifier
    uint8_t          p1,                    ///< [IN] Parameter P1 passed to the SIM
    uint8_t          p2,                    ///< [IN] Parameter P2 passed to the SIM
    uint8_t          p3,                    ///< [IN] Parameter P3 passed to the SIM
    const uint8_t*   dataPtr,               ///< [IN] Data command
    size_t           dataNumElements,       ///< [IN] Size of data command
    const char*      pathPtr,               ///< [IN] Path of the elementary file
    uint8_t*         sw1Ptr,                ///< [OUT] SW1 received from the SIM
    uint8_t*         sw2Ptr,                ///< [OUT] SW2 received from the SIM
    uint8_t*         responsePtr,           ///< [OUT] SIM response
    size_t*          responseNumElementsPtr ///< [IN/OUT] Size of response
)
{
    *sw1Ptr = 0x90;
    *sw2Ptr = 0x00;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to reset the SIM.
 *
 * @return
 *      - LE_OK          On success.
 *      - LE_FAULT       On failure.
 *      - LE_UNSUPPORTED The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_Reset
(
    void
)
{
    LE_ERROR("Unsupported function called");
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to write the FPLMN list into the modem.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_FAULT          On failure.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_WriteFPLMNList
(
    le_dls_List_t *FPLMNListPtr ///< [IN] List of FPLMN operators
)
{
    pa_sim_FPLMNOperator_t* nodePtr;
    le_dls_Link_t *linkPtr;

    int i;
    for(i = 0, linkPtr = le_dls_Peek(FPLMNListPtr); (linkPtr != NULL) && (i < MAX_FPLMN_OPERATOR);
        i++, linkPtr = le_dls_PeekNext(FPLMNListPtr, linkPtr))
    {
        // Get the node from FPLMNList
        nodePtr = CONTAINER_OF(linkPtr, pa_sim_FPLMNOperator_t, link);

        le_utf8_Copy(FPLMNOperator[i].mcc, nodePtr->mobileCode.mcc, LE_MRC_MCC_BYTES, NULL);
        le_utf8_Copy(FPLMNOperator[i].mnc, nodePtr->mobileCode.mnc, LE_MRC_MNC_BYTES, NULL);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the number of FPLMN operators present in the list.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_FAULT          On failure.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_CountFPLMNOperators
(
    uint32_t*  nbItemPtr     ///< [OUT] number of FPLMN operator found if success.
)
{
    *nbItemPtr = MAX_FPLMN_OPERATOR;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to read the FPLMN list.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_NOT_FOUND      If no FPLMN network is available.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_ReadFPLMNOperators
(
    pa_sim_FPLMNOperator_t* FPLMNOperatorPtr,   ///< [OUT] FPLMN operators.
    uint32_t* FPLMNOperatorCountPtr             ///< [IN/OUT] FPLMN operator count.
)
{
    if (*FPLMNOperatorCountPtr > MAX_FPLMN_OPERATOR)
    {
        *FPLMNOperatorCountPtr = MAX_FPLMN_OPERATOR;
    }

    int i;
    for (i = 0; i < *FPLMNOperatorCountPtr; i++)
    {
        le_utf8_Copy(FPLMNOperatorPtr[i].mobileCode.mcc, FPLMNOperator[i].mcc, LE_MRC_MCC_BYTES,
                     NULL);
        le_utf8_Copy(FPLMNOperatorPtr[i].mobileCode.mnc, FPLMNOperator[i].mnc, LE_MRC_MNC_BYTES,
                     NULL);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the last SIM Toolkit status.
 *
 * @return
 *      - LE_OK             On success.
 *      - LE_BAD_PARAMETER  A parameter is invalid.
 *      - LE_UNSUPPORTED    The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_sim_GetLastStkStatus
(
    pa_sim_StkEvent_t*  stkStatus  ///< [OUT] last SIM Toolkit event status
)
{
    if (NULL == stkStatus)
    {
        return LE_BAD_PARAMETER;
    }

    stkStatus->simId = SelectedCard;
    stkStatus->stkEvent = StkEvent;
    stkStatus->stkRefreshMode = StkRefreshMode;
    stkStatus->stkRefreshStage = StkRefreshStage;

    return LE_OK;
}
