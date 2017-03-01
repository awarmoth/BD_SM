#include "MasterHSM.h"
#include "SPI_Module.h"
#include "ByteTransferSM.h"
#include "LOC_HSM.h"
#include "ConstructingSM.h"
#include "DrivingAlongTapeSM.h"
#include "hardware.h"

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

// module level variables: MyPriority, CurrentState, TeamColor, GameState
static uint8_t myPriority;
static MasterState_t CurrentState;
static uint8_t TeamColor;
static uint8_t GameState;
// Most recent LOC responses
static uint8_t SB1_Byte;
static uint8_t SB2_Byte;
static uint8_t SB3_Byte;
static uint8_t RR_Byte;
static uint8_t RS_Byte;

bool InitMasterSM(uint8_t Priority)
{
	// local variable ThisEvent
	ES_Event ThisEvent;
	// Initialize MyPriority to Priority
	myPriority = Priority;
	// Initialize ThisEvent to ES_ENTRY
	ThisEvent.EventType = ES_ENTRY;
	// Initialize the SPI module
	InitSPI_Comm();
	InitializePins();
	// Call StartMasterSM with ThisEvent as the passed parameter
	StartMasterSM(ThisEvent);
	// Return true
	return true;
}
// End InitMasterSM


bool PostMasterSM(ES_Event ThisEvent)
{
	// Return ThisEvent posted successfully to the service associated with MyPriority
	return ES_PostToService( myPriority, ThisEvent);
}
// End PostMasterSM


void StartMasterSM(ES_Event CurrentEvent)
{
	// Set CurrentState to Waiting2Start
	CurrentState = Waiting2Start;
	// Call RunMasterSM with CurrentEvent as the passed parameter 
	// to initialize lower level SMs
	RunMasterSM(CurrentEvent);
}
// End StartMasterSM


ES_Event RunMasterSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	bool MakeTransition;
	// local variable NextState
	MasterState_t NextState;
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable EntryEvent
	ES_Event EntryEvent;
	
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
		// If CurrentState is Waiting2Start
		case(Waiting2Start):
		if (SM_TEST) printf("Master: Waiting2Start\r\n");
		// Run DuringWaiting2Start and store the output in CurrentEvent
			CurrentEvent = DuringWaiting2Start(CurrentEvent);
//			printf("curr event: %i",CurrentEvent.EventType);
			// If CurrentEvent is not an ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					if (!NO_LOC){
					// Get response bytes from LOC
					SB1_Byte = getSB1_Byte();
					SB2_Byte = getSB2_Byte();
					SB3_Byte = getSB3_Byte();
					}
					// Set GameState to getGameState
					GameState = getGameState();
					// If GameState is WAITING_FOR_START
					if (GameState == WAITING_FOR_START)
					{	
						// Set MakeTransition to true
						MakeTransition = true;
					}
					// Else
					else
					{	
						// Set MakeTransition to true
						MakeTransition = true;
						// Set NextState to Constructing
						NextState = Constructing;
					}
					// EndIf
				}
				// Else If Event is ES_TEAM_SWITCH // from event checker
				else if (CurrentEvent.EventType == ES_TEAM_SWITCH)
				{
					// Set MakeTransition to true
					MakeTransition = true;
				}
				// EndIf
			}
			// 	Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// 	EndIf	
		break;
		// End Waiting2Start block
	
		// If CurrentState is Constructing
		case(Constructing):
			if (SM_TEST) printf("Master: Constructing\r\n");
			// Run DuringConstructing and store the output in CurrentEvent
			CurrentEvent = DuringConstructing(CurrentEvent);
			// If CurrentEvent is not an ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_NORMAL_GAME_COMPLETE
				if (CurrentEvent.EventType == ES_NORMAL_GAME_COMPLETE)
				{
					// Post ES_START_FREE_4_ALL to Master
					ES_Event Event2Post;
					Event2Post.EventType = ES_START_FREE_4_ALL;
					PostMasterSM(Event2Post);
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to Free4All
					NextState = Free4All;
				}
				// Else If CurrentEvent is ES_TIMEOUT from GAME_TIMER
				else if ((CurrentEvent.EventType == ES_TIMEOUT) &&
						(CurrentEvent.EventParam == GAME_TIMER))
				{
					printf("here");
					// Post ES_START_FREE_4_ALL to Master
					ES_Event Event2Post;
					Event2Post.EventType = ES_START_FREE_4_ALL;
					PostMasterSM(Event2Post);
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to Free4All
					NextState = Free4All;
				}
				// EndIf
			}
			// 	Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// 	EndIf
			break;
		// End Constructing block
	
		// If CurrentState is Free4All
		case (Free4All):
			if (SM_TEST) printf("Master: Free4All\r\n");
			// Run DuringConstructing and store the output in CurrentEvent
			CurrentEvent = DuringConstructing(CurrentEvent);
			// If CurrentEvent is not an ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_FREE_4_ALL_COMPLETE
				if (CurrentEvent.EventType == ES_FREE_4_ALL_COMPLETE)
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to GameComplete
					NextState = GameComplete;
				}
				// Else If CurrentEvent is ES_TIMEOUT from FREE_4_ALL_TIMER
				else if ((CurrentEvent.EventType == ES_TIMEOUT) &&
						(CurrentEvent.EventParam == FREE_4_ALL_TIMER))
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to GameComplete
					NextState = GameComplete;
					if (SM_TEST) printf("Master: GameComplete\r\n");
				}
				// EndIf
			}
			// 	Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// 	EndIf
		break;
		// End Free4All block
	}
	// If MakeTransition is true
	if (MakeTransition == true)
	{
		// Set CurrentEvent to ES_EXIT
		CurrentEvent.EventType = ES_EXIT;
		// Run MasterSM with CurrentEvent to allow lower level SMs to exit
		RunMasterSM(CurrentEvent);
		// Set CurrentState to NextState
		CurrentState = NextState;
		// RunMasterSM with EntryEvent to allow lower level SMs to enter
		RunMasterSM(EntryEvent);
	}
	// EndIf
	// Return ReturnEvent
	return ReturnEvent;
}
// End RunMasterSM


static ES_Event DuringWaiting2Start(ES_Event ThisEvent)
{
	// local event ReturnEvent
	ES_Event ReturnEvent;
	// local event Event2Post
	ES_Event Event2Post;
	// local uint8_t Byte2Write
	uint8_t Byte2Write;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		if (SM_TEST) printf("Master: Entering Waiting2Start\r\n");
		// Set TeamColor to getTeamColor
		TeamColor = getTeamColor();
		// Turn on respective LEDs
		//TurnOnLEDs(TeamColor);
		// Set Event2Post type to ES_COMMAND
		Event2Post.EventType = ES_COMMAND;
		// Set Byte2Write to status byte
		Byte2Write = STATUS_COMMAND;
		Event2Post.EventParam = Byte2Write;
		// Post Event2Post to LOC_HSM
		if (NO_LOC) printf("Posting Command: Status to LOC\r\n");
		else PostLOC_SM(Event2Post);
	}
	// Return ReturnEvent
	return ReturnEvent;
}
// End DuringWaiting2Start



static ES_Event DuringConstructing(ES_Event ThisEvent)
{
	// local event ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		if (SM_TEST) printf("Master: Entering Constructing\r\n");
		// Start one shot timer
		HWREG(WTIMER3_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
				
		// Start ConstructingSM
		StartConstructingSM(ThisEvent);
		
		// Start GAME_TIMER
		//ES_Timer_InitTimer(GAME_TIMER,GAME_TIMEOUT);
	}
	// Else
	else if (ThisEvent.EventType == ES_EXIT){
		if (SM_TEST) printf("Master: Exiting Constructing");
	}
	else
	{
		// Run ConstructingSM and store output in ReturnEvent
		if (SM_TEST) printf("Master: Constructing: Running Constructing SM\r\n");

		ReturnEvent = RunConstructingSM(ThisEvent);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}

static ES_Event DuringFree4All(ES_Event ThisEvent)
{
	// local event ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		if (SM_TEST) printf("Master: Entering Free4All\r\n");
		// Start Free4AllSM
		//StartFree4AllSM(ThisEvent);
		// Start FREE_4_ALL_TIMER
		//ES_Timer_InitTimer(FREE_4_ALL_TIMER, FREE_4_ALL_TIMEOUT);
	}
	else if (ThisEvent.EventType == ES_EXIT){
		if (SM_TEST) printf("Master: Exiting Free4All");
	}
	else
	{
		// Run Free4AllSM and store output in ReturnEvent
		//ReturnEvent = RunFree4AllSM(ThisEvent);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}

static ES_Event DuringGameComplete(ES_Event ThisEvent)
{
	// local event ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		if (SM_TEST) printf("Master: Entering Free4All\r\n");
		
		// Turn off hardware/peripherals
		// Stop functions/idle
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}

uint8_t SetSB1_Byte(uint8_t Byte2Write) {
	SB1_Byte = Byte2Write;
	return SB1_Byte;
}

uint8_t SetSB2_Byte(uint8_t Byte2Write) {
	SB2_Byte = Byte2Write;
	return SB2_Byte;

}

uint8_t SetSB3_Byte(uint8_t Byte2Write) {
	SB3_Byte = Byte2Write;
	return SB3_Byte;
}

uint8_t SetRS_Byte(uint8_t Byte2Write) {
	RS_Byte = Byte2Write;
	return RS_Byte;
}

uint8_t SetRR_Byte(uint8_t Byte2Write) {
	RR_Byte = Byte2Write;
	return RR_Byte;
}

uint8_t getTeamColor(void) {
	return TeamColor;
}

uint8_t getCheckShootGreen(void) {
	return (SB1_Byte & CHECK_SHOOT_GREEN_MASK) >> CHECK_SHOOT_GREEN_RIGHT_SHIFT;
}

uint8_t getActiveStageGreen(void) {
	return (SB1_Byte & GREEN_STAGE_ACTIVE_MASK) >> GREEN_STAGE_ACTIVE_RIGHT_SHIFT;
}

uint8_t getActiveGoalGreen(void) {
	return (SB1_Byte & GREEN_GOAL_ACTIVE_MASK) >> GREEN_GOAL_ACTIVE_RIGHT_SHIFT;
}

uint8_t getCheckShootRed(void) {
	return (SB1_Byte & CHECK_SHOOT_RED_MASK) >> CHECK_SHOOT_RED_RIGHT_SHIFT;
}

uint8_t getActiveStageRed(void) {
	return (SB1_Byte & RED_STAGE_ACTIVE_MASK) >> RED_STAGE_ACTIVE_RIGHT_SHIFT;
}

uint8_t getActiveGoalRed(void) {
	return (SB1_Byte & RED_GOAL_ACTIVE_MASK) >> RED_GOAL_ACTIVE_RIGHT_SHIFT;
}

uint8_t getScoreGreen(void) {
	return SB2_Byte & GREEN_SCORE_MASK;
}

uint8_t getScoreRed(void) {
	return SB2_Byte & RED_SCORE_MASK;

}
uint8_t getGameState(void) {
	return (SB3_Byte & GAME_STATUS_MASK) >> GAME_STATUS_RIGHT_SHIFT;
}

uint8_t getResponseReady(void) {
	return RR_Byte;
}

uint8_t getReportStatus(void) {
	return (RS_Byte & REPORT_STATUS_MASK) >> REPORT_STATUS_RIGHT_SHIFT;
}

uint8_t getLocation(void) {
	return RS_Byte & LOCATION_MASK;
}

