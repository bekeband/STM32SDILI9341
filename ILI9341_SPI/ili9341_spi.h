/*
 * ili9341_spi.h
 *
 *  Created on: 2017. júl. 18.
 *      Author: bekeband
 *      ILI9341 programming on SPI way.
 *      https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf
 *
 *	Reset states.
 *      					After Powered ON 	After Hardware Reset 	After Software Reset
 *      Frame Memory 		Random 				Repair data 			No Change
 *      Sleep 				In 					In 						In
 *      Display Mode 		Normal 				Normal 					Normal
 *      Display 			Off 				Off 					Off
 *      Idle 				Off 				Off 					Off
 *      Column Start Address 0000 h 			0000 h 					0000 h
 *      Column End Address 	00EF h 				00EF h 					If MADCTL’s B5=0:00EF h If MADCTL’s B5=1:013F h
 *      Page Start Address 	0000 h 				0000 h 					0000 h
 *      Page End Address 	013F h 				013F h 					If MADCTL’s B5 = 0:013F h If MADCTL’s B5=1:00EF h
 *      Gamma Setting 		GC0 				GC0 					GC0
 *      Partial Area Start 	0000 h 				0000 h 					0000 h
 *      Partial Area End 	013F h 				013F h 					013F h
 *      Memory Data Access Control 00 h 		00 h 					No Change
 *      RDDPM 				08 h 				08 h 					08 h
 *      RDDMADCTL 			00 h 				00 h 					No Change
 *      RDDCOLMOD 			06 h 				06 h 					06 h
 *      RDDIM 				00 h 				00 h 					00 h
 *      RDDSM 				00 h 				00 h 					00 h
 *      RDDSDR 				00 h 				00 h 					00 h
 *      TE Output Line 		Off 				Off 					Off
 *      TE Line Mode 		Mode 1 (Note 3) 	Mode 1 (Note 3) 		Mode 1 (Note 3)
 */

#include <stdint.h>
#include "stm32f1xx_hal.h"


#ifndef ILI9341_SPI_ILI9341_SPI_H_
#define ILI9341_SPI_ILI9341_SPI_H_

/*
 * Definitions the ILI9341 Display controller I/O ports..
 *
 */

/* ----------------- Display SPI signal definitions --------------------------------*/

#define SPI1_SIGNAL_PORT	GPIOA

#define SPI1_SIGNAL_PORT_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define DISPLAY_SOGNAL_PORT_CLK_ENABLE()	__HAL_RCC_GPIOA_CLK_ENABLE()

#define SPI1_CLK_PIN	GPIO_PIN_5
#define SPI1_MOSI_PIN	GPIO_PIN_7
#define SPI1_MISO_PIN	GPIO_PIN_6

/*
 * We are going to connect all pins to the PORTA GPIO port.
 */

#define DISPLAY_CONTROL_PORT	GPIOA

/*
 * CSX I MCU (VDDI/VSS) Chip select input pin (“Low” enable).
 * This pin can be permanently fixed “Low” in MPU interface mode only.
 */

#define DISPLAY_CS_PIN	GPIO_PIN_4

/*
 * D/CX (SCL) I MCU (VDDI/VSS) This pin is used to select “Data or Command” in the parallel interface
 * or 4-wire 8-bit serial data interface. When DCX = ’1’, data is selected. When DCX = ’0’, command is selected.
 * This pin is used serial interface clock in 3-wire 9-bit / 4-wire 8-bit serial data interface.
 * If not used, this pin should be connected to VDDI or VSS.
 */

#define DISPLAY_DC_PIN	GPIO_PIN_2
/*
 * RESX I MCU (VDDI/VSS)
 * This signal will reset the device and must be applied to properly initialize the chip. Signal is active low
 */

#define DISPLAY_RST_PIN	GPIO_PIN_3

/*
 * This is a output port which is connected the LCD backlight LED helping with a PNP transistor.
 * Since the transistor is PNP, the LCD will be light on the active low output level .
 */

#define DISPLAY_LCD_PIN	GPIO_PIN_1


#define SELECT_DISPLAY()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_CS_PIN, GPIO_PIN_RESET)
#define DESELECT_DISPLAY()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_CS_PIN, GPIO_PIN_RESET)

#define RESET_ACTIVE()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_RST_PIN, GPIO_PIN_RESET)
#define RESET_PASSIVE()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_RST_PIN, GPIO_PIN_SET)

#define SELECT_DATA()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_DC_PIN, GPIO_PIN_SET)
#define SELECT_COMMAND()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_DC_PIN, GPIO_PIN_RESET)

#define DISPLAY_ON()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_LCD_PIN, GPIO_PIN_RESET)
#define DISPLAY_OFF()	HAL_GPIO_WritePin(DISPLAY_CONTROL_PORT, DISPLAY_LCD_PIN, GPIO_PIN_SET)


#define DISPLAY_SPI_TRANSMIT_TIMEOUT 1000
/*
 * Define the display spi1 memory to periphera tx DMA channel.
 */
#define DISPLAY_TX_DMA_CHANNEL	DMA1_Channel3
#define DISPLAY_RX_DMA_CHANNEL	DMA1_Channel2

#define DISPLAY_DMA_TX_IRQn	DMA1_Channel3_IRQn
#define DISPLAY_DMA_RX_IRQn	DMA1_Channel2_IRQn


/*
 * defines ILI9341 commands ----------------------------------
 */

#define ILI9341_RESET				0x01
#define ILI9341_SLEEP_OUT			0x11
#define ILI9341_GAMMA				0x26
#define ILI9341_DISPLAY_OFF			0x28
#define ILI9341_DISPLAY_ON			0x29
#define ILI9341_COLUMN_ADDR			0x2A
#define ILI9341_PAGE_ADDR			0x2B
#define ILI9341_RAMWR				0x2C
#define ILI9341_RAMRD				0x2E
#define ILI9341_MAC					0x36
#define ILI9341_PIXEL_FORMAT		0x3A
#define ILI9341_WDB					0x51
#define ILI9341_WCD					0x53
#define ILI9341_RGB_INTERFACE		0xB0
#define ILI9341_FRC					0xB1
#define ILI9341_BPC					0xB5
#define ILI9341_DFC					0xB6
#define ILI9341_POWER1				0xC0
#define ILI9341_POWER2				0xC1
#define ILI9341_VCOM1				0xC5
#define ILI9341_VCOM2				0xC7
#define ILI9341_POWERA				0xCB
#define ILI9341_POWERB				0xCF
#define ILI9341_PGAMMA				0xE0
#define ILI9341_NGAMMA				0xE1
#define ILI9341_DTCA				0xE8
#define ILI9341_DTCB				0xEA
#define ILI9341_POWER_SEQ			0xED
#define ILI9341_3GAMMA_EN			0xF2
#define ILI9341_INTERFACE			0xF6
#define ILI9341_PRC					0xF7
/* read commands */
#define ILI9341_READDID4			0xD3

#define RGB_BGR_COLOR

#define PIXEL_FORMAT_18_BIT

#ifdef PIXEL_FORMAT_18_BIT
#define BYTE_PER_PIXEL	3
typedef uint8_t t_color[3];
#else
#define BYTE_PER_PIXEL	2
typedef uint8_t t_color[2];
#endif

//#define ILI9341_DMA

/*
 * screen fillerect, and imagerect (get/set) procedures buffer's size in pixel.
 */
#define SCR_BUFFER_IN_PIXELS	240
/*
 * The real fillrect buffer size.
 */
#define SCR_BUFFER_SIZE	(SCR_BUFFER_IN_PIXELS * BYTE_PER_PIXEL)


void SendData(uint16_t data);
void SendCommand(uint8_t data);
void SendSPIData(uint16_t data);
void DisplayOn(void);
void DisplayOff(void);

/* Size of buffer */
#define BUFFERSIZE                       (COUNTOF(aTxBuffer) - 1)

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

void DISPLAY_SPI1_Init();
void ILI9341_Init();
void DisplaySoftOn();
void DisplaySoftOff();

#define DP_DUMMY_BYTE	(0xFF)

typedef enum {
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
} e_dma_transfer_state;

typedef struct {
  uint16_t  	 width;
  uint8_t  	 height;
  uint8_t  	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  uint8_t 	pixel_data[160 * 100 * BYTE_PER_PIXEL];
} s_image;

HAL_StatusTypeDef ILI9341_fillrectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, t_color color);
HAL_StatusTypeDef ILI9341_displaybitmap(uint16_t x, uint16_t y, uint16_t widthi, uint16_t heighti, s_image* image);
HAL_StatusTypeDef ILI9341_getpixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t* pixels);
//HAL_StatusTypeDef ILI9341_getrectangle(uint16_t x, uint16_t y, uint16_t widthi, uint16_t heighti, uint8_t* image);

#endif /* ILI9341_SPI_ILI9341_SPI_H_ */
