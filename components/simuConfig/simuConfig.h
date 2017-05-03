/**
 * @file pa_sim_simu.h
 *
 * Config Tree management for the simulation PA.
 *
 * It is possible to update values returned by the simulation PA through the config tree.
 * For instance, to set the Platform Serial Number:
 * @verbatim config set /simulation/modem/info/psn TESTPSN @endverbatim
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef PA_SIMUCONFIG_H_INCLUDE_GUARD
#define PA_SIMUCONFIG_H_INCLUDE_GUARD

//--------------------------------------------------------------------------------------------------
/**
 * Prototype for a string setter with just one parameter.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*simuConfig_StringSetter_t)
(
    const char* valuePtr            ///< Value for the property as read from configuration.
);

//--------------------------------------------------------------------------------------------------
/**
 * Prototype for a string setter with just one parameter.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*simuConfig_BoolSetter_t)
(
    const bool value                ///< Value for the property as read from configuration.
);

//--------------------------------------------------------------------------------------------------
/**
 * Prototype for a complex property setter.
 */
//--------------------------------------------------------------------------------------------------
typedef le_result_t (*simuConfig_ComplexSetter_t)
(
    const char* serviceNamePtr,     ///< Service name
    const char* entryNamePtr,       ///< Entry name
    const void* valuePtr            ///< Pointer to the value, which expected type is defined
                                    ///  in the property.
);

//--------------------------------------------------------------------------------------------------
/**
 * Union providing various prototypes for setter functions.
 */
//--------------------------------------------------------------------------------------------------
typedef union {
    simuConfig_StringSetter_t stringFn;
    simuConfig_BoolSetter_t boolFn;
    simuConfig_ComplexSetter_t complexFn;
}
simuConfig_Setters_t;

//--------------------------------------------------------------------------------------------------
/**
 * Type used to define the validity of a particular setter function in an union.
 */
//--------------------------------------------------------------------------------------------------
typedef enum {
    SIMUCONFIG_HANDLER_STRING,
    SIMUCONFIG_HANDLER_BOOL,
    SIMUCONFIG_HANDLER_COMPLEX
}
simuConfig_HandlerType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Defines one 'set' handler through the combination of an enum+union.
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    simuConfig_HandlerType_t type;
    simuConfig_Setters_t handler;
}
simuConfig_Setter_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure to declare a service property.
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    const char* name;               ///< Name of the property
    simuConfig_Setter_t setter;     ///< Handler used to set value
}
simuConfig_Property_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure to declare a service.
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    const char* namePtr;                        ///< Name of the service
    const char* configTreeRootPathPtr;          ///< Root path for the configuration in the ConfigTree
    const simuConfig_Property_t* properties;    ///< Properties
}
simuConfig_Service_t;

#if !defined(WITHOUT_SIMUCONFIG)

//--------------------------------------------------------------------------------------------------
/**
 * Register (and initialize) a service to simuConfig.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED void simuConfig_RegisterService
(
    const simuConfig_Service_t* servicePtr  ///< Description of the service to register
                                            ///< simuConfig only keeps a reference so content should
                                            ///< stay valid.
);

#else

//--------------------------------------------------------------------------------------------------
/**
 * If WITHOUT_SIMUCONFIG is defined, provide an empty function as we should not do anything.
 */
//--------------------------------------------------------------------------------------------------
#define simuConfig_RegisterService(X) \
    (void)(X)

#endif

#endif // PA_SIMUCONFIG_H_INCLUDE_GUARD
