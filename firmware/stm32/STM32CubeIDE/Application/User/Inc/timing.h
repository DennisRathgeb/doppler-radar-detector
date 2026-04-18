/*
 * vco.h
 *
 *  Created on: Mar 24, 2025
 *      Author: Dennis Rathgeb
 */

#ifndef APPLICATION_USER_INC_TIMING_H_
#define APPLICATION_USER_INC_TIMING_H_



#ifdef __cplusplus
extern "C"
{
#endif

/************************************
 * INCLUDES
 ************************************/
#include "stm32f4xx_hal.h"
#include "log.h"
#include "error_handler.h"
#include "main.h"
#include "vco.h"

#define TIMING_MAX_INSTANCES 1
#define TIMING_SAMP_PERIOD 700 //700steps at 180mhz ->256khz
#define TIMING_SWEEP_PERIOD 89999 //(at 90mhz): 1ms
#define TIMING_PAUSE_PERIOD (89999/2) //
#define TIMING_N_STEPS 256

    /**
     * @struct Timing_HandleTypeDef_t
     * @brief  Timing handle for the sampling and wave generation
     *
     */
    typedef struct
    {
               /**< ADC handle */
        TIM_HandleTypeDef *htim_mas;       /**< Master Timer handle (total duration) */
        TIM_HandleTypeDef *htim_sig; 		/**< Slave Timer handle (outputs signal)*/

    } Timing_HandleTypeDef_t;

    /************************************
     * GLOBAL FUNCTION PROTOTYPES
     ************************************/
    Error_t Timing_Init(Timing_HandleTypeDef_t *htiming, TIM_HandleTypeDef *htim_mas, TIM_HandleTypeDef *htim_sig);
    Error_t Timing_start_seq(Timing_HandleTypeDef_t *htiming);
    Error_t Timing_stop_seq(Timing_HandleTypeDef_t *htiming);


#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_USER_INC_TIMING_H_ */
