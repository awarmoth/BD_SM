// module level variables: 
 static uint8_t MyPriority;
 static CheckInState_t CurrentState;
 static uint8_t ResponseReady;
 static uint8_t ReportStatus;
 static uint8_t BadResponseCounter = 0;
 static uint8_t Byte2Write;

 #define MAX_BAD_RESPONSES 3 
 #define RESPONSE_NOT_READY 0
 #define RESPOSE_READY 0xAA
 
 #define ACK 0
 #define NACK 0b10
 #define INACTIVE 0b11


void StartCheckInSM(ES_Event CurrentEvent)
{
	// Set CurrentState to Reporting_1
	CurrentState = Reporting_1;
	// Run CheckInSM with CurrentEvent
	RunCheckInSM(CurrentEvent);
}
End StartCheckInSM


ES_Event RunCheckInSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	static bool MakeTransition;
	// local variable NextState
	static CheckInState_t NextState;
	// local variable EntryEvent
	static ES_Event EntryEvent;
	// local variable ReturnEvent
	static ES_Event ReturnEvent;

	// Initialize MakeTransition to false
	MakeTransition = false;
	// Initialize NextState to CurrentState
	NextState = CurrentState;
	// Initialize EntryEvent to ES_ENTRY
	EntryEvent = ES_ENTRY;
	// Initialize ReturnEvent to CurrentEvent to assume no consumption of event
	ReturnEvent = CurrentEvent;
	
	switch (CurrentState) {

		// If CurrentState is Reporting_1
		case(Reporting_1):	
			// 	Run DuringReporting_1 and store the output in CurrentEvent
			CurrentEvent = DuringReporting_1(CurrentEvent);
				
			// 	If CurrentEvent is not ES_NO_EVENT
			if ( CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to WaitForResponse_1
					NextState = WaitForResponse_1;
				// End ES_LOC_COMPLETE block		
				}
			}		
			// 	Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent = ES_NO_EVENT;
			}
			// 	EndIf	

			// End Reporting_1 block
			break;
		
		// If CurrentState is WaitForResponse_1
		case(WaitForResponse_1):
			// Run DuringWaitForResponse_1 and store the output in CurrentEvent
			CurrentEvent = DuringWaitForResponse_1(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					// Set ResponseReady to getResponseReady
					ResponseReady = getResponseReady();
					// If ResponseReady = not ready
					if (ResponseReady = RESPONSE_NOT_READY)
					{
						// Set MakeTransition to true
						MakeTransition = true;
						// Transform ReturnEvent to ES_NO_EVENT
						ReturnEvent.EventType = ES_NO_EVENT;
					// Else If BadResponseCounter > MaxBadResponses
					else if (BadResponseCounter > MAX_BAD_RESPONSES)
					{ 
						// Transform ReturnEvent to ES_Reorient
						ReturnEvent.EventType = ES_REORIENT;
					// Else
					}
					else
					{
						// Set Report Status to getReportStatus
						ReportStatus = getReportStatus();
						// If ReportStatus = ACK
						if ( ReportStatus == ACK )
						{
							// Reset BadResponseCounter
							BadResponseCounter = 0;
							// Set MakeTransition to true
							MakeTransition = true;
							// Set NextState to Reporting_2
							NextState = Reporting_2;
						}
						// Else If ReportStatus = NACK
						else if (ReportStatus == NACK)
						{ 
							// Increment BadResponseCounter
							BadResponseCounter++;
							// Set MakeTransition to true
							MakeTransition = true;
							// Set NextState to Reporting_1
							NextState = Reporting_1;
						}
						// Else // report status = inactive
						else
						{
							// Transform ReturnEvent to ES_Reorient
							ReturnEvent.EventType = ES_REORIENT;
						}
						// EndIf
					}
					// EndIf
				}
				// EndIf
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent = ES_NO_EVENT;
			}
			// EndIf
			
			// End WaitForResponse_1 block
			break;

		// If CurrentState is Reporting_2
		case ( Reporting_2 ):		
			// Run DuringReporting_2 and store the output in CurrentEvent
			CurrentEvent = DuringReporting_2(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if
		// 		If CurrentEvent is ES_LOC_COMPLETE
		// 			Set MakeTransition to true
		// 			Set NextState to WaitForResponse_2
		// 		End ES_LOC_COMPLETE block
					
		// 	Else
		// 		Set ReturnEvent to ES_NO_EVENT
		// 	EndIf
			
		// End Reporting_2 block
		
		// If CurrentState is WaitForResponse_2
		
		// 	Run DuringWaitForResponse_2 and store the output in CurrentEvent
			
		// 	If CurrentEvent is not ES_NO_EVENT
		// 		If CurrentEvent is ES_LOC_COMPLETE
		// 			Set ResponseReady to getResponseReady
		// 			If ResponseReady = not ready
		// 				Set MakeTransition to true
		// 				Transform ReturnEvent to ES_NO_EVENT
		// 			Else If BadResponseCounter > MaxBadResponses
		// 				Transform ReturnEvent to ES_Reorient
		// 			Else
		// 				Set ReportStatus to getReportStatus
		// 				If ReportStatus = ACK
		// 					Transform ReturnEvent to ES_Goal_Ready
		// 					Set ReturnEvent paramater to getLocation //from Report Status byte
		// 				Else If ReportStatus = NACK
		// 					Increment BadResponseCounter
		// 					Set MakeTransition to true
		// 					Set NextState to Reporting_2
		// 				Else // report status = inactive
		// 					Transform ReturnEvent to ES_Reorient
		// 				EndIf
		// 			EndIf
		// 		EndIf
			
		// 	Else
		// 		Set ReturnEvent to ES_NO_EVENT
		// 	EndIf
			
		// End WaitForResponse_2 block
	}
	
	// If MakeTransition is true
	
	// 	Set CurrentEvent to ES_EXIT
	// 	Run CheckInSM with CurrentEvent to allow lower level SMs to exit
		
	// 	Set CurrentState to NextState
	// 	Run CheckInSM with EntryEvent to allow lower level SMs to enter
		
	// EndIf
	
	// Return ReturnEvent
}
// End RunCheckInSM


ES_Event DuringReporting_1(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	// 	Frequency = getFrequency // ISR is constantly updating
	// 	Set Byte2Write to report byte based on Frequency
	// 	Post ES_Command to LOC w/ parameter: Byte2Write
	// EndIf
	
	// Return ReturnEvent
	
}
// End DuringReporting_1


ES_Event DuringWaitForResponse_1(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	// 	Set Byte2Write to report query response byte
	// 	Post ES_Command to LOC w/ parameter: Byte2Write
	// EndIf
	
	// Return ReturnEvent
	
}
// End DuringWaitForResponse_1


ES_Event DuringReporting_2(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	// 	Frequency = getFrequency // ISR is constantly updating
	// 	Set Byte2Write to report byte based on Frequency
	// 	Post ES_Command to LOC w/ parameter: Byte2Write
	// EndIf
	
	// Return ReturnEvent
	
}
// End DuringReporting_2


ES_Event DuringWaitForResponse_2(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	// 	Set Byte2Write to report query response byte
	// 	Post ES_Command to LOC w/ parameter: Byte2Write
	// EndIf
	
	// Return ReturnEvent
	
}
// End DuringWaitForResponse_2

