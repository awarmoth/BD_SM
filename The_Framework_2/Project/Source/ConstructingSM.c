#include "MasterHSM.h"
#include "CheckInSM.h"
#include "LOC_HSM.h"
#include "DrivingAlongTapeSM.h"
#include "PWM_Module.h"
#include "hardware.h"
#include "ShootingSM.h"
#include "ReloadingService.h"

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
static ConstructingState_t CurrentState;
static uint8_t TeamColor;
static uint8_t TargetStation;
//static uint8_t LastStation = START;
static uint8_t TargetGoal;
static uint8_t Score;
static uint32_t LastCapture, HallSensorPeriod;
static uint32_t LastPeriod, DeltaAvg;
static uint8_t HasLeftStage = true;
static bool GameTimeoutFlag = false;
static bool initHallEffect = true;

static ES_Event DuringGettingTargetStation( ES_Event Event);
static ES_Event DuringDrivingAlongTape( ES_Event Event);
static ES_Event DuringCheckIn( ES_Event Event);
static ES_Event DuringShooting( ES_Event Event);
static ES_Event DuringShooting( ES_Event Event);
static ES_Event DuringAlignToTape (ES_Event Event);
static ES_Event DuringReloading(ES_Event ThisEvent);

void StartConstructingSM(ES_Event CurrentEvent)
{
	// Set CurrentState to GettingTargetStation
	CurrentState = GettingTargetStation;
	// Run ConstructingSM with CurrentEvent
	RunConstructingSM(CurrentEvent);
	TeamColor = getTeamColor();
	if(SM_TEST) TeamColor = TEAM_COLOR;
}
// End StartConstructingSM


ES_Event RunConstructingSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	bool MakeTransition;
	// local variable NextState
	ConstructingState_t NextState;
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
			if (SM_TEST) printf("Constructing: GettingTargetStation\r\n");
			// Run DuringGettingTargetStation and store the output in CurrentEvent
			CurrentEvent = DuringGettingTargetStation(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					// Get response bytes from LOC
					if (!NO_LOC){
					// Set SB1_byte to getSB1_Byte
					SetSB1_Byte(getSB1_Byte());
					// Set SB2_byte to getSB2_Byte
					SetSB2_Byte(getSB2_Byte());
					// Set SB3_byte to getSB3_Byte
					SetSB3_Byte(getSB3_Byte());
					}
					// Update status variables
					UpdateStatus();
					if (TeamColor == GREEN) {
						TargetStation = getActiveStageGreen();
					} else {
						TargetStation = getActiveStageRed();
					}
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to DrivingAlongTape
					NextState = DrivingAlongTape;
					// Set Event2Post type to ES_DRIVE_ALONG_TAPE
					Event2Post.EventType = ES_DRIVE_ALONG_TAPE;
					// Set Event2Post Param to TargetStation
					Event2Post.EventParam = TargetStation;
					// Post Event2Post to Master
					if (!NO_LOC) PostMasterSM(Event2Post);
					else if (SM_TEST) printf("Target station from constructing: %i",TargetStation);
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
			if (SM_TEST) printf("Constructing: DrivingAlongTape\r\n");
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
			//if (SM_TEST) printf("Constructing: CheckIn\r\n");
			// Run DuringCheckIn and store the output in CurrentEvent
			CurrentEvent = DuringCheckIn(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_REORIENT
				if (CurrentEvent.EventType == ES_REORIENT)
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
					NextState = Shooting;
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
			if (SM_TEST) printf("Constructing: Shooting\r\n");
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
						// Set MakeTransition to true
						MakeTransition = true;
						// Set NextState to DrivingAlongTape
						NextState = AlignToTape;
				} else if ((CurrentEvent.EventType == ES_TIMEOUT) && 
				(CurrentEvent.EventParam == GAME_TIMER)){
					ReturnEvent.EventType = ES_NO_EVENT;
					GameTimeoutFlag = true;
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
			
	// If CurrentState is AlignToTape
		case (AlignToTape):
		{
			if (SM_TEST) printf("ConstructingSM: AlignToTape\r\n");
			// Run DuringAlignToTape and store the output in CurrentEvent
			CurrentEvent = DuringAlignToTape(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_TAPE_DETECTED
				if(CurrentEvent.EventType == ES_TAPE_DETECTED)
				{
					if (getBallCount() == 0)
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
						if (!NO_LOC) PostMasterSM(Event2Post);
					}
					// Else
					else
					{
						// Set MakeTransition to true
						MakeTransition = true;
						// Set NextState to GettingTargetStation
						NextState = GettingTargetStation;
					}
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

		// If CurrentState is Reloading
		case(Reloading):
			if (SM_TEST) printf("Construction: Reloading\r\n");
			// Run DuringReloading and store the output in CurrentEvent
			CurrentEvent = DuringReloading(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_RELOAD_COMPLETE
				if (CurrentEvent.EventType == ES_RELOAD_COMPLETE)
				{
					// If BallCount < MAX_BALLS
					if (getBallCount() < MAX_BALLS)
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
		if (NO_LOC) printf("Posting Command: Status to LOC\r\n");
		else PostLOC_SM(Event2Post);	}
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
		SetDutyA(0);
		SetDutyB(0);
		SetMotorController(STOP_DRIVING);
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
		ClearBadResponseCounter();
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
		StartShootingSM(ThisEvent);
	}
	// Else If ThisEvent is ES_EXIT
	else if (ThisEvent.EventType == ES_EXIT)
	{
		// Run ShootingSM with ThisEvent
		RunShootingSM(ThisEvent);
		SetLauncherCommand(0);
	}
	// Else
	else
	{
		// Run ShootingSM with ThisEvent and store result in ReturnEvent
		ReturnEvent = RunShootingSM(ThisEvent);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;	
}
// End DuringShooting

static ES_Event DuringAlignToTape(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		SetLauncherCommand(0);
		uint8_t TeamColor = getTeamColor();
		if (TeamColor == GREEN) {
			SetMotorController(ROTATE_CCW);
		} else {
			SetMotorController(ROTATE_CW);
		}
		SetMotorSensorDirection(FORWARD_DIR);
		FindTape();
		// direction based on team color, opposite of AlignToGoal
	}// EndIf
	if (ThisEvent.EventType == ES_EXIT) {
		SetMotorController(CONTROLLER_OFF);
		if (GameTimeoutFlag){
			ES_Event Event2Post;
			Event2Post.EventType = ES_NORMAL_GAME_COMPLETE;
			PostMasterSM(Event2Post);
		}
	}
	
	// Return ReturnEvent
	return ReturnEvent;
}

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
		if (getBallCount() == MAX_BALLS){
			// Set Event2Post type to ES_RELOAD_START
			Event2Post.EventType = ES_RELOAD_COMPLETE;
			// Post Event2Post to ReloadService
			PostMasterSM(Event2Post);
		} else {
			// Set Event2Post type to ES_RELOAD_START
			Event2Post.EventType = ES_RELOAD_START;
			// Post Event2Post to ReloadService
			PostReloadingService(Event2Post);
			SetLED(LED_SOLID_MODE,ORANGE_LED);
		}
	}
	// Else If ThisEvent is ES_EXIT
	else if (ThisEvent.EventType == ES_EXIT)
	{
			uint8_t TeamColor = getTeamColor();
			if (TeamColor == GREEN) SetLED(LED_SOLID_MODE, GREEN_LED);
			if (TeamColor == RED) SetLED(LED_SOLID_MODE, RED_LED);
		// If normalgame timeout flag set
		if (GameTimeoutFlag)
		{
			//post ES_Norm_Game_Complete to Master
			Event2Post.EventType = ES_NORM_GAME_COMPLETE;
			PostMasterSM(Event2Post);
			SetMotorSensorDirection(REVERSE_DIR);
			FindTape();

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

void HallEffect_ISR( void )
{
	//	Static local variable LastTen array initialized to ten zeros
	static uint32_t LastTen[RUN_AVERAGE_LENGTH];
	static uint32_t LastDeltas[RUN_AVERAGE_LENGTH];
	static uint16_t Throwaway = 0;
	if (initHallEffect){
		for (int i=0;i<RUN_AVERAGE_LENGTH;i++){
			LastTen[i] = 0;
			LastDeltas[i] = 0;
			Throwaway = 0;
		}
		initHallEffect = false;
	}
	if (Throwaway< MAX_THROWAWAY){
		Throwaway++;
		return;
	}
	//	Static local variable 8 bit integer counter initialized to 0
	static uint8_t counter = 0;
	static uint8_t deltacounter = 0;
	ES_Event PostEvent;
	uint32_t ThisCapture, CurrentPeriod;
		
	// Clear source of interrupt
	HWREG(WTIMER2_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
	
	// Restart the one shot timer
	HWREG(WTIMER3_BASE+TIMER_O_TAV) = ONE_SHOT_TIMEOUT;
	
	// Clear last HallSensorPeriod
	HallSensorPeriod = 0;
	
	//	Get captured value
	//	Set CurrentPeriod to subtract LastCapture from ThisCapture
	ThisCapture = HWREG(WTIMER2_BASE+TIMER_O_TAR);
	CurrentPeriod = ThisCapture - LastCapture;
	
	LastTen[deltacounter] = CurrentPeriod;
	for(int i = 0; i <RUN_AVERAGE_LENGTH; i++){
		DeltaAvg += LastDeltas[i];
	}
	
	if(deltacounter == RUN_AVERAGE_LENGTH-1){
		deltacounter = 0;
	} else {
		deltacounter++;
	}
	DeltaAvg /= RUN_AVERAGE_LENGTH;
	//printf("period = %i, this=%i, Last=%i\r\n",CurrentPeriod,ThisCapture,LastCapture);
	if ((CurrentPeriod <= MAX_ALLOWABLE_PER) && (CurrentPeriod >= MIN_ALLOWABLE_PER)) {
		//	Update counter position in LastTen to CurrentPeriod
		LastTen[counter] = CurrentPeriod;
		
		//	Set HallSensorPeriod to average of LastTen
		for(int i = 0; i <RUN_AVERAGE_LENGTH; i++){
			HallSensorPeriod += LastTen[i];
		}
		HallSensorPeriod = HallSensorPeriod/RUN_AVERAGE_LENGTH;
		
		//	If HallSensorPeriod is less than MaxAllowablePer and greater than LeastAllowablePer 
		//	and HasLeftStage is true
		if((HallSensorPeriod <= MAX_ALLOWABLE_PER) && (HallSensorPeriod >= MIN_ALLOWABLE_PER) && HasLeftStage && DeltaAvg < 15) {
		//	Post ES_StationDetected Event
			PostEvent.EventType = ES_STATION_DETECTED;
			Throwaway = 0;
			
			PostMasterSM(PostEvent);
			//printf("Good Frequency: %i\r\n", HallSensorPeriod);
			for(int j = 0; j < RUN_AVERAGE_LENGTH; j++)
			{
				LastTen[j] = 0;
				LastDeltas[j] = 0;
			}
			HasLeftStage = false;
		} else if(HasLeftStage){
			//printf("Bad Period: %i\r\n", HallSensorPeriod);
		}
		//	If counter equals 9
		if(counter == RUN_AVERAGE_LENGTH-1){
			counter = 0;
		} else {
			counter++;
		}
	}
	LastCapture = ThisCapture;
	
	LastPeriod = CurrentPeriod;
}

void HallEffectOneShotTimer_ISR( void )
{	
	// Clear source of interrupt
	HWREG(WTIMER3_BASE+TIMER_O_ICR) = TIMER_ICR_TATOCINT;
	
	// Restart the one shot timer
	HWREG(WTIMER3_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
	
	// Set HasLeftStage to true
	HasLeftStage = true;
	
	HallSensorPeriod = 0;
	
	//printf("Left the current stage\r\n");
}

uint32_t getPeriod( void )
{
	return HallSensorPeriod;
}

uint8_t incrementScore(void){
	Score++;
	return Score;
}

uint8_t getScore(void){
	return Score;
}

uint8_t getTargetGoal(void) {
	return TargetGoal;
}

void setGameTimeoutFlag(bool flag)
{
	GameTimeoutFlag = flag;
}

bool getGameTimeoutFlag(void)
{
	return GameTimeoutFlag;
}

bool getHasLeftStage(void) {
	return HasLeftStage;
}

void setInitHallEffect(bool value){
	initHallEffect = value;
}