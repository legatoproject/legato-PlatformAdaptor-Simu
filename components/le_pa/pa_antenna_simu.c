/**
 * @file pa_antenna_simu.c
 *
 * SIMU implementation of antenna API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_antenna.h"
#include "pa_antenna_simu.h"

//------------------------------------------------------------------------------------------------
/**
 * Antenna Short Limit
 */
//--------------------------------------------------------------------------------------------------
static uint32_t ShortLimit[LE_ANTENNA_MAX];


//------------------------------------------------------------------------------------------------
/**
 * Antenna Open Limit
 */
//--------------------------------------------------------------------------------------------------
static uint32_t OpenLimit[LE_ANTENNA_MAX];


//------------------------------------------------------------------------------------------------
/**
 * Antenna Adc Id
 */
//--------------------------------------------------------------------------------------------------
static int8_t AdcId[LE_ANTENNA_MAX];

//--------------------------------------------------------------------------------------------------
/*
 * Return code
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReturnCode = LE_FAULT;

//------------------------------------------------------------------------------------------------
/**
 * Antenna status
 */
//--------------------------------------------------------------------------------------------------
static le_antenna_Status_t Status = LE_ANTENNA_CLOSE_CIRCUIT;


//------------------------------------------------------------------------------------------------
/**
 * Antenna status Event
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t AntennaStatusEvent;


//--------------------------------------------------------------------------------------------------
/**
 * Antenna identifier definition
 */
//--------------------------------------------------------------------------------------------------
typedef uint8_t pa_antenna_Id_t;


//--------------------------------------------------------------------------------------------------
/**
 * Antenna selected for the status indication event
 */
//--------------------------------------------------------------------------------------------------
static uint8_t AntennaSelectionMask = 0;


//--------------------------------------------------------------------------------------------------
/**
 * PA Antenna context
 */
//--------------------------------------------------------------------------------------------------
static struct
{
    le_antenna_Status_t currentStatus;

} PaAntennaCtx[LE_ANTENNA_MAX];


//--------------------------------------------------------------------------------------------------
/**
 * This function is called to convert the le_antenna_Type_t type  into
 * the pa internal identifier.
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT when the antenna type given in parameter is unknown
 *
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ConvertAntennaTypeToId
(
    le_antenna_Type_t antennaType,
    pa_antenna_Id_t* antennaIdPtr
)
{
    switch (antennaType)
    {
        case LE_ANTENNA_PRIMARY_CELLULAR:
        {
            *antennaIdPtr = 1;
        }
        break;

        case LE_ANTENNA_DIVERSITY_CELLULAR:
        {
            *antennaIdPtr = 2;
        }
        break;

        case LE_ANTENNA_GNSS:
        {
            *antennaIdPtr = 4;
        }
        break;

        default:
            return LE_FAULT;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Set the return code.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_antennaSimu_SetReturnCode
(
    le_result_t res
)
{
    ReturnCode = res;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the short limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetShortLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t             shortLimit
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);

        return LE_FAULT;
    }

    if (ReturnCode == LE_FAULT)
    {
        return LE_FAULT;
    }
    ShortLimit[antennaType] = shortLimit;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the short limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetShortLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t*            shortLimitPtr
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    if (ReturnCode == LE_FAULT)
    {
        return LE_FAULT;
    }
    *shortLimitPtr = ShortLimit[antennaType];
    return LE_OK;

}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the open limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetOpenLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t             openLimit
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    if (ReturnCode == LE_FAULT)
    {
        return LE_FAULT;
    }
    OpenLimit[antennaType] = openLimit;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the open limit
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetOpenLimit
(
    le_antenna_Type_t    antennaType,
    uint32_t*            openLimitPtr
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    if (ReturnCode == LE_FAULT)
    {
        return LE_FAULT;
    }
    *openLimitPtr = OpenLimit[antennaType];
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to get the antenna status
 *
 * @return
 * - LE_OK on success
 * - LE_UNSUPPORTED request not supported
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetStatus
(
    le_antenna_Type_t    antennaType,
    le_antenna_Status_t* statusPtr
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    if (ReturnCode == LE_UNSUPPORTED)
    {
        return LE_UNSUPPORTED;
    }
    *statusPtr = Status;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the external ADC used to monitor the requested antenna.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT on other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetExternalAdc
(
    le_antenna_Type_t    antennaType,   ///< Antenna type
    int8_t               adcId          ///< The external ADC used to monitor the requested antenna
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    if (ReturnCode == LE_UNSUPPORTED)
    {
        return LE_UNSUPPORTED;
    }
    le_antenna_Type_t    antenna;
    // flag to monitor whether adc index already set to other antenna type.
    bool IsUsed = false;
    // set the flag if  adcId is already used for other antenna type
    for (antenna = LE_ANTENNA_PRIMARY_CELLULAR; antenna < LE_ANTENNA_MAX; antenna++)
    {
        if ((antenna != antennaType) && (AdcId[antenna] == adcId))
        {
            IsUsed = true;
            break;
        }
    }
    if (false == IsUsed)
    {
        AdcId[antennaType] = adcId;
        return LE_OK;
    }
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the external ADC used to monitor the requested antenna.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT on other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_GetExternalAdc
(
    le_antenna_Type_t    antennaType,  ///< Antenna type
    int8_t*              adcIdPtr      ///< The external ADC used to monitor the requested antenna
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }


    if (ReturnCode == LE_UNSUPPORTED)
    {
        *adcIdPtr = -1;
        return LE_UNSUPPORTED;
    }
    *adcIdPtr = AdcId[antennaType];
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to set the status indication on a specific antenna
 *
 * * @return
 * - LE_OK on success
 * - LE_BUSY if the status indication is already set for the requested antenna
 * - LE_FAULT on other failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_SetStatusIndication
(
    le_antenna_Type_t antennaType
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    //  Check if antenna status is not already selected
    if ( !(AntennaSelectionMask & antennaId) )
    {
        // initialize the current status
        if (pa_antenna_GetStatus(antennaType, &PaAntennaCtx[antennaType].currentStatus) != LE_OK)
        {
            LE_ERROR("Unable to get the status");
            return LE_FAULT;
        }

        // Remember the request for the next time
        AntennaSelectionMask |= antennaId;
        LE_DEBUG("AntennaSelectionMask %d", AntennaSelectionMask);
        return LE_OK;
    }
    else
    {
        return LE_BUSY;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to remove the status indication on a specific antenna
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_RemoveStatusIndication
(
    le_antenna_Type_t antennaType
)
{
    pa_antenna_Id_t antennaId;

    // Convert the LE antenna type into antenna Id
    if (ConvertAntennaTypeToId(antennaType, &antennaId) != LE_OK)
    {
        LE_ERROR("Unknown antenna type %d", antennaType);
        return LE_FAULT;
    }

    if (AntennaSelectionMask & antennaId)
    {
        AntennaSelectionMask &= (uint8_t) ~antennaId;
        LE_DEBUG("AntennaSelectionMask %d", AntennaSelectionMask);
        return LE_OK;
    }
    else
    {
        // Antenna not selected
        LE_ERROR("No subscribed to the status indication");
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to add a status notification handler
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t* pa_antenna_AddStatusHandler
(
    pa_antenna_StatusIndHandlerFunc_t   msgHandler
)
{
    le_event_HandlerRef_t  handlerRef = NULL;

    if (msgHandler != NULL)
    {
        handlerRef = le_event_AddHandler("PaAntennaStatusHandler",
                              AntennaStatusEvent,
                            (le_event_HandlerFunc_t) msgHandler);
    }
    else
    {
        LE_ERROR("Null handler given in parameter");
    }

    return (le_event_HandlerRef_t*) handlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 *
 * This function is used to intialize the PA antenna
 *
 * @return
 * - LE_OK on success
 * - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_antenna_Init
(
    void
)
{

    // Create the event for signaling user handlers.
    AntennaStatusEvent = le_event_CreateId("AntennaStatusEvent",sizeof(pa_antenna_StatusInd_t));

    le_antenna_Type_t    antenna;
    // set default value -1 to adcId for all antenna type
    for (antenna = LE_ANTENNA_PRIMARY_CELLULAR; antenna < LE_ANTENNA_MAX; antenna++)
    {
        AdcId[antenna] = -1;
    }
    return LE_OK;
}
