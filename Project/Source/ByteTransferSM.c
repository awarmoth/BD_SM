/****************************************************************************
 Module
   ByteTransferSM.c

 Revision
   2.0.1

 Description
	This is a state machine that handles sending the 5 successive bytes to the LOC 
	over SSI communication protocol.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/20/17 10:14 jec      correction to Run function to correctly assign 
                         ReturnEvent in the situation where a lower level
                         machine consumed an event.
 02/03/16 12:38 jec      updated comments to reflect changes made in '14 & '15
                         converted unsigned char to bool where appropriate
                         spelling changes on true (was True) to match standard
                         removed local var used for debugger visibility in 'C32
                         commented out references to Start & RunLowerLevelSM so
                         that this can compile. 
 02/07/13 21:00 jec      corrections to return variable (should have been
                         ReturnEvent, not CurrentEvent) and several EV_xxx
                         event names that were left over from the old version
 02/08/12 09:56 jec      revisions for the Events and Services Framework Gen2
 02/13/10 14:29 jec      revised Start and run to add new kind of entry function
                         to make implementing history entry cleaner
 02/13/10 12:29 jec      added NewEvent local variable to During function and
                         comments about using either it or Event as the return
 02/11/10 15:54 jec      more revised comments, removing last comment in during
                         function that belongs in the run function
 02/09/10 17:21 jec      updated comments about internal transitions on During funtion
 02/18/09 10:14 jec      removed redundant call to RunLowerlevelSM in EV_Entry
                         processing in During function
 02/20/07 21:37 jec      converted to use enumerated type for events & states
 02/13/05 19:38 jec      added support for self-transitions, reworked
                         to eliminate repeated transition code
 02/11/05 16:54 jec      converted to implment hierarchy explicitly
 02/25/03 10:32 jec      converted to take a passed event parameter
 02/18/99 10:19 jec      built template from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
// Framework inclusions
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

// Hardware definitions
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ByteTransferSM.h"
#include "SPI_Module.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines
#define LOC_COOLDOWN_TIME 2


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringWait2Start(ES_Event ThisEvent);
static ES_Event DuringWait4EOT(ES_Event ThisEvent);
static ES_Event DuringWait4Timeout(ES_Event ThisEvent);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ByteTransferState_t CurrentState;
static uint8_t ByteCounter;
//array of values written back by the LOC
//will need to check to see if static arrays are valid
static uint8_t BytesArray[5];

/*------------------------------ Module Code ------------------------------*/


/****************************************************************************
 Function
    RunByteTransferSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   This functions handles the execution of the Byte Transfer State Machine.
	 This machine idles until it receives a write command at which point it writes
	 5 bytes of data to the LOC.
 Notes
   uses nested switch/case to implement the machine.
 Author
 Brett Glasner, 2/22/17, 12:59AM
****************************************************************************/
ES_Event RunByteTransferSM(ES_Event CurrentEvent)
{
	//local variable MakeTransition
		bool MakeTransition;
	//local variable NextState
		ByteTransferState_t NextState;
	//local variable EntryEvent
		ES_Event EntryEvent;
	//local variable ReturnEvent
		ES_Event ReturnEvent;
	
	//Initialize MakeTransition to false
		MakeTransition = false;
	//Initialize NextState to CurrentState
		NextState = CurrentState;
	//Initialize EntryEvent to ES_ENTRY
		EntryEvent.EventType = ES_ENTRY;
	//Initialize ReturnEvent to CurrentEvent to assume no consumption of event
		ReturnEvent = CurrentEvent;
	
	switch(CurrentState)
	{
	//If CurrentState is BT_Wait2Start
		case BT_Wait2Start:
			
		//Run DuringWait2Start and store the output in CurrentEvent
			CurrentEvent = DuringWait2Start(CurrentEvent);
		
		//If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
			//If CurrentEvent is ES_Command
				if(CurrentEvent.EventType == ES_COMMAND) //IF WE ONLY WRITE 1 BYTE AT A TIME, HOW SHOULD WE PASS THE DATA WE WANT TO WRITE?
				{
				//Set MakeTransition to true
					MakeTransition = true;
				//Set NextState to BT_Wait4EOT
					NextState = BT_Wait4EOT;
					//Write the first byte command to the SPI Module
						//uint8_t QueryVal = ((uint8_t)CurrentEvent.EventParam);
						//QueryLOC(QueryVal);
					
				}
			//End ES_Command block
			}
		//Else
			else //CurrentEvent is an ES_NO_EVENT
			{
			//Update ReturnEvent
				ReturnEvent = CurrentEvent;
			}
		//EndIf
		
			break;
	//End Wait2Start block
	
	//If CurrentState is BT_Wait4EOT
		case BT_Wait4EOT:
			
		//Run DuringWait4EOT and store the output in CurrentEvent
			CurrentEvent = DuringWait4EOT(CurrentEvent);
		
		//If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
			//If CurrentEvent is ES_EOT and ByteCounter is less than 5
				if((CurrentEvent.EventType == ES_EOT) && (ByteCounter < 5))
				{
				//Set MakeTransition to true
					MakeTransition = true;
				//Store value written by LOC in BytesArray element corresponding to ByteCounter - 1
				//EventParam is a uint16_t, so we have to cast it down to a uint8_t
					BytesArray[ByteCounter-1] = ((uint8_t)CurrentEvent.EventParam);
				//Write the next byte command to the SPI Module
					//uint8_t QueryVal = 0; //Bytes 2-5 are always zeros
					//QueryLOC(QueryVal);
				}
			
			//ElseIf CurrentEvent is ES_EOT and ByteCounter is 5
				else if((CurrentEvent.EventType == ES_EOT) && (ByteCounter == 5))
				{
				//Set MakeTransition to true
					MakeTransition = true;
				//Store value written by LOC in BytesArray element corresponding to ByteCounter - 1
				//EventParam is a uint16_t, so we have to cast it down to a uint8_t
					BytesArray[ByteCounter-1] = ((uint8_t)CurrentEvent.EventParam);
				//Set NextState to BT_Wait4Timeout
					NextState = BT_Wait4Timeout;
				}

			}
			
		//Else CurrentEvent must be ES_NO_EVENT
			else
			{
			//Update ReturnEvent
				ReturnEvent = CurrentEvent;
			}
			
			break;
		//End Wait4EOT block
	
	//If CurrentState is BT_Wait4Timeout
		case BT_Wait4Timeout:
		//Run DuringWait4Timeout and store the output in CurrentEvent
			CurrentEvent = DuringWait4Timeout(CurrentEvent);
		
		//If CurrentEvent is not ES_NO_EVENT
			if(CurrentEvent.EventType != ES_NO_EVENT)
			{
			//If CurrentEvent is ES_TIMEOUT
				if(CurrentEvent.EventType == ES_TIMEOUT)
				{
				//Set MakeTransition to true
					MakeTransition = true;
				//Transform ReturnEvent to ES_Ready2Write
					ReturnEvent.EventType = ES_READY_2_WRITE;
				}
			}
		//Else CurrentEvent is ES_NO_EVENT
			else
			{
			//Update ReturnEvent
				ReturnEvent = CurrentEvent;
			}

			break;
	//End Wait4Timeout block
	
	} //end switch
	
	//If MakeTransition is true
		if(MakeTransition)
		{
		//Set CurrentEvent to ES_EXIT
			CurrentEvent.EventType = ES_EXIT;
		//Run ByteTransferSM with CurrentEvent to allow lower level SMs to exit
			RunByteTransferSM(CurrentEvent);
			
		//Set CurrentState to 
			CurrentState = NextState;
		//Run ByteTransferSM with EntryEvent to allow lower level SMs to enter
			RunByteTransferSM(EntryEvent);
		}

	//Return ReturnEvent
		return ReturnEvent;
}
/****************************************************************************
 Function
     StartByteTransferSM

 Parameters
     ES_Event the entry event to process

 Returns
     None

 Description
     Performs the entry procedure for the Byte Transfer state machine
 Notes

 Author
 Brett Glasner, 2/23/17, 12:46am
****************************************************************************/
void StartByteTransferSM(ES_Event CurrentEvent)
{
	//Set CurrentState to BT_Wait2Start
		CurrentState = BT_Wait2Start;
	//Run ByteTransferSM with CurrentEvent to initialize lower level SMs
		RunByteTransferSM(CurrentEvent);
}



/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringWait2Start(ES_Event ThisEvent)
{
	//local variable ReturnEvent
		ES_Event ReturnEvent;
	//Initialize ReturnEvent to ThisEvent
		ReturnEvent = ThisEvent;
	
	//If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY)){}
			//do nothing
		
	//ElseIf ThisEvent is ES_EXIT
		else if(ThisEvent.EventType == ES_EXIT)
		{
		//Reset the ByteCounter
			ByteCounter = 0;
		}
	
	//Else
		else{}
			//do nothing
	//EndIf
	
	//Return ReturnEvent
		return ReturnEvent;
	
}

static ES_Event DuringWait4EOT(ES_Event ThisEvent)
{
	//local variable ReturnEvent
		ES_Event ReturnEvent;
	//Initialize ReturnEvent to ThisEvent
		ReturnEvent = ThisEvent;
	
	//If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
		{
			
		//Write command to SPI module
			/**************** HOW DO WE KNOW WHAT VALUE TO WRITE HERE??????????????***************/
			
		//Increment ByteCounter
			ByteCounter++;
		}
		
	//ElseIf ThisEvent is ES_EXIT
		else if(ThisEvent.EventType == ES_EXIT)
		{
			//do nothing
		}
		
	//Else
		else
		{
			//do nothing
		}
	
	//Return ReturnEvent
		return ReturnEvent;
}


static ES_Event DuringWait4Timeout(ES_Event ThisEvent)
{
	//local variable ReturnEvent
		ES_Event ReturnEvent;
	//Initialize ReturnEvent to ThisEvent
		ReturnEvent = ThisEvent;
	
	//If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
		{
		//Start ByteTransfer timer for 2ms
			ES_Timer_InitTimer(LOC_TIMER, LOC_COOLDOWN_TIME);
		}
	
	//ElseIf ThisEvent is ES_EXIT
		else if(ThisEvent.EventType == ES_EXIT)
		{
			//do nothing
		}

	//Else
		else
		{
			//do nothing
		}
	
	//Return ReturnEvent
		return ReturnEvent;
	
}
