//module level variables: MyPriority, CurrentState, ShootingTimeoutFlag GameTimeoutFlag, ExitFlag, Score, BallCount
//ShootingState_t: AlignToGoal; 

static uint8_t Shooting_Timeout_Flag = 0;

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
                case (CurrentState == AlignToGoal):
		{
			// Run DuringAlignToGoal and store the output in CurrentEvent
			CurrentEvent = DuringAlignToGoal(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
                        if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_GOAL_BEACON_DETECTED
                                if (CurrentEvent.EventType == ES_GOAL_BEACON_DETECTED)
				{
					// Stop rotating
                                        //ROTATE//////////////////////////////////////////////////////////////////////////////////////////////////////
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
                                ReturnEven.EventType = ES_NO_EVENT;
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
                                        Shooting_Timeout_Flag = 1;
                                }
				// Else If CurrentEvent is ES_TIMEOUT from GAME_TIMER
                                else if
					// Transform ReturnEvent to ES_NO_EVENT
					// Set GameTimeoutFlag
				// Else If CurrentEvent is ES_FIRE_COMPLETE
					// Set MakeTransition to true
					// Set BallCount to getBallCount
					// Set NextState to WaitForShotResult
					// If BallCount = 0 or GameTimeoutFlag Set or ShootingTimeout Flag Set
						// Set ExitFlag
					// EndIf
				// EndIf
		
			// Else
				// Set ReturnEvent to ES_NO_EVENT
			// EndIf
		
		// End Firing block

		// If CurrentState is WaitForShotResult
	
			// Run DuringWaitForShotResult and store the output in CurrentEvent
		
			// If CurrentEvent is not ES_NO_EVENT
				// If CurrentEvent is ES_TIMEOUT from SHOT_RESULT_TIMER
					// Set MakeTransition to true
					// Set NextState to WaitForScoreUpdate
				// EndIf
				
			// Else
				// Set ReturnEvent to ES_NO_EVENT
			// EndIf
		
		// End WaitForShotResult block
	
		// If CurrentState is WaitForScoreUpdate
	
			// Run DuringWaitForScoreUpdate and store the output in CurrentEvent
		
			// If CurrentEvent is not ES_NO_EVENT
				// If CurrentEvent is ES_LOC_COMPLETE
					// Get response bytes from LOC
					// SetSB1_Byte(getSB1_Byte())
					// SetSB2_Byte(getSB2_Byte())
					// SetSB3_Byte(getSB3_Byte())

					// Set MakeTransition to true
					// Initialize NewScore to getScore
					// If NewScore = Score
						// Set NextState to Firing
					// Else
						// Set NextState to AlignToTape
					// EndIf
					// Score = NewScore
				// EndIf
		
			// Else
				// Set ReturnEvent to ES_NO_EVENT
			// EndIf
		
		// End WaitForScoreUpdate block

		// If CurrentState is AlignToTape
	
			// Run DuringAlignToTape and store the output in CurrentEvent
		
			// If CurrentEvent is not ES_NO_EVENT
				// If CurrentEvent is ES_TAPE_DETECTED
					// Transform ReturnEvent to ES_SHOOTING_COMPLETE
					// If GameTimeoutFlag Set
						// Post ES_NORMAL_GAME_COMPLETE to Master
					// EndIf
				// EndIf
				
			// Else
				// Set ReturnEvent to ES_NO_EVENT
			// EndIf
		
		// End AlignToTape block
	}//End switch
	
		// If MakeTransition is true
	
		// Set CurrentEvent to ES_EXIT
		// Run ShootingSM with CurrentEvent to allow lower level SMs to exit
	
		// Set CurrentState to NextState
		// Run ShootingSM with EntryEvent to allow lower level SMs to enter
		
	// EndIf
	
	// Return ReturnEvent
	
}



ES_Event DuringAlignToGoal(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		// Start rotating // direction based on team color
		// Set OldScore to getScore
		// Clear timeout flags
	// EndIf
	
	// Return ReturnEvent
	
}



ES_Event DuringFiring(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	// local variable Event2Post
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		// Set Event2Post to a ES_FIRE_COMMAND
		// Post Event2Post to Firing Service
	// EndIf
	
	// Return ReturnEvent
	
}



ES_Event DuringWaitForShotResult(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		// Start SHOT_RESULT_TIMER
	// EndIf
	
	// Return ReturnEvent
	
}



ES_Event DuringWaitForScoreUpdate(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	// local variable Event2Post
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		// Set Byte2Write to status byte
		// Post ES_Command to LOC w/ parameter: Byte2Write
	// EndIf
	
	// Return ReturnEvent
	
}


ES_Event DuringAlignToTape(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		// Start rotating // direction based on team color, opposite of AlignToGoal
	// EndIf
	
	// Return ReturnEvent
	
}


