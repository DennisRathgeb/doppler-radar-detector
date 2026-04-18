/**
 ********************************************************************************
 * @file    error_handler.h
 * @author  rathgden
 * @date  	27. 12. 2024
 * @brief   See error_handler.c
 * Centralized error handling for embedded systems with custom error levels.
 *
 * Provides functionality to handle errors with severity levels:
 * - ERR_WARNING: Non-critical warnings.
 * - ERR_ERROR: Recoverable errors.
 * - ERR_FATAL: Critical errors that may halt the system.
 *
 * Also includes HAL-specific error handling and an ASSERT macro.
 *
 ********************************************************************************
 */
#ifndef INC_ERROR_HANDLER_H_
#define INC_ERROR_HANDLER_H_

#ifdef __cplusplus
extern "C"
{
#endif
/************************************
 * INCLUDES
 ************************************/
#include <log.h>
#include <stm32f4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"

    /************************************
     * GLOBAL ENUMS
     ************************************/
    /**
     * @enum ErrorLevel_t
     * @brief Represents the severity of an error.
     */
    typedef enum
    {
        ERR_OK = 0,      /**< No error. */
        ERR_WARNING = 1, /**< Warning: Non-critical error. */
        ERR_ERROR = 2,   /**< Error: Recoverable error. */
        ERR_FATAL = 3,   /**< Fatal: Critical error, may halt execution. */
        SPASSTIER = 4
    } ErrorLevel_t;

    /************************************
     * GLOBAL TYPEDEFS
     ************************************/
    /**
     * @struct Error_t
     * @brief Structure to hold error information.
     *
     * Used to encapsulate information about errors, including the module name,
     * severity level, and a descriptive message.
     */
    typedef struct
    {
        const char *module;  /**< Name of the module where the error occurred. */
        ErrorLevel_t level;  /**< Severity level of the error. */
        const char *message; /**< Description of the error. */
    } Error_t;

    /************************************
     * GLOBAL FUNCTION PROTOTYPES
     ************************************/
    Error_t createError(const char *module, ErrorLevel_t level, const char *message);
    void handleError(const Error_t *error);
    void handleHalError(HAL_StatusTypeDef status, const char *module);

/************************************
 * GLOBAL MACROS
 ************************************/
#define RETURN_OK() \
    return createError(__FILE__, ERR_OK, "Success");

#define RETURN_WARNING(message) \
    return createError(__FILE__, ERR_WARNING, message);

#define RETURN_ERROR(message) \
    return createError(__FILE__, ERR_ERROR, message);

#define RETURN_FATAL(message) \
    return createError(__FILE__, ERR_FATAL, message);

#define exec_hal(function_call)                                        \
    do                                                                 \
    {                                                                  \
        HAL_StatusTypeDef result = function_call;                      \
        if (result != HAL_OK)                                          \
        {                                                              \
            handleHalError(result, __FILE__); /* Call error handler */ \
        }                                                              \
    } while (0)
#define exec(function_call)                                \
    do                                                     \
    {                                                      \
        Error_t result = function_call;                    \
        if (result.level != ERR_OK)                        \
        {                                                  \
            handleError(&result); /* Call error handler */ \
        }                                                  \
    } while (0)

#endif /* INC_ERROR_HANDLER_H_ */
