#include "esp.h"


static SPI_HandleTypeDef *hspi_callback;
static uint8_t spi_tx_busy;

uint8_t SPI_Send_DMA(ESP_HandleTypeDef_t *hesp, uint8_t *tx_buf, uint16_t size) {
	if (spi_tx_busy)
		return 1; // busy

	if (HAL_SPI_Transmit_DMA(hesp->hspi, tx_buf, size) == HAL_OK) {
		spi_tx_busy = 1;
		return 0; // success
	}

	return 2; // failure
}

//Hal init spi
Error_t esp_spi_init(ESP_HandleTypeDef_t *hesp) {

	SPI_HandleTypeDef *hspi = hesp->hspi;
	/* SPI3 parameter configuration*/
	hspi->Instance = SPI3;
	hspi->Init.Mode = SPI_MODE_MASTER;
	hspi->Init.Direction = SPI_DIRECTION_2LINES;
	hspi->Init.DataSize = SPI_DATASIZE_8BIT;
	hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi->Init.NSS = SPI_NSS_HARD_OUTPUT;
	hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi->Init.TIMode = SPI_TIMODE_DISABLE;
	hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi->Init.CRCPolynomial = 10;
	exec_hal(HAL_SPI_Init(hspi));

	RETURN_OK();
	//set priority for dma stream
	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
	/* DMA1_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}
//init esp
Error_t ESP_Init(ESP_HandleTypeDef_t *hesp, SPI_HandleTypeDef *hspi) {

	hesp->esp_enabled = 0;
	spi_tx_busy = 0;
	hesp->hspi = hspi;
	hspi_callback = hspi;

	exec(esp_spi_init(hesp));

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : ESP_EN_Pin */
	GPIO_InitStruct.Pin = ESP_EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ESP_EN_GPIO_Port, &GPIO_InitStruct);


	RETURN_OK();
}

void esp_enable(ESP_HandleTypeDef_t *hesp){
	if(!hesp->esp_enabled){
		HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, 1);
		hesp->esp_enabled = 1;
	}
}

void esp_disable(ESP_HandleTypeDef_t *hesp){
	if(hesp->esp_enabled){
		HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, 0);
		hesp->esp_enabled = 0;
	}
}

//Callbacks
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {


	if (hspi == hspi_callback) {
		spi_tx_busy = 0;
		// Optionally: signal task, toggle GPIO, etc.
	}
}
