/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Scanning Sonar – NUCLEO-G491RE
  ******************************************************************************
  * HARDWARE:  STM32G491RETx, 170 MHz
  * PERIPHERALS:
  *   TIM7  – 1 ms app tick (Prescaler=16999, Period=9, IRQ)
  *   TIM2  – us counter for HC-SR04 (Prescaler=169, Period=65535)
  *   TIM3  – Servo PWM CH1 PB4 (Prescaler=169, Period=19999, 50 Hz)
  *   SPI1  – TFT ILI9341 + Touch XPT2046 (PA5/PA6/PA7, DMA Ch3 TX)
  *   USART2 – Debug UART (PA2/PA3, DMA Ch2 TX)
  * PIN MAP:
  *   PB0=HCSR04_TRIG  PB1=HCSR04_ECHO(EXTI1,pri1)
  *   PB2=TFT_RST      PB4=SERVO_PWM(TIM3_CH1)
  *   PB10=TOUCH_CS    PC4=TFT_CS   PC5=TFT_DC
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "hcsr04.h"
#include "servo.h"
#include "tft_ili9341.h"
#include "touch_xpt2046.h"
#include "scanner.h"
#include "gui.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define TX_MSG_MAX_LEN  120
#define TX_QUEUE_SIZE    16
/* USER CODE END PD */

/* USER CODE BEGIN PV */
volatile uint32_t appTickMs = 0;

static int  txBusy  = 0;
static char txCurBuf[TX_MSG_MAX_LEN];
static char txQueue[TX_QUEUE_SIZE][TX_MSG_MAX_LEN];
static int  txQHead  = 0;
static int  txQTail  = 0;
static int  txQCount = 0;
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
uint32_t APP_GetTick(void);
void     UART_SendText(const char *text);
void     UART_StartNextTx(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
uint32_t APP_GetTick(void) { return appTickMs; }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM7) appTickMs++;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == HCSR04_ECHO_Pin) HCSR04_EchoIRQ();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        txBusy = 0;
        UART_StartNextTx();
    }
}

void UART_StartNextTx(void) {
    if (txBusy || txQCount == 0) return;
    strncpy(txCurBuf, txQueue[txQTail], TX_MSG_MAX_LEN - 1);
    txCurBuf[TX_MSG_MAX_LEN - 1] = 0;
    if (++txQTail >= TX_QUEUE_SIZE) txQTail = 0;
    txQCount--;
    txBusy = 1;
    if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *)txCurBuf,
                               (uint16_t)strlen(txCurBuf)) != HAL_OK)
        txBusy = 0;
}

void UART_SendText(const char *text) {
    if (!text || txQCount >= TX_QUEUE_SIZE) return;
    strncpy(txQueue[txQHead], text, TX_MSG_MAX_LEN - 1);
    txQueue[txQHead][TX_MSG_MAX_LEN - 1] = 0;
    if (++txQHead >= TX_QUEUE_SIZE) txQHead = 0;
    txQCount++;
    UART_StartNextTx();
}
/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM7_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim7);

  HCSR04_Init();
  Servo_Init();
  TFT_Init();
  Touch_Init();
  Scanner_Init();
  GUI_Init();

  UART_SendText("Sonar ready\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HCSR04_Task();
    Scanner_Task();
    GUI_Task();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN            = 85;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) Error_Handler();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1) {}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
