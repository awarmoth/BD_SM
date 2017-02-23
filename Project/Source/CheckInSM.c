// module level variables: 
 static uint8_t MyPriority;
 static CheckInState_t CurrentState;
 static uint8_t ResponseReady;
 static uint8_t ReportStatus;
 static uint8_t BadResponseCounter = 0;
 static uint8_t Byte2Write;

 #define MAX_BAD_RESPONSES 3 
 #define RESPONSE_NOT_READY 0
 #define RESPONSE_READY 0xAA
 
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
					// Get response bytes from LOC
					SetRR_Byte(getRR_Byte());
					SetRS_Byte(getRS_Byte());
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
					}
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
			}
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
			if (CurrentEvent != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent == ES_LOC_COMPLETE)
				{
					// Set MakeTransition to true
					MakeTransition = true;
					// Set NextState to WaitForResponse_2
					NextState = WaitForResponse_2;
				// End ES_LOC_COMPLETE block
				}
			}	
			// Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent = ES_NO_EVENT;
			}
			// EndIf
			
		// End Reporting_2 block
		break;
		
		// If CurrentState is WaitForResponse_2
		case (WaitForResponse_2):
			// Run DuringWaitForResponse_2 and store the output in CurrentEvent
			CurrentEvent = DuringWaitForResponse_2(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent == ES_LOC_COMPLETE)
				{
					// Get response bytes from LOC
					SetRR_Byte(getRR_Byte());
					SetRS_Byte(getRS_Byte());
					// Set ResponseReady to getResponseReady
					ResponseReady = getResponseReady();
					// If ResponseReady = not ready
					if (ResponseReady == RESPONSE_NOT_READY)
					{
						// Set MakeTransition to true
						MakeTransition = true;
						// Transform ReturnEvent to ES_NO_EVENT
						ReturnEvent.EventType = ES_NO_EVENT;
					// Else If BadResponseCounter > MAX_BAD_RESPONSES
					}
					else if (BadResponseCounter > MAX_BAD_RESPONSES)
					{
						// Transform ReturnEvent to ES_Reorient
						ReturnEvent = ES_REORIENT;
					}
					// Else
					else
					{
						// Set ReportStatus to getReportStatus
						ReportStatus = getReportStatus();
						// If ReportStatus = ACK
						if (ReportStatus == ACK)
						{
							// Transform ReturnEvent to ES_GOAL_READY
							ReturnEvent.EventType = ES_GOAL_READY;
							// Set ReturnEvent parameter to getLocation //from Report Status byte
							ReturnEvent.EventParam = getLocation();
						}
						// Else If ReportStatus = NACK
						else if (ReportStatus == NACK)
						{
							// Increment BadResponseCounter
							BadResponseCounter++;
							// Set MakeTransition to true
							MakeTransition = true;
							// Set NextState to Reporting_2
							NextState = Reporting_2;
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
			}
			// 	Else
			else
			{
				// Set ReturnEvent to ES_NO_EVENT
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// 	EndIf
				
		// End WaitForResponse_2 block
		break;
	}
	
	// If MakeTransition is true
	if (MakeTransition == true)
	{
		// Set CurrentEvent to ES_EXIT
		CurrentEvent.EventType = ES_EXIT;
		// Run CheckInSM with CurrentEvent to allow lower level SMs to exit
		RunCheckInSM(CurrentEvent);
		// Set CurrentState to NextState
		CurrentState = NextState;
		// Run CheckInSM with EntryEvent to allow lower level SMs to enter
		RunCheckInSM(CurrentEvent);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
}
// End RunCheckInSM


ES_Event DuringReporting_1(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable Event2Post
	ES_Event Event2Post;
	// local variable Period
	uint32_t Period;
	
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if (ThisEvent.EventType == ES_ENTRY || ThisEvent.EventType == ES_ENTRY_HISTORY)
	{
		// Period = getPeriod // ISR is constantly updating
		Period = getPeriod();
		// Set Byte2Write to report byte based on Period
		Byte2Write = 0b1000 0000+getPeriodCode(Period);
		// Post ES_COMMAND to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC(Event2Post);
		// Reinitialize variables
		BadResponseCounter = 0;
		ResponseReady = RESPONSE_NOT_READY;


	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
	
}
// End DuringReporting_1


ES_Event DuringWaitForResponse_1(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if (ThisEvent.EventType == ES_ENTRY || ThisEvent.EventType == ES_ENTRY_HISTORY)
	{
		// Set Byte2Write to report query response byte
		Byte2Write = 0b0111 0000;
		// Post ES_Command to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC(Event2Post);
	}
	// EndIf

	// Return ReturnEvent
	return ReturnEvent;	
}
// End DuringWaitForResponse_1


ES_Event DuringReporting_2(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// local variable Event2Post
	ES_Event Event2Post;
	// local variable Period
	uint32_t Period;
	
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if (ThisEvent.EventType == ES_ENTRY || ThisEvent.EventType == ES_ENTRY_HISTORY)
	{
		// Period = getPeriod // ISR is constantly updating
		Period = getPeriod();
		// Set Byte2Write to report byte based on Period
		Byte2Write = 0b1000+getPeriodCode(Period);
		// Post ES_COMMAND to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC(Event2Post);
	}
	// EndIf
	
	// Return ReturnEvent
	return ReturnEvent;
	
}
// End DuringReporting_2


ES_Event DuringWaitForResponse_2(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if (ThisEvent.EventType == ES_ENTRY || ThisEvent.EventType == ES_ENTRY_HISTORY)
	{
		// Set Byte2Write to query response byte
		Byte2Write = 0b0111 0000;
		// Post ES_Command to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC(Event2Post);
	}
	// EndIf

	// Return ReturnEvent
	return ReturnEvent;	
}
// End DuringWaitForResponse_2

