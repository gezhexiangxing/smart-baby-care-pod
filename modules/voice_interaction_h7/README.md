# Voice interaction module driver for STM32H7 HAL

This folder contains the STM32H7 HAL port for the voice recognition and
interaction module.

The original package includes STM32F1 standard peripheral library examples,
factory firmware, a 3D model, ROS/ESP32 examples, and an Excel protocol table.
The useful MCU protocol parts are:

- UART frame: `AA 55 <type> <id> FB`
- Passive broadcast frame used by the F1 demo: `AA 55 FF <broadcast_id> FB`
- Command recognition frame from the protocol table: `AA 55 00 <command_id> FB`
- I2C address: `0x2B`
- I2C broadcast register: `0x03`
- I2C command read register: `0x64`

## Files

- `voice_interaction.h`
- `voice_interaction.c`

Add both files to the STM32H743 project and include `voice_interaction.h`.

## UART usage

Configure one UART in CubeMX, for example `USART1`, with the baud rate required
by the module. The original STM32F1 example uses `115200`.

```c
#include "voice_interaction.h"

extern UART_HandleTypeDef huart1;

static Voice_HandleTypeDef hvoice;

void App_Init(void)
{
    Voice_Init(&hvoice, &huart1, NULL);
    Voice_UART_StartReceiveIT(&hvoice);

    Voice_UART_PlayBroadcast(&hvoice, VOICE_LEGACY_INIT);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    (void)Voice_UART_RxCpltCallback(&hvoice, huart);
}

void App_Loop(void)
{
    Voice_FrameTypeDef frame;

    if (Voice_UART_GetLatestFrame(&hvoice, &frame) == VOICE_OK) {
        if (Voice_GetEventType(&frame) == VOICE_EVENT_COMMAND) {
            switch (frame.id) {
            case VOICE_CMD_FORWARD:
                break;
            case VOICE_CMD_STOP_1:
            case VOICE_CMD_STOP_2:
                break;
            default:
                break;
            }
        }
    }
}
```

If your project already receives UART data by DMA or a custom interrupt, call
`Voice_UART_ParseByte(&hvoice, byte, &frame)` for each received byte instead of
using `Voice_UART_StartReceiveIT`.

## I2C usage

Configure one I2C peripheral in CubeMX and pass its handle to `Voice_Init`.

```c
extern I2C_HandleTypeDef hi2c1;

static Voice_HandleTypeDef hvoice;

Voice_Init(&hvoice, NULL, &hi2c1);
Voice_I2C_PlayBroadcast(&hvoice, VOICE_LEGACY_INIT);

uint8_t command_id;
if (Voice_I2C_ReadCommand(&hvoice, &command_id) == VOICE_OK) {
    /* command_id is the latest recognized command. */
}
```

HAL I2C APIs use the 8-bit shifted address internally, so the driver defines the
module address as the 7-bit value `0x2B` and shifts it before calling HAL.

## Protocol IDs

The header contains two groups of IDs:

- Protocol-table IDs from `命令词播报词协议列表V3_中文.xlsx`, such as
  `VOICE_CMD_FORWARD`, `VOICE_CMD_STOP_1`, and
  `VOICE_BROADCAST_INIT_DONE`.
- Legacy IDs from the STM32F1 demo source, such as `VOICE_LEGACY_INIT = 0x67`.

The legacy IDs are kept because the factory firmware in the package may match
the demo source rather than the generic Excel table.

## Hardware notes

- Do not connect 5 V UART TX directly to STM32H743 RX unless the selected pin is
  5 V tolerant and the board wiring is verified. A level shifter is safer.
- For UART, connect module TX to STM32 RX and module RX to STM32 TX.
- For I2C, use pull-up resistors on SDA/SCL. Many modules already include them,
  but the final bus voltage must match the STM32H743 IO bank.
