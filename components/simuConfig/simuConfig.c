/**
 * @file pa_sim_simu.c
 *
 * Config Tree management for the simulation PA.
 *
 * It is possible to update values returned by the simulation PA through the config tree.
 * For instance, to set the Platform Serial Number:
 * @verbatim config set /simulation/modem/info/fsn TESTFSN @endverbatim
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "simuConfig.h"

//--------------------------------------------------------------------------------------------------
/**
 * Pool of all registered services.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t ConfigServicesPool = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Hashmap of all registered services.
 */
//--------------------------------------------------------------------------------------------------
static le_hashmap_Ref_t ConfigServicesMap = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Find a property structure with the given (service,property) names.
 */
//--------------------------------------------------------------------------------------------------
static const simuConfig_Property_t* FindProperty
(
    const char* serviceNamePtr,     ///< Service name to find
    const char* propertyNamePtr     ///< Property name to find
)
{
    const simuConfig_Service_t* servicePtr = le_hashmap_Get(ConfigServicesMap, serviceNamePtr);

    if (NULL != servicePtr)
    {
        // Found service, going through properties
        int propIdx;
        for (propIdx = 0; servicePtr->properties[propIdx].name != NULL; propIdx++)
        {
            const simuConfig_Property_t* propPtr = &(servicePtr->properties[propIdx]);

            if (0 != strcmp(propPtr->name, propertyNamePtr))
            {
                continue;
            }

            // Property found
            return propPtr;
        }
    }

    LE_DEBUG("Property for [%s][%s] not found", serviceNamePtr, propertyNamePtr);
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Handle an entry from the Config tree.
 */
//--------------------------------------------------------------------------------------------------
static void HandleConfigEntry
(
    le_cfg_IteratorRef_t iteratorRef,
    const char* parentNamePtr,
    const char* entryNamePtr
)
{
    le_cfg_nodeType_t nodeType = le_cfg_GetNodeType(iteratorRef, "");

    // Find the property
    const simuConfig_Property_t* propPtr = FindProperty(parentNamePtr, entryNamePtr);

    if (propPtr == NULL)
    {
        LE_INFO("Ignoring entry %s.%s", parentNamePtr, entryNamePtr);
        return;
    }

    switch(nodeType)
    {
        case LE_CFG_TYPE_STRING:
        {
            char stringBuffer[LE_CFG_STR_LEN_BYTES];

            LE_ASSERT(le_cfg_GetString(iteratorRef,
                                       "",
                                       stringBuffer,
                                       sizeof(stringBuffer),
                                       "") == LE_OK);


            LE_DEBUG("Setting %s.%s: %s", parentNamePtr, entryNamePtr,
                                          stringBuffer);

            switch(propPtr->setter.type)
            {
                case SIMUCONFIG_HANDLER_STRING:
                    propPtr->setter.handler.stringFn(stringBuffer);
                    break;

                case SIMUCONFIG_HANDLER_COMPLEX:
                    propPtr->setter.handler.complexFn(parentNamePtr, entryNamePtr, stringBuffer);
                    break;

                default:
                    LE_ERROR("Entry %s.%s is not expecting a string, Ignoring value.",
                             parentNamePtr, entryNamePtr);
                    break;
            }

            break;
        }

        case LE_CFG_TYPE_BOOL:
        {
            bool value = le_cfg_GetBool(iteratorRef, "", false);

            LE_DEBUG("Setting %s.%s: %s", parentNamePtr, entryNamePtr,
                                          value ? "true" : "false");

            switch(propPtr->setter.type)
            {
                case SIMUCONFIG_HANDLER_BOOL:
                    propPtr->setter.handler.boolFn(value);
                    break;

                case SIMUCONFIG_HANDLER_COMPLEX:
                    propPtr->setter.handler.complexFn(parentNamePtr, entryNamePtr, &value);
                    break;

                default:
                    LE_ERROR("Entry %s.%s is not expecting a bool, Ignoring value.",
                             parentNamePtr, entryNamePtr);
                    break;
            }

            break;
        }

        default:
            LE_ERROR("Node type %d not handled", nodeType);
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Go through nodes from the Config tree, and eventually handle them.
 */
//--------------------------------------------------------------------------------------------------
static void HandleConfigNode
(
    const simuConfig_Service_t* servicePtr,     ///< Service to configure
    le_cfg_IteratorRef_t iteratorRef            ///< Ref to iterator
)
{
    do
    {
        le_cfg_nodeType_t nodeType;

        nodeType = le_cfg_GetNodeType(iteratorRef, "");

        if (nodeType == LE_CFG_TYPE_STEM)
        {
            if (le_cfg_GoToFirstChild(iteratorRef) == LE_OK)
            {
                HandleConfigNode(servicePtr, iteratorRef);
                le_cfg_GoToNode(iteratorRef, "..");
            }
        }
        else
        {
            char name[LE_CFG_STR_LEN_BYTES] = {0};
            le_cfg_GetNodeName(iteratorRef, "", name, sizeof(name));
            HandleConfigEntry(iteratorRef, servicePtr->namePtr, name);
        }
    }
    while (le_cfg_GoToNextSibling(iteratorRef) == LE_OK);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set simulation configuration as provided in the config tree.
 */
//--------------------------------------------------------------------------------------------------
void ConfigureFromTree
(
    const simuConfig_Service_t* servicePtr      ///< Service to configure
)
{
    le_cfg_IteratorRef_t iteratorRef = le_cfg_CreateReadTxn(servicePtr->configTreeRootPathPtr);
    HandleConfigNode(servicePtr, iteratorRef);
    le_cfg_CancelTxn(iteratorRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function trigger to process changes from the config tree.
 */
//--------------------------------------------------------------------------------------------------
void ChangeHandler
(
    void *contextPtr
)
{
    LE_INFO("Configuration change detected");
    ConfigureFromTree(contextPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Register (and initialize) a service to simuConfig.
 */
//--------------------------------------------------------------------------------------------------
void simuConfig_RegisterService
(
    const simuConfig_Service_t* servicePtr  ///< Description of the service to register
                                            ///  simuConfig only keeps a reference so content should
                                            ///  stay valid.
)
{
    LE_INFO("Registering new service %s", servicePtr->namePtr);

    // Copy content
    // Note that the structure contain references to arrays stored elsewhere which are not copied.
    simuConfig_Service_t* entryPtr = le_mem_ForceAlloc(ConfigServicesPool);
    memcpy(entryPtr, servicePtr, sizeof(simuConfig_Service_t));

    // Register in the map
    le_hashmap_Put(ConfigServicesMap, entryPtr->namePtr, entryPtr);

    // Provide handler to be notified upon changes
    LE_ASSERT(servicePtr->configTreeRootPathPtr != NULL);
    le_cfg_AddChangeHandler(servicePtr->configTreeRootPathPtr, ChangeHandler, entryPtr);

    // Handle existing values
    ConfigureFromTree(entryPtr);
}

COMPONENT_INIT
{
    // Create memory pool to store the data
    ConfigServicesPool = le_mem_CreatePool("configServices", sizeof(simuConfig_Service_t));

    // Create hashmap to associate <service> to structure.
    ConfigServicesMap = le_hashmap_Create("configServices", 8, le_hashmap_HashString,
                                                               le_hashmap_EqualsString);
}
