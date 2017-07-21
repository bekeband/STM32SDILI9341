/*
 * ili9341_spi.c
 *
 *  Created on: 2017. j�l. 18.
 *      Author: bekeband
 */

#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "spi.h"
#include "ili9341_spi.h"
#include "interrupts.h"

/*
 * ILI9341 initialization data constants ------------------------------------
 */
const uint8_t datas_pwa[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
const uint8_t datas_pwb[] = {0x00, 0xC1, 0x30};
const uint8_t datas_dtca[] = {0x85, 0x00, 0x78};
const uint8_t datas_dtcb[] = {0x00, 0x00};
const uint8_t datas_pw_seq[] = {0x64, 0x03, 0x12, 0x81};
const uint8_t datas_prc[] = {0x20};
const uint8_t datas_pw1[] = {0x23};
const uint8_t datas_pw2[] = {0x10};
const uint8_t datas_vcom1[] = {0x3E, 0x28};
const uint8_t datas_vcom2[] = {0x86};
const uint8_t datas_mac[] = {0x48};
const uint8_t datas_pform[] = {0x55};
const uint8_t datas_frc[] = {0x00, 0x18};
const uint8_t datas_dfc[] = {0x08, 0x82, 0x27};
const uint8_t datas_gen[] = {0x00};
const uint8_t datas_caddr[] = {0x00, 0x00, 0x00, 0xEF};
const uint8_t datas_paddr[] = {0x00, 0x00, 0x01, 0x3F};
const uint8_t datas_gamma[] = {0x01};
const uint8_t datas_pgamma[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00};
const uint8_t datas_ngamma[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F};

#if defined (ILI9341_DMA)
static DMA_HandleTypeDef hdma_spi1_tx;
static DMA_HandleTypeDef hdma_spi1_rx;

e_dma_transfer_state wTransferState;

#endif



/*
 * @brief ILI9341 DISPLAY IC SPI handle. SPI1 channel.
 */

static SPI_HandleTypeDef 	display_spi1_handle =
{.Instance = SPI1,
.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_2,
.Init.Direction          = SPI_DIRECTION_2LINES,
.Init.CLKPhase           = SPI_PHASE_1EDGE,
.Init.CLKPolarity        = SPI_POLARITY_LOW,
.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE,
.Init.CRCPolynomial      = 7,
.Init.DataSize           = SPI_DATASIZE_8BIT,
.Init.FirstBit           = SPI_FIRSTBIT_MSB,
.Init.NSS                = SPI_NSS_SOFT,
.Init.TIMode             = SPI_TIMODE_DISABLE,
.Init.Mode               = SPI_MODE_MASTER
};

DMA_HandleTypeDef OpenMemoryToDisplayDMAChannel(SPI_HandleTypeDef* handle)
{	DMA_HandleTypeDef result;

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(handle, hdmatx, result);

	/* Configure the DMA handler for Transmission process */
	result.Instance                 	= DISPLAY_TX_DMA_CHANNEL;
	result.Init.Direction           	= DMA_MEMORY_TO_PERIPH;
	result.Init.PeriphInc           	= DMA_PINC_DISABLE;
	result.Init.MemInc              	= DMA_MINC_ENABLE;
	result.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
	result.Init.MemDataAlignment    	= DMA_MDATAALIGN_BYTE;
	result.Init.Mode                	= DMA_NORMAL;
	result.Init.Priority            	= DMA_PRIORITY_HIGH;

	HAL_DMA_Init(&result);

    /*##-4- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (SPI2_TX) */
    HAL_NVIC_SetPriority(DISPLAY_DMA_TX_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(DISPLAY_DMA_TX_IRQn);


	return result;
}

DMA_HandleTypeDef OpenDisplayToMemoryDMAChannel(SPI_HandleTypeDef* handle)
{	DMA_HandleTypeDef result;

	/* Configure the DMA handler for Transmission process */
	result.Instance                 	= DISPLAY_RX_DMA_CHANNEL;
	result.Init.Direction           	= DMA_PERIPH_TO_MEMORY;
	result.Init.PeriphInc           	= DMA_PINC_DISABLE;
	result.Init.MemInc              	= DMA_MINC_ENABLE;
	result.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
	result.Init.MemDataAlignment    	= DMA_MDATAALIGN_BYTE;
	result.Init.Mode                	= DMA_NORMAL;
	result.Init.Priority            	= DMA_PRIORITY_HIGH;

	HAL_DMA_Init(&result);

    /*##-4- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (SPI2_TX) */
    HAL_NVIC_SetPriority(DISPLAY_DMA_RX_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(DISPLAY_DMA_RX_IRQn);

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(handle, hdmatx, result);

	return result;
}


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

#if defined (ILI9341_DMA)

	  /* Enable DMA clock */
	  __HAL_RCC_DMA1_CLK_ENABLE();

	  hdma_spi1_tx.Instance                 	= DISPLAY_TX_DMA_CHANNEL;
	  hdma_spi1_tx.Init.Direction           	= DMA_MEMORY_TO_PERIPH;
	  hdma_spi1_tx.Init.PeriphInc           	= DMA_PINC_DISABLE;
	  hdma_spi1_tx.Init.MemInc              	= DMA_MINC_ENABLE;
	  hdma_spi1_tx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
	  hdma_spi1_tx.Init.MemDataAlignment    	= DMA_MDATAALIGN_BYTE;
	  hdma_spi1_tx.Init.Mode                	= DMA_NORMAL;
	  hdma_spi1_tx.Init.Priority            	= DMA_PRIORITY_HIGH;

	  HAL_DMA_Init(&hdma_spi1_tx);

	  __HAL_LINKDMA(&display_spi1_handle, hdmatx, hdma_spi1_tx);

	  HAL_NVIC_SetPriority(DISPLAY_DMA_TX_IRQn, 1, 1);
	  HAL_NVIC_EnableIRQ(DISPLAY_DMA_TX_IRQn);


#endif
//	  SELECT_DISPLAY();

}

void ILI9341_writecmd(uint8_t cmd)
{
	SELECT_DISPLAY();
	SELECT_COMMAND();
	if (HAL_SPI_Transmit(&display_spi1_handle, &cmd, sizeof(uint8_t), DISPLAY_SPI_TRANSMIT_TIMEOUT) != HAL_OK)
	{

	}
	DESELECT_DISPLAY();
}

void ILI9341_writedatas(uint8_t* data, int size)
{
	SELECT_DISPLAY();
	SELECT_DATA();
	if (HAL_SPI_Transmit(&display_spi1_handle, data, size, DISPLAY_SPI_TRANSMIT_TIMEOUT) != HAL_OK)
	{

	}
	DESELECT_DISPLAY();
}

void ILI9341_writedata(uint8_t data)
{
	SELECT_DISPLAY();
	SELECT_DATA();
	if (HAL_SPI_Transmit(&display_spi1_handle, &data, sizeof(uint8_t), DISPLAY_SPI_TRANSMIT_TIMEOUT) != HAL_OK)
	{

	}
	DESELECT_DISPLAY();
}

void ILI9341_Init()
{
	/* Force reset */
	RESET_ACTIVE();
	HAL_Delay(200);
	RESET_PASSIVE();
	/* Delay for RST response */
	HAL_Delay(200);

	/* Software reset */
	ILI9341_writecmd(ILI9341_RESET);
	HAL_Delay(200);

	ILI9341_writecmd(ILI9341_POWERA);
	ILI9341_writedatas((uint8_t*)&datas_pwa, sizeof(datas_pwa));
	ILI9341_writecmd(ILI9341_POWERB);
	ILI9341_writedatas((uint8_t*)&datas_pwb, sizeof(datas_pwb));
	ILI9341_writecmd(ILI9341_DTCA);
	ILI9341_writedatas((uint8_t*)&datas_dtca, sizeof(datas_dtca));
	ILI9341_writecmd(ILI9341_DTCB);
	ILI9341_writedatas((uint8_t*)&datas_dtcb, sizeof(datas_dtcb));
	ILI9341_writecmd(ILI9341_POWER_SEQ);
	ILI9341_writedatas((uint8_t*)&datas_pw_seq, sizeof(datas_pw_seq));
	ILI9341_writecmd(ILI9341_PRC);
	ILI9341_writedatas((uint8_t*)&datas_prc, sizeof(datas_prc));
	ILI9341_writecmd(ILI9341_POWER1);
	ILI9341_writedatas((uint8_t*)&datas_pw1, sizeof(datas_pw1));
	ILI9341_writecmd(ILI9341_POWER2);
	ILI9341_writedatas((uint8_t*)&datas_pw2, sizeof(datas_pw2));
	ILI9341_writecmd(ILI9341_VCOM1);
	ILI9341_writedatas((uint8_t*)&datas_vcom1, sizeof(datas_vcom1));
	ILI9341_writecmd(ILI9341_VCOM2);
	ILI9341_writedatas((uint8_t*)&datas_vcom2, sizeof(datas_vcom2));
	ILI9341_writecmd(ILI9341_MAC);

	#ifdef RGB_BGR_COLOR
	  #ifdef ROW_COL_EXCH
	    SendData(0x68);
	  #else
	    ILI9341_writedatas((uint8_t*)&datas_mac, sizeof(datas_mac));
	  #endif
	#else
	  #ifdef ROW_COL_EXCH
	  #else
	    SendData(0x60);
	#endif
	  SendData(0x40);
	#endif
	  ILI9341_writecmd(ILI9341_PIXEL_FORMAT);
	#ifdef PIXEL_FORMAT_18_BIT
	  SendData(0x66);	// 18 bits pixel format
	#else
	  ILI9341_writedatas((uint8_t*)&datas_pform, sizeof(datas_pform));
	#endif
	  ILI9341_writecmd(ILI9341_FRC);
	  ILI9341_writedatas((uint8_t*)&datas_frc, sizeof(datas_frc));
	  ILI9341_writecmd(ILI9341_DFC);
	  ILI9341_writedatas((uint8_t*)&datas_dfc, sizeof(datas_dfc));
	  ILI9341_writecmd(ILI9341_3GAMMA_EN);
	  ILI9341_writedatas((uint8_t*)&datas_gen, sizeof(datas_gen));
	  ILI9341_writecmd(ILI9341_COLUMN_ADDR);
	  ILI9341_writedatas((uint8_t*)&datas_caddr, sizeof(datas_caddr));
	  ILI9341_writecmd(ILI9341_PAGE_ADDR);
	  ILI9341_writedatas((uint8_t*)&datas_paddr, sizeof(datas_paddr));
	  ILI9341_writecmd(ILI9341_GAMMA);
	  ILI9341_writedatas((uint8_t*)&datas_gamma, sizeof(datas_gamma));
	  ILI9341_writecmd(ILI9341_PGAMMA);
	  ILI9341_writedatas((uint8_t*)&datas_pgamma, sizeof(datas_pgamma));
	  ILI9341_writecmd(ILI9341_NGAMMA);
	  ILI9341_writedatas((uint8_t*)&datas_ngamma, sizeof(datas_ngamma));
	  ILI9341_writecmd(ILI9341_SLEEP_OUT);

	  HAL_Delay(200);

	  ILI9341_writecmd(ILI9341_DISPLAY_ON);
	  ILI9341_writecmd(ILI9341_GRAM);
}

void ILI9341_setaddr(uint8_t x1,uint8_t y1,uint16_t x2,uint16_t y2)
{
	ILI9341_writecmd(0x2A);
	ILI9341_writedata(x1>>8);
	ILI9341_writedata(x1);
	ILI9341_writedata(x2>>8);
	ILI9341_writedata(x2);

	ILI9341_writecmd(0x2B);
	ILI9341_writedata(y1>>8);
	ILI9341_writedata(y1);
	ILI9341_writedata(y2>>8);
	ILI9341_writedata(y2);

	ILI9341_writecmd(0x2C);
}

void DisplaySoftOn()
{
	ILI9341_writecmd(ILI9341_DISPLAY_ON);
}

void DisplaySoftOff()
{
	ILI9341_writecmd(ILI9341_DISPLAY_OFF);
}

#if defined (ILI9341_DMA)

/*
 *
 */

HAL_StatusTypeDef SPI_Transmit_DMA_HW(SPI_HandleTypeDef *hspi, void* pData, uint32_t Size)
{ 	int32_t remain = Size; uint32_t DT;
	uint8_t last_remain = 0;
do
{
	if ((remain - 0x10000) >= 0) { DT = 0xFFFF; remain -= 0x10000lu;
	} else
	{ last_remain = 1; DT = remain; }

    wTransferState = TRANSFER_WAIT;
    if (HAL_SPI_Transmit_DMA(hspi, pData, DT) != HAL_OK)
    {
  	  return HAL_ERROR;
    }
    while (wTransferState != TRANSFER_COMPLETE){ };
} while (!last_remain);
return HAL_OK;
}

#endif

/*
 * HAL_StatusTypeDef ILI9341_displaybitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, void* buffer, int BufSize)
 */

HAL_StatusTypeDef ILI9341_displaybitmap(uint16_t x, uint16_t y, uint16_t widthi, uint16_t heighti, s_image* image)
{

	ILI9341_setaddr(x, y, x + image->width - 1, y + image->height - 1);
	ILI9341_writecmd(ILI9341_GRAM);

#if defined (ILI9341_DMA)

	/*
	 * Make the filled buffer for to fill the color bytes.
	 */

	uint8_t* fillbuffer; int i, j; int32_t DT, remain; int last_remain; HAL_StatusTypeDef result;
	uint8_t* pixelptr = image->pixel_data;

	  SELECT_DATA();
	  remain = (image->width * image->height * BYTE_PER_PIXEL);
	  do
	  {
	  	if ((remain - FILLRECT_BUFFER_SIZE) > 0)
	  	{
	  		last_remain = 0;
	  		DT = FILLRECT_BUFFER_SIZE;
	  		remain -= FILLRECT_BUFFER_SIZE;
	  	} else
	  	{
	  		last_remain = 1;
	  		DT = remain;
	  	}

	  	  wTransferState = TRANSFER_WAIT;
	  	  if (HAL_SPI_Transmit_DMA(&display_spi1_handle, pixelptr, DT) != HAL_OK)
	  	    {
	  	  	  result = HAL_ERROR;
	  	    }
	  	  pixelptr += FILLRECT_BUFFER_SIZE;
	  	  while (wTransferState != TRANSFER_COMPLETE){ };

	  } while (!last_remain);


#else
	  uint8_t ch, cl; int i;
	for (i = 0; i < (width * height) - 1; i++)
	{
		ch = (color & 0xFF00) >> 8;
		cl = (color & 0xFF);
		ILI9341_writedata(ch);
		ILI9341_writedata(cl);
	}
#endif
	return HAL_OK;
}

/**
  * @brief  HAL_StatusTypeDef ILI9341_fillrectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
  * @params
  * @retval None
  */

HAL_StatusTypeDef ILI9341_fillrectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{	HAL_StatusTypeDef result = HAL_OK;
	ILI9341_setaddr(x, y, x + width - 1, y + height - 1);
	ILI9341_writecmd(ILI9341_GRAM);

#if defined (ILI9341_DMA)


	/*
	 * Make the filled buffer for to fill the color bytes.
	 */

	uint8_t* fillbuffer; int i, j; int32_t DT, remain; int last_remain; uint8_t* cc = (uint8_t*)&color;
	if ((fillbuffer = malloc(FILLRECT_BUFFER_SIZE)) == NULL)
		{
			ForceErrorNumber(5);
			return HAL_ERROR;
		}

	/*
	 * Load the fill buffer for color datas.
	 */
	for (i = 0; i < FILLRECT_BUFFER_SIZE; i += BYTE_PER_PIXEL)
	{
		for (j = 0; j < BYTE_PER_PIXEL; j++)
		{
			fillbuffer[j + i] = cc[(BYTE_PER_PIXEL - 1) - j];
		}
	}

	  SELECT_DATA();
	  remain = (width * height * BYTE_PER_PIXEL);
	  do
	  {
	  	if ((remain - FILLRECT_BUFFER_SIZE) > 0)
	  	{
	  		last_remain = 0;
	  		DT = FILLRECT_BUFFER_SIZE;
	  		remain -= FILLRECT_BUFFER_SIZE;
	  	} else
	  	{
	  		last_remain = 1;
	  		DT = remain;
	  	}

	  	  wTransferState = TRANSFER_WAIT;
	  	  if (HAL_SPI_Transmit_DMA(&display_spi1_handle, fillbuffer, DT) != HAL_OK)
	  	    {
	  	  	  result = HAL_ERROR;
	  	    }
	  	  while (wTransferState != TRANSFER_COMPLETE){ };

	  } while (!last_remain);



#else
	  uint8_t ch, cl; int i;
	for (i = 0; i < (width * height) - 1; i++)
	{
		ch = (color & 0xFF00) >> 8;
		cl = (color & 0xFF);
		ILI9341_writedata(ch);
		ILI9341_writedata(cl);
	}
#endif

	free(fillbuffer);
	return result;

}

void ILI9341_draw(uint8_t *buff)
{
//	ili9341_setaddr(0, 0, 153, 144);//11088 for 153*144
//	HAL_SPI_Transmit_DMA(&display_spi1_handle, buff, 44352);
}

#if defined (ILI9341_DMA)
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
	wTransferState = TRANSFER_COMPLETE;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi)
{

}

/*
 * This is an interrupt handle for DMA Display SPI tx channel.
 */

void DMA1_Channel3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(display_spi1_handle.hdmatx);
}
#endif


