/**
 * @file pa_pcm_simu.c
 *
 * This file contains the source code of the low level Audio API for playback / capture
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "le_audio_local.h"
#include "pa_audio.h"
#include "pa_pcm.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------

#define PACKET_SIZE 10

//--------------------------------------------------------------------------------------------------
//                                       Static declarations
//--------------------------------------------------------------------------------------------------

static uint8_t* DataPtr = NULL;
static uint32_t DataLen=0;
static uint32_t DataIndex=0;
static intptr_t PcmHandle = 0xBADCAFE;
static le_sem_Ref_t*    RecSemaphorePtr = NULL;
static le_thread_Ref_t PcmThreadRef = NULL;
static GetSetFramesFunc_t GetSetFramesFunc = NULL;
static ResultFunc_t ResultFunc = NULL;
static void* HandlerContextPtr = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Playback thread
 *
 */
//--------------------------------------------------------------------------------------------------
static void* PlaybackThread
(
    void* contextPtr
)
{
    le_result_t res = LE_OK;

    LE_DEBUG("Playback started");
    uint32_t len = PACKET_SIZE;
    uint32_t index = 0;
    bool previousNullLen = false;

    LE_ASSERT(GetSetFramesFunc != NULL);

    while (1)
    {
        if (index >= DataLen)
        {
            // just read a char in that case
            uint8_t data;
            LE_ASSERT( GetSetFramesFunc(&data, &len, HandlerContextPtr) == LE_OK );
        }
        else
        {
            len = ((index + PACKET_SIZE) < DataLen) ? PACKET_SIZE : (DataLen-index);
            LE_ASSERT(len != 0);
            LE_ASSERT( GetSetFramesFunc(DataPtr+index, &len, HandlerContextPtr) == LE_OK );
            index += len;
        }

        if (len == 0)
        {
            if (previousNullLen)
            {
                res = LE_UNDERFLOW;
            }
            else
            {
                res = LE_OK;
                previousNullLen = true;
            }

            LE_ASSERT(ResultFunc != NULL);
            ResultFunc(res, HandlerContextPtr);
        }

        pthread_testcancel();
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Capture thread
 *
 */
//--------------------------------------------------------------------------------------------------
static void* CaptureThread
(
    void* contextPtr
)
{
    le_result_t res = LE_OK;
    uint32_t len = PACKET_SIZE;

    while (1)
    {
        if (DataPtr && DataLen)
        {
            LE_ASSERT( GetSetFramesFunc(DataPtr+DataIndex, &len, HandlerContextPtr) == LE_OK );
            LE_ASSERT(len == PACKET_SIZE);
            DataIndex += len;

            if ( DataIndex == DataLen )
            {
                DataIndex = 0;

                if (RecSemaphorePtr != NULL)
                {
                    le_sem_Post(*RecSemaphorePtr);
                    RecSemaphorePtr = NULL;
                }

                break;
            }
        }
        else
        {
            res = LE_FAULT;
            break;
        }
    }

    LE_ASSERT(GetSetFramesFunc != NULL);
    ResultFunc(res, HandlerContextPtr);

    le_event_RunLoop();

    return NULL;
}


//--------------------------------------------------------------------------------------------------
//                                       Public declarations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Set the semaphore to unlock the test thread.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_pcmSimu_SetSemaphore
(
    le_sem_Ref_t*    semaphorePtr
)
{
    RecSemaphorePtr = semaphorePtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Init the data buffer with the correct size.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_pcmSimu_InitData
(
    uint32_t len
)
{
    DataPtr = malloc(len);
    memset(DataPtr,0,len);
    DataLen = len;
}

//--------------------------------------------------------------------------------------------------
/**
 * Release the data buffer.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_pcmSimu_ReleaseData
(
    void
)
{
    free(DataPtr);
    DataLen = 0;
    DataIndex = 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the data buffer.
 *
 */
//--------------------------------------------------------------------------------------------------
uint8_t* pa_pcmSimu_GetDataPtr
(
    void
)
{
    return DataPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start the playback.
 * The function is asynchronous: it starts the playback thread, then returns.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_Play
(
    pcm_Handle_t pcmHandle                  ///< [IN] Handle of the audio resource given by
                                            ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                            ///< initialization functions
)
{
    LE_ASSERT(pcmHandle == (pcm_Handle_t) PcmHandle);
    LE_ASSERT(PcmThreadRef == NULL);

    char threadName[]="PlaybackThread";

    PcmThreadRef = le_thread_Create( threadName,
                                     PlaybackThread,
                                     NULL );

    le_thread_SetJoinable(PcmThreadRef);

    le_thread_Start(PcmThreadRef);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Start the recording.
 * The function is asynchronous: it starts the recording thread, then returns.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_Capture
(
    pcm_Handle_t pcmHandle                 ///< [IN] Handle of the audio resource given by
                                            ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                            ///< initialization functions
)
{
    LE_ASSERT(pcmHandle == (pcm_Handle_t) PcmHandle);
    LE_ASSERT(PcmThreadRef == NULL);

    char threadName[]="CaptureThread";

    PcmThreadRef = le_thread_Create( threadName,
                                     CaptureThread,
                                     NULL );

    le_thread_SetJoinable(PcmThreadRef);

    le_thread_Start(PcmThreadRef);

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Close sound driver.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_Close
(
    pcm_Handle_t pcmHandle                 ///< [IN] Handle of the audio resource given by
                                           ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                           ///< initialization functions
)
{
    if (PcmThreadRef)
    {
        le_thread_Cancel(PcmThreadRef);
        le_thread_Join(PcmThreadRef, NULL);
        PcmThreadRef = NULL;
    }

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the period Size from sound driver.
 *
 */
//--------------------------------------------------------------------------------------------------
uint32_t pa_pcm_GetPeriodSize
(
    pcm_Handle_t pcmHandle                  ///< [IN] Handle of the audio resource given by
                                            ///< pa_pcm_InitPlayback() / pa_pcm_InitCapture()
                                            ///< initialization functions
)
{
    return 3000;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize sound driver for PCM capture.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_InitCapture
(
    pcm_Handle_t *pcmHandlePtr,             ///< [OUT] Handle of the audio resource, to be used on
                                            ///< further PA PCM functions call
    char* devicePtr,                        ///< [IN] Device to be initialized
    le_audio_SamplePcmConfig_t* pcmConfig   ///< [IN] Samples PCM configuration
)
{
    *pcmHandlePtr = (pcm_Handle_t) PcmHandle;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize sound driver for PCM playback.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_InitPlayback
(
    pcm_Handle_t *pcmHandlePtr,             ///< [OUT] Handle of the audio resource, to be used on
                                            ///< further PA PCM functions call
    char* devicePtr,                        ///< [IN] Device to be initialized
    le_audio_SamplePcmConfig_t* pcmConfig   ///< [IN] Samples PCM configuration
)
{
    *pcmHandlePtr = (pcm_Handle_t) PcmHandle;
    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the pa_pcm simu.
 *
 */
//--------------------------------------------------------------------------------------------------
void pa_pcmSimu_Init
(
    void
)
{
    return;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the callbacks called during a playback/recording to:
 * - get/set PCM frames thanks to getFramesFunc callback: this callback will be called by the pa_pcm
 * to get the next PCM frames to play (playback case), or to send back PCM frames to record
 * (recording case).
 * - get the playback/recording result thanks to setResultFunc callback: this callback will be
 * called by the PA_PCM to inform the caller about the status of the playback or the recording.
 *
 */
//--------------------------------------------------------------------------------------------------
le_result_t pa_pcm_SetCallbackHandlers
(
    pcm_Handle_t pcmHandle,
    GetSetFramesFunc_t getSetFramesFunc,
    ResultFunc_t setResultFunc,
    void* contextPtr
)
{
    GetSetFramesFunc = getSetFramesFunc;
    ResultFunc = setResultFunc;
    HandlerContextPtr = contextPtr;

    return LE_OK;
}
