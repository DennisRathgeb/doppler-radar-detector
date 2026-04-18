/*
 * esp.h
 *
 *  Created on: Apr 8, 2025
 *      Author: burni
 */
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "error_handler.h"
#include "log.h"

#ifndef INC_ESP_H_
#define INC_ESP_H_

typedef struct{
	SPI_HandleTypeDef *hspi;
	uint8_t esp_enabled;


}ESP_HandleTypeDef_t;


Error_t ESP_Init(ESP_HandleTypeDef_t *hesp, SPI_HandleTypeDef *hspi);
uint8_t SPI_Send_DMA(ESP_HandleTypeDef_t *hesp, uint8_t *tx_buf, uint16_t size);



#endif /* INC_ESP_H_ */
