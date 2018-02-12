/**
 * @file pa_mdc_simu.c
 *
 * Simulation implementation of @ref c_pa_mdc API.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */
#include "legato.h"
#include "pa_mdc.h"
#include "interfaces.h"
#include "pa_mdc_simu.h"
#include "pa_mrc.h"
#include <netinet/in.h>

typedef struct
{
    uint32_t profileIndex;
    pa_mdc_ProfileData_t profileData;
    struct MdcSimuProfile_t* nextPtr;
    /* session */
    bool sessionStarted[LE_MDC_PDP_IPV4V6];
    /* gateway */
    char  gatewayAddrStr[LE_MDMDEFS_IPMAX][LE_MDC_IPV6_ADDR_MAX_BYTES];
    /* ip addr */
    char  ipAddrStr[LE_MDMDEFS_IPMAX][LE_MDC_IPV6_ADDR_MAX_BYTES];
    /* interface name */
    char interfaceName[LE_MDC_INTERFACE_NAME_MAX_BYTES];
    /* dns */
    char dns1AddrStr[LE_MDMDEFS_IPMAX][LE_MDC_IPV6_ADDR_MAX_BYTES];
    char dns2AddrStr[LE_MDMDEFS_IPMAX][LE_MDC_IPV6_ADDR_MAX_BYTES];
} MdcSimuProfile_t;

static MdcSimuProfile_t* MdcSimuProfile;
static pa_mdc_SessionStateHandler_t SessionStateHandler;
static le_mem_PoolRef_t NewSessionStatePool , MdcSimuProfileDataPool;
static pa_mdc_PktStatistics_t DataStatistics;

#define LE_MDMDEFS_IPVERSION_2_LE_MDC_PDP(X) ((X == LE_MDMDEFS_IPV4) ? LE_MDC_PDP_IPV4:\
                                              ((X == LE_MDMDEFS_IPV6) ? LE_MDC_PDP_IPV6:\
                                              LE_MDC_PDP_UNKNOWN))

//--------------------------------------------------------------------------------------------------
/**
 * Get a profile context
 *
 **/
//--------------------------------------------------------------------------------------------------
static MdcSimuProfile_t* GetProfile
(
    uint32_t profileIndex
)
{
    MdcSimuProfile_t *profileTmpPtr = MdcSimuProfile;

    while ( profileTmpPtr && (profileTmpPtr->profileIndex != profileIndex) )
    {
        profileTmpPtr = (MdcSimuProfile_t*) profileTmpPtr->nextPtr;
    }

    return profileTmpPtr;
}



//--------------------------------------------------------------------------------------------------
/**
 * Start a session
 *
 **/
//--------------------------------------------------------------------------------------------------
static le_result_t StartSession
(
    uint32_t profileIndex,
    le_mdc_Pdp_t pdp
)
{
    MdcSimuProfile_t *profileTmpPtr = GetProfile(profileIndex);

    if (!profileTmpPtr || (profileTmpPtr->profileData.pdp != pdp))
    {
        return LE_FAULT;
    }

    if ( profileTmpPtr->sessionStarted[LE_MDC_PDP_IPV4] ||
        profileTmpPtr->sessionStarted[LE_MDC_PDP_IPV6] )
    {
        return LE_DUPLICATE;
    }

    switch (pdp)
    {
        case LE_MDC_PDP_IPV4:
        case LE_MDC_PDP_IPV6:
            profileTmpPtr->sessionStarted[pdp] = true;
        break;
        case LE_MDC_PDP_IPV4V6:
            profileTmpPtr->sessionStarted[LE_MDC_PDP_IPV4] =
            profileTmpPtr->sessionStarted[LE_MDC_PDP_IPV6] = true;
        break;
        default:
        break;
    }

    /* send the handler */
    if (SessionStateHandler)
    {
        pa_mdc_SessionStateData_t* sessionStateDataPtr = le_mem_ForceAlloc(NewSessionStatePool);
        sessionStateDataPtr->profileIndex = profileIndex;
        sessionStateDataPtr->newState = LE_MDC_CONNECTED;
        SessionStateHandler(sessionStateDataPtr);
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
// APIs.
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Set the gateway IP address for the given profile.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdcSimu_SetGatewayAddress
(
    uint32_t profileIndex,                 ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,      ///< [IN] IP Version
    const char* gatewayAddrStr             ///< [IN] The gateway IP address in dotted format
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return;
    }

    le_utf8_Copy(profilePtr->gatewayAddrStr[ipVersion], gatewayAddrStr, LE_MDC_IPV6_ADDR_MAX_BYTES, NULL);
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the gateway IP address for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the IP address would not fit in gatewayAddrStr
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetGatewayAddress
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,               ///< [IN] IP Version
    char*  gatewayAddrStr,                  ///< [OUT] The gateway IP address in dotted format
    size_t gatewayAddrStrSize               ///< [IN] The size in bytes of the address buffer
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return LE_FAULT;
    }

    if (!profilePtr->sessionStarted[LE_MDMDEFS_IPVERSION_2_LE_MDC_PDP(ipVersion)])
    {
        return LE_FAULT;
    }

    if (gatewayAddrStrSize >= strlen(profilePtr->gatewayAddrStr[ipVersion]))
    {
        strncpy(gatewayAddrStr, profilePtr->gatewayAddrStr[ipVersion], gatewayAddrStrSize);
        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get session type for the given profile ( IP V4 or V6 )
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetSessionType
(
    uint32_t profileIndex,              ///< [IN] The profile to use
    pa_mdc_SessionType_t* sessionIpPtr  ///< [OUT] IP family session
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return LE_FAULT;
    }

    if (profilePtr->sessionStarted[LE_MDC_PDP_IPV4] &&
        profilePtr->sessionStarted[LE_MDC_PDP_IPV6])
    {
        *sessionIpPtr = PA_MDC_SESSION_IPV4V6;
        return LE_OK;
    }
    else if (profilePtr->sessionStarted[LE_MDC_PDP_IPV4])
    {
        *sessionIpPtr = PA_MDC_SESSION_IPV4;
        return LE_OK;
    }
    else if (profilePtr->sessionStarted[LE_MDC_PDP_IPV6])
    {
        *sessionIpPtr = PA_MDC_SESSION_IPV6;
        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the name of the network interface for the given profile
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdcSimu_SetInterfaceName
(
    uint32_t profileIndex,
    const char* interfaceNameStr
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    le_utf8_Copy(profilePtr->interfaceName, interfaceNameStr, LE_MDC_INTERFACE_NAME_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the name of the network interface for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the interface name would not fit in interfaceNameStr
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetInterfaceName
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    char*  interfaceNameStr,                ///< [OUT] The name of the network interface
    size_t interfaceNameStrSize             ///< [IN] The size in bytes of the name buffer
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return LE_FAULT;
    }

    if ((profilePtr->sessionStarted[LE_MDC_PDP_IPV4] ||
        profilePtr->sessionStarted[LE_MDC_PDP_IPV6]) &&
        (interfaceNameStrSize >= strlen(profilePtr->interfaceName)))
    {
        strncpy(interfaceNameStr, profilePtr->interfaceName, interfaceNameStrSize);

        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check whether the profile already exists on the modem ; if not, ask to the modem to create a new
 * profile.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_InitializeProfile
(
    uint32_t   profileIndex     ///< [IN] The profile to write
)
{
    MdcSimuProfile_t *profileTmpPtr = MdcSimuProfile;

    while ( profileTmpPtr && (profileTmpPtr->profileIndex != profileIndex) )
    {
        profileTmpPtr = (MdcSimuProfile_t*) profileTmpPtr->nextPtr;
    }

    if (profileTmpPtr && (profileTmpPtr->profileIndex == profileIndex))
    {
        return LE_OK;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Write the profile data for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_WriteProfile
(
    uint32_t profileIndex,                    ///< [IN] The profile to write
    pa_mdc_ProfileData_t* profileDataPtr    ///< [IN] The profile data
)
{
    pa_mdcSimu_SetProfile(profileIndex, profileDataPtr);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the connection failure reason
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdc_GetConnectionFailureReason
(
    uint32_t profileIndex,              ///< [IN] The profile to use
    pa_mdc_ConnectionFailureCode_t* failureCodesPtr  ///< [OUT] The specific Failure Reason codes
)
{
    memset(failureCodesPtr, 0, sizeof(pa_mdc_ConnectionFailureCode_t));
    failureCodesPtr->callEndFailure = LE_MDC_DISC_UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the index of the default profile (link to the platform)
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDefaultProfileIndex
(
    uint32_t* profileIndexPtr
)
{
    le_mrc_Rat_t   rat=LE_MRC_RAT_GSM;
    le_result_t res;

    res = pa_mrc_GetRadioAccessTechInUse(&rat);

    if (rat==LE_MRC_RAT_GSM)
    {
        *profileIndexPtr = PA_MDC_MIN_INDEX_3GPP_PROFILE;
    }
    else
    {
        *profileIndexPtr = PA_MDC_MIN_INDEX_3GPP2_PROFILE;
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the index of the default profile for Bearer Independent Protocol (link to the platform)
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetBipDefaultProfileIndex
(
    uint32_t* profileIndexPtr   ///< [OUT] index of the profile.
)
{
    // TODO: implement this function
    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the profile data
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdcSimu_SetProfile
(
    uint32_t profileIndex,                        ///< [IN] The profile to read
    const pa_mdc_ProfileData_t* profileDataPtr    ///< [OUT] The profile data
)
{
    MdcSimuProfile_t *profileTmpPtr = MdcSimuProfile, *profilePrevPtr = NULL;

    /* Looking for profileIndex in the MdcSimuProfile chain */
    while ( profileTmpPtr && (profileTmpPtr->profileIndex != profileIndex) )
    {
        profilePrevPtr = profileTmpPtr;
        profileTmpPtr = (MdcSimuProfile_t *) profileTmpPtr->nextPtr;
    }

    if (!profileTmpPtr)
    {
        /* profileIndex has not been found, allocating new profile */
        MdcSimuProfile_t *profilePtr = le_mem_ForceAlloc(MdcSimuProfileDataPool);
        memset(profilePtr,0,sizeof(MdcSimuProfile_t));
        profilePtr->profileIndex = profileIndex;
        profilePtr->profileData = *profileDataPtr;

        if (profilePrevPtr)
        {
            profilePrevPtr->nextPtr = (struct MdcSimuProfile_t *) profilePtr;
        }
        else
        {
            MdcSimuProfile = profilePtr;
        }
    }
    else
    {
        /* profileIndex has been found, setting data */
        profileTmpPtr->profileData = *profileDataPtr;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Free all profiles
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdcSimu_DeleteProfiles
(
    void
)
{
    MdcSimuProfile_t *profileTmpPtr = MdcSimuProfile, *profilePrevPtr = NULL;

    while ( profileTmpPtr )
    {
        profilePrevPtr = profileTmpPtr;
        profileTmpPtr = (MdcSimuProfile_t *) profileTmpPtr->nextPtr;

        free(profilePrevPtr);
    }

    MdcSimuProfile = NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Read the profile data for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on failure
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_ReadProfile
(
    uint32_t profileIndex,                  ///< [IN] The profile to read
    pa_mdc_ProfileData_t* profileDataPtr    ///< [OUT] The profile data
)
{
    MdcSimuProfile_t *profileTmpPtr = MdcSimuProfile;

    while ( profileTmpPtr && (profileTmpPtr->profileIndex != profileIndex) )
    {
        profileTmpPtr = (MdcSimuProfile_t *) profileTmpPtr->nextPtr;
    }

    if ( profileTmpPtr )
    {
        *profileDataPtr = profileTmpPtr->profileData;
        return LE_OK;
    }
    else
    {
        return LE_FAULT;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Register a handler for session state notifications.
 *
 * If the handler is NULL, then the previous handler will be removed.
 *
 * @note
 *      The process exits on failure
 */
//--------------------------------------------------------------------------------------------------
le_event_HandlerRef_t pa_mdc_AddSessionStateHandler
(
    pa_mdc_SessionStateHandler_t handlerRef, ///< [IN] The session state handler function.
    void*                        contextPtr  ///< [IN] The context to be given to the handler.
)
{
    SessionStateHandler = handlerRef;
    return (le_event_HandlerRef_t) SessionStateHandler;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the IP address for the given profile.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdcSimu_SetIPAddress
(
    uint32_t profileIndex,
    le_mdmDefs_IpVersion_t ipVersion,
    const char* ipAddrStr
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return;
    }

    le_utf8_Copy(profilePtr->ipAddrStr[ipVersion], ipAddrStr, LE_MDC_IPV6_ADDR_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the IP address for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the IP address would not fit in gatewayAddrStr
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetIPAddress
(
    uint32_t profileIndex,             ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,          ///< [IN] IP Version
    char*  ipAddrStr,                  ///< [OUT] The IP address in dotted format
    size_t ipAddrStrSize               ///< [IN] The size in bytes of the address buffer
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return LE_FAULT;
    }

    if ( !profilePtr->sessionStarted[LE_MDMDEFS_IPVERSION_2_LE_MDC_PDP(ipVersion)] )
    {
        return LE_FAULT;
    }

    if (ipAddrStrSize >= strlen(profilePtr->ipAddrStr[ipVersion]) )
    {
        le_utf8_Copy(ipAddrStr, profilePtr->ipAddrStr[ipVersion], ipAddrStrSize, NULL);

        return LE_OK;
    }

    return LE_FAULT;

}

//--------------------------------------------------------------------------------------------------
/**
 * Get the session state for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT on error
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetSessionState
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    le_mdc_ConState_t*   sessionStatePtr    ///< [OUT] Data session state
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return LE_FAULT;
    }

    if (profilePtr->sessionStarted[LE_MDC_PDP_IPV4] || profilePtr->sessionStarted[LE_MDC_PDP_IPV6])
    {
        *sessionStatePtr = LE_MDC_CONNECTED;
    }
    else
    {
        *sessionStatePtr = LE_MDC_DISCONNECTED;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Data Bearer Technology for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDataBearerTechnology
(
    uint32_t                       profileIndex,              ///< [IN] The profile to use
    le_mdc_DataBearerTechnology_t* downlinkDataBearerTechPtr, ///< [OUT] downlink data bearer technology
    le_mdc_DataBearerTechnology_t* uplinkDataBearerTechPtr    ///< [OUT] uplink data bearer technology
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start a data session with the given profile using IPV4
 *
 * @return
 *      - LE_OK on success
 *      - LE_DUPLICATE if the data session is already connected
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartSessionIPV4
(
    uint32_t profileIndex        ///< [IN] The profile to use
)
{
    return StartSession(profileIndex, LE_MDC_PDP_IPV4);
}

//--------------------------------------------------------------------------------------------------
/**
 * Start a data session with the given profile using IPV6
 *
 * @return
 *      - LE_OK on success
 *      - LE_DUPLICATE if the data session is already connected
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartSessionIPV6
(
    uint32_t profileIndex       ///< [IN] The profile to use
)
{
    return StartSession(profileIndex, LE_MDC_PDP_IPV6);
}

//--------------------------------------------------------------------------------------------------
/**
 * Start a data session with the given profile using IPV4-V6
 *
 * @return
 *      - LE_OK on success
 *      - LE_DUPLICATE if the data session is already connected
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartSessionIPV4V6
(
    uint32_t profileIndex        ///< [IN] The profile to use
)
{

    return StartSession(profileIndex, LE_MDC_PDP_IPV4V6);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set data flow statistics.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdcSimu_SetDataFlowStatistics
(
    const pa_mdc_PktStatistics_t *dataStatisticsPtr ///< [OUT] Statistics data
)
{
    DataStatistics = *dataStatisticsPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get data flow statistics since the last reset.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDataFlowStatistics
(
    pa_mdc_PktStatistics_t *dataStatisticsPtr ///< [OUT] Statistics data
)
{
    *dataStatisticsPtr = DataStatistics;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Reset data flow statistics
 *
 * * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_ResetDataFlowStatistics
(
    void
)
{
    memset(&DataStatistics, 0, sizeof(pa_mdc_PktStatistics_t));
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop collecting data flow statistics
 *
 * * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StopDataFlowStatistics
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start collecting data flow statistics
 *
 * * @return
 *      - LE_OK on success
 *      - LE_FAULT for all other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StartDataFlowStatistics
(
    void
)
{
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Stop a data session for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER if the input parameter is not valid
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_StopSession
(
    uint32_t profileIndex       ///< [IN] The profile to use
)
{
    MdcSimuProfile_t* profilePtr = GetProfile(profileIndex);

    if (!profilePtr->sessionStarted[LE_MDC_PDP_IPV4] && !profilePtr->sessionStarted[LE_MDC_PDP_IPV6])
    {
        return LE_FAULT;
    }
    else
    {
        profilePtr->sessionStarted[LE_MDC_PDP_IPV4] = profilePtr->sessionStarted[LE_MDC_PDP_IPV6]
                                                                                            = false;

        /* send the handler */
        if (SessionStateHandler)
        {
            pa_mdc_SessionStateData_t* sessionStateDataPtr = le_mem_ForceAlloc(NewSessionStatePool);
            sessionStateDataPtr->profileIndex = profilePtr->profileIndex;
            sessionStateDataPtr->newState = LE_MDC_DISCONNECTED;
            sessionStateDataPtr->disc = LE_MDC_DISC_REGULAR_DEACTIVATION;
            sessionStateDataPtr->discCode = 2;
            SessionStateHandler(sessionStateDataPtr);
        }
        return LE_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Reject a MT-PDP data session for the given profile
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED if not supported by the target
 *      - LE_FAULT for other failures
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_RejectMtPdpSession
(
    uint32_t profileIndex
)
{
    MdcSimuProfile_t* profilePtr = GetProfile(profileIndex);

    if (profilePtr->sessionStarted[LE_MDC_PDP_IPV4] || profilePtr->sessionStarted[LE_MDC_PDP_IPV6])
    {
        return LE_FAULT;
    }
    else
    {
        /* send the handler */
        if (SessionStateHandler)
        {
            pa_mdc_SessionStateData_t* sessionStateDataPtr = le_mem_ForceAlloc(NewSessionStatePool);
            sessionStateDataPtr->profileIndex = profilePtr->profileIndex;
            sessionStateDataPtr->newState = LE_MDC_SUSPENDING;
            sessionStateDataPtr->disc = LE_MDC_DISC_NO_SERVICE;
            sessionStateDataPtr->discCode = 0;
            SessionStateHandler(sessionStateDataPtr);
        }
        return LE_OK;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Set the primary/secondary DNS addresses for the given profile
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_mdcSimu_SetDNSAddresses
(
    uint32_t profileIndex,
    le_mdmDefs_IpVersion_t ipVersion,
    const char* dns1AddrStr,
    const char* dns2AddrStr
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    le_utf8_Copy(profilePtr->dns1AddrStr[ipVersion], dns1AddrStr, LE_MDC_IPV6_ADDR_MAX_BYTES, NULL);
    le_utf8_Copy(profilePtr->dns2AddrStr[ipVersion], dns2AddrStr, LE_MDC_IPV6_ADDR_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the primary/secondary DNS addresses for the given profile, if the data session is connected.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if the IP address would not fit in buffer
 *      - LE_FAULT for all other errors
 *
 * @note
 *      If only one DNS address is available, then it will be returned, and an empty string will
 *      be returned for the unavailable address
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_GetDNSAddresses
(
    uint32_t profileIndex,                  ///< [IN] The profile to use
    le_mdmDefs_IpVersion_t ipVersion,               ///< [IN] IP Version
    char*  dns1AddrStr,                     ///< [OUT] The primary DNS IP address in dotted format
    size_t dns1AddrStrSize,                 ///< [IN] The size in bytes of the dns1AddrStr buffer
    char*  dns2AddrStr,                     ///< [OUT] The secondary DNS IP address in dotted format
    size_t dns2AddrStrSize                  ///< [IN] The size in bytes of the dns2AddrStr buffer
)
{
    MdcSimuProfile_t *profilePtr = GetProfile(profileIndex);

    if (!profilePtr)
    {
        return LE_FAULT;
    }

    if (ipVersion == LE_MDMDEFS_IPV4)
    {
        if ( dns1AddrStrSize < INET_ADDRSTRLEN)
        {
               return LE_OVERFLOW;
        }
        if ( dns2AddrStrSize < INET_ADDRSTRLEN)
        {
               return LE_OVERFLOW;
        }
    }

    if (ipVersion == LE_MDMDEFS_IPV6)
    {
        if ( dns1AddrStrSize < LE_MDC_IPV6_ADDR_MAX_BYTES)
        {
                return LE_OVERFLOW;
        }
        if ( dns2AddrStrSize < LE_MDC_IPV6_ADDR_MAX_BYTES)
        {
               return LE_OVERFLOW;
        }
    }

    if ( profilePtr->sessionStarted[LE_MDMDEFS_IPVERSION_2_LE_MDC_PDP(ipVersion)] &&
        ( dns1AddrStrSize >= strlen(profilePtr->dns1AddrStr[ipVersion]) ) &&
        ( dns2AddrStrSize >= strlen(profilePtr->dns2AddrStr[ipVersion]) ))
    {
        le_utf8_Copy(dns1AddrStr, profilePtr->dns1AddrStr[ipVersion], dns1AddrStrSize, NULL);
        le_utf8_Copy(dns2AddrStr, profilePtr->dns2AddrStr[ipVersion], dns2AddrStrSize, NULL);

        return LE_OK;
    }

    return LE_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Provide a default data profile for 3GPP.
 *
 **/
//--------------------------------------------------------------------------------------------------
static void ProvideDefaultProfile
(
    void
)
{
    pa_mdc_ProfileData_t profileData;
    int profileIndex = PA_MDC_MIN_INDEX_3GPP_PROFILE;
    memset(&profileData, 0, sizeof(pa_mdc_ProfileData_t));
    le_utf8_Copy(profileData.apn, "apn", sizeof(profileData.apn), NULL);
    profileData.authentication.type = LE_MDC_AUTH_NONE;
    profileData.pdp = LE_MDC_PDP_IPV4;
    pa_mdcSimu_SetProfile(profileIndex, &profileData);
    pa_mdcSimu_SetInterfaceName(profileIndex, "eth0");

    // This case is only valid for OPT_NET="user" in legato-qemu
    // TODO: Detect IP automatically or use OPT_NET2 to have a dedicated network
    char ipAddrStrIpv4[] = "192.168.76.15";
    char gatewayAddrStrIpv4[] = "192.168.76.2";
    char dns1AddrIpv4[] = "192.168.76.3";
    char dns2AddrIpv4[] = "";
    pa_mdcSimu_SetIPAddress(profileIndex, LE_MDMDEFS_IPV4, ipAddrStrIpv4);
    pa_mdcSimu_SetGatewayAddress(profileIndex, LE_MDMDEFS_IPV4, gatewayAddrStrIpv4);
    pa_mdcSimu_SetDNSAddresses(profileIndex, LE_MDMDEFS_IPV4, dns1AddrIpv4, dns2AddrIpv4);
}

//--------------------------------------------------------------------------------------------------
/**
 * simu init
 *
 **/
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdcSimu_Init
(
    void
)
{
    NewSessionStatePool = le_mem_CreatePool("NewSessionStatePool", sizeof(pa_mdc_SessionStateData_t));
    MdcSimuProfileDataPool = le_mem_CreatePool("MdcSimuProfileDataPool", sizeof(MdcSimuProfile_t));

    ProvideDefaultProfile();

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Map a profile on a network interface
 *
 * * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED if not supported by the target
 *      - LE_FAULT for all other errors
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_mdc_MapProfileOnNetworkInterface
(
    uint32_t         profileIndex,         ///< [IN] The profile to use
    const char*      interfaceNamePtr      ///< [IN] Network interface name
)
{
    return LE_OK;
}
