#include "hcsr04.h"
#include "main.h"
#include "tim.h"

#define TIMEOUT_MS 30u

typedef enum { IDLE, TRIG_HIGH, WAIT_ECHO, MEASURING } HcState;

static HcState  state      = IDLE;
static uint32_t trigStart  = 0;
static uint32_t echoStart  = 0;
static uint32_t toutStart  = 0;
static int32_t  distMm     = -1;
static uint8_t  ready      = 0;

extern volatile uint32_t appTickMs;

static uint32_t GetUs(void) { return htim2.Instance->CNT; }

static uint32_t ElapsedUs(uint32_t from) {
    uint32_t now = GetUs();
    return (now >= from) ? (now - from) : (65536u - from + now);
}

void HCSR04_Init(void) {
    HAL_TIM_Base_Start(&htim2);
    state = IDLE;
    ready = 0;
}

void HCSR04_Trigger(void) {
    if (state != IDLE) return;
    HAL_GPIO_WritePin(HCSR04_TRIG_GPIO_Port, HCSR04_TRIG_Pin, GPIO_PIN_SET);
    trigStart = GetUs();
    toutStart = appTickMs;
    state     = TRIG_HIGH;
}

void HCSR04_EchoIRQ(void) {
    if (HAL_GPIO_ReadPin(HCSR04_ECHO_GPIO_Port, HCSR04_ECHO_Pin) == GPIO_PIN_SET) {
        if (state == WAIT_ECHO) {
            echoStart = GetUs();
            state     = MEASURING;
        }
    } else {
        if (state == MEASURING) {
            uint32_t dur = ElapsedUs(echoStart);
            int32_t  d   = (int32_t)(dur * 17u / 100u); /* us * 0.17 mm/us */
            distMm = (d > 20 && d < 4000) ? d : -1;
            ready  = 1;
            state  = IDLE;
        }
    }
}

void HCSR04_Task(void) {
    if (state == TRIG_HIGH && ElapsedUs(trigStart) >= 10u) {
        HAL_GPIO_WritePin(HCSR04_TRIG_GPIO_Port, HCSR04_TRIG_Pin, GPIO_PIN_RESET);
        state = WAIT_ECHO;
    }

    if ((state == WAIT_ECHO || state == MEASURING) &&
        (appTickMs - toutStart >= TIMEOUT_MS)) {
        HAL_GPIO_WritePin(HCSR04_TRIG_GPIO_Port, HCSR04_TRIG_Pin, GPIO_PIN_RESET);
        distMm = -1;
        ready  = 1;
        state  = IDLE;
    }
}

int32_t HCSR04_GetDistance_mm(void) { return distMm; }
uint8_t HCSR04_IsReady(void)        { return ready;  }
void    HCSR04_ClearReady(void)     { ready = 0;     }
