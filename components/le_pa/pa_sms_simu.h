/**
 * @file pa_sms_simu.h
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef LEGATO_PA_SMS_SIMU_INCLUDE_GUARD
#define LEGATO_PA_SMS_SIMU_INCLUDE_GUARD

#include "pa_sms.h"
#include "pa_sms_simu.h"
#include "pa_mrc_simu.h"

typedef struct __attribute__((__packed__)) {
    uint8_t origAddress[LE_MDMDEFS_PHONE_NUM_MAX_LEN];
    uint8_t destAddress[LE_MDMDEFS_PHONE_NUM_MAX_LEN];
    pa_sms_Protocol_t protocol;
    uint32_t dataLen;
    uint8_t data[];
}
pa_sms_SimuPdu_t;

// simulate storage type values
#define SIMU_SMS_STORAGE_SIM    0
#define SIMU_SMS_STORAGE_NV     1
#define SIMU_SMS_STORAGE_ERROR  2

#define PA_SIMU_SMS_DEFAULT_SMSC    ""

//--------------------------------------------------------------------------------------------------
/**
 * Set the type of storage in case of full storage indication
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_sms_SetFullStorageType
(
    int storage_type
);


//--------------------------------------------------------------------------------------------------
/**
 * Set the incoming sms
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_sms_SetSmsInStorage
(
  pa_sms_NewMessageIndication_t* msgPtr
);


//--------------------------------------------------------------------------------------------------
/**
 * Set SMS error code
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_sms_SetSmsErrCause
(
   int errorCode
);

le_result_t sms_simu_Init
(
    void
);


#endif
