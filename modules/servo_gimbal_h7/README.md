# Servo gimbal driver for STM32H743 HAL

This folder contains the STM32H7 HAL port of the original STM32 standard-library
servo gimbal module.

The original project drives two servos by toggling GPIO pins from a 10 us TIM3
interrupt. The H7 version uses hardware PWM through HAL timers, which is more
stable and leaves the CPU free for AI, sensors, and communication work.

## Files

- `servo_gimbal.h`
- `servo_gimbal.c`

Add both files to the STM32H743 project and include `servo_gimbal.h`.

## CubeMX / HAL setup

Configure one or two timer PWM channels at 50 Hz:

- PWM period: 20 ms
- Servo pulse range: 500 us to 2500 us
- Angle range: 0 to 180 degrees

Recommended timer setup:

- Timer counter clock: 1 MHz, so 1 timer count equals 1 us
- Prescaler: chosen from the timer input clock to reach 1 MHz
- Period / ARR: `20000 - 1`
- PWM mode: PWM Generation CHx

The driver also works when ARR is not `19999`, as long as the PWM period is
20 ms. It maps microseconds to compare values using `ARR + 1`.

## Example

```c
#include "servo_gimbal.h"

extern TIM_HandleTypeDef htim3;

static ServoGimbal_HandleTypeDef hgimbal;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM3_Init();

    ServoGimbal_Init(&hgimbal,
                     &htim3, TIM_CHANNEL_1,   // pan servo
                     &htim3, TIM_CHANNEL_2);  // tilt servo

    ServoGimbal_Start(&hgimbal);
    ServoGimbal_SetAngles(&hgimbal, 90, 90);

    while (1) {
        ServoGimbal_SetAngles(&hgimbal, 45, 120);
        HAL_Delay(1000);
        ServoGimbal_SetAngles(&hgimbal, 135, 60);
        HAL_Delay(1000);
    }
}
```

## ESP32 remote frame compatibility

The original project parses four-byte remote frames:

- `A090`: servo A to 90 degrees
- `B120`: servo B to 120 degrees

This driver keeps that parser:

```c
ServoGimbal_ParseRemoteFrame(&hgimbal, (uint8_t *)"A090", 4);
ServoGimbal_ParseRemoteFrame(&hgimbal, (uint8_t *)"B120", 4);
```

The original code reverses the `B` servo direction with `180 - angle`; the H7
driver keeps this behavior by default. Use this if your mounting direction is
different:

```c
ServoGimbal_SetTiltReverse(&hgimbal, 0);
```

## API

- `Servo_Init`, `Servo_Start`, `Servo_SetAngle`: single-servo API.
- `ServoGimbal_Init`, `ServoGimbal_Start`, `ServoGimbal_SetAngles`: two-axis
  gimbal API.
- `Servo_SetPulseRange`: adjust pulse width for different servos, for example
  500 to 2400 us.
- `Servo_SetAngleRange`: constrain travel to protect the mechanism.
- `Servo_SetPeriodUs`: set the real PWM period when it is not exactly 20 ms.

## Hardware notes

- Do not power servos from the STM32H743 board 3.3 V pin. Use a separate servo
  power supply with enough current.
- Connect servo power ground and STM32 ground together.
- Start with a limited angle range if the gimbal mechanism can hit a hard stop.
