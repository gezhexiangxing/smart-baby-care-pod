#include "voice_interaction.h"
#include <string.h>

#ifndef VOICE_DEFAULT_TIMEOUT_MS
#define VOICE_DEFAULT_TIMEOUT_MS 100U
#endif

static uint8_t voice_i2c_hal_addr(void)
{
    return (uint8_t)(VOICE_I2C_ADDR_7BIT << 1U);
}

static uint8_t voice_is_valid_uart_frame(const uint8_t *frame)
{
    return (frame != NULL &&
            frame[0] == VOICE_UART_HEAD_0 &&
            frame[1] == VOICE_UART_HEAD_1 &&
            frame[4] == VOICE_UART_TAIL);
}

static void voice_store_frame(Voice_HandleTypeDef *hvoice, const uint8_t *raw)
{
    memcpy(hvoice->latest_frame.raw, raw, VOICE_UART_FRAME_LEN);
    hvoice->latest_frame.type = raw[2];
    hvoice->latest_frame.id = raw[3];
    hvoice->has_frame = 1U;

    if (hvoice->callback != NULL) {
        hvoice->callback(&hvoice->latest_frame, hvoice->callback_context);
    }
}

void Voice_Init(Voice_HandleTypeDef *hvoice,
                UART_HandleTypeDef *huart,
                I2C_HandleTypeDef *hi2c)
{
    if (hvoice == NULL) {
        return;
    }

    memset(hvoice, 0, sizeof(*hvoice));
    hvoice->huart = huart;
    hvoice->hi2c = hi2c;
    hvoice->timeout_ms = VOICE_DEFAULT_TIMEOUT_MS;
}

void Voice_SetCallback(Voice_HandleTypeDef *hvoice,
                       Voice_FrameCallback callback,
                       void *user_context)
{
    if (hvoice == NULL) {
        return;
    }

    hvoice->callback = callback;
    hvoice->callback_context = user_context;
}

Voice_StatusTypeDef Voice_UART_StartReceiveIT(Voice_HandleTypeDef *hvoice)
{
    if (hvoice == NULL || hvoice->huart == NULL) {
        return VOICE_ERROR_PARAM;
    }

    if (HAL_UART_Receive_IT(hvoice->huart, &hvoice->rx_byte, 1U) != HAL_OK) {
        return VOICE_ERROR_HAL;
    }

    return VOICE_OK;
}

Voice_StatusTypeDef Voice_UART_RxCpltCallback(Voice_HandleTypeDef *hvoice, UART_HandleTypeDef *huart)
{
    Voice_StatusTypeDef status;

    if (hvoice == NULL || hvoice->huart == NULL || huart == NULL) {
        return VOICE_ERROR_PARAM;
    }

    if (huart != hvoice->huart) {
        return VOICE_ERROR_PARAM;
    }

    status = Voice_UART_ParseByte(hvoice, hvoice->rx_byte, NULL);
    (void)Voice_UART_StartReceiveIT(hvoice);

    return status;
}

Voice_StatusTypeDef Voice_UART_ParseByte(Voice_HandleTypeDef *hvoice, uint8_t byte, Voice_FrameTypeDef *frame)
{
    if (hvoice == NULL) {
        return VOICE_ERROR_PARAM;
    }

    if (hvoice->rx_index == 0U && byte != VOICE_UART_HEAD_0) {
        return VOICE_ERROR_NO_FRAME;
    }

    if (hvoice->rx_index == 1U && byte != VOICE_UART_HEAD_1) {
        if (byte == VOICE_UART_HEAD_0) {
            hvoice->rx_buf[0] = VOICE_UART_HEAD_0;
            hvoice->rx_index = 1U;
        } else {
            hvoice->rx_index = 0U;
        }
        return VOICE_ERROR_NO_FRAME;
    }

    hvoice->rx_buf[hvoice->rx_index++] = byte;

    if (hvoice->rx_index < VOICE_UART_FRAME_LEN) {
        return VOICE_ERROR_NO_FRAME;
    }

    hvoice->rx_index = 0U;

    if (!voice_is_valid_uart_frame(hvoice->rx_buf)) {
        return VOICE_ERROR_FRAME;
    }

    voice_store_frame(hvoice, hvoice->rx_buf);

    if (frame != NULL) {
        *frame = hvoice->latest_frame;
    }

    return VOICE_OK;
}

Voice_StatusTypeDef Voice_UART_GetLatestFrame(Voice_HandleTypeDef *hvoice, Voice_FrameTypeDef *frame)
{
    if (hvoice == NULL || frame == NULL) {
        return VOICE_ERROR_PARAM;
    }

    if (!hvoice->has_frame) {
        return VOICE_ERROR_NO_FRAME;
    }

    *frame = hvoice->latest_frame;
    hvoice->has_frame = 0U;

    return VOICE_OK;
}

Voice_StatusTypeDef Voice_UART_SendFrame(Voice_HandleTypeDef *hvoice, uint8_t type, uint8_t id)
{
    uint8_t frame[VOICE_UART_FRAME_LEN] = {
        VOICE_UART_HEAD_0,
        VOICE_UART_HEAD_1,
        type,
        id,
        VOICE_UART_TAIL
    };

    if (hvoice == NULL || hvoice->huart == NULL) {
        return VOICE_ERROR_PARAM;
    }

    if (HAL_UART_Transmit(hvoice->huart, frame, VOICE_UART_FRAME_LEN, hvoice->timeout_ms) != HAL_OK) {
        return VOICE_ERROR_HAL;
    }

    return VOICE_OK;
}

Voice_StatusTypeDef Voice_UART_PlayBroadcast(Voice_HandleTypeDef *hvoice, uint8_t broadcast_id)
{
    return Voice_UART_SendFrame(hvoice, VOICE_UART_BROADCAST_TYPE, broadcast_id);
}

Voice_StatusTypeDef Voice_I2C_PlayBroadcast(Voice_HandleTypeDef *hvoice, uint8_t broadcast_id)
{
    if (hvoice == NULL || hvoice->hi2c == NULL) {
        return VOICE_ERROR_PARAM;
    }

    if (HAL_I2C_Mem_Write(hvoice->hi2c,
                          voice_i2c_hal_addr(),
                          VOICE_I2C_WRITE_REGISTER,
                          I2C_MEMADD_SIZE_8BIT,
                          &broadcast_id,
                          1U,
                          hvoice->timeout_ms) != HAL_OK) {
        return VOICE_ERROR_HAL;
    }

    return VOICE_OK;
}

Voice_StatusTypeDef Voice_I2C_ReadCommand(Voice_HandleTypeDef *hvoice, uint8_t *command_id)
{
    if (hvoice == NULL || hvoice->hi2c == NULL || command_id == NULL) {
        return VOICE_ERROR_PARAM;
    }

    if (HAL_I2C_Mem_Read(hvoice->hi2c,
                         voice_i2c_hal_addr(),
                         VOICE_I2C_READ_REGISTER,
                         I2C_MEMADD_SIZE_8BIT,
                         command_id,
                         1U,
                         hvoice->timeout_ms) != HAL_OK) {
        return VOICE_ERROR_HAL;
    }

    return VOICE_OK;
}

Voice_EventTypeDef Voice_GetEventType(const Voice_FrameTypeDef *frame)
{
    if (frame == NULL) {
        return VOICE_EVENT_NONE;
    }

    if (frame->type == VOICE_UART_CMD_TYPE) {
        return VOICE_EVENT_COMMAND;
    }

    if (frame->type == VOICE_UART_BROADCAST_TYPE) {
        return VOICE_EVENT_BROADCAST;
    }

    if (frame->type >= VOICE_SYS_WELCOME && frame->type <= VOICE_SYS_REPORT_OFF) {
        return VOICE_EVENT_SYSTEM;
    }

    return VOICE_EVENT_NONE;
}
