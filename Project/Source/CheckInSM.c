#include "MasterHSM.h"
#include "ConstructingSM.h"
#include "CheckInSM.h"
#include "LOC_HSM.h"
#include "ByteTransferSM.h"

#include "constants.h"
#include "termio.h"

// module level variables: 
 static CheckInState_t CurrentState;
 static uint8_t ResponseReady;
 static uint8_t ReportStatus;
 static uint8_t BadResponseCounter = 0;
 static uint8_t Byte2Write;

 #define MAX_BAD_RESPONSES 10 
 #define RESPONSE_NOT_READY 0
 #define RESPONSE_READY 0xAA
 
 #define ACK 0
 #define NACK 0x02
 #define INACTIVE 0x03


void StartCheckInSM(ES_Event CurrentEvent)
{
	// Set CurrentState to Reporting_1
	CurrentState = Reporting_1;
	// Run CheckInSM with CurrentEvent
	RunCheckInSM(CurrentEvent);
}
// End StartCheckInSM


ES_Event RunCheckInSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	static bool MakeTransition;
	// local variable NextState
	CheckInState_t NextState = CurrentState;
	// local variable EntryEvent
	static ES_Event EntryEvent;
	// local variable ReturnEvent
	static ES_Event ReturnEvent;

	// Initialize MakeTransition to false
	MakeTransition = false;
	// Initialize EntryEvent to ES_ENTRY
	EntryEvent.EventType = ES_ENTRY;
	// Initialize ReturnEvent to CurrentEvent to assume no consumption of event
	ReturnEvent = CurrentEvent;
	
	switch (CurrentState) {
		// If CurrentState is Reporting_1
		case(Reporting_1):{
			if(SM_TEST) printf("CheckIn: Reporting_1\r\n");
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
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// 	EndIf	

		// End Reporting_1 block
		break;
		}
		
		// If CurrentState is WaitForResponse_1
		case(WaitForResponse_1):{
			//if(SM_TEST) printf("CheckIn: WaitingForResponse_1\r\n");
			// Run DuringWaitForResponse_1 and store the output in CurrentEvent
			CurrentEvent = DuringWaitForResponse_1(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
				{
					//printf("Got response\r\n");
					// Get response bytes from LOC
					SetRR_Byte(getRR_Byte());
					SetRS_Byte(getRS_Byte());
					// Set ResponseReady to getResponseReady
					ResponseReady = getResponseReady();
					ReportStatus = getReportStatus();
					// If ResponseReady = not ready
					if (ResponseReady == RESPONSE_NOT_READY)
					{
						//printf("Response not ready");
						// Set MakeTransition to true
						MakeTransition = true;
						// Transform ReturnEvent to ES_NO_EVENT
						ReturnEvent.EventType = ES_NO_EVENT;
					// Else If BadResponseCounter > MaxBadResponses
					}
					else if (BadResponseCounter > MAX_BAD_RESPONSES)
					{ 
						printf("here");
						// Transform ReturnEvent to ES_Reorient
						ReturnEvent.EventType = ES_REORIENT;
					// Else
					}
					else
					{
						printf("ReportStatus = %i\r\n",ReportStatus);
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
						else if ( ReportStatus == NACK )
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
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
			
		// End WaitForResponse_1 block
		break;}

		// If CurrentState is Reporting_2
		case ( Reporting_2): {
			if(SM_TEST) printf("CheckIn: Reporting_2\r\n");
			// Run DuringReporting_2 and store the output in CurrentEvent
			CurrentEvent = DuringReporting_2(CurrentEvent);
			// If CurrentEvent is not ES_NO_EVENT
			if (CurrentEvent.EventType != ES_NO_EVENT)
			{
				// If CurrentEvent is ES_LOC_COMPLETE
				if (CurrentEvent.EventType == ES_LOC_COMPLETE)
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
				ReturnEvent.EventType = ES_NO_EVENT;
			}
			// EndIf
			
		// End Reporting_2 block
		break;}
		
		// If CurrentState is WaitForResponse_2
		case (WaitForResponse_2):{
			if(SM_TEST) printf("CheckIn: WaitForResponse_2\r\n");
			// Run DuringWaitForResponse_2 and store the output in CurrentEvent
			CurrentEvent = DuringWaitForResponse_2(CurrentEvent);
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
					// Set ReportStatus to getReportStatus
					ReportStatus = getReportStatus();

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
						ReturnEvent.EventType = ES_REORIENT;
					}
					// Else
					else
					{
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
		break;}
	}
	
	// If MakeTransition is true
	if (MakeTransition == true)
	{
		// Set CurrentEvent to ES_EXIT
		CurrentEvent.EventType = ES_EXIT;
		// Run CheckInSM with CurrentEvent
		RunCheckInSM(CurrentEvent);
		// Set CurrentState to NextState
		CurrentState = NextState;
		// Run CheckInSM with EntryEvent
		RunCheckInSM(EntryEvent);
//		if (NextState == WaitForResponse_1) printf("wfr\r\n");
//		else printf("not wfr\r\n");
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
	if ((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Period = getPeriod // ISR is constantly updating
		Period = getPeriod();
		// Set Byte2Write to report byte based on Period
		Byte2Write = REPORT_COMMAND;
		Byte2Write += getPeriodCode(Period);
		printf("Period:%i",getPeriodCode(Period));
		// Post ES_COMMAND to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC_SM(Event2Post);
		// Reinitialize variables
//		BadResponseCounter = 0;
//		ResponseReady = RESPONSE_NOT_READY;
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
	ES_Event Event2Post;
	
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if ((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Set Byte2Write to report query response byte
		Byte2Write = QUERY_RESPONSE_COMMAND;
		// Post ES_Command to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC_SM(Event2Post);
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
	if ((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Period = getPeriod // ISR is constantly updating
		Period = getPeriod();
		// Set Byte2Write to report byte based on Period
		Byte2Write = REPORT_COMMAND;
		Byte2Write += getPeriodCode(Period);
		// Post ES_COMMAND to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC_SM(Event2Post);
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
	ES_Event Event2Post;
	
	// Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if ((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		// Set Byte2Write to query response byte
		Byte2Write = QUERY_RESPONSE_COMMAND;
		// Post ES_Command to LOC w/ parameter: Byte2Write
		Event2Post.EventType = ES_COMMAND;
		Event2Post.EventParam = Byte2Write;
		PostLOC_SM(Event2Post);
	}
	// EndIf

	// Return ReturnEvent
	return ReturnEvent;	
}
// End DuringWaitForResponse_2

uint8_t getPeriodCode(uint32_t Period) {
//	If Period is less than or equal to 1361 us and greater than 1305
	if(Period <= 1361*TICKS_PER_USEC && Period > 1305*TICKS_PER_USEC){
//		Return 0x00
	return 0x00;
//	Elseif Period is less than or equal to 1305 and greater than 1249
	}else if(Period <= 1305*TICKS_PER_USEC && Period > 1249*TICKS_PER_USEC){
//		Return 0x01
	return 0x01;
//	Elseif rPeriod is less than or equal to 1249 and greater than 1194
	}else if(Period <= 1249*TICKS_PER_USEC && Period > 1194*TICKS_PER_USEC){
//		Return 0x02
	return 0x02;
//	Elseif Period is less than or equal to 1194 and greater than 1138
	}else if(Period <= 1194*TICKS_PER_USEC && Period > 1138*TICKS_PER_USEC){
//		Return 0x03
	return 0x03;
//	Elseif Period is less than or equal to 1138 and greater than 1083
	}else if(Period <= 1138*TICKS_PER_USEC && Period > 1083*TICKS_PER_USEC){
//		Return 0x04
	return 0x04;
//	Elseif Period is less than or equal to 1083 and greater than 1027
	}else if(Period <= 1083*TICKS_PER_USEC && Period > 1027*TICKS_PER_USEC){
//		Return 0x05
	return 0x05;
//	Elseif Period is less than or equal to 1027 and greater than 972
	}else if(Period <= 1027*TICKS_PER_USEC && Period > 972*TICKS_PER_USEC){
//		Return 0x06
	return 0x06;
//	Elseif Period is less than or equal to 972 and greater than 916
	}else if(Period <= 972*TICKS_PER_USEC && Period > 916*TICKS_PER_USEC){
//		Return 0x07
	return 0x07;
//	Elseif Period is less than or equal to 916 and greater than 861
	}else if(Period <= 916*TICKS_PER_USEC && Period > 861*TICKS_PER_USEC){
//		Return 0x08
	return 0x09;
//	Elseif Period is less than or equal to 861 and greater than 805
	}else if(Period <= 861*TICKS_PER_USEC && Period > 805*TICKS_PER_USEC){
//		Return 0x09
		return 0x09;
//	Elseif Period is less than or equal to 805 and greater than 750
	}else if(Period <= 805*TICKS_PER_USEC && Period > 750*TICKS_PER_USEC){
//		Return 0x0A
		return 0x0A;
//	Elseif Period is less than or equal to 750 and greater than 694
	}else if(Period <= 750*TICKS_PER_USEC && Period > 694*TICKS_PER_USEC){
//		Return 0x0B
		return 0x0B;
//	Elseif Period is less than or equal to 694 and greater than 639
	}else if(Period <= 694*TICKS_PER_USEC && Period > 639*TICKS_PER_USEC){
//		Return 0x0C
		return 0x0C;
//	Elseif Period is less than or equal to 639 and greater than 583
	}else if(Period <= 639*TICKS_PER_USEC && Period > 583*TICKS_PER_USEC){
//		Return 0x0D
		return 0x0D;
//	Elseif Period is less than or equal to 583 and greater than 528
	}else if(Period <= 583*TICKS_PER_USEC && Period > 528*TICKS_PER_USEC){
//		Return 0x0E
		return 0x0E;
//	Else Period is less than or equal to 528 and greater than 472
	}else if(Period <= 528*TICKS_PER_USEC && Period > 472*TICKS_PER_USEC) {
//		Return 0x0F
		printf("yay I think we found the frequency\r\n");
		return 0x0F;
//	Endif
	} else {
		printf("Wrong frequency: only should get here in SM testing");
		return 0xFF;
	}
}

void ClearBadResponseCounter(void) {
	BadResponseCounter = 0;
}
