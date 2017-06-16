/** @file pa_info_simu.h
 *
 * Legato @ref pa_info_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_INFO_SIMU_H_INCLUDE_GUARD
#define PA_INFO_SIMU_H_INCLUDE_GUARD

#include "pa_info.h"


#define PA_SIMU_INFO_DEFAULT_IMEI              "314159265358979"
#define PA_SIMU_INFO_DEFAULT_FW_VERSION        "Firmware 1.00"
#define PA_SIMU_INFO_DEFAULT_BOOT_VERSION      "Bootloader 1.00"
#define PA_SIMU_INFO_DEFAULT_IMEISV            "0"
#define PA_SIMU_INFO_DEFAULT_DEVICE_MODEL      "VIRT_SIMU"
#define PA_SIMU_INFO_DEFAULT_MEID              "0"
#define PA_SIMU_INFO_DEFAULT_ESN               "0"
#define PA_SIMU_INFO_DEFAULT_MIN               "0"
#define PA_SIMU_INFO_DEFAULT_PRL               0
#define PA_SIMU_INFO_DEFAULT_NAI               "0"
#define PA_SIMU_INFO_DEFAULT_MFR               "SW"
#define PA_SIMU_INFO_DEFAULT_PRIID_PN          "0"
#define PA_SIMU_INFO_DEFAULT_PRIID_REV         "0"
#define PA_SIMU_INFO_DEFAULT_SKU               "0"
#define PA_SIMU_INFO_DEFAULT_PSN               "0"
#define PA_SIMU_INFO_DEFAULT_CAPRI_NAME        "0"
#define PA_SIMU_INFO_DEFAULT_CAPRI_REV         "0"

//--------------------------------------------------------------------------------------------------
/**
 * Reset the return error management
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_ResetErrorCase
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * Set the reset information information
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetResetInformation
(
    le_info_Reset_t reset,
    const char*     reason
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the return error
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetErrorCase
(
    le_result_t  res
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the IMEI
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetImei
(
    const pa_info_Imei_t imei
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the IMEISV
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetImeiSv
(
    const pa_info_ImeiSv_t imeiSv
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the FW version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetFirmwareVersion
(
    const char* versionPtr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the boot version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetBootloaderVersion
(
    const char* versionPtr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the device model
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetDeviceModel
(
   pa_info_DeviceModel_t model
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA device Mobile Equipment Identifier (MEID).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetMeid
(
    const char* meidStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA Electronic Serial Number (ESN) of the device.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetEsn
(
    const char* esnStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA Mobile Identification Number (MIN).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetMin
(
    const char* minStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the version of Preferred Roaming List (PRL).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPrlVersion
(
    uint16_t prlVersionPtr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the Cdma PRL only preferences Flag.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPrlOnlyPreference
(
    bool prlOnlyPreferencePtr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the CDMA Network Access Identifier (NAI) string in ASCII text.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetNai
(
    const char* naiStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the Manufacturer Name.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetManufacturerName
(
    const char* mfrNameStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the Product Requirement Information Part Number and Revision Number strings.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPriId
(
    const char* priIdPnStr,
    const char* priIdRevStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the product stock keeping unit number (SKU).
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetSku
(
    const char* skuIdStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the Platform Serial Number (PSN) string.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetPlatformSerialNumber
(
    const char* platformSerialNumberStr
);

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
);

//--------------------------------------------------------------------------------------------------
/**
 * Init the PA.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_infoSimu_Init
(
    void
);

#endif // PA_INFO_SIMU_H_INCLUDE_GUARD

