# STM32H743 baby care integration notes

This CubeIDE project has been integrated with the STM32H7 HAL demo drivers in
`Core/Inc` and `Core/Src`.

## Demo pin map

```text
DHT11 data: PB0
DC motor PWM: TIM2_CH3 / PB10
DC motor IN1/IN2: PA4 / PA5
Servo gimbal pan: TIM3_CH1 / PA6
Servo gimbal tilt: TIM3_CH2 / PA7
Voice module UART: USART2 TX/RX = PD5 / PD6, 9600 baud
Debug UART: USART1 TX/RX = PA9 / PA10, 115200 baud
```

## Runtime behavior

- `BabyCare_Init()` is called after all `MX_*_Init()` functions.
- `BabyCare_Task()` is called in the main loop.
- DHT11 is sampled every 2 seconds.
- USART1 prints status lines.
- USART2 receives voice frames in interrupt mode.
- Voice commands switch the demo mode between idle, sleep, soothe, and alert.

## CubeMX note

The C source is configured manually so the project can be compiled directly.
If the `.ioc` is opened and code is regenerated, sync these peripherals in
CubeMX before generation:

- Enable TIM2 channel 3 as PWM on PB10.
- Enable TIM3 channels 1 and 2 as PWM on PA6 and PA7.
- Enable USART1 asynchronous mode on PA9 and PA10 at 115200.
- Enable USART2 asynchronous mode on PD5 and PD6 at 9600, with USART2 global
  interrupt enabled.
- Keep PB0 as GPIO input with pull-up for DHT11.

Most integration code is placed inside CubeMX `USER CODE` sections where
possible, but manually added peripheral initialization should still be checked
after regeneration.
