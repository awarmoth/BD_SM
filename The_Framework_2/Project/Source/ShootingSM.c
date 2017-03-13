//ShootingSM.c

//module level variables: MyPriority, CurrentState, ShootingTimeoutFlag GameTimeoutFlag, ExitFlag, Score, BallCount
//ShootingState_t: AlignToGoal; 

#include "MasterHSM.h"
#include "FiringService.h"
#include "LOC_HSM.h"
#include "ShootingSM.h"
#include "constants.h"
#include "SPI_Module.h"
#include "ByteTransferSM.h"
#include "ConstructingSM.h"
#include "DrivingAlongTapeSM.h"
#include "hardware.h"
#include "FiringService.h"
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

static bool ExitShootingFlag;
static bool GameTimeoutFlag = false;
static uint8_t Score;
static uint8_t BallCount;
static ShootingState_t CurrentState;

//module functions
static ES_Event DuringAlignToGoal(ES_Event ThisEvent);
static ES_Event DuringFiring(ES_Event ThisEvent);
static ES_Event DuringWaitForShotResult(ES_Event ThisEvent);
static ES_Event DuringWaitForScoreUpdate(ES_Event ThisEvent);
static ES_Event DuringWarmingUp(ES_Event ThisEvent);

static uint8_t getFlywheelDuty(uint8_t TargetGoal,uint8_t LastStation);

/****************************************************************************
 Function
     StartShootingSM

 Parameters
     None

 Returns
     None

 Description
     Does any required initialization for this state machine
 Notes

 Author
     Adam Warmoth
****************************************************************************/
void StartShootingSM(ES_Event CurrentEvent)  
{
	// Set CurrentState to AlignToGoal
	CurrentState = AlignToGoal;
	// Run ShootingSM with CurrentEvent
	RunShootingSM(CurrentEvent);
}
//End StartShootingSM

/****************************************************************************
 Function
    RunShootingSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   Runs the shooting state machine which handles aligning to the goal and commanding a shot to fire
 Notes
   uses nested switch/case to implement the machine.
 Author
   Adam Warmoth
****************************************************************************/
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
			if (SM_TEST) printf("ShootingSM: AlignToGoal\r\n");
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
					NextState = WarmingUp;
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
	
		case (WarmingUp):
			if (SM_TEST) printf("ShootingSM: WarmingUp\r\n");
			// Run DuringFiring and store the output in CurrentEvent
			CurrentEvent = DuringWarmingUp(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
      if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_TIMEOUT from WARM_UP_TIMER
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == WARM_UP_TIMER))
				{
					MakeTransition = true;
					NextState = Firing;
				}
			} else {
				// Set ReturnEvent to ES_NO_EVENT
        ReturnEvent.EventType = ES_NO_EVENT;
			} 
		
		// If CurrentState is Firing
    case (Firing):
        {
					if (SM_TEST) printf("ShootingSM: Firing\r\n");
					// Run DuringFiring and store the output in CurrentEvent
					CurrentEvent = DuringFiring(CurrentEvent);
					// If CurrentEvent is not ES_NO_EVENT
          if (CurrentEvent.EventType != ES_NO_EVENT)
          {
						GameTimeoutFlag = getGameTimeoutFlag();
						// If CurrentEvent is ES_TIMEOUT from SHOOTING_TIMER
            if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == SHOOTING_TIMER))
            {
							// Transform ReturnEvent to ES_NO_EVENT
              ReturnEvent.EventType = ES_NO_EVENT;
							// Set ShootingTimeoutFlag
              ExitShootingFlag = true;
            }
					// Else If CurrentEvent is ES_TIMEOUT from GAME_TIMER
          else if((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == GAME_TIMER))
					{
						// Transform ReturnEvent to ES_NO_EVENT
              ReturnEvent.EventType = ES_NO_EVENT;
						// Set GameTimeoutFlag
						setGameTimeoutFlag(true);
					}
				//Else If CurrentEvent is ES_FIRE_COMPLETE and normal game has ended (we want to enter FFA)
				else if ((CurrentEvent.EventType == ES_FIRE_COMPLETE) && GameTimeoutFlag)
				{
					if (!BALL_TRACKING) decrementBallCount();
					if (SM_TEST) printf("Ball count is now %d\r\n", getBallCount());
					ReturnEvent.EventType = ES_SHOOTING_COMPLETE;
				}
				// Else If CurrentEvent is ES_FIRE_COMPLETE and the normal game has not ended
				else if ((CurrentEvent.EventType == ES_FIRE_COMPLETE) && (~GameTimeoutFlag))
				{
					if (!BALL_TRACKING) decrementBallCount();
					if (SM_TEST) printf("Ball count is now %d\r\n", getBallCount());
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
						if (SM_TEST) printf("exitflag");
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
			if (SM_TEST) printf("ShootingSM: WaitForShotResult\r\n");
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
				} else {
					// If CurrentEvent is ES_TIMEOUT from SHOOTING_TIMER
            if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == SHOOTING_TIMER))
            {
							// Transform ReturnEvent to ES_NO_EVENT
              ReturnEvent.EventType = ES_NO_EVENT;
							// Set ShootingTimeoutFlag
              ExitShootingFlag = true;
            }
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
			if (SM_TEST) printf("ShootingSM: WaitForScoreUpdate\r\n");
			// Run DuringWaitForScoreUpdate and store the output in CurrentEvent
			CurrentEvent = DuringWaitForScoreUpdate(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if(CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					// Get response bytes from LOC
					// SetSB1_Byte(getSB1_Byte())
					if (!NO_LOC){
					SetSB1_Byte(getSB1_Byte());
					// SetSB2_Byte(getSB2_Byte())
					SetSB2_Byte(getSB2_Byte());
					// SetSB3_Byte(getSB3_Byte())
					SetSB3_Byte(getSB3_Byte());
					}
					// Set MakeTransition to true
					MakeTransition = true;
					
	//****************** Initialize NewScore to getScore ****************//
					uint8_t NewScore;
					if (!NO_LOC){
						uint8_t Team = getTeamColor();
						if (Team == GREEN){
							NewScore = getScoreGreen();
						}else {
							NewScore = getScoreRed();
						}
					}
					else {NewScore = getScore();}
					// If NewScore = Score
					if((NewScore == Score) && (ExitShootingFlag == false))
					{
						// Set NextState to Firing
						NextState = Firing;
					}
					// Else
					else //we must have scored or 20s is up
					{
						// Transform to ShootingComplete
						ReturnEvent.EventType = ES_SHOOTING_COMPLETE;
					}
					// EndIf
					// Score = NewScore
					Score = NewScore;
				}// EndIf
			} else if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == SHOOTING_TIMER))
            {
							// Transform ReturnEvent to ES_NO_EVENT
              ReturnEvent.EventType = ES_NO_EVENT;
							// Set ShootingTimeoutFlag
              ExitShootingFlag = true;
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
		printf("align to goal entry");
		
		HWREG(WTIMER0_BASE+TIMER_O_CTL) |= TIMER_CTL_TBEN;
		// Start rotating // direction based on team color
		uint8_t TeamColor = getTeamColor();
		if (TeamColor == GREEN) {
			SetMotorController(ROTATE_CW);
		} else {
			SetMotorController(ROTATE_CCW);
		}
		Score = getScore();
		//reset exit flag
		ExitShootingFlag = false;
	} else if (ThisEvent.EventType == ES_EXIT) {
		// EndIf
	}
	// Return ReturnEvent
	return ReturnEvent;
	
}

static ES_Event DuringWarmingUp(ES_Event ThisEvent) {
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		SetFlywheelDuty(FIRING_SPEED);
		ES_Timer_InitTimer(WARM_UP_TIMER, WARM_UP_TIME);
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
		if (NO_LOC) printf("Posting Command: Status to LOC\r\n");
		else PostLOC_SM(Event2Post);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}

static uint8_t getFlywheelDuty(uint8_t TargetGoal,uint8_t LastStation) {
	return 60;
}
