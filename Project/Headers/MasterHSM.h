// MasterState_t: Waiting2Start, Constructing, Free4All, GameComplete
// Module level functions: DuringWaiting2Start, DuringConstructing, DuringFree4All, DuringGameComplete

#ifndef MasterHSM_H
#define MasterHSM_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// State definitions for use with the query function
typedef enum { Waiting2Start, Constructing, Free4All, GameComplete } MasterState_t ;

// Public Function Prototypes

ES_Event RunMasterSM( ES_Event CurrentEvent );
void StartMasterSM ( ES_Event CurrentEvent );
bool PostMasterSM( ES_Event ThisEvent );
bool InitMasterSM ( uint8_t Priority );
static ES_Event DuringWaiting2Start( ES_Event Event);
static ES_Event DuringConstructing( ES_Event Event);
static ES_Event DuringFree4All( ES_Event Event);
static ES_Event DuringGameComplete( ES_Event Event);
uint8_t SetSB1_Byte(uint8_t Byte2Write);
uint8_t SetSB2_Byte(uint8_t Byte2Write);
uint8_t SetSB3_Byte(uint8_t Byte2Write);
uint8_t SetRS_Byte(uint8_t Byte2Write);
uint8_t SetRR_Byte(uint8_t Byte2Write);
uint8_t getTeamColor(void);
uint8_t getCheckShootGreen(void);
uint8_t getActiveStageGreen(void);
uint8_t getActiveGoalGreen(void);
uint8_t getCheckShootRed(void);
uint8_t getActiveStageRed(void);
uint8_t getActiveGoalRed(void);
uint8_t getScoreGreen(void);
uint8_t getScoreRed(void);
uint8_t getGameState(void);
uint8_t getResponseReady(void);
uint8_t getReportStatus(void);
uint8_t getLocation(void);


#endif /*MasterHSM_H */
