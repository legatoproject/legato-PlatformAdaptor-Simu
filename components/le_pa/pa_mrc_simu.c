/**
 * @file pa_mrc_simu.c
 *
 * Simulation implementation of @ref c_pa_mrc API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_simu.h"
#include "pa_mrc_simu.h"
#include "pa_sim_simu.h"

//--------------------------------------------------------------------------------------------------
/**
 * Minimum signal delta for RAT TD-SCDMA.
 */
//--------------------------------------------------------------------------------------------------
#define MIN_SIGNAL_DELTA_FOR_TDSCDMA  10

//--------------------------------------------------------------------------------------------------
/**
 * Maximum and default values for SAR backoff state
 */
//--------------------------------------------------------------------------------------------------
#define SAR_BACKOFF_STATE_MAX       8
#define SAR_BACKOFF_STATE_DEFAULT   0

//--------------------------------------------------------------------------------------------------
/**
 * The internal current RAT setting
 */
//--------------------------------------------------------------------------------------------------
static le_mrc_Rat_t   Rat = LE_MRC_RAT_GSM;

//--------------------------------------------------------------------------------------------------
/**
 * The internal current BAND settings
 */
//--------------------------------------------------------------------------------------------------
static le_mrc_BandBitMask_t CurrentBand = LE_MRC_BITMASK_BAND_GSM_DCS_1800;
static le_mrc_LteBandBitMask_t CurrentLteBand = LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_11;
static le_mrc_TdScdmaBandBitMask_t CurrentTdScdmaBand = LE_MRC_BITMASK_TDSCDMA_BAND_C;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a Radio Access Technology change indication is received from the
 * modem. The report data is allocated from the associated pool.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t RatChangeEvent;

//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a registration state indication is received from the modem. The
 * report data is allocated from the associated pool.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t NewRegStateEvent;


//--------------------------------------------------------------------------------------------------
/**
 * This event is reported when a Packet Switched change indication is received from the
 * modem. The report data is allocated from the associated pool.
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t PSChangeEventId;

//--------------------------------------------------------------------------------------------------
/**
 * Pool for Packet Switched change indication reporting.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t PSChangePool;

//--------------------------------------------------------------------------------------------------
/**
 * The pa_mrc_ScanInformation_t pool
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t ScanInformationPool;

//--------------------------------------------------------------------------------------------------
/**
 * The internal radio power state
 */
//--------------------------------------------------------------------------------------------------
static le_onoff_t RadioPower = LE_ON;

//--------------------------------------------------------------------------------------------------
/**
 * The internal SAR state
 */
//--------------------------------------------------------------------------------------------------
static uint8_t SarBackoffStatus = SAR_BACKOFF_STATE_DEFAULT;

//--------------------------------------------------------------------------------------------------
/**
 * The internal manual section mode status
 */
//--------------------------------------------------------------------------------------------------
static bool IsManual = false;


static char CurentMccStr[4];
static char CurentMncStr[4];

//--------------------------------------------------------------------------------------------------
/**
 * Static for jamming detection avtivation state
 */
//--------------------------------------------------------------------------------------------------
static pa_mrcSimu_JammingDetection_t JammingActivationState;

//--------------------------------------------------------------------------------------------------
/**
 * Static for jamming pool
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t JammingDetectionIndPool;

//--------------------------------------------------------------------------------------------------
/**
 * Jamming detection reference structure.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    le_msg_SessionRef_t sessionRef;     ///< Message session reference
    le_dls_Link_t       link;           ///< Object node link
}
JammingDetectionRef_t;

//--------------------------------------------------------------------------------------------------
/**
 * Static event for jamming detection
 */
//--------------------------------------------------------------------------------------------------
static le_event_Id_t JammingDetectionEventId;

//--------------------------------------------------------------------------------------------------
/**
 * This function determine if the tupple (rat,mcc,mnc) is currently provided by the simulation.
 */
//--------------------------------------------------------------------------------------------------
static bool IsNetworkInUse
(
    le_mrc_Rat_t   rat,      /// [IN] RAT
    const char    *mccPtr,   /// [IN] MCC
    const char    *mncPtr    /// [IN] MNC
)
{
    le_mrc_Rat_t currentRat;
    le_result_t res;

    res =  pa_mrc_GetRadioAccessTechInUse(&currentRat);
    LE_FATAL_IF( (res != LE_OK), "Unable to get current RAT");

    if(currentRat != rat)
    {
        return false;
    }

    // @TODO Compare MCC/MNC

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Append a simulated result for the specified RAT to the list of Scan Information
 */
//--------------------------------------------------------------------------------------------------
static void AppendNetworkScanResult
(
    le_mrc_Rat_t   rat,                   /// [IN] Requested simulated RAT result
    le_dls_List_t *scanInformationListPtr ///< [OUT] list of pa_mrc_ScanInformation_t
)
{
    pa_mrc_ScanInformation_t *newScanInformationPtr = NULL;

    char mccStr[LE_MRC_MCC_BYTES];
    char mncStr[LE_MRC_MNC_BYTES];

    newScanInformationPtr = le_mem_ForceAlloc(ScanInformationPool);

    memset(newScanInformationPtr, 0, sizeof(*newScanInformationPtr));
    newScanInformationPtr->link = LE_DLS_LINK_INIT;

    // @TODO Default to SIM MCC/MNC
    strncpy(mccStr, PA_SIMU_SIM_DEFAULT_MCC, sizeof(mccStr));
    strncpy(mncStr, PA_SIMU_SIM_DEFAULT_MNC, sizeof(mncStr));

    newScanInformationPtr->rat = rat;
    le_utf8_Copy(newScanInformationPtr->mobileCode.mcc, mccStr,
                 sizeof(newScanInformationPtr->mobileCode.mcc), NULL);
    le_utf8_Copy(newScanInformationPtr->mobileCode.mnc, mncStr,
                 sizeof(newScanInformationPtr->mobileCode.mnc), NULL);
    newScanInformationPtr->isInUse = IsNetworkInUse(rat, mccStr, mncStr);
    newScanInformationPtr->isAvailable = !(newScanInformationPtr->isInUse);
    newScanInformationPtr->isHome = true;
    newScanInformationPtr->isForbidden = false;

    le_dls_Queue(scanInformationListPtr, &(newScanInformationPtr->link));
}


//--------------------------------------------------------------------------------------------------
/**
 * Report jamming detection event.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrcSimu_ReportJammingDetection
(
    le_mrc_JammingReport_t  report,     ///< Notification type
    le_mrc_JammingStatus_t  status      ///< Jamming status
)
{
    pa_mrc_JammingDetectionIndication_t* paJammingEvent =
                                        le_mem_ForceAlloc(JammingDetectionIndPool);

    paJammingEvent->report = report;
    paJammingEvent->status = status;

    le_event_ReportWithRefCounting(JammingDetectionEventId, paJammingEvent);
}

//--------------------------------------------------------------------------------------------------
/**
 * The first-layer jamming detection indication handler.
 */
//--------------------------------------------------------------------------------------------------
static void FirstLayerJammingDetectionIndHandler
(
    void*   reportPtr,
    void*   secondLayerFunc
)
{
    if (NULL == reportPtr)
    {
        LE_ERROR("reportPtr is NULL");
        return;
    }

    if (NULL == secondLayerFunc)
    {
        LE_ERROR("secondLayerFunc is NULL");
        return;
    }
    LE_DEBUG("FirstLayerJammingDetectionIndHandler");

    pa_mrc_JammingDetectionIndication_t* jammingDetectionIndPtr =
                                   (pa_mrc_JammingDetectionIndication_t*) reportPtr;

    pa_mrc_JammingDetectionHandlerFunc_t handlerFunc =
                                   (pa_mrc_JammingDetectionHandlerFunc_t) secondLayerFunc;

    handlerFunc(jammingDetectionIndPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the jamming detection activation status
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrcSimu_SetJammingDetection
(
    pa_mrcSimu_JammingDetection_t activation    ///< [IN] Jamming activation state
)
{
    JammingActivationState = activation;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the jamming detection activation status
 *
 */
//--------------------------------------------------------------------------------------------------
pa_mrcSimu_JammingDetection_t pa_mrcSimu_GetJammingDetection
(
     void
)
{
    return JammingActivationState;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the power of the Radio Module.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetRadioPower
(
    le_onoff_t    power   ///< [IN] The power state.
)
{
    if(RadioPower == power)
    {
        return LE_OK;
    }

    switch(power)
    {
        case LE_ON:
        case LE_OFF:
            RadioPower = power;
            break;
        default:
            return LE_FAULT;
    }

    LE_INFO("Turning radio %s", (RadioPower == LE_ON) ? "On" : "Off");

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Radio Module power state.
 *
 * @return LE_FAULT  The function failed.
 * @return LE_OK     The function succeed.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetRadioPower
(
     le_onoff_t*    powerPtr   ///< [OUT] The power state.
)
{
    *powerPtr = RadioPower;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Radio Access Technology change handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_SetRatChangeHandler
(
    pa_mrc_RatChangeHdlrFunc_t handlerFuncPtr ///< [IN] The handler function.
)
{
    LE_ASSERT(handlerFuncPtr != NULL);

    return le_event_AddHandler(
                "RatChangeHandler",
                RatChangeEvent,
                (le_event_HandlerFunc_t) handlerFuncPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Radio Access Technology change
 * handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemoveRatChangeHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Network registration state handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddNetworkRegHandler
(
    pa_mrc_NetworkRegHdlrFunc_t regStateHandler ///< [IN] The handler function to handle the
                                                ///        Network registration state.
)
{
    LE_ASSERT(regStateHandler != NULL);

    return le_event_AddHandler(
                "NewRegStateHandler",
                NewRegStateEvent,
                (le_event_HandlerFunc_t) regStateHandler);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Network registration state handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_RemoveNetworkRegHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function configures the Network registration setting.
 *
 * @return LE_NOT_POSSIBLE  The function failed to configure the Network registration setting.
 * @return LE_BAD_PARAMETER The parameters are invalid.
 * @return LE_OUT_OF_RANGE  The parameters values are not in the allowed range.
 * @return LE_COMM_ERROR    The communication device has returned an error.
 * @return LE_TIMEOUT       No response was received from the Modem.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_ConfigureNetworkReg
(
    pa_mrc_NetworkRegSetting_t  setting ///< [IN] The selected Network registration setting.
)
{
    if(setting == PA_MRC_ENABLE_REG_NOTIFICATION)
    {
        return LE_OK;
    }

    return LE_NOT_POSSIBLE;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Network registration setting.
 *
 * @return LE_NOT_POSSIBLE The function failed to get the Network registration setting.
 * @return LE_COMM_ERROR   The communication device has returned an error.
 * @return LE_TIMEOUT      No response was received from the Modem.
 * @return LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetNetworkRegConfig
(
    pa_mrc_NetworkRegSetting_t*  settingPtr   ///< [OUT] The selected Network registration setting.
)
{
    *settingPtr = PA_MRC_ENABLE_REG_NOTIFICATION;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Network registration state.
 *
 * @return LE_NOT_POSSIBLE The function failed to get the Network registration state.
 * @return LE_COMM_ERROR   The communication device has returned an error.
 * @return LE_TIMEOUT      No response was received from the Modem.
 * @return LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetNetworkRegState
(
    le_mrc_NetRegState_t* statePtr  ///< [OUT] The network registration state.
)
{
    *statePtr = LE_MRC_REG_HOME;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the platform specific network registration error code.
 *
 * @return the platform specific registration error code
 *
 */
//--------------------------------------------------------------------------------------------------
int32_t pa_mrc_GetPlatformSpecificRegistrationErrorCode
(
    void
)
{
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Signal Strength information.
 *
 * @return LE_BAD_PARAMETER Bad parameter passed to the function
 * @return LE_OUT_OF_RANGE  The signal strength values are not known or not detectable.
 * @return LE_NOT_POSSIBLE  The function failed.
 * @return LE_TIMEOUT       No response was received.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetSignalStrength
(
    int32_t*          rssiPtr    ///< [OUT] The received signal strength (in dBm).
)
{
    if(RadioPower != LE_ON)
    {
        return LE_OUT_OF_RANGE;
    }

    *rssiPtr = -60;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the current network information.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the current network name can't fit in nameStr
 *      - LE_NOT_POSSIBLE on any other failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetCurrentNetwork
(
    char       *nameStr,               ///< [OUT] the home network Name
    size_t      nameStrSize,           ///< [IN]  the nameStr size
    char       *mccStr,                ///< [OUT] the mobile country code
    size_t      mccStrNumElements,     ///< [IN]  the mccStr size
    char       *mncStr,                ///< [OUT] the mobile network code
    size_t      mncStrNumElements      ///< [IN]  the mncStr size
)
{
    le_result_t res = LE_OK;

    if (RadioPower != LE_ON)
    {
        *nameStr = '\0';
        return LE_NOT_POSSIBLE;
    }

    if (nameStr != NULL)
    {
        res = le_utf8_Copy(nameStr, PA_SIMU_MRC_DEFAULT_NAME, nameStrSize, NULL);
        if (res != LE_OK)
        {
            return res;
        }
    }

    if (mccStr != NULL)
    {
        res = le_utf8_Copy(mccStr, CurentMccStr, mccStrNumElements, NULL);
        if (res != LE_OK)
        {
            return res;
        }
    }

    if (mncStr != NULL)
    {
        res = le_utf8_Copy(mncStr, CurentMncStr, mncStrNumElements, NULL);
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to delete the list of Scan Information
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_DeleteScanInformation
(
    le_dls_List_t *scanInformationListPtr ///< [IN] list of pa_mrc_ScanInformation_t
)
{
    pa_mrc_ScanInformation_t * nodePtr;
    le_dls_Link_t * linkPtr;

    while ((linkPtr = le_dls_Pop(scanInformationListPtr)) != NULL)
    {
        nodePtr = CONTAINER_OF(linkPtr, pa_mrc_ScanInformation_t, link);
        le_mem_Release(nodePtr);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to delete the list of pci Scan Information
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_DeletePciScanInformation
(
    le_dls_List_t *scanInformationListPtr ///< [IN] list of pa_mrc_ScanInformation_t
)
{
    pa_mrc_PciScanInformation_t* nodePtr;
    le_dls_Link_t *linkPtr;

    while ((linkPtr=le_dls_Pop(scanInformationListPtr)) != NULL)
    {
        nodePtr = CONTAINER_OF(linkPtr, pa_mrc_PciScanInformation_t, link);
        le_mem_Release(nodePtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to delete the list of Plmn Information
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_DeletePlmnScanInformation
(
    le_dls_List_t *scanInformationListPtr ///< [IN] list of pa_mrc_PlmnInformation_t
)
{
    pa_mrc_PlmnInformation_t* nodePtr;
    le_dls_Link_t *linkPtr;

    while ((linkPtr=le_dls_Pop(scanInformationListPtr)) != NULL)
    {
        nodePtr = CONTAINER_OF(linkPtr, pa_mrc_PlmnInformation_t, link);
        le_mem_Release(nodePtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to perform a network scan.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_TIMEOUT       No response was received.
 * @return LE_COMM_ERROR    Radio link failure occurred.
 * @return LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_PerformNetworkScan
(
    le_mrc_RatBitMask_t ratMask,               ///< [IN] The network mask
    pa_mrc_ScanType_t   scanType,              ///< [IN] the scan type
    le_dls_List_t      *scanInformationListPtr ///< [OUT] list of pa_mrc_ScanInformation_t
)
{
    if(RadioPower != LE_ON)
    {
        return LE_NOT_POSSIBLE;
    }
    if(scanType != PA_MRC_SCAN_PLMN || scanType != PA_MRC_SCAN_CSG || scanType != PA_MRC_SCAN_PCI)
    {
        LE_ERROR("ScanType is invalid");
        return LE_FAULT;
    }
    if(scanInformationListPtr == NULL)
    {
        LE_ERROR("Invalid list is given");
        return LE_FAULT;
    }
    if (ratMask & LE_MRC_BITMASK_RAT_GSM)
    {
        AppendNetworkScanResult(LE_MRC_RAT_GSM, scanInformationListPtr);
    }
    if (ratMask & LE_MRC_BITMASK_RAT_UMTS)
    {
        AppendNetworkScanResult(LE_MRC_RAT_UMTS, scanInformationListPtr);
    }
    if (ratMask & LE_MRC_BITMASK_RAT_LTE)
    {
        AppendNetworkScanResult(LE_MRC_RAT_LTE, scanInformationListPtr);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to perform a network scan.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the operator name would not fit in buffer
 *      - LE_NOT_POSSIBLE for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetScanInformationName
(
    pa_mrc_ScanInformation_t *scanInformationPtr,   ///< [IN] The scan information
    char *namePtr, ///< [OUT] Name of operator
    size_t nameSize ///< [IN] The size in bytes of the namePtr buffer
)
{
    if ((!scanInformationPtr) || (!namePtr))
    {
        return LE_NOT_POSSIBLE;
    }

    // @TODO Handle other names than default SIM MCC/MNC
    if ( (0 == strcmp(scanInformationPtr->mobileCode.mcc, PA_SIMU_SIM_DEFAULT_MCC)) &&
         (0 == strcmp(scanInformationPtr->mobileCode.mnc, PA_SIMU_SIM_DEFAULT_MNC)) )
    {
        return le_utf8_Copy(namePtr, PA_SIMU_MRC_DEFAULT_NAME, nameSize, NULL);
    }

    return LE_NOT_POSSIBLE;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the number of preferred operators present in the list.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_CountPreferredOperators
(
    bool      plmnStatic,   ///< [IN] Include Static preferred Operators.
    bool      plmnUser,     ///< [IN] Include Users preferred Operators.
    int32_t*  nbItemPtr     ///< [OUT] number of Preferred operator found if success.
)
{
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the current preferred operators.
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if Preferred operator list is not available
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetPreferredOperators
(
    pa_mrc_PreferredNetworkOperator_t*   preferredOperatorPtr,
                       ///< [IN/OUT] The preferred operators pointer.
    bool  plmnStatic,  ///< [IN] Include Static preferred Operators.
    bool  plmnUser,    ///< [IN] Include Users preferred Operators.
    int32_t* nbItemPtr ///< [IN/OUT] number of Preferred operator to find (in) and written (out).
)
{
    return LE_NOT_FOUND;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to apply the preferred list into the modem
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SavePreferredOperators
(
    le_dls_List_t      *PreferredOperatorsListPtr ///< [IN] List of preferred network operator
)
{
    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register on a mobile network [mcc;mnc]
 *
 * @return LE_NOT_POSSIBLE  The function failed to register.
 * @return LE_OK            The function succeeded to register,
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_RegisterNetwork
(
    const char *mccPtr,   ///< [IN] Mobile Country Code
    const char *mncPtr    ///< [IN] Mobile Network Code
)
{

    IsManual = true;

    le_utf8_Copy(CurentMccStr, mccPtr, sizeof(CurentMccStr), NULL);
    le_utf8_Copy(CurentMncStr, mncPtr, sizeof(CurentMncStr), NULL);

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register automatically on network
 *
 * @return
 *      - LE_OK             on success
 *      - LE_FAULT          for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetAutomaticNetworkRegistration
(
    void
)
{
    IsManual = false;

    le_utf8_Copy(CurentMccStr, PA_SIMU_MRC_DEFAULT_MCC, sizeof(CurentMccStr), NULL);
    le_utf8_Copy(CurentMncStr, PA_SIMU_MRC_DEFAULT_MNC, sizeof(CurentMncStr), NULL);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function set the current Radio Access Technology in use.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrcSimu_SetRadioAccessTechInUse
(
    le_mrc_Rat_t   rat  ///< [IN] The Radio Access Technology.
)
{
    Rat = rat;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function gets the Radio Access Technology.
 *
 * @return LE_FAULT The function failed to get the Signal Quality information.
 * @return LE_OK    The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetRadioAccessTechInUse
(
    le_mrc_Rat_t*   ratPtr    ///< [OUT] The Radio Access Technology.
)
{
    *ratPtr = Rat;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Radio Access Technology Preferences
 *
 * @return
 * - LE_OK              On success
 * - LE_FAULT           On failure
 * - LE_UNSUPPORTED     Not supported by platform
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetRatPreferences
(
    le_mrc_RatBitMask_t bitMask ///< [IN] A bit mask to set the Radio Access Technology preference.
)
{
    // TODO: implement this function
    return LE_UNSUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Rat Automatic Radio Access Technology Preference
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetAutomaticRatPreference
(
    void
)
{
    // TODO: implement this function
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Radio Access Technology Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetRatPreferences
(
    le_mrc_RatBitMask_t* ratMaskPtr ///< [OUT] A bit mask to get the Radio Access Technology
                                        ///<  preferences.
)
{
    // TODO: implement this function
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetBandPreferences
(
    le_mrc_BandBitMask_t bands ///< [IN] A bit mask to set the Band preferences.
)
{
    CurrentBand = bands;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetBandPreferences
(
    le_mrc_BandBitMask_t* bandsPtr ///< [OUT] A bit mask to get the Band preferences.
)
{
    *bandsPtr = CurrentBand;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Set the LTE Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetLteBandPreferences
(
    le_mrc_LteBandBitMask_t bands ///< [IN] A bit mask to set the LTE Band preferences.
)
{

    CurrentLteBand = bands;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the LTE Band Preferences
 *
 * @return
 * - LE_OK              on success
 * - LE_FAULT           on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetLteBandPreferences
(
    le_mrc_LteBandBitMask_t* bandsPtr ///< [OUT] A bit mask to get the LTE Band preferences.
)
{
    *bandsPtr = CurrentLteBand;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the TD-SCDMA Band Preferences
 *
 * @return
 * - LE_OK           On success
 * - LE_FAULT        On failure
 * - LE_UNSUPPORTED  The platform doesn't support setting TD-SCDMA Band preferences.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetTdScdmaBandPreferences
(
    le_mrc_TdScdmaBandBitMask_t bands ///< [IN] A bit mask to set the TD-SCDMA Band Preferences.
)
{
    CurrentTdScdmaBand = bands;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the TD-SCDMA Band Preferences
 *
 * @return
 * - LE_OK           On success
 * - LE_FAULT        On failure
 * - LE_UNSUPPORTED  The platform doesn't support getting TD-SCDMA Band preferences.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetTdScdmaBandPreferences
(
    le_mrc_TdScdmaBandBitMask_t* bandsPtr ///< [OUT] A bit mask to set the TD-SCDMA Band
                                          ///<  preferences.
)
{
    *bandsPtr = CurrentTdScdmaBand;
    return LE_OK;
}
//--------------------------------------------------------------------------------------------------
/**
 * This function retrieves the Neighboring Cells information.
 * Each cell information is queued in the list specified with the IN/OUT parameter.
 * Neither add nor remove of elements in the list can be done outside this function.
 *
 * @return LE_FAULT          The function failed to retrieve the Neighboring Cells information.
 * @return a positive value  The function succeeded. The number of cells which the information have
 *                           been retrieved.
 */
//--------------------------------------------------------------------------------------------------
int32_t pa_mrc_GetNeighborCellsInfo
(
    le_dls_List_t*   cellInfoListPtr    ///< [OUT] The Neighboring Cells information.
)
{
    // TODO: implement this function
    // No neighbours cells.
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to delete the list of neighboring cells information.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_DeleteNeighborCellsInfo
(
    le_dls_List_t *cellInfoListPtr ///< [IN] list of pa_mrc_CellInfo_t
)
{
    // TODO: implement this function
    return ;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get current registration mode
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetNetworkRegistrationMode
(
    bool*   isManualPtr,  ///< [OUT] true if the scan mode is manual, false if it is automatic.
    char*   mccPtr,       ///< [OUT] Mobile Country Code
    size_t  mccPtrSize,   ///< [IN] mccPtr buffer size
    char*   mncPtr,       ///< [OUT] Mobile Network Code
    size_t  mncPtrSize    ///< [IN] mncPtr buffer size
)
{

    le_utf8_Copy(mccPtr, CurentMccStr, mccPtrSize, NULL);
    le_utf8_Copy(mncPtr, CurentMncStr, mncPtrSize, NULL);

    *isManualPtr = IsManual;

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function measures the Signal metrics.
 *
 * @return LE_FAULT         The function failed.
 * @return LE_OK            The function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_MeasureSignalMetrics
(
    pa_mrc_SignalMetrics_t* metricsPtr    ///< [OUT] The signal metrics.
)
{
    // TODO: implement this function
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Signal Strength change handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddSignalStrengthIndHandler
(
    pa_mrc_SignalStrengthIndHdlrFunc_t ssIndHandler, ///< [IN] The handler function to handle the
                                                     ///        Signal Strength change indication.
    void*                              contextPtr    ///< [IN] The context to be given to the handler.
)
{
    // TODO: implement this function
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Signal Strength change handling.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemoveSignalStrengthIndHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    // TODO: implement this function
    return ;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set and activate the signal strength thresholds for signal
 * strength indications
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetSignalStrengthIndThresholds
(
    le_mrc_Rat_t rat,                 ///< Radio Access Technology
    int32_t      lowerRangeThreshold, ///< [IN] lower-range threshold in dBm
    int32_t      upperRangeThreshold  ///< [IN] upper-range strength threshold in dBm
)
{
    switch(rat)
    {
        case LE_MRC_RAT_GSM:
        case LE_MRC_RAT_UMTS:
        case LE_MRC_RAT_TDSCDMA:
        case LE_MRC_RAT_LTE:
        case LE_MRC_RAT_CDMA:
            break;

        case LE_MRC_RAT_UNKNOWN:
        default:
            LE_ERROR("Bad parameter!");
            return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set and activate the delta for signal strength indications.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 *  - LE_BAD_PARAMETER  Bad parameters.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetSignalStrengthIndDelta
(
    le_mrc_Rat_t rat,    ///< [IN] Radio Access Technology
    uint16_t     delta   ///< [IN] Signal delta in units of 0.1 dB
)
{
    if (!delta)
    {
        return LE_BAD_PARAMETER;
    }

    switch(rat)
    {
        case LE_MRC_RAT_GSM:
        case LE_MRC_RAT_UMTS:
        case LE_MRC_RAT_LTE:
        case LE_MRC_RAT_CDMA:
            break;
        case LE_MRC_RAT_TDSCDMA:
            if (delta < MIN_SIGNAL_DELTA_FOR_TDSCDMA)
            {
               return LE_BAD_PARAMETER;
            }
            break;
        case LE_MRC_RAT_UNKNOWN:
        default:
            LE_ERROR("Bad parameter!");
            return LE_FAULT;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the serving cell Identifier.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetServingCellId
(
    uint32_t* cellIdPtr ///< [OUT] main Cell Identifier.
)
{
    // TODO: implement this function
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Location Area Code of the serving cell.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetServingCellLocAreaCode
(
    uint32_t* lacPtr ///< [OUT] Location Area Code of the serving cell.
)
{
    // TODO: implement this function
    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to get the Tracking Area Code of the serving cell.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetServingCellLteTracAreaCode
(
    uint16_t* tacPtr ///< [OUT] Tracking Area Code of the serving cell.
)
{
    *tacPtr = 0xABCD;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Band capabilities
 *
 * @return
 *  - LE_OK              on success
 *  - LE_FAULT           on failure
 *  - LE_UNSUPPORTED     The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetBandCapabilities
(
    le_mrc_BandBitMask_t* bandsPtr ///< [OUT] A bit mask to get the Band capabilities.
)
{
    *bandsPtr = LE_MRC_BITMASK_BAND_CLASS_1_ALL_BLOCKS | LE_MRC_BITMASK_BAND_GSM_DCS_1800;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the LTE Band capabilities
 *
 * @return
 *  - LE_OK              on success
 *  - LE_FAULT           on failure
 *  - LE_UNSUPPORTED     The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetLteBandCapabilities
(
    le_mrc_LteBandBitMask_t* bandsPtr ///< [OUT] Bit mask to get the LTE Band capabilities.
)
{
    *bandsPtr = LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_3 | LE_MRC_BITMASK_LTE_BAND_E_UTRA_OP_BAND_7;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the TD-SCDMA Band capabilities
 *
 * @return
 *  - LE_OK              on success
 *  - LE_FAULT           on failure
 *  - LE_UNSUPPORTED     The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetTdScdmaBandCapabilities
(
    le_mrc_TdScdmaBandBitMask_t* bandsPtr ///< [OUT] Bit mask to get the TD-SCDMA Band capabilities.
)
{
    *bandsPtr = LE_MRC_BITMASK_TDSCDMA_BAND_A | LE_MRC_BITMASK_TDSCDMA_BAND_C;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Packet Switched state.
 *
 * @return
 *  - LE_FAULT  Function failed.
 *  - LE_OK     Function succeeded.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetPacketSwitchedState
(
    le_mrc_NetRegState_t* statePtr  ///< [OUT] The current Packet switched state.
)
{
    if (statePtr)
    {
        *statePtr = LE_MRC_REG_HOME;
        return LE_OK;
    }

    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler for Packet Switched change handling.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_SetPSChangeHandler
(
    pa_mrc_ServiceChangeHdlrFunc_t handlerFuncPtr ///< [IN] The handler function.
)
{
    LE_ASSERT(handlerFuncPtr != NULL);

    return le_event_AddHandler("PSChangeHandler",
                               PSChangeEventId,
                               (le_event_HandlerFunc_t) handlerFuncPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Packet Switched change
 * handling.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemovePSChangeHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
}


/**
 * MRC Stub initialization.
 *
 * @return LE_OK           The function succeeded.
 */
le_result_t mrc_simu_Init
(
    void
)
{
    LE_INFO("PA MRC Init");

    NewRegStateEvent = le_event_CreateIdWithRefCounting("NewRegStateEvent");
    RatChangeEvent = le_event_CreateIdWithRefCounting("RatChangeEvent");

    PSChangeEventId = le_event_CreateIdWithRefCounting("PSChangeEvent");
    PSChangePool = le_mem_CreatePool("PSChangePool", sizeof(le_mrc_NetRegState_t));

    ScanInformationPool = le_mem_CreatePool("ScanInformationPool",
                                            sizeof(pa_mrc_ScanInformation_t));

    JammingDetectionIndPool = le_mem_CreatePool("JammingDetectionIndPool",
                                                  sizeof(pa_mrc_JammingDetectionIndication_t));

    JammingDetectionEventId = le_event_CreateIdWithRefCounting("JammingDetectionInd");

    return LE_OK;
}

bool mrc_simu_IsOnline
(
    void
)
{
    le_mrc_NetRegState_t state;
    le_result_t res;

    res = pa_mrc_GetNetworkRegState(&state);
    if(res != LE_OK)
    {
        return false;
    }

    return ( (state == LE_MRC_REG_HOME) || (state == LE_MRC_REG_ROAMING) );
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler to report network reject code.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddNetworkRejectIndHandler
(
    pa_mrc_NetworkRejectIndHdlrFunc_t networkRejectIndHandler, ///< [IN] The handler function to
                                                               ///< report network reject
                                                               ///< indication.
    void*                             contextPtr               ///< [IN] The context to be given to
                                                               ///< the handler.
)
{
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to unregister the handler for Network Reject Indication handling.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mrc_RemoveNetworkRejectIndHandler
(
    le_event_HandlerRef_t handlerRef
)
{
    le_event_RemoveHandler(handlerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * This function activates or deactivates jamming detection notification.
 *
 * * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 *      - LE_DUPLICATE if jamming detection is already activated and an activation is requested
 *      - LE_UNSUPPORTED if jamming detection is not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetJammingDetection
(
    bool activation     ///< [IN] Notification activation request
)
{
    le_result_t res;
    switch(JammingActivationState)
    {
        case PA_MRCSIMU_JAMMING_UNSUPPORTED:
            res = LE_UNSUPPORTED;
            break;

        case PA_MRCSIMU_JAMMING_ACTIVATED:
            if (activation)
            {
                res = LE_DUPLICATE;
            }
            else
            {
                // Deactivate Jamming Detection
                pa_mrcSimu_SetJammingDetection(PA_MRCSIMU_JAMMING_DEACTIVATED);
                res = LE_OK;
            }
            break;

        case PA_MRCSIMU_JAMMING_DEACTIVATED:
            if (activation)
            {
                // Activate Jamming Detection
                pa_mrcSimu_SetJammingDetection(PA_MRCSIMU_JAMMING_ACTIVATED);
            }
            res = LE_OK;
            break;

        default:
            res = LE_FAULT;
            break;
    }
    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function returns the jamming detection notification status.
 *
 * * @return
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER if the parameter is invalid
 *      - LE_FAULT on failure
 *      - LE_UNSUPPORTED if jamming detection is not supported or if this request is not supported
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetJammingDetection
(
    bool* activationPtr     ///< [IN] Notification activation request
)
{
    if (!activationPtr)
    {
        return LE_BAD_PARAMETER;
    }

    switch(JammingActivationState)
    {
        case PA_MRCSIMU_JAMMING_UNSUPPORTED:
            return LE_UNSUPPORTED;

        case PA_MRCSIMU_JAMMING_ACTIVATED:
            *activationPtr = true;
            return LE_OK;

        case PA_MRCSIMU_JAMMING_DEACTIVATED:
            *activationPtr = false;
            return LE_OK;

        default:
            return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the SAR backoff state
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_UNSUPPORTED    The feature is not supported.
 *  - LE_OUT_OF_RANGE   The provided index is out of range.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_SetSarBackoffState
(
    uint8_t state
)
{
    if (state > SAR_BACKOFF_STATE_MAX)
    {
        return LE_OUT_OF_RANGE;
    }

    SarBackoffStatus = state;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the SAR backoff state
 *
 * @return
 *  - LE_OK             The function succeeded.
 *  - LE_FAULT          The function failed.
 *  - LE_UNSUPPORTED    The feature is not supported.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mrc_GetSarBackoffState
(
    uint8_t* statePtr
)
{
    if (!statePtr)
    {
        return LE_FAULT;
    }

    *statePtr = SarBackoffStatus;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to register a handler to report jamming detection notification.
 *
 * @return A handler reference, which is only needed for later removal of the handler.
 *
 * @note Doesn't return on failure, so there's no need to check the return value for errors.
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mrc_AddJammingDetectionIndHandler
(
    pa_mrc_JammingDetectionHandlerFunc_t handler,   ///< [IN] The handler function to handle jamming
                                                    ///  detection indication.
    void*                               contextPtr  ///< [IN] The context to be given to the
                                                    ///  handler.
)
{
    if (NULL != handler)
    {
        le_event_HandlerRef_t handlerRef = le_event_AddLayeredHandler(
                                           "JammingDetectionIndHandler",
                                           JammingDetectionEventId,
                                           FirstLayerJammingDetectionIndHandler,
                                           handler);

        le_event_SetContextPtr(handlerRef, contextPtr);

        return handlerRef;
    }
    else
    {
        return NULL;
    }
}
