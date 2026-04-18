/*
 * config.h
 *
 *  Created on: Apr 9, 2025
 *      Author: Dennis Rathgeb
 */

#ifndef APPLICATION_USER_INC_CONFIG_H_
#define APPLICATION_USER_INC_CONFIG_H_

/************************************
 * LOG CONFIG
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

#endif /* APPLICATION_USER_INC_CONFIG_H_ */
