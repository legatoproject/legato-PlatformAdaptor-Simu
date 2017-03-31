/**
 * @file pa_simu.c
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_rsim_simu.h"

//--------------------------------------------------------------------------------------------------
/**
 * Component initializer automatically called by the application framework when the process starts.
 *
 * This is not used because the PA component is shared by two different processes (the Modem Daemon
 * and the Positioning Daemon), and each needs different QMI services initialized.
 **/
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    le_result_t res;

    LE_INFO("PA remote SIM simu Init");

    res = pa_rsim_Init();
    LE_FATAL_IF(res != LE_OK, "PA RemoteSim Init Failed");
}

