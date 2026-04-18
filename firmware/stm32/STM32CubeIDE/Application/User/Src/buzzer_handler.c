/**
 ********************************************************************************
 * @file    buzzer_handler.c
 * @author  uhlmabry
 * @date    27.12.2024
 * @brief   Control the buzzer frequency for a distance.
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "buzzer_handler.h"
#include "error_handler.h"
#include "main.h"

/************************************
 * PRIVATE TYPEDEFS
 ************************************/
#define CLOCKSPEED 		180000000
#define PRESCALER 		0xFFFF
#define COUNTER_FREQ 	CLOCKSPEED / PRESCALER
// Frquency in Hz
#define FREQUENCY		1000

/************************************
 * STATIC VARIABLES
 ************************************/
static bool buzzerebable = false;

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void MX_TIM10_Init(TIM_HandleTypeDef *htim10);

/************************************
 * GLOBAL FUNCTIONS
 ************************************/

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Enable the buzzer (enable TIM 9 PWM)
 ********************************************************************************
 **/
void buzzer_enable(BUZ_HandleTypeDef_t *hbuz)
{
    if(!buzzerebable){
    	HAL_TIM_PWM_Start(hbuz->htim10, TIM_CHANNEL_1);
    }
    buzzerebable = true;
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Disable the buzzer (disable TIM 9 PWM)
 ********************************************************************************
 **/
void buzzer_disable(BUZ_HandleTypeDef_t *hbuz)
{
	if(buzzerebable){
		HAL_TIM_PWM_Stop(hbuz->htim10, TIM_CHANNEL_1);
	}
    buzzerebable = false;
}

/**
 ********************************************************************************
 * @param TIM_HandleTypeDef* htim9
 * @return void
 * @brief Initialize the buzzer (TIM9 for PWM)
 ********************************************************************************
 **/
void buzzer_init(TIM_HandleTypeDef *htim10, BUZ_HandleTypeDef_t *hbuz)
{
	hbuz->htim10 = htim10;
	MX_TIM10_Init(hbuz->htim10);
	TIM10->ARR = COUNTER_FREQ / FREQUENCY;
	// Set PWM to 50%
	TIM10->CCR2 = (TIM9->ARR/2);
    HAL_TIM_PWM_Start(hbuz->htim10, TIM_CHANNEL_1);
}

/************************************
 * LOCAL FUNCTIONS
 ************************************/
/**
 * @brief TIM10 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM10_Init(TIM_HandleTypeDef *htim10){

	/* USER CODE BEGIN TIM10_Init 0 */

	/* USER CODE END TIM10_Init 0 */

	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM10_Init 1 */

	/* USER CODE END TIM10_Init 1 */
	htim10->Instance = TIM10;
	htim10->Init.Prescaler = 0xFFFF;
	htim10->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim10->Init.Period = 65535;
	htim10->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim10->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(htim10) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(htim10) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(htim10, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM10_Init 2 */

	/* USER CODE END TIM10_Init 2 */
	HAL_TIM_MspPostInit(htim10);

}
