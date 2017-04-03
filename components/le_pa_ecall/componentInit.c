/**
 * @file componentInit.c
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"

#include "interfaces.h"
#include "pa_ecall_simu.h"
//--------------------------------------------------------------------------------------------------
/**
 * Component initializer automatically called by the application framework when the process starts.
 *
 **/
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    le_result_t res;

    LE_INFO("PA Init");

    /* Init sub-PAs */
    res = ecall_simu_Init();
    LE_FATAL_IF(res != LE_OK, "PA eCall Init Failed");
}
