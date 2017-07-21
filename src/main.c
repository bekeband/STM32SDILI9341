

/* The indicator LED connect to C port 13-th PIN */

#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "interrupts.h"
#include "init.h"
#include "sd_spi.h"
#include "ili9341_spi.h"

extern s_image button;

/**
 System Clock, and osc configuration for 8000000 Hz Minimal STM 32 board.
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}


uint8_t buffer[1000];
uint8_t i = 0;

char cid_string[6];

uint16_t colors[8] = {	0b1111100000000000,
						0b0000011111100000,
						0b0000000000011111};

int main()
{
	/* Enable the DIVZERO exception. */
  SCB->CCR |= SCB_CCR_DIV_0_TRP;
	HAL_Init();
	SystemClock_Config();
	
	Init_ErrLED();
	
	SD_SPI2_Init(); // Initialize SPI2 for SD card. 

	DISPLAY_SPI1_Init();
	ILI9341_Init();

	ILI9341_fillrectangle(0, 0, 240, 320, 0xFFFF);
	ILI9341_displaybitmap(10, 10, 34, 54, &button);

/* Main program loop. */
	while (1)
	{	
		
		while (1)
		{

			HAL_Delay(50);

//			DISPLAY_ON();
//			ILI9341_fillrectangle(0, 0, 240, 320, colors[i]);
			if (++i > 2) i = 0;
			DisplaySoftOn();
			HAL_Delay(50);
//			DisplaySoftOff();
//			DISPLAY_OFF();
		}

		if (GetSDCardCheckFlag())
		{

			SD_Card_SPI_Select();
			if (ResetCard() == SD_SPI_OK)
			{	
				ForceErrorNumber(4);
				SPIModeInitialize();
				GetCIDRegister(cid_string);
//				GetCSDRegister();
				ReadDataBlock(i++, (uint8_t*)&buffer);
			} else
			{
				ForceErrorNumber(2);				
			}
				
			ClearSDCardCheckFlag();
		}
	}		
	
	return 0;
	}
