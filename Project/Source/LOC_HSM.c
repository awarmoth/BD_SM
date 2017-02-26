/****************************************************************************
 Module
   LOC_HSM.c

 Revision
   1.0.0

 Description
   This is the Hierarchical state machine for managing the LOC communication

 Notes

 History
 When           Who     What/Why
 --------------
 02/22/17 21:59 bag      Began coding
 02/16/17 21:04 asw      Began converting template to LOC_HSM
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
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

//module includes
#include "LOC_HSM.h"
#include "ByteTransferSM.h"
#include "SPI_Module.h"
#include "MasterHSM.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringLOC_Waiting( ES_Event Event);
static ES_Event DuringLOC_Transmitting( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, though if the top level state machine
// is just a single state container for orthogonal regions, you could get
// away without it
static LOC_State_t CurrentState;
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLOC_SM

 Parameters
     uint8_t : the priority of this service

 Returns
     boolean, False if error in initialization, True otherwise

 Description
     Saves away the priority,  and starts
		 the LOC master state machine
 Notes

 Author
 Brett Glasner, 02/22/17, 21:53
****************************************************************************/

bool InitLOC_SM(uint8_t Priority)
{
		TERMIO_Init();
	//local variable ThisEvent
		ES_Event ThisEvent;
	//Initialize MyPriority to Priority
		MyPriority = Priority;
	
	//Initialize ThisEvent to ES_ENTRY
		ThisEvent.EventType = ES_ENTRY;
	
	//Initialize the SPI module
/***InitSPI_Comm();***************/
	
	//Call StartLOC_SM with ThisEvent as the passed parameter
		StartLOC_SM(ThisEvent);
	//Return true
		return true;
}


/****************************************************************************
 Function
     PostLOC_SM

 Parameters
     ES_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the post operation failed, True otherwise

 Description
     Posts an event to the LOC HSM event queue
 Notes

 Author
Brett Glasner, 02/22/17, 22:00
****************************************************************************/
bool PostLOC_SM( ES_Event ThisEvent )
{
	//Return ThisEvent posted successfully to the service associated with MyPriority
  return ES_PostToService( MyPriority, ThisEvent);
}


/****************************************************************************
 Function
    RunLOC_SM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   the run function for the LOC HSM top level state machine 
 Notes
   uses nested switch/case to implement the machine.
 Author
   Brett Glasner, 02/22/17, 22:03
****************************************************************************/
ES_Event RunLOC_SM( ES_Event CurrentEvent )
{
	//local variable MakeTransition
		bool MakeTransition;
	//local variable NextState
		LOC_State_t NextState;
	//local variable ReturnEvent
		ES_Event ReturnEvent;
	//local variable EntryEvent
		ES_Event EntryEvent;
	//local variable Event2Post
		ES_Event Event2Post;

	//Initialize MakeTransition to false
		MakeTransition = false;
	//Initialize NextState to CurrentState
		NextState = CurrentState;
	//Initialize EntryEvent to ES_ENTRY
		EntryEvent.EventType = ES_ENTRY;
	//Initialize ReturnEvent to ES_NO_EVENT
		ReturnEvent.EventType = ES_NO_EVENT;

		switch(CurrentState)
		{
			
		//If CurrentState is LOC_Waiting
			case LOC_Waiting:
	
				//Run DuringWaiting and store the output in CurrentEvent
				CurrentEvent = DuringLOC_Waiting(CurrentEvent);
			
				//If CurrentEvent is not an ES_NO_EVENT
				if(CurrentEvent.EventType != ES_NO_EVENT)
				{
					//If CurrentEvent is ES_Command
					if(CurrentEvent.EventType == ES_COMMAND)
					{
						printf("Moving to Transmitting %d\r\n", CurrentEvent.EventParam);
						//Post an ES_Command event with the same event parameter to the LOC_SM
						PostLOC_SM(CurrentEvent);
						//Set MakeTransition to true
						MakeTransition = true;
						//Set NextState to Transmitting
						NextState = LOC_Transmitting;
					}
					//End ES_Command block	
				}
				// 	Else
				else
				{
					// Set ReturnEvent to ES_NO_EVENT
					ReturnEvent = ES_NO_EVENT;
				}
				// 	EndIf	
				break;
			//End Waiting block
				
				
		//If CurrentState is LOC_Transmitting
			case LOC_Transmitting:
		
			//Run DuringTransmitting and store the output in CurrentEvent
				CurrentEvent = DuringLOC_Transmitting(CurrentEvent);
			
			//If CurrentEvent is not an ES_NO_EVENT
				if(CurrentEvent.EventType != ES_NO_EVENT)
				{
				//If CurrentEvent is ES_Ready2Write
					if(CurrentEvent.EventType == ES_READY_2_WRITE)
					{
					//Post ES_LOC_Complete to the MasterSM
						Event2Post.EventType = ES_LOC_COMPLETE;
					// PostMasterSM(Event2Post);
					//Set MakeTransition to true
						MakeTransition = true;
					//Set NextState to Waiting
						NextState = LOC_Waiting;
					}
				//End ES_Ready2Write block
				}
				// 	Else
				else
				{
					// Set ReturnEvent to ES_NO_EVENT
					ReturnEvent = ES_NO_EVENT;
				}
				// 	EndIf	
				break;
			//End Transmitting block
				
		}//end switch
		
	//If MakeTransition is true
		if(MakeTransition)
		{
	
		//Set CurrentEvent to ES_EXIT
			CurrentEvent.EventType = ES_EXIT;
		//Run LOC_SM with CurrentEvent to allow lower level SMs to exit
			RunLOC_SM(CurrentEvent);
		
		//Set CurrentState to NextState
			CurrentState = NextState;
		//RunLOC_SM with EntryEvent to allow lower level SMs to enter
			RunLOC_SM(EntryEvent);
		
		}
	
	//Return ReturnEvent
		return ReturnEvent;
}
/****************************************************************************
 Function
     StartMasterSM

 Parameters
     ES_Event CurrentEvent

 Returns
     nothing

 Description
     Does any required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 02/06/12, 22:15
****************************************************************************/
void StartLOC_SM ( ES_Event CurrentEvent )
{
	//Set CurrentState to LOC_Waiting
	CurrentState = LOC_Waiting;
	//Call RunLOC_SM with CurrentEvent as the passed parameter to initialize lower level SMs
	RunLOC_SM(CurrentEvent);
}


/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringLOC_Waiting(ES_Event ThisEvent)
{
	//local event ReturnEvent
	ES_Event ReturnEvent;
	//Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	//If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY do nothing
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY)){}
		
	//ElseIf ThisEvent is ES_EXIT do nothing
	else if(ThisEvent.EventType == ES_EXIT){}

	//Else do nothing
	else{}
	
	//Return ReturnEvent
		return ReturnEvent;
}


static ES_Event DuringLOC_Transmitting(ES_Event ThisEvent)
{
	//local event ReturnEvent
	ES_Event ReturnEvent;
	//Initialize ReturnEvent to ThisEvent
	ReturnEvent = ThisEvent;
	//If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
	if((ThisEvent.EventType == ES_ENTRY) || (ThisEvent.EventType == ES_ENTRY_HISTORY))
	{
		//Start ByteTransferSM
		StartByteTransferSM(ThisEvent);
	}
	
	//ElseIf ThisEvent is ES_EXIT do nothing
	else if(ThisEvent.EventType == ES_EXIT){}
	
	//Else
	else
	{
		//Run ByteTransferSM and store output in ReturnEvent
		ReturnEvent = RunByteTransferSM(ThisEvent);
	}
	
	//Return ReturnEvent
	return ReturnEvent;
}
