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
#define PA_SIMU_INFO_DEFAULT_IMEISV            "0";
#define PA_SIMU_INFO_DEFAULT_DEVICE_MODEL      "VIRT_SIMU";

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
    pa_info_Imei_t imei
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the IMEISV
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetImeiSv
(
    pa_info_ImeiSv_t imeiSv
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the FW version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetFirmwareVersion
(
    char* versionPtr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the boot version
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_infoSimu_SetBootloaderVersion
(
    char* BootVersionPtr
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


#endif // PA_SIM_SIMU_H_INCLUDE_GUARD

