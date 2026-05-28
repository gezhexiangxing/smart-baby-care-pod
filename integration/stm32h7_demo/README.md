# STM32H7 demo integration layer

This folder turns the standalone STM32H7 modules into a first demo loop for the
smart baby care pod.

It is intentionally separate from `Core/` because the real STM32CubeMX project
has not been added yet. After CubeMX generates the STM32H7 project, copy these
files into the project and add the include paths.

## What this demo does

- Initializes DHT11, DC motor, two-axis servo gimbal, and UART voice module.
- Reads temperature and humidity every 2 seconds.
- Receives voice command frames through UART interrupt mode.
- Maps voice commands to simple modes:
  - stop -> idle
  - sleep -> sleep mode
  - forward/backward/rotate -> soothe mode
  - alarm -> alert mode
- Runs a simple soothing movement with the gimbal and motor.
- Prints status through debug UART.

## Files

- `Inc/baby_care_config.h`: hardware mapping and feature switches.
- `Inc/baby_care_app.h`: public app API.
- `Src/baby_care_app.c`: integrated demo state machine.
- `Src/main_integration_example.c`: example snippets for CubeMX `main.c`.

## Required include paths

Add these folders to the STM32H7 project include path:

```text
integration/stm32h7_demo/Inc
modules/dht11_h7
modules/dc_motor_pwm_h7
modules/servo_gimbal_h7
modules/voice_interaction_h7
```

Add these source files to the build:

```text
integration/stm32h7_demo/Src/baby_care_app.c
modules/dht11_h7/dht11.c
modules/dc_motor_pwm_h7/dc_motor_pwm.c
modules/servo_gimbal_h7/servo_gimbal.c
modules/voice_interaction_h7/voice_interaction.c
```

Do not compile `main_integration_example.c` directly if CubeMX already generated
`Core/Src/main.c`. Use it as a copy-and-paste reference.

## CubeMX peripherals expected by the default config

The defaults in `baby_care_config.h` are placeholders:

```text
DHT11: PB0
DC motor PWM: TIM2_CH3
DC motor IN1/IN2: PA4 / PA5
Servo gimbal pan/tilt: TIM3_CH1 / TIM3_CH2
Voice UART: USART2
Debug UART: USART1
```

Before flashing hardware, replace them with the real pin table for the baby care
pod. Avoid pin conflicts between motor PWM, voice UART, DHT11 and servo PWM.

## CubeMX timer notes

- DC motor PWM can use a normal motor-friendly PWM frequency.
- Servo PWM should use a 20 ms period. A timer period of 19999 with a 1 MHz timer
  tick makes pulse widths easy to map in microseconds.
- Start PWM through the app layer. The module calls `HAL_TIM_PWM_Start`.

## CubeMX UART notes

- Configure the voice UART with the baud rate required by the voice module.
- Configure the debug UART if `BABY_CARE_USE_DEBUG_UART` is enabled.
- Add this callback to CubeMX `main.c` or `stm32h7xx_it.c` callback area:

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BabyCare_OnUartRxComplete(huart);
}
```

## Main loop hook

In CubeMX `main.c`, after all `MX_*_Init()` calls:

```c
BabyCare_Init();
```

Inside `while (1)`:

```c
BabyCare_Task();
```

## Next files still needed

To turn this into a fully buildable project, add the real STM32H7 CubeMX project:

- `.ioc`
- `Core/Inc`
- `Core/Src`
- `Drivers/`
- STM32CubeIDE or Keil project files
- final hardware pin table
