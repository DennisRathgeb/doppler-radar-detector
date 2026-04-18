/*
 * timing.c
 *
 *  Created on: Apr 7, 2025
 *      Author: Dennis Rathgeb
 */

#include "timing.h"

uint8_t timing_n_instances = 0;
//master TIM
static Error_t init_tim_signal(Timing_HandleTypeDef_t *htiming) {

	TIM_HandleTypeDef *htim = htiming->htim_sig;

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_SlaveConfigTypeDef sSlaveConfig = { 0 };

	htim->Instance = TIM8;
	htim->Init.Prescaler = 0;
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim->Init.Period = TIMING_SAMP_PERIOD;
	htim->Init.RepetitionCounter = TIMING_N_STEPS - 1;
	htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	exec_hal(HAL_TIM_Base_Init(htim));


	TIM_OC_InitTypeDef oc = {0};
	oc.OCMode = TIM_OCMODE_ACTIVE;
	oc.Pulse = TIMING_SAMP_PERIOD-1;
	oc.OCPolarity = TIM_OCPOLARITY_HIGH;
	oc.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(htim, &oc, TIM_CHANNEL_1);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	exec_hal(HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig));
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	exec_hal(HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig));

	// TIM1 waits for TIM2 trigger (ITRx)
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
	sSlaveConfig.InputTrigger = TIM_TS_ITR1; //RM090 p.570:  ITR1 = TIM2_TRGO
	exec_hal(HAL_TIM_SlaveConfigSynchro(htim, &sSlaveConfig));

	// Fully disable TIM8 to prevent early triggering
	__HAL_TIM_DISABLE(htim);                // Disable counter
	__HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE); // Disable update interrupt if any
	__HAL_TIM_SET_COUNTER(htim, 0);
       // Reset counter to zero

	htim->Instance->BDTR &= ~TIM_BDTR_MOE;

	RETURN_OK();
}

//SIGNAL TIM
static Error_t init_tim_master(Timing_HandleTypeDef_t *htiming) {

	TIM_HandleTypeDef *htim = htiming->htim_mas;

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	htim->Instance = TIM2;
	htim->Init.Prescaler = 0;
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim->Init.Period = TIMING_SWEEP_PERIOD + TIMING_PAUSE_PERIOD;
	htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	exec_hal(HAL_TIM_Base_Init(htim));



	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	exec_hal(HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig));
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	exec_hal(HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig));

	RETURN_OK();
}

Error_t Timing_Init(Timing_HandleTypeDef_t *htiming,
		TIM_HandleTypeDef *htim_mas, TIM_HandleTypeDef *htim_sig) {
	if (htiming == NULL || htim_mas == NULL || htim_sig == NULL) {
		RETURN_ERROR("passed empty handles!");
	}
	//catch multiple instance creations.
	if (timing_n_instances > TIMING_MAX_INSTANCES) {
		RETURN_ERROR("max vco instances exceeded");
	}
	timing_n_instances++;

	htiming->htim_mas = htim_mas;
	htiming->htim_sig = htim_sig;


	exec(init_tim_master(htiming));
	exec(init_tim_signal(htiming));

	RETURN_OK();

}
Error_t Timing_start_seq(Timing_HandleTypeDef_t *htiming) {
    if (htiming == NULL || htiming->htim_mas == NULL || htiming->htim_sig == NULL) {
        RETURN_ERROR("Invalid timer handle");
    }
	exec_hal(HAL_TIM_OnePulse_Init(htiming->htim_sig, TIM_OPMODE_SINGLE));
	__HAL_TIM_SET_COUNTER(htiming->htim_sig, 0);
	exec_hal(HAL_TIM_Base_Start(htiming->htim_mas));
	RETURN_OK();

}

Error_t Timing_stop_seq(Timing_HandleTypeDef_t *htiming) {
    if (htiming == NULL || htiming->htim_mas == NULL || htiming->htim_sig == NULL) {
        RETURN_ERROR("Invalid timer handle");
    }

    // Stop the master timer
    exec_hal(HAL_TIM_Base_Stop(htiming->htim_mas));

    // Stop the signal/slave timer (optional but good for symmetry)
    exec_hal(HAL_TIM_Base_Stop(htiming->htim_sig));

    // Also reset counters if needed
    __HAL_TIM_SET_COUNTER(htiming->htim_mas, 0);
    __HAL_TIM_SET_COUNTER(htiming->htim_sig, 0);

    RETURN_OK();
}

/*void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim_base) {
	if (htim_base->Instance == TIM8) {

		__HAL_RCC_TIM8_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);

	} else if (htim_base->Instance == TIM2) {

		__HAL_RCC_TIM2_CLK_ENABLE();

	}

}*/
