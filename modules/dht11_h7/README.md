# DHT11 driver for STM32H7 HAL

This folder contains the STM32H7 HAL port of the original STM32F1 DHT11 module.
It removes the F1-only `sys.h` and `delay.h` dependency and uses the Cortex-M7
DWT cycle counter for microsecond timing.

## Files

- `dht11.h`
- `dht11.c`

Add both files to the STM32H7 project, then include `dht11.h`.

## Example

```c
#include "dht11.h"

static DHT11_HandleTypeDef hdht11;
static DHT11_DataTypeDef dht11_data;

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    DHT11_Init(&hdht11, GPIOA, GPIO_PIN_5);

    while (1) {
        if (DHT11_Read(&hdht11, &dht11_data) == DHT11_OK) {
            float temperature = DHT11_GetTemperature(&dht11_data);
            float humidity = DHT11_GetHumidity(&dht11_data);

            (void)temperature;
            (void)humidity;
        }

        HAL_Delay(2000);
    }
}
```

## Hardware notes

- The DHT11 data line needs a pull-up resistor. Many DHT11 modules already have
  one onboard. If using the bare sensor, add an external 4.7 kOhm to 10 kOhm
  pull-up resistor.
- Keep reads at least 1 second apart. A 2 second interval is recommended.
- Change `GPIOA, GPIO_PIN_5` in `DHT11_Init` to the actual STM32H7 pin used in
  your project.

## Return values

- `DHT11_OK`: read success.
- `DHT11_ERROR_TIMEOUT`: sensor did not respond or wiring/timing is wrong.
- `DHT11_ERROR_CHECKSUM`: data checksum failed.
- `DHT11_ERROR_PARAM`: invalid handle or output pointer.
