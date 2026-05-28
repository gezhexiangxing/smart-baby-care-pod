#include "dc_motor_pwm.h"

static void dc_motor_enable_gpio_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (port == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (port == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    } else if (port == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
#ifdef GPIOI
    else if (port == GPIOI) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
#endif
#ifdef GPIOJ
    else if (port == GPIOJ) {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
    }
#endif
#ifdef GPIOK
    else if (port == GPIOK) {
        __HAL_RCC_GPIOK_CLK_ENABLE();
    }
#endif
}

static uint8_t dc_motor_is_valid_channel(uint32_t channel)
{
    switch (channel) {
    case TIM_CHANNEL_1:
    case TIM_CHANNEL_2:
    case TIM_CHANNEL_3:
    case TIM_CHANNEL_4:
        return 1U;
#ifdef TIM_CHANNEL_5
    case TIM_CHANNEL_5:
        return 1U;
#endif
#ifdef TIM_CHANNEL_6
    case TIM_CHANNEL_6:
        return 1U;
#endif
    default:
        return 0U;
    }
}

static void dc_motor_gpio_init(DC_Motor_HandleTypeDef *hmotor)
{
    GPIO_InitTypeDef gpio = {0};

    dc_motor_enable_gpio_clock(hmotor->in1_port);
    dc_motor_enable_gpio_clock(hmotor->in2_port);

    gpio.Pin = hmotor->in1_pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hmotor->in1_port, &gpio);

    gpio.Pin = hmotor->in2_pin;
    HAL_GPIO_Init(hmotor->in2_port, &gpio);
}

static void dc_motor_set_pwm_percent(DC_Motor_HandleTypeDef *hmotor, uint8_t percent)
{
    uint32_t period;
    uint32_t compare;

    if (percent > 100U) {
        percent = 100U;
    }

    period = __HAL_TIM_GET_AUTORELOAD(hmotor->htim) + 1U;
    compare = (period * percent) / 100U;

    __HAL_TIM_SET_COMPARE(hmotor->htim, hmotor->channel, compare);
}

static void dc_motor_apply_direction(DC_Motor_HandleTypeDef *hmotor, int8_t speed)
{
    if (speed > 0) {
        HAL_GPIO_WritePin(hmotor->in1_port, hmotor->in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(hmotor->in2_port, hmotor->in2_pin, GPIO_PIN_RESET);
    } else if (speed < 0) {
        HAL_GPIO_WritePin(hmotor->in1_port, hmotor->in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(hmotor->in2_port, hmotor->in2_pin, GPIO_PIN_SET);
    } else if (hmotor->stop_mode == DC_MOTOR_STOP_BRAKE) {
        HAL_GPIO_WritePin(hmotor->in1_port, hmotor->in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(hmotor->in2_port, hmotor->in2_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(hmotor->in1_port, hmotor->in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(hmotor->in2_port, hmotor->in2_pin, GPIO_PIN_RESET);
    }
}

static uint8_t dc_motor_abs_speed(int8_t speed)
{
    return (speed < 0) ? (uint8_t)(-speed) : (uint8_t)speed;
}

static uint8_t dc_motor_is_valid_handle(const DC_Motor_HandleTypeDef *hmotor)
{
    return (hmotor != NULL &&
            hmotor->htim != NULL &&
            hmotor->in1_port != NULL &&
            hmotor->in2_port != NULL &&
            hmotor->in1_pin != 0U &&
            hmotor->in2_pin != 0U &&
            dc_motor_is_valid_channel(hmotor->channel));
}

DC_Motor_StatusTypeDef DC_Motor_Init(DC_Motor_HandleTypeDef *hmotor,
                                     TIM_HandleTypeDef *htim,
                                     uint32_t channel,
                                     GPIO_TypeDef *in1_port,
                                     uint16_t in1_pin,
                                     GPIO_TypeDef *in2_port,
                                     uint16_t in2_pin)
{
    if (hmotor == NULL || htim == NULL || in1_port == NULL || in2_port == NULL ||
        in1_pin == 0U || in2_pin == 0U || !dc_motor_is_valid_channel(channel)) {
        return DC_MOTOR_ERROR_PARAM;
    }

    hmotor->htim = htim;
    hmotor->channel = channel;
    hmotor->in1_port = in1_port;
    hmotor->in1_pin = in1_pin;
    hmotor->in2_port = in2_port;
    hmotor->in2_pin = in2_pin;
    hmotor->speed = 0;
    hmotor->stop_mode = DC_MOTOR_STOP_COAST;

    dc_motor_gpio_init(hmotor);
    dc_motor_apply_direction(hmotor, 0);
    dc_motor_set_pwm_percent(hmotor, 0U);

    return DC_MOTOR_OK;
}

DC_Motor_StatusTypeDef DC_Motor_Start(DC_Motor_HandleTypeDef *hmotor)
{
    if (!dc_motor_is_valid_handle(hmotor)) {
        return DC_MOTOR_ERROR_PARAM;
    }

    if (HAL_TIM_PWM_Start(hmotor->htim, hmotor->channel) != HAL_OK) {
        return DC_MOTOR_ERROR_HAL;
    }

    return DC_MOTOR_OK;
}

DC_Motor_StatusTypeDef DC_Motor_Stop(DC_Motor_HandleTypeDef *hmotor)
{
    if (!dc_motor_is_valid_handle(hmotor)) {
        return DC_MOTOR_ERROR_PARAM;
    }

    hmotor->speed = 0;
    dc_motor_set_pwm_percent(hmotor, 0U);
    dc_motor_apply_direction(hmotor, 0);

    if (HAL_TIM_PWM_Stop(hmotor->htim, hmotor->channel) != HAL_OK) {
        return DC_MOTOR_ERROR_HAL;
    }

    return DC_MOTOR_OK;
}

DC_Motor_StatusTypeDef DC_Motor_SetSpeed(DC_Motor_HandleTypeDef *hmotor, int8_t speed)
{
    if (!dc_motor_is_valid_handle(hmotor)) {
        return DC_MOTOR_ERROR_PARAM;
    }

    if (speed > 100) {
        speed = 100;
    } else if (speed < -100) {
        speed = -100;
    }

    hmotor->speed = speed;
    dc_motor_apply_direction(hmotor, speed);
    dc_motor_set_pwm_percent(hmotor, dc_motor_abs_speed(speed));

    return DC_MOTOR_OK;
}

void DC_Motor_SetStopMode(DC_Motor_HandleTypeDef *hmotor, DC_Motor_StopModeTypeDef stop_mode)
{
    if (hmotor == NULL) {
        return;
    }

    hmotor->stop_mode = stop_mode;

    if (hmotor->speed == 0) {
        dc_motor_apply_direction(hmotor, 0);
    }
}

int8_t DC_Motor_GetSpeed(const DC_Motor_HandleTypeDef *hmotor)
{
    if (hmotor == NULL) {
        return 0;
    }

    return hmotor->speed;
}
