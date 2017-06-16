/**
 * @file pa_info_simu.c
 *
 * Simulation implementation of @ref c_pa_info API.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "pa_info.h"
#include "interfaces.h"
#include "pa_simu.h"
#include "pa_info_simu.h"
#include "simuConfig.h"

#include <sys/utsname.h>

//--------------------------------------------------------------------------------------------------
/**
 * Info parameters
 */
//--------------------------------------------------------------------------------------------------
static pa_info_Imei_t Imei = PA_SIMU_INFO_DEFAULT_IMEI;
static pa_info_ImeiSv_t ImeiSv = PA_SIMU_INFO_DEFAULT_IMEISV;
static pa_info_DeviceModel_t DeviceModel = PA_SIMU_INFO_DEFAULT_DEVICE_MODEL;
static char FirmwareVersion[PA_INFO_VERS_MAX_BYTES] = PA_SIMU_INFO_DEFAULT_FW_VERSION;
static char BootLoaderVersion[PA_INFO_VERS_MAX_BYTES] = PA_SIMU_INFO_DEFAULT_BOOT_VERSION;
static char Meid[LE_INFO_MAX_MEID_BYTES] = PA_SIMU_INFO_DEFAULT_MEID;
static char Esn[LE_INFO_MAX_ESN_BYTES] = PA_SIMU_INFO_DEFAULT_ESN;
static char Min[LE_INFO_MAX_MIN_BYTES] = PA_SIMU_INFO_DEFAULT_MIN;
static uint16_t Prl = PA_SIMU_INFO_DEFAULT_PRL;
static bool PrlFlag = false;
static char Nai[LE_INFO_MAX_NAI_BYTES] = PA_SIMU_INFO_DEFAULT_NAI;
static char MfrName[LE_INFO_MAX_MFR_NAME_BYTES] = PA_SIMU_INFO_DEFAULT_MFR;
static char PriIdPn[LE_INFO_MAX_PRIID_PN_BYTES] = PA_SIMU_INFO_DEFAULT_PRIID_PN;
static char PriIdRev[LE_INFO_MAX_PRIID_REV_BYTES] = PA_SIMU_INFO_DEFAULT_PRIID_REV;
static char Sku[LE_INFO_MAX_SKU_BYTES] = PA_SIMU_INFO_DEFAULT_SKU;
static char Psn[LE_INFO_MAX_PSN_BYTES] = PA_SIMU_INFO_DEFAULT_PSN;
static char CapriName[LE_INFO_MAX_CAPRI_NAME_BYTES] = PA_SIMU_INFO_DEFAULT_CAPRI_NAME;
static char CapriRev[LE_INFO_MAX_CAPRI_REV_BYTES] = PA_SIMU_INFO_DEFAULT_CAPRI_REV;
static char ResetReasonStr[LE_INFO_MAX_RESET_BYTES] = "";
static le_info_Reset_t ResetInformation = LE_INFO_RESET_UNKNOWN;


//--------------------------------------------------------------------------------------------------
/**
 * Rf device status
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    bool isValid;
    uint16_t manufacturedId;
    uint8_t productId;
    bool status;
}
RfDeviceStatus_t;

static RfDeviceStatus_t RfDeviceStatus[LE_INFO_RF_DEVICES_STATUS_MAX] = {{0}};

//--------------------------------------------------------------------------------------------------
/**
 * simu return values
 */
//--------------------------------------------------------------------------------------------------
static le_result_t SimuRes;
static bool        ApplySimuErrorCode;

//--------------------------------------------------------------------------------------------------
/**
 * Definition of settings that are settable through simuConfig.
 */
//--------------------------------------------------------------------------------------------------
static const simuConfig_Property_t ConfigProperties[] = {
    { .name = "psn",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_infoSimu_SetPlatformSerialNumber } } },
    { .name = "imei",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_infoSimu_SetImei } } },
    { .name = "imeiSv",
      .setter = { .type = SIMUCONFIG_HANDLER_STRING,
                  .handler = { .stringFn = pa_infoSimu_SetImeiSv } } },
    {0}
};

//--------------------------------------------------------------------------------------------------
/**
 * Services available for configuration.
 */
//--------------------------------------------------------------------------------------------------
static const simuConfig_Service_t ConfigService = {
    "info",
    PA_SIMU_CFG_MODEM_ROOT "/info",
    ConfigProperties
};

//--------------------------------------------------------------------------------------------------
/**
 * Reset the return error management
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_ResetErrorCase
(
    void
)
{
    ApplySimuErrorCode = false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the return error
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetErrorCase
(
    le_result_t  res     ///< [IN] type of return error
)
{
    ApplySimuErrorCode = true;
    SimuRes = res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the IMEI
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetImei
(
    const pa_info_Imei_t imei
)
{
    le_utf8_Copy(Imei, imei, PA_INFO_IMEI_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the IMEISV
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetImeiSv
(
    const pa_info_ImeiSv_t imeiSv
)
{
    le_utf8_Copy(ImeiSv, imeiSv, PA_INFO_IMEISV_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Firmware version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetFirmwareVersion
(
    const char* firmwareVersionPtr
)
{
    le_utf8_Copy(FirmwareVersion, firmwareVersionPtr, PA_INFO_VERS_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the bootloader Version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetBootloaderVersion
(
    const char* bootLoaderVersionPtr
)
{
    le_utf8_Copy(BootLoaderVersion, bootLoaderVersionPtr, PA_INFO_VERS_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the device model
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetDeviceModel
(
    pa_info_DeviceModel_t deviceModel
)
{
    le_utf8_Copy(DeviceModel, deviceModel, PA_INFO_MODEL_MAX_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA device Mobile Equipment Identifier (MEID).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetMeid
(
    const char* meidStrPtr
)
{
    le_utf8_Copy(Meid, meidStrPtr, LE_INFO_MAX_MEID_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA Electronic Serial Number (ESN) of the device.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetEsn
(
    const char* esnStrPtr
)
{
    le_utf8_Copy(Esn, esnStrPtr, LE_INFO_MAX_ESN_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA Mobile Identification Number (MIN).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetMin
(
    const char* minStrPtr
)
{
    le_utf8_Copy(Min, minStrPtr, LE_INFO_MAX_MIN_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the version of Preferred Roaming List (PRL).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPrlVersion
(
    uint16_t prlVersion
)
{
    Prl = prlVersion;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Cdma PRL only preferences Flag.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPrlOnlyPreference
(
    bool prlOnlyPreference
)
{
    PrlFlag = prlOnlyPreference;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA Network Access Identifier (NAI) string in ASCII text.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetNai
(
    const char* naiStrPtr
)
{
    le_utf8_Copy(Nai, naiStrPtr, LE_INFO_MAX_NAI_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Manufacturer Name.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetManufacturerName
(
    const char* mfrNameStrPtr
)
{
    le_utf8_Copy(MfrName, mfrNameStrPtr, LE_INFO_MAX_MFR_NAME_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Product Requirement Information Part Number and Revision Number strings.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPriId
(
    const char* priIdPnStrPtr,
    const char* priIdRevStrPtr
)
{
    le_utf8_Copy(PriIdPn, priIdPnStrPtr, LE_INFO_MAX_PRIID_PN_BYTES, NULL);
    le_utf8_Copy(PriIdRev, priIdRevStrPtr, LE_INFO_MAX_PRIID_REV_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the product stock keeping unit number (SKU).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetSku
(
    const char* skuIdStrPtr
)
{
    le_utf8_Copy(Sku, skuIdStrPtr, LE_INFO_MAX_PSN_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Platform Serial Number (PSN) string.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPlatformSerialNumber
(
    const char* platformSerialNumberStrPtr
)
{
    le_utf8_Copy(Psn, platformSerialNumberStrPtr, LE_INFO_MAX_PSN_BYTES, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Rf Device Status.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetRfDeviceStatus
(
    uint16_t index,
    uint16_t manufacturedId,
    uint8_t productId,
    bool status
)
{
    // set the status field if index is below max device status
    if (index < LE_INFO_RF_DEVICES_STATUS_MAX)
    {
        RfDeviceStatus[index].isValid = true;
        RfDeviceStatus[index].manufacturedId = manufacturedId;
        RfDeviceStatus[index].productId = productId;
        RfDeviceStatus[index].status = status;
        return;
    }
    LE_ERROR("Failed to set Rf Device Status for index = %d",index);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the reset information information
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetResetInformation
(
    le_info_Reset_t reset,
    const char*     reasonStr
)
{
    ResetInformation = reset;
    if(reasonStr)
    {
        le_utf8_Copy(ResetReasonStr, reasonStr, LE_INFO_MAX_RESET_BYTES, NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Init the PA.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_infoSimu_Init
(
    void
)
{
    simuConfig_RegisterService(&ConfigService);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Equipment Identity (IMEI).
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_TIMEOUT       No response was received from the Modem.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetImei
(
    pa_info_Imei_t imei   ///< [OUT] IMEI value
)
{
    return (le_utf8_Copy(imei, Imei, sizeof(pa_info_Imei_t), NULL));
}

//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Equipment Identity software version number (IMEISV).
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_TIMEOUT       No response was received from the Modem.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetImeiSv
(
    pa_info_ImeiSv_t imeiSv   ///< [OUT] IMEISV value
)
{
    return (le_utf8_Copy(imeiSv, ImeiSv, sizeof(pa_info_ImeiSv_t), NULL));
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
le_result_t pa_info_GetFirmwareVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    // added to simulate the LE_NOT_FOUND error
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }
    return (le_utf8_Copy(versionPtr, FirmwareVersion,versionSize, NULL));
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
le_result_t pa_info_GetBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
)
{
    // added to simulate the LE_NOT_FOUND error
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }
    return (le_utf8_Copy(versionPtr, BootLoaderVersion, versionSize, NULL));
}


//--------------------------------------------------------------------------------------------------
/**
 * This function gets the device model identity.
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_OVERFLOW      The device model identity length exceed the maximum length.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetDeviceModel
(
    pa_info_DeviceModel_t model   ///< [OUT] Model string (null-terminated).
)
{
    return (le_utf8_Copy(model, DeviceModel, sizeof(pa_info_DeviceModel_t), NULL));
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA device Mobile Equipment Identifier (MEID).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The device Mobile Equipment identifier length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetMeid
(
    char* meidStrPtr,           ///< [OUT] Firmware version string
    size_t meidStrSize       ///< [IN] Size of version buffer
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if (meidStrSize < strlen(Meid))
    {
        return LE_OVERFLOW;
    }
    return (le_utf8_Copy(meidStrPtr, Meid, meidStrSize, NULL));
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Electronic Serial Number (ESN) of the device.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Electric SerialNumbe length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetEsn
(
    char* esnStrPtr,
        ///< [OUT]
        ///< The Electronic Serial Number (ESN) of the device
        ///<  string (null-terminated).

    size_t esnStrNumElements
        ///< [IN]
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if (esnStrNumElements < strlen(Esn))
    {
        return LE_OVERFLOW;
    }

    return (le_utf8_Copy(esnStrPtr, Esn, esnStrNumElements, NULL));
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Mobile Identification Number (MIN).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The CDMA Mobile Identification Number length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetMin
(
    char        *minStrPtr,    ///< [OUT] The phone Number
    size_t       minStrSize ///< [IN]  Size of phoneNumberStr
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if (minStrSize < strlen(Min))
    {
        return LE_OVERFLOW;
    }

    return (le_utf8_Copy(minStrPtr, Min, minStrSize, NULL));
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the version of Preferred Roaming List (PRL).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_NOT_FOUND     The information is not available.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPrlVersion
(
    uint16_t* prlVersionPtr
        ///< [OUT]
        ///< The Preferred Roaming List (PRL) version.
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    *prlVersionPtr = Prl;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the Cdma PRL only preferences Flag.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_NOT_FOUND     The information is not availble.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPrlOnlyPreference
(
    bool* prlOnlyPreferencePtr      ///< The Cdma PRL only preferences status.
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }
    *prlOnlyPreferencePtr = PrlFlag;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Network Access Identifier (NAI) string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Network Access Identifier (NAI) length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetNai
(
    char* naiStrPtr,
        ///< [OUT]
        ///< The Network Access Identifier (NAI)
        ///<  string (null-terminated).

    size_t naiStrNumElements
        ///< [IN]
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if (naiStrNumElements < strlen(Nai))
    {
        return LE_OVERFLOW;
    }

    return (le_utf8_Copy(naiStrPtr, Nai, naiStrNumElements, NULL));
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Manufacturer Name string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Manufacturer Name length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetManufacturerName
(
    char* mfrNameStrPtr,
        ///< [OUT]
        ///< The Manufacturer Name string (null-terminated).

    size_t mfrNameStrNumElements
        ///< [IN]
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if (mfrNameStrNumElements < strlen(MfrName))
    {
        return LE_OVERFLOW;
    }

    return (le_utf8_Copy(mfrNameStrPtr, MfrName, mfrNameStrNumElements, NULL));
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the Product Requirement Information Part Number and Revision Number strings in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Part or the Revision Number strings length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPriId
(
    char* priIdPnStrPtr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Part Number string (null-terminated).

    size_t priIdPnStrNumElements,
        ///< [IN]

    char* priIdRevStrPtr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Revision Number string (null-terminated).

    size_t priIdRevStrNumElements
        ///< [IN]
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if ( (priIdPnStrPtr == NULL) || (priIdRevStrPtr == NULL))
    {
        LE_ERROR("priIdPnStr or priIdRevStr is NULL.");
        return LE_FAULT;
    }

    if (priIdPnStrNumElements < strlen(PriIdPn))
    {
        LE_ERROR("priIdPnStrNumElements lentgh (%d) too small < %d",
                        (int) priIdPnStrNumElements, LE_INFO_MAX_PRIID_PN_BYTES);
        return LE_OVERFLOW;
    }

    if (priIdRevStrNumElements < strlen(PriIdRev))
    {
        LE_ERROR("priIdRevStrNumElements lentgh (%d) too small < %d",
                        (int) priIdRevStrNumElements, LE_INFO_MAX_PRIID_REV_BYTES);
        return LE_OVERFLOW;
    }

    le_utf8_Copy(priIdPnStrPtr, PriIdPn, priIdPnStrNumElements, NULL);
    le_utf8_Copy(priIdRevStrPtr, PriIdRev, priIdRevStrNumElements, NULL);
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the Carrier PRI Name and Revision Number strings in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Name or the Revision Number strings length exceed the maximum length.
 *      - LE_UNSUPPORTED   The function is not supported on the platform.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetCarrierPri
(
    char* capriNameStrPtr,
        ///< [OUT]
        ///< The Carrier Product Requirement Information Name
        ///< Carrier PRI Name string (null-terminated).

    size_t capriNameStrPtrNumElements,
        ///< [IN]

    char* capriRevStrPtr,
        ///< [OUT]
        ///< The Carrier Product Requirement Information
        ///< Carrier PRI Revision Number string (null-terminated).

    size_t capriRevStrPtrNumElements
        ///< [IN]
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if ( (capriNameStrPtr == NULL) || (capriRevStrPtr == NULL))
    {
        LE_ERROR("capriNameStrPtr or capriRevStrPtr is NULL.");
        return LE_FAULT;
    }

    if (capriNameStrPtrNumElements < (strlen(CapriName) + 1))
    {
        LE_ERROR("capriNameStrPtrNumElements length (%d) too small < %d",
                        (int) capriNameStrPtrNumElements, LE_INFO_MAX_CAPRI_NAME_BYTES);
        return LE_OVERFLOW;
    }

    if (capriRevStrPtrNumElements < (strlen(CapriRev) + 1))
    {
        LE_ERROR("capriRevStrPtrNumElements length (%d) too small < %d",
                        (int) capriRevStrPtrNumElements, LE_INFO_MAX_CAPRI_REV_BYTES);
        return LE_OVERFLOW;
    }

    le_utf8_Copy(capriNameStrPtr, PriIdPn, capriNameStrPtrNumElements, NULL);
    le_utf8_Copy(capriRevStrPtr, PriIdRev, capriRevStrPtrNumElements, NULL);
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the product stock keeping unit number (SKU).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The SKU number string length exceeds the maximum length.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetSku
(
    char* skuIdStrPtr,
        ///< [OUT] Product SKU ID string.

    size_t skuIdStrNumElements
        ///< [IN]
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if (skuIdStrPtr == NULL)
    {
        LE_ERROR("skuIdStr is NULL.");
        return LE_FAULT;
    }

    if (skuIdStrNumElements < strlen(Sku))
    {
        return LE_OVERFLOW;
    }

    return (le_utf8_Copy(skuIdStrPtr, Sku, skuIdStrNumElements, NULL));
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the Platform Serial Number (PSN) string.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if Platform Serial Number to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPlatformSerialNumber
(
    char* platformSerialNumberStrPtr,
        ///< [OUT]
        ///< Platform Serial Number string.

    size_t platformSerialNumberStrNumElements
        ///< [IN]
)
{
    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    if (platformSerialNumberStrNumElements < strlen(Psn))
    {
        return LE_OVERFLOW;
    }

    return (le_utf8_Copy(platformSerialNumberStrPtr, Psn,
                         platformSerialNumberStrNumElements, NULL));
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the RF devices working status (i.e. working or broken) of modem's RF devices such as
 * power amplifier, antenna switch and transceiver. That status is updated every time the module
 * power on.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT function failed to get the RF devices working status
 *      - LE_OVERFLOW the number of statuses exceeds the maximum size
 *        (LE_INFO_RF_DEVICES_STATUS_MAX)
 *      - LE_BAD_PARAMETER Null pointers provided
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetRfDeviceStatus
(
    uint16_t* manufacturedIdPtr,
        ///< [OUT] Manufactured identifier (MID)

    size_t* manufacturedIdNumElementsPtr,
        ///< [INOUT]

    uint8_t* productIdPtr,
        ///< [OUT] Product identifier (PID)

    size_t* productIdNumElementsPtr,
        ///< [INOUT]

    bool* statusPtr,
        ///< [OUT] Status of the specified device (MID,PID):
        ///<       0 means something wrong in the RF device,
        ///<       1 means no error found in the RF device

    size_t* statusNumElementsPtr
        ///< [INOUT]
)
{
    uint16_t i = 0;
    uint16_t index = 0;
    size_t statusLen = 0;

    if (true == ApplySimuErrorCode)
    {
        return SimuRes;
    }

    // find the status length
    for (i=0; i<LE_INFO_RF_DEVICES_STATUS_MAX; i++)
    {
        if (true == RfDeviceStatus[i].isValid)
        {
            statusLen = statusLen+1;
        }
    }
    // check for overflow
    if (statusLen > LE_INFO_RF_DEVICES_STATUS_MAX)
    {
        LE_ERROR("Status length overflow !!");
        // Update returned length
        *manufacturedIdNumElementsPtr = 0;
        *productIdNumElementsPtr = 0;
        *statusNumElementsPtr = 0;
        return LE_OVERFLOW;
    }

    // Update returned length
    *manufacturedIdNumElementsPtr = statusLen;
    *productIdNumElementsPtr = statusLen;
    *statusNumElementsPtr = statusLen;

    // Get the status fields
    for (i=0; i<LE_INFO_RF_DEVICES_STATUS_MAX; i++)
    {
        if (true == RfDeviceStatus[i].isValid)
        {
            manufacturedIdPtr[index] = RfDeviceStatus[i].manufacturedId;
            productIdPtr[index] = RfDeviceStatus[i].productId;
            statusPtr[index] = RfDeviceStatus[i].status;
            index = index+1;
        }
    }
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the last reset information reason
 *
 * @return
 *      - LE_OK          on success
 *      - LE_UNSUPPORTED if it is not supported by the platform
 *        LE_OVERFLOW    specific reset information length exceeds the maximum length.
 *      - LE_FAULT       for any other errors
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetResetInformation
(
    le_info_Reset_t* resetPtr,              ///< [OUT] Reset information
    char* resetSpecificInfoStr,             ///< [OUT] Reset specific information
    size_t resetSpecificInfoNumElements     ///< [IN] The length of specific information string.
)
{
    *resetPtr = ResetInformation;
    return le_utf8_Copy(resetSpecificInfoStr, ResetReasonStr, resetSpecificInfoNumElements, NULL);
}
