/**
 * @file pa_adc_simu.c
 *
 * simu implementation of ADC API - stub.
 *
 * Copyright (C) Sierra Wireless Inc.
 */


#include "legato.h"
#include "interfaces.h"
#include "pa_adc.h"


#define adcValue    120

//--------------------------------------------------------------------------------------------------
/*
 * Return code
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReturnCode = LE_FAULT;

//--------------------------------------------------------------------------------------------------
/**
 * Set the return code.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_adcSimu_SetReturnCode
(
    le_result_t res
)
{
    ReturnCode = res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Read the value of a given ADC channel in units appropriate to that channel.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_adc_ReadValue
(
    const char* adcNamePtr,
        ///< [IN]
        ///< Name of the ADC to read.

    int32_t* adcValuePtr
        ///< [OUT]
        ///< The adc value
)
{
    if (ReturnCode == LE_FAULT)
    {
        return LE_FAULT;
    }
    *adcValuePtr = adcValue;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Stub init function
 *
 **/
//--------------------------------------------------------------------------------------------------

le_result_t pa_adc_Init
(
    void
)
{
    LE_INFO("simulation pa_adc init - stub");

    return LE_OK;
}
