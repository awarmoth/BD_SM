// module level variables: MyPriority, CurrentState, TeamColor, GameState
static uint8_t myPriority;
static MasterState_t CurrentState;
static uint8_t TeamColor;
static uint8_t GameState;

bool InitMasterSM(uint8_t Priority)
{
	// local variable ThisEvent
	ES_Event ThisEvent;
	// Initialize MyPriority to Priority
	myPriority = Priority;
	// Initialize ThisEvent to ES_NO_EVENT
	ThisEvent.EventType = ES_NO_EVENT;
	// Initialize the SPI module
	InitSPI_Comm();
	// Call StartMasterSM with ThisEvent as the passed parameter
	StartMasterSM(ThisEvent);
	// Return true
	return true;
}
// End InitMasterSM


bool PostMasterSM(ES_Event ThisEvent)
{
	// Return ThisEvent posted successfully to the service associated with MyPriority
	return ES_PostToService( MyPriority, ThisEvent);
}
// End PostMasterSM


void StartMasterSM(ES_Event CurrentEvent)
{
	// Set CurrentState to Waiting2Start
	CurrentState = Waiting2Start
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
	EntryEvent = ES_ENTRY;
	// Initialize ReturnEvent to ES_NO_EVENT
	ReturnEvent = ES_NO_EVENT;
	
	switch (CurrentState)
	{
		// If CurrentState is Waiting2Start
		case(Waiting2Start):
		// Run DuringWaiting2Start and store the output in CurrentEvent
			CurrentEvent = DuringWaiting2Start(CurrentEvent);
			// If CurrentEvent is not an ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					// Get response bytes from LOC
					SetSB1_Byte(getSB1_Byte());
					SetSB2_Byte(getSB2_Byte());
					SetSB3_Byte(getSB3_Byte());
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
				ReturnEvent = ES_NO_EVENT;
			}
			// 	EndIf	
		break;
		// End Waiting2Start block
	
		// If CurrentState is Constructing
		case(Constructing):
			// Run DuringConstructing and store the output in CurrentEvent
			CurrentEvent = DuringConstructing(CurrentEvent);
			// If CurrentEvent is not an ES_NO_EVENT
			if (CurrentEvent.EventType == ES_NO_EVENT)
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
						(CurrentEvent.EventParam = GAME_TIMER))
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
				// EndIf
			}
			// 	Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent = ES_NO_EVENT;
			}
			// 	EndIf
			break;
		// End Constructing block
	
		// If CurrentState is Free4All
		case (Free4All):
			// Run DuringConstructing and store the output in CurrentEvent
			CurrentEvent = DuringConstructing(CurrentEvent)
			// If CurrentEvent is not an ES_NO_EVENT
			if (CurrentEvent.EventType == ES_NO_EVENT)
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
						(CurrentEvent.EventParam = FREE_4_ALL_TIMER))
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to GameComplete
					NextState = GameComplete;
				}
				// EndIf
			}
			// 	Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent = ES_NO_EVENT;
			}
			// 	EndIf
		break;
		// End Free4All block
	
	// If MakeTransition is true
	if (MakeTransition == true)
	{
		// Set CurrentEvent to ES_EXIT
		CurrentEvent = ES_EXIT;
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
		// Set TeamColor to getTeamColor
		TeamColor = getTeamColor();
		// Turn on respective LEDs
		TurnOnLEDs(TeamColor);
		// Set Event2Post type to ES_COMMAND
		Event2Post.EventType = ES_COMMAND;
		// Set Byte2Write to status byte
		Byte2Write = STATUS_COMMAND;
		// Post Event2Post to LOC_HSM
		PostLOC_SM(Event2Post);
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
		// Start ConstructingSM
		StartConstructingSM(ThisEvent);
		// Start GAME_TIMER
		ES_Timer_InitTimer(GAME_TIMER,GAME_TIMEOUT);
	}
	// Else
	else
	{
		// Run ConstructingSM and store output in ReturnEvent
		ReturnEvent = ConstructingSM(ThisEvent);
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
		// Start Free4AllSM
		StartFree4AllSM(ThisEvent);
		// Start FREE_4_ALL_TIMER
		ES_Timer_InitTimer(FREE_4_ALL_TIMER, FREE_4_ALL_TIMEOUT);
	}
	// Else
	else
	{
		// Run Free4AllSM and store output in ReturnEvent
		ReturnEvent = RunFree4AllSM(ThisEvent);
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
		// Turn off hardware/peripherals
		// Stop functions/idle
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}

