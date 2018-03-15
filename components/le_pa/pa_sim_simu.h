/** @file pa_sim_simu.h
 *
 * Legato @ref pa_sim_simu include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_SIM_SIMU_H_INCLUDE_GUARD
#define PA_SIM_SIMU_H_INCLUDE_GUARD

#include "pa_sim.h"

//--------------------------------------------------------------------------------------------------
/**
 * Provide default values used by the simulation engine.
 */
//--------------------------------------------------------------------------------------------------
#define PA_SIMU_SIM_DEFAULT_MCC                     "001"
#define PA_SIMU_SIM_DEFAULT_MNC                     "01"
#define PA_SIMU_SIM_DEFAULT_PIN_REMAINING_ATTEMPTS  3
#define PA_SIMU_SIM_DEFAULT_PUK_REMAINING_ATTEMPTS  3
#define PA_SIMU_SIM_DEFAULT_IMSI                    "001012345678910"
#define PA_SIMU_SIM_DEFAULT_ICCID                   "12345678901234567890"
#define PA_SIMU_SIM_DEFAULT_EID                     "69876501010101010101010101050028"
#define PA_SIMU_SIM_DEFAULT_PHONE_NUMBER            "+15032541000"
#define PA_SIMU_SIM_DEFAULT_HOME_NETWORK            "test"
#define PA_SIMU_SIM_DEFAULT_PIN                     "0000"
#define PA_SIMU_SIM_DEFAULT_PUK                     "12345678"

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network MCC MNC.
 *
 * If a parameter exceeds the allowed size the application will exit.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkMccMnc
(
    const char *mccPtr, ///< [IN] MCC (max length LE_MRC_MCC_BYTES)
    const char *mncPtr  ///< [IN] MNC (max length LE_MRC_MNC_BYTES)
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network MCC.
 *
 * If a parameter exceeds the allowed size the application will exit.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkMcc
(
    const char *mccPtr  ///< [IN] MCC (max length LE_MRC_MCC_BYTES)
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network MNC.
 *
 * If a parameter exceeds the allowed size the application will exit.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkMnc
(
    const char *mncPtr  ///< [IN] MNC (max length LE_MRC_MNC_BYTES)
);

//--------------------------------------------------------------------------------------------------
/**
 * This function creates a semaphore that should be used to wait for an STK confirmation call
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_CreateSempahoreForSTKConfirmation
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * This function deletes the semaphore used in STK confirmation
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_DeleteSempahoreForSTKConfirmation
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * This function  waits for an STK confirmation call. This function requires a semaphore to be
 * created by calling pa_simSimu_CreateSempahoreForSTKConfirmation()
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_WaitForSTKConfirmation
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Report the SIM state.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_ReportSIMState
(
    le_sim_States_t state
);

//--------------------------------------------------------------------------------------------------
/**
 * Select the SIM card currently in use.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetSelectCard
(
    le_sim_Id_t simId     ///< [IN] The SIM currently selected
);

//--------------------------------------------------------------------------------------------------
/**
 * SIM Stub initialization.
 *
 * @return LE_OK           The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_simSimu_Init
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * This function set the International Mobile Subscriber Identity (IMSI).
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetIMSI
(
    pa_sim_Imsi_t imsi   ///< [IN] IMSI value
);

//--------------------------------------------------------------------------------------------------
/**
 * This function set the card identification (ICCID).
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetCardIdentification
(
    const pa_sim_CardId_t iccid     ///< [IN] ICCID value
);

//--------------------------------------------------------------------------------------------------
/**
 * This function sets the EID.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetEID
(
    const pa_sim_Eid_t eid     ///< [IN] EID value
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the SIM Phone Number.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetSubscriberPhoneNumber
(
    const char* phoneNumberStr
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the Home Network Name information.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetHomeNetworkOperator
(
    const char* nameStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the PIN code.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetPIN
(
    const char* pin
);

//--------------------------------------------------------------------------------------------------
/**
 * Enable/disable the PIN code usage.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetPINSecurity
(
    bool enable ///< [IN] Should the PIN code be used or not.
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the PUK code.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetPUK
(
    const char* puk
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the STK refresh mode.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetRefreshMode
(
    le_sim_StkRefreshMode_t mode
);

//--------------------------------------------------------------------------------------------------
/**
 * Set the STK refresh stage.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetRefreshStage
(
    le_sim_StkRefreshStage_t stage
);

//--------------------------------------------------------------------------------------------------
/**
 * Report a SIM toolkit event.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_ReportSTKEvent
(
    le_sim_StkEvent_t  stkEvent
);

//--------------------------------------------------------------------------------------------------
/**
 * This function must be called to set the expected confirmation command.
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetExpectedSTKConfirmationCommand
(
    bool  confirmation ///< [IN] True to accept, false to reject
);

//--------------------------------------------------------------------------------------------------
/**
 * Set SimAccessTest variable
 */
//--------------------------------------------------------------------------------------------------
void pa_simSimu_SetSIMAccessTest
(
    bool testInProgress
);

#endif // PA_SIM_SIMU_H_INCLUDE_GUARD

