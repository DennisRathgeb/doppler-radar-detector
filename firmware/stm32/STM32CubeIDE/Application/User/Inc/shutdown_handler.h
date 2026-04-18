/**
 ********************************************************************************
 * @file    shutdown_handler.h
 * @author  uhlmabry
 * @date  	27. 12. 2024
 * @brief   See shutdown_handler.c
 ********************************************************************************
 */
#ifndef INC_SHUTDOWN_H_
#define INC_SHUTDOWN_H_

/************************************
 * INCLUDES
 ************************************/
#include "stm32f4xx_hal.h"
#include "measurement.h"
#include "main.h"

/************************************
 * GLOBAL TYPEDEFS
 ************************************/
typedef struct{
	RTC_HandleTypeDef *hrtc;
}STDWN_HandleTypeDef_t;

/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/
void 		sys_check_shutdown 				(STDWN_HandleTypeDef_t *hstdwn);
void 		sys_init_shutdown 				(STDWN_HandleTypeDef_t *hstdwn, RTC_HandleTypeDef* hrtc);
void 		sys_enable_autoshutdown			(void);
void 		sys_disable_autoshutdown		(void);
void 		sys_reset_auto_shutdown_time	(STDWN_HandleTypeDef_t *hstdwn);
uint32_t 	sys_get_shutdown_time			(void);
void        sys_shutdown					(void);

#endif /* INC_SHUTDOWN_H_ */
