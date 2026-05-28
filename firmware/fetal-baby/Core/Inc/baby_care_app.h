#ifndef BABY_CARE_APP_H
#define BABY_CARE_APP_H

#include "baby_care_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BABY_CARE_MODE_IDLE = 0,
    BABY_CARE_MODE_SLEEP,
    BABY_CARE_MODE_SOOTHE,
    BABY_CARE_MODE_ALERT
} BabyCare_ModeTypeDef;

typedef struct
{
    float temperature_c;
    float humidity_percent;
    uint8_t sensor_valid;
    uint8_t latest_voice_command;
    BabyCare_ModeTypeDef mode;
} BabyCare_StatusTypeDef;

void BabyCare_Init(void);
void BabyCare_Task(void);
void BabyCare_OnUartRxComplete(UART_HandleTypeDef *huart);
const BabyCare_StatusTypeDef *BabyCare_GetStatus(void);

#ifdef __cplusplus
}
#endif

#endif
