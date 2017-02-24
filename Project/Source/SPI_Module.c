/****************************************************************************
 Module
  SPI_Module.c

 Revision
   1.0.1

 Description
   This file initializes and controls the communication between the TIVA
	 and the command generator over the SPI protocol.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* 
*/
#include <stdint.h>
#include <stdbool.h>
#include "termio.h"

// Framework inclusions
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"


// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_ssi.h"
#include "inc/hw_nvic.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"

// Module inclusions
#include "BITDEFS.H"
#include <Bin_Const.h>
#include "LOC_HSM.h"


// Module defines
#define BITS_PER_NIBBLE 4
#define CLOCK_PRE_DIVISOR 50
#define SCR_POS 8
#define SERIAL_CLOCK_RATE 59
#define QUERY_INPUT (0xAA)


// Module variables
static uint8_t command;


/****************************************************************************
 Function
     InitTSPIComm

 Parameters
     None

 Returns
     Nothing

 Description
     Initializes the registers to set up SSI0 on the TIVA for SPI communication
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
void InitSPI_Comm(void)
{
	
	//Enable the clock to GPIO Port A
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	//Enable the clock to SSI module
	HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R0;

	//Wait for GPIO Port A to be ready
	while((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R0) != SYSCTL_RCGCGPIO_R0){};
	printf("\r\nIs anything happening");
	//Program PA2, PA3, PA4, PA5 to use the alternate function
	HWREG(GPIO_PORTA_BASE+GPIO_O_AFSEL) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);

	//Set mux position (2) in GPIOPCTL to select the SSI use of the pins
	HWREG(GPIO_PORTA_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTA_BASE+GPIO_O_PCTL) & 0xff0000ff) | (2<<(2*BITS_PER_NIBBLE))
		| (2<<(3*BITS_PER_NIBBLE)) | (2<<(4*BITS_PER_NIBBLE)) | (2<<(5*BITS_PER_NIBBLE));

	//PA2 = SSI0Clk
	//PA3 = SSI0Fss
	//PA4 = SSI0Rx
	//PA5 = SSI0Tx	
	//Set PA2. PA3, PA4, PA5 as digital
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);

	//Set PA2, PA3, PA5 as outputs
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (BIT2HI | BIT3HI | BIT5HI);

	//Set PA4 as an input
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) &= ~(BIT4HI);

	//Program the pull-up on the clock line (Set GPIOPUR HI for PA2))
	HWREG(GPIO_PORTA_BASE+GPIO_O_PUR) |= BIT2HI;
	printf("\r\nSSI0 is starting");
	//Wait for the SSI0 to be ready
	while((HWREG(SYSCTL_RCGCSSI) & SYSCTL_RCGCSSI_R0) != SYSCTL_RCGCSSI_R0);
	printf("\r\nSSI0 is ready");
	
	//Make sure that the SSI is disabled before programming mode bits
	HWREG(SSI0_BASE+SSI_O_CR1) &= ~(SSI_CR1_SSE);
	
	//Select master mode (MS) & TXRIS indicating End of Transmit (EOT)
	HWREG(SSI0_BASE+SSI_O_CR1) &= ~(SSI_CR1_MS);
	HWREG(SSI0_BASE+SSI_O_CR1) |= SSI_CR1_EOT;
	
	//Configure the SSI clock source to the system clock
	HWREG(SSI0_BASE+SSI_O_CC) = (~SSI_CC_CS_M & HWREG(SSI0_BASE+SSI_O_CC)) + SSI_CC_CS_SYSPLL;
	//Configure the clock pre-scaler to a value of 50 (CPSDVSR = 50)
	HWREG(SSI0_BASE+SSI_O_CPSR) = (HWREG(SSI0_BASE+SSI_O_CPSR) & ~SSI_CPSR_CPSDVSR_M) + CLOCK_PRE_DIVISOR;
	
	//Configure clock rate (SCR = 59), phase & polarity (SPH = 1, SPO = 1), mode (FRF = 0), 
	//data size (DSS = 8 bits, must be right justified?)
	HWREG(SSI0_BASE+SSI_O_CR0) = (HWREG(SSI0_BASE+SSI_O_CR0) & ~SSI_CR0_SCR_M) + (SERIAL_CLOCK_RATE<<SCR_POS); //Set SCR to 59
	HWREG(SSI0_BASE+SSI_O_CR0) |= (SSI_CR0_SPH | SSI_CR0_SPO); //Set SPH = 1, SPO = 1
	HWREG(SSI0_BASE+SSI_O_CR0) = (HWREG(SSI0_BASE+SSI_O_CR0) & ~(SSI_CR0_FRF_M)) + SSI_CR0_FRF_MOTO; //Set FRF = 0
	HWREG(SSI0_BASE+SSI_O_CR0) = (HWREG(SSI0_BASE+SSI_O_CR0) & ~(SSI_CR0_DSS_M)) + SSI_CR0_DSS_8; //Set data size = 8 bits
	
	//Locally enable interrupts (TXIM in SSIIM)
	HWREG(SSI0_BASE+SSI_O_IM) |= SSI_IM_TXIM; \
	
	//globally enable interrupts
	__enable_irq();
	
	//Make sure that the SSI is enabled for operation
	HWREG(SSI0_BASE+SSI_O_CR1) |= SSI_CR1_SSE;
}


/****************************************************************************
 Function
     EOT_Response_ISR

 Parameters
     None

 Returns
     Nothing

 Description
     Reads the command from the slave device stored in the data register of the master
		 when an end of transmission interrupt is triggered.
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
void EOT_Response_ISR(void)
{
	//printf("EOT INTERRUPT TRIGGERED\r\n");
	//disable the interrupt in the NVIC
	HWREG(NVIC_EN0) &= ~(BIT7HI);
	//read the value from the SSI Data Register
	command = HWREG(SSI0_BASE+SSI_O_DR);
	//printf("Command = %d\r\n",command);
	//Post an ES_EOT Event to the ByteTransferSM with the event parameter being the value read
	ES_Event ThisEvent;
	ThisEvent.EventType = ES_EOT;
	ThisEvent.EventParam = command;
	//Post to LOC_SM
	PostLOC_SM(ThisEvent);
	
}


/****************************************************************************
 Function
     QueryCommandGen

 Parameters
     None

 Returns
     Nothing

 Description
     Writes a query command to the slave to see what command the slave is storing
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
void QueryLOC(uint8_t QueryVal)
{
	//enable the interrupt in the NVIC
	
	//write QueryVal to the SSI Data Register
	HWREG(SSI0_BASE+SSI_O_DR) = (HWREG(SSI0_BASE+SSI_O_DR) & ~SSI_DR_DATA_M) + QueryVal;
	HWREG(NVIC_EN0) |= BIT7HI;
}


