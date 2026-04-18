/**
 ********************************************************************************
 * @file    measurement.h
 * @author  rathgden
 * @date  	27. 12. 2024
 * @brief   See measurement.c
 * Header file for ADC and TIM measurement management
 *
 ********************************************************************************
 */

#ifndef INC_MEASUREMENT_H_
#define INC_MEASUREMENT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/************************************
 * INCLUDES
 ************************************/
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "log.h"
#include "error_handler.h"
#include <stdlib.h>
#include "arm_math.h"
#include "main.h"

/************************************
 * GLOBAL DEFINES
 ************************************/
/* Fixed parameters for ADC configuration */
#define MEAS_ADC_CLOCK_PRESCALER ADC_CLOCK_SYNC_PCLK_DIV2    /**< ADC clock prescaler */
#define MEAS_ADC_RESOLUTION ADC_RESOLUTION_12B                 /**< ADC resolution: 12 bits */
#define MEAS_ADC_SCAN_MODE DISABLE                            /**< disable ADC scan mode */
#define MEAS_ADC_CONTINUOUS_MODE DISABLE                       /**< Disable continuous conversion mode */
#define MEAS_ADC_DISCONTINUOUS_MODE DISABLE                    /**< Disable discontinuous conversion mode */
#define MEAS_ADC_DATA_ALIGNMENT ADC_DATAALIGN_RIGHT            /**< Align ADC data to the right */
#define MEAS_ADC_EOC_SELECTION ADC_EOC_SINGLE_CONV             /**< End of Conversion selection: single conversion */
#define MEAS_ADC_EXTERNAL_EDGE ADC_EXTERNALTRIGCONVEDGE_RISING /**< Trigger conversion on rising edge */
#define MEAS_ADC_EXTERNAL_TRIGGER ADC_EXTERNALTRIGCONV_T8_TRGO /**< External trigger: Timer 2 TRGO */
#define MEAS_ADC_SAMPLING_TIME ADC_SAMPLETIME_15CYCLES         /**< ADC sampling time: 15 cycles */
#define MEAS_ADC_NUM_OF_CONVERSIONS 1                          /**< Number of ADC conversions */

/* ADC channels */
#define MEAS_Q_INSTANCE ADC1
#define MEAS_ADC_Q_CHANNEL ADC_CHANNEL_11 /**< ADC channel for I  */

#define MEAS_I_INSTANCE ADC2
#define MEAS_ADC_I_CHANNEL ADC_CHANNEL_13 /**< ADC channel for Q */


/* Buffer size */
#define MEAS_SAMPLE_BUFFER_LENGTH (256*2)     /**< Total buffer length for ADC samples */
#define MEAS_FFT_BUF_SIZE 16 //how many samples fit into the buffer


/* Logging modes */
#define MEAS_LOG_NONE 0                     /**< No logging */
#define MEAS_LOG_FFT 1                      /**< Log FFT data */
#define MEAS_LOG_SAMPLES 2                  /**< Log raw samples */
#define MEAS_LOG_MEASUREMENTS 3             /**< Log measurements */
#define MEAS_LOG_MODE MEAS_LOG_NONE /**< Current logging mode */



    /************************************
     * GLOBAL ENUMS
     ************************************/
    /**
     * States for Measurement state machine
     */
    typedef enum
    {
        MEAS_SAMPLING = 0,   /**< SAMPLING state */
		MEAS_PROCESSING = 1,
        MEAS_WAITING = 2     /**< WAITING state */
    } MEAS_State_t;


    /************************************
     * GLOBAL TYPEDEFS
     ************************************/
    typedef struct{
    	uint8_t *fft_cmplt; //0 = not  started, 1 = started, 2 = complete ,array for each fft buff element element
    	uint8_t *sig_sel; //0 = None(not valid measurement), 1 = left sensor, 2 = right sensor;,array for each fft buff element element
    	uint32_t *sample_buf;
    	float32_t *fft_buf;
    	uint16_t fft_buf_index;
    }MEAS_Data_t;
    /**
     * @struct MEAS_HandleTypeDef_t
     * @brief  This structure contains all the necessary information for handling measurements,
     * including ADC and Timer handles, buffer indices, measurement results, and state
     * machine states.
     *
     */
    typedef struct
    {
    	ADC_HandleTypeDef *hadc_q;       /**< ADC handle */
        ADC_HandleTypeDef *hadc_i;       /**< ADC handle */
        TIM_HandleTypeDef *htim;       /**< Timer handle */
        MEAS_Data_t data;
        MEAS_State_t state;            /**< Current state of the measurement state machine */
        MEAS_State_t last_state;       /**< Previous state of the measurement state machine */
    } MEAS_HandleTypeDef_t;

    /************************************
     * GLOBAL FUNCTION PROTOTYPES
     ************************************/
    Error_t meas_init(MEAS_HandleTypeDef_t *hmeas, ADC_HandleTypeDef *hadc_q,
    		ADC_HandleTypeDef *hadc_i);
    Error_t meas_task(MEAS_HandleTypeDef_t *hmeas);
    Error_t meas_stop_Sampling(MEAS_HandleTypeDef_t *hmeas);
    Error_t meas_start_Sampling(MEAS_HandleTypeDef_t *hmeas);
    void meas_setTaskHandle(osThreadId_t taskHandle);

#ifdef __cplusplus
}
#endif

#endif /* INC_MEASUREMENT_H_ */
