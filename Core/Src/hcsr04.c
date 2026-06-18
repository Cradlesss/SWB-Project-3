#include "hcsr04.h"
#include "main.h"
#include "tim.h"

extern void UART_SendText(const char *text);
extern volatile uint32_t appTickMs;

#define TIMEOUT_MS 30u

typedef enum { IDLE, WAIT_ECHO, MEASURING } HcState;

static HcState state = IDLE;
static uint32_t riseCapture = 0;
static uint32_t toutStart = 0;
static int32_t distMm = -1;
static uint8_t ready = 0;

static void TrigDelayUs(uint32_t us) {
    uint32_t period = htim2.Instance->ARR + 1u;
    uint32_t start = htim2.Instance->CNT;
    while (1) {
        uint32_t now = htim2.Instance->CNT;
        uint32_t elapsed = (now >= start) ? (now - start) : (period - start + now);
        if (elapsed >= us) break;
    }
}

void HCSR04_Init(void) {
    HAL_GPIO_WritePin(SONAR_TRIG_GPIO_Port, SONAR_TRIG_Pin, GPIO_PIN_RESET);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
    state = IDLE;
    ready = 0;
    distMm = -1;
}

void HCSR04_Trigger(void) {
    if (state != IDLE) return;
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    riseCapture = 0;
    HAL_GPIO_WritePin(SONAR_TRIG_GPIO_Port, SONAR_TRIG_Pin, GPIO_PIN_RESET);
    TrigDelayUs(2);
    HAL_GPIO_WritePin(SONAR_TRIG_GPIO_Port, SONAR_TRIG_Pin, GPIO_PIN_SET);
    TrigDelayUs(10);
    HAL_GPIO_WritePin(SONAR_TRIG_GPIO_Port, SONAR_TRIG_Pin, GPIO_PIN_RESET);
    toutStart = appTickMs;
    state = WAIT_ECHO;
}

void HCSR04_IC_Callback(TIM_HandleTypeDef *htim) {
    if (htim->Instance != TIM3) return;
    if (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1) return;

    uint32_t captured = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

    if (HAL_GPIO_ReadPin(SONAR_ECHO_GPIO_Port, SONAR_ECHO_Pin) == GPIO_PIN_SET) {
        if (state == WAIT_ECHO) {
            riseCapture = captured;
            state = MEASURING;
        }
    } else {
        if (state == MEASURING) {
            uint32_t width = (captured >= riseCapture) ?
                             (captured - riseCapture) :
                             (65536u - riseCapture + captured);
            int32_t d = (int32_t)(width * 17u / 100u);
            distMm = (d > 20 && d < 4000) ? d : -1;
            ready = 1;
            state = IDLE;
        }
    }
}

void HCSR04_Task(void) {
    if ((state == WAIT_ECHO || state == MEASURING) &&
        (appTickMs - toutStart >= TIMEOUT_MS)) {
        HAL_GPIO_WritePin(SONAR_TRIG_GPIO_Port, SONAR_TRIG_Pin, GPIO_PIN_RESET);
        UART_SendText("HCSR04: timeout (no echo)\r\n");
        distMm = -1;
        ready = 1;
        state = IDLE;
    }
}

int32_t HCSR04_GetDistance_mm(void) { return distMm; }
uint8_t HCSR04_IsReady(void) { return ready; }
void HCSR04_ClearReady(void) { ready = 0; }
