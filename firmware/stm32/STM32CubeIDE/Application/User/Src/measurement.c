/**
 ********************************************************************************
 * @file    measurement.c
 * @author  rathgden
 * @date    27. 12. 2024
 * @brief   Source file for ADC measurement management
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "measurement.h"
#include <stdbool.h>

extern DMA_HandleTypeDef hdma_adc1;

/************************************
 * LOCAL VARIABLES
 ************************************/
static uint8_t measurement_complete = 0;
static uint8_t current_sample_half = 0;
static osThreadId_t local_taskHandle;

/*alocate buffers for fft in sdram to safe stack size*/
__attribute__((section(".sdram"), aligned(4))) uint32_t sampleBuffer[MEAS_SAMPLE_BUFFER_LENGTH] =
	{}; /**< Array buffer for ADC samples */

__attribute__((section(".sdram"), aligned(4))) float32_t fftBuffer[MEAS_SAMPLE_BUFFER_LENGTH * MEAS_FFT_BUF_SIZE] =
	{}; /**< Array buffer for ADC samples */
static uint8_t fft_cmplt[MEAS_FFT_BUF_SIZE] = {};
static uint8_t sig_sel[MEAS_FFT_BUF_SIZE] = {};

static arm_cfft_instance_f32 cfft __attribute__((section(".sdram")));

static uint8_t fft_initialized = 0;



//-------------INIT FUNCTIONS --------------------------------------
static Error_t meas_init_adc_Q(MEAS_HandleTypeDef_t *hmeas)
{ // ADC1
	if (hmeas == NULL || hmeas->hadc_i == NULL)
	{
		RETURN_ERROR("ADC init failed, hmeas or hmeas->hadc empty");
	}

	ADC_HandleTypeDef *hadc = hmeas->hadc_q;

	ADC_MultiModeTypeDef multimode = {0};
	ADC_ChannelConfTypeDef sConfig = {0};

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc->Instance = MEAS_Q_INSTANCE;
	hadc->Init.ClockPrescaler = MEAS_ADC_CLOCK_PRESCALER;
	hadc->Init.Resolution = MEAS_ADC_RESOLUTION;
	hadc->Init.ScanConvMode = MEAS_ADC_SCAN_MODE;
	hadc->Init.ContinuousConvMode = MEAS_ADC_CONTINUOUS_MODE;
	hadc->Init.DiscontinuousConvMode = MEAS_ADC_DISCONTINUOUS_MODE;
	hadc->Init.ExternalTrigConvEdge = MEAS_ADC_EXTERNAL_EDGE;
	hadc->Init.ExternalTrigConv = MEAS_ADC_EXTERNAL_TRIGGER;
	hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc->Init.NbrOfConversion = 1;
	hadc->Init.DMAContinuousRequests = ENABLE;
	hadc->Init.EOCSelection = DISABLE;
	exec_hal(HAL_ADC_Init(hadc));

	/** Configure the ADC multi-mode
	 */
	multimode.Mode = ADC_DUALMODE_REGSIMULT;
	multimode.DMAAccessMode = ADC_DMAACCESSMODE_2;
	multimode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES; // 13
	exec_hal(HAL_ADCEx_MultiModeConfigChannel(hadc, &multimode));

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = MEAS_ADC_Q_CHANNEL;
	sConfig.Rank = 1;
	sConfig.SamplingTime = MEAS_ADC_SAMPLING_TIME; // 28
	exec_hal(HAL_ADC_ConfigChannel(hadc, &sConfig));

	RETURN_OK();
}

static Error_t meas_init_adc_I(MEAS_HandleTypeDef_t *hmeas)
{ // ADC2
	if (hmeas == NULL || hmeas->hadc_i == NULL)
	{
		RETURN_ERROR("ADC init failed, hmeas or hmeas->hadc empty");
	}

	ADC_HandleTypeDef *hadc = hmeas->hadc_i;

	ADC_ChannelConfTypeDef sConfig = {0};

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc->Instance = MEAS_I_INSTANCE;
	hadc->Init.ClockPrescaler = MEAS_ADC_CLOCK_PRESCALER;
	hadc->Init.Resolution = MEAS_ADC_RESOLUTION;
	hadc->Init.ScanConvMode = MEAS_ADC_SCAN_MODE;
	hadc->Init.ContinuousConvMode = DISABLE;
	hadc->Init.DiscontinuousConvMode = MEAS_ADC_DISCONTINUOUS_MODE;
	// hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
	// hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc->Init.NbrOfConversion = 1;
	hadc->Init.DMAContinuousRequests = DISABLE;
	hadc->Init.EOCSelection = DISABLE;

	exec_hal(HAL_ADC_Init(hadc));

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = MEAS_ADC_I_CHANNEL;
	sConfig.Rank = 1;
	sConfig.SamplingTime = MEAS_ADC_SAMPLING_TIME;
	exec_hal(HAL_ADC_ConfigChannel(hadc, &sConfig));

	RETURN_OK();
}

static void meas_init_dma(void)
{
	__HAL_RCC_DMA2_CLK_ENABLE();

	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

Error_t meas_init(MEAS_HandleTypeDef_t *hmeas, ADC_HandleTypeDef *hadc_q,
				  ADC_HandleTypeDef *hadc_i)
{

	if (hmeas == NULL || hadc_i == NULL || hadc_q == NULL)
	{
		RETURN_ERROR("init failed, passed empty handle");
	}

	memset(&fftBuffer, 0, MEAS_SAMPLE_BUFFER_LENGTH * MEAS_FFT_BUF_SIZE * sizeof(float32_t));
	memset(&sampleBuffer, 0, MEAS_SAMPLE_BUFFER_LENGTH* sizeof(uint32_t));

	MEAS_Data_t data;
	data.fft_buf = fftBuffer;
	data.fft_buf_index = 0;
	data.sample_buf = sampleBuffer;
	data.fft_cmplt = fft_cmplt;
	data.sig_sel = sig_sel;
	hmeas->data = data;

	hmeas->hadc_i = hadc_i;
	hmeas->hadc_q = hadc_q;
	hmeas->state = MEAS_WAITING;
	hmeas->last_state = MEAS_WAITING;

	exec(meas_init_adc_I(hmeas));
	//__HAL_ADC_ENABLE(hmeas->hadc_i);
	exec(meas_init_adc_Q(hmeas));

	meas_init_dma();

	// ADC->CCR = (ADC->CCR & ~0xF) | 0x5;

	/*volatile uint32_t counter = (ADC_STAB_DELAY_US
			* (SystemCoreClock / 1000000U));
	while (counter != 0U) {
		counter--;
	}
	//meas_start_Sampling(hmeas);*/
	RETURN_OK();
}

void meas_setTaskHandle(osThreadId_t taskHandle)
{
	local_taskHandle = taskHandle;
}
//------------- --------------------------------------
//-------------FFT FUNCTIONS --------------------------------------

static void meas_inc_buf_index(MEAS_HandleTypeDef_t *hmeas)
{
	if (hmeas->data.fft_buf_index < MEAS_FFT_BUF_SIZE - 1)
	{
		hmeas->data.fft_buf_index++;
	}
	else
	{
		hmeas->state = MEAS_WAITING;
		hmeas->data.fft_buf_index = 0;
	}
}
// transfers sample buffer to fft buffer at current bufferindex(start/stop relative to it). converts to float and takes offset out
static Error_t meas_sample2fft_buf(MEAS_HandleTypeDef_t *hmeas, uint16_t startIndex, uint16_t stopIndex)
{
	if (startIndex > stopIndex)
	{
		RETURN_ERROR("start index bigegr eq than stop index");
	}
	uint16_t *iq_data = (uint16_t *)hmeas->data.sample_buf;

	if (startIndex >= stopIndex || stopIndex > MEAS_SAMPLE_BUFFER_LENGTH)
	{
		RETURN_ERROR("invalid start/stop index");
	}

	// calc offsett
	float32_t avg_I = 0; // I is LSB
	float32_t avg_Q = 0; // Q is MSB

	for (uint16_t i = startIndex; i <= stopIndex; ++i)
	{
		avg_I += iq_data[2 * i];
		avg_Q += iq_data[2 * i + 1];
	}

	uint16_t numSamples = stopIndex - startIndex + 1;
	avg_I /= numSamples;
	avg_Q /= numSamples;

	// convert and move to fft buffer

	for (uint16_t i = startIndex; i <= stopIndex; ++i)
	{
		uint16_t fftIndex = ((hmeas->data.fft_buf_index * MEAS_SAMPLE_BUFFER_LENGTH/2) + i - startIndex) * 2; // offset by bufferindex
		hmeas->data.fft_buf[fftIndex] = (float32_t)iq_data[2 * i] - avg_I;									 // I
		hmeas->data.fft_buf[fftIndex + 1] = (float32_t)iq_data[2 * i + 1] - avg_Q;							 // Q
	}

	RETURN_OK();
}

Error_t meas_compute_fft(float32_t *input_buf, float32_t *output_buf,
						 uint16_t buf_len)
{

	if (input_buf == NULL || output_buf == NULL)
	{
		RETURN_ERROR("Null pointer passed to FFT function");
	}

	// Ensure the buffer length is a power of 2 (required for FFT)
	if ((buf_len & (buf_len - 1)) != 0)
	{
		RETURN_ERROR("Buffer length must be a power of 2");
	}

	// Initialize FFT instance
	if (arm_cfft_init_f32(&cfft, buf_len) != ARM_MATH_SUCCESS)
	{
		RETURN_ERROR("FFT initialization failed");
	}
	fft_initialized = 1;

	// Perform FFT
	// arm_cfft_f32(S, p1, ifftFlag, bitReverseFlag)(&rfft, input_buf, output_buf, 0);

	/*// Compute magnitude of the FFT
	 arm_cmplx_mag_f32(output_fft, magnitudes_fft, buf_len / 2);

	 // Compute magnitude for 50 Hz
	 uint16_t bin_index = (uint16_t) ((MEAS_FFT_TARGET_FREQ * buf_len)
	 / MEAS_SAMPLE_FREQ);
	 *magnitude = magnitudes_fft[bin_index] / buf_len;
	 */
	RETURN_OK();
}

//---------------------------------------------------
//-------------SAMPLING FUNCTIONS --------------------------------------
Error_t meas_start_Sampling(MEAS_HandleTypeDef_t *hmeas)
{
	hmeas->state = MEAS_SAMPLING;
	__HAL_ADC_ENABLE(hmeas->hadc_i);
	volatile uint32_t counter = (ADC_STAB_DELAY_US * (SystemCoreClock / 1000000U));
	while (counter != 0U)
	{
		counter--;
	}
	exec_hal(
		HAL_ADCEx_MultiModeStart_DMA(hmeas->hadc_q, (uint32_t *)sampleBuffer, MEAS_SAMPLE_BUFFER_LENGTH));
	RETURN_OK();
}

Error_t meas_stop_Sampling(MEAS_HandleTypeDef_t *hmeas)
{
	hmeas->state = MEAS_WAITING;
	__HAL_ADC_DISABLE(hmeas->hadc_i);
	if (hmeas->hadc_i->DMA_Handle->State == HAL_DMA_STATE_BUSY)
	{
		exec_hal(HAL_ADCEx_MultiModeStop_DMA(hmeas->hadc_q));
	}
	RETURN_OK();
}
Error_t meas_process_samples(MEAS_HandleTypeDef_t *hmeas)
{

	if (hmeas == NULL)
	{
		RETURN_ERROR("meas_process_samples failed, null pointer passed");
	}
	// TODO implement fft
	// exec(meas_compute_fft(pad_r_buf, &hmeas->mag_pad_r, buf_len));

	// fft_initialized = 0;

	RETURN_OK();
}

Error_t meas_task(MEAS_HandleTypeDef_t *hmeas)
{
	if (hmeas == NULL)
	{
		RETURN_ERROR("meas_task failed, hmeas handle empty");
	}

	if (measurement_complete && hmeas->state != MEAS_WAITING)
	{
		hmeas->state = MEAS_PROCESSING;
		measurement_complete = 0;
	}

	switch (hmeas->state)
	{
	case MEAS_SAMPLING:
		if (hmeas->last_state == MEAS_SAMPLING)
				break;

		hmeas->last_state = MEAS_SAMPLING;
		break;
	case MEAS_PROCESSING:
		if (hmeas->last_state == MEAS_PROCESSING)
			break;

		uint16_t start = current_sample_half ? MEAS_SAMPLE_BUFFER_LENGTH / 2 : 0;
		uint16_t stop = current_sample_half ? MEAS_SAMPLE_BUFFER_LENGTH : MEAS_SAMPLE_BUFFER_LENGTH / 2;
		meas_sample2fft_buf(hmeas, start, stop-1);
		meas_inc_buf_index(hmeas);
		//hmeas->last_state = MEAS_PROCESSING; no need for last state
		hmeas->state = MEAS_SAMPLING;
		break;
	case MEAS_WAITING:
		if (hmeas->last_state == MEAS_WAITING)
			break;

		hmeas->last_state = MEAS_WAITING;
		break;
	default:
		break;
	}
	RETURN_OK();
}
//---------------------------------------------------

//-------------CALLBACK & MSP INIT FUNCTIONS --------------------------------------
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	measurement_complete = 1;
	current_sample_half = 1;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(local_taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	measurement_complete = 1;
	current_sample_half = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(local_taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (hadc->Instance == ADC1)
	{
		/* USER CODE BEGIN ADC1_MspInit 0 */

		/* USER CODE END ADC1_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_ADC1_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**ADC1 GPIO Configuration
		 PC1     ------> ADC1_IN11
		 */
		GPIO_InitStruct.Pin = SIGNAL_Q_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(SIGNAL_Q_GPIO_Port, &GPIO_InitStruct);

		/* ADC1 DMA Init */
		/* ADC1 Init */
		hdma_adc1.Instance = DMA2_Stream0;
		hdma_adc1.Init.Channel = DMA_CHANNEL_0;
		hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma_adc1.Init.Mode = DMA_CIRCULAR;
		hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		exec_hal(HAL_DMA_Init(&hdma_adc1));

		__HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
	}
	else if (hadc->Instance == ADC2)
	{
		/* USER CODE BEGIN ADC2_MspInit 0 */

		/* USER CODE END ADC2_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_ADC2_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**ADC2 GPIO Configuration
		 PC3     ------> ADC2_IN13
		 */
		GPIO_InitStruct.Pin = SIGNAL_I_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(SIGNAL_I_GPIO_Port, &GPIO_InitStruct);

		/* USER CODE BEGIN ADC2_MspInit 1 */

		/* USER CODE END ADC2_MspInit 1 */
	}
}

//------------- --------------------------------------
