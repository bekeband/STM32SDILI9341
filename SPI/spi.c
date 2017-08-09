

#include "spi.h"


/*
 * Global variable because may set this value of the transfer complete DMA interrupt routine.
 */

#if defined (SPI1_W_DMA)

e_dma_transfer_state SPI1_DMA_TransferState;

#endif

#if defined (SPI2_W_DMA)

e_dma_transfer_state SPI2_DMA_TransferState;

#endif

/* @brief Handle for SPI general error.  
 * @TODO What we are to do?   
 * */

void SPI_Error()
{
	
}

/* @brief SPI_Init(SPI channel) Initialize on the got parameter SPI channel */

void SPI_Init(SPI_HandleTypeDef* SPI_Chan)
{
	HAL_SPI_DeInit(SPI_Chan);
  if (HAL_SPI_Init(SPI_Chan) != HAL_OK)
  {
    /* Should not occur */
    while(1) {};
  }
}

/**
  * @brief SPI_WriteByte Write a byte to SPI handle port.
  * @param Value the byte to written, TimeOut the timeout value to write. 
  * @retval None
  */
void SPI_WriteByte(uint8_t Value, SPI_HandleTypeDef handle, uint32_t TimeOut)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&handle, (uint8_t*) &Value, 1, TimeOut);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPI_Error(handle);
  }
}

/**
  * @brief SPI_WriteBuf Write buffer bytes to the selected SPI port.
  * @param Value the pointer for buffer, size: size of buffer in bytes 
	* TimeOut the timeout value to write. 
  * @retval None
  */

void SPI_WriteBuf(void* Buffer, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut)
{
  HAL_StatusTypeDef status = HAL_OK;
  status = HAL_SPI_Transmit(&handle, (uint8_t*) Buffer, size, TimeOut);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPI_Error(handle);
  }
}

/**
  * @brief SPI_ReadBuf Read size of byte to the Buffer from SPI port designated by handle.
  * @param Value the byte to read, TimeOut the timeout value to read. 
  * @retval None
  */
void SPI_ReadBuf(uint8_t* Buffer, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut)
{
  HAL_StatusTypeDef status = HAL_OK;
  status = HAL_SPI_Receive(&handle, (uint8_t*) Buffer, size, TimeOut);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPI_Error(handle);
  }
}


/**
  * @brief SPI_ReadByte Read a byte from SPI port designated by handle.
  * @param Value the byte to read, TimeOut the timeout value to read. 
  * @retval None
  */
void SPI_ReadByte(uint8_t* Value, SPI_HandleTypeDef handle, uint32_t TimeOut)
{
  HAL_StatusTypeDef status = HAL_OK;
	
  status = HAL_SPI_Receive(&handle, (uint8_t*) Value, 1, TimeOut);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPI_Error(handle);
  }
}

e_dma_transfer_state* GetTransferStatePtr(SPI_HandleTypeDef* handle)
{
	if (handle->Instance == SPI1)
	{
		return &SPI1_DMA_TransferState;
	} else
	if (handle->Instance == SPI2)
	{
		return &SPI2_DMA_TransferState;
	} else
		return NULL;
}


/**
  * @brief SPI_WriteBufDMA Write buffer bytes to the selected SPI port with DMA
  * @param Value the pointer for buffer, size: size of buffer in bytes
	* TimeOut the timeout value to write.
  * @retval None
  */

HAL_StatusTypeDef SPI_WriteBufDMA(void* Buffer, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut)
{	e_dma_transfer_state* state;
	state = GetTransferStatePtr(&handle);
	*state = TRANSFER_WAIT;
	if (HAL_SPI_Transmit_DMA(&handle, Buffer, size) != HAL_OK)
	{
		return HAL_ERROR;
	} else
	{
	    /* Execute user timeout callback */
	    SPI_Error(handle);
	}
	while (*state != TRANSFER_COMPLETE){ };
	return HAL_OK;
}

/**
  * @brief SPI_ReadBufDMA Read size of byte to the Buffer from SPI port designated by handle with DMA.
  * @param Value the byte to read, TimeOut the timeout value to read.
  * @retval None
  */
HAL_StatusTypeDef SPI_ReadBufDMA(uint8_t* Buffer, uint16_t size, SPI_HandleTypeDef handle, uint32_t TimeOut)
{	e_dma_transfer_state* state;
	state = GetTransferStatePtr(&handle);
	*state = TRANSFER_WAIT;
	if (HAL_SPI_Receive_DMA(&handle, Buffer, size) != HAL_OK)
	{
		return HAL_ERROR;
	} else
	{
	    /* Execute user timeout callback */
	    SPI_Error(handle);
	}
	while (*state != TRANSFER_COMPLETE){ };
	return HAL_OK;
}

#if defined (SPI1_W_DMA)
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{	e_dma_transfer_state* state;
	state = GetTransferStatePtr(hspi);
	*state = TRANSFER_COMPLETE;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi)
{	e_dma_transfer_state* state;
	state = GetTransferStatePtr(hspi);
	*state = TRANSFER_COMPLETE;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi)
{

}

#endif

