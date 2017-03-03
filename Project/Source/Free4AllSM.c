// Module level variables: MakeTransition, CurrentState
// FFAState_t: WaitingFFA, Driving2ReloadFFA, AlignFFA, RapidFiringFFA, FiringFFA, ReloadingFFA 
// Module level functions: DuringWaitingFFA, Driving2ReloadFFA, DuringAlignFFA, DuringRapidFiringFFA, DuringFiring, DuringReloading



void StartFFA_SM(ES_Event CurrentEvent)
{
	// Set CurrentState to WaitingFFA
	CurrentState = WaitingFFA;
	// Run FFA_SM with CurrentEvent to initialize lower level SMs
	RunFFA_SM(CurrentEvent);
}
// End StartByteTransferSM


ES_Event RunFFA_SM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	bool MakeTransition = false;
	// local variable NextState
	FFAState_t NextState = CurrentState;
	// local variable EntryEvent
	ES_Event EntryEvent;
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	ES_Event Event2Post;
	// Initialize EntryEvent to ES_ENTRY
	EntryEvent.EventType = ES_ENTRY;
	// Initialize ReturnEvent to CurrentEvent to assume no consumption of event
	ReturnEvent.EventType = CurrentEvent;
	// If CurrentState is WaitingFFA
	switch (CurrentState){
		case(WaitingFFA):
			// Run DuringWaitingFFA and store the output in CurrentEvent
			CurrentEvent = DuringWaitingFFA(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				// If CurrentEvent is ES_START_FFA
				if (CurrentEvent.EventType == ES_START_FFA){
					// Post ES_DRIVE_ALONG_TAPE to MasterHSM with SUPPLY_DEPOT as the value
					Event2Post.EventType = ES_DRIVE_ALONG_TAPE;
					Event2Post.EventParam = SUPPLY_DEPOT;
					PostMasterSM(Event2Post);
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to Driving2ReloadFFA
					NextState = Driving2ReloadFFA;
				}
				// EndIf
			}
			// EndIf
			
			// Else CurrentEvent must be an ES_NO_EVENT
			else {
				// Update ReturnEvent to be an ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End WaitingFFA block
		
		// If CurrentState is Driving2ReloadFFA
		case (Driving2ReloadFFA):
			// Run DuringDriving2ReloadFFA and store the output in CurrentEvent
			CurrentEvent = DuringDriving2ReloadFFA(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				// If CurrentEvent is ES_ARRIVED_AT_RELOAD
				if (CurrentEvent.EventType == ES_ARRIVED_AT_RELOAD) {
					// Set NextState to AlignFFA
					NextState = AlignFFA;
					// Set MakeTransition to true
					MakeTransition = true;
				}
				// EndIf
			}
			// EndIf
			
			// Else CurrentEvent must be an ES_NO_EVENT
			else { 
				// Update ReturnEvent to be an ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End Driving2ReloadFFA block
		
		// If CurrentState is AlignFFA
		case (AlignFFA):
			// Run DuringAlignFFA and store the output in CurrentEvent
			CurrentEvent = DuringAlignFFA(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT) {
				// If CurrentEvent is ES_FFA_READY and ball count is MAX_BALLS
				if ((CurrentEvent.EventType == ES_FFA_READY) && (BallCount == MAX_BALLS)) {
					// Set NextState to Firing
					NextState = FiringFFA;
					// Set MakeTransition to true
					MakeTransition = true;
				}
				// Else If CurrentEvent is ES_FFA_READY and ball count is NO_BALLS
				else if ((CurrentEvent.EventType == FFA_READY) && (BallCount == NO_BALLS)){
					// Set NextState to ReloadingFFA
					NextState = ReloadingFFA;
					// Set MakeTransition to true
					MakeTransition = true;
				}
				// Else If CurrentEvent is ES_FFA_READY
				else if (CurrentEvent.EventType = ES_FFA_READY) {
					// Set NextState to RapidFiringFFA
					NextState = RapidFiringFFA;
					// Set MakeTransition to true
					MakeTransition = true;
				}
				// EndIf
			}
			// EndIf
			// Else CurrentEvent must be an ES_NO_EVENT
			else {
				// Update ReturnEvent to be an ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End AlignFFA block
		
		// If CurrentState is FiringFFA
		case (FiringFFA):
			// Run DuringFiring and store the output in CurrentEvent
			CurrentEvent = DuringFiringFFA(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				// If CurrentEvent is ES_FIRE_COMPLETE
				if (CurrentEvent.EventType == ES_FIRE_COMPLETE) {
					// Post ES_RELOAD to Reload Service
					PostReloadService(ES_RELOAD);
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to RapidFiringFFA
					NextState = RapidFiringFFA;
				}
				// EndIf
				// Else If CurrentEvent is ES_TIMEOUT and the timer is FFA_TIMER
				else if ((CurrentEvent.EventType == ES_TIMEOUT) && CurrentEvent.EventParam = FFA_TIMER){
					// Consume the event and set the FFA_Timeout flag to true
					ReturnEvent.EventType = ES_NO_EVENT;
					FFA_Timeout = true;
					// Set MakeTransition to true
					MakeTransition = true;
				}
				// EndIf
			}
			// EndIf
			// Else CurrentEvent must be an ES_NO_EVENT
			else {
				Update ReturnEvent to be an ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
		break;
		// End Firing block
		
		If CurrentState is ReloadingFFA 
		case (ReloadingFFA)
			Run DuringReloading and store the output in CurrentEvent
			
			If CurrentEvent is not ES_NO_EVENT
				If CurrentEvent is ES_RELOAD_COMPLETE
					Post ES_FIRE to Firing Service
					Set MakeTransition to true
					Set NextState to RapidFiringFFA
				EndIf
				
				//do we need an else if there as an ES_TIMEOUT because the game ended?
				
			EndIf
			
			Else CurrentEvent must have been an ES_NO_EVENT
				Update ReturnEvent to be ES_NO_EVENT
			EndIf
		
		End Reloading block
		
		If CurrentState is RapidFiringFFA
		
			Run DuringRapidFiringFFA and store the output in CurrentEvent
			
			If CurrentEvent is not ES_NO_EVENT
				If CurrentEvent is ES_FIRE_COMPLETE and ball count is NO_BALLS
					Set MakeTransition to true
					Set NextState to Reloading
				EndIf
				
				Else If CurrentEvent is ES_RELOAD_COMPLETE and ball count is MAX_BALLS
					Set MakeTransition to true
					Set NextState to Firing
				EndIf
				
				Else If CurrentEvent is ES_FIRE_COMPLETE //ball count not zero
					Post ES_FIRE to Firing Service
					If FFA_Timeout flag set then post ES_FFA_COMPLETE to MasterHSM //do we need to post? Shouldn't we just transform and pass back up?
					Set MakeTransition to true
				EndIf
				
				Else If CurrentEvent is ES_RELOAD_COMPLETE //ball count not 5
					Post ES_RELOAD to Reload Service
					Set MakeTransition to true //do these need to make transitions? State doesn't do anything on entry or exit
				EndIf
				
				Else If CurrentEvent is ES_TIMEOUT and timer is FFA_TIMER
					Consume the event and set the FFA_Timeout flag to true //why don't we just pass back up an end game event?
					Set MakeTransition to true
				EndIf
			EndIf
			
			Else CurrentEvent must be an ES_NO_EVENT
				Update ReturnEvent to an ES_NO_EVENT
			EndIf
		
		End RapidFiringFFA block
	
	If MakeTransition is true
	
		Set CurrentEvent to ES_EXIT
		Run the FFA state machine with CurrentEvent to allow lower level SMs to exit
		
		Set CurrentState to NextState
		Run the FFA state machine with EntryEvent to allow lower level SMs to enter
	
	EndIf
	
	Return ReturnEvent

}
End RunFFA_SM



static ES_Event DuringWaitingFFA(ES_Event ThisEvent)
{
	local variable ReturnEvent
	
	Initialize ReturnEvent to ThisEvent
	
	If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		do nothing
	EndIf
	
	Else If ThisEvent is ES_EXIT
		do nothing
	EndIf
	
	Else 
		do nothing
	EndIf
	
	Return ReturnEvent
	
}
End DuringWaitingFFA



static ES_Event DuringDriving2ReloadFFA(ES_Event ThisEvent)
{
	local variable ReturnEvent
	
	Initialize ReturnEvent to ThisEvent
	
	If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		do nothing
	EndIf
	
	Else If ThisEvent is ES_EXIT
		Turn off drive motors
		Disable wire following control //these functions might both be handled inside the exit of the DrivingAlongTapeSM
	EndIf
	
	Else 
		Run the DrivingAlongTape state machine
	EndIf
	
	Return ReturnEvent
}
End DuringDriving2ReloadFFA



static ES_Event DuringAlignFFA(ES_Event ThisEvent)
{
	//on entry enable motor encoder control
	//enable FFA beacon detector
	//if red begin rotating motors one way
	//if green begin rotating motors opposite way
	local variable ReturnEvent
	
	Initialize ReturnEvent to ThisEvent
	
	If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		Enable motor encoder control
		Enable FFA beacon detector
		If red begin rotating motors one way
		If green begin rotating motors opposite way
	EndIf
	
	Else If ThisEvent is ES_EXIT
		//all of the motor turnoffs should be handled after processing a beacon detected event (or maybe FFA_READY?)
	EndIf
	
	Else 
		do nothing
	EndIf
	
	Return ReturnEvent	
}
End DuringAlignFFA



static ES_Event DuringRapidFiringFFA(ES_Event ThisEvent)
{
	local variable ReturnEvent
	
	Initialize ReturnEvent to ThisEvent
	
	If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		do nothing
	EndIf
	
	Else If ThisEvent is ES_EXIT
		do nothing
	EndIf
	
	Else 
		do nothing
	EndIf
	
	Return ReturnEvent	
}
End DuringRapidFiringFFA



static ES_Event DuringFiring(ES_Event ThisEvent)
{
	local variable ReturnEvent
	
	Initialize ReturnEvent to ThisEvent
	
	If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		Post ES_FIRE event to Firing Service
	EndIf
	
	Else If ThisEvent is ES_EXIT
		If FFA_Timeout flag set Post ES_FFA_COMPLETE to MasterHSM //why not just transforma and pass back up?
	EndIf
	
	Else 
		do nothing
	EndIf
	
	Return ReturnEvent
	
}
End DuringFiring



static ES_Event DuringReloading(ES_Event ThisEvent)
{
	local variable ReturnEvent
	
	Initialize ReturnEvent to ThisEvent
	
	If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		Post ES_RELOAD to Reload Service
	EndIf
	
	Else If ThisEvent is ES_EXIT
		do nothing //why do none of the reload services/events deal with if an ES_TIMEOUT appears? If they ignore it we could miss the timeout
	EndIf
	
	Else 
		do nothing
	EndIf
	
	Return ReturnEvent
	
}
End DuringReloading