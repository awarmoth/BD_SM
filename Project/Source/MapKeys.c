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
#include <stdlib.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MapKeys.h"
#include "LOC_HSM.h"
#include "MasterHSM.h"
#include "constants.h"
#include "ByteTransferSM.h"


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
							ThisEvent.EventParam = STATUS_COMMAND;
							printf("Status Command\r\n");
							if (NO_LOC) printf("Posting Command: Status to LOC\r\n");
							else PostLOC_SM(ThisEvent);
							
              break;
						
						case '2' :
							ThisEvent.EventType = ES_COMMAND;
							ThisEvent.EventParam = REPORT_COMMAND;
							printf("Report Command\r\n");
							if (NO_LOC) printf("Posting Command: Report to LOC\r\n");
							else PostLOC_SM(ThisEvent);PostLOC_SM(ThisEvent);						
							break;
						
						case '3' :
							
							ThisEvent.EventType = ES_COMMAND;
							ThisEvent.EventParam = QUERY_RESPONSE_COMMAND;
							printf("Query Command\r\n");						
							if (NO_LOC) printf("Posting Command: Query to LOC\r\n");
							else PostLOC_SM(ThisEvent);PostLOC_SM(ThisEvent);						
							break;
						
						case '4' :
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE\r\n");						
							PostMasterSM(ThisEvent);
							break;
						
						case '5' :
							
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE: GameStatus = Waiting\r\n");						
							PostMasterSM(ThisEvent);
							SetSB3_Byte(getSB3_Byte() & ~GAME_STATUS_MASK);
							break;
						
						case '6' :
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE: GameStatus = Game Started\r\n");						
							PostMasterSM(ThisEvent);
							SetSB3_Byte(getSB3_Byte() | GAME_STATUS_MASK);
							break;
						
						case '7' :
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE: Response Ready = Not Ready\r\n");						
							PostMasterSM(ThisEvent);
							SetRR_Byte(RESPONSE_NOT_READY);
							break;
						
						case '8' :
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE: Response Ready = Ready, Report Status = ACK\r\n");						
							PostMasterSM(ThisEvent);
							SetRR_Byte(RESPONSE_READY);
							SetRS_Byte((getRS_Byte() & ~REPORT_STATUS_MASK) | REPORT_ACK);
							break;
						
						case '9':
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE: Response Ready = Ready, Report Status = NACK\r\n");						
							PostMasterSM(ThisEvent);
							SetRR_Byte(RESPONSE_READY);
							SetRS_Byte((getRS_Byte() & ~REPORT_STATUS_MASK) | REPORT_NACK);
							break;
							
						case '0':
							ThisEvent.EventType = ES_LOC_COMPLETE;
							printf("ES_LOC_COMPLETE: Response Ready = Ready, Report Status = INACTIVE\r\n");						
							PostMasterSM(ThisEvent);
							SetRR_Byte(RESPONSE_READY);
							SetRS_Byte((getRS_Byte() & ~REPORT_STATUS_MASK) | REPORT_INACTIVE);
							break;
						
													
						case 'Q':
							ThisEvent.EventType = ES_FIRE_COMPLETE;
							bool GameTimeout = getGameTimeout();
							bool Exit = getExitFlag();
							printf("ES_FIRE_COMPLETE: GameTimeout = %i, Exit = %i", Exit, GameTimeout); 
							break;
						case 'W':
							ThisEvent.EventType = ES_TAPE_DETECTED;
							printf("ES_TAPE_DETECTED");
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
							ThisEvent.EventType = ES_STATION_DETECTED;
							printf("ES_STATION_DETECTED\r\n");
							PostMasterSM(ThisEvent);
							break;
						case 'G':
							ThisEvent.EventType = ES_SHOOTING_COMPLETE;
							printf("ES_SHOOTING_COMPLETE\r\n");
							PostMasterSM(ThisEvent);
							break;
						case 'H':
							ThisEvent.EventType = ES_DRIVE_ALONG_TAPE;
							ThisEvent.EventParam = rand() % 3 + 1;
							printf("ES_DRIVE_ALONG_TAPE: Target = %i", ThisEvent.EventParam);
							PostMasterSM(ThisEvent);
							break;
						
						
						
	
						case 'Z':
							ThisEvent.EventType = ES_TIMEOUT;
							ThisEvent.EventParam = SHOOTING_TIMER;
							printf("ES_TIMEOUT: SHOOTING_TIMER\r\n");
							PostMasterSM(ThisEvent);
							break;
						case 'X':
							ThisEvent.EventType = ES_TIMEOUT;
							ThisEvent.EventParam = GAME_TIMER;
							printf("ES_TIMEOUT: GAME_TIMER\r\n");
							PostMasterSM(ThisEvent);
							break;
						case 'C':
							ThisEvent.EventType = ES_TIMEOUT;
							ThisEvent.EventParam = FREE_4_ALL_TIMER;
							printf("ES_TIMEOUT: FREE_4_ALL_TIMER\r\n");
							PostMasterSM(ThisEvent);
							break;
						case 'V':
							ThisEvent.EventType = ES_TIMEOUT;
							ThisEvent.EventParam = SHOT_RESULT_TIMER;
							printf("ES_TIMEOUT: SHOT_RESULT_TIMER\r\n");
							PostMasterSM(ThisEvent);
							break;
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

