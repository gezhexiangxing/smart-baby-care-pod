# smart-baby-care-pod
STM32H7 + AI multimodal smart baby care cabin project with sensor, motor, edge AI, and app modules.

## Current modules

- `modules/dht11_h7`: DHT11 temperature and humidity driver for STM32H7 HAL.
- `modules/dc_motor_pwm_h7`: PWM DC motor driver with two direction pins.
- `modules/servo_gimbal_h7`: two-axis servo gimbal driver.
- `modules/voice_interaction_h7`: UART/I2C voice recognition and playback protocol driver.
- `integration/stm32h7_demo`: first integrated demo app layer that wires the modules into a simple care-pod loop.
- `firmware/fetal-baby`: STM32H743 CubeIDE firmware with the demo loop integrated into `Core`.

## Next integration step

Open `firmware/fetal-baby` in STM32CubeIDE, build the Debug configuration, then
flash the board and watch USART1 at 115200 baud for status output.
