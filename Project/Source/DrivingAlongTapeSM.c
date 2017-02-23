/****************************************************************************
 Module
   DrivingAlongTapeSM.c

 Revision
   2.0.1

 Description
	This is a state machine that handles navigation of the robot along the wire
	to the check in stations or to the reload station.

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


/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines



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

//array of values written back by the LOC
//will need to check to see if static arrays are valid


/*------------------------------ Module Code ------------------------------*/


/****************************************************************************
 Function
    RunDrivingAlongTapeSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   This functions handles the execution of the DrivingAlongTape state machine.
	 This machine idles until it receives a target station which will either be
	 a staging area or reloading station. Then it will drive to that station.
 Notes
   uses nested switch/case to implement the machine.
 Author
 Brett Glasner, 2/22/17, 12:59AM
****************************************************************************/
ES_Event RunDrivingAlongTapeSM(ES_Event CurrentEvent)
{
	// local variable MakeTransition
	// local variable NextState
	// local variable EntryEvent
	// local variable ReturnEvent
	
	// Initialize MakeTransition to false
	// Initialize NextState to CurrentState
	// Initialize EntryEvent to ES_ENTRY by default
	// Initialize ReturnEvent to CurrentEvent assuming no consuming of event
	
	// If CurrentState is Waiting //NOTE CORRECT STATE MACHINE TO NOT TURN ON TAPE CONTROL ON WAITING EXIT
		
		// Call DuringWaiting and set CurrentEvent to its return value
		
		// If CurrentEvent is not an ES_NO_EVENT
		
			// If CurrentEvent is ES_DriveAlongTape
			
				// If TargetStation is LastStation //module variables can be set from checking the event param
					// Set MakeTransition to true				
					// Set ReturnEvent to ES_ArrivedAtStation
				// Endif
				
				// If TargetStation is not LastStation and TargetStation is not supply depot //module variables can be set from checking the event param
					// Set NextState to DrivingToStation
					// Set MakeTransition to true
					// Set ReturnEvent to ES_NO_EVENT
					// Enable wire following control law
					// Start driving
					// Set TargetStation to getTargetStation
					// Set Direction from sign(LastStation - TargetStation)
				// Endif
				
				// If TargetStation is supply depot //module variables aren't set until exit runs
					// Set NextState to DrivingToReload
					// Set MakeTransition to true
					// Set ReturnEvent to ES_NO_EVENT
					// Enable wire following control law
					// Start driving
					// Set TargetStation to getTargetStation
					// Set Direction from sign(LastStation - TargetStation)
				// Endif
			
			// End ES_DriveAlongTape block
			
		// Endif
		
		// Else CurrentEvent is an ES_NO_EVENT
			// Update ReturnEvent to ES_NO_EVENT
		// Endf
		
	// End Waiting block
	
	// If CurrentState is DrivingToStation
		
		// Call DuringDrivingToStation and set CurrentEvent to its return value
		
		// If CurrentEvent is not an ES_NO_EVENT
		
			// If CurrentEvent is ES_StationDetected
			
				// If LastStation - Direction is TargetStation
					// Set MakeTransition to true	
					// Stop driving
					// Disable wire following control law
					// Set ReturnEvent to ES_ArrivedAtStation
				// Endif
				

				//Else we must not be at the target station so we keep going
					// Set MakeTransition to true
				// Endif
			
			// End ES_StationDetected block
			
		// Endif
		
		// Else CurrentEvent is an ES_NO_EVENT
			// Update ReturnEvent to ES_NO_EVENT
		// Endf
		
	// End DrivingToStation block
	
	// If CurrentState is DrivingToReload
		
		// Call DuringDrivingToReload and set CurrentEvent to its return value
		
		// If CurrentEvent is not an ES_NO_EVENT
		
			// If CurrentEvent is ES_Front_Bump_Detected
				// Set MakeTransition to true	
				// Stop driving
				// Disable wire following control law
				// Set ReturnEvent to ES_ArrivedAtReload			
			// End ES_Front_Bump_Detected block
			
		// Endif
		
		// Else CurrentEvent is an ES_NO_EVENT
			// Update ReturnEvent to ES_NO_EVENT
		// Endf
	
	// End DrivingToReload block
	
	// If MakeTransition is true
		
		// Set CurrentEvent to ES_EXIT
		// Call RunDrivingAlongTapeSM with CurrentEvent as the event parameter
					
		// Set CurrentState to NextState
		// Call RunDrivingAlongTapeSM with EntryEvent as the event parameter
			
	// Endif
	
	// Return ReturnEvent
}
/****************************************************************************
 Function
     StartDrivingAlongTapeSM

 Parameters
     ES_Event the entry event to process

 Returns
     None

 Description
     Performs the entry procedure for the Driving Along Tape state machine
 Notes

 Author
 Brett Glasner, 2/23/17, 11:49am
****************************************************************************/
void StartDrivingAlongTapeSM(ES_Event CurrentEvent)
{
	// If CurrentEvent is not ES_ENTRY_HISTORY
		// Set CurrentState to Waiting
	// Endif
	
	// Call RunDrivingAlongTapeSM with CurrentEvent as the event parameter
}



/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringWaiting(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent assuming no re-mapping or consumption
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY
		// Set LastStation to getLastStation
		// //Probably want to initialize module variables here as well
	// EndIf
	
	// ElseIf ThisEvent is ES_EXIT

	// EndIf
	
	// Return ReturnEvent
}

static ES_Event DuringDrivingToStation(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent assuming no re-mapping or consumption
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY

	// EndIf
	
	// ElseIf ThisEvent is ES_EXIT
		// Set LastStation to (LastStation - Direction)
	// EndIf
	
	// Return ReturnEvent
}


static ES_Event DuringDrivingToReload(ES_Event ThisEvent)
{
	// local variable ReturnEvent
	
	// Initialize ReturnEvent to ThisEvent assuming no re-mapping or consumption
	
	// If ThisEvent is ES_ENTRY or ES_ENTRY_HISTORY

	// EndIf
	
	// ElseIf ThisEvent is ES_EXIT
		// Set LastStation to supply depot
	// EndIf
	
	// Return ReturnEvent
}
