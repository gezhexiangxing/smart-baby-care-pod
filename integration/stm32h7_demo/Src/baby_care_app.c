#include "baby_care_app.h"

#include "dht11.h"
#include "dc_motor_pwm.h"
#include "servo_gimbal.h"
#include "voice_interaction.h"

#include <stdio.h>
#include <string.h>

#if BABY_CARE_USE_DHT11
static DHT11_HandleTypeDef s_dht11;
static DHT11_DataTypeDef s_dht11_data;
#endif

#if BABY_CARE_USE_DC_MOTOR
static DC_Motor_HandleTypeDef s_motor;
#endif

#if BABY_CARE_USE_SERVO_GIMBAL
static ServoGimbal_HandleTypeDef s_gimbal;
#endif

#if BABY_CARE_USE_VOICE_UART
static Voice_HandleTypeDef s_voice;
#endif

static BabyCare_StatusTypeDef s_status;
static uint32_t s_last_sensor_tick;
static uint32_t s_last_report_tick;
static uint32_t s_last_soothe_tick;
static uint8_t s_soothe_phase;

static void BabyCare_Log(const char *message)
{
#if BABY_CARE_USE_DEBUG_UART
    if (message == NULL) {
        return;
    }

    (void)HAL_UART_Transmit(&BABY_CARE_DEBUG_UART,
                            (uint8_t *)message,
                            (uint16_t)strlen(message),
                            100U);
#else
    (void)message;
#endif
}

static void BabyCare_SetMode(BabyCare_ModeTypeDef mode)
{
    s_status.mode = mode;

    switch (mode) {
    case BABY_CARE_MODE_SLEEP:
#if BABY_CARE_USE_DC_MOTOR
        (void)DC_Motor_SetSpeed(&s_motor, 0);
#endif
#if BABY_CARE_USE_SERVO_GIMBAL
        (void)ServoGimbal_SetAngles(&s_gimbal, 90U, 90U);
#endif
        BabyCare_Log("mode=sleep\r\n");
        break;

    case BABY_CARE_MODE_SOOTHE:
        s_soothe_phase = 0U;
        s_last_soothe_tick = HAL_GetTick();
        BabyCare_Log("mode=soothe\r\n");
        break;

    case BABY_CARE_MODE_ALERT:
#if BABY_CARE_USE_DC_MOTOR
        (void)DC_Motor_SetSpeed(&s_motor, 0);
#endif
        BabyCare_Log("mode=alert\r\n");
        break;

    case BABY_CARE_MODE_IDLE:
    default:
#if BABY_CARE_USE_DC_MOTOR
        (void)DC_Motor_SetSpeed(&s_motor, 0);
#endif
#if BABY_CARE_USE_SERVO_GIMBAL
        (void)ServoGimbal_SetAngles(&s_gimbal, 90U, 90U);
#endif
        BabyCare_Log("mode=idle\r\n");
        break;
    }
}

static void BabyCare_HandleVoiceCommand(uint8_t command_id)
{
    s_status.latest_voice_command = command_id;

    switch (command_id) {
    case VOICE_CMD_STOP_1:
    case VOICE_CMD_STOP_2:
        BabyCare_SetMode(BABY_CARE_MODE_IDLE);
        break;

    case VOICE_CMD_SLEEP:
        BabyCare_SetMode(BABY_CARE_MODE_SLEEP);
        break;

    case VOICE_CMD_FORWARD:
    case VOICE_CMD_BACKWARD:
    case VOICE_CMD_ROTATE_LEFT:
    case VOICE_CMD_ROTATE_RIGHT:
        BabyCare_SetMode(BABY_CARE_MODE_SOOTHE);
        break;

    case VOICE_CMD_ALARM:
        BabyCare_SetMode(BABY_CARE_MODE_ALERT);
        break;

    default:
        break;
    }
}

static void BabyCare_PollVoice(void)
{
#if BABY_CARE_USE_VOICE_UART
    Voice_FrameTypeDef frame;

    if (Voice_UART_GetLatestFrame(&s_voice, &frame) == VOICE_OK &&
        Voice_GetEventType(&frame) == VOICE_EVENT_COMMAND) {
        BabyCare_HandleVoiceCommand(frame.id);
    }
#endif
}

static void BabyCare_ReadSensors(void)
{
#if BABY_CARE_USE_DHT11
    if (DHT11_Read(&s_dht11, &s_dht11_data) == DHT11_OK) {
        s_status.temperature_c = DHT11_GetTemperature(&s_dht11_data);
        s_status.humidity_percent = DHT11_GetHumidity(&s_dht11_data);
        s_status.sensor_valid = 1U;
    } else {
        s_status.sensor_valid = 0U;
    }
#endif
}

static void BabyCare_RunSootheMotion(void)
{
    if (s_status.mode != BABY_CARE_MODE_SOOTHE) {
        return;
    }

    if ((HAL_GetTick() - s_last_soothe_tick) < BABY_CARE_SOOTHE_PERIOD_MS) {
        return;
    }

    s_last_soothe_tick = HAL_GetTick();
    s_soothe_phase ^= 1U;

#if BABY_CARE_USE_SERVO_GIMBAL
    if (s_soothe_phase != 0U) {
        (void)ServoGimbal_SetAngles(&s_gimbal, 70U, 95U);
    } else {
        (void)ServoGimbal_SetAngles(&s_gimbal, 110U, 85U);
    }
#endif

#if BABY_CARE_USE_DC_MOTOR
    (void)DC_Motor_SetSpeed(&s_motor, (s_soothe_phase != 0U) ? 35 : -35);
#endif
}

static void BabyCare_ReportStatus(void)
{
    char line[128];
    const char *mode_text = "idle";
    const int32_t temp_deci = (int32_t)(s_status.temperature_c * 10.0f);
    const int32_t hum_deci = (int32_t)(s_status.humidity_percent * 10.0f);

    switch (s_status.mode) {
    case BABY_CARE_MODE_SLEEP:
        mode_text = "sleep";
        break;
    case BABY_CARE_MODE_SOOTHE:
        mode_text = "soothe";
        break;
    case BABY_CARE_MODE_ALERT:
        mode_text = "alert";
        break;
    case BABY_CARE_MODE_IDLE:
    default:
        break;
    }

    (void)snprintf(line,
                   sizeof(line),
                   "status mode=%s temp=%ld.%ldC hum=%ld.%ld%% sensor=%u voice=0x%02X\r\n",
                   mode_text,
                   (long)(temp_deci / 10),
                   (long)(temp_deci % 10),
                   (long)(hum_deci / 10),
                   (long)(hum_deci % 10),
                   s_status.sensor_valid,
                   s_status.latest_voice_command);
    BabyCare_Log(line);
}

void BabyCare_Init(void)
{
    memset(&s_status, 0, sizeof(s_status));
    s_status.mode = BABY_CARE_MODE_IDLE;

#if BABY_CARE_USE_DHT11
    DHT11_Init(&s_dht11, BABY_CARE_DHT11_PORT, BABY_CARE_DHT11_PIN);
#endif

#if BABY_CARE_USE_DC_MOTOR
    (void)DC_Motor_Init(&s_motor,
                        &BABY_CARE_MOTOR_TIM,
                        BABY_CARE_MOTOR_CHANNEL,
                        BABY_CARE_MOTOR_IN1_PORT,
                        BABY_CARE_MOTOR_IN1_PIN,
                        BABY_CARE_MOTOR_IN2_PORT,
                        BABY_CARE_MOTOR_IN2_PIN);
    (void)DC_Motor_Start(&s_motor);
#endif

#if BABY_CARE_USE_SERVO_GIMBAL
    (void)ServoGimbal_Init(&s_gimbal,
                           &BABY_CARE_GIMBAL_PAN_TIM,
                           BABY_CARE_GIMBAL_PAN_CH,
                           &BABY_CARE_GIMBAL_TILT_TIM,
                           BABY_CARE_GIMBAL_TILT_CH);
    (void)ServoGimbal_Start(&s_gimbal);
    (void)ServoGimbal_SetAngles(&s_gimbal, 90U, 90U);
#endif

#if BABY_CARE_USE_VOICE_UART
    Voice_Init(&s_voice, &BABY_CARE_VOICE_UART, NULL);
    (void)Voice_UART_StartReceiveIT(&s_voice);
    (void)Voice_UART_PlayBroadcast(&s_voice, VOICE_LEGACY_INIT);
#endif

    BabyCare_Log("baby care app init done\r\n");
}

void BabyCare_Task(void)
{
    const uint32_t now = HAL_GetTick();

    BabyCare_PollVoice();
    BabyCare_RunSootheMotion();

    if ((now - s_last_sensor_tick) >= BABY_CARE_SENSOR_PERIOD_MS) {
        s_last_sensor_tick = now;
        BabyCare_ReadSensors();
    }

    if ((now - s_last_report_tick) >= BABY_CARE_REPORT_PERIOD_MS) {
        s_last_report_tick = now;
        BabyCare_ReportStatus();
    }
}

void BabyCare_OnUartRxComplete(UART_HandleTypeDef *huart)
{
#if BABY_CARE_USE_VOICE_UART
    (void)Voice_UART_RxCpltCallback(&s_voice, huart);
#else
    (void)huart;
#endif
}

const BabyCare_StatusTypeDef *BabyCare_GetStatus(void)
{
    return &s_status;
}
