/****************************************************************************
 Module
   DrivingAlongTapeSM.c

 Revision
   2.0.1

 Description
	This is a state machine that handles navigation of the robot along the wire
	to the check in stations or to the reload station.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/20/17 10:14 jec      correction to Run function to correctly assign 
                         ReturnEvent in the situation where a lower level
                         machine consumed an event.
 02/03/16 12:38 jec      updated comments to reflect changes made in '14 & '15
                         converted unsigned char to bool where appropriate
                         spelling changes on true (was True) to match standard
                         removed local var used for debugger visibility in 'C32
                         commented out references to Start & RunLowerLevelSM so
                         that this can compile. 
 02/07/13 21:00 jec      corrections to return variable (should have been
                         ReturnEvent, not CurrentEvent) and several EV_xxx
                         event names that were left over from the old version
 02/08/12 09:56 jec      revisions for the Events and Services Framework Gen2
 02/13/10 14:29 jec      revised Start and run to add new kind of entry function
                         to make implementing history entry cleaner
 02/13/10 12:29 jec      added NewEvent local variable to During function and
                         comments about using either it or Event as the return
 02/11/10 15:54 jec      more revised comments, removing last comment in during
                         function that belongs in the run function
 02/09/10 17:21 jec      updated comments about internal transitions on During funtion
 02/18/09 10:14 jec      removed redundant call to RunLowerlevelSM in EV_Entry
                         processing in During function
 02/20/07 21:37 jec      converted to use enumerated type for events & states
 02/13/05 19:38 jec      added support for self-transitions, reworked
                         to eliminate repeated transition code
 02/11/05 16:54 jec      converted to implment hierarchy explicitly
 02/25/03 10:32 jec      converted to take a passed event parameter
 02/18/99 10:19 jec      built template from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
// Framework inclusions
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

#include "MasterHSM.h"
#include "ConstructingSM.h"
#include "DrivingAlongTapeSM.h"
#include "ByteTransferSM.h"
#include "constants.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/


/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines
#define INITIAL_STATION 4 //dummy station for when the robot is at the start location
#define STAGING_AREA_1 1
#define STAGING_AREA_2 2
#define STAGING_AREA_3 3
#define SUPPLY_DEPOT 0
#define FORWARD 1
#define REVERSE -1
#define TICKS_PER_MS 40000
#define CONTROLLER_TIME_US 2000
#define TICKS_PER_US 40


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringWaiting(ES_Event ThisEvent);
static ES_Event DuringDriving2Station(ES_Event ThisEvent);
static ES_Event DuringDriving2Reload(ES_Event ThisEvent);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static DrivingState_t CurrentState;
static uint8_t LastStation = INITIAL_STATION;
static uint8_t TargetStation = 1;
static int8_t Direction;


/*------------------------------ Module Code ------------------------------*/


/****************************************************************************
 Function
    RunDrivingAlongTapeSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   This functions handles the execution of the DrivingAlongTape state machine.
	 This machine idles until it receives a target station which will either be
	 a staging area or reloading station. Then it will drive to that station.
 Notes
   uses nested switch/case to implement the machine.
 Author
 Brett Glasner, 2/22/17, 12:59AM
****************************************************************************/
ES_Event RunDrivingAlongTapeSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	bool MakeTransition;
	// local variable NextState
	DrivingState_t NextState;
	// local variable EntryEvent
	ES_Event EntryEvent;
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	
	// Initialize MakeTransition to false
	MakeTransition = false;
	// Initialize NextState to CurrentState
	NextState = CurrentState;
	// Initialize EntryEvent to ES_ENTRY by default
	EntryEvent.EventType = ES_ENTRY;
	// Initialize ReturnEvent to CurrentEvent assuming no consuming of event
	ReturnEvent = CurrentEvent;
	
	
	switch(CurrentState)
	{
		// If CurrentState is Waiting //NOTE CORRECT STATE MACHINE TO NOT TURN ON TAPE CONTROL ON WAITING EXIT
		case Waiting:
		{
			if(SM_TEST) printf("DrivingAlongTape: Waiting\r\n");

			// Call DuringWaiting and set CurrentEvent to its return value
			CurrentEvent = DuringWaiting(CurrentEvent);
			// If CurrentEvent is not an ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_DriveAlongTape
				if(CurrentEvent.EventType == ES_DRIVE_ALONG_TAPE)
				{
					TargetStation = CurrentEvent.EventParam;
					// If TargetStation is LastStation
					if(TargetStation == LastStation)
					{
						// Set MakeTransition to true			
						MakeTransition = true;
						// Set ReturnEvent to ES_ArrivedAtStation
						if (!SM_TEST) ReturnEvent.EventType = ES_ARRIVED_AT_STATION;
					}
				
					// If TargetStation is not LastStation and TargetStation is not supply depot
					else if((TargetStation != LastStation) && (TargetStation != SUPPLY_DEPOT))
					{
						// Set NextState to Driving2Station
						NextState = Driving2Station;
						// Set MakeTransition to true
						MakeTransition = true;
						
						/*******Enable wire following control law********/
						/*****************Start driving******************/						

						// Set Direction from sign(LastStation - TargetStation)
						if((LastStation - TargetStation) > 0) //LastStation is greater than TargetStation, so to count down we drive towards the SUPPLY_DEPOT
						{
							Direction = FORWARD;
						}
						
						else //LastStation is less than TargetStation, so to count up we drive towards the NAV_BEACON
						{
							Direction = REVERSE;
						}
					}
					
					// If TargetStation is supply depot
					else if(TargetStation == SUPPLY_DEPOT)
					{
						// Set NextState to Driving2Reload
						NextState = Driving2Reload;
						// Set MakeTransition to true
						MakeTransition = true;
						
						/*******Enable wire following control law********/
						/*****************Start driving******************/							

						// Set Direction from sign(LastStation - TargetStation)
						if((LastStation - TargetStation) > 0) //LastStation is greater than TargetStation, so to count down we drive towards the SUPPLY_DEPOT
						{
							Direction = FORWARD;
						}
						
						else //LastStation is less than TargetStation, so to count up we drive towards the NAV_BEACON
						{
							Direction = REVERSE;
						}
					}
					
					else //something unexpected happened
					{
						printf("Unexpected Station: Station Number = %d\r\n", TargetStation);
					}
				}
				// End ES_DriveAlongTape block
			}
			
			else // Else CurrentEvent is an ES_NO_EVENT
			{
				ReturnEvent = CurrentEvent;// Update ReturnEvent to ES_NO_EVENT
			}
		
			break;
		}
		// End Waiting block
	
		// If CurrentState is DrivingToStation
		case Driving2Station:
			
		{
			if(SM_TEST) printf("DrivingAlongTape: Driving2Station\r\n");
			// Call DuringDrivingToStation and set CurrentEvent to its return value
			CurrentEvent = DuringDriving2Station(CurrentEvent);
		
			// If CurrentEvent is not an ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_StationDetected
				if(CurrentEvent.EventType == ES_STATION_DETECTED)
				{
					// If LastStation - Direction is TargetStation
					if((LastStation - Direction) == TargetStation)
					{
						// Set MakeTransition to true	
						MakeTransition = true;
						
						/*******Disable wire following control law********/
						/*****************Stop driving******************/
						
						// Set ReturnEvent to ES_ArrivedAtStation
						if (!SM_TEST) ReturnEvent.EventType = ES_ARRIVED_AT_STATION;
					}
					// Endif
			
					//Else we must not be at the target station so we keep going
					else
					{
						// Set MakeTransition to true
						MakeTransition = true;
					}
					// Endif	
				// End ES_StationDetected block
				}
			// Endif
			}
			
			// Else CurrentEvent is an ES_NO_EVENT
			else
			{
				// Update ReturnEvent to ES_NO_EVENT
				ReturnEvent = CurrentEvent;
			}
			// Endf
			
		// End DrivingToStation block
		}
		
		// If CurrentState is DrivingToReload
		case Driving2Reload:
		{
			// Call DuringDrivingToReload and set CurrentEvent to its return value
			CurrentEvent = DuringDriving2Reload(CurrentEvent);
			
			// If CurrentEvent is not an ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_Front_Bump_Detected
				if(CurrentEvent.EventType == ES_FRONT_BUMP_DETECTED)
				{
					// Set MakeTransition to true
					MakeTransition = true;
					
					/*******Disable wire following control law********/
					/*****************Stop driving******************/
					
					// Set ReturnEvent to ES_ArrivedAtReload	
					ReturnEvent.EventType = ES_ARRIVED_AT_RELOAD;
				}					
				// End ES_Front_Bump_Detected block
			}
			// Endif
		
			// Else CurrentEvent is an ES_NO_EVENT
			else
			{
				// Update ReturnEvent to ES_NO_EVENT
				ReturnEvent = CurrentEvent;
			}
			// Endf
		}
		// End DrivingToReload block
	}//end switch
		
	// If MakeTransition is true
	if(MakeTransition)
	{
		// Set CurrentEvent to ES_EXIT
		CurrentEvent.EventType = ES_EXIT;
		// Call RunDrivingAlongTapeSM with CurrentEvent as the event parameter
		RunDrivingAlongTapeSM(CurrentEvent);
				
		// Set CurrentState to NextState
		CurrentState = NextState;
		// Call RunDrivingAlongTapeSM with EntryEvent as the event parameter
		RunDrivingAlongTapeSM(EntryEvent);
	}
			
	// Endif
	
	// Return ReturnEvent
	return ReturnEvent;
}
/****************************************************************************
 Function
     StartDrivingAlongTapeSM

 Parameters
     ES_Event the entry event to process

 Returns
     None

 Description
     Performs the entry procedure for the Driving Along Tape state machine
 Notes

 Author
 Brett Glasner, 2/23/17, 11:49am
****************************************************************************/
void StartDrivingAlongTapeSM(ES_Event CurrentEvent)
{
	// If CurrentEvent is not ES_ENTRY_HISTORY
	if(CurrentEvent.EventType != ES_ENTRY_HISTORY)
	{
		// Set CurrentState to Waiting
		CurrentState = Waiting;
	}
	// Endif
	
	// Call RunDrivingAlongTapeSM with CurrentEvent as the event parameter
	RunDrivingAlongTapeSM(CurrentEvent);
}



/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringWaiting(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent assuming no re-mapping or consumption
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Set LastStation to getLastStation
		// //Probably want to initialize module variables here as well
	}
	// EndIf
	
	// ElseIf ThisEvent is ES_EXIT
	else if(ThisEvent.EventType == ES_EXIT)
	{
		//do nothing
	}
	
	else
	{
		//do nothing
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}

static ES_Event DuringDriving2Station(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent assuming no re-mapping or consumption
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		//do nothing
	}
	// EndIf
	
	// ElseIf ThisEvent is ES_EXIT
	else if(ThisEvent.EventType == ES_EXIT)
	{
		// Set LastStation to (LastStation - Direction)
		LastStation = (LastStation - Direction); //update LastStation to be the station we just passed
	}
	// EndIf
	
	else
	{
		//do nothing
	}
	
	// Return ReturnEvent
	return ReturnEvent;
}


static ES_Event DuringDriving2Reload(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent assuming no re-mapping or consumption
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		//do nothing
	}
	// EndIf
	
	// ElseIf ThisEvent is ES_EXIT
	else if(ThisEvent.EventType == ES_EXIT)
	{
		// Set LastStation to supply depot
		LastStation = SUPPLY_DEPOT;
	}
	// EndIf
	
	else
	{
		//do nothing
	}
	
	// Return ReturnEvent
	return ReturnEvent;
}

//static void ControlTimerInit(void)
//{
//	// Enable the clock to the timer (wide timer 2)
//	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R2;
//	
//	// Make sure the clock has gotten going
//	while((HWREG(SYSCTL_PRWTIMER) & SYSCTL_PRWTIMER_R2) != SYSCTL_PRWTIMER_R2){
//	}
//	
//	// Make sure the timer is disabled before configuring
//	HWREG(WTIMER2_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
//	
//	// Set up timer in 32 bit wide mode
//	HWREG(WTIMER2_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
//	
//	// Set up timer in periodic mode
//	HWREG(WTIMER2_BASE+TIMER_O_TAMR) = (HWREG(WTIMER2_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAMR_M) | TIMER_TAMR_TAMR_PERIOD;
//	
//	// Set the timeout
//	HWREG(WTIMER2_BASE+TIMER_O_TAILR) = CONTROLLER_TIMEOUT;
//	
//	// Enable a local timeout interrupt
//	HWREG(WTIMER2_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
//	
//	// Enable timer A in NVIC
//	HWREG(NVIC_EN3) |= BIT2HI;
//	
//	// Set the response to a lower priority
//	HWREG(NVIC_PRI24) = (HWREG(NVIC_PRI24) & ~NVIC_PRI24_INTC_M) | BIT5HI;
//	
//	// Enable interrupts globally
//	__enable_irq();
//	
//	// Enable timer and enable to stall when stopped by debugger
//	HWREG(WTIMER2_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
//}

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

