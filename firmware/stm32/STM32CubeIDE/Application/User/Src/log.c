/**
 ********************************************************************************
 * @file    log.c
 * @author  rathgden
 * @date    27. 12. 2024
 * @brief   Implements a non-blocking logging feature with multiple log levels.
 *
 *          This module supports multi-tasking in an RTOS environment by using a
 *          FreeRTOS queue for logging messages. A dedicated logging task
 *          processes and transmits messages via UART.
 *          Log levels allow for filtering messages based on severity.
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "log.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/************************************
 * STATIC VARIABLES
 ************************************/
/**
 * @brief Handle for the UART used for logging output.
 */
static UART_HandleTypeDef log_huart;

/**
 * @brief Current log level threshold.
 *
 * Only messages with a level >= current_log_level will be logged.
 */
static LogLevel_t current_log_level = LOG_LEVEL_THRESHOLD;

/**
 * @brief FreeRTOS queue for storing log messages.
 *
 * Ensures non-blocking behavior and avoids delays in real-time tasks.
 */
static QueueHandle_t logQueue = NULL;

/**
 * @brief FreeRTOS mutex for protecting log configurations.
 */
static SemaphoreHandle_t logMutex = NULL;

/**
 * @brief Optional task handle for the logging task.
 */
static TaskHandle_t loggingTaskHandle = NULL;

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void loggingTask(void *params);

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
/**
 ********************************************************************************
 * @param      UART_HandleTypeDef huart
 * @return     void
 * @brief      Initializes the logging system.
 *             Creates the log queue and mutex, sets the UART handle, and starts
 *             the logging task.
 *             This function must be called before using any logging functions.
 ********************************************************************************
 **/
void Log_init(UART_HandleTypeDef huart)
{
    log_huart = huart;

    // Create the queue for storing log messages
    logQueue = xQueueCreate(LOG_QUEUE_SIZE, LOG_MESSAGE_SIZE + LOG_MESSAGE_PREFIX_SIZE);
    if (logQueue == NULL)
    {
        logMsg(LOG_ERROR, "Unable to create Logging Task Queue!");
    }
    vQueueAddToRegistry(logQueue, "LogQueue");

    // Create the mutex for thread safety
    logMutex = xSemaphoreCreateMutex();
    if (logMutex == NULL)
    {
        logMsg(LOG_ERROR, "Unable to create Logging Task Mutex!");
    }

    // Create the logging task
    BaseType_t result = xTaskCreate(loggingTask, "LoggingTask", 1024, NULL, tskIDLE_PRIORITY + 3, &loggingTaskHandle);
    if (result != pdPASS)
    {
        logMsg(LOG_ERROR, "Unable to create Logging Task!");
    }
}

/**
 ********************************************************************************
 * @param      LogLevel_t level (The new log level threshold)
 * @return     void
 * @brief      Sets the current log level threshold.
 *             Messages with levels lower than the threshold will not be logged.
 ********************************************************************************
 **/
void log_set_Level(LogLevel_t level)
{
    if (xSemaphoreTake(logMutex, portMAX_DELAY))
    {
        current_log_level = level;
        xSemaphoreGive(logMutex);
    }
}

/**
 ********************************************************************************
 * @param      void
 * @return     LogLevel_t (The current log level threshold)
 * @brief      Retrieves the current log level threshold.
 ********************************************************************************
 **/
LogLevel_t log_get_Level(void)
{
    LogLevel_t level;
    if (xSemaphoreTake(logMutex, portMAX_DELAY))
    {
        level = current_log_level;
        xSemaphoreGive(logMutex);
        return level;
    }
    return LOG_INFO;
}

/**
 ********************************************************************************
 * @param      LogLevel_t level    (The log level for the message, e.g., LOG_DEBUG, LOG_INFO)
 *             const char *format  (The message format string, similar to printf)
 *             ...                 (Additional arguments for the format string)
 * @return     void
 * @brief      Logs a message with a specified log level.
 *             Logs the formatted message by enqueuing it in the logQueue.
 *             The message is prefixed with the log level name
 *             (e.g., DEBUG, INFO, WARNING, ERROR).
 *
 *             This function is non-blocking. If the queue is full, the message
 *             is discarded.
 ********************************************************************************
 **/
void logMsg(LogLevel_t level, const char *format, ...)
{
    if (level >= current_log_level)
    {
        va_list args;
        va_start(args, format);
        char buffer[LOG_MESSAGE_SIZE];
        char logBuffer[LOG_MESSAGE_SIZE + LOG_MESSAGE_PREFIX_SIZE];
        vsnprintf(buffer, sizeof(buffer), format, args);

        const char *levelStr;
        switch (level)
        {
        case LOG_DEBUG:
            levelStr = "DEBUG";
            break;
        case LOG_INFO:
            levelStr = "INFO";
            break;
        case LOG_WARNING:
            levelStr = "WARNING";
            break;
        case LOG_ERROR:
            levelStr = "ERROR";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
        }

        snprintf(logBuffer, sizeof(logBuffer), "%s: %s", levelStr, buffer);

        // From ISR
        if (xPortIsInsideInterrupt() == pdTRUE)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (xQueueSendFromISR(logQueue, logBuffer, &xHigherPriorityTaskWoken) != pdPASS)
            {
                // Optionally track dropped messages
            }
        }
        // From task
        else
        {
            if (xQueueSend(logQueue, logBuffer, 0) != pdPASS)
            {
                // Optionally track dropped messages
            }
        }
    }
}

/**
 ********************************************************************************
 * @param      void *params (Unused)
 * @return     void
 * @brief      Task to process log messages and send them via UART.
 *             This FreeRTOS task runs at low priority and handles all UART
 *             transmissions for log messages. Messages are dequeued from the
 *             logQueue and transmitted sequentially.
 ********************************************************************************
 **/
static void loggingTask(void *params)
{
    char message[LOG_MESSAGE_SIZE];
    while (1)
    {
        if (xQueueReceive(logQueue, message, portMAX_DELAY) == pdPASS)
        {
#ifdef REROUTE_PRINTF
            printf("%s\r\n", message);
#else
            HAL_UART_Transmit(&log_huart, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
#endif
        }
    }
}

#ifdef REROUTE_PRINTF
/**
 ********************************************************************************
 * @param      ch The character to transmit.
 * @return     The transmitted character.
 * @brief      Redirects `printf` output to UART directly.
 *
 *             If `REROUTE_PRINTF` is defined, this function transmits characters
 *             via UART immediately, bypassing the logging queue.
 ********************************************************************************
 **/
PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&log_huart, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 ********************************************************************************
 * @param      ch The character to transmit.
 * @return     The received character.
 * @brief      Redirects `getchar` input from UART.
 *
 *             When `REROUTE_PRINTF` is defined, this function reroutes getchar calls to receive
 *             characters from the UART handle.
 ********************************************************************************
 **/
GETCHAR_PROTOTYPE
{
    uint8_t ch = 0;
    HAL_UART_Receive(&log_huart, &ch, 1, HAL_MAX_DELAY); // Blocking receive
    return (int)ch;
}
#endif
