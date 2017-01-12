/**
 * @file pa_info_simu.c
 *
 * Simulation implementation of @ref c_pa_info API.
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 */

#include "legato.h"
#include "pa_info.h"
#include "interfaces.h"
#include "pa_simu.h"
#include "pa_info_simu.h"

#include <sys/utsname.h>

static pa_info_Imei_t Imei = PA_SIMU_INFO_DEFAULT_IMEI;
static pa_info_ImeiSv_t ImeiSv = PA_SIMU_INFO_DEFAULT_IMEISV;
static pa_info_DeviceModel_t DeviceModel = PA_SIMU_INFO_DEFAULT_DEVICE_MODEL;
static char FirmwareVersion[PA_INFO_VERS_MAX_BYTES] = PA_SIMU_INFO_DEFAULT_FW_VERSION;
static char BootLoaderVersion[PA_INFO_VERS_MAX_BYTES] = PA_SIMU_INFO_DEFAULT_BOOT_VERSION;

static le_result_t SimuRes;
static bool        ApplySimuErrorCode;

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
    pa_info_Imei_t imei
)
{
    strncpy(Imei, imei, PA_INFO_IMEI_MAX_LEN);
    Imei[PA_INFO_IMEI_MAX_LEN]='\0';
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the IMEISV
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetImeiSv
(
    pa_info_ImeiSv_t imeiSv
)
{
    strncpy(ImeiSv, imeiSv, PA_INFO_IMEISV_MAX_LEN);
    ImeiSv[PA_INFO_IMEISV_MAX_LEN]='\0';
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Firmware version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetFirmwareVersion
(
    char* firmwareVersionPtr
)
{
    strncpy(FirmwareVersion, firmwareVersionPtr, PA_INFO_VERS_MAX_LEN);
    FirmwareVersion[PA_INFO_VERS_MAX_LEN]='\0';
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the bootloader Version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetBootloaderVersion
(
    char* bootLoaderVersionPtr
)
{
    strncpy(BootLoaderVersion, bootLoaderVersionPtr, PA_INFO_VERS_MAX_LEN);
    BootLoaderVersion[PA_INFO_VERS_MAX_LEN]='\0';
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
    strncpy(DeviceModel, deviceModel, PA_INFO_MODEL_MAX_LEN);
    DeviceModel[PA_INFO_MODEL_MAX_LEN]='\0';
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
    char* meidStr,           ///< [OUT] Firmware version string
    size_t meidStrSize       ///< [IN] Size of version buffer
)
{
    return LE_FAULT;
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
    char* esnStr,
        ///< [OUT]
        ///< The Electronic Serial Number (ESN) of the device
        ///<  string (null-terminated).

    size_t esnStrNumElements
        ///< [IN]
)
{
    return LE_FAULT;
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
    char        *minStr,    ///< [OUT] The phone Number
    size_t       minStrSize ///< [IN]  Size of phoneNumberStr
)
{
    return LE_FAULT;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get the version of Preferred Roaming List (PRL).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_info_GetPrlVersion
(
    uint16_t* prlVersionPtr
        ///< [OUT]
        ///< The Preferred Roaming List (PRL) version.
)
{
    return LE_FAULT;
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
    return LE_FAULT;
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
    char* naiStr,
        ///< [OUT]
        ///< The Network Access Identifier (NAI)
        ///<  string (null-terminated).

    size_t naiStrNumElements
        ///< [IN]
)
{
    return LE_FAULT;
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
    char* mfrNameStr,
        ///< [OUT]
        ///< The Manufacturer Name string (null-terminated).

    size_t mfrNameStrNumElements
        ///< [IN]
)
{
    return LE_FAULT;
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
    char* priIdPnStr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Part Number string (null-terminated).

    size_t priIdPnStrNumElements,
        ///< [IN]

    char* priIdRevStr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Revision Number string (null-terminated).

    size_t priIdRevStrNumElements
        ///< [IN]
)
{
    le_result_t res = LE_FAULT;

    if ( (priIdPnStr == NULL) || (priIdRevStr == NULL))
    {
        LE_ERROR("priIdPnStr or priIdRevStr is NULL.");
        res =  LE_FAULT;
    }

    if (priIdPnStrNumElements < LE_INFO_MAX_PRIID_PN_BYTES)
    {
        LE_ERROR("priIdPnStrNumElements lentgh (%d) too small < %d",
                        (int) priIdPnStrNumElements, LE_INFO_MAX_PRIID_PN_BYTES);
        res = LE_OVERFLOW;
    }

    if (priIdRevStrNumElements < LE_INFO_MAX_PRIID_REV_BYTES)
    {
        LE_ERROR("priIdRevStrNumElements lentgh (%d) too small < %d",
                        (int) priIdRevStrNumElements, LE_INFO_MAX_PRIID_REV_BYTES);
        res = LE_OVERFLOW;
    }

    return res;
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
    char* skuIdStr,
        ///< [OUT] Product SKU ID string.

    size_t skuIdStrNumElements
        ///< [IN]
)
{
    le_result_t res = LE_OK;

    if (skuIdStr == NULL)
    {
        LE_ERROR("skuIdStr is NULL.");
        res =  LE_FAULT;
    }

    return res;
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
    char* platformSerialNumberStr,
        ///< [OUT]
        ///< Platform Serial Number string.

    size_t platformSerialNumberStrNumElements
        ///< [IN]
)
{
    return LE_FAULT;
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
    return LE_FAULT;
}
