/****************************************************************************
 Module
   MapKeys.c

 Revision
   1.0.0

 Description
   This service maps keystrokes to events 

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/16/17 21:27 asw      modified from template
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include <stdio.h>
#include <ctype.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MapKeys.h"
#include "LOC_HSM.h"
#include "MasterHSM.h"


/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMapKeys

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

****************************************************************************/
bool InitMapKeys ( uint8_t Priority )
{
  MyPriority = Priority;

  return true;
}

/****************************************************************************
 Function
     PostMapKeys

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

****************************************************************************/
bool PostMapKeys( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}


/****************************************************************************
 Function
    RunMapKeys

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   maps keys to Events for HierMuWave Example
 Notes
   
****************************************************************************/
ES_Event RunMapKeys( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    if ( ThisEvent.EventType == ES_NEW_KEY) // there was a key pressed
    {
				printf("Key Pressed = %d\r\n", ThisEvent.EventParam);
        switch ( toupper(ThisEvent.EventParam))
        {
          // This posts an ES_COMMAND to the LOC_SM with a parameter value of 12 (0b1100)
            case '1' : 
							ThisEvent.EventType = ES_COMMAND; 
							ThisEvent.EventParam = 192;
							printf("Status Command\r\n");
							PostLOC_SM(ThisEvent);
							
              break;
						
						case '2' :
							ThisEvent.EventType = ES_EOT;
							ThisEvent.EventParam = 0;
							//PostLOC_SM(ThisEvent);
						
							break;
						
						case '3' :
							ThisEvent.EventType = ES_EOT;
							ThisEvent.EventParam = 1;
							//PostLOC_SM(ThisEvent);
							
							break;
						
						case '4' :
							ThisEvent.EventType = ES_EOT;
							ThisEvent.EventParam = 2;
							//PostLOC_SM(ThisEvent);
							
							break;
						
						case '5' :
							ThisEvent.EventType = ES_EOT;
							ThisEvent.EventParam = 3;
							//PostLOC_SM(ThisEvent);
						
							break;
						
						case '6' :
							ThisEvent.EventType = ES_EOT;
							ThisEvent.EventParam = 4;
							//PostLOC_SM(ThisEvent);
						
							break;
						
						case '7' :
							ThisEvent.EventType = ES_COMMAND;
							ThisEvent.EventParam = 120;
							printf("Report Command\r\n");
							PostLOC_SM(ThisEvent);
						
							break;
						
						case '8' :
							
							ThisEvent.EventType = ES_COMMAND;
							ThisEvent.EventParam = 112;
							printf("Query Command\r\n");						
							PostLOC_SM(ThisEvent);						
						
							break;
						
						case '9' :
							
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE\r\n");						
							PostMasterSM(ThisEvent);
							break;
						
						case 'A':
							ThisEvent.EventType = ES_ARRIVED_AT_STATION;
							printf("ES_ARRIVED_AT_STATION\r\n");						
							PostMasterSM(ThisEvent);
							break;
						case 'S':
							ThisEvent.EventType = ES_REORIENT;
							printf("ES_REORIENT\r\n");						
							PostMasterSM(ThisEvent);
							break;
						case 'D':
							ThisEvent.EventType = ES_GOAL_READY;
							printf("ES_GOAL_READY\r\n");						
							PostMasterSM(ThisEvent);
							break;
						case 'F':
							ThisEvent.EventType = ES_DRIVE_ALONG_TAPE;
							printf("ES_DRIVE_ALONG_TAPE\r\n");
							PostMasterSM(ThisEvent);

        }

    }
    
  return ReturnEvent;
}

//								ES_START,
//								ES_TEAM_SWITCH,
//								ES_DRIVE_ALONG_TAPE,
//								ES_ARRIVED_AT_STATION,
//								ES_ARRIVED_AT_RELOAD,
//								ES_REORIENT,
//								ES_GOAL_READY,
//								ES_SHOOTING_COMPLETE,
//								ES_RELOAD_COMPLETE,
//								ES_RELOAD_START,
//								ES_NORM_GAME_COMPLETE,
//								ES_STATION_DETECTED,
//								ES_FRONT_BUMP_DETECTED,
//								ES_FREE_4_ALL_COMPLETE,
//								ES_START_FREE_4_ALL,
//								ES_NORMAL_GAME_COMPLETE

