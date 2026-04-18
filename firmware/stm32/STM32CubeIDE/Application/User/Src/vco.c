/*
 * vco.c
 *
 *  Created on: Mar 24, 2025
 *      Author: Dennis Rathgeb
 */

#include "vco.h"

//DAC CONFIG
#define DAC_MAX_VALUE 4096 //12Bit
#define DAC_OUTPUT_BUF DAC_OUTPUTBUFFER_DISABLE
#define DAC_CHANNEL DAC_CHANNEL_2
#define DAC_ALIGN DAC_ALIGN_12B_R


//private vars
#define RAMP_BUF_LEN 256

uint16_t ramp_buf[RAMP_BUF_LEN];


uint8_t vco_n_instances = 0;

extern DMA_HandleTypeDef hdma_dac2;

//Forward declarations
static Error_t init_dac(VCO_HandleTypeDef_t *hvco);
static void generate_ramp(VCO_HandleTypeDef_t *hvco);

void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdac->Instance==DAC)
  {
    /* Peripheral clock enable */
    __HAL_RCC_DAC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DAC GPIO Configuration
    PA5     ------> DAC_OUT2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* DAC DMA Init */
    /* DAC2 Init */
    hdma_dac2.Instance = DMA1_Stream6;
    hdma_dac2.Init.Channel = DMA_CHANNEL_7;
    hdma_dac2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_dac2.Init.Mode = DMA_CIRCULAR;
    hdma_dac2.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dac2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    exec_hal(HAL_DMA_Init(&hdma_dac2));

    __HAL_LINKDMA(hdac,DMA_Handle2,hdma_dac2);

    /* DAC interrupt Init */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

  }

}

//init dac peripheral
static Error_t init_dac(VCO_HandleTypeDef_t *hvco) {

	DAC_HandleTypeDef *hdac = hvco->hdac;
	DAC_ChannelConfTypeDef sConfig = { 0 };

	/** DAC Initialization
	 */
	hdac->Instance = DAC;
	/* Check the parameters */
	exec_hal(HAL_DAC_Init(hdac));

	/** DAC channel OUT2 config
	 */
	sConfig.DAC_Trigger = hvco->trigger;
	sConfig.DAC_OutputBuffer = DAC_OUTPUT_BUF;
	exec_hal(HAL_DAC_ConfigChannel(hdac, &sConfig, DAC_CHANNEL_2));

	RETURN_OK();
}

//init vco
Error_t VCO_Init(VCO_HandleTypeDef_t *hvco, DAC_HandleTypeDef *hdac, uint16_t ramp_steps, uint32_t trigger) {
	if (hdac == NULL){
		RETURN_ERROR("empty adc handle passed");
	}
	if (hvco == NULL){
		RETURN_ERROR("empty hvco handle passed");
	}

	if(ramp_steps > RAMP_BUF_LEN){
		RETURN_ERROR("adjust local buf length");
	}
	//catch multiple instance creations.
	if (vco_n_instances > VCO_MAX_INSTANCES) {
		RETURN_ERROR("max vco instances exceeded");
	}
	vco_n_instances++;

	hvco->hdac = hdac;
	hvco->ramp_steps = ramp_steps;

	hvco->trigger = trigger;

	generate_ramp(hvco);

	exec(init_dac(hvco));
	//HAL_TIM_Base_Start(htim);


	RETURN_OK();

}

Error_t VCO_start(VCO_HandleTypeDef_t *hvco){
	if (hvco == NULL || hvco->hdac == NULL) {
	        RETURN_ERROR("VCO handle is NULL");
	    }
	exec_hal(HAL_DAC_Start_DMA(hvco->hdac, DAC_CHANNEL, (uint32_t*)ramp_buf, (uint32_t)hvco->ramp_steps, DAC_ALIGN_12B_R));
}

Error_t VCO_stop(VCO_HandleTypeDef_t *hvco) {
    if (hvco == NULL || hvco->hdac == NULL) {
        RETURN_ERROR("VCO handle is NULL");
    }

    // Stop DAC DMA for the configured channel
    exec_hal(HAL_DAC_Stop_DMA(hvco->hdac, DAC_CHANNEL));

    // Optionally stop the DAC itself too (if needed)
    //exec_hal(HAL_DAC_Stop(hvco->hdac, DAC_CHANNEL));

    RETURN_OK();
}


//generate ramp
static void generate_ramp(VCO_HandleTypeDef_t *hvco) {
	for (int i = 1; i < (hvco->ramp_steps); i++) {
		ramp_buf[i-1] = (uint32_t)((float)(i) * DAC_MAX_VALUE / (hvco->ramp_steps - 1));
	}
	ramp_buf[hvco->ramp_steps-1] = 0;
}
