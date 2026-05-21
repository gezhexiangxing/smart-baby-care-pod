# PWM DC motor driver for STM32H7 HAL

This folder contains a STM32H7 HAL version of the original STM32F1 PWM DC motor
module. The original module used:

- `TIM2_CH3` as PWM output.
- `PA4` and `PA5` as H-bridge direction pins.
- speed range `-100` to `100`.

The H7 version keeps the same behavior, but uses HAL and a generic timer handle,
so it can be used with any PWM-capable timer/channel configured in CubeMX.

## Files

- `dc_motor_pwm.h`
- `dc_motor_pwm.c`

Add both files to the STM32H7 project, then include `dc_motor_pwm.h`.

## CubeMX / HAL setup

Configure one timer channel as PWM Generation. For example:

- Timer: `TIM2`
- Channel: `TIM_CHANNEL_3`
- PWM pin: the matching alternate-function GPIO pin for your STM32H7 board
- Counter mode: up
- Prescaler and period: choose according to your motor driver frequency

For a simple 0 to 100 percent duty API, a timer period of `999` or `9999` is
easy to reason about. The driver reads the timer auto-reload value and converts
speed percent into CCR compare value automatically.

The two direction pins are normal GPIO output pins connected to the motor driver
inputs, for example `IN1` and `IN2` on an L298N/TB6612-style board.

## Example

```c
#include "dc_motor_pwm.h"

extern TIM_HandleTypeDef htim2;

static DC_Motor_HandleTypeDef hmotor;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    DC_Motor_Init(&hmotor,
                  &htim2,
                  TIM_CHANNEL_3,
                  GPIOA,
                  GPIO_PIN_4,
                  GPIOA,
                  GPIO_PIN_5);

    DC_Motor_Start(&hmotor);

    while (1) {
        DC_Motor_SetSpeed(&hmotor, 60);    // forward, 60%
        HAL_Delay(2000);

        DC_Motor_SetSpeed(&hmotor, -60);   // reverse, 60%
        HAL_Delay(2000);

        DC_Motor_SetSpeed(&hmotor, 0);     // stop
        HAL_Delay(1000);
    }
}
```

## API

- `DC_Motor_Init`: binds a motor object to one PWM timer channel and two
  direction GPIO pins.
- `DC_Motor_Start`: starts PWM output with `HAL_TIM_PWM_Start`.
- `DC_Motor_SetSpeed`: accepts `-100` to `100`. Positive is forward, negative is
  reverse, zero is stop.
- `DC_Motor_Stop`: sets speed to zero and stops PWM output.
- `DC_Motor_SetStopMode`: choose coast stop or brake stop.

## Notes

- If your motor driver only supports one direction pin plus one PWM pin, this
  driver can still be adapted, but the current version targets the two-direction
  input wiring used by the original F1 code.
- Do not power a DC motor directly from an STM32H7 GPIO. Use a proper motor
  driver module and common ground between driver power and the STM32 board.
