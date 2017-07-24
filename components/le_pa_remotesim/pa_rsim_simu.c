/**
 * @file pa_rsim_simu.c
 *
 * Simulation implementation of @ref c_pa_rsim API.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_rsim.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Minimum APDU response length: 2 bytes for the SWI return code
 */
//--------------------------------------------------------------------------------------------------
#define APDU_MIN 2

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a SIM action request is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t ActionRequestEvent;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when an APDU indication is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t ApduIndicationEvent;


//--------------------------------------------------------------------------------------------------
// Private functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer SIM action request handler
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerActionRequestHandler
(
    void* reportPtr,
    void* secondLayerHandlerFunc
)
{
    pa_rsim_Action_t* eventDataPtr = reportPtr;
    pa_rsim_SimActionHdlrFunc_t clientHandlerFunc = secondLayerHandlerFunc;

    clientHandlerFunc(*eventDataPtr);
}


//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to add an APDU indication notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_rsim_AddApduNotificationHandler
(
    pa_rsim_ApduIndHdlrFunc_t indicationHandler ///< [IN] The handler function to handle an APDU
                                                ///  notification reception.
)
{
    le_event_HandlerRef_t apduIndHandler;
    LE_ASSERT(NULL != indicationHandler);

    apduIndHandler = le_event_AddHandler("PaApduNotificationHandler",
                                         ApduIndicationEvent,
                                         (le_event_HandlerFunc_t) indicationHandler);

    return (le_event_HandlerRef_t) apduIndHandler;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to unregister an APDU indication notification handler.
 */
//--------------------------------------------------------------------------------------------------
void pa_rsim_RemoveApduNotificationHandler
(
    le_event_HandlerRef_t apduIndHandler
)
{
    le_event_RemoveHandler(apduIndHandler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is used to add a SIM action request notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_rsim_AddSimActionRequestHandler
(
    pa_rsim_SimActionHdlrFunc_t actionHandler   ///< [IN] The handler function to handle a SIM
                                                ///  action request notification reception.
)
{
    le_event_HandlerRef_t actionRequestHandler;
    LE_ASSERT(NULL != actionHandler);

    actionRequestHandler = le_event_AddLayeredHandler("PaSimActionRequestHandler",
                                                      ActionRequestEvent,
                                                      FirstLayerActionRequestHandler,
                                                      actionHandler);

    return (le_event_HandlerRef_t) actionRequestHandler;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to unregister a SIM action request notification handler.
 */
//--------------------------------------------------------------------------------------------------
void pa_rsim_RemoveSimActionRequestHandler
(
    le_event_HandlerRef_t actionRequestHandler
)
{
    le_event_RemoveHandler(actionRequestHandler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to notify the modem of the remote SIM disconnection.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_Disconnect
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to notify the modem of a remote SIM status change.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_BAD_PARAMETER  Unknown SIM status.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_NotifyStatus
(
    pa_rsim_SimStatus_t simStatus   ///< [IN] SIM status change
)
{
    // Check if status is correct
    if (simStatus >= PA_RSIM_STATUS_COUNT)
    {
        LE_ERROR("Unknown SIM status %d reported!", simStatus);
        return LE_BAD_PARAMETER;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to transfer an APDU response to the modem.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_BAD_PARAMETER  APDU too long.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_TransferApduResp
(
    const uint8_t* apduPtr,     ///< [IN] APDU buffer
    uint16_t       apduLen      ///< [IN] APDU length in bytes
)
{
    LE_DEBUG("Transfer APDU response (length %d):", apduLen);
    LE_DUMP(apduPtr, apduLen);

    if (apduLen < APDU_MIN)
    {
        return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to indicate an APDU response error to the modem.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_TransferApduRespError
(
    void
)
{
    LE_DEBUG("Received APDU response error");

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function is called to transfer an Answer to Reset (ATR) response to the modem.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_BAD_PARAMETER  ATR too long.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_TransferAtrResp
(
    pa_rsim_SimStatus_t simStatus,  ///< [IN] SIM status change
    const uint8_t* atrPtr,          ///< [IN] ATR buffer
    uint16_t       atrLen           ///< [IN] ATR length in bytes
)
{
    LE_DEBUG("Received ATR:");
    LE_DUMP(atrPtr, atrLen);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function indicates if the Remote SIM service is supported by the PA.
 *
 * @return
 *  - true      Remote SIM service is supported.
 *  - false     Remote SIM service is not supported.
 */
//--------------------------------------------------------------------------------------------------
bool pa_rsim_IsRsimSupported
(
    void
)
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function checks if the remote SIM card is selected.
 *
 * @return true         If the remote SIM is selected.
 * @return false        It the remote SIM is not selected.
 */
//--------------------------------------------------------------------------------------------------
bool pa_rsim_IsRemoteSimSelected
(
    void
)
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA Remote SIM service module.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_rsim_Init
(
    void
)
{
    // Create the events for signaling user handlers.
    ActionRequestEvent = le_event_CreateId("ActionRequestEvent",
                                           sizeof(pa_rsim_Action_t));
    ApduIndicationEvent = le_event_CreateId("ApduIndicationEvent",
                                            sizeof(pa_rsim_ApduInd_t));

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Send a SIM action request to the remote SIM service
 */
//--------------------------------------------------------------------------------------------------
void pa_rsimSimu_SendSimActionRequest
(
    pa_rsim_Action_t action
)
{
    le_event_Report(ActionRequestEvent, (void *)&action, sizeof(action));
}

//--------------------------------------------------------------------------------------------------
/**
 * Send an APDU indication to the remote SIM service
 */
//--------------------------------------------------------------------------------------------------
void pa_rsimSimu_SendApduInd
(
    const uint8_t* apduPtr,
    size_t apduLen
)
{
    pa_rsim_ApduInd_t apduInd;
    memset(&apduInd, 0, sizeof(apduInd));

    memcpy(apduInd.apduData, apduPtr, apduLen);
    apduInd.apduLength = apduLen;

    le_event_Report(ApduIndicationEvent, &apduInd, sizeof(apduInd));
}
