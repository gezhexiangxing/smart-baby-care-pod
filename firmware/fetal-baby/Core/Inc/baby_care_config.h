#ifndef BABY_CARE_CONFIG_H
#define BABY_CARE_CONFIG_H

#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Integration switches.
 * Set a module to 0 if the matching peripheral is not enabled in CubeMX yet.
 */
#define BABY_CARE_USE_DHT11          1
#define BABY_CARE_USE_DC_MOTOR       1
#define BABY_CARE_USE_SERVO_GIMBAL   1
#define BABY_CARE_USE_VOICE_UART     1
#define BABY_CARE_USE_DEBUG_UART     1

/*
 * Default demo hardware mapping.
 * Replace these definitions after the real STM32H7 pin table is finalized.
 */
#define BABY_CARE_DHT11_PORT         GPIOB
#define BABY_CARE_DHT11_PIN          GPIO_PIN_0

#define BABY_CARE_MOTOR_TIM          htim2
#define BABY_CARE_MOTOR_CHANNEL      TIM_CHANNEL_3
#define BABY_CARE_MOTOR_IN1_PORT     GPIOA
#define BABY_CARE_MOTOR_IN1_PIN      GPIO_PIN_4
#define BABY_CARE_MOTOR_IN2_PORT     GPIOA
#define BABY_CARE_MOTOR_IN2_PIN      GPIO_PIN_5

#define BABY_CARE_GIMBAL_PAN_TIM     htim3
#define BABY_CARE_GIMBAL_PAN_CH      TIM_CHANNEL_1
#define BABY_CARE_GIMBAL_TILT_TIM    htim3
#define BABY_CARE_GIMBAL_TILT_CH     TIM_CHANNEL_2

#define BABY_CARE_VOICE_UART         huart2
#define BABY_CARE_DEBUG_UART         huart1

#define BABY_CARE_SENSOR_PERIOD_MS   2000U
#define BABY_CARE_REPORT_PERIOD_MS   2000U
#define BABY_CARE_SOOTHE_PERIOD_MS   800U

#if BABY_CARE_USE_DC_MOTOR
extern TIM_HandleTypeDef BABY_CARE_MOTOR_TIM;
#endif

#if BABY_CARE_USE_SERVO_GIMBAL
extern TIM_HandleTypeDef BABY_CARE_GIMBAL_PAN_TIM;
extern TIM_HandleTypeDef BABY_CARE_GIMBAL_TILT_TIM;
#endif

#if BABY_CARE_USE_VOICE_UART
extern UART_HandleTypeDef BABY_CARE_VOICE_UART;
#endif

#if BABY_CARE_USE_DEBUG_UART
extern UART_HandleTypeDef BABY_CARE_DEBUG_UART;
#endif

#ifdef __cplusplus
}
#endif

#endif
