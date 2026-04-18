/**
 ********************************************************************************
 * @file    log.h
 * @author  rathgden
 * @date    27. 12. 2024
 * @brief   See log.c
 * Header file for the non-blocking logging feature with various logging levels.
 *
 * This file defines the logging interface with the following log levels:
 * - LOG_DEBUG: Detailed debug information.
 * - LOG_INFO: General informational messages.
 * - LOG_WARNING: Warnings that do not halt execution.
 * - LOG_ERROR: Critical errors requiring immediate attention.
 *
 * The logging system is designed for RTOS environments and uses a FreeRTOS queue
 * to ensure non-blocking operation. It also supports rerouting `printf` via UART.
 *
 ********************************************************************************
 */

#ifndef INC_LOG_H_
#define INC_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************************
 * INCLUDES
 ************************************/
#include <stdio.h>
#include <stdarg.h>
#include "stm32f4xx_hal.h"


/************************************
 * GLOBAL ENUMS
 ************************************/
/**
 * @enum LogLevel_t
 * @brief Defines logging levels for filtering messages.
 */
typedef enum {
    LOG_DEBUG = 0,  /**< Debug messages for development purposes. */
    LOG_INFO = 1,   /**< Informational messages. */
    LOG_WARNING = 2,/**< Warning messages indicating potential issues. */
    LOG_ERROR = 3   /**< Error messages indicating critical failures. */
} LogLevel_t;

/************************************
 * GLOBAL DEFINES
 ************************************/
/**
 * @def REROUTE_PRINTF
 * @brief flag that eneables rerouting of printf to UART.
 *
 * If this flag is defined, the `printf` function will be rerouted to UART for logging.
 */
#define REROUTE_PRINTF
/**
 * @def LOG_LEVEL_THRESHOLD
 * @brief Sets the default log level threshold.
 *
 * Logs with a level equal to or higher than this will be processed.
 */
#define LOG_LEVEL_THRESHOLD LOG_DEBUG
/**
 * @def LOG_QUEUE_SIZE
 * @brief Max queue size of log messages.
 * 
 */
#define LOG_QUEUE_SIZE 28
/**
 * @def LOG_MESSAGE_SIZE
 * @brief Maximum size of a log message.
 * 
 */
#define LOG_MESSAGE_SIZE 255

/**
 * @def LOG_MESSAGE_PREFIX_SIZE
 * @brief Maximum size of a log message prefix.
 * Eg. "INFO: " or "ERROR: "
 */
#define LOG_MESSAGE_PREFIX_SIZE 10

/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/
void Log_init(UART_HandleTypeDef huart);
void logMsg(LogLevel_t level, const char *format, ...);
void log_set_Level(LogLevel_t level);
LogLevel_t log_get_Level(void);

/************************************
 * GLOBAL MACROS
 ************************************/
#ifdef REROUTE_PRINTF
	#ifdef __GNUC__
		#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
		#define GETCHAR_PROTOTYPE int __io_getchar(void)
	#else
		#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
		#define GETCHAR_PROTOTYPE int getc(FILE *f)
	#endif
#endif /* REROUTE_PRINTF */
#ifdef __cplusplus
}
#endif

#endif /* INC_LOG_H_ */
