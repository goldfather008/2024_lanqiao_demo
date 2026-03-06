/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct button{
	uint16_t ticks;		//按下的持续时间
	uint8_t repeat;		//重复按下的次数
	uint8_t level;  	//读到的电平
	uint8_t debounce_cnt; //消抖次数
	uint8_t state;      //按键状态
	uint8_t id;         //按键编号
	GPIO_TypeDef *GPIOx; //按键所属的GPIO组
	uint16_t GPIO_Pin;	 //按键所属的引脚编号
}button;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_buffer[30];//串口接收缓冲

button btns[4];

uint8_t led = 0xff; //led的初始状态，1表示熄灭，0是点亮

char dis_str[21];

uint8_t sys_state = 0;	//显示界面
uint8_t data_flag = 0;	// 在数据界面下，按下 B3 按键，切换频率或周期显示模式


/* 全局变量：仅存储频率相关参数 */

uint32_t A_Period = 0;      // PWM周期（单位：μs，计数频率1MHz）
float A_Freq = 0.0f;        // PWM频率（单位：Hz）

uint32_t B_Period = 0;      // PWM周期（单位：μs，计数频率1MHz）
float B_Freq = 0.0f;        // PWM频率（单位：Hz）



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//数据显示界面
void data_display(void)
{
	LCD_DisplayStringLine(Line1, (uint8_t *)"        DATA        ");
	
	if(data_flag == 0)
	{
		//显示频率并转换单位
		if(A_Freq > 1000)
		{
			sprintf(dis_str, "     A=%.2fKHz        ", A_Freq / 1000); 
		}else
		{
			sprintf(dis_str, "     A=%dHz        ", (int)A_Freq);
		}
		LCD_DisplayStringLine(Line4, (uint8_t *)dis_str);
		if(B_Freq > 1000)
		{
			sprintf(dis_str, "     B=%.2fKHz        ", B_Freq / 1000); 
		}else
		{
			sprintf(dis_str, "     B=%dHz        ", (int)B_Freq);
		}
		LCD_DisplayStringLine(Line5, (uint8_t *)dis_str);
	}else
	{
		//显示周期并转换单位
		if(A_Period > 1000)
		{
			sprintf(dis_str, "     A=%.2fmS        ", (float)A_Period / 1000); 
		}else
		{
			sprintf(dis_str, "     A=%duS        ", A_Period);
		}
		LCD_DisplayStringLine(Line4, (uint8_t *)dis_str);
		if(B_Period > 1000)
		{
			sprintf(dis_str, "     B=%.2fmS        ", (float)B_Period / 1000); 
		}else
		{
			sprintf(dis_str, "     B=%duS        ", B_Period);
		}
		LCD_DisplayStringLine(Line5, (uint8_t *)dis_str);
	}
	
}
//参数显示界面
void para_display(void)
{
	LCD_DisplayStringLine(Line1, (uint8_t *)"        PARA        ");
	LCD_DisplayStringLine(Line4, (uint8_t *)"     PD=1000Hz      ");
	LCD_DisplayStringLine(Line5, (uint8_t *)"     PH=1000Hz      ");
	LCD_DisplayStringLine(Line6, (uint8_t *)"     PX=200Hz      ");
}
//统计界面
void recd_display(void)
{
	LCD_DisplayStringLine(Line1, (uint8_t *)"        RECD        ");
	LCD_DisplayStringLine(Line4, (uint8_t *)"     NDA=3          ");
	LCD_DisplayStringLine(Line5, (uint8_t *)"     NDB=1          ");
	LCD_DisplayStringLine(Line6, (uint8_t *)"     NHA=0          ");
	LCD_DisplayStringLine(Line7, (uint8_t *)"     NHB=2          ");
}

//更新LED函数
void update_led(void)
{
	GPIOC->ODR &= 0x00ff;//保留低8位，对高8位清零
	GPIOC->ODR |= led << 8; //设置高8位
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 1);//更新输出
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 0);
}

void long_press_handler(uint8_t id)
{
	switch(id){
		case 0:
			printf("B1长按...\r\n");
			break;
		case 1:
			printf("B2长按...\r\n");
			break;
		case 2:
			printf("B3长按...\r\n");
			break;
		case 3:
			printf("B4长按...\r\n");
			break;
			
	}
}

void single_click_handler(uint8_t id)
{

	switch(id){
		case 0:
			printf("B1单击...\r\n");

			break;
		case 1:
			printf("B2单击...\r\n");

			break;
		case 2:
			printf("B3单击...\r\n");
			if(sys_state == 0)
			{
				//在数据显示界面下，切换频率或周期显示模式
				data_flag == 0 ? (data_flag = 1) : (data_flag = 0);
			}
			break;
		case 3:
			printf("界面切换...\r\n");
			LCD_Clear(Black); // 先清屏
		    sys_state == 2 ? (sys_state = 0) : (sys_state++);

			break;
			
	}
}

//键盘扫描函数
void button_handler(button* btn)
{
	uint8_t gpio_level = HAL_GPIO_ReadPin(btn->GPIOx, btn->GPIO_Pin);
	
	//按键从按下开始，记录ticks
	if(btn->state > 0) btn->ticks++;
	
	if(gpio_level != btn->level){
		if(++(btn->debounce_cnt) >= 3){
		//读3次都是同一个电平
			btn->level = gpio_level;
			btn->debounce_cnt = 0;
		}
	
	}else{
		//电平没有改变，重新消抖
		btn->debounce_cnt = 0;
	}

	//按键状态机
	switch(btn->state){
		case 0:
			if(btn->level == 0){		//按下
				btn->ticks = 0; // 开始计时
				btn->repeat = 1; //记录按下次数
				btn->state = 1;
			}
			break;
		case 1:
			if(btn->level != 0){		//按键松开
				btn->ticks = 0; //重新计时
				btn->state = 2; 
			}else if(btn->ticks > 50){						//500ms没松开，发生长按事件
				//开始长按
				//printf("长按...\r\n");
				long_press_handler(btn->id);
				btn->ticks = 0;	// 重新计时
				btn->repeat = 0; // 防止释放的时候再次触发单击事件
			}
			break;
		case 2:
			  if(btn->ticks > 15){
				//松开了150ms以上，可能是单击、双击或长按释放
				if(btn->repeat == 1){
				  //printf("单击...\r\n");
					single_click_handler(btn->id);
				}else if(btn->repeat == 2){
				  //printf("双击...\r\n");
				}
				btn->state = 0;
			  }else{
				//检测是否再按下
				if (btn->level == 0){
				  btn->repeat++;
				  btn->ticks = 0;
				  btn->state = 1;
				}
				
			  }
			
			break;

	}

}

float get_adc2(void)
{
	uint32_t temp = 0;
	//采用10次
	for (uint8_t i = 0; i < 10; i++)
	{
		HAL_ADC_Start(&hadc2);
		//等待转换结束
		if(HAL_ADC_PollForConversion(&hadc2, 100) == HAL_OK){
			temp += HAL_ADC_GetValue(&hadc2);
		}
	}
	temp = temp / 10; //取平均值
	return temp * 3.3 / 4095;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	
   uint32_t tick1=uwTick;
   uint32_t tick2=uwTick;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
 	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  printf("串口测试...\r\n");
  update_led();//LED状态更新

  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer,sizeof(rx_buffer));
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);
  
  btns[0].GPIOx = GPIOB;
  btns[0].GPIO_Pin = GPIO_PIN_0;
  btns[0].level = 1;
  btns[0].id = 0;
  btns[1].GPIOx = GPIOB;
  btns[1].GPIO_Pin = GPIO_PIN_1;
  btns[1].level = 1;
  btns[1].id = 1;
  btns[2].GPIOx = GPIOB;
  btns[2].GPIO_Pin = GPIO_PIN_2;
  btns[2].level = 1;
  btns[2].id = 2;
  btns[3].GPIOx = GPIOA;
  btns[3].GPIO_Pin = GPIO_PIN_0;
  btns[3].level = 1;
  btns[3].id = 3;
  
  
	LCD_Init();

	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
  // 启用定时器捕获模式
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if(uwTick >= tick1 + 10)  //间隔10ms执行键盘扫描
	{
		
		for(uint8_t i=0; i<4; i++){
			button_handler(&btns[i]);
		}
		tick1 = uwTick;   //更新节拍
	}
		
	if(uwTick >= tick2 + 100)  //间隔100ms 界面数据刷新一次
	{
		
		switch(sys_state){
		case 0:
			data_display();
			break;
		case 1:
			para_display();
			break;
		case 2:
			recd_display();
			break;
		}
		tick2 = uwTick;   //更新节拍
		
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == &huart1)
	{
		printf("接收到%d个字符：%s\r\n", Size, rx_buffer);

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer,sizeof(rx_buffer));
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);
	}

}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
	  A_Period = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1); //两个上升沿之间的计数值即周期，单位us
	  A_Freq = 1000000.0f / A_Period;//单位Hz
  }
  
  if (htim == &htim3)
  {
	  B_Period = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1);
	  B_Freq = 1000000.0f / B_Period;//单位Hz
  }
  

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

