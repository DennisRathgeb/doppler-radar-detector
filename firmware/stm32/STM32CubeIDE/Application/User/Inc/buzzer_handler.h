/**
 ********************************************************************************
 * @file    buzzer_handler.h
 * @author  uhlmabry
 * @date    27. 12. 2024
 * @brief   See @buzzer_handler.c
 ********************************************************************************
 */

#ifndef INC_BUZZER_HANDLER_H_
#define INC_BUZZER_HANDLER_H_
/************************************
 * INCLUDES
 ************************************/
#include "stm32f4xx_hal.h"
#include "main.h"
#include "stdbool.h"

/************************************
 * GLOBAL TYPEDEFS
 ************************************/
typedef struct{
	TIM_HandleTypeDef *htim10;
}BUZ_HandleTypeDef_t;


/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/
void buzzer_init(TIM_HandleTypeDef *htim10, BUZ_HandleTypeDef_t *hbuz);
void buzzer_enable(BUZ_HandleTypeDef_t *hbuz);
void buzzer_disable(BUZ_HandleTypeDef_t *hbuz);

#endif /* INC_BUZZER_HANDLER_H_ */
