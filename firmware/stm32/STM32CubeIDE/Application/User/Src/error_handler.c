/**
 ********************************************************************************
 * @file    error_handler.c
 * @author  rathgden
 * @date    27. 12. 2024
 * @brief   Implements centralized error handling with custom severity levels.
 * 			This file defines functions to process and log errors, as well as handle
 * 			HAL-specific failures. Errors are logged via the logMsg feature.
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "error_handler.h"

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
/**
 ********************************************************************************
 * @param			const char *module	(module origin of error.
 * 										__FILE__ can be used)
 * 					ErrorLevel_t level	(level Error levelof type ErrorLevel_t)
 * 					const char *message	(message Errormessage to be passed)
 * @return			Error_t 			(Error code)
 * @brief   		Create a Error object
 ********************************************************************************
 **/
Error_t createError(const char *module, ErrorLevel_t level, const char *message)
{
    return (Error_t){.module = module, .level = level, .message = message};
}

/**
 ********************************************************************************
 * @param			const Error_t *error	(error Pointer to the Error_t
 * 											structure with error details.)
 * @return			void
 * @brief
 * Handles general errors by logging and taking appropriate actions.
 * Logs the error message and severity. Fatal errors may trigger a system halt or reset.
 ********************************************************************************
 **/
void handleError(const Error_t *error)
{
    if (error == NULL)
    {
        logMsg(LOG_ERROR, "Error handler received NULL error structure.");
        return;
    }

    // Map error level to log level
    const char *levelStr;
    switch (error->level)
    {
    case ERR_WARNING:
        levelStr = "WARNING";
        logMsg(LOG_ERROR, "%s in %s: %s", levelStr, error->module, error->message);
        break;
    case ERR_ERROR:
        levelStr = "ERROR";
        logMsg(LOG_ERROR, " %s! in %s: %s", levelStr, error->module, error->message);
        break;
    case ERR_FATAL:
        levelStr = "FATAL";
        logMsg(LOG_ERROR, " %s! in %s: %s", levelStr, error->module, error->message);
        logMsg(LOG_WARNING, "System will now be reset!");
        // Perform fatal error action: system reset
        vTaskDelay(pdMS_TO_TICKS(100));
        NVIC_SystemReset();

        break;
    default:
        logMsg(LOG_ERROR, "Module: %s | Level: UNKNOWN | Message: %s", error->module, error->message);
        break;
    }
}

/**
 ********************************************************************************
 * @param			HAL_StatusTypeDef status	(status The HAL_StatusTypeDef error code.)
 * 					const char *module			(module The name of the module where the HAL error occurred.)
 * @return			void
 * @brief
 * Handles HAL-specific errors by logging and reporting the failure.
 * Converts HAL_StatusTypeDef error codes to human-readable messages and passes
 * them to the general error handler.
 ********************************************************************************
 **/
void handleHalError(HAL_StatusTypeDef status, const char *module)
{
    const char *message;

    switch (status)
    {
    case HAL_OK:
        return; // No error, do nothing
    case HAL_ERROR:
        message = "HAL_ERROR";
        break;
    case HAL_BUSY:
        message = "HAL_BUSY";
        break;
    case HAL_TIMEOUT:
        message = "HAL_TIMEOUT";
        break;
    default:
        message = "UNKNOWN HAL ERROR";
        break;
    }

    Error_t halError = {
        .module = module,
        .level = ERR_ERROR, // HAL errors are treated as recoverable by default
        .message = message};

    handleError(&halError);
}
