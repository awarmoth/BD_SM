Module level variables: MakeTransition, CurrentState
FFAState_t: Waiting, Driving2Reload, AlignFFA, RapidFiring, Firing, Reloading 
Module level functions: DuringWaiting, Driving2Reload, DuringAlignFFA, DuringRapidFiring, DuringFiring, DuringReloading



void StartFFA_SM(ES_Event CurrentEvent)
{
	Set CurrentState to Waiting
	Run FFA_SM with CurrentEvent to initialize lower level SMs
}
End StartByteTransferSM




ES_Event RunFFA_SM(ES_Event CurrentEvent)
{
	local variable MakeTransition
	local variable NextState
	local variable EntryEvent
	local variable ReturnEvent
	
	Initialize MakeTransition to false
	Initialize NextState to CurrentState
	Initialize EntryEvent to ES_ENTRY
	Initialize ReturnEvent to CurrentEvent to assume no consumption of event
	
	If CurrentState is Waiting
		
		Run DuringWaiting and store the output in CurrentEvent
		
		If CurrentEvent is not ES_NO_EVENT
			If CurrentEvent is ES_START_FFA
				Post ES_DRIVE_ALONG_TAPE to MasterHSM with SUPPLY_DEPOT as the value
				Set MakeTransition to true
				Set NextState to Driving2Reload
			EndIf
		EndIf
		
		Else CurrentEvent must be an ES_NO_EVENT
			Update ReturnEvent to be an ES_NO_EVENT
		EndIf
		
	End Waiting block
	
	If CurrentState is Driving2Reload
	
		Run DuringDriving2Reload and store the output in CurrentEvent
		
		If CurrentEvent is not ES_NO_EVENT
			If CurrentEvent is ES_ARRIVED_AT_RELOAD
				Set NextState to AlignFFA
				Set MakeTransition to true
			EndIf
		EndIf
		
		Else CurrentEvent must be an ES_NO_EVENT
			Update ReturnEvent to be an ES_NO_EVENT
		EndIf
		
	End Driving2Reload block
	
	If CurrentState is AlignFFA

		Run DuringAlignFFA and store the output in CurrentEvent
		
		If CurrentEvent is not ES_NO_EVENT
			If CurrentEvent is ES_FFA_READY and ball count is MAX_BALLS
				Set NextState to Firing
				Set MakeTransition to true
			
			Else If CurrentEvent is ES_FFA_READY and ball count is NO_BALLS
				Set NextState to Reloading
				Set MakeTransition to true
				
			Else If CurrentEvent is ES_FFA_READY
				Set NextState to RapidFiring
				Set MakeTransition to true
			
			EndIf
		EndIf
		
		Else CurrentEvent must be an ES_NO_EVENT
			Update ReturnEvent to be an ES_NO_EVENT
		EndIf
	
	End AlignFFA block
	
	If CurrentState is Firing
		
		Run DuringFiring and store the output in CurrentEvent
		
		If CurrentEvent is not ES_NO_EVENT
			If CurrentEvent is ES_FIRE_COMPLETE
				Post ES_RELOAD to Reload Service
				Set MakeTransition to true
				Set NextState to RapidFiring
			EndIf
			
			Else If CurrentEvent is ES_TIMEOUT and the timer is FFA_TIMER
				Consume the event and set the FFA_Timeout flag to true //Why do we consume? Isn't the game over? Shouldn't we just pass a game over event back up?
				Set MakeTransition to true
			EndIf
		EndIf
		
		Else CurrentEvent must be an ES_NO_EVENT
			Update ReturnEvent to be an ES_NO_EVENT
		EndIf
	
	End Firing block
	
	If CurrentState is Reloading 
	
		Run DuringReloading and store the output in CurrentEvent
		
		If CurrentEvent is not ES_NO_EVENT
			If CurrentEvent is ES_RELOAD_COMPLETE
				Post ES_FIRE to Firing Service
				Set MakeTransition to true
				Set NextState to RapidFiring
			EndIf
			
			//do we need an else if there as an ES_TIMEOUT because the game ended?
			
		EndIf
		
		Else CurrentEvent must have been an ES_NO_EVENT
			Update ReturnEvent to be ES_NO_EVENT
		EndIf
	
	End Reloading block
	
	If CurrentState is RapidFiring
	
		Run DuringRapidFiring and store the output in CurrentEvent
		
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
	
	End RapidFiring block
	
	If MakeTransition is true
	
		Set CurrentEvent to ES_EXIT
		Run the FFA state machine with CurrentEvent to allow lower level SMs to exit
		
		Set CurrentState to NextState
		Run the FFA state machine with EntryEvent to allow lower level SMs to enter
	
	EndIf
	
	Return ReturnEvent

}
End RunFFA_SM



static ES_Event DuringWaiting(ES_Event ThisEvent)
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
End DuringWaiting



static ES_Event DuringDriving2Reload(ES_Event ThisEvent)
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
End DuringDriving2Reload



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



static ES_Event DuringRapidFiring(ES_Event ThisEvent)
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
End DuringRapidFiring



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