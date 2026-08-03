/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Buck Converter 24V to 5V)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* Sensor and reading variables */
uint16_t adc_buffer[2];
float out_voltage = 0.0;
float out_current = 0.0;
float raw_voltage = 0.0;
float raw_current = 0.0;

/* System and Control Variables */
uint8_t System_Mode = 1;  /* 0: Manual, 1: PI_V, 2: PI_I */
uint8_t Relay_State = 0;  /* 0: OFF, 1: ON */
int Duty_Cycle = 0;       /* 0 - 399 */
float I_Limit = 12.0;     /* OCP Limit */

/* Voltage PI Parameters */
float V_Ref = 0.0;
float Kp_V = 0.0;
float Ki_V = 0.0;
float V_Cal = 1.0;
float V_Offset = 0.0;
float integral_V = 0.0;

/* Current PI Parameters */
float I_Ref = 0.0;
float Kp_I = 0.0;
float Ki_I = 0.0;
float I_Cal = 1.0;
float I_Offset = 0.0;
float integral_I = 0.0;

/* Manual Mode Parameters */
float M_Offset = 0.0;

/* UART RX Buffers */
uint8_t rx_byte;
char rx_line_buffer[64];
uint8_t rx_line_index = 0;
volatile uint8_t rx_line_ready = 0;
char rx_line_copy[64];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Process_Nextion_Command(char *line);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USB_PCD_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

  /* Start UART Interrupt */
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* ADC Reading and Calibration Block */
    float pin_voltage_v = ((float)adc_buffer[1] / 4095.0) * 3.3;
    float pin_voltage_i = ((float)adc_buffer[0] / 4095.0) * 3.3;

    raw_voltage = pin_voltage_v * 2.0;
    float sensor_viout = pin_voltage_i * 1.5;
    raw_current = (sensor_viout - 2.5) * 10.0;

    out_voltage = (raw_voltage * V_Cal) + V_Offset;
    out_current = (raw_current * I_Cal) + I_Offset;

    /* UART Parsing Block */
    if (rx_line_ready)
    {
        Process_Nextion_Command(rx_line_copy);
        rx_line_ready = 0;
    }

    /* OCP (Over Current Protection) Block */
    if (out_current > I_Limit)
    {
        Relay_State = 0;
        Duty_Cycle = 0;
    }

    /* Control Algorithms Block (PI & Manual) */
    if (Relay_State == 0)
    {
        /* System OFF: Reset PWM and Integrals */
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
        integral_V = 0.0;
        integral_I = 0.0;
    }
    else
    {
        /* Voltage PI Control */
        if (System_Mode == 1)
        {
            float error = V_Ref - out_voltage;
            integral_V += (Ki_V * error);

            if (integral_V > 399.0) integral_V = 399.0;
            if (integral_V < 0.0) integral_V = 0.0;

            float calc_duty = (Kp_V * error) + integral_V;

            if (calc_duty > 399.0) calc_duty = 399.0;
            if (calc_duty < 0.0) calc_duty = 0.0;

            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (uint32_t)calc_duty);
        }
        /* Current PI Control */
        else if (System_Mode == 2)
        {
            float error = I_Ref - out_current;
            integral_I += (Ki_I * error);

            if (integral_I > 399.0) integral_I = 399.0;
            if (integral_I < 0.0) integral_I = 0.0;

            float calc_duty = (Kp_I * error) + integral_I;

            if (calc_duty > 399.0) calc_duty = 399.0;
            if (calc_duty < 0.0) calc_duty = 0.0;

            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (uint32_t)calc_duty);
        }
        /* Manual Duty Control */
        else if (System_Mode == 0)
        {
            if (Duty_Cycle > 399) Duty_Cycle = 399;
            if (Duty_Cycle < 0) Duty_Cycle = 0;
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (uint32_t)Duty_Cycle);
        }
    }

    /* Nextion Telemetry Update Block */
    static uint32_t last_uart_time = 0;
    if (HAL_GetTick() - last_uart_time > 200)
    {
        float out_power = out_voltage * out_current;
        if (out_power < 0) out_power = 0; /* Prevent negative power display */

        char uart_buf[64];
        uint8_t nextion_end[3] = {0xFF, 0xFF, 0xFF};
        int len;

        /* t0: Current(A), t2: Voltage(V), t4: Power(W) */
        len = sprintf(uart_buf, "t0.txt=\"%.2f\"", out_current);
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 50);
        HAL_UART_Transmit(&huart1, nextion_end, 3, 50);

        len = sprintf(uart_buf, "t2.txt=\"%.2f\"", out_voltage);
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 50);
        HAL_UART_Transmit(&huart1, nextion_end, 3, 50);

        len = sprintf(uart_buf, "t4.txt=\"%.2f\"", out_power);
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 50);
        HAL_UART_Transmit(&huart1, nextion_end, 3, 50);

        last_uart_time = HAL_GetTick();
    }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* UART RX Interrupt Callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (rx_byte == '\n')
        {
            rx_line_buffer[rx_line_index] = '\0';
            strcpy(rx_line_copy, rx_line_buffer);
            rx_line_ready = 1;
            rx_line_index = 0;
        }
        else if (rx_byte != '\r' && rx_line_index < (sizeof(rx_line_buffer) - 1))
        {
            rx_line_buffer[rx_line_index++] = rx_byte;
        }
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

/* Nextion Data Parser */
void Process_Nextion_Command(char *line)
{
    char *colon_ptr = strchr(line, ':');
    if (colon_ptr != NULL)
    {
        float value = atof(colon_ptr + 1);

        /* System Commands */
        if (strncmp(line, "MODE", 4) == 0) System_Mode = (uint8_t)value;
        else if (strncmp(line, "RELAY", 5) == 0) Relay_State = (uint8_t)value;

        /* Voltage PI Commands */
        else if (strncmp(line, "SETV", 4) == 0) V_Ref = value;
        else if (strncmp(line, "KPV", 3) == 0) Kp_V = value;
        else if (strncmp(line, "KIV", 3) == 0) Ki_V = value;
        else if (strncmp(line, "OFFV", 4) == 0) V_Offset = value;
        else if (strncmp(line, "CALV", 4) == 0) V_Cal = value;

        /* Current PI Commands */
        else if (strncmp(line, "SETI", 4) == 0) I_Ref = value;
        else if (strncmp(line, "KPI", 3) == 0) Kp_I = value;
        else if (strncmp(line, "KII", 3) == 0) Ki_I = value;
        else if (strncmp(line, "OFFI", 4) == 0) I_Offset = value;
        else if (strncmp(line, "CALI", 4) == 0) I_Cal = value;

        /* Manual Commands */
        else if (strncmp(line, "DUTY", 4) == 0) Duty_Cycle = (int)value;
        else if (strncmp(line, "OFFM", 4) == 0) M_Offset = value;
    }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
