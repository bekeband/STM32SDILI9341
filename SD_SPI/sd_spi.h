
#ifndef __SD_SPI_H
#define __SD_SPI_H

/*
 * Definitions the SD card SPI hardware installations.
 *
 */

/* ----------------- SD card I/O-s --------------------------------*/

#define SPI2_SIGNAL_PORT	GPIOB

#define SPI2_SIGNAL_PORT_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define SPI2_CLK_PIN	GPIO_PIN_13
#define SPI2_MOSI_PIN	GPIO_PIN_15
#define SPI2_MISO_PIN	GPIO_PIN_14

#define SD_CARD_CS_PORT	GPIOB
#define SD_CARD_CS_PORT_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define SD_CARD_CS_PIN	GPIO_PIN_11


#define SELECT_SD()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define DESELECT_SD()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)


/* Definitions and macros for SD card SPIcommunication.
 * SDCARD https://www.sdcard.org/downloads/pls/ 
 * Physical Layer Simplified Specification pdf */

/*7.3.1.3 Detailed Command Description 
 * The following table provides a detailed description of the SPI bus commands. 
 * The responses are defined in Section 7.3.2. Table 7-3 lists all SD Memory Card commands. 
 * A "yes" in the SPI mode column indicates that the command is supported in SPI mode. 
 * With these restrictions, the command class description in the CSD is still valid. 
 * If a command does not require an argument, the value of this field should be set to zero. 
 * The reserved commands are reserved in SD mode as well. The binary code of a command is 
 * defined by the mnemonic symbol. As an example, the content of the command index field 
 * is (binary) '000000' for CMD0 and '100111' for CMD39. The card shall ignore stuff bits 
 * and reserved bits in an argument.*/

#define GO_IDLE_STATE (0)	// 	[31:0] stuff bits R1	No	Resets the SD Memory Card

/* @brief SEND_OP_COND Sends host capacity support information and activates the card's 
 * initialization process. HCS is effective when card receives SEND_IF_COND command. Reserved 
 * bits shall be set to '0'. [31]Reserved bit [30]HCS [29:0]Reserved bits*/
#define SEND_OP_COND	(1)	//	 R1	

/* CMD8 9 Yes [31:12]Reserved SEND_IF_COND Sends SD Memory Card interface bits condition that 
 * includes host supply [11:8]supply voltage information and asks the voltage(VHS) accessed 
 * card whether card can [7:0]check pattern operate in supplied voltage range. Reserved bits 
 * shall be set to '0'. */
 

#define APP_SEND_OP_COND (41) // *2 		R1	No		For only SDC. Initiate initialization process.
#define SEND_IF_COND (8)	//CMD8	*3			R7	No		For only SDC V2. Check voltage range.
#define SEND_CSD	(9)	//		CMD9	None(0)	R1	Yes		Read CSD register.
#define SEND_CID	(10)//		CMD10	None(0)	R1	Yes		Read CID register.
#define STOP_TRANSMISSION	(12)	// CMD12	None(0)	R1b	No		Stop to read data.
#define SET_BLOCKLEN (16)	CMD16	// Blocklength[31:0]	R1	No		Change R/W block size.

#define READ_SINGLE_BLOCK (17)	//CMD17	Address[31:0]	R1	Yes		Read a block.
#define READ_MULTIPLE_BLOCK (18)//CMD18	Address[31:0]	R1	Yes		Read multiple blocks.

#define SET_BLOCK_COUNT (23)	/* CMD23	Number of blocks[15:0]	R1	No		For only MMC. Define number of blocks to transfer
						with next multi-block read/write command.*/
#define SET_WR_BLOCK_ERASE_COUNT (23) /* ACMD23(*1)	Number of blocks[22:0]	R1	No		For only SDC. Define number of blocks to pre-erase
						with next multi-block write command.*/
#define WRITE_BLOCK (24) // CMD24	Address[31:0]	R1	Yes		Write a block.
#define WRITE_MULTIPLE_BLOCK (25) // CMD25	Address[31:0]	R1	Yes		Write multiple blocks.
#define APP_CMD (55) // CMD55(*1)	None(0)	R1	No		Leading command of ACMD<n> command.
#define READ_OCR (58) // CMD58	None(0)	R3	No		Read OCR.

/*
 * CMD59 Yes [31:1] stuff bits [0:0] CRC option R1 CRC_ON_OFF Turns the CRC option on or off. A '1' in
 * the CRC option bit will turn the option on, a '0' will turn it off
 */
#define CRC_ON_OFF	(59)
  
#define SD_DUMMY_BYTE	(0xFF) 
//* SDCARD https://www.sdcard.org/downloads/pls/ 
// * Physical Layer Simplified Specification pdf
/* Retry of cmd 8 command in initialize process. */
#define CMD8_RETRY	3

#define ACMD41_RETRY	20

/* For Single Block Read, Single Block Write and Multiple Block Read:
 * First byte: Start Block
 */

#define PATTERN_SBR	(0b11111110)

/* @brief enumeration SD_SPI result states */

typedef enum {
	SD_SPI_OK = 0,
	SD_TIMEOUT 	= 1,	// SD card not response (no start bit) in the time timeout.
	SD_CMD_CRC7_ERR = 2,	// CMD SPI ERROR flag, if error check is enabled (SD_CRC7 defined)
	SD_DATA_CRC16_ERR = 3,	// SPI Data error flag, if data error is checked. (CRC_SD_DATA defined)
	SD_ERROR 	= 20		// SD card error indicated by the SD card. Further information on the LestError variable.
} SD_SPI_STATE;

#define SETBIT(DATA, BIT)	DATA |= BIT
#define RESBIT(DATA, BIT)	DATA &= ^BIT
#define GETBIT(DATA, BIT)	(DATA & BIT)

/* Union structs of command or response arguments (32 bits) */
typedef union {
  uint32_t argw;
  struct __attribute__ ((__packed__)){		// parameter of ACMD41 command.
	uint8_t resACMD4100: 6;
	uint8_t HCS: 1;		// HCS bit
	uint8_t res01: 1;
    uint32_t resACMD4101: 24;
  };
  struct __attribute__ ((__packed__)){		// response of CMD58 command (OCR register)
//D3
	uint8_t	S18A:	1;
	uint8_t	resOCR04: 4;
	uint8_t UHSIISTAT:	1;
	uint8_t CCS:		1;
	uint8_t BUSY:		1;
// D2
	uint8_t V2728:		1;
	uint8_t V2829:		1;
	uint8_t V2930:		1;
	uint8_t V3031:		1;
	uint8_t V3132:		1;
	uint8_t V3233:		1;
	uint8_t V3334:		1;
	uint8_t V3435:		1;
//D1
	uint8_t resOCR01: 	7;
	uint8_t V3536:		1;
//D0
	uint8_t resOCR00: 	8;
  };
  struct __attribute__ ((__packed__)){		// Response of CMD08 command
	uint8_t res000: 8;
	uint8_t res001: 8;
    uint8_t VHS: 	4;
    uint8_t res002:	4;
    uint8_t chk_pattern: 8;
  };
  struct __attribute__ ((__packed__)){		// Parameters of CRC command CMD59
	  uint8_t RESCMD5900[3];
	  uint8_t CRCBIT:	1;
	  uint8_t RESCMD5901: 7;
  };
  struct __attribute__ ((__packed__)){
    uint16_t LW;
    uint16_t HW;
  };
  struct __attribute__ ((__packed__)){
    uint8_t B0;
    uint8_t B1;
    uint8_t B2;
    uint8_t B3;
  };
  struct __attribute__ ((__packed__)){
    uint8_t DT[4];
  };
}s_args;

/*
 * R2 (CID, CSD register)
 * Code length is 136 bits. The contents of the CID register are sent as a response to the commands CMD2
 * and CMD10. The contents of the CSD register are sent as a response to CMD9. Only the bits [127...1] of
 * the CID and CSD are transferred, the reserved bit [0] of these registers is replaced by the end bit of the
 * response.
 * Bit position 135 134 [133:128] [127:1] 0
 * Width (bits) 1 1 6 127 1
 * Value '0' '0' '111111' x '1'
 * Description start bit transmission
 * bit reserved CID or CSD register incl. internal CRC7 end bit
 */

typedef union {
	struct __attribute__ ((__packed__)){
		uint8_t MID;

		uint16_t	OID;

		uint8_t 	NAME[5];

		uint8_t	PRV;

		uint32_t	PSN;

		uint16_t	MDT: 12;
		uint16_t	RES: 4;

	};
} CID_STRUCT;


#define CSD_STRUCT_VER10	(0)	// CSD Version 1.0 Standard Capacity

#define CSD_STRUCT_VER20	(1)	// CSD Version 2.0 High Capacity and Extended Capacity

/*
 * CSD Register (CSD Version 1.0)
 */

typedef union {
	struct __attribute__ ((__packed__)){
		uint8_t DT[15];
	};
	struct __attribute__ ((__packed__)){
// D14
	uint8_t D14:		6;	//reserved - 6 00 0000b R [125:120]
	uint8_t CSD_STRUCT:	2;	//	CSD structure CSD_STRUCTURE 2 00b R [127:126]
// D13
	uint8_t TAAC;			//data read access-time-1 TAAC 8 xxh R [119:112]
// D12
	uint8_t	NSAC;			//data read access-time-2 in CLK cycles (NSAC*100) NSAC 8 xxh R [111:104]
// D11
	uint8_t	TRAN_SPEED;		//max. data transfer rate TRAN_SPEED 8 32h or 5Ah R [103:96]
// D10, D09
	uint16_t	READ_BL_LEN: 4;	//max. read data block length READ_BL_LEN 4 xh R [83:80]
	uint16_t	CCC:	12;		// card command classes CCC 12 01x110110101b R [95:84]
// D08
	uint8_t		READ_BL_PARTIAL: 1;		// partial blocks for read allowed  1 1b R [79:79]
	uint8_t		WRITE_BLK_MISALIGN: 1;	// write block misalignment WRITE_BLK_MISALIGN 1 xb R [78:78]
	uint8_t		READ_BLK_MISALIGN: 1;	// read block misalignment READ_BLK_MISALIGN 1 xb R [77:77]
	uint8_t		DSR_IMP: 1;				//DSR implemented DSR_IMP 1 xb R [76:76]

	uint8_t		RES:	2;				//reserved - 2 00b R [75:74]

	uint16_t	C_SIZE: 12;				// device size C_SIZE 12 xxxh R [73:62]
//
	uint8_t		D05;
	uint8_t		D04;
	uint8_t		D03;
	uint8_t		D02;
	uint8_t		D01;
	uint8_t		D00;


	};
} CSD_STRUCT;

/*
 *
 */


typedef union {
  struct __attribute__ ((__packed__)){
	union {
	CID_STRUCT CID;
	CSD_STRUCT CSD;
  	};
//  uint8_t END_BIT: 1;
//  uint8_t CRC_VAL: 7;
  };
  struct __attribute__ ((__packed__)){
    uint8_t DTS[15];
  };
} CID_CSD_RESP;


/* 4.7.2 Command Format All commands have a fixed code length of 48 bits.
 * SDCARD https://www.sdcard.org/downloads/pls/ 
 * Physical Layer Simplified Specification pdf
*/

typedef union {
  struct __attribute__ ((__packed__)){
  uint8_t INDEX: 6;
  uint8_t TRANS_BIT: 1;
  uint8_t START_BIT: 1;
  s_args args;
  uint8_t END_BIT: 1;
  uint8_t CRC_VAL: 7;
  };
  struct __attribute__ ((__packed__)){
    uint8_t DTS[6];
  };
} s_command;

/* Typedef for R1 result byte from SD card to host. */

typedef union {
	struct __attribute__ ((__packed__)){
	uint8_t idle_state : 1; 	// in idle state
	uint8_t erase_res : 1; 	// erase reset
	uint8_t ill_comm_err : 1; 	// illegal command error
	uint8_t com_crc_err : 1; 	// com crc error
	uint8_t er_seq_err : 1; 	// erase sequence error
	uint8_t addr_err : 1; 	// address error
	uint8_t parm_err : 1;	// parameter error
	uint8_t m_0: 1;				// Mandatory 0 if response.
	};
	uint8_t b;		
} s_r1;

#define SD_SPI_CHANNEL	SPI2

/*
 * Define the sd card spi channel, and DMA channels for spi transmit, and receive.
 *
 */

#define SD_SPI_DMA


#if defined (SD_SPI_DMA)
/*
 * Channels for SPI dma rec/trans from ST32F10XX programming guide.
 */
#define SD_SPI2_TX_DMA_CHANNEL	DMA1_Channel5
#define SD_SPI2_RX_DMA_CHANNEL	DMA1_Channel4

#define SD_SPI2_DMA_TX_IRQn	DMA1_Channel5_IRQn
#define SD_SPI2_DMA_RX_IRQn	DMA1_Channel4_IRQn
#endif

#define SD_IN_IDLE	(0x01)	// SD card idle state, and no error anything else.
#define SD_IN_DUTY	(0x00)	// SD card no idle state, and no any communication error.

/* SD Memory card type */

typedef enum  {
	VER1X,			// Ver1.X Standard Capacity SD Memory Card
	VER2SD,			// Ver2.00 or later Standard Capacity SD Memory Card
	VER2HCSD,				// Ver2.00 Ver2.00 High Capacity or Extended Capacity SD Memory Card
	VER_NONE = 100
} SD_TYPE; 

#define VHS_27_36V	(0b0001)	// 2.7-3.6 V voltage rangefor sd card.

typedef enum {
	NO_RESP_VOLTAGE,
	RESP_VOLTAGE
} e_resp_voltage;

#define PATT_8BIT		(0xAA)

/* @brief Baud rate prescaler for configuration phase, To the SD card SPI mode
 * must be at least 74 bits times keep high the data line. The max. SPI speed of 
 * 400 kHz,and this prescaler applies this SPI speed. 
 * */
#define CONFIG_BAUD_PRESCALER	256

/* @brief Th prescaler determine the data stream of SPI2 SD card channel. 
 * */
#define DATA_BAUD_PRESCALER		4
 
/* Timeout for ad card spi write read datas. */ 
 
#define SD_SPI2_TIMEOUT		1000 
#define SD_IDLE_WAIT_MAX	1000

#define SDHX_BLOCSIZE	512

/*Timeout for reset card procedure. */

#define SD_RESET_CARD_TIMEOUT		1000

#define SD_READ_BLOCK_TIMEOUT	1000
#define READ_PATTERN_TIMEOUT	1000
 
 /* define SDCRC7 to calculate for crc7 value with SD card I/O procedures. */
 
#define SD_CRC7

/*
 * @TODO No CRC calculate routines !!!
 */
//#define CRC_SD_DATA
 
#if defined (SD_CRC7)
	void GenerateCRCTable(); 
	uint8_t getCRCVal(uint8_t message[], int length); 
#endif 
 
void SD_SPI2_Init(); 
void SD_Card_SPI_Select(); 

void SetFastSPI();

SD_SPI_STATE SPIModeInitialize();

/* @brief ResetCard() */
SD_SPI_STATE ResetCard();

SD_SPI_STATE ReadDataBlock(uint32_t blocknum, uint8_t* buffer);
/*
 * The host can turn the CRC option on and off using the CRC_ON_OFF command (CMD59). Host should
 * enable CRC verification before issuing ACMD41.
 */

SD_SPI_STATE CRCSet(uint8_t ONOFF);

SD_SPI_STATE GetCIDRegister();

SD_SPI_STATE GetCSDRegister();

SD_SPI_STATE WaitForPattern(uint8_t pattern, SPI_HandleTypeDef handle, uint32_t TimeOut);



uint16_t crc16(uint8_t* data_p, int length);

#endif

