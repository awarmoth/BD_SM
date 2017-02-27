/****************************************************************************
Module file for exectuing all hardware initialization

 ****************************************************************************/
 
 #include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

// Hardware definitions
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "constants.h"

#include "ADMulti.h"
#include "PWM_Module.h"
 
#define CONTROLLER_TIME_US 2000
#define TICKS_PER_US 40
 
static void Init_Controller(void);
static void AD_Init(void);
static void MagneticTimerInit(void);
static void OneShotTimerInit(void);

void InitializePins(void) {
	Init_Controller();
	AD_Init();
	InitPWM();
	MagneticTimerInit();
	OneShotTimerInit();
}

static void Init_Controller(void)
{
	//enable clock to the timer (use timer 1)
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
	while ((HWREG(SYSCTL_PRWTIMER)&SYSCTL_PRWTIMER_R1)!=SYSCTL_PRWTIMER_R1) {}
	//disable the timer (A)
	HWREG(WTIMER1_BASE+TIMER_O_CTL)&=~(TIMER_CTL_TAEN);
	//set 32 bit wide mode
	HWREG(WTIMER1_BASE+TIMER_O_CFG)=TIMER_CFG_16_BIT;
	//set up periodic mode
	HWREG(WTIMER1_BASE+TIMER_O_TAMR)=(HWREG(WTIMER1_BASE+TIMER_O_TAMR)&~TIMER_TAMR_TAMR_M)|TIMER_TAMR_TAMR_PERIOD;
	//set timeout to 2 ms
	HWREG(WTIMER1_BASE+TIMER_O_TAILR)=(uint32_t)CONTROLLER_TIME_US*TICKS_PER_US;
	//enable local interupt
	HWREG(WTIMER1_BASE+TIMER_O_IMR)|=(TIMER_IMR_TATOIM);
	//enable NVIC interupt
	HWREG(NVIC_EN3)|=(BIT0HI);
	//globally enable interrupts
	__enable_irq();
	//set priority to 6 (anything more important than the other ISRs)
	HWREG(NVIC_PRI24)=(HWREG(NVIC_PRI24)&~NVIC_PRI24_INTA_M)|(0x6<<NVIC_PRI24_INTA_S);
	//enable the timer
	HWREG(WTIMER1_BASE+TIMER_O_CTL)|=(TIMER_CTL_TAEN|TIMER_CTL_TASTALL);
	
}


static void AD_Init(void)
{
	//set up AD pin
	HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R4;
	while((HWREG(SYSCTL_PRGPIO)& SYSCTL_PRGPIO_R4)!=SYSCTL_PRGPIO_R4){}
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN)|=(GPIO_PIN_0|GPIO_PIN_1);
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR)&=(~GPIO_PIN_0|GPIO_PIN_1);
	ADC_MultiInit(2);
}

/****************************************************************************
 Function
    MagneticTimerInit

 Parameters
   None

 Returns
   None

 Description
   Function that sets up the encoder timer system
 Author
   Matthew Miller			1/19/17
****************************************************************************/
static void MagneticTimerInit(void)
{
	// Enable the clock to the timer
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R2;
	
	// Enable the clock to Port D
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	
	// Make sure the timer is disabled before configuring
	HWREG(WTIMER2_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	
	// Set up timer in 32 bit wide mode
	HWREG(WTIMER2_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	
	// Initialize the interval load register to max/default value
	HWREG(WTIMER2_BASE+TIMER_O_TAILR) = 0xffffffff;
	
	// Set up timer in capture mode
	// Set up timer for edge time
	// Set up timer for up-counting
	HWREG(WTIMER2_BASE+TIMER_O_TAMR) = (HWREG(WTIMER2_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);
	
	// Set the event to rising edge
	HWREG(WTIMER2_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;
	
	// Set up the alternate function for Port D0
	HWREG(GPIO_PORTD_BASE+GPIO_O_AFSEL) |= BIT0HI;
	
	// Map bit 0’s alternate function
	HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) & 0xfffffff0) + 7;
	
	// Enable pin on port D for digital I/O
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= BIT0HI;
	
	// Make pin 0 on port D into input
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= BIT0LO;
	
	// Enable local capture interrupt
	HWREG(WTIMER2_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;
	
	// Enable timer A in NVIC
	HWREG(NVIC_EN3) |= BIT2HI;
	
	// Enable interrupts globally
	__enable_irq();
	
	// Enable timer and enable to stall when stopped by debugger
	HWREG(WTIMER2_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
}

/****************************************************************************
 Function
    OneShotTimerInit

 Parameters
   None

 Returns
   None

 Description
   Function that sets up the one shot timer system to determine if the robot has passed a station
 Author
   Matthew Miller			1/19/17
****************************************************************************/
static void OneShotTimerInit(void)
{
	// Enable the clock to the timer
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R3;
	
	// Make sure the clock has gotten going
	while((HWREG(SYSCTL_PRWTIMER) & SYSCTL_PRWTIMER_R3) != SYSCTL_PRWTIMER_R3){
	}
	
	// Make sure the timer is disabled before configuring
	HWREG(WTIMER3_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	
	// Set up timer in 32 bit wide mode
	HWREG(WTIMER3_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	
	// Set up timer in one shot mode
	HWREG(WTIMER3_BASE+TIMER_O_TAMR) = (HWREG(WTIMER3_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAMR_M) | TIMER_TAMR_TAMR_1_SHOT;
	
	// Set the timeout
	HWREG(WTIMER3_BASE+TIMER_O_TAILR) = ONE_SHOT_TIMEOUT;
	
	// Enable a local timeout interrupt
	HWREG(WTIMER3_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
	
	// Enable timer A in NVIC
	HWREG(NVIC_EN3) |= BIT4HI;
	
	// Enable interrupts globally
	__enable_irq();
	
	// Enable timer and enable to stall when stopped by debugger
	HWREG(WTIMER3_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
}
