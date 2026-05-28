#include "dht11.h"

#ifndef DHT11_DEFAULT_TIMEOUT_US
#define DHT11_DEFAULT_TIMEOUT_US 1000U
#endif

#define DHT11_START_LOW_MS 20U
#define DHT11_RELEASE_US 30U
#define DHT11_BIT_SAMPLE_US 40U

static void dht11_enable_gpio_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (port == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (port == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    } else if (port == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
#ifdef GPIOI
    else if (port == GPIOI) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
#endif
#ifdef GPIOJ
    else if (port == GPIOJ) {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
    }
#endif
#ifdef GPIOK
    else if (port == GPIOK) {
        __HAL_RCC_GPIOK_CLK_ENABLE();
    }
#endif
}

static void dht11_dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void dht11_delay_us(uint32_t us)
{
    const uint32_t start = DWT->CYCCNT;
    const uint32_t ticks = (SystemCoreClock / 1000000U) * us;

    while ((DWT->CYCCNT - start) < ticks) {
    }
}

static void dht11_gpio_input(DHT11_HandleTypeDef *hdht)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = hdht->pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hdht->port, &gpio);
}

static void dht11_gpio_output(DHT11_HandleTypeDef *hdht)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = hdht->pin;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hdht->port, &gpio);
}

static void dht11_write_pin(DHT11_HandleTypeDef *hdht, GPIO_PinState state)
{
    HAL_GPIO_WritePin(hdht->port, hdht->pin, state);
}

static GPIO_PinState dht11_read_pin(DHT11_HandleTypeDef *hdht)
{
    return HAL_GPIO_ReadPin(hdht->port, hdht->pin);
}

static DHT11_StatusTypeDef dht11_wait_for_level(DHT11_HandleTypeDef *hdht,
                                                GPIO_PinState level,
                                                uint32_t timeout_us)
{
    const uint32_t start = DWT->CYCCNT;
    const uint32_t timeout_ticks = (SystemCoreClock / 1000000U) * timeout_us;

    while (dht11_read_pin(hdht) != level) {
        if ((DWT->CYCCNT - start) > timeout_ticks) {
            return DHT11_ERROR_TIMEOUT;
        }
    }

    return DHT11_OK;
}

static DHT11_StatusTypeDef dht11_start(DHT11_HandleTypeDef *hdht)
{
    dht11_gpio_output(hdht);
    dht11_write_pin(hdht, GPIO_PIN_SET);
    HAL_Delay(1U);

    dht11_write_pin(hdht, GPIO_PIN_RESET);
    HAL_Delay(DHT11_START_LOW_MS);

    dht11_write_pin(hdht, GPIO_PIN_SET);
    dht11_delay_us(DHT11_RELEASE_US);
    dht11_gpio_input(hdht);

    if (dht11_wait_for_level(hdht, GPIO_PIN_RESET, hdht->timeout_us) != DHT11_OK) {
        return DHT11_ERROR_TIMEOUT;
    }
    if (dht11_wait_for_level(hdht, GPIO_PIN_SET, hdht->timeout_us) != DHT11_OK) {
        return DHT11_ERROR_TIMEOUT;
    }
    if (dht11_wait_for_level(hdht, GPIO_PIN_RESET, hdht->timeout_us) != DHT11_OK) {
        return DHT11_ERROR_TIMEOUT;
    }

    return DHT11_OK;
}

static DHT11_StatusTypeDef dht11_read_byte(DHT11_HandleTypeDef *hdht, uint8_t *value)
{
    uint8_t data = 0U;

    if (value == NULL) {
        return DHT11_ERROR_PARAM;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        if (dht11_wait_for_level(hdht, GPIO_PIN_SET, hdht->timeout_us) != DHT11_OK) {
            return DHT11_ERROR_TIMEOUT;
        }

        dht11_delay_us(DHT11_BIT_SAMPLE_US);
        data <<= 1U;

        if (dht11_read_pin(hdht) == GPIO_PIN_SET) {
            data |= 1U;
            if (dht11_wait_for_level(hdht, GPIO_PIN_RESET, hdht->timeout_us) != DHT11_OK) {
                return DHT11_ERROR_TIMEOUT;
            }
        }
    }

    *value = data;
    return DHT11_OK;
}

void DHT11_Init(DHT11_HandleTypeDef *hdht, GPIO_TypeDef *port, uint16_t pin)
{
    if (hdht == NULL || port == NULL || pin == 0U) {
        return;
    }

    hdht->port = port;
    hdht->pin = pin;
    hdht->timeout_us = DHT11_DEFAULT_TIMEOUT_US;

    dht11_dwt_init();
    dht11_enable_gpio_clock(port);
    dht11_gpio_input(hdht);
}

DHT11_StatusTypeDef DHT11_Read(DHT11_HandleTypeDef *hdht, DHT11_DataTypeDef *data)
{
    uint8_t raw[5] = {0};
    DHT11_StatusTypeDef status;

    if (hdht == NULL || data == NULL || hdht->port == NULL || hdht->pin == 0U) {
        return DHT11_ERROR_PARAM;
    }

    status = dht11_start(hdht);
    if (status != DHT11_OK) {
        return status;
    }

    for (uint8_t i = 0U; i < 5U; i++) {
        status = dht11_read_byte(hdht, &raw[i]);
        if (status != DHT11_OK) {
            return status;
        }
    }

    if (((uint8_t)(raw[0] + raw[1] + raw[2] + raw[3])) != raw[4]) {
        return DHT11_ERROR_CHECKSUM;
    }

    data->humidity_int = raw[0];
    data->humidity_dec = raw[1];
    data->temperature_int = raw[2];
    data->temperature_dec = raw[3];

    return DHT11_OK;
}

float DHT11_GetTemperature(const DHT11_DataTypeDef *data)
{
    if (data == NULL) {
        return 0.0f;
    }

    return (float)data->temperature_int + ((float)data->temperature_dec / 10.0f);
}

float DHT11_GetHumidity(const DHT11_DataTypeDef *data)
{
    if (data == NULL) {
        return 0.0f;
    }

    return (float)data->humidity_int + ((float)data->humidity_dec / 10.0f);
}
