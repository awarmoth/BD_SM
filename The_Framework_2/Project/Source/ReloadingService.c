//ReloadingService.c

#include "MasterHSM.h"
#include "SPI_Module.h"
#include "ByteTransferSM.h"
#include "LOC_HSM.h"
#include "ConstructingSM.h"
#include "DrivingAlongTapeSM.h"
#include "hardware.h"
#include "FiringService.h"
#include "ReloadingService.h"

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

//module level variables: MyPriority, CurrentState, PulseCounter
static uint8_t MyPriority;
static ReloadState_t CurrentState;
static uint8_t PulseCounter;
//ReloadState_t: WaitingReload, Emitting_High, Emitting_Low, Wait4Delivery

static void Enable_IR_Emitter (void);
static void Disable_IR_Emitter (void);
//Module level functions: Enable_IR_Emitter, Disable_IR_Emitter
//Module defines: PULSE_HIGH_TIME, PULSE_LOW_TIME, NUM_PULSES, DELIVERY_TIME

/****************************************************************************
 Function
     InitReloadingService

 Parameters
     uint8_t a priority number

 Returns
     None

 Description
     Performs the initialization sequence within the framework
 Notes

 Author
     Adam Warmoth
****************************************************************************/
bool InitReloadingService(uint8_t Priority)
{
//	Initialize MyPriority to Priority
	MyPriority = Priority;
//	Initialize CurrentState to WaitingReload
	CurrentState = WaitingReload;
//	Disable the IR Emitter
	Disable_IR_Emitter();
	//	Return true
	return true;
}
//End InitReloadingService


/****************************************************************************
 Function
     PostReloadingService

 Parameters
     ES_Event the event to post

 Returns
     bool if posted successfully

 Description
     Posts the event parameter to the associated event queue
 Notes

 Author
     Adam Warmoth
****************************************************************************/
bool PostReloadingService(ES_Event ThisEvent)
{
//	Return ThisEvent posted successfully to the service associated with MyPriority
	return ES_PostToService( MyPriority, ThisEvent);
}
//End PostReloadingService



/****************************************************************************
 Function
    RunFiringService

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   Runs the state machine that sends the IR pulses to communicate with
   the reload station
 Notes
   uses nested switch/case to implement the machine.
 Author
   Adam Warmoth
****************************************************************************/
ES_Event RunReloadingService(ES_Event ThisEvent)
{
//	local variable NextState
	ReloadState_t NextState;
//	local variable ReturnEvent
	ES_Event ReturnEvent;
//	Initialize ReturnEvent to ES_NO_EVENT
	ReturnEvent.EventType = ES_NO_EVENT;
//	Initialize NextState to CurrentState
	NextState = CurrentState;

	switch(CurrentState){
		case(WaitingReload):
			if (SM_TEST) printf("Reloading: WaitingReload");
			// If ThisEvent is ES_RELOAD_START
			if (ThisEvent.EventType == ES_RELOAD_START) {
				// Enable the IR Emitter
				Enable_IR_Emitter();
				// Set NextState to Emitting_High
				NextState = Emitting_High;
				// Set PulseCounter to zero
				PulseCounter = 0;
				// Start IR_PULSE_TIMER for PULSE_HIGH_TIME
				ES_Timer_InitTimer(IR_PULSE_TIMER, PULSE_HIGH_TIME);
			}
			// EndIf
			break;	
			// End WaitingReload block

		//	If CurrentState is Emitting_High
		case(Emitting_High):
			if (SM_TEST) printf("Reloading: Emitting_High");
			// If ThisEvent is ES_TIMEOUT
			if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == IR_PULSE_TIMER)){
				// Disable the IR Emitter
				Disable_IR_Emitter();
				// Set NextState to Emitting_Low
				NextState = Emitting_Low;
				// Increment PulseCounter
				PulseCounter++;
				// Start IR_PULSE_TIMER for PULSE_LOW_TIME
				ES_Timer_InitTimer(IR_PULSE_TIMER,PULSE_LOW_TIME);
			}	
			// EndIf
			break;
			// End Emitting_High block

		// If CurrentState is Emitting_Low
		case(Emitting_Low):
			if (SM_TEST) printf("Reloading: Emitting_Low");
			// If ThisEvent is ES_TIMEOUT and PulseCounter is less than NUM_PULSES
			if ((ThisEvent.EventType == ES_TIMEOUT) && 
				(ThisEvent.EventParam == IR_PULSE_TIMER) &&
				 (PulseCounter <= NUM_PULSES)){
				// Enable the IR Emitter
				Enable_IR_Emitter();
				// Set NextState to Emitting_High
				NextState = Emitting_High;
				// Start IR_PULSE_TIMER for PULSE_HIGH_TIME
				ES_Timer_InitTimer(IR_PULSE_TIMER,PULSE_HIGH_TIME);
			// Else If ThisEvent is ES_TIMEOUT and PulseCounter is equal to NUM_PULSES
			} else if ((ThisEvent.EventType == ES_TIMEOUT) && 
				(ThisEvent.EventParam == IR_PULSE_TIMER) &&
				 (PulseCounter > NUM_PULSES)){
				// Set NextState to Wait4Delivery
				NextState = Wait4Delivery;
				// Clear PulseCounter
				PulseCounter = 0;
				// Start IR_PULSE_TIMER for DELIVERY_TIME
				ES_Timer_InitTimer(IR_PULSE_TIMER,DELIVERY_TIME);
			}
			// EndIf
			break;
		// End Emitting_Low block

		// If CurrentState is Wait4Delivery
		case(Wait4Delivery):
			if (SM_TEST) printf("Reloading: Wait4Delivery");
			// If ThisEvent is ES_TIMEOUT
			if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == IR_PULSE_TIMER)) {
				// Post ES_RELOAD_COMPLETE to MasterHSM
				ES_Event Event2Post;
				Event2Post.EventType = ES_RELOAD_COMPLETE;
				if (!BALL_TRACKING) incrementBallCount();
				if (SM_TEST) printf("Ball count is now %d\r\n", getBallCount());
				PostMasterSM(Event2Post);
				// Set NextState to WaitingReload
				NextState = WaitingReload;
			}
			// EndIf

		break;
		// End Wait4Delivery block
	}

	// Set CurrentState to NextState
	CurrentState = NextState;
	//	Return ReturnEvent
	return ReturnEvent;
}
//End RunReloadingService



static void Enable_IR_Emitter(void)
{
//	Set the IR Emitter line HIGH
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA+ALL_BITS)) |= BIT4HI;
}


static void Disable_IR_Emitter(void)
{
//	Set the IR Emitter line LOW
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA+ALL_BITS)) &= ~BIT4HI;

}
