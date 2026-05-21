#ifndef VOICE_INTERACTION_H7_H
#define VOICE_INTERACTION_H7_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VOICE_UART_FRAME_LEN       5U
#define VOICE_UART_HEAD_0          0xAAU
#define VOICE_UART_HEAD_1          0x55U
#define VOICE_UART_TAIL            0xFBU

#define VOICE_UART_CMD_TYPE        0x00U
#define VOICE_UART_BROADCAST_TYPE  0xFFU

#define VOICE_I2C_ADDR_7BIT        0x2BU
#define VOICE_I2C_WRITE_REGISTER   0x03U
#define VOICE_I2C_READ_REGISTER    0x64U

typedef enum
{
    VOICE_OK = 0,
    VOICE_ERROR_PARAM,
    VOICE_ERROR_HAL,
    VOICE_ERROR_FRAME,
    VOICE_ERROR_NO_FRAME
} Voice_StatusTypeDef;

typedef enum
{
    VOICE_EVENT_NONE = 0,
    VOICE_EVENT_COMMAND,
    VOICE_EVENT_BROADCAST,
    VOICE_EVENT_SYSTEM
} Voice_EventTypeDef;

typedef struct
{
    uint8_t type;
    uint8_t id;
    uint8_t raw[VOICE_UART_FRAME_LEN];
} Voice_FrameTypeDef;

typedef void (*Voice_FrameCallback)(const Voice_FrameTypeDef *frame, void *user_context);

typedef struct
{
    UART_HandleTypeDef *huart;
    I2C_HandleTypeDef *hi2c;
    uint32_t timeout_ms;

    uint8_t rx_byte;
    uint8_t rx_buf[VOICE_UART_FRAME_LEN];
    uint8_t rx_index;

    Voice_FrameTypeDef latest_frame;
    uint8_t has_frame;

    Voice_FrameCallback callback;
    void *callback_context;
} Voice_HandleTypeDef;

typedef enum
{
    VOICE_SYS_WELCOME = 0x01,
    VOICE_SYS_SLEEP = 0x02,
    VOICE_SYS_WAKEUP = 0x03,
    VOICE_SYS_VOLUME_UP = 0x04,
    VOICE_SYS_VOLUME_DOWN = 0x05,
    VOICE_SYS_VOLUME_MAX = 0x06,
    VOICE_SYS_VOLUME_MEDIUM = 0x07,
    VOICE_SYS_VOLUME_MIN = 0x08,
    VOICE_SYS_REPORT_ON = 0x09,
    VOICE_SYS_REPORT_OFF = 0x0A
} Voice_SystemIdTypeDef;

typedef enum
{
    VOICE_CMD_STOP_1 = 0x01,
    VOICE_CMD_STOP_2 = 0x02,
    VOICE_CMD_SLEEP = 0x03,
    VOICE_CMD_FORWARD = 0x04,
    VOICE_CMD_BACKWARD = 0x05,
    VOICE_CMD_TURN_LEFT = 0x06,
    VOICE_CMD_TURN_RIGHT = 0x07,
    VOICE_CMD_ROTATE_LEFT = 0x08,
    VOICE_CMD_ROTATE_RIGHT = 0x09,
    VOICE_CMD_LIGHT_OFF = 0x0A,
    VOICE_CMD_LIGHT_RED = 0x0B,
    VOICE_CMD_LIGHT_GREEN = 0x0C,
    VOICE_CMD_LIGHT_BLUE = 0x0D,
    VOICE_CMD_LIGHT_YELLOW = 0x0E,
    VOICE_CMD_SHOW_BATTERY = 0x12,
    VOICE_CMD_ALARM = 0x26
} Voice_CommandIdTypeDef;

typedef enum
{
    VOICE_BROADCAST_ACTION_INPUT_READY = 0x1D,
    VOICE_BROADCAST_PICKED_PREFIX = 0x22,
    VOICE_BROADCAST_THIS_IS_RED = 0x3D,
    VOICE_BROADCAST_THIS_IS_BLUE = 0x3E,
    VOICE_BROADCAST_THIS_IS_GREEN = 0x3F,
    VOICE_BROADCAST_THIS_IS_YELLOW = 0x40,
    VOICE_BROADCAST_PLACE_DONE = 0x41,
    VOICE_BROADCAST_RECOGNIZE_YELLOW = 0x42,
    VOICE_BROADCAST_RECOGNIZE_GREEN = 0x43,
    VOICE_BROADCAST_RECOGNIZE_BLUE = 0x44,
    VOICE_BROADCAST_RECOGNIZE_RED = 0x49,
    VOICE_BROADCAST_INIT_DONE = 0x58
} Voice_BroadcastIdTypeDef;

/* IDs used by the original STM32F1 demo source. Keep them for firmware compatibility. */
typedef enum
{
    VOICE_LEGACY_THIS_RED = 0x5F,
    VOICE_LEGACY_THIS_BLUE = 0x60,
    VOICE_LEGACY_THIS_GREEN = 0x61,
    VOICE_LEGACY_THIS_YELLOW = 0x62,
    VOICE_LEGACY_RECOGNIZE_YELLOW = 0x63,
    VOICE_LEGACY_RECOGNIZE_GREEN = 0x64,
    VOICE_LEGACY_RECOGNIZE_BLUE = 0x65,
    VOICE_LEGACY_RECOGNIZE_RED = 0x66,
    VOICE_LEGACY_INIT = 0x67
} Voice_LegacyBroadcastIdTypeDef;

void Voice_Init(Voice_HandleTypeDef *hvoice,
                UART_HandleTypeDef *huart,
                I2C_HandleTypeDef *hi2c);
void Voice_SetCallback(Voice_HandleTypeDef *hvoice,
                       Voice_FrameCallback callback,
                       void *user_context);

Voice_StatusTypeDef Voice_UART_StartReceiveIT(Voice_HandleTypeDef *hvoice);
Voice_StatusTypeDef Voice_UART_RxCpltCallback(Voice_HandleTypeDef *hvoice, UART_HandleTypeDef *huart);
Voice_StatusTypeDef Voice_UART_ParseByte(Voice_HandleTypeDef *hvoice, uint8_t byte, Voice_FrameTypeDef *frame);
Voice_StatusTypeDef Voice_UART_GetLatestFrame(Voice_HandleTypeDef *hvoice, Voice_FrameTypeDef *frame);
Voice_StatusTypeDef Voice_UART_SendFrame(Voice_HandleTypeDef *hvoice, uint8_t type, uint8_t id);
Voice_StatusTypeDef Voice_UART_PlayBroadcast(Voice_HandleTypeDef *hvoice, uint8_t broadcast_id);

Voice_StatusTypeDef Voice_I2C_PlayBroadcast(Voice_HandleTypeDef *hvoice, uint8_t broadcast_id);
Voice_StatusTypeDef Voice_I2C_ReadCommand(Voice_HandleTypeDef *hvoice, uint8_t *command_id);

Voice_EventTypeDef Voice_GetEventType(const Voice_FrameTypeDef *frame);

#ifdef __cplusplus
}
#endif

#endif
