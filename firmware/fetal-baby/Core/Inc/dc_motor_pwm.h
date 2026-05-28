#ifndef DC_MOTOR_PWM_H7_H
#define DC_MOTOR_PWM_H7_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DC_MOTOR_OK = 0,
    DC_MOTOR_ERROR_PARAM,
    DC_MOTOR_ERROR_HAL
} DC_Motor_StatusTypeDef;

typedef enum
{
    DC_MOTOR_STOP_COAST = 0,
    DC_MOTOR_STOP_BRAKE
} DC_Motor_StopModeTypeDef;

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;

    GPIO_TypeDef *in1_port;
    uint16_t in1_pin;
    GPIO_TypeDef *in2_port;
    uint16_t in2_pin;

    int8_t speed;
    DC_Motor_StopModeTypeDef stop_mode;
} DC_Motor_HandleTypeDef;

DC_Motor_StatusTypeDef DC_Motor_Init(DC_Motor_HandleTypeDef *hmotor,
                                     TIM_HandleTypeDef *htim,
                                     uint32_t channel,
                                     GPIO_TypeDef *in1_port,
                                     uint16_t in1_pin,
                                     GPIO_TypeDef *in2_port,
                                     uint16_t in2_pin);

DC_Motor_StatusTypeDef DC_Motor_Start(DC_Motor_HandleTypeDef *hmotor);
DC_Motor_StatusTypeDef DC_Motor_Stop(DC_Motor_HandleTypeDef *hmotor);
DC_Motor_StatusTypeDef DC_Motor_SetSpeed(DC_Motor_HandleTypeDef *hmotor, int8_t speed);
void DC_Motor_SetStopMode(DC_Motor_HandleTypeDef *hmotor, DC_Motor_StopModeTypeDef stop_mode);
int8_t DC_Motor_GetSpeed(const DC_Motor_HandleTypeDef *hmotor);

#ifdef __cplusplus
}
#endif

#endif
