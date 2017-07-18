/*
 * ili9341_spi.c
 *
 *  Created on: 2017. júl. 18.
 *      Author: bekeband
 */

#include "stm32f1xx_hal.h"
#include "spi.h"
#include "ili9341_spi.h"


/*
 * @brief ILI9341 DISPLAY IC SPI handle. SPI1 channel.
 */

static SPI_HandleTypeDef 	display_spi1_handle =
{.Instance = SPI1,
.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_256,
.Init.Direction          = SPI_DIRECTION_2LINES,
.Init.CLKPhase           = SPI_PHASE_2EDGE,
.Init.CLKPolarity        = SPI_POLARITY_HIGH,
.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE,
.Init.CRCPolynomial      = 7,
.Init.DataSize           = SPI_DATASIZE_8BIT,
.Init.FirstBit           = SPI_FIRSTBIT_MSB,
.Init.NSS                = SPI_NSS_SOFT,
.Init.TIMode             = SPI_TIMODE_DISABLE,
.Init.Mode               = SPI_MODE_MASTER
};

/* @brief DISPLAY_SPI1_Init(): initialize SPI 1 signals for ILI9341 DISPLAY SPI communication,
 * and set SPI channel, and initialize CS line for standard I/O. */


void DISPLAY_SPI1_Init()
{
	  GPIO_InitTypeDef  gpioinitstruct = {0};

	  /* Enable SPI1, and signal ports clock. */
		__HAL_RCC_SPI1_CLK_ENABLE();
		SPI1_SIGNAL_PORT_CLK_ENABLE();

	  /* configure SPI1 SCK (GPIOA 5), and SPI1 MOSI (GPIOA 7)*/

	  gpioinitstruct.Pin        = SPI1_CLK_PIN | SPI1_MOSI_PIN;
	  gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
	  gpioinitstruct.Pull       = GPIO_NOPULL;
	  gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(SPI1_SIGNAL_PORT, &gpioinitstruct);

	/*
	*  Configures display control ports to outputs.
	*  */

	  gpioinitstruct.Pin        = DISPLAY_CS_PIN | DISPLAY_DC_PIN | DISPLAY_RST_PIN | DISPLAY_LCD_PIN;
	  gpioinitstruct.Mode       = GPIO_MODE_OUTPUT_PP;
	  gpioinitstruct.Pull       = GPIO_NOPULL;
	  gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(DISPLAY_CONTROL_PORT, &gpioinitstruct);

	  /* Configure SPI MISO to input GPIOA 6 .   */

	  gpioinitstruct.Pin        = SPI1_MISO_PIN;
	  gpioinitstruct.Mode       = GPIO_MODE_AF_INPUT;
	  gpioinitstruct.Pull       = GPIO_PULLUP;
	  gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(SPI1_SIGNAL_PORT, &gpioinitstruct);

	  SPI_Init(&display_spi1_handle);

}
