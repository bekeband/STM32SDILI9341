

#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define SPI1_W_DMA
#define SPI2_W_DMA

#if defined (SPI1_W_DMA) | defined (SPI2_W_DMA)

/*
 * SPI DMA transfer state enumeration.
 */

typedef enum {
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
} e_dma_transfer_state;

#endif



/* @brief SPI_Init(SPI channel) Initialize on the got parameter SPI channel */
void SPI_Init(SPI_HandleTypeDef* SPI_Chan);

/**
  * @brief SPI WriteByte a byte to device General SPI write byte procedure.
  * @param Value the byte to written, TimeOut the timeout value to write. 
  * @retval None
  */
	
void SPI_WriteByte(uint8_t Value, SPI_HandleTypeDef handle, uint32_t TimeOut);

/**
  * @brief SPI_ReadByte Read a byte from SPI port designated by handle.
  * @param Value the byte to read, TimeOut the timeout value to read. 
  * @retval None
  */

void SPI_ReadByte(uint8_t* Value, SPI_HandleTypeDef handle, uint32_t TimeOut);

/**
  * @brief SPI_WriteBuf Write buffer bytes to the selected SPI port.
  * @param Value the pointer for buffer, size: size of buffer in bytes 
	* TimeOut the timeout value to write. 
  * @retval None
  */

void SPI_WriteBuf(void* Value, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut);

/**
  * @brief SPI_ReadBuf Read size of byte to the Buffer from SPI port designated by handle.
  * @param Value the byte to read, TimeOut the timeout value to read. 
  * @retval None
  */
void SPI_ReadBuf(uint8_t* Buffer, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut);

/**
  * @brief HAL_StatusTypeDef SPI_WriteBufDMA Write buffer bytes to the selected SPI port with DMA
  * @param Value the pointer for buffer, size: size of buffer in bytes
	* TimeOut the timeout value to write.
  * @retval None
  */

HAL_StatusTypeDef SPI_WriteBufDMA(void* Value, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut);

/**
  * @brief HAL_StatusTypeDef SPI_ReadBufDMA Read size of byte to the Buffer from SPI port designated by handle with DMA
  * @param Value the byte to read, TimeOut the timeout value to read.
  * @retval None
  */
HAL_StatusTypeDef SPI_ReadBufDMA(uint8_t* Buffer, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut);

#endif
