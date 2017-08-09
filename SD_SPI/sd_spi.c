

#include <stdint.h>
#include <string.h>
#include "spi.h"
#include "sd_spi.h"
#include "stm32f1xx_hal.h"

/* CRC table for CRC7 procedure 
 * Thanks for https://www.pololu.com/docs/0J1?section=5.f
 * */
#if defined (SD_CRC7)
	uint8_t CRCPoly = 0x89;  // the value of our CRC-7 polynomial
	uint8_t CRCTable[256];
#endif

	/* Current SD type. If ver_none, the SD card not initialized yet, or unsuccessfully. */
SD_TYPE sd_type = VER_NONE;

/*	
 * @brief SD card SPI handle. SPI2 channel.Load datas for SD card initialize 
 * process.
 */

static SPI_HandleTypeDef 	sd_spi2_handle = 
{.Instance = SD_SPI_CHANNEL,
.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_256,
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


/* @brief SD_Card_SPI_Select(): Select SD card SPI communication. To use must 
 * be initialize SPI with <400kHz baud rate.
 * Power ON or card insertion
After supply voltage reached 2.2 volts, wait for one millisecond at least. 
 * Set SPI clock rate between 100 kHz and 400 kHz. Set DI and CS high and apply 74 or 
 * more clock pulses to SCLK. The card will enter its native operating mode and go 
 * ready to accept native command. (http://elm-chan.org/docs/mmc/mmc_e.html)
*/

void SD_Card_SPI_Select()
{	uint16_t counter;
	/* deselect chip select line */
	DESELECT_SD();
	/* min. 74 clock bits write */
  for (counter = 0; counter <= 9; counter++)
  {
    SPI_WriteByte(SD_DUMMY_BYTE, sd_spi2_handle, 1000);
  }
}

/* @brief SD_SPI2_Init(): initialize SPI 2 signals for SD card reader, 
 * and set SPI channel, and initialize CS line for standard I/O. */

void SD_SPI2_Init()
{
	/* Generate the CRC7 checksum table. */
#if defined (SD_CRC7)
	GenerateCRCTable(); 
#endif
	
  GPIO_InitTypeDef  gpioinitstruct = {0};

		/* Enable SPI2, and signal ports clock. */
	__HAL_RCC_SPI2_CLK_ENABLE();
	SPI2_SIGNAL_PORT_CLK_ENABLE();
	SD_CARD_CS_PORT_CLK_ENABLE();

  /* configure SPI2 SCK (GPIOB 13), and SPI2 MOSI (GPIOB 15)*/
	 
  gpioinitstruct.Pin        = SPI2_CLK_PIN | SPI2_MOSI_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
  gpioinitstruct.Pull       = GPIO_NOPULL;
  gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI2_SIGNAL_PORT, &gpioinitstruct);

	/* configure SD chip select line (GPIOB 11)*/

  gpioinitstruct.Pin        = SD_CARD_CS_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull       = GPIO_NOPULL;
  gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SD_CARD_CS_PORT, &gpioinitstruct);

  /* Configure SPI MISO to input GPIOB 14 . 
	 * Note! The line must be configuring pull up mode, or pulling up physically 
	 * with resistor , because if there is not SD card, float the input line, 
	 * might cause disturbance input datas.   */
	 
  gpioinitstruct.Pin        = SPI2_MISO_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_AF_INPUT;
  gpioinitstruct.Pull       = GPIO_PULLUP;
  gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI2_SIGNAL_PORT, &gpioinitstruct);

	SPI_Init(&sd_spi2_handle);

} 

void SetFastSPI()
{
	sd_spi2_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	SPI_Init(&sd_spi2_handle);
}

/*
 * SD_SPI_R1_Error(s_r1 R1) Callback if the SD card response in the R1 byte the several errors.
 */
void SD_SPI_R1_Error(s_r1 R1)
{

}

/*
 * void SD_SPI_Timeout_Error() Callback of the case of SD card timeout communication error.
 */

void SD_SPI_Timeout_Error()
{

}

/* SD_SPI_STATE SD_SPI_WaitValidResponse(s_r1* R1, SPI_HandleTypeDef handle, uint32_t TimeOut)
 * @brief Wait for valid response byte, or over time. Valid response byte with
 * most significant bit is 0. Return
 * SD_SPI_OK = startbit OK, SPI OK.
 * SPI_ERROR = */

SD_SPI_STATE SD_SPI_WaitValidResponse(s_r1* R1, SPI_HandleTypeDef handle, uint32_t TimeOut)
{
  SELECT_SD();
  SPI_WriteByte(SD_DUMMY_BYTE, sd_spi2_handle, SD_SPI2_TIMEOUT);
  do {
      SPI_ReadByte((uint8_t*)R1, handle, 1000);
  } while ((R1->m_0) && (TimeOut--));
  DESELECT_SD();
  /*
   * Timeout_Error() if SD card was'nt response after TimeOut cycles.
   */
  if (!TimeOut)
  {
	  SD_SPI_Timeout_Error();
	  return SD_TIMEOUT;
  } else
  {
	  /* Indicate the last R1 byte to further process. Start bit was OK, but the R1 may contain further error
	   * informations. */
	  SD_SPI_R1_Error(*R1);

	  /*
	   * There are several types of response tokens. As in SD mode, all are transmitted MSB first.
	   * Multiple bytes responses are defined in SPI mode but the card outputs only first byte (equivalent to R1)
	   * when Illegal Command Error or Command CRC Error is indicated in it. In this case, host never reads as
	   * the multiple bytes of response.
	   */

	  if ((!R1->ill_comm_err) && (!R1->com_crc_err))
	  {
		  return SD_SPI_OK;
	  } else return SD_ERROR;
  }
}

/* void SD_SPI_ReadLongResponse(s_args resp, SPI_HandleTypeDef handle, uint32_t TimeOut)
 * @brief Read the 32 bits command arguments from SD SPI */


void SD_SPI_ReadLongResponse(s_args* resp, SPI_HandleTypeDef handle, uint32_t TimeOut)
{
  SELECT_SD();
  SPI_ReadBuf((uint8_t*)resp, sizeof(resp), handle, TimeOut);
  DESELECT_SD();
}

/* @brief SendSDCommand(uint8_t index, uint32_t args) 
 * @params: index command index, args: argumentums 
 * Simple command procedure. */

void SendSDCommand(uint16_t index, s_args args)
{ s_command command;

  command.START_BIT = 0;
  command.TRANS_BIT = 1;
  command.END_BIT = 1;
  command.INDEX = index;
  command.args.argw = args.argw;
#if defined (SD_CRC7)
  command.CRC_VAL = getCRCVal(command.DTS, 5);
//  command.CRC_VAL = (0X4A);
#else
  if (index == 0) command.CRC_VAL = (0X4A);
  if (index == 8) command.CRC_VAL = (0X43);
#endif
  SELECT_SD();
  SPI_WriteByte(SD_DUMMY_BYTE, sd_spi2_handle, SD_SPI2_TIMEOUT);
  SPI_WriteBuf(&command, sizeof(command), sd_spi2_handle, SD_SPI2_TIMEOUT);
  DESELECT_SD();
}

/* @brief ResetCard() Reset card with GO_IDLE_STATE. It will enter SPI mode if the CS signal is asserted (negative)
	during the reception of the reset command (CMD0). */

SD_SPI_STATE ResetCard()
{ s_r1 r1; s_args args;
  args.argw = 0;
  SendSDCommand(GO_IDLE_STATE, args);
  return SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT);
}

/*  Part_1_Physical_Layer_Simplified_Specification_Ver6.00.pdf 
 * 	Figure 7-2 : SPI Mode Initialization Flow
 * */

SD_SPI_STATE SPIModeInitialize()
{
	s_command command; s_args args; s_r1 r1; s_args resp;
	e_resp_voltage resp_voltage; uint8_t cmd8_counter = CMD8_RETRY; uint16_t ACMD41_counter = ACMD41_RETRY;
	uint16_t CMD58_counter = ACMD41_RETRY;

	/* SEND_IF_COND (CMD8) is used to verify SD Memory Card interface operating condition. The argument
	 * format of CMD8 is the same as defined in SD mode and the response format of CMD8 is defined in
	 * Section 7.3.2.6. The card checks the validity of operating condition by analyzing the argument of CMD8
	 * and the host checks the validity by analyzing the response of CMD8. The supplied voltage is indicated
	 * by VHS field in the argument. The card assumes the voltage specified in VHS as the current supplied
	 * voltage. Only 1-bit of VHS shall be set to 1 at any given time. Check pattern is used for the host to check
	 * validity of communication between the host and the card.*/


	while (cmd8_counter)
	{
	args.argw = 0;
	args.VHS = VHS_27_36V;
	args.chk_pattern = PATT_8BIT;

	/* Try CMD 8 response SEND_IF_COND (CMD8) is used to verify SD Memory Card interface operating condition. The argument
	format of CMD8 is the same as defined in SD mode and the response format of CMD8 is defined in
	Section 7.3.2.6. The card checks the validity of operating condition by analyzing the argument of CMD8
	and the host checks the validity by analyzing the response of CMD8. The supplied voltage is indicated
	by VHS field in the argument. The card assumes the voltage specified in VHS as the current supplied
	voltage. Only 1-bit of VHS shall be set to 1 at any given time. Check pattern is used for the host to check
	validity of communication between the host and the card.
	If the card indicates an illegal command, the card is legacy and does not support CMD8. If the card
	supports CMD8 and can operate on the supplied voltage, the response echoes back the supply voltage
	and the check pattern that were set in the command argument.*/

	SendSDCommand(SEND_IF_COND, args);

	if (SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT) == SD_SPI_OK)
	{
		/*
		 * @TODO If the card indicates an illegal command, the card is legacy and does not support CMD8.
		 * Must be do something !!!!
		 */
		if (r1.ill_comm_err) { sd_type = VER1X; break;};
		SD_SPI_ReadLongResponse(&resp, sd_spi2_handle, SD_SPI2_TIMEOUT);
		if ((resp.chk_pattern == PATT_8BIT)) //
		{
			/* If the card supports CMD8 and can operate on the supplied voltage, the response echoes back the supply
			 * voltage and the check pattern that were set in the command argument. */

			/*  Check pattern OK, and voltage OK. */
			resp_voltage = (resp.VHS == VHS_27_36V) ? RESP_VOLTAGE : NO_RESP_VOLTAGE;
			sd_type = VER2SD;
			break;	// end trying cmd8 command.
		} else
		{
			/* If check pattern is not matched, CMD8 communication is not valid. In this case, it is recommended to
			 * retry CMD8 sequence. */
			if (!--cmd8_counter) return SD_ERROR;
		}
	} else
	{
		if (!--cmd8_counter) return SD_ERROR;
	}
	}; /* End while (cmd8_counter). */

	/*
	 * The CMD8 CRC verification is always enabled. The Host shall set correct CRC in the argument of CMD8.
	 * If CRC error is detected, card returns CRC error in R1 response regardless of command index.
	 */

#if defined (SD_CRC7)
	if (CRCSet(0) != SD_SPI_OK) return SD_ERROR;
#else
	if (CRCSet(0) != SD_SPI_OK) return SD_ERROR;
#endif



	/* READ_OCR (CMD58) is designed to provide SD Memory Card hosts with a mechanism to identify cards that do not match
	 * the VDD range desired by the host. If the host does not accept voltage range, it shall not proceed with further
	 * initialization sequence. The levels in the OCR register shall be defined accordingly (See Section 5.1).
	 */
	
	args.argw = 0;

	SendSDCommand(READ_OCR, args);

	if (SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT) == SD_SPI_OK)
	{
		if (r1.b == SD_IN_IDLE)
		{
		  SD_SPI_ReadLongResponse(&resp, sd_spi2_handle, SD_SPI2_TIMEOUT);
		};
	};

	/* The "in idle state" bit in the R1 response of ACMD41
	is used by the card to inform the host if initialization of ACMD41 is completed. Setting this bit to "1"
	indicates that the card is still initializing. Setting this bit to "0" indicates completion of initialization.*/

	r1.idle_state = 1;

	args.argw = 0;

    while (!(r1.b == SD_IN_DUTY) && (ACMD41_counter--))
    {
    	/* Flag the next command as an application-specific command */
    	SendSDCommand(APP_CMD, args);
    	if (SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT) == SD_SPI_OK)
    	{
        	/* Tell the card to send its OCR */
    		args.argw = 0;
    		if (sd_type != VER1X)
    		{
    			args.HCS = 1;
    		};
        	SendSDCommand(APP_SEND_OP_COND, args);
        	if (!(SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT) == SD_SPI_OK))
        	{

        	}
    	};
    }

    if (!ACMD41_counter) return SD_ERROR;

    /* Initialization completed. The card is not in idle state.
     * After initialization is completed, the host should get CCS information in the response of CMD58. CCS is
     * valid when the card accepted CMD8 and after the completion of initialization. CCS=0 means that the card
     * is SDSD. CCS=1 means that the card is SDHC or SDXC.
     * */

	r1.idle_state = 1;

	args.argw = 0;

	SendSDCommand(READ_OCR, args);

	if (SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT) == SD_SPI_OK)
	{
		if (r1.b == SD_IN_DUTY)
		{
			SD_SPI_ReadLongResponse(&resp, sd_spi2_handle, SD_SPI2_TIMEOUT);
			if (resp.CCS) sd_type = VER2HCSD;
	/* Bit 31 - Card power up status bit, this status bit is set if the card power up procedure has been finished.
	Bit 30 - Card Capacity Status bit, 0 indicates that the card is SDSC. 1 indicates that the card is SDHC or
	SDXC. The Card Capacity Status bit is valid after the card power up procedure is completed and the card
	power up status bit is set to 1. The Host shall read this status bit to identify SDSC Card or SDHC/SDXC
	Card.*/

		};
	};

	return SD_SPI_OK;
}



/*
 * The host can turn the CRC option on and off using the CRC_ON_OFF command (CMD59). Host should
 * enable CRC verification before issuing ACMD41.
 */

SD_SPI_STATE CRCSet(uint8_t ONOFF)
{	s_args args; s_r1 r1; s_args resp;
	args.argw = 0;
	args.CRCBIT = ONOFF;
	SendSDCommand(CRC_ON_OFF, args);
	SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT);
	return SD_SPI_OK;
}

/*
 * Unlike the SD Memory Card protocol (where the register contents is sent as a command response),
 * reading the contents of the CSD and CID registers in SPI mode is a simple read-block transaction. The
 * card will respond with a standard response token (Refer to Figure 7-3) followed by a data block of 16
 * bytes suffixed with a 16-bit CRC.
 * The data timeout for the CSD command cannot be set to the cards TAAC since this value is stored in the
 * card's CSD. Therefore, the standard response timeout value (NCR) is used for read latency of the CSD
 * register.
 * ReadBlock(void* buffer, int size) read block data
 * @args: void* buffer read buffer, int size read buffer size in byte.
 */


SD_SPI_STATE ReadBlock(void* buffer, int size)
{	s_r1 r1; uint16_t CRCData, CRCVal;
if (SD_SPI_WaitValidResponse(&r1, sd_spi2_handle, SD_RESET_CARD_TIMEOUT) == SD_SPI_OK)
{
	  if (WaitForPattern(PATTERN_SBR, sd_spi2_handle, READ_PATTERN_TIMEOUT) == SD_SPI_OK)
	  {
		  SELECT_SD();
		  SPI_ReadBuf(buffer, size, sd_spi2_handle, SD_SPI2_TIMEOUT);
		  DESELECT_SD();
	  } else return SD_ERROR;
	  /*
	   * Last two bytes: 16 bit CRC
	   */
	  SELECT_SD();
	  SPI_ReadBuf((uint8_t*)&CRCData, 2, sd_spi2_handle, SD_SPI2_TIMEOUT);
	  DESELECT_SD();

#if defined CRC_SD_DATA
	  CRCVal = crc16(buffer, size);
	  if (CRCVal == CRCData)
	  {
		  return SD_SPI_OK;
	  } else return SD_DATA_CRC16_ERR;
#endif

	  return SD_SPI_OK;
//    SPI_ReadBuf(buffer, SDHX_BLOCSIZE, sd_spi2_handle, SD_READ_BLOCK_TIMEOUT);
} else return SD_ERROR;
};

SD_SPI_STATE ReadSpecRegs(void* buffer, int size)
{
	if (WaitForPattern(PATTERN_SBR, sd_spi2_handle, READ_PATTERN_TIMEOUT) == SD_SPI_OK)
	{
		SELECT_SD();
		SPI_ReadBuf((uint8_t*)buffer, size, sd_spi2_handle, SD_SPI2_TIMEOUT);
		DESELECT_SD();
		return SD_SPI_OK;
	};
	return SD_ERROR;
}
SD_SPI_STATE GetCIDRegister(char* cid_string)
{	CID_CSD_RESP resp; s_args args; SD_SPI_STATE retval;
	args.argw = 0;
	SendSDCommand(SEND_CID, args);
	if ((retval = ReadBlock(&resp, sizeof(resp))) == SD_SPI_OK)
	{
		strncpy(cid_string, resp.CID.NAME, 5);
		return SD_SPI_OK;
	} else return retval;
}

SD_SPI_STATE GetCSDRegister()
{	CID_CSD_RESP resp; s_args args; s_r1 r1; int i;
	args.argw = 0;
	SendSDCommand(SEND_CSD, args);
	return ReadBlock(&resp, sizeof(resp));
}

uint32_t GetBlockLength()
{
	if (sd_type != VER_NONE)
	{
		return 512;
	} else return 0;
}

void SetBlockLength(uint32_t new_blocklength)
{	s_args arg;
	if (sd_type == VER2HCSD)
	{
		arg.argw = new_blocklength;
	}else
	{

	}
}

SD_SPI_STATE WaitForPattern(uint8_t pattern, SPI_HandleTypeDef handle, uint32_t TimeOut)
{	uint8_t test_byte;
	SELECT_SD();
	do {
	    SPI_ReadByte((uint8_t*)&test_byte, handle, 1000);
	} while ((pattern != test_byte) && (TimeOut--));
	DESELECT_SD();
	if (!TimeOut) return SD_TIMEOUT;
	else return SD_SPI_OK;
}

/*
 * Read and write commands have data transfers associated with them. Data is being transmitted or
 * received via data tokens. All data bytes are transmitted MSB first. Data tokens are 4 to 515 bytes
 * long and have the following format: For Single Block Read, Single Block Write and Multiple Block Read:
 * First byte: Start Block
 * 7 6 5 4 3 2 1 0
 * 1 1 1 1 1 1 1 0
 * Bytes 2-513 (depends on the data block length): User data
 * Last two bytes: 16 bit CRC.
 */

SD_SPI_STATE ReadDataBlock(uint32_t block_address, uint8_t* buffer)
{
  s_r1 r1; uint32_t i; s_args arg;
  arg.argw = block_address; uint8_t inbyte;
  SendSDCommand(READ_SINGLE_BLOCK, arg);
  return ReadBlock(buffer, SDHX_BLOCSIZE);
}

/* ------------------- CRC16 procedures --------------------------*/
/* @TODO The table maybe to write the flash memory. Or maybe calculate on-line,
 * not with table... */
/* Based on https://www.pololu.com/docs/0J1?section=5.f */

#ifdef CRC_SD_DATA
#define POLY 0x8408
/*
//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.
*/
uint16_t crc16(uint8_t* data_p, int length)
{
      unsigned char i;
      unsigned int data;
      unsigned int crc = 0xffff;

      if (length == 0)
            return (~crc);

      do
      {
            for (i=0, data=(unsigned int)0xff & *data_p++;
                 i < 8;
                 i++, data >>= 1)
            {
                  if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY;
                  else  crc >>= 1;
            }
      } while (--length);

      crc = ~crc;
      data = crc;
      crc = (crc << 8) | (data >> 8 & 0xff);

      return (crc);
}
#endif

/* ------------------- CRC7 procedures --------------------------*/
/* @TODO The table maybe to write the flash memory. Or maybe calculate on-line, 
 * not with table... */
/* Based on https://www.pololu.com/docs/0J1?section=5.f */

#if defined (SD_CRC7)

void GenerateCRCTable()
{
	int i, j;
 
	// generate a table value for all 256 possible byte values
	for (i = 0; i < 256; i++)
	{
		CRCTable[i] = (i & 0x80) ? i ^ CRCPoly : i;
		for (j = 1; j < 8; j++)
			{
				CRCTable[i] <<= 1;
				if (CRCTable[i] & 0x80) CRCTable[i] ^= CRCPoly;
			}
	}
}

// adds a message byte to the current CRC-7 to get a the new CRC-7
uint8_t CRCAdd(uint8_t CRC_VAL, uint8_t message_byte)
{
	return CRCTable[(CRC_VAL << 1) ^ message_byte];
}

// returns the CRC-7 for a message of "length" bytes
uint8_t getCRCVal(uint8_t message[], int length)
{
	int i;
	uint8_t CRC_VAL = 0;
 
	for (i = 0; i < length; i++)
		CRC_VAL = CRCAdd(CRC_VAL, message[i]);
 
  return CRC_VAL;
}

#endif
