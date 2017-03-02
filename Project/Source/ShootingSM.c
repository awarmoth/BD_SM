//module level variables: MyPriority, CurrentState, ShootingTimeoutFlag GameTimeoutFlag, ExitFlag, Score, BallCount
//ShootingState_t: AlignToGoal; 

#include "MasterHSM.h"
#include "FiringService.h"
#include "LOC_HSM.h"
#include "ShootingSM.h"
#include "constants.h"
#include "MasterHSM.h"
#include "SPI_Module.h"
#include "ByteTransferSM.h"
#include "LOC_HSM.h"
#include "ConstructingSM.h"
#include "DrivingAlongTapeSM.h"
#include "hardware.h"
#include "FiringService.h"

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

static bool ExitShootingFlag;
static bool GameTimeoutFlag = false;
static uint8_t Score;
static uint8_t BallCount;
static uint8_t MyPriority;
static ShootingState_t CurrentState;

//module functions
static ES_Event DuringAlignToGoal(ES_Event ThisEvent);
static ES_Event DuringFiring(ES_Event ThisEvent);
static ES_Event DuringWaitForShotResult(ES_Event ThisEvent);
static ES_Event DuringWaitForScoreUpdate(ES_Event ThisEvent);
static ES_Event DuringAlignToTape(ES_Event ThisEvent);


void StartShootingSM(ES_Event CurrentEvent)  
{
	// Set CurrentState to AlignToGoal
	CurrentState = AlignToGoal;
	// Run ShootingSM with CurrentEvent
	RunShootingSM(CurrentEvent);
}
//End StartShootingSM


ES_Event RunShootingSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	bool MakeTransition;
	// local variable NextState
	ShootingState_t NextState;
	
	// local variable EntryEvent
	ES_Event EntryEvent;
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	
	// Initialize MakeTransition to false
	MakeTransition = false;
	// Initialize NextState to CurrentState
	NextState = CurrentState;
	// Initialize EntryEvent to ES_ENTRY
	EntryEvent.EventType = ES_ENTRY;
	// Initialize ReturnEvent to CurrentEvent to assume no consumption of event
	ReturnEvent = CurrentEvent;
	

	switch(CurrentState)
	{
		// If CurrentState is AlignToGoal
        case (AlignToGoal):
		{
			// Run DuringAlignToGoal and store the output in CurrentEvent
			CurrentEvent = DuringAlignToGoal(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
            if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_GOAL_BEACON_DETECTED
                if (CurrentEvent.EventType == ES_GOAL_BEACON_DETECTED) //MAY NEED TO ADD A GUARD CONDITION WITH A TIMER TO MAKE SURE FLYWHEEL IS AT DESIRED SPEED
				{
					// Stop rotating
					SetMotorController(STOP_DRIVING);
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to Firing
					NextState = Firing;
				// EndIf
				}
			}				
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
                ReturnEvent.EventType = ES_NO_EVENT;
				// EndIf
			}
		
			// End AlignToGoal block
            break;
		}
	
		// If CurrentState is Firing
        case (Firing):
        {
			// Run DuringFiring and store the output in CurrentEvent
            CurrentEvent = DuringFiring(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
            if (CurrentEvent.EventType != ES_NO_EVENT)
            {
				// If CurrentEvent is ES_TIMEOUT from SHOOTING_TIMER
                if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == SHOOTING_TIMER))
                {
					// Transform ReturnEvent to ES_NO_EVENT
                    CurrentEvent.EventType = ES_NO_EVENT;
					// Set ShootingTimeoutFlag
                    ExitShootingFlag = true;
                }
				// Else If CurrentEvent is ES_TIMEOUT from GAME_TIMER
                else if((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == GAME_TIMER))
				{
					// Transform ReturnEvent to ES_NO_EVENT to consume it
					ReturnEvent.EventType = ES_NO_EVENT;
					// Set GameTimeoutFlag
					setGameTimeoutFlag(true);
				}
				//Else If CurrentEvent is ES_FIRE_COMPLETE and normal game has ended (we want to enter FFA)
				else if ((CurrentEvent.EventType == ES_FIRE_COMPLETE) && GameTimeoutFlag)
				{
					//Set NextState to AlignToTape
					NextState = AlignToTape;
					//Set MakeTransition to true
					MakeTransition = true;
				}
				// Else If CurrentEvent is ES_FIRE_COMPLETE and the normal game has not ended
				else if ((CurrentEvent.EventType == ES_FIRE_COMPLETE) && (!GameTimeoutFlag))
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set BallCount to getBallCount
					BallCount = getBallCount();
					// Set NextState to WaitForShotResult
					NextState = WaitForShotResult;
					// If BallCount = 0
					if(BallCount == NO_BALLS)
					{
						// Set ExitFlag
						ExitShootingFlag = true;
					}
					// EndIf
				}// EndIf
			}
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
			break;
		}
		// End Firing block

		// If CurrentState is WaitForShotResult
		case WaitForShotResult:
		{
			// Run DuringWaitForShotResult and store the output in CurrentEvent
			CurrentEvent = DuringWaitForShotResult(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_TIMEOUT from SHOT_RESULT_TIMER
				if((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == SHOT_RESULT_TIMER))
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to WaitForScoreUpdate
					NextState = WaitForScoreUpdate;
				// EndIf
				}
			}
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
			break;
		// End WaitForShotResult block
		}
		
		// If CurrentState is WaitForScoreUpdate
		case WaitForScoreUpdate:
		{
			// Run DuringWaitForScoreUpdate and store the output in CurrentEvent
			CurrentEvent = DuringWaitForScoreUpdate(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if(CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					// Get response bytes from LOC //MAKE SURE TO ASK ADAM IF I DID THIS RIGHT
					// SetSB1_Byte(getSB1_Byte())
					SetSB1_Byte(getSB1_Byte());
					// SetSB2_Byte(getSB2_Byte())
					SetSB2_Byte(getSB2_Byte());
					// SetSB3_Byte(getSB3_Byte())
					SetSB3_Byte(getSB3_Byte());
	
					// Set MakeTransition to true
					MakeTransition = true;
					
	//****************** Initialize NewScore to getScore ****************//
					uint8_t NewScore; // = getScore //how about changing getGreenScore and getRedScore to getScore(teamcolor)?
					
					// If NewScore = Score
					if(NewScore == Score)
					{
						// Set NextState to Firing
						NextState = Firing;
					}
					// Else
					else //we must have scored
					{
						// Set NextState to AlignToTape
						NextState = AlignToTape;
					}
					// EndIf
					// Score = NewScore
					Score = NewScore;
				}// EndIf
			}
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}// EndIf
			break;
		}
		// End WaitForScoreUpdate block

		// If CurrentState is AlignToTape
		case AlignToTape:
		{
			// Run DuringAlignToTape and store the output in CurrentEvent
			CurrentEvent = DuringAlignToTape(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_TAPE_DETECTED
				if(CurrentEvent.EventType == ES_TAPE_DETECTED)
				{
					// Transform ReturnEvent to ES_SHOOTING_COMPLETE
					ReturnEvent.EventType = ES_SHOOTING_COMPLETE;
					// If GameTimeoutFlag Set
					if(GameTimeoutFlag)
					{
						// Post ES_NORMAL_GAME_COMPLETE to Master
						ES_Event NewEvent;
						NewEvent.EventType = ES_NORMAL_GAME_COMPLETE;
						PostMasterSM(NewEvent);
					}// EndIf
				// EndIf
				}
			}
			
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}// EndIf
			break;
		}// End AlignToTape block
	}//End switch
	
	// If MakeTransition is true
	if(MakeTransition)
	{
		// Set CurrentEvent to ES_EXIT
		CurrentEvent.EventType = ES_EXIT;
		// Run ShootingSM with CurrentEvent to allow lower level SMs to exit
		RunShootingSM(CurrentEvent);
		
		// Set CurrentState to NextState
		CurrentState = NextState;
		// Run ShootingSM with EntryEvent to allow lower level SMs to enter
		RunShootingSM(EntryEvent);
	}// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
	
}



static ES_Event DuringAlignToGoal(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Start rotating // direction based on team color
		uint8_t TeamColor = getTeamColor();
		if (TeamColor == GREEN) {
			SetMotorController(ROTATE_CCW);
		} else {
			SetMotorController(ROTATE_CW);
		}
		// Set OldScore to getScore
		//reset exit flag
		ExitShootingFlag = false;
	}// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
	
}



static ES_Event DuringFiring(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable Event2Post
	ES_Event Event2Post;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Set Event2Post to a ES_FIRE
		Event2Post.EventType = ES_FIRE;
		// Post Event2Post to Firing Service
		PostFiringService(Event2Post);
	// EndIf
	}
	// Return ReturnEvent
	return ReturnEvent;
}



static ES_Event DuringWaitForShotResult(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY)  || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Start SHOT_RESULT_TIMER
		ES_Timer_InitTimer(SHOT_RESULT_TIMER, BALL_AIR_TIME);
	} 
//	EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}



static ES_Event DuringWaitForScoreUpdate(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable Event2Post
	ES_Event Event2Post;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Set Byte2Write to status byte
		uint8_t Byte2Write = STATUS_COMMAND;
		// Post ES_Command to LOC w/ parameter: Byte2Write
		Event2Post.EventParam = Byte2Write;
		Event2Post.EventType = ES_COMMAND;
		PostLOC_SM(Event2Post);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}


static ES_Event DuringAlignToTape(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		uint8_t TeamColor = getTeamColor();
		if (TeamColor == GREEN) {
			SetMotorController(ROTATE_CW);
		} else {
			SetMotorController(ROTATE_CCW);
		}
		// direction based on team color, opposite of AlignToGoal
	}// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}

void setGameTimeoutFlag(bool flag)
{
	GameTimeoutFlag = flag;
}

bool getGameTimeoutFlag(void)
{
	return GameTimeoutFlag;
}
