#include "MasterHSM.h"
#include "CheckInSM.h"
#include "LOC_HSM.h"
#include "DrivingAlongTapeSM.h"

#include "ConstructingSM.h"
#include "ByteTransferSM.h"

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

// module level variables: 
uint8_t MyPriority;
ConstructingState_t CurrentState;
uint8_t TeamColor;
uint8_t BallCount = 3;
uint8_t TargetStation;
uint8_t LastStation = START;
uint8_t TargetGoal;
uint8_t Score;
bool GameTimeoutFlag = false;

void StartConstructingSM(ES_Event CurrentEvent)
{
	// Set CurrentState to GettingTargetStation
	CurrentState = GettingTargetStation;
	// Run ConstructingSM with CurrentEvent
	RunConstructingSM(CurrentEvent);
}
// End StartConstructingSM


ES_Event RunConstructingSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	bool MakeTransition;
	// local variable NextState
	MasterState_t NextState;
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable EntryEvent
	ES_Event EntryEvent;
	// local variable Event2Post
	ES_Event Event2Post;
	
	// Initialize MakeTransition to false
	MakeTransition = false;
	// Initialize NextState to CurrentState
	NextState = CurrentState;
	// Initialize EntryEvent to ES_ENTRY
	EntryEvent.EventType = ES_ENTRY;
	// Initialize ReturnEvent to ES_NO_EVENT
	ReturnEvent.EventType = ES_NO_EVENT;
	
	switch (CurrentState)
	{
		// If CurrentState is GettingTargetStation
		case(GettingTargetStation):
			// Run DuringGettingTargetStation and store the output in CurrentEvent
			CurrentEvent = DuringGettingTargetStation(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					// Get response bytes from LOC
					// Set SB1_byte to getSB1_Byte
					SetSB1_Byte(getSB1_Byte());
					// Set SB2_byte to getSB2_Byte
					SetSB2_Byte(getSB2_Byte());
					// Set SB3_byte to getSB3_Byte
					SetSB3_Byte(getSB3_Byte());
					// Update status variables
					UpdateStatus();
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to DrivingAlongTape
					NextState = DrivingAlongTape;
					// Set Event2Post type to ES_DRIVE_ALONG_TAPE
					Event2Post.EventType = ES_DRIVE_ALONG_TAPE;
					// Set Event2Post Param to TargetStation
					Event2Post.EventParam = TargetStation;
					// Post Event2Post to Master
					PostMasterSM(Event2Post);
				}
				// End ES_LOC_COMPLETE block
			}	
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End GettingTargetStation block
	
		// If CurrentState is DrivingAlongTape
		case(DrivingAlongTape):
			// Run DuringDrivingAlongTape and store the output in CurrentEvent
			CurrentEvent = DuringDrivingAlongTape(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_ARRIVED_AT_STATION
				if (CurrentEvent.EventType == ES_ARRIVED_AT_STATION)
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to CheckIn
					NextState = CheckIn;
				}
				// Else if CurrentEvent is ES_ARRIVED_AT_RELOAD
				else if (CurrentEvent.EventType == ES_ARRIVED_AT_RELOAD)
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to Reloading
					NextState = Reloading;
					// Post ES_RELOAD_START to ReloadService
					// PostReload(ES_RELOAD_START);
				}
				// EndIf
			}
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End DrivingAlongTape block

		// If CurrentState is CheckIn
		case(CheckIn):
			// Run DuringCheckIn and store the output in CurrentEvent
			CurrentEvent = DuringCheckIn(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_REORIENT
				if (CurrentEvent.EventType == ES_ARRIVED_AT_STATION)
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to DrivingAlongTape
					NextState = DrivingAlongTape;
					// Set Event2Post type to ES_DRIVE_ALONG_TAPE
					Event2Post.EventType = ES_DRIVE_ALONG_TAPE;
					// Set Event2Post param to RELOAD
					Event2Post.EventParam = RELOAD;
					// Post Event2Post to Master
					PostMasterSM(Event2Post);
				}
				// Else If CurrentEvent is ES_GOAL_READY
				else if (CurrentEvent.EventType == ES_GOAL_READY)
				{
					// Set TargetGoal to CurrentEvent Param
					TargetGoal = CurrentEvent.EventParam;
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to Shooting
					if (!CheckOff3) NextState = Shooting;
					else NextState = GettingTargetStation;
				}
				// EndIf
			}
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End CheckIn block
		
		// If CurrentState is Shooting
		case(Shooting):
			// Run DuringShooting and store the output in CurrentEvent
			CurrentEvent = DuringShooting(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_SHOOTING_COMPLETE or ES_TIMEOUT from SHOOTING_TIMER
				if ((CurrentEvent.EventType == ES_SHOOTING_COMPLETE) ||
					((CurrentEvent.EventType == ES_TIMEOUT) &&
					(CurrentEvent.EventParam == SHOOTING_TIMER)))
				{
					// If BallCount = 0
					if (BallCount == 0)
					{
						// Set MakeTransition to true
						MakeTransition = true;
						// Set NextState to DrivingAlongTape
						NextState = DrivingAlongTape;
						// Set Event2Post type to ES_DRIVE_ALONG_TAPE
						Event2Post.EventType = ES_DRIVE_ALONG_TAPE;
						// Set Event2Post param to RELOAD
						Event2Post.EventParam = RELOAD;
						// Post Event2Post to Master
						PostMasterSM(Event2Post);
					}
					// Else
					else
					{
						// Set MakeTransition to true
						MakeTransition = true;
						// Set NextState to GettingTargetStation
						NextState = GettingTargetStation;
					}
				}
				// EndIf
			}
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End Shooting block

		// If CurrentState is Reloading
		case(Reloading):
			// Run DuringReloading and store the output in CurrentEvent
			CurrentEvent = DuringReloading(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_RELOAD_COMPLETE
				if (CurrentEvent.EventType == ES_RELOAD_COMPLETE)
				{
					// If BallCount < 5
					if (BallCount < 5)
					{
						// Set MakeTransition to true
						MakeTransition = true;
					}
					// Else
					else
					{
						// Set MakeTransition to true
						MakeTransition = true;
						// Set NextState to GettingTargetStation
						NextState = GettingTargetStation;
					}
				}
				// EndIf
				// Else If CurrentEvent is ES_TIMEOUT from GAME_TIMER
				else if ((CurrentEvent.EventType == ES_TIMEOUT) &&
						(CurrentEvent.EventParam == GAME_TIMER))
				{
					// Set GameTimeoutFlag
					GameTimeoutFlag = true;
					// Set ReturnEvent to ES_NO_EVENT
					ReturnEvent.EventType = ES_NO_EVENT;
				}
				// EndIf
			}
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End Reloading block
	}
	// End switch

	// If MakeTransition is true
	if (MakeTransition == true)
	{
		// Set CurrentEvent to ES_EXIT
		CurrentEvent.EventType = ES_EXIT;
		// Run ConstructingSM with CurrentEvent to allow lower level SMs to exit
		RunConstructingSM(CurrentEvent);
		
		// Set CurrentState to NextState
		CurrentState = NextState;
		// Run ConstructingSM with EntryEvent to allow lower level SMs to enter
		RunConstructingSM(EntryEvent);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}
// End RunConstructingSM


ES_Event DuringGettingTargetStation(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable Event2Post
	ES_Event Event2Post;

	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if ((ThisEvent.EventType == ES_ENTRY) ||
		(ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Set Event2Post type to ES_Command
		Event2Post.EventType = ES_COMMAND;
		// Set Event2Post param to STATUS_COMMAND
		Event2Post.EventParam = STATUS_COMMAND;
		// Post Event2Post to LOC_SM
		PostLOC_SM(Event2Post);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}
// End DuringGettingTargetStation


ES_Event DuringDrivingAlongTape(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;

	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	if ((ThisEvent.EventType == ES_ENTRY) ||
		(ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Start DrivingAlongTapeSM with ThisEvent
		StartDrivingAlongTapeSM(ThisEvent);
	}
	// Else If ThisEvent is ES_EXIT
	else if (ThisEvent.EventType == ES_EXIT)
	{
		// Run DrivingAlongTapeSM with ThisEvent
		RunDrivingAlongTapeSM(ThisEvent);
		// Turn off motors/control
		/**********************************************/
		/**********************************************/
		/**********************************************/
		/**********************************************/
	// Else
	}
	else
	{
		// Run DrivingAlongTapeSM with ThisEvent and store result in ReturnEvent
		ReturnEvent = RunDrivingAlongTapeSM(ThisEvent);

	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
	
}
//End DuringDrivingAlongTape


ES_Event DuringCheckIn(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;

	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	if ((ThisEvent.EventType == ES_ENTRY) ||
		(ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Start CheckInSM with ThisEvent
		StartCheckInSM(ThisEvent);
	}
	// Else If ThisEvent is ES_EXIT
	else if (ThisEvent.EventType == ES_EXIT)
	{
		// Run CheckInSM with ThisEvent
		RunCheckInSM(ThisEvent);
	// Else
	}
	else
	{
		// Run CheckInSM with ThisEvent and store result in ReturnEvent
		ReturnEvent = RunCheckInSM(ThisEvent);

	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
	
}
// End DuringCheckIn


ES_Event DuringShooting(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;

	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	if ((ThisEvent.EventType == ES_ENTRY) ||
		(ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Start ShootingSM with ThisEvent
		//StartShootingSM(ThisEvent);
	}
	// Else If ThisEvent is ES_EXIT
	else if (ThisEvent.EventType == ES_EXIT)
	{
		// Run ShootingSM with ThisEvent
		//RunShootingSM(ThisEvent);
	}
	// Else
	else
	{
		// Run ShootingSM with ThisEvent and store result in ReturnEvent
		//ReturnEvent = RunShootingSM(ThisEvent);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;	
}
// End DuringShooting

ES_Event DuringReloading(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable Event2Post
	ES_Event Event2Post;

	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if ((ThisEvent.EventType == ES_ENTRY) ||
		(ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Set Event2Post type to ES_RELOAD_START
		Event2Post.EventType = ES_RELOAD_START;
		// Post Event2Post to ReloadService
		//PostReload(Event2Post);
	}
	// Else If ThisEvent is ES_EXIT
	else
	{
		// If normalgame timeout flag set
		if (GameTimeoutFlag)
		{
			//post ES_Norm_Game_Complete to Master
			Event2Post.EventType = ES_NORM_GAME_COMPLETE;
			PostMasterSM(Event2Post);
		}
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}
// End DuringReloading


void UpdateStatus( void )
{
	if (TeamColor == RED) {
		TargetStation = getActiveStageRed();
		Score = getScoreRed();
	}
	else // TeamColor == GREEN
	{
		TargetStation = getActiveStageGreen();
		Score = getScoreGreen();
	}
}
