#ifndef DHT11_H7_H
#define DHT11_H7_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DHT11_OK = 0,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_CHECKSUM,
    DHT11_ERROR_PARAM
} DHT11_StatusTypeDef;

typedef struct
{
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temperature_int;
    uint8_t temperature_dec;
} DHT11_DataTypeDef;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t timeout_us;
} DHT11_HandleTypeDef;

void DHT11_Init(DHT11_HandleTypeDef *hdht, GPIO_TypeDef *port, uint16_t pin);
DHT11_StatusTypeDef DHT11_Read(DHT11_HandleTypeDef *hdht, DHT11_DataTypeDef *data);
float DHT11_GetTemperature(const DHT11_DataTypeDef *data);
float DHT11_GetHumidity(const DHT11_DataTypeDef *data);

#ifdef __cplusplus
}
#endif

#endif
