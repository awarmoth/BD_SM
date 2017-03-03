#include <stdint.h>
#include <stdbool.h>
#include "termio.h"

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"

#include "BITDEFS.H"
#include <Bin_Const.h>

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif

// Standard Configuration for timer
#define GenA_Normal (PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO )
#define GenA_Invert (PWM_0_GENA_ACTCMPAU_ZERO | PWM_0_GENA_ACTCMPAD_ONE )
#define GenB_Normal (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO )
#define GenB_Invert (PWM_0_GENB_ACTCMPBU_ZERO | PWM_0_GENB_ACTCMPBD_ONE )
// System clock frequency
#define TicksPerMS 40000
// Clock scaled to avoid overflow
#define PWMTicksPerMS TicksPerMS/32
// constant for mapping PWM to pins
#define BitsPerNibble 4
// Frequency for PWM signal (1/Frequency)
#define FrequencyInHz 2000
#define ServoFreqHz 50
// Max number of ticks for PWM
#define PWM_LOAD_VALUE PWMTicksPerMS*1000/FrequencyInHz
#define SERVO_PWM_LOAD_VALUE PWMTicksPerMS*1000/ServoFreqHz
// Forward/backward directions for motors
#define FORWARD 0
#define REVERSE 1

// flag for tracking whether returning to normal PWM from 0 or 100 duty cycle
static bool restoreA;
static bool restoreB;
static bool restoreFW;
//int for tracking motor direction
//static int directionA;
//static int directionB;
static uint8_t LastDirA=1;
static uint8_t LastDirB=1;


void InitPWM(void) {
// Enable the clock to the PWM Module
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
	
	// Enable the clock to Port B
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (GPIO_PIN_0 | GPIO_PIN_1);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (GPIO_PIN_0 | GPIO_PIN_1);
	
	// Select the system clock/32
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) | (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
	
	// Make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
		;
	
	// Disable the PWM generator while initializing
	HWREG(PWM0_BASE+PWM_O_0_CTL) = 0;
	
	// Set generator to go up to 1 at rising A, 0 on falling A
	HWREG(PWM0_BASE+PWM_O_0_GENA) = GenA_Normal;
	
	// Set generator to go to 1 at rising B, 0 on falling B
	HWREG(PWM0_BASE+PWM_O_0_GENB) = GenB_Normal;
	
	// Set the load to ½ the desired period of 5 ms since going up and down
	HWREG(PWM0_BASE+PWM_O_0_LOAD) = ((PWM_LOAD_VALUE)) >> 1;
	
	// Set the initial duty cycle on A to 50%
	HWREG(PWM0_BASE+PWM_O_0_CMPA) = HWREG(PWM0_BASE+PWM_O_0_LOAD)>>1;
	
		// Set the initial duty cycle on B to 50%
	HWREG(PWM0_BASE+PWM_O_0_CMPB) = HWREG(PWM0_BASE+PWM_O_0_LOAD)>>1;
	
	// Enable the PWM outputs
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);
	
	// Select the alternate function for PB6 and PB7
	HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT7HI | BIT6HI);
	
	// Choose to map PWM to those pins
	HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0X00ffffff) + (4<<(7*BitsPerNibble)) + (4<<(6*BitsPerNibble)); 
	
	// Enable pins 6 and 7 on Port B for digital outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT7HI | BIT6HI);
	
	// Set the up/down count mode
	// Enable the PWM generator
	// Make generator updates locally synchronized to zero count
	HWREG(PWM0_BASE+PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE | PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);                 
}

void SetDutyA(uint8_t duty) {
	// New Value for comparator to set duty cycle
//	duty*=9/10;
	static uint32_t newCmp;
	if (LastDirA == REVERSE) duty = 100 - duty;
	//printf("dutyA:%i",duty);
	// set new comparator value based on duty cycle
	newCmp = HWREG(PWM0_BASE+PWM_O_0_LOAD)*(100-duty)/100;
	if (duty == 100 | duty == 0) {
		restoreA = true;
		if (duty == 100) {
			// To program 100% DC, simply set the action on Zero to set the output to one
			HWREG( PWM0_BASE+PWM_O_0_GENA) = PWM_0_GENA_ACTZERO_ONE;
		} else {
			// To program 0% DC, simply set the action on Zero to set the output to zero
			HWREG( PWM0_BASE+PWM_O_0_GENA) = PWM_0_GENA_ACTZERO_ZERO;
			}
	} else {
		// if returning from 0 or 100
		if (restoreA) {
			restoreA = false;
			// restore normal operation
			HWREG( PWM0_BASE+PWM_O_0_GENA) = GenA_Normal;
		}
		// write new comparator value to register
		HWREG( PWM0_BASE+PWM_O_0_CMPA) = newCmp;
	}
}

void SetDutyB(uint8_t duty) {
		// New Value for comparator to set duty cycle
	static uint32_t newCmp;
	if (LastDirB == REVERSE) duty = 100 - duty;
	// set new comparator value based on duty cycle
	newCmp = HWREG(PWM0_BASE+PWM_O_0_LOAD)*(100-duty)/100;
	if (duty == 100 | duty == 0) {
		restoreB = true;
		if (duty == 100) {
			// To program 100% DC, simply set the action on Zero to set the output to one
			HWREG( PWM0_BASE+PWM_O_0_GENB) = PWM_0_GENB_ACTZERO_ONE;
		} else {
			// To program 0% DC, simply set the action on Zero to set the output to zero
			HWREG( PWM0_BASE+PWM_O_0_GENB) = PWM_0_GENB_ACTZERO_ZERO;
		}
	} else {
		// if returning from 0 or 100
		if (restoreB) {
			restoreB = false;
			// restore normal operation
			HWREG( PWM0_BASE+PWM_O_0_GENB) = GenB_Normal;
		}
		// write new comparator value to register
		HWREG( PWM0_BASE+PWM_O_0_CMPB) = newCmp;
	}
}

void InitFlywheelPWM(void)
{
// Enable the clock to the PWM Module
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
	
	// Enable the clock to Port D
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	
	// Select the system clock/32
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) | (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
	
	// Make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
		;
	
	// Disable the PWM generator while initializing
	// We are using PWM generator 3
	HWREG(PWM0_BASE+PWM_O_3_CTL) = 0;
	
	restoreFW = true;
	//restoreB = true;
	
	
	// Set generator to go to 1 at rising B, 0 on falling 
	HWREG(PWM0_BASE+PWM_O_3_GENB) = (PWM_3_GENB_ACTCMPBU_ONE | PWM_3_GENB_ACTCMPBD_ZERO );
	
	// Set the load to ½ the desired period of 5 ms since going up and down
	HWREG(PWM0_BASE+PWM_O_3_LOAD) = ((PWM_LOAD_VALUE)) >> 1;
	
		// Set the initial duty cycle on B to 50%
	HWREG(PWM0_BASE+PWM_O_3_CMPB) = HWREG(PWM0_BASE+PWM_O_3_LOAD)>>1;
	
	// Enable the PWM outputs
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM7EN);
	
	// Select the alternate function for PD1
	HWREG(GPIO_PORTD_BASE+GPIO_O_AFSEL) |= (BIT1HI);
	
	// Choose to map PWM to those pins
	HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) & 0Xffffff0f) + (4<<(1*BitsPerNibble)); 
	
	// Enable pin1 on Port D as digital output
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= (BIT1HI);
	
	// Set the up/down count mode
	// Enable the PWM generator
	// Make generator updates locally synchronized to zero count
	HWREG(PWM0_BASE+PWM_O_3_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE | PWM_0_CTL_GENBUPD_LS);

}


void SetFlywheelDuty(uint8_t duty) {
		// New Value for comparator to set duty cycle
	static uint32_t newCmp;

	newCmp = HWREG(PWM0_BASE+PWM_O_3_LOAD)*(100-duty)/100;

	if (duty == 100 | duty == 0) {
		restoreFW = true;
		if (duty == 100) {
			// To program 100% DC, simply set the action on Zero to set the output to one
			HWREG( PWM0_BASE+PWM_O_3_GENB) = PWM_3_GENB_ACTZERO_ONE;
		} else {
			// To program 0% DC, simply set the action on Zero to set the output to zero
			HWREG( PWM0_BASE+PWM_O_3_GENB) = PWM_3_GENB_ACTZERO_ZERO;
		}
	} else {
		// if returning from 0 or 100
		if (restoreFW) {
			restoreFW = false;
			// restore normal operation
			HWREG( PWM0_BASE+PWM_O_3_GENB) = (PWM_3_GENB_ACTCMPBU_ONE | PWM_3_GENB_ACTCMPBD_ZERO );
		}
		// write new comparator value to register
		HWREG( PWM0_BASE+PWM_O_3_CMPB) = newCmp;
	}
}

void SetDirectionA(uint8_t dir) {
	if (dir==REVERSE) {
		HWREG(PWM0_BASE+PWM_O_0_GENA) = GenA_Invert;
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= (GPIO_PIN_0);
	} 
	else if (dir==FORWARD) {
		HWREG(PWM0_BASE+PWM_O_0_GENA) = GenA_Normal;
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= ~(GPIO_PIN_0);
	}
	LastDirA=dir;
}

void SetDirectionB(uint8_t dir) {
	if (dir==REVERSE) {
		HWREG(PWM0_BASE+PWM_O_0_GENB) = GenB_Invert;
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= (GPIO_PIN_1);
	} 
	else if (dir==FORWARD) {
		HWREG(PWM0_BASE+PWM_O_0_GENB) = GenB_Normal;
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= ~(GPIO_PIN_1);
	}
	LastDirB=dir;
}


void InitServoPWM(void)
{
// Enable the clock to the PWM Module
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
	
	// Enable the clock to Port B
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	
	// Select the system clock/32
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) | (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
	
	// Make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
		;
	
	// Disable the PWM generator while initializing
	// We are using PWM generator 1
	HWREG(PWM0_BASE+PWM_O_1_CTL) = 0;
	

	
	// Set generator to go to 1 at rising A, 0 on falling 
	HWREG(PWM0_BASE+PWM_O_1_GENA) = (PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ZERO);
	
	// Set the load to ½ the desired period of 20ms since going up and down
	HWREG(PWM0_BASE+PWM_O_1_LOAD) = ((SERVO_PWM_LOAD_VALUE)) >> 1;
	
	//WHAT DO I INITIALIZE IT TO?
		// Set the initial duty cycle on WT1A to 5%
	HWREG(PWM0_BASE+PWM_O_1_CMPA) = (HWREG(PWM0_BASE+PWM_O_1_LOAD))*(5/100);
	
	// Enable the PWM outputs
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM2EN);
	
	// Select the alternate function for PB4
	HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT4HI);
	
	// Choose to map PWM to those pins
	HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0Xfff0ffff) + (4<<(4*BitsPerNibble)); 
	
	// Enable pin4 on Port B as digital output
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT4HI);
	
	// Set the up/down count mode
	// Enable the PWM generator
	// Make generator updates locally synchronized to zero count
	HWREG(PWM0_BASE+PWM_O_1_CTL) = (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE | PWM_1_CTL_GENAUPD_LS);

}


void SetServoPosition(uint16_t position) {
		// New Value for comparator to set duty cycle
	static uint32_t newCmp;	
	
	if((position > 625) || (position < 0))
	{
		printf("Invalid servo position. No new comparator value set."\r\n);
	}
	else
	{
		newCmp = HWREG(PWM0_BASE+PWM_O_1_LOAD)*(11875-position)/12500;
		// write new comparator value to register
		HWREG(PWM0_BASE+PWM_O_1_CMPA) = newCmp;
	}
	
	/*
	if (duty == 100 | duty == 0) {
		restoreFW = true;
		if (duty == 100) {
			// To program 100% DC, simply set the action on Zero to set the output to one
			HWREG( PWM0_BASE+PWM_O_3_GENB) = PWM_3_GENB_ACTZERO_ONE;
		} else {
			// To program 0% DC, simply set the action on Zero to set the output to zero
			HWREG( PWM0_BASE+PWM_O_3_GENB) = PWM_3_GENB_ACTZERO_ZERO;
		}
	} else {
		// if returning from 0 or 100
		if (restoreFW) {
			restoreFW = false;
			// restore normal operation
			HWREG( PWM0_BASE+PWM_O_3_GENB) = (PWM_3_GENB_ACTCMPBU_ONE | PWM_3_GENB_ACTCMPBD_ZERO );
		}
		// write new comparator value to register
		HWREG( PWM0_BASE+PWM_O_3_CMPB) = newCmp;
	}
	*/
}

