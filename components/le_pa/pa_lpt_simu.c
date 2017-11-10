/**
 * @file pa_lpt_simu.c
 *
 * Simulation implementation of @ref c_pa_lpt API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_lpt_simu.h"


//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Invalid value for eDRX cycle length, used for initialization.
 */
//--------------------------------------------------------------------------------------------------
#define INVALID_EDRX_VALUE  -1


//--------------------------------------------------------------------------------------------------
// Static declarations.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when an eDRX parameters change indication is received from the modem.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t EDrxParamsChangeEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for eDRX parameters change indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t EDrxParamsChangeIndPool;

//--------------------------------------------------------------------------------------------------
/**
 * Requested eDRX cycle length values for the different Radio Access Technologies.
 */
//--------------------------------------------------------------------------------------------------
static int8_t RequestedEDrxValue[LE_LPT_EDRX_RAT_MAX] =
{
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_UNKNOWN
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_EC_GSM_IOT
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_GSM
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_UTRAN
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_LTE_M1
    INVALID_EDRX_VALUE,     ///< LE_LPT_EDRX_RAT_LTE_NB1
};


//--------------------------------------------------------------------------------------------------
// APIs.
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for eDRX parameters change indication.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_lpt_AddEDrxParamsChangeHandler
(
    pa_lpt_EDrxParamsChangeIndHandlerFunc_t handlerFuncPtr  ///< [IN] The handler function.
)
{
    LE_ASSERT(handlerFuncPtr);

    return le_event_AddHandler("EDrxParamsChange",
                               EDrxParamsChangeEventId,
                               (le_event_HandlerFunc_t) handlerFuncPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the eDRX activation state for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_SetEDrxState
(
    le_lpt_EDrxRat_t    eDrxRat,    ///< [IN] Radio Access Technology.
    le_onoff_t          activation  ///< [IN] eDRX activation state.
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the requested eDRX cycle value for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_SetRequestedEDrxValue
(
    le_lpt_EDrxRat_t    eDrxRat,    ///< [IN] Radio Access Technology.
    uint8_t             eDrxValue   ///< [IN] Requested eDRX cycle value.
)
{
    RequestedEDrxValue[eDrxRat] = eDrxValue;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the requested eDRX cycle value for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_GetRequestedEDrxValue
(
    le_lpt_EDrxRat_t    eDrxRat,        ///< [IN] Radio Access Technology.
    uint8_t*            eDrxValuePtr    ///< [OUT] Requested eDRX cycle value
)
{
    if ((LE_LPT_EDRX_RAT_UNKNOWN == eDrxRat) || (eDrxRat >= LE_LPT_EDRX_RAT_MAX))
    {
        LE_ERROR("Invalid Radio Access Technology: %d", eDrxRat);
        return LE_BAD_PARAMETER;
    }

    if (!eDrxValuePtr)
    {
        LE_ERROR("Invalid parameter");
        return LE_BAD_PARAMETER;
    }

    if (INVALID_EDRX_VALUE == RequestedEDrxValue[eDrxRat])
    {
        *eDrxValuePtr = 0;
        return LE_UNAVAILABLE;
    }

    *eDrxValuePtr = RequestedEDrxValue[eDrxRat];
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the network-provided eDRX cycle value for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_UNAVAILABLE    No network-provided eDRX cycle value.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_GetNetworkProvidedEDrxValue
(
    le_lpt_EDrxRat_t    eDrxRat,        ///< [IN]  Radio Access Technology.
    uint8_t*            eDrxValuePtr    ///< [OUT] Network-provided eDRX cycle value.
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the network-provided Paging Time Window for the given Radio Access Technology.
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_BAD_PARAMETER  A parameter is invalid.
 *  - LE_UNSUPPORTED    eDRX is not supported by the platform.
 *  - LE_UNAVAILABLE    No defined Paging Time Window.
 *  - LE_FAULT          The function failed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_lpt_GetNetworkProvidedPagingTimeWindow
(
    le_lpt_EDrxRat_t    eDrxRat,            ///< [IN]  Radio Access Technology.
    uint8_t*            pagingTimeWindowPtr ///< [OUT] Network-provided Paging Time Window.
)
{
    return LE_OK;
}

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
)
{
    pa_lpt_EDrxParamsIndication_t* eDrxParamsChangeIndPtr;

    eDrxParamsChangeIndPtr = le_mem_ForceAlloc(EDrxParamsChangeIndPool);
    eDrxParamsChangeIndPtr->rat = rat;
    eDrxParamsChangeIndPtr->activation = activation;
    eDrxParamsChangeIndPtr->eDrxValue = eDrxValue;
    eDrxParamsChangeIndPtr->pagingTimeWindow = pagingTimeWindow;

    le_event_ReportWithRefCounting(EDrxParamsChangeEventId, eDrxParamsChangeIndPtr);
}

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
)
{
    LE_INFO("LPT simulated PA initialization");

    // Create event
    EDrxParamsChangeEventId = le_event_CreateIdWithRefCounting("EDrxParamsChangeEvent");

    // Create associated memory pool
    EDrxParamsChangeIndPool = le_mem_CreatePool("EDrxParamsChangeIndPool",
                                                sizeof(pa_lpt_EDrxParamsIndication_t));

    return LE_OK;
}
