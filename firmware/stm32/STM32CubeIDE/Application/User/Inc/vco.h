/*
 * vco.h
 *
 *  Created on: Mar 24, 2025
 *      Author: Dennis Rathgeb
 */

#ifndef APPLICATION_USER_INC_VCO_H_
#define APPLICATION_USER_INC_VCO_H_



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

#define VCO_MAX_INSTANCES 1




    /**
     * @struct VCO_HandleTypeDef_t
     * @brief  handle for VCO wave output generation using DAC
     *
     */
    typedef struct
    {
        DAC_HandleTypeDef *hdac;       /**< ADC handle */
        //TIM_HandleTypeDef *htim;       /**< Timer handle */
        uint16_t ramp_steps;
        uint32_t trigger;

    } VCO_HandleTypeDef_t;

    /************************************
     * GLOBAL FUNCTION PROTOTYPES
     ************************************/
    Error_t VCO_Init(VCO_HandleTypeDef_t *hvco, DAC_HandleTypeDef *hdac, uint16_t ramp_steps, uint32_t trigger);
    Error_t VCO_start(VCO_HandleTypeDef_t *hvco);
    Error_t VCO_stop(VCO_HandleTypeDef_t *hvco);


#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_USER_INC_VCO_H_ */
