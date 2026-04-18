/**
 ********************************************************************************
 * @file    shutdown_handler.c
 * @author  uhlmabry
 * @date    27. 12. 2024
 * @brief   Control the shutdown process and turn-on process of the system.
 ********************************************************************************
 **/

/************************************
 * INCLUDES
 ************************************/
#include "shutdown_handler.h"
#include "stm32f4xx_it.h"
#include "main.h"
#include "error_handler.h"
#include <stdbool.h>

/************************************
 * LOCAL VARIABLES
 ************************************/
static RTC_TimeTypeDef 			startTime 				= {0};
static RTC_TimeTypeDef 			actualTime 				= {0};
static bool						system_auto_shutdown 	= true;

/************************************
 * MACROS AND DEFINES
 ************************************/
#define SHUTDOWNTIME 	   10	// in seconds
#define FINALSHUTDOWNTIME 120 	// in minutes

/************************************
 * LOCAL FUNCTIONS PROTOTYPES
 ************************************/
static void sys_enable(void);
static void MX_RTC_Init(RTC_HandleTypeDef *hrtc);
static void sys_get_actual_time(RTC_TimeTypeDef *time, STDWN_HandleTypeDef_t *hstdwne);

/************************************
 * LOCAL FUNCTIONS
 ************************************/
/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Enable the load switch.
 ********************************************************************************
 **/
static void sys_enable(void){
	HAL_GPIO_WritePin(EN_PCB_OUT_GPIO_Port, EN_PCB_OUT_Pin, GPIO_PIN_SET);
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Initialize the RTC for the shutdown timer.
 ********************************************************************************
 **/
static void MX_RTC_Init(RTC_HandleTypeDef *hrtc){
	/* Set default types */
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	RTC_AlarmTypeDef sAlarm = {0};

	/** Initialize RTC Only */
	hrtc->Instance = RTC;
	hrtc->Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc->Init.AsynchPrediv = 127;
	hrtc->Init.SynchPrediv = 255;
	hrtc->Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	exec_hal(HAL_RTC_Init(hrtc));

	/** Initialize RTC and set the Time and Date */
	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	exec_hal(HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BCD));

	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 0x1;
	sDate.Year = 0x0;
	exec_hal(HAL_RTC_SetDate(hrtc, &sDate, RTC_FORMAT_BCD));

	/** Enable the Alarm A */
	sAlarm.AlarmTime.Hours = 0x0;
	sAlarm.AlarmTime.Minutes = 0x0;
	sAlarm.AlarmTime.Seconds = 0x2;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 0x1;
	sAlarm.Alarm = RTC_ALARM_A;
	exec_hal(HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, RTC_FORMAT_BCD));
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Load in the actual time.
 ********************************************************************************
 **/
static void sys_get_actual_time(RTC_TimeTypeDef *time, STDWN_HandleTypeDef_t *hstdwn){
	time->Seconds = (hstdwn->hrtc->Instance->TR & (RTC_TR_ST | RTC_TR_SU));
	time->Minutes = (hstdwn->hrtc->Instance->TR & (RTC_TR_MNT | RTC_TR_MNU));
}


/************************************
 * GLOBAL FUNCTIONS
 ************************************/
/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Disable the load switch.
 ********************************************************************************
 **/
void sys_shutdown(void){
	HAL_GPIO_WritePin(EN_PCB_OUT_GPIO_Port, EN_PCB_OUT_Pin, GPIO_PIN_RESET);
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Disable the load switch.
 ********************************************************************************
 **/
void sys_enable_autoshutdown(void){
	system_auto_shutdown = true;
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Disable the load switch.
 ********************************************************************************
 **/
void sys_disable_autoshutdown(void){
	system_auto_shutdown = false;
}

/**
 ********************************************************************************
 * @param GPIO_Pin
 * @return void
 * @brief disable the load switch if a rising edge is detected on
 ********************************************************************************
 **/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == EN_PCB_IN_Pin){
		sys_shutdown();
	}
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief reset the time fot the autoshutdown
 ********************************************************************************
 **/
void sys_reset_auto_shutdown_time(STDWN_HandleTypeDef_t *hstdwn){
	sys_get_actual_time(&startTime, hstdwn);
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Shut down the whole system on the following events:
 *        - After a SHUTDOWNTIME is reached.
 *          The shutdown timer will be reset if the device measures
 *          a distance.
 ********************************************************************************
 **/
void sys_check_shutdown(STDWN_HandleTypeDef_t *hstdwn){
	// Auto-shutdown after SHUTDOWNTIME is reached
	if(system_auto_shutdown == true){
		sys_get_actual_time(&actualTime, hstdwn);
		uint32_t difference = sys_get_shutdown_time();
		if (difference >= SHUTDOWNTIME || actualTime.Minutes >= FINALSHUTDOWNTIME){
			sys_shutdown();
		}
	}
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief reset the time fot the autoshutdown
 ********************************************************************************
 **/
uint32_t sys_get_shutdown_time(void){
	uint32_t difference = (actualTime.Seconds + (60 * actualTime.Minutes )) -
						  (startTime.Seconds  + (60 * startTime.Minutes  ));
	return(difference);
}

/**
 ********************************************************************************
 * @param void
 * @return void
 * @brief Turn on the enable pin on the load switch and initialize the RTC for auto
 *        shutdown. Also get the start time of the shutdown timer.
 ********************************************************************************
 **/
void sys_init_shutdown(STDWN_HandleTypeDef_t *hstdwn, RTC_HandleTypeDef *hrtc){
	sys_enable();
	hstdwn->hrtc = hrtc;
	MX_RTC_Init(hrtc);
	startTime.Seconds = (hrtc->Instance->TR & (RTC_TR_ST | RTC_TR_SU));
}


