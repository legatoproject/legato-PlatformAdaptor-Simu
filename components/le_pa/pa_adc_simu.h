/**
 * @file pa_adc_simu.h
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef LEGATO_PA_ADC_SIMU_INCLUDE_GUARD
#define LEGATO_PA_ADC_SIMU_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * Set the return code.
 *
 * @return
 *     - void
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_adcSimu_SetReturnCode
(
    le_result_t res
);


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
);

//--------------------------------------------------------------------------------------------------
/**
 * Stub init function
 *
 **/
//--------------------------------------------------------------------------------------------------

le_result_t pa_adc_Init
(
    void
);

#endif
