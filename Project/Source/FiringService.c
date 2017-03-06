//module level variables: MyPriority, CurrentState
//FiringState_t: WaitingFire, SendingUp, SendingDown, Idling
//Module level functions: InitLoadServo, SendLoadServo
//Module defines: LATCH_SERVO, LATCH_DOWN, LATCH_DOWN_TIME, LATCH_UP, LATCH_UP_TIME, LATCH_SERVO_TIMER, PUSHER_SERVO, PUSHER_DOWN, PUSHER_DOWN_TIME, PUSHER_UP
//PUSHER_UP_TIME, PUSHER_SERVO_TIMER, LOAD_SERVO_UP, LOAD_SERVO_DOWN, LOAD_UP_TIME, LOAD_WAIT_TIME, LOAD_DOWN_TIME, LOAD_SERVO_TIMER

#include "MasterHSM.h"
#include "SPI_Module.h"
#include "ByteTransferSM.h"
#include "LOC_HSM.h"
#include "ConstructingSM.h"
#include "DrivingAlongTapeSM.h"
#include "hardware.h"
#include "FiringService.h"
#include "PWM10Tiva.h"
#include "PWM_Module.h"

#include "constants.h" 

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
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"


// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "BITDEFS.H"
#include <Bin_Const.h>

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif

// readability defines

#include "BITDEFS.H"

#define LOAD_SERVO_UP 300
#define LOAD_SERVO_DOWN 2000
#define LOAD_UP_TIME 500
#define LOAD_WAIT_TIME 1000
#define LOAD_DOWN_TIME 500


static uint8_t MyPriority;
static FiringState_t CurrentState;

static void InitLoadServo(void);
static void SendLoadServo(uint16_t position);


bool InitFiringService(uint8_t Priority)
{
	// Initialize MyPriority to Priority
	MyPriority = Priority;
	//Initialize CurrentState to WaitingFire
	CurrentState = WaitingFire;
	
	//Initialize the PWM mode for the servos
	InitLoadServo();
	SendLoadServo(LOAD_SERVO_DOWN);
	
	// Return true
	return true;
}



bool PostFiringService(ES_Event ThisEvent)
{
	// Return ThisEvent posted successfully to the service associated with MyPriority
	return ES_PostToService( MyPriority, ThisEvent);
}




ES_Event RunFiringService(ES_Event ThisEvent)
{
	// local variable NextState
	FiringState_t NextState;
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ES_NO_EVENT
	ReturnEvent.EventType = ES_NO_EVENT;
	
	// Initialize NextState to CurrentState
	NextState = CurrentState;
	
	switch(CurrentState)
	{
		// If CurrentState is WaitingFire
		case WaitingFire:
		{
			printf("FiringService: WaitingFire\r\n");
			// If ThisEvent is ES_FIRE
			if(ThisEvent.EventType == ES_FIRE)
			{
				// Send LOAD_SERVO to LOAD_SERVO_UP position
				SendLoadServo(LOAD_SERVO_UP);
				// Start LOAD_SERVO_TIMER for LOAD_UP_TIME
				ES_Timer_InitTimer(LOAD_SERVO_TIMER, LOAD_UP_TIME);
				// Set NextState to SendingUp
				NextState = SendingUp;
			}// EndIf
			break;
		} //End WaitingFire block
	
		// If CurrentState is SendingUp
		case SendingUp:
		{
			printf("FiringService: SendingUp\r\n");
			// If ThisEvent is ES_TIMEOUT
			if((ThisEvent.EventType == ES_TIMEOUT)&& (ThisEvent.EventParam == LOAD_SERVO_TIMER))
			{
				// Start LOAD_SERVO_TIMER for LOAD_WAIT_TIME
				ES_Timer_InitTimer(LOAD_SERVO_TIMER, LOAD_WAIT_TIME);
				// Set NextState to Idling
				NextState = Idling;
			}// EndIf
			break;
		}// End SendingUp block
		
		// If CurrentState is Idling
		case Idling:
		{
			printf("FiringService: Idling\r\n");
			// If ThisEvent is ES_TIMEOUT
			if((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == LOAD_SERVO_TIMER))
			{
				// Send LOAD_SERVO to LOAD_SERVO_DOWN
				SendLoadServo(LOAD_SERVO_DOWN);
				// Start LOAD_SERVO_TIMER for LOAD_DOWN_TIME
				ES_Timer_InitTimer(LOAD_SERVO_TIMER, LOAD_DOWN_TIME);
				// Set NextState to SendingDown
				NextState = SendingDown;
			}// EndIf
			break;
		}// End Idling block
		

		// If CurrentState is SendingDown
		case SendingDown:
		{
			printf("FiringService: SendingDown\r\n");
			// If This is ES_TIMEOUT
			if(ThisEvent.EventType == ES_TIMEOUT && (ThisEvent.EventParam == LOAD_SERVO_TIMER))
			{
				//Post ES_FIRE_COMPLETE to MasterHSM
				ES_Event NewEvent;
				NewEvent.EventType = ES_FIRE_COMPLETE;
				PostMasterSM(NewEvent);
				// Set NextState to WaitingFire
				NextState = WaitingFire;
			}// EndIf
			break;
		}// End SendingDown block

	} //end switch
	
	// Set CurrentState to NextState
	CurrentState = NextState;
	// Return ReturnEvent
	return ReturnEvent;
}

static void InitLoadServo(void)
{
	InitServoPWM();
}

static void SendLoadServo(uint16_t position)
{
	//PWM_TIVA_SetPulseWidth(position, TIMING_CHANNEL);
	SetServoPosition(position);
}

