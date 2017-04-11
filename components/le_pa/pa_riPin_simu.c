/**
 * @file pa_riPin_simu.c
 *
 * Simulation implementation of PA Ring Indicator signal.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "pa_riPin.h"


static le_sem_Ref_t SemRef;
static le_result_t ReturnCode = LE_FAULT;
static bool AmIOwner = false;
uint8_t RingSignalValue = 0;

//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Set the stub return code.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_riPinSimu_SetReturnCode
(
    le_result_t res
)
{
    ReturnCode = res;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the "AmIOwner" flag
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_riPinSimu_SetAmIOwnerOfRingSignal
(
    bool amIOwner
)
{
    AmIOwner = amIOwner;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the "AmIOwner" value
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_riPinSimu_CheckAmIOwnerOfRingSignal
(
    bool amIOwner
)
{
    LE_ASSERT(AmIOwner == amIOwner);
}

//--------------------------------------------------------------------------------------------------
/**
 * Get RI signal value
 *
 */
//--------------------------------------------------------------------------------------------------
uint8_t pa_riPinSimu_Get
(
       void
)
{
    le_sem_Wait(SemRef);
    return RingSignalValue;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to initialize the PA Ring Indicator signal module.
 *
 * @return
 *   - LE_FAULT         The function failed.
 *   - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_Init
(
    void
)
{
    // Init semaphore to synchronize pa_riPinSimu_Get() on pa_riPin_Set()
    SemRef = le_sem_Create("PaSimuRiPinSem", 0);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check whether the application core is the current owner of the Ring Indicator signal.
 *
 * @return
 *      - LE_OK              The function succeeded.
 *      - LE_FAULT           The function failed.
 *      - LE_BAD_PARAMETER   Bad input parameter.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_AmIOwnerOfRingSignal
(
    bool* amIOwnerPtr ///< [OUT] true when application core is the owner of the Ring Indicator
                      ///        signal,
                      ///        false when modem core is the owner of the Ring Indicator signal.
)
{
      // Check input pointer
    if (NULL == amIOwnerPtr)
    {
        LE_ERROR("Null pointer");
        return LE_BAD_PARAMETER;
    }

    if (ReturnCode == LE_OK)
    {
        *amIOwnerPtr = AmIOwner;
    }

    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Take control of the Ring Indicator signal.
 *
 * @return
 *      - LE_OK           The function succeeded.
 *      - LE_FAULT        The function failed.
 *      - LE_UNSUPPORTED  The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_TakeRingSignal
(
    void
)
{
    if(ReturnCode == LE_OK)
    {
        AmIOwner = true;
    }

    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Release control of the Ring Indicator signal.
 *
 * @return
 *      - LE_OK           The function succeeded.
 *      - LE_FAULT        The function failed.
 *      - LE_UNSUPPORTED  The platform does not support this operation.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_riPin_ReleaseRingSignal
(
    void
)
{
    if(ReturnCode == LE_OK)
    {
        AmIOwner = false;
    }

    return ReturnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set RI GPIO value
 */
//--------------------------------------------------------------------------------------------------
void pa_riPin_Set
(
    uint8_t     set ///< [IN] 1 to Pull up GPIO RI or 0 to lower it down
)
{
    RingSignalValue = set;
    le_sem_Post(SemRef);

    return ;
}

